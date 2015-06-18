
/* Priority Queue functions are not implemented in NS-3. Implementing them */

#ifndef fifo_hybridQ_H
#define fifo_hybridQ_H

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

using std::queue;


namespace ns3 {
class Ipv4Address;
class Ipv4Header;

class fifo_hybridQ : public Queue {
public:

  bool fifo_1_qempty(void);
  bool fifo_2_qempty(void);

  static TypeId GetTypeId (void);
  fifo_hybridQ ();
  virtual ~fifo_hybridQ ();

  bool IsEmpty (void) const;
  void SetMode (fifo_hybridQ::QueueMode mode);
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

  fifo_hybridQ::QueueMode GetMode (void);

  void setFlowID(std::string flowkey, uint32_t fid, double fweight);
  uint32_t GetCurCount(uint32_t fid);
  // uint32_t GetCurSize(uint32_t fid);
  uint32_t GetFifo_1_Size(void);
  uint32_t GetFifo_2_Size(void);
  uint32_t GetMaxBytes(void);
  uint32_t getFlowID(Ptr<Packet> p);
  
bool fifo_1_EnQ(Ptr<Packet> p);
bool fifo_2_EnQ(Ptr<Packet> p);
  
bool known_flow(uint32_t flowid);
Ptr<Packet> fifo_2_Dequeue(void);
Ptr<Packet> fifo_1_Dequeue(void);

private:
  virtual bool DoEnqueue (Ptr<Packet> p);
  virtual Ptr<Packet> DoDequeue (void);

  bool removeFifo_1(Ptr<Packet> p);
  bool removeFifo_2(Ptr<Packet> p);
  uint32_t turn;


  std::queue<Ptr <Packet> >m_fifo_1_pkts;
  std::queue<Ptr <Packet> >m_fifo_2_pkts;
  
  uint32_t m_fifo_1_size;
  uint32_t m_fifo_2_size;
  uint32_t m_fifo_1_bytesInQueue;
  uint32_t m_fifo_2_bytesInQueue;

  std::map<uint32_t, std::queue<Ptr <Packet> > >m_packets; //!< the packets in the queue
  std::map<uint32_t, uint32_t> m_size;
  std::map<std::string, uint32_t> flow_ids;
  std::map<uint32_t, double> flow_weights;
  uint32_t m_maxPackets;              //!< max packets in the queue
  uint32_t m_maxBytes;                //!< max bytes in the queue
  std::map<uint32_t, uint32_t> m_bytesInQueue;            //!< actual bytes in the queue
  DataRate m_bps;
  QueueMode m_mode;                   //!< queue mode (packets or bytes limited)

  uint32_t vpackets;
  
  TcpHeader GetTCPHeader(Ptr<Packet> p);
  Ptr<const Packet> DoPeek (void) const;
  
};

} // namespace ns3

#endif /* PRIO_QUEUE_H */

