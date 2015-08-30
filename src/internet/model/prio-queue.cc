
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


NS_LOG_COMPONENT_DEFINE ("PrioQueue");

double eps_diff = 0.0;

static double MAX_DOUBLE = 10000.0;
static double ONEPACKET = 1200.0;

namespace ns3 {


 TypeId 
 MyTag::GetTypeId (void)
 {
   static TypeId tid = TypeId ("ns3::MyTag")
     .SetParent<Tag> ()
     .AddConstructor<MyTag> ()
     .AddAttribute ("SimpleValue",
                    "A simple value",
                    EmptyAttributeValue (),
                    MakeDoubleAccessor (&MyTag::GetValue),
                    MakeDoubleChecker<double> ())
   ;
   return tid;
 }
 TypeId 
 MyTag::GetInstanceTypeId (void) const
 {
   return GetTypeId ();
 }
 uint32_t 
 MyTag::GetSerializedSize (void) const
 {
   return 2*sizeof(double);
 }
 void 
 MyTag::Serialize (TagBuffer i) const
 {
     uint8_t a[sizeof(double)];
     memcpy((void *)a, (void *)&m_Value, sizeof(double));
  
     i.Write(a, sizeof(double));

     uint8_t b[sizeof(double)];
     memcpy((void *)b, (void *)&m_ArrTime, sizeof(double));
  
     i.Write(b, sizeof(double));
 }
 void 
 MyTag::Deserialize (TagBuffer i)
 {
  uint8_t a[sizeof(double)];
  i.Read(a, sizeof(double)); 
  memcpy((void *)&m_Value, (void *)a, sizeof(double));

  uint8_t b[sizeof(double)];
  i.Read(b, sizeof(double)); 
  memcpy((void *)&m_ArrTime, (void *)b, sizeof(double));
 }

 void 
 MyTag::Print (std::ostream &os) const
 {
   os << "v=" << (double)m_Value;
 }
 void 
 MyTag::SetValue (double value, double arr_time)
 {
   m_Value = value;
  m_ArrTime = arr_time;
 }

 double 
 MyTag::GetArrival (void) const
 {
    return m_ArrTime;
 }

 double 
 MyTag::GetValue (void) const
 {
   return m_Value;
 }

NS_OBJECT_ENSURE_REGISTERED (PrioQueue);

TypeId PrioQueue::GetTypeId (void) 
{
  static TypeId tid = TypeId ("ns3::PrioQueue")
    .SetParent<Queue> ()
    .AddConstructor<PrioQueue> ()
    .AddAttribute("m_pfabricdequeue",
                  "Enable or disable only pfabric like behavior",
                  BooleanValue(false),
                  MakeBooleanAccessor (&PrioQueue::m_pfabricdequeue),
                  MakeBooleanChecker ())
    .AddAttribute("m_pkt_tag",
                  "Enable or disable wfq like behavior",
                  BooleanValue(true),
                  MakeBooleanAccessor (&PrioQueue::m_pkt_tagged),
                  MakeBooleanChecker ())
    .AddAttribute("delay_mark",
                  "ECN mark based on delay experienced by packet",
                  BooleanValue(true),
                  MakeBooleanAccessor (&PrioQueue::delay_mark),
                  MakeBooleanChecker ())
    .AddAttribute ("Mode", 
                   "Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&PrioQueue::SetMode),
                   MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets accepted by this PrioQueue.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&PrioQueue::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes", 
                   "The maximum number of bytes accepted by this PrioQueue.",
                   UintegerValue (100 * 1500),
                   MakeUintegerAccessor (&PrioQueue::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
 
    /* Switch to enable different behavior of switches */ 
    .AddAttribute("pFabric",
                  "Enable or disable priority dropping",
                   UintegerValue(1),
                   MakeUintegerAccessor (&PrioQueue:: m_pFabric),
                   MakeUintegerChecker<uint32_t> ())
                  
    .AddAttribute ("ECNThreshBytes", 
                   "The maximum number of packets accepted by this PrioQueue before it starts marking",
                   UintegerValue (100),
                   MakeUintegerAccessor (&PrioQueue::m_ECNThreshBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("DataRate", 
                   "The default data rate for point to point links",
                   DataRateValue (DataRate ("32768b/s")),
                   MakeDataRateAccessor (&PrioQueue::m_bps),
                   MakeDataRateChecker ())

    .AddAttribute ("vpackets", 
                   "The number of packets after which to calculate slope",
                   UintegerValue (1),
                   MakeUintegerAccessor (&PrioQueue::vpackets),
                   MakeUintegerChecker<uint32_t> ())

    .AddAttribute ("xfabric_price", 
                   "Calculate network price according to xfabric",
                   BooleanValue (true),
                   MakeBooleanAccessor (&PrioQueue::xfabric_price),
                   MakeBooleanChecker ())
    .AddAttribute ("host_compensate", 
                   "host compensates for the price",
                   BooleanValue (false),
                   MakeBooleanAccessor (&PrioQueue::host_compensate),
                   MakeBooleanChecker ())
	 .AddAttribute("target_queue",
                  "Target Queue",
                  DoubleValue(10000.0),
                  MakeDoubleAccessor (&PrioQueue::m_target_queue),
                  MakeDoubleChecker <double> ())


    .AddAttribute ("gamma", 
                   "gamma value for DGD",
                   DoubleValue(0.000001),
                   MakeDoubleAccessor (&PrioQueue::m_gamma),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("gamma1", 
                   "Value of gamma1 for DGD",
                   DoubleValue(10.0),
                   MakeDoubleAccessor (&PrioQueue::m_gamma1),
                   MakeDoubleChecker <double>())
    .AddAttribute ("alpha", 
                   "gamma value for xfabric",
                   DoubleValue(0.01),
                   MakeDoubleAccessor (&PrioQueue::m_alpha),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("PriceUpdateTime", 
                   "The time after which to update priority",
                   TimeValue(Seconds(0.0001)),
                   MakeTimeAccessor (&PrioQueue::m_updatePriceTime),
                   MakeTimeChecker())
;
  return tid;
}

PrioQueue::PrioQueue () :
  Queue (),
  m_packets (),
  m_bytesInQueue (0),
  m_pFabric(1)
{
  NS_LOG_FUNCTION (this);
  nodeid = 1000;
  eps = 0.00001;
  running_min_prio = latest_min_prio = MAX_DOUBLE;
  running_avg_prio = latest_avg_prio = 0.0;
  total_samples = 0;
  current_virtualtime = 0.0;
  updated_virtual_time = 0.0;
  current_price = 0.0;

  xfabric_price = true;
  Simulator::Schedule(Seconds(1.0), &ns3::PrioQueue::updateLinkPrice, this);
  
  NS_LOG_LOGIC(" link data rate "<<m_bps<<" ecn_delaythreshold "<<ecn_delaythreshold);
}

uint32_t
PrioQueue::getFlowID(Ptr<Packet> p)
{
  std::string flowkey = GetFlowKey(p);
  if (flow_ids.find(flowkey) != flow_ids.end()) {
//    NS_LOG_LOGIC("flowkey "<<flowkey<<" fid "<<flow_ids[flowkey]);
    return flow_ids[flowkey];
  }
  return 0; //TBD - convert flows to ids
}

void
PrioQueue::setFlowID(std::string flowkey, uint32_t fid, double fweight, uint32_t known)
{
 // NS_LOG_LOGIC("SetFlowID Queue "<<linkid_string<<" flowkey "<<flowkey<<" fid "<<fid);
  flow_ids[flowkey] = fid;
  flow_weights[fid] = fweight;
  flow_known[fid] = known;

}


double
PrioQueue::getRateDifference(Time time_interval)
{
    double link_incoming_rate = (8.0 * incoming_bytes) /(1000000.0 * time_interval.GetSeconds());

    double available_capacity = (m_bps.GetBitRate() / 1000000.0); // units -Megabits per Second
    double rate_difference = link_incoming_rate - available_capacity;

    if(nodeid == 0 || nodeid == 1) {
//      NS_LOG_LOGIC(Simulator::Now().GetSeconds()<<" queue_id "<<nodeid<<" Incoming bytes "<<incoming_bytes<<" link_incoming_rate "<<link_incoming_rate<<" available capacity "<<available_capacity<<" rate_difference "<<rate_difference);
    }
    //current_incoming_rate = link_incoming_rate;
    incoming_bytes = 0.0;
    return rate_difference;
}

double
PrioQueue::getRateDifferenceNormalized(Time time_interval)
{
    double link_incoming_rate = (8.0 * incoming_bytes) /(1000000.0 * time_interval.GetSeconds());
    double available_capacity = (m_bps.GetBitRate() / 1000000.0); // units -Megabits per Second

    double ratio = 1.0 - link_incoming_rate / available_capacity;
    //double ratio1 = std::max(0.0, ratio);
 
    //current_incoming_rate = link_incoming_rate;
    //std::cout<<" queuerate "<<linkid_string<<" "<<Simulator::Now().GetSeconds()<<" current_incoming_rate "<<link_incoming_rate<<" bytes "<<incoming_bytes<<" available_capacity "<<available_capacity<<" ratio "<<ratio<<std::endl;
    incoming_bytes = 0.0;
    return ratio;
}

double
PrioQueue::getCurrentPrice(void)
{
  return current_price;
}
  

void
PrioQueue::updateLinkPrice(void)
{

  if(!xfabric_price) 
  {

    //NS_LOG_UNCOND("Using updateLinkPrice");

    double current_queue = GetCurSize();
    
    double price_hike = m_gamma * getRateDifference(m_updatePriceTime) + m_alpha * (current_queue - m_target_queue);
    current_price = current_price + price_hike;
    // cap it to positive value
    current_price = std::max(current_price, 0.0);
    current_price = std::min(current_price, 1.0);

//    if(m_is_switch) {
      //NS_LOG_UNCOND(Simulator::Now().GetSeconds()<< " current price "<<current_price<<" node "<<nodeid<<" price_raise "<<price_hike<<" queue_term "<< (m_alpha *(current_queue - m_target_queue))<<" rate_term "<<price_hike<<" current_queue "<<current_queue<<" target_queue "<<m_target_queue);
 //   } 
    NS_LOG_LOGIC(Simulator::Now().GetSeconds()<<" NOXFABRIC Queue_id "<<linkid_string<<" price "<<current_price<<" min_price_inc "<<price_hike);
  } else {
    if(running_min_prio != MAX_DOUBLE) {
      latest_min_prio = running_min_prio;
    } else {
      latest_min_prio = 0.0;
    }

    if(total_samples > 0) {
      latest_avg_prio = running_avg_prio/(total_samples*1.0);
    } else {
      latest_avg_prio = 0.0;
    }
    //reset values
    running_avg_prio = 0.0;
    total_samples = 0;
    running_min_prio = MAX_DOUBLE;

    NS_LOG_LOGIC("running_min_prio_log nodeid "<<nodeid<<" "<<latest_min_prio);
    
    //if(m_is_switch) {
     //NS_LOG_UNCOND(Simulator::Now().GetSeconds()<<" queueu_id "<<nodeid<<" copied running_min_prio "<<latest_min_prio<<" as latest");
    //}
  
  
    double old_price = current_price;
    double min_price_inc = old_price + latest_min_prio;
    double rate_increase = getRateDifferenceNormalized(m_updatePriceTime);

    double incr = std::max(rate_increase, 0.0); // we don't want this term to increase price
  
    //double new_price = min_price_inc - m_gamma1*incr*current_price;
    double new_price = min_price_inc - m_gamma1*incr; // TODO : try different gamma
    
   if(new_price < 0.0) {
    new_price = 0.0;
   }
   //current_price = new_price;
   //
   if(!host_compensate) {
     current_price = 0.5*new_price + 0.5*current_price;
   } else {
     current_price = new_price;
   }
  
    NS_LOG_LOGIC(Simulator::Now().GetSeconds()<<" XFABRIC nodeid "<<nodeid<<" price "<<current_price<<" min_price_inc "<<min_price_inc<<" new_price "<<new_price<<" rate_increase "<<rate_increase);
   std::cout<<Simulator::Now().GetSeconds()<<" Queue_id "<<GetLinkIDString()<<" old_price "<<old_price<<" min_price_inc "<<min_price_inc<<" rate_increase "<<incr<<" new_price "<<new_price<<" current_price "<<current_price<<" current_virtualtime "<<current_virtualtime<<" m_gamma1 "<<m_gamma1<<" latest_min_prio "<<latest_min_prio<<" hcompensate "<<host_compensate<<std::endl;
   // when you update the price - set a timer to not update the minimum for an interval
   update_minimum = false;
   std::cout<<Simulator::Now().GetSeconds()<<" updateminimum Queue_id "<<GetLinkIDString()<<" disabling update minimum "<<std::endl;
   Simulator::Schedule(Seconds(0.0005), &ns3::PrioQueue::enableUpdates, this); // 10ms
  }
  Simulator::Schedule(m_updatePriceTime, &ns3::PrioQueue::updateLinkPrice, this);
 
}

void
PrioQueue::enableUpdates(void)
{
  std::cout<<Simulator::Now().GetSeconds()<<" updateminimum Queue_id "<<GetLinkIDString()<<" enabling update minimum "<<std::endl;
  update_minimum = true;
}

PrioQueue::~PrioQueue ()
{
//  NS_LOG_FUNCTION (this);
  update_minimum = true;
}

void PrioQueue::SetNodeID(uint32_t node_id)
{
  NS_LOG_LOGIC("setnodeid prioqueue");
  nodeid = node_id;
}

void PrioQueue::SetLinkID(uint32_t link_id)
{
  linkid = link_id;
}

void PrioQueue::SetLinkIDString(std::string linkid_string1)
{
  linkid_string = linkid_string1;
  NS_LOG_LOGIC("setting node id "<<nodeid<<" linkid_string "<<linkid_string);
}

std::string PrioQueue::GetLinkIDString(void)
{
  return linkid_string;
}

void
PrioQueue::SetMode (PrioQueue::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

uint32_t
PrioQueue::GetMaxPackets(void)
{
  return m_maxPackets;
}

uint32_t
PrioQueue::GetCurCount(void)
{

  return m_size;
}

double
PrioQueue::getCurrentSlope()
{
  return current_slope;
}

uint32_t
PrioQueue::GetCurSize(void)
{

  
   typedef std::list<Ptr<Packet> >::iterator PacketQueueI;
   std::map<uint32_t,uint32_t> flow_count;
   uint32_t total_pkts = 0;
 
 
   //NS_LOG_UNCOND("Current queue .. "); 
   for (PacketQueueI pp = m_packets.begin (); pp != m_packets.end (); pp++)
   {

//       std::string flowkey = GetFlowKey(*pp);
       uint32_t fid = getFlowID(*pp);
       flow_count[fid] += 1;
   } 
  
   std::map<uint32_t,uint32_t>::iterator it;
   for (std::map<uint32_t,uint32_t>::iterator it=flow_count.begin(); it!=flow_count.end(); ++it)
   {
     std::cout<<"QOCCU "<<Simulator::Now().GetSeconds()<<" flow "<<it->first<<" pktcount "<<it->second<<" queue "<<linkid_string<<std::endl;
     total_pkts += it->second;
   }
  
   //NS_LOG_UNCOND(Simulator::Now().GetSeconds()<<" totalpktscounter "<<total_pkts<<" nodeid "<<nodeid);  
   

  return m_bytesInQueue;
}

uint32_t
PrioQueue::GetMaxBytes(void)
{
  return m_maxBytes;
}

PrioQueue::QueueMode
PrioQueue::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

/***** Functions that will implement a queue using a std::list *****/
bool PrioQueue::enqueue(Ptr<Packet> p)
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
PrioQueue::remove(Ptr<Packet> p)
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
PrioQueue::GetTCPHeader(Ptr<Packet> p)
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
PrioQueue::GetIPHeader(Ptr<Packet> p)
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
PrioQueue::GetPrioHeader(Ptr<Packet> p)
{
  PppHeader ppp;
  PrioHeader pheader;
  p->RemoveHeader(ppp);
  p->PeekHeader(pheader); 
  p->AddHeader(ppp);

  return pheader;
}

std::string 
PrioQueue::GetFlowKey(Ptr<Packet> p)
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


/* function for the previous deadlines */
double
PrioQueue::get_stored_deadline(std::string fkey)
{
  if(flow_prevdeadlines.find(fkey) == flow_prevdeadlines.end()) {
    // not found
    return -1;
  } else {
    return flow_prevdeadlines[fkey];
  }
}

double
PrioQueue::get_virtualtime()
{
  return current_virtualtime;
}

inline void
PrioQueue::set_stored_deadline(std::string fkey, double new_deadline)
{
  flow_prevdeadlines[fkey] = new_deadline;
}


inline bool 
PrioQueue::remove_tag(uint64_t pktid)
{
  std::map<uint64_t, double>::iterator it = pkt_tag.begin();
  for(; it != pkt_tag.end(); it++) 
  {
    if(it->first == pktid)
    {
      pkt_tag.erase(it);
//      NS_LOG_UNCOND("remove_tag:Removed packet with id "<<pktid);
      return true;
    }
  }
  return false;

}

inline Ptr<Packet>
PrioQueue::get_packet_with_id(uint64_t pktid)
{
   typedef std::list<Ptr<Packet> >::iterator PacketQueueI;
   for (PacketQueueI pp = m_packets.begin (); pp != m_packets.end (); pp++)
   {
      if((*pp)->GetUid() == pktid) {
        return *pp;
      }
   }
   //should not happen
//   NS_LOG_UNCOND("Did not find packet with id "<<pktid);
   return NULL;
}


inline struct tag_elem
PrioQueue::get_highest_tagid()
{
  struct tag_elem elem;
  elem.deadline = 0;
  elem.pktid = -1;
  if(pkt_tag.empty() && !m_packets.empty()) {
    // should not happen
    NS_LOG_UNCOND("get_highest_tagid.. something happened.. it's empty while there are packets");
    
    return elem;
  }
  std::map<uint64_t, double>::iterator min_item = pkt_tag.begin();
  std::map<uint64_t, double>::iterator it = pkt_tag.begin();
  double lowest_tagid = it->second;
  uint64_t lowest_pktid = it->first;
   
  for(; it != pkt_tag.end(); ++it)
  {
    if(it->second > lowest_tagid) {
      lowest_tagid = it->second;
      lowest_pktid = it->first;
      min_item = it;
    }
  }
  
//  NS_LOG_UNCOND("get_lowest_tagid: returning "<<lowest_tagid<<" pkt "<<lowest_pktid);
//  pkt_tag.erase(min_item);
  elem.deadline = lowest_tagid;
  elem.pktid = lowest_pktid;
  return elem;
}


    
/*
inline double
PrioQueue::get_lowest_deadline()
{
  if(pkt_tag.empty()) {
    // should not happen
    return -1;
  }
  std::map<uint64_t, double>::iterator it = pkt_tag.begin();
  double lowest_tagid = it->second;
  uint64_t lowest_pktid = it->first;
   
  for(; it != pkt_tag.end(); ++it)
  {
    if(it->second < lowest_tagid) {
      lowest_tagid = it->second;
      lowest_pktid = it->first;
    }
  }
//  NS_LOG_UNCOND("returning packet with tag "<<lowest_tagid<<" packet id "<<lowest_pktid);
  return lowest_tagid;
}
*/

Ptr<Packet>
PrioQueue::get_lowest_tag_packet()
{
  double lowest_tag = 11111111111;
  Ptr<Packet> pkt_ptr;

  typedef std::list<Ptr<Packet> >::iterator PacketQueueI;
  for (PacketQueueI pp = m_packets.begin (); pp != m_packets.end (); pp++)
  {
      /* get the tag */
      MyTag t1; 
      (*pp)->PeekPacketTag(t1);
      double tagval = t1.GetValue();

      if(tagval < lowest_tag || lowest_tag == 11111111111) {
        lowest_tag = tagval;
        pkt_ptr = (*pp);
      }

  }

  NS_LOG_LOGIC(Simulator::Now().GetSeconds()<<" link "<<linkid_string<<" get_lowest_tagid RETURNING "<<" tagid "<<lowest_tag);
  return pkt_ptr;
}

/*
inline struct tag_elem
PrioQueue::get_lowest_tagid()
{
  struct tag_elem elem;
  elem.deadline = 0;
  elem.pktid = -1;
  if(pkt_tag.empty() && !m_packets.empty()) {
    // should not happen
    //NS_LOG_UNCOND("get_lowest_tagid.. something happened.. it's empty");
    //NS_LOG_UNCOND("get_lowest_tagid.. something happened.. it's empty while there are packets");
    NS_LOG_LOGIC("get_lowest_tagid.. something happened.. it's empty while there are "<< m_packets.size()<<" packets "<<(m_packets.front())->GetUid());
    return elem;
  }
  std::map<uint64_t, double>::iterator min_item = pkt_tag.begin();
  std::map<uint64_t, double>::iterator it = pkt_tag.begin();
  double lowest_tagid = it->second;
  uint64_t lowest_pktid = it->first;
   
  for(; it != pkt_tag.end(); ++it)
  {
    NS_LOG_LOGIC(Simulator::Now().GetSeconds()<<" link "<<linkid_string<<" get_lowest_tagid tag "<<it->second);
    if(it->second < lowest_tagid) {
      lowest_tagid = it->second;
      lowest_pktid = it->first;
      min_item = it;
    }
  }
  
//  NS_LOG_UNCOND("get_lowest_tagid: returning "<<lowest_tagid<<" pkt "<<lowest_pktid);
  pkt_tag.erase(min_item);
  elem.deadline = lowest_tagid;
  elem.pktid = lowest_pktid;
  NS_LOG_LOGIC(Simulator::Now().GetSeconds()<<" link "<<linkid_string<<" get_lowest_tagid RETURNING "<<elem.deadline<<" "<<elem.pktid<<" tagid "<<lowest_tagid);
  return elem;
}

*/
    
     
 
std::vector<std::string> 
PrioQueue::sort_by_priority(std::map<std::string, double> prios)
{
  std::map<double, std::string> temp_map;
  std::vector<std::string> sorted_flows;

  for(std::map<std::string,double>::iterator it=prios.begin(); it!= prios.end(); ++it)
  {
    /* maps from priorities to flows */
    temp_map[it->second] = it->first;
  }
  /* A map maintains elements in order of their keys. So, if we iterate over this map
   * we should get the elements in sorted order of their priorities 
   */

  for(std::map<double, std::string>::iterator it=temp_map.begin(); it != temp_map.end(); ++it)
  {
    /* maps back from flows to priorities */
    sorted_flows.push_back(it->second);
  }
  
  return sorted_flows;
}
  


bool 
PrioQueue::DoEnqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  Ipv4Address min_source, min_dest;
  Ipv4Header h1 = GetIPHeader(p);
  PrioHeader pheader  = GetPrioHeader(p);
  TcpHeader tcph = GetTCPHeader(p);

  min_source = h1.GetSource();
  min_dest = h1.GetDestination();


  Ptr<Packet> min_pp = p;
  PriHeader min_prio_header = pheader.GetData();
  double min_wfq_weight = min_prio_header.wfq_weight; //this is the deadline  
  double p_residue = min_prio_header.residue;

  bool control_packet = false;


/*  if(p->GetSize() <= 94) {
    control_packet = true;
  }
*/
  uint32_t pkt_uid = min_pp->GetUid();
//  NS_LOG_LOGIC("ENQUEU PKT WITH UID "<<pkt_uid<<" AT QUEUE "<<linkid_string);

   if(p_residue < running_min_prio && !control_packet && update_minimum) {
     std::cout<<Simulator::Now().GetSeconds()<<" link "<<linkid_string<<" updating running_min_prio to "<<running_min_prio<<" to residue "<<p_residue<<std::endl;
     running_min_prio = p_residue;
   }

  if(m_pkt_tagged) {
    MyTag tag;
    std::string flowkey = GetFlowKey(min_pp);
    double deadline = get_stored_deadline(flowkey);
    if(deadline == -1 || control_packet) {
//      NS_LOG_LOGIC(Simulator::Now().GetSeconds()<<" linkid "<<linkid_string<<" pkt_flow "<<flowkey<<" not found deadline = "<<current_virtualtime<<" pktid "<<min_pp->GetUid());
      //pkt_tag[pkt_uid] = current_virtualtime*1.0;
      tag.SetValue(current_virtualtime * 1.0, Simulator::Now().GetNanoSeconds());
    } else {
      double new_start_time= std::max(current_virtualtime*1.0, deadline);
      tag.SetValue(new_start_time, Simulator::Now().GetNanoSeconds());
//      pkt_tag[pkt_uid] = new_start_time;   //  + min_wfq_weight;
    }
    p->AddPacketTag(tag);
    // insert the new deadline now
//      NS_LOG_LOGIC(Simulator::Now().GetSeconds()<<" nodeid "<<linkid_string<<" pkt_flow "<<flowkey<<" storing deadline = "<< tag.GetValue()+ min_wfq_weight<<" tagged packet with "<<tag.GetValue()<<" pktid "<<p->GetUid());
    set_stored_deadline(flowkey, tag.GetValue()+ min_wfq_weight); //there is no need to divide 8/16
  }

  /* Also store time of arrival for each packet */
  pkt_arrival[pkt_uid] = Simulator::Now().GetNanoSeconds();
    

  // Now, enqueue    
//  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
//  double rand_num = uv->GetValue(0.0, 1.0);

  enqueue(min_pp);

  /* First check if the queue size exceeded */
  if ((m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue >= m_maxBytes)) ||
        (m_mode == QUEUE_MODE_PACKETS && m_packets.size() >= m_maxPackets))
    {
        if(m_pFabric == 0)
        {
            NS_LOG_UNCOND ("Queue full (packet would exceed max bytes) -- dropping pkt");
            remove(p);
            Drop (p);
            return false;
        } 
        else if (m_pFabric == 1)
        {
             
              typedef std::list<Ptr<Packet> >::iterator PacketQueueI;
              //NS_LOG_UNCOND("Current queue .. "); 
              for (PacketQueueI pp = m_packets.begin (); pp != m_packets.end (); pp++)
              {
                Ipv4Header h = GetIPHeader(*(pp));
                PrioHeader pheader = GetPrioHeader(*(pp));
                
                // NS_LOG_UNCOND("DoEnqueue : source "<<h.GetSource()<<" dest : "<<h.GetDestination()<<" priority "<<pheader.GetData());
                PriHeader phdata = pheader.GetData();
                if(phdata.wfq_weight > min_wfq_weight) {
                  min_wfq_weight = phdata.wfq_weight;
                  min_pp = *pp;
                  min_source = h.GetSource();
                  min_dest = h.GetDestination();
                  //NS_LOG_UNCOND("DoEnqueue : min_source "<<min_source<<" min_dest : "<<min_dest<<" priority "<<min_pp);
                }
              }

              remove(min_pp);
              Drop(min_pp);
              NS_LOG_UNCOND(Simulator::Now().GetSeconds()<<" nodeid "<<nodeid<<" dropped_packet source:"<<min_source<<"dest "<<min_dest<<" prio "<<min_wfq_weight);
              return true; 
        } // if pfabric == 1 
    } /* if queue is going to be full */

  else if ((m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue >= m_ECNThreshBytes)) ||
        (m_mode == QUEUE_MODE_PACKETS && m_packets.size() >= m_ECNThreshPackets))
    {
          // NS_LOG_UNCOND("Queue size greater than ECNThreshold. Marking packet");
          // Find the lowest priority packet and mark it with ECN marking 
          //
          
          if(m_pfabricdequeue) { 
            typedef std::list<Ptr<Packet> >::iterator PacketQueueI;
        
            for (PacketQueueI pp = m_packets.begin (); pp != m_packets.end (); pp++)
            {
              Ipv4Header h = GetIPHeader (*(pp));
              PrioHeader pheader = GetPrioHeader (*(pp));
              PriHeader phdata = pheader.GetData();
              double cur_wfq_weight = phdata.wfq_weight;
              if(cur_wfq_weight > min_wfq_weight) {
                min_wfq_weight = cur_wfq_weight;
                min_pp = *pp;
              }
            }
          } else if(m_pkt_tagged && !m_pfabricdequeue && !delay_mark) { 
            // determine the packet id with the highest tag 
            struct tag_elem elem = get_highest_tagid(); //get the lowest deadline pkt and remove it
            uint64_t highest_tag_id = elem.pktid;
            //highest_deadline = elem.deadline;
            min_pp = get_packet_with_id(highest_tag_id);
            if(!min_pp) {
              NS_LOG_UNCOND("could not get packet with id "<<highest_tag_id);
            }
            NS_LOG_LOGIC("marking pkt at enqueue");
          } 
          
        // Now add ECN bit to the IP header of the min packet 
          
        Ipv4Header min_ipheader;
        PrioHeader pheader;
        PppHeader ppp;

        if(min_pp)
        {
          min_pp->RemoveHeader(ppp);
          min_pp->RemoveHeader(pheader);
          min_pp->RemoveHeader(min_ipheader);

          min_ipheader.SetEcn(Ipv4Header::ECN_CE);
          
          min_pp->AddHeader(min_ipheader);
          min_pp->AddHeader(pheader);
          min_pp->AddHeader(ppp);

          
        } 
          
    } 

  return true;
}

int
PrioQueue::getflowid_temp(std::string fkey)
{
  if(fkey=="10.1.1.2:10.2.1.2:50000") {
    return 1;
  }
  if(fkey=="10.1.2.2:10.3.2.2:50000") {
    return 2;
  }
  if(fkey=="10.4.2.2:10.2.2.2:50000") {
    return 3;
  }
  return 0;
}

Ptr<Packet>
PrioQueue::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  if(m_packets.empty())
  {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
  }

  /* Toss a coin - 50% of the do regular dequeue and 50% do priority dequeue */
//  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
//  double rand_num = uv->GetValue(0.0, 1.0);
  Ptr<Packet> p = m_packets.front ();
  incoming_bytes += p->GetSize(); 



  double lowest_deadline = 0.0;
  double pkt_wait_duration = 0.0;

  if(!m_pkt_tagged) { 

    typedef std::list<Ptr<Packet> >::iterator PacketQueueI;
	  double highest_wfq_weight_;
	  PacketQueueI pItr = m_packets.begin();
    Ipv4Header h;
    PrioHeader pheader;
    Ipv4Address highSource, highDest;

    if (m_packets.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

	  if (*pItr != NULL) 
    {
//      NS_LOG_UNCOND("Dequeued packet with id "<<(*pItr)->GetUid()<<" size "<<(*pItr)->GetSize());
      pheader = GetPrioHeader(*pItr);
      h = GetIPHeader(*pItr);

		  highest_wfq_weight_ = (pheader.GetData()).wfq_weight;
      highSource = h.GetSource();
      highDest = h.GetDestination();
    }
	  else
    { //should not occur !!
      //NS_LOG_UNCOND("DoDequeue: p is NULL");
		  return 0;
    }

    for (PacketQueueI pp = m_packets.begin (); pp != m_packets.end (); pp++)
    {
      pheader = GetPrioHeader(*pp);
      h = GetIPHeader(*pp);

	    double  cur_wfq_weight = (pheader.GetData()).wfq_weight;
		  //deque from the head
		  if (cur_wfq_weight < highest_wfq_weight_) {
			  pItr = pp;
				highest_wfq_weight_ = cur_wfq_weight;
        highSource = h.GetSource();
        highDest = h.GetDestination();
		  }
	  }
    //if(nodeid == 0) {
//    NS_LOG_LOGIC(Simulator::Now().GetSeconds()<<" node "<<nodeid<<" prio "<<highest_prio_<<" flowkey "<<GetFlowKey(p)<<"linkid "<<linkid<<" DEQUEUED ");
    //}
  
    p = (*pItr);
  } 
  else if (m_pkt_tagged)
  {
    // determine the packet id with the highest tag 
    //struct tag_elem elem = get_lowest_tagid(); //get the lowest deadline pkt and remove it
    //uint64_t lowest_tag_id = elem.pktid;
    //lowest_deadline = elem.deadline;
    //p = get_packet_with_id(lowest_tag_id); 
    p = get_lowest_tag_packet();
    if(!p) {
      NS_LOG_LOGIC("could not get packet with "<<linkid_string);
    }
    MyTag t1;
    p->RemovePacketTag(t1);
    lowest_deadline = t1.GetValue();

    double pkt_depart = Simulator::Now().GetNanoSeconds();
    pkt_wait_duration = pkt_depart - t1.GetArrival();

    //std::cout<<"packetdeadline "<<Simulator::Now().GetSeconds()<<" "<<lowest_deadline<<" removed at node "<<nodeid<<" "<<GetFlowKey(p)<<std::endl;
    // increment virtual time
    current_virtualtime = std::max(current_virtualtime*1.0, lowest_deadline);

    virtualtime_updated += 1;
    if(virtualtime_updated >= vpackets) {
     virtualtime_updated = 0;
     current_slope = CalcSlope();
//     NS_LOG_LOGIC("SLOPEINFO "<<linkid_string<<" "<<Simulator::Now().GetMicroSeconds()<<" "<<current_slope);
    }
    //std::cout<<"virtualtime at switch "<<nodeid<<" "<<Simulator::Now().GetSeconds()<<" "<<current_virtualtime<<std::endl;  
    
  } 
 

   // debug 
   Ptr<Packet> ret_packet = p;
   bool removesuc = remove(p);
   if(removesuc == false) {
     NS_LOG_UNCOND("ERROR !! Remove failed for packet "<<p->GetUid());
   }

   /* At Dequeue, update the network price field of the packet with the current link price */

   PppHeader temp_ppp;
   PrioHeader temp_pheader;
   Ipv4Header  temp_ip_header;
   TcpHeader   temp_tcpheader;  

   ret_packet->RemoveHeader(temp_ppp);
   ret_packet->RemoveHeader(temp_pheader);
   ret_packet->RemoveHeader(temp_ip_header);

   if(pkt_wait_duration >= 0) {
    if(pkt_wait_duration > ONEPACKET) {
      // bottlenecked
      std::cout<<" setting ecn ";
      temp_ip_header.SetEcn(Ipv4Header::ECN_CE);
    } else {
      std::cout<<" notsetting ecn ";
    }
   }

   PriHeader ph = temp_pheader.GetData();
   ph.netw_price += current_price;
   temp_pheader.SetData(ph);

   ret_packet->AddHeader(temp_ip_header);
   ret_packet->AddHeader(temp_pheader);
   ret_packet->AddHeader(temp_ppp);

   std::cout<<"Queue "<<linkid_string<<" node "<<nodeid<<" pkt_wait_duration "<<pkt_wait_duration<<" "<<Simulator::Now().GetNanoSeconds()<<" flow "<<getflowid_temp(GetFlowKey(ret_packet))<<" pkt "<<ret_packet->GetUid()<<std::endl;

   if(delay_mark) {
    //double pkt_depart = Simulator::Now().GetNanoSeconds();
    //double pkt_wait_duration = pkt_depart - pkt_arrival[ret_packet->GetUid()];
    ecn_delaythreshold = 1000000000.0 * (m_ECNThreshBytes * 8.0)/(m_bps.GetBitRate()); // in ns, assuming m_link_datarate is in bps
    std::string flowkey = GetFlowKey(ret_packet);
  //  if(linkid_string == "0_0_1") {
 //     NS_LOG_LOGIC("QWAIT "<<linkid_string<<" "<<Simulator::Now().GetSeconds()<<" "<<flow_ids[flowkey]<<" spent "<<pkt_wait_duration<<" in queue "<<linkid_string);
//      NS_LOG_LOGIC("DEQUEUE "<<linkid_string<<" "<<Simulator::Now().GetMicroSeconds()<<" "<<flow_ids[flowkey]<<" pkt size "<<ret_packet->GetSize());
   // }
//    NS_LOG_LOGIC(" DEQUEUE PKT "<<ret_packet->GetUid()<<" link "<<linkid_string);
  
    if(pkt_wait_duration > ecn_delaythreshold) {
        Ipv4Header ipheader;
        PrioHeader pheader;
        PppHeader  ppp;

        ret_packet->RemoveHeader(ppp);
        ret_packet->RemoveHeader(pheader);
        ret_packet->RemoveHeader(ipheader);

        ipheader.SetEcn(Ipv4Header::ECN_CE);
        
        ret_packet->AddHeader(ipheader);
        ret_packet->AddHeader(pheader);
        ret_packet->AddHeader(ppp);

        //std::string flowkey = GetFlowKey(ret_packet);
        //NS_LOG_LOGIC("marking pkt from flow "<<flowkey);

    }
  }

  /* Update the network price - 08/09 kn */
	


  return ret_packet;
}

void
PrioQueue::SetVPkts(uint32_t vpkts)
{
  vpackets = vpkts;
}

double 
PrioQueue::CalcSlope(void)
{
  double time_elapsed = Simulator::Now().GetNanoSeconds() - last_virtualtime_time;
  double slope = (current_virtualtime - last_virtualtime) * 1000.0 / time_elapsed;
  last_virtualtime_time = Simulator::Now().GetNanoSeconds();
  last_virtualtime = current_virtualtime;
  return slope;
} 


Ptr<const Packet>
PrioQueue::DoPeek (void) const
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


