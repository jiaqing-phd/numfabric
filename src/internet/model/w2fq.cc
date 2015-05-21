
#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/node.h"
#include "ns3/uinteger.h"
#include "w2fq.h"
#include "ns3/ppp-header.h"
#include "ns3/prio-header.h"
#include "ns3/simulator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/double.h"
#include <limits>

NS_LOG_COMPONENT_DEFINE ("W2FQ");

static uint32_t MAXUINT = std::numeric_limits<uint32_t>::max();
//static double MAXDOUBLE = std::numeric_limits<double>::max();
static double MAXDOUBLE=1.79e+100;

namespace ns3 {
typedef std::map<uint32_t, std::queue<Ptr <Packet> > >::iterator pktq_iter; //!< the packets in the queue

NS_OBJECT_ENSURE_REGISTERED (W2FQ);

TypeId W2FQ::GetTypeId (void) 
{
  static TypeId tid = TypeId ("ns3::W2FQ")
    .SetParent<Queue> ()
    .AddConstructor<W2FQ> ()
    .AddAttribute ("Mode", 
                   "Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&W2FQ::SetMode),
                   MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets accepted by this W2FQ.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&W2FQ::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes", 
                   "The maximum number of bytes accepted by this W2FQ.",
                   UintegerValue (100 * 1500),
                   MakeUintegerAccessor (&W2FQ::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("DataRate", 
                   "The default data rate for point to point links",
                   DataRateValue (DataRate ("32768b/s")),
                   MakeDataRateAccessor (&W2FQ::m_bps),
                   MakeDataRateChecker ())


;
  return tid;
}

W2FQ::W2FQ ()
{
  NS_LOG_FUNCTION (this);
  nodeid = 1000;
  current_virtualtime = 0.0;
}

W2FQ::~W2FQ ()
{
//  NS_LOG_FUNCTION (this);
}

void W2FQ::SetNodeID(uint32_t node_id)
{
  nodeid = node_id;
}

void W2FQ::SetLinkID(uint32_t link_id)
{
  linkid = link_id;
}

void W2FQ::SetLinkIDString(std::string linkid_string1)
{
  linkid_string = linkid_string1;
  std::cout<<"setting node id "<<nodeid<<" linkid_string "<<linkid_string<<std::endl;
}

std::string W2FQ::GetLinkIDString(void)
{
  return linkid_string;
}

void
W2FQ::SetMode (W2FQ::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}


uint32_t W2FQ::GetCurCount(uint32_t fid)
{

  return m_size[fid];
}


uint32_t W2FQ::GetCurSize(uint32_t fid)
{
  uint32_t tot_bytes = 0;
  for(std::map<uint32_t, uint32_t>::iterator it = m_bytesInQueue.begin(); it!=m_bytesInQueue.end(); ++it) {
    std::cout<<" GetCurSize : fid "<<fid<<" size "<<m_bytesInQueue[fid]<<std::endl;
    tot_bytes += m_bytesInQueue[fid];
  }
  return tot_bytes;
}

uint32_t W2FQ::GetMaxBytes(void)
{
  return m_maxBytes;
}

W2FQ::QueueMode
W2FQ::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

/***** Functions that will implement a queue using a std::list *****/
bool W2FQ::enqueue(Ptr<Packet> p, uint32_t flowid)
{
  /* We assume all checks are done by the time
     the packet is here 
   */

  (m_packets[flowid]).push(p);
  m_size[flowid] = (m_packets[flowid]).size();
  m_bytesInQueue[flowid] += p->GetSize(); // TBD : headers?

  return true;
}

bool 
W2FQ::remove(Ptr<Packet> p, int32_t flowid)
{
  NS_LOG_FUNCTION(this << p);
  m_packets[flowid].pop();
  m_size[flowid] = (m_packets[flowid]).size();
  m_bytesInQueue[flowid] -= p->GetSize();
  return true;
}
  
/*** private functions ****/ 
TcpHeader
W2FQ::GetTCPHeader(Ptr<Packet> p)
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
W2FQ::GetIPHeader(Ptr<Packet> p)
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
W2FQ::getWFQweight(Ptr<Packet> p)
{
  PrioHeader pheader = GetPrioHeader(p);
  double wfq_weight = (pheader.GetData()).wfq_weight;
  return wfq_weight;
}

PrioHeader 
W2FQ::GetPrioHeader(Ptr<Packet> p)
{
  PppHeader ppp;
  PrioHeader pheader;
  p->RemoveHeader(ppp);
  p->PeekHeader(pheader); 
  p->AddHeader(ppp);

  return pheader;
}

std::string 
W2FQ::GetFlowKey(Ptr<Packet> p)
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
W2FQ::get_virtualtime()
{
  return current_virtualtime;
}

uint32_t
W2FQ::getFlowID(Ptr<Packet> p)
{
  std::string flowkey = GetFlowKey(p);
  if (flow_ids.find(flowkey) != flow_ids.end()) {
//    std::cout<<"flowkey "<<flowkey<<" fid "<<flow_ids[flowkey]<<std::endl;
    return flow_ids[flowkey];
  }
  return 0; //TBD - convert flows to ids
}

void
W2FQ::setFlowID(std::string flowkey, uint32_t fid)
{
 // std::cout<<"SetFlowID Queue "<<linkid_string<<" flowkey "<<flowkey<<" fid "<<fid<<std::endl;
  flow_ids[flowkey] = fid;
  start_time[fid] = 0;
  finish_time[fid] = 0;
  current_virtualtime = 0.0;
}

bool 
W2FQ::DoEnqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  uint32_t flowid = getFlowID(p);

  /* First check if the queue size exceeded */
  if ((m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue[flowid] >= m_maxBytes)) ||
        (m_mode == QUEUE_MODE_PACKETS && m_packets[flowid].size() >= m_maxPackets))
    {
            NS_LOG_UNCOND ("Queue full (packet would exceed max bytes) -- dropping pkt");
            remove(p, flowid);
            Drop (p);
            return false;
    }
 
   if(m_packets[flowid].size() <= 0) { 
     double pkt_wfq_weight = getWFQweight(p);
//      if(linkid_string == "0_0_1") 
//     std::cout<<"before: flowid "<<flowid<<" start_time "<<start_time[flowid]<<" pkt_wfq_weight "<<pkt_wfq_weight<<" finish_time "<<finish_time[flowid]<<" vtime "<<current_virtualtime<<" "<<linkid_string<<std::endl;
     start_time[flowid] = std::max(current_virtualtime, finish_time[flowid]);
     finish_time[flowid] = start_time[flowid] + pkt_wfq_weight;
//      if(linkid_string == "0_0_1") 
 //    std::cout<<"after:flowid "<<flowid<<" start_time "<<start_time[flowid]<<" pkt_wfq_weight "<<pkt_wfq_weight<<" finish_time "<<finish_time[flowid]<<" vtime "<<current_virtualtime<<" "<<linkid_string<<std::endl;
 
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
//      if(linkid_string == "0_0_1") 
 //    std::cout<<"final: flowid "<<flowid<<" start_time "<<start_time[flowid]<<" pkt_wfq_weight "<<pkt_wfq_weight<<" finish_time "<<finish_time[flowid]<<" vtime "<<current_virtualtime<<std::endl;
  }

  pkt_arrival[p->GetUid()] = Simulator::Now().GetNanoSeconds();
  enqueue(p, flowid); 
 // if(linkid_string == "0_0_1") {
 //   std::cout<<"enqueued packet with flowid "<<flowid<<" current_virtualtime "<<current_virtualtime<<" pakcte starttime "<<start_time[flowid]<<" finish time "<<finish_time[flowid]<<" "<<linkid_string<<std::endl;
  //} 
  return true;
  
}

bool
W2FQ::QueueEmpty(void) 
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

Ptr<Packet>
W2FQ::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  if (QueueEmpty())
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
   //   std::cout<<"DoDequeue flow "<<fid<<" has "<<m_packets[fid].size()<<" packets: st "<<start_time[fid]<<" ft "<<finish_time[fid]<<" minfinish "<<min_finishtime<<" virtual time "<<current_virtualtime<<" "<<linkid_string<<std::endl;
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
  remove(pkt, flow);

  /* TBD : any other updates ? */
  
  /* Set the start and finish times of the remaining packets in the queue */
  double pkt_wfq_weight = getWFQweight(pkt);
  uint32_t pktSize = pkt->GetSize();
  if(m_packets[flow].size() > 0) {
    start_time[flow] = finish_time[flow];
    finish_time[flow] = start_time[flow] + pkt_wfq_weight; 
//    std::cout<<"updated starttime finishtime "<<start_time[flow]<<" "<<finish_time[flow]<<" "<<linkid_string<<std::endl;
  }

  /* update the virtual clock */
  //uint32_t minS = start_time[flow];
  double minS = start_time[flow];
  bool minSreset = false;
  //double W = 0.00000001; 
  for (it=m_packets.begin(); it != m_packets.end(); ++it)
  {
    uint32_t fid = it->first;
    //W += fid;
    if(m_packets[fid].size() > 0 && !minSreset) {
      minSreset = true;
      minS = start_time[fid];
    }
    if(m_packets[fid].size() > 0 && start_time[fid] < minS) {
      minS = start_time[fid];
    }
  }
  //if(linkid_string == "0_0_1") {
 //   std::cout<<"determined minstartime "<<minS<<" "<<MAXDOUBLE<<" "<<abs(minS-MAXDOUBLE)<<" "<<linkid_string<<std::endl;
 //   std::cout<<"pktsize "<<pktSize <<" bitrate "<<m_bps.GetBitRate()<<" "<<linkid_string<<std::endl;
  //}
  //current_virtualtime = std::max(minS*1.0, (1.0*current_virtualtime + (double)(pktSize/W)));
  current_virtualtime = std::max(minS*1.0, (1.0*current_virtualtime + (double)(pktSize * 8.0 * 1000000)/m_bps.GetBitRate()));
  double wait_duration = Simulator::Now().GetNanoSeconds() - pkt_arrival[pkt->GetUid()];
  if(linkid_string == "0_0_1") {
    std::cout<<"QWAIT "<<Simulator::Now().GetSeconds()<<" "<<flow<<" spent "<<wait_duration<<" in queue "<<linkid_string<<std::endl;
//    std::cout<<"End of dequeue: flow "<<flow<<" stime "<<start_time[flow]<<" ftime "<<finish_time[flow]<<" vtime "<<current_virtualtime<<" "<<linkid_string<<std::endl;
  }
  return (pkt);
}


Ptr<const Packet>
W2FQ::DoPeek (void) const
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


