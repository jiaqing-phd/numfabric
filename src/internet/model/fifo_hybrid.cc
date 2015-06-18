#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/node.h"
#include "ns3/uinteger.h"
#include "fifo_hybrid.h"
#include "ns3/ppp-header.h"
#include "ns3/prio-header.h"
#include "ns3/simulator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/double.h"
#include <limits>

NS_LOG_COMPONENT_DEFINE ("fifo_hybridQ");

#define FIFO_1 1
#define FIFO_2 0

namespace ns3 {
typedef std::map<uint32_t, std::queue<Ptr <Packet> > >::iterator pktq_iter; //!< the packets in the queue

NS_OBJECT_ENSURE_REGISTERED (fifo_hybridQ);

TypeId fifo_hybridQ::GetTypeId (void) 
{
  static TypeId tid = TypeId ("ns3::fifo_hybridQ")
    .SetParent<Queue> ()
    .AddConstructor<fifo_hybridQ> ()
    .AddAttribute ("Mode", 
                   "Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&fifo_hybridQ::SetMode),
                   MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets accepted by this fifo_hybridQ.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&fifo_hybridQ::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes", 
                   "The maximum number of bytes accepted by this fifo_hybridQ.",
                   UintegerValue (100 * 1500),
                   MakeUintegerAccessor (&fifo_hybridQ::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("DataRate", 
                   "The default data rate for point to point links",
                   DataRateValue (DataRate ("32768b/s")),
                   MakeDataRateAccessor (&fifo_hybridQ::m_bps),
                   MakeDataRateChecker ())

;
  return tid;
}

fifo_hybridQ::fifo_hybridQ ()
{
  NS_LOG_FUNCTION (this);
  nodeid = 1000;
  turn = FIFO_2;
}

fifo_hybridQ::~fifo_hybridQ ()
{
//  NS_LOG_FUNCTION (this);
}

void fifo_hybridQ::SetNodeID(uint32_t node_id)
{
  nodeid = node_id;
}

void fifo_hybridQ::SetLinkID(uint32_t link_id)
{
  linkid = link_id;
}

void fifo_hybridQ::SetLinkIDString(std::string linkid_string1)
{
  linkid_string = linkid_string1;
  std::cout<<"setting node id "<<nodeid<<" linkid_string "<<linkid_string<<std::endl;
}

std::string fifo_hybridQ::GetLinkIDString(void)
{
  return linkid_string;
}

void
fifo_hybridQ::SetMode (fifo_hybridQ::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}


uint32_t fifo_hybridQ::GetFifo_1_Size()
{
  //return m_fifo_1_pkts.size();
  return m_fifo_1_bytesInQueue;

}


uint32_t fifo_hybridQ::GetFifo_2_Size()
{
  //return m_fifo_2_pkts.size();
  return m_fifo_2_bytesInQueue;

}


uint32_t fifo_hybridQ::GetCurCount(uint32_t fid)
{

  return m_size[fid];
}


/* uint32_t fifo_hybridQ::GetCurSize(uint32_t fid_dummy)
{
  uint32_t tot_bytes = 0;
  for(std::map<uint32_t, uint32_t>::iterator it = m_bytesInQueue.begin(); it!=m_bytesInQueue.end(); ++it) {
    uint32_t fid = it->first;
    std::cout<<"QOCCU "<<linkid_string<<" "<<Simulator::Now().GetSeconds()<<" "<<fid<<" "<<m_bytesInQueue[fid]<<std::endl;
    tot_bytes += m_bytesInQueue[fid];
  }
  return tot_bytes;
}
*/

uint32_t fifo_hybridQ::GetMaxBytes(void)
{
  return m_maxBytes;
}

fifo_hybridQ::QueueMode
fifo_hybridQ::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}


bool
fifo_hybridQ::removeFifo_1(Ptr<Packet> p)
{
  m_fifo_1_pkts.pop();
  m_fifo_1_size -= 1;
  m_fifo_1_bytesInQueue -= p->GetSize();

  return true;
} 


bool
fifo_hybridQ::removeFifo_2(Ptr<Packet> p)
{
  m_fifo_2_pkts.pop();
  m_fifo_2_size -= 1;
  m_fifo_2_bytesInQueue -= p->GetSize();

  return true;
} 

  
/*** private functions ****/ 
TcpHeader
fifo_hybridQ::GetTCPHeader(Ptr<Packet> p)
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
fifo_hybridQ::GetIPHeader(Ptr<Packet> p)
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


PrioHeader 
fifo_hybridQ::GetPrioHeader(Ptr<Packet> p)
{
  PppHeader ppp;
  PrioHeader pheader;
  p->RemoveHeader(ppp);
  p->PeekHeader(pheader); 
  p->AddHeader(ppp);

  return pheader;
}

std::string 
fifo_hybridQ::GetFlowKey(Ptr<Packet> p)
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
fifo_hybridQ::getFlowID(Ptr<Packet> p)
{
  std::string flowkey = GetFlowKey(p);
  if (flow_ids.find(flowkey) != flow_ids.end()) {
    std::cout<<"flowkey "<<flowkey<<" fid "<<flow_ids[flowkey]<<std::endl;
    return flow_ids[flowkey];
  }
  return 0; //TBD - convert flows to ids
}

void
fifo_hybridQ::setFlowID(std::string flowkey, uint32_t fid, double fweight)
{
  std::cout<<"SetFlowID Queue "<<linkid_string<<" flowkey "<<flowkey<<" fid "<<fid<<std::endl;
  flow_ids[flowkey] = fid;
  flow_weights[fid] = fweight;

}

bool
fifo_hybridQ::known_flow(uint32_t flowid)
{
  /* This is hacky and hardcoded and wrong. Correct this */
 
  /* 
  std::vector<uint32_t> v(8);
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  v.push_back(4);
  v.push_back(5);
  v.push_back(6);
  v.push_back(7);
  v.push_back(8);
  */

   
  std::vector<uint32_t> v(4);
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  v.push_back(4);
  

  /* 
  std::vector<uint32_t> v(2);
  v.push_back(1);
  v.push_back(2);
  */  

  for(uint32_t idx=0; idx < v.size(); idx++) {
    if(v[idx] == flowid) {

      std::cout<<"SC known flow "<< flowid <<std::endl;

      return true;
    }
  } 

  return false;
}
 
bool
fifo_hybridQ::DoEnqueue(Ptr<Packet> p)
{
 uint32_t flowid = getFlowID(p);
 if(known_flow(flowid)) {
  std::cout<<"SC Enqueued to fifo_2, flow id "<< flowid << std::endl;
  return fifo_2_EnQ(p);  
 } else {
  std::cout<<"SC Enqueued to fifo_1, flow id "<< flowid << std::endl;
  return fifo_1_EnQ(p);
 }
}

bool
fifo_hybridQ::fifo_1_EnQ(Ptr<Packet> p)
{
  m_fifo_1_pkts.push(p);
  m_fifo_1_size += 1;
  m_fifo_1_bytesInQueue += p->GetSize(); // TBD : headers?
  return true;
}


bool
fifo_hybridQ::fifo_2_EnQ(Ptr<Packet> p)
{
  m_fifo_2_pkts.push(p);
  m_fifo_2_size += 1;
  m_fifo_2_bytesInQueue += p->GetSize(); // TBD : headers?
  return true;
}



Ptr<Packet>
fifo_hybridQ::fifo_1_Dequeue(void)
{
  if(m_fifo_1_pkts.empty()) {
    // q empty
    return 0;
  }
  Ptr<Packet> p = m_fifo_1_pkts.front();
  removeFifo_1(p);
  return p;
}


Ptr<Packet>
fifo_hybridQ::fifo_2_Dequeue(void)
{
  if(m_fifo_2_pkts.empty()) {
    // q empty
    return 0;
  }
  Ptr<Packet> p = m_fifo_2_pkts.front();
  removeFifo_2(p);
  return p;
}


bool
fifo_hybridQ::fifo_1_qempty(void) 
{
  if(m_fifo_1_pkts.size() > 0) {
    return false;
  }
  return true;

}


bool
fifo_hybridQ::fifo_2_qempty(void) 
{
  if(m_fifo_2_pkts.size() > 0) {
    return false;
  }
  return true;

}



Ptr<Packet>
fifo_hybridQ::DoDequeue(void)
{
  Ptr<Packet> p;
  if((turn == FIFO_2 && !fifo_2_qempty()) || fifo_1_qempty()) {
    turn = FIFO_1;
    if(linkid_string == "0_0_1") {
      std::cout<<"SC FIFO 2 deq"<<std::endl;
    }
    return fifo_2_Dequeue();
  } 

  if((turn == FIFO_1 && !fifo_1_qempty()) || fifo_2_qempty()){
    turn = FIFO_2;
    if(linkid_string == "0_0_1") {
      std::cout<<"SC FIFO 1 deq"<<std::endl;
    }
    return fifo_1_Dequeue();
  }
  
  return 0;
}


Ptr<const Packet>
fifo_hybridQ::DoPeek (void) const
{
  return 0;

}

} // namespace ns3


