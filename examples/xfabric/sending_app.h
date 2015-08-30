#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/prio-queue.h"


using namespace ns3;
extern void setuptracing(uint32_t sindex, Ptr<Socket> skt);

class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  //void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate, uint32_t maxbytes, double flow_start, Address address1);
  void Setup (Address address, uint32_t packetSize, DataRate dataRate, uint32_t maxbytes, double flow_start, Address address1, Ptr<Node> pnode, uint32_t fid, Ptr<Node> dnode, uint32_t tcp, uint32_t fknown, double stop_time=-1);
  virtual void StartApplication (void);
  void ChangeRate (DataRate passed_in_rate);
  uint32_t getFlowId(void);
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
  double          m_stoptime;
  EventId         m_startEvent;
  uint32_t        m_totBytes;
  Address         myAddress;
  Ptr<Node>       srcNode;
  Ptr<Node>       destNode;
  uint32_t        m_fid;
  uint32_t        m_udp;
  uint32_t        flow_known;
  
  Ptr<Tracker>         flowTracker;
};
