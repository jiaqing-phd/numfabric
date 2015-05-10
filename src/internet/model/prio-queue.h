
/* Priority Queue functions are not implemented in NS-3. Implementing them */

#ifndef PRIO_QUEUE_H
#define PRIO_QUEUE_H

#include <queue>
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/queue.h"
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/prio-header.h"
#include "ns3/data-rate.h"
#include "ns3/boolean.h"
#include "tcp-header.h"

//#include "ns3/traced-callback.h"

//#include <tr1/unordered_map>
//#include <tr1/functional>

using std::queue;
//using std::tr1::unordered_map;


namespace ns3 {
struct tag_elem {
    uint64_t pktid;
    double deadline;
};

class Ipv4Address;
class Ipv4Header;

typedef struct Flowkey {
  Ipv4Address src;
  Ipv4Address dst;
  int fid;
} FlowKey;

class TraceContainer;

class PrioQueue : public Queue {
public:

  static TypeId GetTypeId (void);
  PrioQueue ();
  virtual ~PrioQueue ();

  bool IsEmpty (void) const;
  void SetMode (PrioQueue::QueueMode mode);
  uint32_t nodeid;
  uint32_t linkid;
  std::string linkid_string;
  void SetNodeID (uint32_t nodeid);
  void SetLinkID (uint32_t linkid);
  void SetLinkIDString (std::string linkid_string);
  std::string GetLinkIDString(void);
  std::string GetFlowKey(Ptr<Packet> p);
  PrioHeader GetPrioHeader(Ptr<Packet> p);
  Ipv4Header GetIPHeader(Ptr<Packet> p);
  std::vector<std::string> sort_by_priority(std::map<std::string, double> prios);
  double eps;
  Time m_updateMinPrioTime;
  Time m_updatePriceTime;
  double latest_avg_prio, running_avg_prio;
  int total_samples;
  double latest_min_prio, running_min_prio;
  void updateMinPrio(void);
  double incoming_rate;
  double outgoing_rate;
  double incoming_bytes;
  

  /**
   * Get the encapsulation mode of this device.
   *
   * \returns The encapsulation mode of this device.
   */
  PrioQueue::QueueMode GetMode (void);
  uint32_t GetMaxPackets(void);
  uint32_t GetMaxBytes(void);
  uint32_t GetCurCount(void);
  uint32_t GetCurSize(void);
  double getCurrentPrice(void);

  uint32_t m_ECNThreshBytes;  
  uint32_t m_ECNThreshPackets;

  double ecn_delaythreshold;
  bool delay_mark;


  void updateLinkPrice(void);
  double getRateDifference(Time t);
  double getRateDifferenceNormalized(Time t);

  /* data structure to maintain prev_deadline for every flow */
  std::map<std::string , double> flow_prevdeadlines;
  std::map<uint64_t, double> pkt_tag;
  std::map<uint64_t, double> pkt_arrival;
  double get_stored_deadline(std::string fkey);
  void set_stored_deadline(std::string fkey, double new_deadline);
  bool remove_tag(uint64_t pktid);
  Ptr<Packet> get_packet_with_id(uint64_t pktid);
  struct tag_elem get_lowest_tagid();
  struct tag_elem get_highest_tagid();
  double get_lowest_deadline();
  TcpHeader GetTCPHeader(Ptr<Packet> p);

private:
  virtual bool DoEnqueue (Ptr<Packet> p);
  virtual Ptr<Packet> DoDequeue (void);
  virtual Ptr<const Packet> DoPeek (void) const;

  bool enqueue(Ptr<Packet> p);
  bool remove(Ptr<Packet> p);

  std::list<Ptr<Packet> > m_packets; //!< the packets in the queue
  uint32_t m_maxPackets;              //!< max packets in the queue
  uint32_t m_maxBytes;                //!< max bytes in the queue
  uint32_t m_bytesInQueue;            //!< actual bytes in the queue
  uint32_t m_size;
  uint32_t m_pFabric;
  bool m_fluid_model;
  bool m_subtract_min;
  bool m_recv_marks;
  bool m_pkt_tagged;
  bool m_onlydctcp;
  bool m_pfabricdequeue;
  bool m_strawmancc;
  bool m_dctcp_mark;
  DataRate m_bps;
  QueueMode m_mode;                   //!< queue mode (packets or bytes limited)
  uint32_t  current_virtualtime;

//  unsigned int sq_limit_;
//  unordered_map<size_t, int> sq_counts_;
//  std::queue<size_t> sq_queue_;
  
  std::map<std::string, double>prios;
  std::map<std::string, double>rates;



};

} // namespace ns3

#endif /* PRIO_QUEUE_H */

