#include "declarations.h"

class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  //void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate, uint32_t maxbytes, double flow_start, Address address1);
  void Setup (Address address, uint32_t packetSize, DataRate dataRate, uint32_t maxbytes, double flow_start, Address address1, Ptr<Node> pnode, uint32_t fid, Ptr<Node> dnode);
  virtual void StartApplication (void);
private:

  void ScheduleTx (void);
  void SendPacket (void);
  virtual void StopApplication (void);


  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
  uint32_t        m_maxBytes;
  double          m_startTime;
  EventId         m_startEvent;
  uint32_t        m_totBytes;
  Address         myAddress;
  Ptr<Node>       srcNode;
  Ptr<Node>       destNode;
  uint32_t        m_fid;
};
