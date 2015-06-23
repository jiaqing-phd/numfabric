#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/node.h"
#include "ns3/uinteger.h"
#include "prio-queue.h"
#include "ns3/ppp-header.h"
#include "ns3/prio-header.h"
#include "ns3/simulator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/double.h"
#include "ns3/fifoqueue.h"

NS_LOG_COMPONENT_DEFINE ("FifoQueue");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (FifoQueue);

TypeId FifoQueue::GetTypeId (void) 
{
  static TypeId tid = TypeId ("ns3::FifoQueue")
    .SetParent<Queue> ()
    .AddConstructor<FifoQueue> ()
    .AddAttribute ("Mode", 
                   "Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&FifoQueue::SetMode),
                   MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets accepted by this FifoQueue.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&FifoQueue::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes", 
                   "The maximum number of bytes accepted by this FifoQueue.",
                   UintegerValue (100 * 1500),
                   MakeUintegerAccessor (&FifoQueue::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
 
    .AddAttribute ("ECNThreshBytes", 
                   "The maximum number of packets accepted by this FifoQueue before it starts marking",
                   UintegerValue (100),
                   MakeUintegerAccessor (&FifoQueue::m_ECNThreshBytes),
                   MakeUintegerChecker<uint32_t> ())

;
  return tid;
}

FifoQueue::FifoQueue () :
  Queue (),
  m_packets (),
  m_bytesInQueue (0)
{
  NS_LOG_FUNCTION (this);
  nodeid = 1000;
  total_deq = 0;
}

uint32_t
FifoQueue::getFlowID(Ptr<Packet> p)
{
  std::string flowkey = GetFlowKey(p);
  if (flow_ids.find(flowkey) != flow_ids.end()) {
//    std::cout<<"flowkey "<<flowkey<<" fid "<<flow_ids[flowkey]<<std::endl;
    return flow_ids[flowkey];
  }
  return 0;
}

void
FifoQueue::setFlowID(std::string flowkey, uint32_t fid, double fweight)
{
 // std::cout<<"SetFlowID Queue "<<linkid_string<<" flowkey "<<flowkey<<" fid "<<fid<<std::endl;
  flow_ids[flowkey] = fid;
  flow_weights[fid] = fweight;

}

FifoQueue::~FifoQueue ()
{
//  NS_LOG_FUNCTION (this);
}

void FifoQueue::SetNodeID(uint32_t node_id)
{
  nodeid = node_id;
}

void FifoQueue::SetLinkID(uint32_t link_id)
{
  linkid = link_id;
}

void FifoQueue::SetLinkIDString(std::string linkid_string1)
{
  linkid_string = linkid_string1;
  std::cout<<"setting node id "<<nodeid<<" linkid_string "<<linkid_string<<std::endl;
}

std::string FifoQueue::GetLinkIDString(void)
{
  return linkid_string;
}

void
FifoQueue::SetMode (FifoQueue::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

uint32_t
FifoQueue::GetMaxPackets(void)
{
  return m_maxPackets;
}

uint32_t
FifoQueue::GetCurCount(void)
{

  return m_size;
}

uint32_t
FifoQueue::GetCurSize(void)
{

  /*
   typedef std::list<Ptr<Packet> >::iterator PacketQueueI;
   std::map<std::string,uint32_t> flow_count;
   uint32_t total_pkts = 0;
 
 
   //NS_LOG_UNCOND("Current queue .. "); 
   for (PacketQueueI pp = m_packets.begin (); pp != m_packets.end (); pp++)
   {
       Ipv4Header h;
       PrioHeader pheader;
       PppHeader ppp;

       (*(pp))->RemoveHeader(ppp);
       (*(pp))->RemoveHeader(pheader);
       (*(pp))->PeekHeader(h);
       (*(pp))->AddHeader(pheader);
       (*(pp))->AddHeader(ppp);

       Ipv4Address src = h.GetSource();
       Ipv4Address dst = h.GetDestination();

       std::stringstream ss;
       ss <<src<<":"<<dst;
       std::string flowkey = ss.str();

       flow_count[flowkey] += 1;
   } 
  
   std::map<std::string,uint32_t>::iterator it;
   for (std::map<std::string,uint32_t>::iterator it=flow_count.begin(); it!=flow_count.end(); ++it)
   {
       NS_LOG_UNCOND("QOCCU "<<Simulator::Now().GetSeconds()<<" flow "<<it->first<<" pktcount "<<it->second<<" nodeid "<<nodeid);
     total_pkts += it->second;
   }
  
   NS_LOG_UNCOND(Simulator::Now().GetSeconds()<<" totalpktscounter "<<total_pkts<<" nodeid "<<nodeid);  
   */

  return m_bytesInQueue;
}

uint32_t
FifoQueue::GetMaxBytes(void)
{
  return m_maxBytes;
}

FifoQueue::QueueMode
FifoQueue::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

/***** Functions that will implement a queue using a std::list *****/
bool FifoQueue::enqueue(Ptr<Packet> p)
{
  /* We assume all checks are done by the time
     the packet is here 
   */
  NS_LOG_FUNCTION (this << p);
  m_packets.push_back(p);
  m_size++;
  m_bytesInQueue += p->GetSize();

  return true;
}

bool 
FifoQueue::remove(Ptr<Packet> p)
{
  NS_LOG_FUNCTION(this << p);
  typedef std::list<Ptr<Packet> >::iterator PacketQueueI;
  PacketQueueI it = m_packets.begin();
  
  for(; it != m_packets.end(); it++) 
  {
    if(*it == p)
    {
      m_packets.erase(it);
      m_size--;
      m_bytesInQueue -= p->GetSize();
      return true;
    }
  }
  return false;

}
  
/*** private functions ****/ 
TcpHeader
FifoQueue::GetTCPHeader(Ptr<Packet> p)
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
FifoQueue::GetIPHeader(Ptr<Packet> p)
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
FifoQueue::GetPrioHeader(Ptr<Packet> p)
{
  PppHeader ppp;
  PrioHeader pheader;
  p->RemoveHeader(ppp);
  p->PeekHeader(pheader); 
  p->AddHeader(ppp);

  return pheader;
}

std::string 
FifoQueue::GetFlowKey(Ptr<Packet> p)
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

bool 
FifoQueue::DoEnqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);


  incoming_bytes += p->GetSize(); 

  uint32_t pkt_uid = p->GetUid();

  /* Also store time of arrival for each packet */
  pkt_arrival[pkt_uid] = Simulator::Now().GetNanoSeconds();

  enqueue(p);

  /* First check if the queue size exceeded */
  if ((m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue >= m_maxBytes)) ||
        (m_mode == QUEUE_MODE_PACKETS && m_packets.size() >= m_maxPackets))
    {
            NS_LOG_UNCOND ("Queue full (packet would exceed max bytes) -- dropping pkt");
            remove(p);
            Drop (p);
            return false;
    } /* if queue is going to be full */

  else if ((m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue >= m_ECNThreshBytes)) ||
        (m_mode == QUEUE_MODE_PACKETS && m_packets.size() >= m_ECNThreshPackets))
    {
        // Now add ECN bit to the IP header of the this packet 
          
        Ipv4Header ipheader;
        PrioHeader pheader;
        PppHeader ppp;
        p->RemoveHeader(ppp);
        p->RemoveHeader(pheader);
        p->RemoveHeader(ipheader);

        ipheader.SetEcn(Ipv4Header::ECN_CE);
        
        p->AddHeader(ipheader);
        p->AddHeader(pheader);
        p->AddHeader(ppp);

//        std::cout<<"marking ECN "<<linkid_string<<"  qsize "<<m_bytesInQueue<<" ecnthresh "<<m_ECNThreshBytes<<std::endl;

    } 

  return true;
}


Ptr<Packet>
FifoQueue::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  if(m_packets.empty())
  {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
  }

  Ptr<Packet> p = m_packets.front ();
  total_deq += p->GetSize();
  //std::cout<<"Dequeued packet of size "<<p->GetSize()<<" total_deq "<<total_deq<<std::endl;
  remove(p);
  return p;
}


Ptr<const Packet>
FifoQueue::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);

  if (m_packets.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<Packet> p = m_packets.front ();

  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return p;
}

} // namespace ns3


