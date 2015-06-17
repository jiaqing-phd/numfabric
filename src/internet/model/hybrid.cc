
#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/node.h"
#include "ns3/uinteger.h"
#include "hybrid.h"
#include "ns3/ppp-header.h"
#include "ns3/prio-header.h"
#include "ns3/simulator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/double.h"
#include <limits>

NS_LOG_COMPONENT_DEFINE ("hybridQ");

//static uint32_t MAXUINT = std::numeric_limits<uint32_t>::max();
//static double MAXDOUBLE = std::numeric_limits<double>::max();
static double MAXDOUBLE=1.79e+100;
//static double PKTSIZE=1500*8.0;
//
#define W2FQ 1
#define FIFO 0

namespace ns3 {
typedef std::map<uint32_t, std::queue<Ptr <Packet> > >::iterator pktq_iter; //!< the packets in the queue

NS_OBJECT_ENSURE_REGISTERED (hybridQ);

TypeId hybridQ::GetTypeId (void) 
{
  static TypeId tid = TypeId ("ns3::hybridQ")
    .SetParent<Queue> ()
    .AddConstructor<hybridQ> ()
    .AddAttribute ("Mode", 
                   "Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&hybridQ::SetMode),
                   MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets accepted by this hybridQ.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&hybridQ::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes", 
                   "The maximum number of bytes accepted by this hybridQ.",
                   UintegerValue (100 * 1500),
                   MakeUintegerAccessor (&hybridQ::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("DataRate", 
                   "The default data rate for point to point links",
                   DataRateValue (DataRate ("32768b/s")),
                   MakeDataRateAccessor (&hybridQ::m_bps),
                   MakeDataRateChecker ())

    .AddAttribute ("vpackets", 
                   "The number of packets after which to calculate slope",
                   UintegerValue (1),
                   MakeUintegerAccessor (&hybridQ::vpackets),
                   MakeUintegerChecker<uint32_t> ())

;
  return tid;
}

hybridQ::hybridQ ()
{
  NS_LOG_FUNCTION (this);
  nodeid = 1000;
  current_virtualtime = 0.0;
  init_reset = false;
  last_virtualtime_time = 0.0;
  last_virtualtime = 0.0;
  virtualtime_updated = 0;
  turn = W2FQ;
}

hybridQ::~hybridQ ()
{
//  NS_LOG_FUNCTION (this);
}

void hybridQ::SetNodeID(uint32_t node_id)
{
  nodeid = node_id;
}

void hybridQ::SetLinkID(uint32_t link_id)
{
  linkid = link_id;
}

void hybridQ::SetLinkIDString(std::string linkid_string1)
{
  linkid_string = linkid_string1;
  std::cout<<"setting node id "<<nodeid<<" linkid_string "<<linkid_string<<std::endl;
}

std::string hybridQ::GetLinkIDString(void)
{
  return linkid_string;
}

void
hybridQ::SetMode (hybridQ::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}


uint32_t hybridQ::GetFifoSize()
{
  return m_fifopkts.size();
}

uint32_t hybridQ::GetCurCount(uint32_t fid)
{

  return m_size[fid];
}


uint32_t hybridQ::GetCurSize(uint32_t fid_dummy)
{
  uint32_t tot_bytes = 0;
  for(std::map<uint32_t, uint32_t>::iterator it = m_bytesInQueue.begin(); it!=m_bytesInQueue.end(); ++it) {
    uint32_t fid = it->first;
    std::cout<<"QOCCU "<<linkid_string<<" "<<Simulator::Now().GetSeconds()<<" "<<fid<<" "<<m_bytesInQueue[fid]<<std::endl;
    tot_bytes += m_bytesInQueue[fid];
  }
  return tot_bytes;
}

uint32_t hybridQ::GetMaxBytes(void)
{
  return m_maxBytes;
}

hybridQ::QueueMode
hybridQ::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

/***** Functions that will implement a queue using a std::list *****/
bool hybridQ::enqueue(Ptr<Packet> p, uint32_t flowid)
{
  /* We assume all checks are done by the time
     the packet is here 
   */

  if(linkid_string == "0_0_1") {
    std::cout<<"E queuenumber "<<linkid_string<<" "<<Simulator::Now().GetNanoSeconds()<<" flow "<<flowid<<" pkts "<<m_size[flowid]<<std::endl;
  }
  (m_packets[flowid]).push(p);
  m_size[flowid] += 1;
  m_bytesInQueue[flowid] += p->GetSize(); // TBD : headers?
  return true;
}

bool 
hybridQ::removeW2FQ(Ptr<Packet> p, int32_t flowid)
{
  NS_LOG_FUNCTION(this << p);
  m_packets[flowid].pop();
  m_size[flowid]-= 1;
  m_bytesInQueue[flowid] -= p->GetSize();
  if(linkid_string == "0_0_1") {
    std::cout<<"D queuenumber "<<linkid_string<<" "<<Simulator::Now().GetNanoSeconds()<<" flow "<<flowid<<" pkts "<<m_size[flowid]<<std::endl;
  }
  return true;
}

bool
hybridQ::removeFifo(Ptr<Packet> p)
{
  m_fifopkts.pop();
  m_fifosize -= 1;
  m_fifobytesInQueue -= p->GetSize();

  return true;
} 
  
/*** private functions ****/ 
TcpHeader
hybridQ::GetTCPHeader(Ptr<Packet> p)
{
  TcpHeader tcph;
  Ipv4Header h1;
  PppHeader ppp;
  PrioHeader pheader;
  //strip off all headers
  p->RemoveHeader(ppp);
  p->RemoveHeader(pheader); 
  p->RemoveHeader(h1);

  p->PeekHeader(tcph);
  // add all headers
  p->AddHeader(h1);
  p->AddHeader(pheader);
  p->AddHeader(ppp);
  return tcph;
}
   

Ipv4Header 
hybridQ::GetIPHeader(Ptr<Packet> p)
{
  Ipv4Header h1;
  PppHeader ppp;
  PrioHeader pheader;
  p->RemoveHeader(ppp);
  p->RemoveHeader(pheader); 
  p->PeekHeader(h1);
  p->AddHeader(pheader);
  p->AddHeader(ppp);

  return h1;
}

double
hybridQ::getWFQweight(Ptr<Packet> p)
{
  PrioHeader pheader = GetPrioHeader(p);
  double wfq_weight = (pheader.GetData()).wfq_weight;
  return wfq_weight;
}

PrioHeader 
hybridQ::GetPrioHeader(Ptr<Packet> p)
{
  PppHeader ppp;
  PrioHeader pheader;
  p->RemoveHeader(ppp);
  p->PeekHeader(pheader); 
  p->AddHeader(ppp);

  return pheader;
}

std::string 
hybridQ::GetFlowKey(Ptr<Packet> p)
{
  Ipv4Header h = GetIPHeader(p);
  TcpHeader tcph = GetTCPHeader(p);
  Ipv4Address source = h.GetSource();
  Ipv4Address destination = h.GetDestination();
  uint16_t destPort = tcph.GetDestinationPort();
  std::stringstream ss;
  ss <<source<<":"<<destination<<":"<<destPort;
  std::string flowkey = ss.str();
  
  return flowkey;
}


uint32_t
hybridQ::get_virtualtime()
{
  return current_virtualtime;
}

uint32_t
hybridQ::getFlowID(Ptr<Packet> p)
{
  std::string flowkey = GetFlowKey(p);
  if (flow_ids.find(flowkey) != flow_ids.end()) {
//    std::cout<<"flowkey "<<flowkey<<" fid "<<flow_ids[flowkey]<<std::endl;
    return flow_ids[flowkey];
  }
  return 0; //TBD - convert flows to ids
}

void
hybridQ::setFlowID(std::string flowkey, uint32_t fid, double fweight)
{
  std::cout<<"SetFlowID Queue "<<linkid_string<<" flowkey "<<flowkey<<" fid "<<fid<<std::endl;
  flow_ids[flowkey] = fid;
  flow_weights[fid] = fweight;

  /*if(m_packets[fid].size() > 0) {
    Ptr<Packet> p = (m_packets[fid]).front();
    if(p) {
      re_resetFlows(fid, p);
    }
  } */
}


void
hybridQ::re_resetFlows(uint32_t flowid, Ptr<Packet> p)
{
     //double fw = flow_weights[flowid];
     //if(fw < 1.0) {
     // fw = 1000.0;
     //}
     double fw = getWFQweight(p);
     double pkt_wfq_weight = p->GetSize()*8.0/fw;
     local_flow_weights[flowid] = fw;
    
    start_time[flowid] = current_virtualtime;
    finish_time[flowid] = start_time[flowid] + pkt_wfq_weight;
 
 
    double min_starttime = start_time[flowid]; 
    /* update virtual time */
    pktq_iter it;
    for (it = m_packets.begin(); it != m_packets.end(); ++it) {
      uint32_t fid = it->first;
      if(linkid_string == "0_0_1") {
        std::cout<<"start_time flowid "<<fid<<" "<<start_time[fid]<<" "<<Simulator::Now().GetSeconds()<<std::endl;
        std::cout<<"finish_time flowid "<<fid<<" "<<finish_time[fid]<<" "<<Simulator::Now().GetSeconds()<<std::endl;
      }
  
      if(((m_packets[fid].size()) > 0) && (start_time[fid] < min_starttime)) {
        min_starttime = start_time[fid];
      }
    }


    current_virtualtime = std::max(min_starttime, current_virtualtime);
//      if(linkid_string == "0_0_1") 
//    std::cout<<"re_resetFlows: flowid "<<flowid<<" start_time "<<start_time[flowid]<<" pkt_wfq_weight "<<pkt_wfq_weight<<" finish_time "<<finish_time[flowid]<<" vtime "<<current_virtualtime<<" min_starttime "<<min_starttime<<" VTIMEUPDATE "<<linkid_string<<std::endl;
  return;
}
void
hybridQ::resetFlows(uint32_t flowid, Ptr<Packet> p)
{
   /*  double fw = flow_weights[flowid];
     if(fw < 1.0) {
      fw = 1000.0;
     }
    */
    double fw = getWFQweight(p);
    double pkt_wfq_weight = p->GetSize()*8.0/fw;
     //double pkt_wfq_weight = PKTSIZE/fw;
     local_flow_weights[flowid] = fw;
     
//      if(linkid_string == "0_0_1") 
//     std::cout<<"before: flowid "<<flowid<<" start_time "<<start_time[flowid]<<" pkt_wfq_weight "<<pkt_wfq_weight<<" finish_time "<<finish_time[flowid]<<" vtime "<<current_virtualtime<<" "<<linkid_string<<" "<<Simulator::Now().GetSeconds()<<std::endl;
     start_time[flowid] = std::max(current_virtualtime, finish_time[flowid]);
     finish_time[flowid] = start_time[flowid] + pkt_wfq_weight;
//      if(linkid_string == "0_0_1") 
//     std::cout<<"after:flowid "<<flowid<<" start_time "<<start_time[flowid]<<" pkt_wfq_weight "<<pkt_wfq_weight<<" finish_time "<<finish_time[flowid]<<" vtime "<<current_virtualtime<<" "<<linkid_string<<std::endl;
 
    //uint32_t min_starttime = MAXUINT; 
    double min_starttime = start_time[flowid]; 
    /* update virtual time */
    pktq_iter it;
    for (it = m_packets.begin(); it != m_packets.end(); ++it) {
      uint32_t fid = it->first;
      if(linkid_string == "0_0_1") {
        std::cout<<"start_time flowid "<<fid<<" "<<start_time[fid]<<" "<<Simulator::Now().GetSeconds()<<std::endl;
        std::cout<<"finish_time flowid "<<fid<<" "<<finish_time[fid]<<" "<<Simulator::Now().GetSeconds()<<std::endl;
      }
  
      if(((m_packets[fid].size()) > 0) && (start_time[fid] < min_starttime)) {
        min_starttime = start_time[fid];
      }
    }

    current_virtualtime = std::max(min_starttime, current_virtualtime);
   /*   if(linkid_string == "0_0_1") {
      std::cout.precision(15);
      std::cout<<"VIRTUALTIMETRACK ENQUEUE "<<Simulator::Now().GetNanoSeconds()<<" "<<std::fixed<<old_virtualtime<<" "<<std::fixed<<current_virtualtime<<std::endl;
      } 
//      if(linkid_string == "0_0_1") 
    std::cout<<"resetFlows: flowid "<<flowid<<" start_time "<<start_time[flowid]<<" pkt_wfq_weight "<<pkt_wfq_weight<<" finish_time "<<finish_time[flowid]<<" vtime "<<current_virtualtime<<" min_starttime "<<min_starttime<<" VTIMEUPDATE "<<linkid_string<<" fw "<<fw<<std::endl; */
  return;
}

bool
hybridQ::known_flow(uint32_t flowid)
{
  /* This is hacky and hardcoded and wrong. Correct this */
  
  std::vector<uint32_t> v(8);
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  v.push_back(4);
  v.push_back(5);
  v.push_back(6);
  v.push_back(7);
  v.push_back(8);

  for(uint32_t idx=0; idx < v.size(); idx++) {
    if(v[idx] == flowid) {
      return true;
    }
  } 

  return false;
}
 
bool
hybridQ::DoEnqueue(Ptr<Packet> p)
{
 uint32_t flowid = getFlowID(p);
 if(known_flow(flowid)) {
  std::cout<<"Enqueued to W2FQ"<<std::endl;
  return W2FQEnQ(p);  
 } else {
  std::cout<<"Enqueued to Fifo"<<std::endl;
  return FifoEnQ(p);
 }
}

bool
hybridQ::FifoEnQ(Ptr<Packet> p)
{
  m_fifopkts.push(p);
  m_fifosize += 1;
  m_fifobytesInQueue += p->GetSize(); // TBD : headers?
  return true;
}

Ptr<Packet>
hybridQ::FifoDequeue(void)
{
  if(m_fifopkts.empty()) {
    // q empty
    return 0;
  }
  Ptr<Packet> p = m_fifopkts.front();
  removeFifo(p);
  return p;
}
  
bool 
hybridQ::W2FQEnQ (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  uint32_t flowid = getFlowID(p);

  /* First check if the queue size exceeded */
  if ((m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue[flowid] >= m_maxBytes)) ||
        (m_mode == QUEUE_MODE_PACKETS && m_packets[flowid].size() >= m_maxPackets))
    {
            NS_LOG_UNCOND ("Queue full (packet would exceed max bytes) -- dropping pkt");
            Drop (p);
            return false;
    }
 
   if(m_packets[flowid].size() <= 0) { 
    resetFlows(flowid, p);
   }

   if((local_flow_weights.find(flowid) != local_flow_weights.end()) && (getWFQweight(p) != local_flow_weights[flowid])) {
      re_resetFlows(flowid, p);
  }

  pkt_arrival[p->GetUid()] = Simulator::Now().GetNanoSeconds();
  enqueue(p, flowid); 
 // if(linkid_string == "0_0_1") {
 //   std::cout<<"enqueued packet with flowid "<<flowid<<" current_virtualtime "<<current_virtualtime<<" pakcte starttime "<<start_time[flowid]<<" finish time "<<finish_time[flowid]<<" "<<linkid_string<<" pkt size "<<p->GetSize()<<std::endl;
  //} 
  return true;
  
}

bool
hybridQ::W2FQempty(void) 
{
  std::map<uint32_t, std::queue < Ptr <Packet> > >::iterator it;
  it = m_packets.begin();
  while(it != m_packets.end()) {
    uint32_t fid = it->first;
    if(m_packets[fid].size() > 0) {
      return false;
    }
    ++it;
  }
  return true;
}

bool
hybridQ::fifoqempty(void) 
{
  if(m_fifopkts.size() > 0) {
    return false;
  }
  return true;

}


Ptr<Packet>
hybridQ::DoDequeue(void)
{
  Ptr<Packet> p;
  if((turn == W2FQ && !W2FQempty()) || fifoqempty()) {
    turn = FIFO;
    if(linkid_string == "0_0_1") {
      std::cout<<"W2fq deq"<<std::endl;
    }
    return W2FQDequeue();
  } 

  if((turn == FIFO && !fifoqempty()) || W2FQempty()){
    turn = W2FQ;
    if(linkid_string == "0_0_1") {
      std::cout<<"fifo deq"<<std::endl;
    }
    return FifoDequeue();
  }
  
  return 0;
}
   
    

Ptr<Packet>
hybridQ::W2FQDequeue (void)
{
  NS_LOG_FUNCTION (this);

  if (W2FQempty())
  {
    //  std::cout<<"Queue empty"<<std::endl;
      return 0;
  }

  //uint32_t min_finishtime = MAXUINT;
  double min_finishtime = MAXDOUBLE;
  pktq_iter it;
  int32_t flow = -1;
  for (it=m_packets.begin(); it != m_packets.end(); ++it)
  {
    uint32_t fid = it->first;
     //if(linkid_string == "0_0_1") {
//      std::cout<<"DoDequeue flow "<<fid<<" has "<<m_packets[fid].size()<<" packets: st "<<start_time[fid]<<" ft "<<finish_time[fid]<<" minfinish "<<min_finishtime<<" virtual time "<<current_virtualtime<<" "<<linkid_string<<std::endl;
     // }
    if((m_packets[fid].size() > 0) && (start_time[fid] <= current_virtualtime)) {
      if(finish_time[fid] < min_finishtime) {
        flow = fid;
        min_finishtime = finish_time[fid];
      }
    }
  }

  if(flow == -1 || min_finishtime == MAXDOUBLE) {
    return 0;
  }

  Ptr<Packet> pkt = m_packets[flow].front();
  removeW2FQ(pkt, flow);

  /* TBD : any other updates ? */
  
  /* Set the start and finish times of the remaining packets in the queue */
  //double pkt_wfq_weight = getWFQweight(pkt);
  if(m_packets[flow].size() > 0) {
     double pktSize = (m_packets[flow].front())->GetSize() * 8.0;

/*     double fw = flow_weights[flow];
     if(fw < 1.0) {
      fw = 1000.0;
     }
*/
    double fw = getWFQweight(pkt);
    double pkt_wfq_weight = pktSize/fw;
    start_time[flow] = finish_time[flow];
    finish_time[flow] = start_time[flow] + pkt_wfq_weight; 
//    std::cout<<"updated starttime finishtime "<<start_time[flow]<<" "<<finish_time[flow]<<" "<<linkid_string<<" weight "<<pkt_wfq_weight<<" flow "<<flow<<" "<<pktSize<<" "<<fw<<std::endl;
  }

  /* update the virtual clock */
  double minS = start_time[flow];
  bool minSreset = false;
  double W = 0.00000001; 
/*  std::map<uint32_t, double>::iterator it1;
  for(it1=local_flow_weights.begin(); it1 != local_flow_weights.end(); ++it1)
  {
    uint32_t fid = it1->first;
    double fweight = it1->second;
    W += fweight;
    if(m_packets[fid].size() > 0 && !minSreset) {
      minSreset = true;
      minS = start_time[fid];
    }
    if(m_packets[fid].size() > 0 && start_time[fid] < minS) {
      minS = start_time[fid];
    }
  }
 */ 
  for (it=m_packets.begin(); it != m_packets.end(); ++it)
  {
    uint32_t fid = it->first;
    double fweight = local_flow_weights[fid];
/*    if(fweight < 1.0) {
      fweight = 1000.0;
    }
*/
    W += fweight;
    if(m_packets[fid].size() > 0 && !minSreset) {
      minSreset = true;
      minS = start_time[fid];
    }
    if(m_packets[fid].size() > 0 && start_time[fid] < minS) {
      minS = start_time[fid];
    }
  } 
/*  if(minSreset == false) {
    minS = 0.0;
    std::cout<<"minSRESET is false "<<linkid_string<<std::endl;
  }
*/  
  //if(linkid_string == "0_0_1") {
//    std::cout<<"determined minstartime "<<minS<<" second term "<< (1.0*current_virtualtime + (double)(pkt->GetSize()*8.0/W))<<" "<<linkid_string<<" W = "<<W<<std::endl;
  //}
  current_virtualtime = std::max(minS*1.0, (1.0*current_virtualtime + (double)(pkt->GetSize()*8.0/W)));
    /*  if(linkid_string == "0_0_1") {
      std::cout.precision(15);
      std::cout<<"VIRTUALTIMETRACK DEQUEUE "<<Simulator::Now().GetNanoSeconds()<<" "<<std::fixed<<old_virtualtime<<" "<<std::fixed<<current_virtualtime<<std::endl;
      }*/
//  current_virtualtime = std::max(minS*1.0, (1.0*current_virtualtime + PKTSIZE/W));
  double wait_duration = Simulator::Now().GetNanoSeconds() - pkt_arrival[pkt->GetUid()];
//  if(linkid_string == "0_0_1") {
    std::cout<<"QWAIT "<<linkid_string<<" "<<Simulator::Now().GetSeconds()<<" "<<flow<<" spent "<<wait_duration<<" in queue "<<linkid_string<<std::endl;
    std::cout<<"DEQUEUE "<<linkid_string<<" "<<Simulator::Now().GetMicroSeconds()<<" "<<flow<<" pkt size "<<pkt->GetSize()<<std::endl;
    
    virtualtime_updated += 1;
    if(virtualtime_updated >= vpackets) {
     virtualtime_updated = 0;
     current_slope = CalcSlope();
     std::cout<<"SLOPEINFO "<<linkid_string<<" "<<Simulator::Now().GetMicroSeconds()<<" "<<current_slope<<std::endl;
    } 
  
//    std::cout<<"End of dequeue: flow "<<flow<<" stime "<<start_time[flow]<<" ftime "<<finish_time[flow]<<" vtime "<<current_virtualtime<<" min_starttime "<<minS<<" "<<linkid_string<<" VTIMEUPDATE"<<std::endl;
//  }
  return (pkt);
}

void
hybridQ::SetVPkts(uint32_t vpkts)
{
  std::cout<<"setting vpackets to "<<vpkts<<std::endl;
  vpackets = vpkts;
}

double 
hybridQ::CalcSlope(void)
{
  double time_elapsed = Simulator::Now().GetNanoSeconds() - last_virtualtime_time;
  double slope = (current_virtualtime - last_virtualtime) * 1000.0 / time_elapsed;
  last_virtualtime_time = Simulator::Now().GetNanoSeconds();
  last_virtualtime = current_virtualtime;
  return slope;
} 
   



Ptr<const Packet>
hybridQ::DoPeek (void) const
{

  //if (QueueEmpty())
  //  {
  //    NS_LOG_LOGIC ("Queue empty");
  //    return 0;
  //  }
  return 0;

//  Ptr<Packet> p = (m_packets[1]).front ();

 // return p;
}

} // namespace ns3


