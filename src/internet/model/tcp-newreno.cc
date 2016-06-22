/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Adrian Sai-wah Tam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Adrian Sai-wah Tam <adrian.sw.tam@gmail.com>
 */

#define NS_LOG_APPEND_CONTEXT \
  if (m_node) { std::clog << Simulator::Now ().GetSeconds () << " [node " << m_node->GetId () << "] "; }

#include "tcp-newreno.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "ns3/node.h"
#include "ns3/ipv4-end-point.h"
#include "ns3/ipv4-l3-protocol.h"

NS_LOG_COMPONENT_DEFINE ("TcpNewReno");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpNewReno);

TypeId
TcpNewReno::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpNewReno")
    .SetParent<TcpSocketBase> ()
    .AddConstructor<TcpNewReno> ()
    .AddAttribute ("ReTxThreshold", "Threshold for fast retransmit",
                    UintegerValue (3),
                    MakeUintegerAccessor (&TcpNewReno::m_retxThresh),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("LimitedTransmit", "Enable limited transmit",
                   BooleanValue (false),
                   MakeBooleanAccessor (&TcpNewReno::m_limitedTx),
                   MakeBooleanChecker ())
    .AddAttribute ("dctcp", "Enable DCTCP",
                   BooleanValue (false),
                   MakeBooleanAccessor (&TcpNewReno::m_dctcp),
                   MakeBooleanChecker ())
    .AddTraceSource ("CongestionWindow",
                     "The TCP connection's congestion window",
                     MakeTraceSourceAccessor (&TcpNewReno::m_cWnd))
    .AddTraceSource ("SlowStartThreshold",
                     "TCP slow start threshold (bytes)",
                     MakeTraceSourceAccessor (&TcpNewReno::m_ssThresh))
    .AddAttribute ("xfabric", "xfabric",
                   BooleanValue (true),
                   MakeBooleanAccessor (&TcpNewReno::m_xfabric),
                   MakeBooleanChecker ())
    .AddAttribute ("strawman", "strawman",
                   BooleanValue (false),
                   MakeBooleanAccessor (&TcpNewReno::m_strawmancc),
                   MakeBooleanChecker ())
	  .AddAttribute("dt_value",
                  "Target delay for this flow in us",
                  DoubleValue(0.000024),
                  MakeDoubleAccessor (&TcpNewReno::m_dt),
                  MakeDoubleChecker <double> ())
	  .AddAttribute("line_rate",
                  "line_rate",
                  DoubleValue(10000.0),
                  MakeDoubleAccessor (&TcpNewReno::line_rate),
                  MakeDoubleChecker <double> ())
 ;
  return tid;
}

TcpNewReno::TcpNewReno (void)
  : m_retxThresh (3), // mute valgrind, actual value set by the attribute system
    m_inFastRec (false),
    m_limitedTx (false) // mute valgrind, actual value set by the attribute system
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC("TcpNewReno constructor-1 ");
  init_values();

}

void
TcpNewReno::init_values(void)
{
  d0 = 0.000016; //setting it at 200us. But, we need to set it to right value link_delay*max_links*2 from command line
//  dt = 0.000048;
  m_dt = 0.00048;
  highest_ack_recvd = 0;
  beta = 1.0/32.0;
  bytes_with_ecn = total_bytes_acked = 0.0;
  dctcp_alpha = 0.0;
  dctcp_reacted = false;
  xfabric_reacted = false;
  utilize_link = true;
  one_rtt = 0;
  ecn_is_one = false;
  
}
  

TcpNewReno::TcpNewReno (const TcpNewReno& sock)
  : TcpSocketBase (sock),
    m_cWnd (sock.m_cWnd),
    m_ssThresh (sock.m_ssThresh),
    m_initialCWnd (sock.m_initialCWnd),
    m_initialSsThresh (sock.m_initialSsThresh),
    m_retxThresh (sock.m_retxThresh),
    m_inFastRec (false),
    m_limitedTx (sock.m_limitedTx)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("Invoked the copy constructor");
  NS_LOG_LOGIC("TcpNewReno constructor-2 ");
  one_rtt = 0;
  init_values();
}

TcpNewReno::~TcpNewReno (void)
{
}

/* We initialize m_cWnd from this function, after attributes initialized */
int
TcpNewReno::Listen (void)
{
  NS_LOG_FUNCTION (this);
  InitializeCwnd ();
  return TcpSocketBase::Listen ();
}

/* We initialize m_cWnd from this function, after attributes initialized */
int
TcpNewReno::Connect (const Address & address)
{
  NS_LOG_FUNCTION (this << address);
  InitializeCwnd ();
  return TcpSocketBase::Connect (address);
}

/* Limit the size of in-flight data by cwnd and receiver's rxwin */
uint32_t
TcpNewReno::Window (void)
{
  NS_LOG_FUNCTION (this);
  return std::min (m_rWnd.Get (), m_cWnd.Get ());
}

Ptr<TcpSocketBase>
TcpNewReno::Fork (void)
{
  return CopyObject<TcpNewReno> (this);
}

/* New ACK (up to seqnum seq) received. Increase cwnd and call TcpSocketBase::NewAck() */
void
TcpNewReno::NewAck (const SequenceNumber32& seq)
{

  NS_LOG_FUNCTION (this << seq);
  NS_LOG_LOGIC ("TcpNewReno received ACK for seq " << seq <<
                " cwnd " << m_cWnd <<
                " ssthresh " << m_ssThresh);

  if(!m_strawmancc) {
  // Check for exit condition of fast recovery
  if (m_inFastRec && seq < m_recover)
    { // Partial ACK, partial window deflation (RFC2582 sec.3 bullet #5 paragraph 3)
      m_cWnd += m_segmentSize - (seq - m_txBuffer.HeadSequence ());
      NS_LOG_INFO ("Partial ACK in fast recovery: cwnd set to " << m_cWnd);
      m_txBuffer.DiscardUpTo(seq);  //Bug 1850:  retransmit before newack
      DoRetransmit (); // Assume the next seq is lost. Retransmit lost packet
      TcpSocketBase::NewAck (seq); // update m_nextTxSequence and send new data if allowed by window
      return;
    }
  else if (m_inFastRec && seq >= m_recover)
    { // Full ACK (RFC2582 sec.3 bullet #5 paragraph 2, option 1)
      m_cWnd = std::min (m_ssThresh.Get (), BytesInFlight () + m_segmentSize);
      m_inFastRec = false;
      NS_LOG_INFO ("Received full ACK. Leaving fast recovery with cwnd set to " << m_cWnd);
    }
  }

  if(!m_xfabric && !m_strawmancc && !ecn_is_one) {
  // Increase of cwnd based on current phase (slow start or congestion avoidance)
  if (m_cWnd < m_ssThresh)
    { // Slow start mode, add one segSize to cWnd. Default m_ssThresh is 65535. (RFC2001, sec.1)
      m_cWnd += m_segmentSize;
      NS_LOG_INFO ("In SlowStart, updated to cwnd " << m_cWnd << " ssthresh " << m_ssThresh);
//      std::cout<<"In SlowStart, updated to cwnd " << m_cWnd << " ssthresh " << m_ssThresh<<std::endl;
    }
  else
    {
        std::stringstream ss;
        ss<<m_endPoint->GetLocalAddress()<<":"<<m_endPoint->GetPeerAddress()<<":"<<m_endPoint->GetPeerPort();
        std::string flowkey = ss.str();
//        std::cout<<" adjusting window -- dctcp xfabric "<<m_xfabric<<" strawman "<<m_strawmancc<<std::endl; 
        // Congestion avoidance mode, increase by (segSize*segSize)/cwnd. (RFC2581, sec.3.1)
        // To increase cwnd for one segSize per RTT, it should be (ackBytes*segSize)/cwnd
        double adder = static_cast<double> (m_segmentSize * m_segmentSize) / m_cWnd.Get ();
        adder = std::max (1.0, adder);
      //  std::cout<<"adding to cwnd "<<adder<<" flow "<<flowkey<<std::endl;
        m_cWnd += static_cast<uint32_t> (adder);
    }
  } else if(m_dctcp && ecn_is_one) {
      //std::cout<<" ecn_is_one.. not modifying window node "<<m_node->GetId()<<std::endl;
  }
  TcpSocketBase::NewAck (seq);
}

uint32_t 
TcpNewReno::getFlowId(std::string flowkey)
{
  uint32_t fid = 0;
  Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol > (m_node->GetObject<Ipv4> ());
  if((ipv4->flowids).find(flowkey) != (ipv4->flowids).end()) {
    fid = ipv4->flowids[flowkey];
  }
   
//  std::map<std::string, uint32_t>::iterator it;
//  for (it=ipv4->flowids.begin(); it!=ipv4->flowids.end(); ++it) {
//    NS_LOG_LOGIC(it->first<<" "<<it->second);
//  }
 
//  NS_LOG_LOGIC(" flowkey "<<flowkey<<" fid "<<fid); 
  return fid;
}

double
TcpNewReno::getFlowIdealRate(std::string flowkey)
{
  uint32_t fid = 0;
  Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol > (m_node->GetObject<Ipv4> ());
  if((ipv4->flowids).find(flowkey) != (ipv4->flowids).end()) {
    fid = ipv4->flowids[flowkey];
  }
  double rate = ipv4->flow_idealrate[fid]; 
  return rate;
}

void
TcpNewReno::setdctcp(bool dctcp_value)
{
//  std::cout<<"unknown flow setting m_dctcp "<<dctcp_value<<" node "<<m_node->GetId()<<std::endl;
  m_dctcp = dctcp_value;
}
void
TcpNewReno::setxfabric(bool xfabric_value)
{
// std::cout<<"unknown flow setting m_xfabric "<<xfabric_value<<" node "<<m_node->GetId()<<std::endl;
 m_xfabric = xfabric_value;

}

uint32_t
TcpNewReno::getBytesAcked(const TcpHeader &tcpheader)
{
  //uint32_t header_corrections = 86;
  uint32_t header_corrections = 90;
  uint32_t bytes_acked = tcpheader.GetAckNumber() - m_txBuffer.HeadSequence();
//  std::cout<<"bytes_acked "<<bytes_acked<<std::endl;
  //NS_LOG_LOGIC(Simulator::Now().GetSeconds()<<" node "<<m_node->GetId()<<" bytes_acked "<<bytes_acked);
  return bytes_acked + header_corrections;
} 

bool
TcpNewReno::link_underutilized(const TcpHeader &tcpHeader)
{
  if(tcpHeader.GetECN() == 1) {
    return false;
  } 

  //NS_LOG_LOGIC("path has no bottlenecks "<<m_node->GetId()<<" "<<Simulator::Now().GetSeconds());
  return true;

}

double 
TcpNewReno::utilInverse(std::string ss, double cnp)
{
  Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol > (m_node->GetObject<Ipv4> ());
  return ipv4->utilInverse(ss, cnp);
} 

void
TcpNewReno::processRate(const TcpHeader &tcpHeader)
{

  std::stringstream ss;
  ss<<m_endPoint->GetLocalAddress()<<":"<<m_endPoint->GetPeerAddress()<<":"<<m_endPoint->GetPeerPort();
  std::string flowkey = ss.str();
  //std::cout<<" ack recvd for flow "<<flowkey<<" seq number "<<tcpHeader.GetAckNumber()<<" ack number "<<tcpHeader.GetSequenceNumber()<<std::endl;
  // if we are here, we have got an ACK - so we can say that the price is valid 
  double inter_arrival = tcpHeader.GetRate();
  uint32_t bytes_acked = getBytesAcked(tcpHeader);
  // get the ipv4 object 
  Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol > (m_node->GetObject<Ipv4> ());
  ipv4->setPriceValid(flowkey);

  ipv4->setNumHops(flowkey, tcpHeader.GetHopCount());
  uint32_t fid = ipv4->flowids[flowkey];

/*  if(fid == 8) {
  std::cout<<Simulator::Now().GetSeconds()<<" processRate flow "<<flowkey<<" node "<<m_node->GetId()<<" d0+dt "<<d0+m_dt<<" m_cWnd "<<m_cWnd<<" inter_arrival "<<inter_arrival<<" "<<Simulator::Now().GetNanoSeconds()<<" bytes_acked "<<bytes_acked<<" rtt "<<lastRtt_copy.GetNanoSeconds()<<" new cwnd "<<unquantized_window<<" using dt "<<m_dt<<std::endl; 
    } */

  if(m_strawmancc || m_dctcp) { // we want to update rates in case of both strawman and dctcp
//    std::cout<<"either true.. strawman"<<m_strawmancc<<" dctcp "<<m_dctcp<<" xfabric "<<m_xfabric<<" node "<<m_node->GetId()<<std::endl;
    ipv4->updateAverages(flowkey, inter_arrival, getBytesAcked(tcpHeader));
    return;
  }


  if(m_xfabric) {

//    std::cout<<Simulator::Now().GetSeconds()<<" m_xfabric "<<m_xfabric<<" strawman "<<m_strawmancc<<" node "<<m_node->GetId()<<" flow "<<flowkey<<std::endl;


    double window_spread_factor = 10.0;
    double dmin=0.00000;
    //double instant_rate = bytes_acked * 1.0 * 8.0 /(inter_arrival * 1.0e-9 * 1.0e+6);
    double target_cwnd = 0;
    uint32_t burst_size = 4;

    bool fixed_window = false;
    bool scheme2 = false;
    bool scheme3 = false;
    double ONENANO = 1000000000.0;

/*    double rtt_rate = m_cWnd * 8.0 * ONENANO /lastRtt_copy.GetNanoSeconds();
    std::cout<<"rtt_rate "<<Simulator::Now().GetSeconds()<<" "<<fid<<" "<<rtt_rate<<" "<<" "<<lastRtt_copy.GetNanoSeconds()<<" "<<m_cWnd*8.0<<std::endl;*/

    if(lastRtt_copy.GetNanoSeconds() / ONENANO < d0) {
//      std::cout<<"updating d0 from "<<d0<<" to "<<lastRtt_copy.GetNanoSeconds()/ONENANO <<" node "<<m_node->GetId()<<" at time "<<Simulator::Now().GetSeconds()<<std::endl;
      d0 = lastRtt_copy.GetNanoSeconds() / ONENANO;
    }

    if(fixed_window) 
    {
      //unquantized_window = 10*1000000000.0/8.0 * (dt+d0);
      unquantized_window = 10*1000000000.0/8.0 * (m_dt+0.000045);
      ipv4->updateAverages(flowkey, inter_arrival, getBytesAcked(tcpHeader));
    } 
    //else if(scheme2 || scheme3) 
    else 
    {
   /* 
      //ecn_highest = m_highTxMark;
      if(lastRtt_copy.GetNanoSeconds()  < ((d0+dmin)*1000000000.0)) {

        if(scheme2) {
          unquantized_window = 10*1000000000.0/8.0 * (m_dt+d0);
          NS_LOG_LOGIC("scheme2 is true ");
        } else {
          double line_rate = 10000.0;
          SequenceNumber32 ack_num = tcpHeader.GetAckNumber();
          if(ack_num >= one_rtt) {
            double trate = ipv4->flow_target_rate[flowkey];
            if(trate > line_rate) { trate = line_rate;}
            unquantized_window = trate * (m_dt+d0)*1000000.0/8.0 ;
            one_rtt = m_highTxMark + unquantized_window;
            std::cout<<"Time now is "<<Simulator::Now().GetNanoSeconds()<<" seconds "<<Simulator::Now().GetSeconds()<<" one_rtt "
            <<one_rtt<<" cwnd "<<unquantized_window<<" m_highTxMark "<<m_highTxMark<< 
            " available window "<<AvailableWindow()<<" node number "<<m_node->GetId()<<"flowkey "<<flowkey<<std::endl; 
          } // no else.. we don't do anything if it's too early
        }
        
      } else { // for usable RTT */

        if(scheme2 || scheme3) {

          // send the inter-arrival to update averages 
          ipv4->updateAverages(flowkey, inter_arrival, getBytesAcked(tcpHeader));
          double Rsmall = ipv4->GetShortTermRate(flowkey);
          // Now get the short term average for setting window 
          target_cwnd = Rsmall * (1000000.0/8.0) * (d0+m_dt); 

          double ste_size = (int((target_cwnd - unquantized_window)*bytes_acked) / int(window_spread_factor * unquantized_window));
          double temp_cwnd = unquantized_window + ste_size;
          double cur_possible_min = Rsmall * (1000000.0/8.0)* (d0+2.0*dmin);

          unquantized_window = std::max(temp_cwnd, cur_possible_min);
    
         /* NS_LOG_LOGIC("processRate instantaneous_rate "<<instant_rate<<" flow "<<flowkey<<" node "
          <<m_node->GetId()<<" d0+dt "<<d0+dt<<" m_cWnd "<<m_cWnd<<" inter_arrival "<<inter_arrival<<
          " "<<Simulator::Now().GetNanoSeconds()<<" bytes_acked "<<bytes_acked<<" rtt "<<
          lastRtt_copy.GetNanoSeconds()<<" target_cwnd "<<target_cwnd<<" cur_possible_min "<<cur_possible_min<<
          " temp_cwnd "<<temp_cwnd<<"  outhere difference "<<target_cwnd-m_cWnd<<" bytes_acked "<<bytes_acked); */
        }
        else 
        {
          Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol > (m_node->GetObject<Ipv4> ());
          ipv4->updateAverages(flowkey, inter_arrival, getBytesAcked(tcpHeader));
          /* Now get the short term average for setting window */
          double estimated_rate = ipv4->GetShortTermRate(flowkey);
          if(estimated_rate < 0.0) {
            estimated_rate = 0.0;
//            return;
          }
          unquantized_window = estimated_rate * (1000000.0/8.0) * (d0+m_dt);
        
      //    std::cout<<"processRate flow "<<flowkey<<" node "<<m_node->GetId()<<" d0+dt "<<d0+m_dt<<" m_cWnd "<<m_cWnd<<" inter_arrival "<<inter_arrival<<" "<<Simulator::Now().GetNanoSeconds()<<" bytes_acked "<<bytes_acked<<" rtt "<<lastRtt_copy.GetNanoSeconds()<<" new cwnd "<<unquantized_window<<" using dt "<<m_dt<<std::endl; 
      
          // our old xfabric scheme
        }
 //     } // usable RTT end
    } // fixed window else end

    // common stuff

    m_cWnd = (ceil) (unquantized_window/m_segmentSize) * m_segmentSize;
    if(m_cWnd < 1* m_segmentSize) 
    {
      m_cWnd = 1 * m_segmentSize;
    }
    m_ssThresh = m_cWnd;
  } else {
    // just a debug 
  }
}
  
 
void 
TcpNewReno::resetCW(double target_rate)
{

    if(!m_xfabric) return;

//    double line_rate = 10000.0; // HARD CODED
    if(target_rate > line_rate) {
	target_rate = line_rate;
    }
    double unquantized_window = target_rate * (1000000.0/8.0) * (d0+m_dt);
        
    m_cWnd = (ceil) (unquantized_window/m_segmentSize) * m_segmentSize;
    if(m_cWnd < 1* m_segmentSize) 
    {
      m_cWnd = 1 * m_segmentSize;
    }
//    std::cout<<"resetCW called with rate "<<target_rate<<" unquantized window "<<unquantized_window<<" window "<<m_cWnd<<" time "<<Simulator::Now().GetSeconds()<<" node "<<m_node->GetId()<<" line_rate "<<line_rate<<std::endl;
    m_ssThresh = m_cWnd;
}


void
TcpNewReno::ProcessECN(const TcpHeader &tcpHeader)
{

  if(m_xfabric || m_strawmancc) {
//    std::cout<<" not processing ecn.. m_xfabric "<<m_xfabric<<" strawman "<<m_strawmancc<<" node "<<m_node->GetId()<<std::endl;

    return;
  }
  
  NS_LOG_FUNCTION (this << tcpHeader);
  SequenceNumber32 ack_num = tcpHeader.GetAckNumber();

  std::stringstream ss;
  ss<<m_endPoint->GetLocalAddress()<<":"<<m_endPoint->GetPeerAddress()<<":"<<m_endPoint->GetPeerPort();
  std::string flowkey = ss.str();

  /* update the last ack recvd variable */
  int32_t num_bytes_acked = ack_num.GetValue() - highest_ack_recvd.GetValue();
  NS_LOG_INFO("DCTCP_DEBUG "<<highest_ack_recvd.GetValue()<<" ack_num "<<ack_num.GetValue());
  NS_LOG_INFO(Simulator::Now().GetSeconds()<<" DCTCP_DEBUG num_bytes_acked "<<num_bytes_acked<<" highest_ack_recd "<<highest_ack_recvd<<" total_bytes_acked "<<total_bytes_acked);
  highest_ack_recvd = ack_num;
  total_bytes_acked += num_bytes_acked;
  double new_alpha = 0.0;

  if(tcpHeader.GetECN() == 1) {
    bytes_with_ecn += num_bytes_acked;
    ecn_is_one = true;
  } else {
    ecn_is_one = false;
  }
  //if(total_bytes_acked > (int)m_cWnd) {
  if(ack_num >= last_outstanding_num) {
     if(total_bytes_acked > 0.0) {
       new_alpha = double((bytes_with_ecn * 1.0) / (total_bytes_acked * 1.0));
       dctcp_alpha = (1.0 - beta)*dctcp_alpha + beta* new_alpha;
       total_bytes_acked = 0;
       bytes_with_ecn = 0; 
       //std::cout<<Simulator::Now().GetSeconds()<<" node "<<m_node->GetId()<<" DCTCP_DEBUG new_alpha "<<new_alpha<<" DCTCP_ALPHA "<<dctcp_alpha<<" "<<flowkey<<" fid "<<getFlowId(flowkey)<<" ECN "<<tcpHeader.GetECN()<<" ssthresh "<<m_ssThresh<<" initcwnd "<<m_initialCWnd<<" lastoutstanding "<<last_outstanding_num<<" ack_num "<<ack_num<<" hightxmark "<<m_highTxMark<<std::endl;
       last_outstanding_num = m_highTxMark;
     } 
   }
  
  
  if(tcpHeader.GetECN() == 1) {
    /* If the mark is on a packet which is acking a packet lesser than highest tx number,
     * don't react */
 //   NS_LOG_INFO("ProcessECN .. ACK recvd for seq "<<ack_num<<" ecn_highest set to "<<ecn_highest<< "current highest "<<m_highTxMark); 

    if(m_dctcp) {
   //   NS_LOG_LOGIC("m_dctcp is true");
      //NS_LOG_UNCOND("bytes_with_ecn "<<bytes_with_ecn<<" totalbytes "<<total_bytes_acked);
      if(ack_num >= ecn_highest) {

          //NS_LOG_UNCOND("m_smoother_dctcp is false");
          m_cWnd = (m_cWnd) * (1.0 - dctcp_alpha/2.0);

          if(m_cWnd < 1* m_segmentSize) 
          {
            m_cWnd = 1 * m_segmentSize;
          }
          m_ssThresh = m_cWnd;
          dctcp_reacted = true;
          /* note that we won't react for another RTT */
          ecn_highest = m_highTxMark;
          //bytes_with_ecn = 0.0;
          //total_bytes_acked = 0.0;
          //std::cout<<Simulator::Now().GetMicroSeconds()<<" m_dctcp -- processing ECN ack_num "<<ack_num<<" ecn_highest now "<<ecn_highest<<" "<<flowkey<<std::endl;
        
      } else {
          //std::cout<<Simulator::Now().GetMicroSeconds()<<" m_dctcp -- not processing ECN ack_num "<<ack_num<<" ecn_highest "<<ecn_highest<<" "<<flowkey<<std::endl;
//      NS_LOG_INFO("Notreacting "<<Simulator::Now().GetSeconds());
        // no reaction 
      }

    } else if(!m_xfabric && !m_dctcp) { //m_dctcp and m_xfabric are false - so, this is regular tcp reacting to ECN
      //std::cout<<" should not happen "<<std::endl;
      
      /*
      if(ack_num < ecn_highest) {
        NS_LOG_INFO("ecn mark recvd. but, it is not yet time to react");
        return;
      } 
      */
  
      /* On receipt of ECN marked ACK, cut the congestion window by half */
      //  m_ssThresh = std::max (2 * m_segmentSize, BytesInFlight () / 2);
      //  m_cWnd = m_ssThresh + 3 * m_segmentSize;


      m_cWnd = (m_cWnd *19)/20;
      if(m_cWnd < 1* m_segmentSize) 
      {
        m_cWnd = 1 * m_segmentSize;
      }
      m_ssThresh = m_cWnd;
      ecn_highest = m_highTxMark;
    }
  } 
    

}
  
  

/* Cut cwnd and enter fast recovery mode upon triple dupack */
void
TcpNewReno::DupAck (const TcpHeader& t, uint32_t count)
{
  NS_LOG_FUNCTION (this << count);
  if (count == m_retxThresh && !m_inFastRec)
    { // triple duplicate ack triggers fast retransmit (RFC2582 sec.3 bullet #1)
        m_ssThresh = std::max (2 * m_segmentSize, BytesInFlight () / 2);
        m_cWnd = m_ssThresh + 3 * m_segmentSize;
        m_recover = m_highTxMark;
        m_inFastRec = true;
        NS_LOG_INFO ("Triple dupack. Enter fast recovery mode. Reset cwnd to " << m_cWnd <<
                   ", ssthresh to " << m_ssThresh << " at fast recovery seqnum " << m_recover);
      DoRetransmit ();
    } 
    else if (m_inFastRec) 
    { // Increase cwnd for every additional dupack (RFC2582, sec.3 bullet #3)
      m_cWnd += m_segmentSize;
      NS_LOG_INFO ("Dupack in fast recovery mode. Increase cwnd to " << m_cWnd);
      SendPendingData (m_connected);
    }
  else if (!m_inFastRec && m_limitedTx && m_txBuffer.SizeFromSequence (m_nextTxSequence) > 0)
    { // RFC3042 Limited transmit: Send a new packet for each duplicated ACK before fast retransmit
      NS_LOG_INFO ("Limited transmit");
      uint32_t sz = SendDataPacket (m_nextTxSequence, m_segmentSize, true);
      m_nextTxSequence += sz;                    // Advance next tx sequence
    };
}

/* Retransmit timeout */
void
TcpNewReno::Retransmit (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ReTxTimeout Expired at time " << Simulator::Now ().GetSeconds ());

/*
  if(ecn_backoff_) {
     We don't need to enter 
 */
  m_inFastRec = false;

  // If erroneous timeout in closed/timed-wait state, just return
  if (m_state == CLOSED || m_state == TIME_WAIT) return;
  // If all data are received (non-closing socket and nothing to send), just return
  if (m_state <= ESTABLISHED && m_txBuffer.HeadSequence () >= m_highTxMark) return;

  // According to RFC2581 sec.3.1, upon RTO, ssthresh is set to half of flight
  // size and cwnd is set to 1*MSS, then the lost packet is retransmitted and
  // TCP back to slow start
  m_ssThresh = std::max (2 * m_segmentSize, BytesInFlight () / 2);
  m_cWnd = m_segmentSize;
  m_nextTxSequence = m_txBuffer.HeadSequence (); // Restart from highest Ack
  NS_LOG_INFO ("RTO. Reset cwnd to " << m_cWnd <<
               ", ssthresh to " << m_ssThresh << ", restart from seqnum " << m_nextTxSequence);
  m_rtt->IncreaseMultiplier ();             // Double the next RTO
  DoRetransmit ();                          // Retransmit the packet
}

void
TcpNewReno::SetSegSize (uint32_t size)
{
  NS_ABORT_MSG_UNLESS (m_state == CLOSED, "TcpNewReno::SetSegSize() cannot change segment size after connection started.");
  m_segmentSize = size;
}

void
TcpNewReno::SetInitialSSThresh (uint32_t threshold)
{
  NS_ABORT_MSG_UNLESS (m_state == CLOSED, "TcpNewReno::SetSSThresh() cannot change initial ssThresh after connection started.");
  m_initialSsThresh = threshold;
}

uint32_t
TcpNewReno::GetInitialSSThresh (void) const
{
  return m_initialSsThresh;
}

void
TcpNewReno::SetInitialCwnd (uint32_t cwnd)
{
  NS_ABORT_MSG_UNLESS (m_state == CLOSED, "TcpNewReno::SetInitialCwnd() cannot change initial cwnd after connection started.");
  m_initialCWnd = cwnd;
}

uint32_t
TcpNewReno::getBDP(void)
{
  return GetInitialCwnd() * m_segmentSize;
}

uint32_t
TcpNewReno::GetInitialCwnd (void) const
{
  return m_initialCWnd;
}

void 
TcpNewReno::InitializeCwnd (void)
{
  /*
   * Initialize congestion window, default to 1 MSS (RFC2001, sec.1) and must
   * not be larger than 2 MSS (RFC2581, sec.3.1). Both m_initiaCWnd and
   * m_segmentSize are set by the attribute system in ns3::TcpSocket.
   */
  m_cWnd = m_initialCWnd * m_segmentSize;
  m_ssThresh = m_initialSsThresh;

  unquantized_window = m_cWnd * 1.0;

  NS_LOG_LOGIC(" InitializeCwnd : "<<m_initialCWnd<<" "<<m_cWnd<<" "<<m_ssThresh);
}

void
TcpNewReno::resetSSThresh(uint32_t ssthresh)
{
  m_initialSsThresh = ssthresh;
  m_ssThresh = m_initialSsThresh;
  NS_LOG_LOGIC(" ssthresh reset to "<<m_ssThresh);
}

void
TcpNewReno::resetInitCwnd(uint32_t cwnd)
{
  NS_ABORT_MSG_UNLESS (m_state == CLOSED, "TcpNewReno::SetInitialCwnd() cannot change initial cwnd after connection started.");
  m_initialCWnd = cwnd;
  InitializeCwnd();
}
  

} // namespace ns3
