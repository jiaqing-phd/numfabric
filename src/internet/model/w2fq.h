
/* Priority Queue functions are not implemented in NS-3. Implementing them */

#ifndef W2FQ_H
#define W2FQ_H

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
class Ipv4Address;
class Ipv4Header;

class W2FQ : public Queue {
public:

  static TypeId GetTypeId (void);
  W2FQ ();
  virtual ~W2FQ ();

  bool IsEmpty (void) const;
  void SetMode (W2FQ::QueueMode mode);
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

  W2FQ::QueueMode GetMode (void);

  uint32_t get_virtualtime(void);
  void setFlowID(std::string flowkey, uint32_t fid, double fweight, uint32_t a);
  std::map<uint32_t, uint64_t> pkt_arrival;
  uint32_t GetCurCount(uint32_t fid);
  uint32_t GetCurSize(uint32_t fid);
  uint32_t GetMaxBytes(void);
  uint32_t getFlowID(Ptr<Packet> p);

  void SetVPkts(uint32_t);
  

private:
  bool init_reset;
  virtual bool DoEnqueue (Ptr<Packet> p);
  virtual Ptr<Packet> DoDequeue (void);

  bool enqueue(Ptr<Packet> p, uint32_t flowid);
  bool remove(Ptr<Packet> p, int32_t flowid);

  std::map<uint32_t, std::queue<Ptr <Packet> > >m_packets; //!< the packets in the queue
  std::map<uint32_t, uint32_t> m_size;
  std::map<std::string, uint32_t> flow_ids;
  std::map<uint32_t, double> flow_weights;
  uint32_t m_maxPackets;              //!< max packets in the queue
  uint32_t m_maxBytes;                //!< max bytes in the queue
  std::map<uint32_t, uint32_t> m_bytesInQueue;            //!< actual bytes in the queue
  DataRate m_bps;
  QueueMode m_mode;                   //!< queue mode (packets or bytes limited)
  double current_virtualtime;
  std::map<uint32_t, double> local_flow_weights;

  
  std::map<uint32_t, double> start_time;
  std::map<uint32_t, double> finish_time;

  double current_slope;
  double CalcSlope(void);
  double last_virtualtime_time;
  double last_virtualtime;
  uint32_t vpackets;
  uint32_t virtualtime_updated;
  
  double getWFQweight(Ptr<Packet> p);
  bool QueueEmpty(void);
  void resetFlows(uint32_t fid, Ptr<Packet> p);
  void re_resetFlows(uint32_t fid, Ptr<Packet> p);
  
  TcpHeader GetTCPHeader(Ptr<Packet> p);
  Ptr<const Packet> DoPeek (void) const;
  
};

} // namespace ns3

#endif /* PRIO_QUEUE_H */

