
/* Droptail queue with ECN marking */

#ifndef FIFO_QUEUE_H
#define FIFO_QUEUE_H

#include <queue>
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/queue.h"
#include "ns3/header.h"
#include "ns3/prio-header.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/data-rate.h"
#include "ns3/boolean.h"
#include "tcp-header.h"
#include <map>

using std::queue;

namespace ns3 {
class Ipv4Address;
class Ipv4Header;

class TraceContainer;

class FifoQueue : public Queue {
public:

  static TypeId GetTypeId (void);
  FifoQueue ();
  virtual ~FifoQueue ();

  bool IsEmpty (void) const;
  void SetMode (FifoQueue::QueueMode mode);
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
  double incoming_rate;
  double outgoing_rate;
  double incoming_bytes;
   
  uint32_t getFlowID(Ptr<Packet> p);
  void setFlowID(std::string flowkey, uint32_t fid, double fweight);
  std::map<std::string, uint32_t>flow_ids;
  std::map<uint32_t, double>flow_weights;
  bool init_reset;
  uint32_t total_deq;


  /**
   * Get the encapsulation mode of this device.
   *
   * \returns The encapsulation mode of this device.
   */
  FifoQueue::QueueMode GetMode (void);
  uint32_t GetMaxPackets(void);
  uint32_t GetMaxBytes(void);
  uint32_t GetCurCount(void);
  uint32_t GetCurSize();

  uint32_t m_ECNThreshBytes;  
  uint32_t m_ECNThreshPackets;

  double ecn_delaythreshold;



  std::map<uint64_t, double> pkt_arrival;
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
  DataRate m_bps;
  QueueMode m_mode;                   //!< queue mode (packets or bytes limited)
  uint32_t  current_virtualtime;
  

};

} // namespace ns3

#endif /* PRIO_QUEUE_H */

