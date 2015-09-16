#include "sending_app.h"
#include "declarations.h"

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0),
    m_maxBytes (0)
{
  m_totBytes = 0;
  
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

uint32_t
MyApp::getFlowId(void)
{
  return m_fid;
}

void
MyApp::ChangeRate (DataRate passed_in_rate)
{
  std::cout<<"ChangeRate called flow "<<m_fid<<" rate "<<passed_in_rate<<std::endl;
  m_dataRate = passed_in_rate; 
}

void
//MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate, Ptr<RandomVariableStream> interArrival)
//MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate, uint32_t maxBytes, double start_time, Address ownaddress, Ptr<Node> sNode)
MyApp::Setup (Address address, uint32_t packetSize, DataRate dataRate, uint32_t maxBytes, double start_time, Address ownaddress, Ptr<Node> sNode, uint32_t fid, Ptr<Node> dNode, uint32_t tcp, uint32_t fknown, double stop_time)
{
  //m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_dataRate = dataRate;
  m_totBytes = 0;
  m_maxBytes = maxBytes; // assuming we are getting this in bits
  m_startTime = start_time;
  m_fid = fid;

  if(tcp == 0) {
    m_udp = 1;
  } else {
    m_udp = 0;
  }
  flow_known = fknown;
  
  Time tNext = Time(Seconds(m_startTime));
  myAddress = ownaddress;
  srcNode = sNode;
  destNode = dNode;

  m_stoptime = stop_time;

  //NS_LOG_UNCOND("Scheduling start of flow "<<fid<<" at time "<<Time(tNext).GetSeconds());
  m_startEvent = Simulator::Schedule (tNext, &MyApp::StartApplication, this);
}

void
MyApp::StartApplication (void)
{

  if(Simulator::Now().GetNanoSeconds() < Time(Seconds(m_startTime)).GetNanoSeconds()) {
//    std::cout<<"Time "<<Simulator::Now().GetNanoSeconds()<<" spurious call flowid "<<m_fid<<" returning before start_time "<<  Time(Seconds(m_startTime)).GetNanoSeconds()<<std::endl;
    if(Simulator::IsExpired(m_startEvent)) {
      Time tNext = Time(Seconds(m_startTime));
      m_startEvent = Simulator::Schedule (tNext, &MyApp::StartApplication, this);
//      std::cout<<"Time "<<Simulator::Now().GetSeconds()<<" spurious call flowid "<<m_fid<<" rescheduling at  "<<tNext.GetSeconds()<<std::endl;
      
    }
      
    return;

  }

  
  m_running = true;
  m_packetsSent = 0;
  m_totBytes = 0;

  Ptr<Socket> ns3TcpSocket;
  if(m_udp) {
    ns3TcpSocket = Socket::CreateSocket (srcNode, UdpSocketFactory::GetTypeId());
  } else {
    ns3TcpSocket = Socket::CreateSocket (srcNode, TcpSocketFactory::GetTypeId ());
    Ptr<TcpNewReno> nReno = StaticCast<TcpNewReno> (ns3TcpSocket);
    if(flow_known == 0) {
      nReno->setdctcp(true); // unknown flows always run dctcp
      nReno->setxfabric(false);
      /* Set initial cwnd to 10 pkts = 1500 * 10 Bytes */
      uint32_t unknown_initcwnd = ((1500 * 10)/max_segment_size) + 1;
      uint32_t ssthresh = unknown_initcwnd * max_segment_size;

      nReno->resetInitCwnd(unknown_initcwnd);
      nReno->resetSSThresh(ssthresh);
    }
  }
  //setuptracing(m_fid, ns3TcpSocket);
  m_socket = ns3TcpSocket;
  if (InetSocketAddress::IsMatchingType (m_peer))
  { 
    //NS_LOG_UNCOND("flow_start "<<m_fid<<" time "<<(Simulator::Now()).GetSeconds());
    //m_socket->Bind (myAddress);
    m_socket->Bind ();
  }
  else
  {
    m_socket->Bind6 ();
  }
  m_socket->Connect (m_peer);
    
  SendPacket ();
  std::cout<<"flow_start "<<m_fid<<" start_time "<<Simulator::Now().GetNanoSeconds()<<" flow_size "<<m_maxBytes<<" "<<srcNode->GetId()<<" "<<destNode->GetId()<<" port "<< InetSocketAddress::ConvertFrom (m_peer).GetPort () <<" "<<flow_known<<std::endl;
  //FlowData dt(m_fid, m_maxBytes, flow_known, srcNode->GetId(), destNode->GetId(), fweight);
  //flowTracker->registerEvent(1);
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
  std::cout<<Simulator::Now().GetSeconds()<<" flowid "<<m_fid<<" stopped sending after seding "<<m_totBytes<<std::endl;
}

void
MyApp::SendPacket (void)
{
  
  Ptr<Packet> packet = Create<Packet> (m_packetSize);

  int ret_val = m_socket->Send( packet ); 
  //std::cout<<"***  "<<Simulator::Now().GetSeconds()<<" sent packet with id "<<packet->GetUid()<<" size "<<packet->GetSize()<<" flowid "<<m_fid<<" source "<<srcNode->GetId()<<" destNode "<<destNode->GetId()<<" myaddress "<<myAddress<<" peeraddress "<<m_peer<<" *** "<<std::endl;  
  if(ret_val != -1) {
    m_totBytes += packet->GetSize();
  } else {
 //   NS_LOG_UNCOND(Simulator::Now().GetSeconds()<<" flowid "<<m_fid<<" Tcp buffer is overflowing.. trying later");
  }

//    std::cout<<" flow_id "<<m_fid<<" sent "<<ret_val<<" bytes total bytes so far "<<m_totBytes<<std::endl;
//  NS_LOG_UNCOND(Simulator::Now().GetSeconds()<<" flowid "<<m_fid<<" bytes sent "<<m_totBytes<<" maxBytes "<<m_maxBytes);

  //if (m_totBytes < m_maxBytes)
  //  {
    if((m_stoptime == -1) || ((m_stoptime != -1) && (Simulator::Now().GetSeconds() < m_stoptime))) {
      ScheduleTx ();
    } else {
      StopApplication();
    }
  
}

void
MyApp::ScheduleTx (void)
{
  //if (m_running)
  if ((m_maxBytes == 0) || (m_totBytes < m_maxBytes))
    {
      Time tNext (Seconds ((m_packetSize+38) * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    } else {
      StopApplication();
    }
}
