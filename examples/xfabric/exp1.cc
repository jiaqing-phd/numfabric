/* 3 flows start - f2 stops at 1.5 seconds */


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

NS_LOG_COMPONENT_DEFINE ("xfabric");

int checkTimes = 0;
std::stringstream filePlotQueue;

double sim_time = 2.0;
double measurement_starttime = 1.2;
double prio_q_min_update_time = 0.001;
double gamma1_value = 10.0;

double price_update_time = 0.0001;
double rate_update_time = 0.0001;
double gamma_value = 0.000001;
double flow2_stoptime=1.5;
double flow1_stoptime=1.5;
double flow3_stoptime=1.5;
double flow2_starttime=0;
double flow1_starttime=0;
double flow3_starttime=0;

double alpha_value = 1.0*1e-10;
double target_queue = 30000.0;


float sampling_interval = 0.0001;
uint32_t pkt_size = 1040;
//uint32_t max_queue_size = 150000;
uint32_t max_ecn_thresh = 0;
uint32_t max_queue_size = 450000000;
//uint32_t max_ecn_thresh = 30000;
//uint32_t max_segment_size = 1440;
uint32_t max_segment_size = 1402;
uint32_t ssthresh_value = 3000;
std::map<std::string, uint32_t> flowids;
uint32_t recv_buf_size = 1310720;
uint32_t send_buf_size = 1310720;
double link_delay = 7.0; //in microseconds
//double link_delay = 7.5e-6;
double link_rate = 1000000000; //1G
std::string link_rate_string = "1Gbps";
bool margin_util_price = false;


class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate, double starttime, double stoptime);
  virtual void StartApplication (void);
private:

  void ScheduleTx (void);
  void SendPacket (void);
  virtual void StopApplication (void);


  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
  double          m_stop_time;
  double          m_start_time;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate, double start_time, double stoptime)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
  m_start_time = start_time;
  m_stop_time = stoptime;
  
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind ();
    }
  else
    {
      m_socket->Bind6 ();
    }
  m_socket->Connect (m_peer);
  SendPacket ();
  std::cout<<"Application started.... "<<Simulator::Now().GetSeconds()<<std::endl;
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
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  /* put the new header in the packet - how ? */
/*  packet->AddHeader(pFabric); */
  m_socket->Send (packet);
 // m_socket->Send (packet);
 // m_socket->Send (packet);
//  m_socket->Send (packet);
//  m_socket->Send (packet);
  

//  if (++m_packetsSent < m_nPackets)
  if ((Simulator::Now().GetSeconds()) < m_stop_time)
    {
      ScheduleTx ();
    } else {
      StopApplication();
    }
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
//  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

void
RTOChange (Ptr<OutputStreamWrapper> stream, ns3::Time oldRTO, ns3::Time newRTO)
{
//  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
  *stream->GetStream() << Simulator::Now ().GetSeconds () << "\t" << oldRTO.GetSeconds() << "\t" << newRTO.GetSeconds() << std::endl;
}

/*
static void
RxDrop (Ptr<PcapFileWrapper> file, Ptr<const Packet> p)
{
  NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
  file->Write (Simulator::Now (), p);
}
*/



void change_delay(Ptr<PointToPointChannel> chan1, Ptr<PointToPointChannel> chan2)
{
  chan1->SetAttribute("Delay", TimeValue(MicroSeconds(30.0)));
//  chan2->SetAttribute("Delay", TimeValue(MicroSeconds(22.0)));
}

void
CheckQueueSize (Ptr<Queue> queue)
{
  uint32_t qSize = StaticCast<PrioQueue> (queue)->GetCurSize ();
  uint32_t nid = StaticCast<PrioQueue> (queue)->nodeid;
  double qPrice = StaticCast<PrioQueue> (queue)->getCurrentPrice ();
  
  std::cout<<"QueueStats "<<nid<<" "<<Simulator::Now ().GetSeconds () << " " << qSize<<" "<<qPrice<<std::endl;
  // check queue size every 1/1000 of a second
  //
  //
  // Get other statistics like price
  std::map<std::string, uint32_t>::iterator it;
  for (std::map<std::string,uint32_t>::iterator it= flowids.begin(); it!= flowids.end(); ++it) {
    double dline = StaticCast<PrioQueue> (queue)->get_stored_deadline(it->first);
    std::cout<<"QueueStats1 "<<nid<<" "<<Simulator::Now().GetSeconds()<<" "<<it->second<<" "<<dline<<std::endl;
   }
    
  Simulator::Schedule (Seconds (sampling_interval), &CheckQueueSize, queue);
  if(Simulator::Now().GetSeconds() >= sim_time) {
    Simulator::Stop();
  }
  checkTimes++;
}

void
CheckIpv4Rates (NodeContainer &allNodes)
{
  uint32_t N = allNodes.GetN(); 
  for(uint32_t nid=1; nid < N ; nid++)
  {
    Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(nid))->GetObject<Ipv4> ());
    std::map<std::string,uint32_t>::iterator it;
    for (std::map<std::string,uint32_t>::iterator it=ipv4->flowids.begin(); it!=ipv4->flowids.end(); ++it)
    {
    
      double rate = ipv4->GetStoreRate (it->first);
      double prio = ipv4->GetStorePrio (it->first);
      double destrate = ipv4->GetStoreDestRate (it->first);
      double flow_price = ipv4->getCurrentNetwPrice(it->first);
      double increments = ipv4->getCurrentDeadline();
      double targetRate = ipv4->getCurrentTargetRate();
      
      if(((it->second == 1) && (nid == 2)) || (it->second == 2 && nid == 3)) {
        double rate1, rate2, ratio;
        if(nid == 2) {
          rate1 = rate;
        } else if (nid == 3) {
          rate2 = rate;
        }
        if(rate1 > 0.0) {  
          ratio = rate2/rate1;
        }
        std::cout<<"RatePrio flowid "<<it->second<<" "<<Simulator::Now ().GetSeconds () << " " << rate << " "<<prio<<" "<<flow_price<<" "<<increments<<" "<<targetRate<<" "<<ratio<<std::endl;
      }
      if(((it->second == 1) && (nid == 4)) || (it->second == 2 && nid == 5)) {
        double rate1, rate2, ratio;
        if(nid == 4) {
          rate1 = destrate;
        } else if (nid == 5) {
          rate2 = destrate;
        }
        if(rate1 > 0.0) {  
          ratio = rate2/rate1;
        }
        std::cout<<"DestRatePrio flowid "<<it->second<<" "<<Simulator::Now ().GetSeconds () << " " << destrate<<" ratio "<<ratio<<std::endl;
      }

    }
  }
  
  // check queue size every 1/1000 of a second
  Simulator::Schedule (Seconds (sampling_interval), &CheckIpv4Rates, allNodes);
  if(Simulator::Now().GetSeconds() >= sim_time) {
    Simulator::Stop();
  }
}

int 
main (int argc, char *argv[])
{
  uint32_t N = 6; //number of nodes in the star
  uint32_t skip = 2;
  std::string prefix;
  std::string queue_type;
  double epoch_update_time = 0.001;
  std::string flow_util_file;
  bool pkt_tag, onlydctcp, wfq, dctcp_mark;
  bool strawmancc = false;
  
  

  // Allow the user to override any of the defaults and the above
  // Config::SetDefault()s at run-time, via command-line arguments
  CommandLine cmd;
  cmd.AddValue ("nNodes", "Number of nodes to place in the star", N);
  cmd.AddValue ("prefix", "Output prefix", prefix);
  cmd.AddValue ("queuetype", "Queue Type", queue_type);
  cmd.AddValue ("epoch_update_time", "Epoch Update", epoch_update_time);
  cmd.AddValue ("flow_util_file", "flow_util_file", flow_util_file);
  cmd.AddValue("pkt_tag","pkt_tag",pkt_tag);
  cmd.AddValue ("onlydctcp", "onlydctcp", onlydctcp);
  cmd.AddValue ("dctcp_mark", "dctcp_mark", dctcp_mark);
  cmd.AddValue ("wfq", "wfq", wfq);
  cmd.AddValue ("sim_time", "sim_time", sim_time);
  cmd.AddValue ("pkt_size", "pkt_size", pkt_size);
  cmd.AddValue ("link_rate","link_rate",link_rate);
  cmd.AddValue ("link_delay","link_delay",link_delay);
  cmd.AddValue ("ecn_thresh", "ecn_thresh", max_ecn_thresh);
  cmd.AddValue ("price_update_time", "price_update_time", price_update_time);
  cmd.AddValue ("rate_update_time", "rate_update_time", rate_update_time);
  cmd.AddValue ("flow2_stoptime", "flow2_stoptime", flow2_stoptime);
  cmd.AddValue ("flow2_starttime", "flow2_starttime", flow2_starttime);

  cmd.AddValue ("flow1_stoptime", "flow1_stoptime", flow1_stoptime);
  cmd.AddValue ("flow1_starttime", "flow1_starttime", flow1_starttime);

  cmd.AddValue ("flow3_stoptime", "flow3_stoptime", flow3_stoptime);
  cmd.AddValue ("flow3_starttime", "flow3_starttime", flow3_starttime);

  cmd.AddValue("gamma", "gamma", gamma_value);
  cmd.AddValue("margin_util_price", "margin_util_price", margin_util_price);
  cmd.AddValue("strawmancc", "strawmancc", strawmancc);

  cmd.Parse (argc, argv);

  std::string link_twice_string = "20Gbps";

  if(link_rate == 10000000000) {
    link_rate_string = "10Gbps";
    link_twice_string = "20Gbps";
  } else if(link_rate == 100000000000) {
    link_rate_string = "100Gbps";
  }


  NS_LOG_UNCOND("file prefix "<<prefix);

  double total_rtt = link_delay * 5.0;
  uint32_t bdproduct = link_rate *total_rtt/(1000000.0 *8.0);
//  uint32_t bdproduct = link_rate *total_rtt/8.0;
  uint32_t initcwnd = (bdproduct / max_segment_size)+1;
  if(strawmancc) {
    initcwnd = initcwnd*4.0;
  }
  uint32_t ssthresh = initcwnd * max_segment_size;


  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue(max_segment_size));
//  Config::SetDefault ("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue(ssthresh_value));
  Config::SetDefault ("ns3::TcpSocketBase::Timestamp", BooleanValue(false));
  Config::SetDefault ("ns3::TcpSocketBase::ReceiverWillMark", BooleanValue(false));
//  Config::SetDefault ("ns3::RttEstimator::MinRTO", TimeValue(Seconds(0.001)));
  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (recv_buf_size));
  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (send_buf_size));

  Config::SetDefault ("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue(ssthresh));
  Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue(initcwnd));

  // Disable delayed ack
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  Config::SetDefault("ns3::TcpNewReno::dctcp", BooleanValue(true));
  Config::SetDefault("ns3::TcpNewReno::smoother_dctcp", BooleanValue(false));
  Config::SetDefault("ns3::TcpNewReno::strawmancc", BooleanValue(strawmancc));

  Config::SetDefault("ns3::PacketSink::StartMeasurement",TimeValue(Seconds(measurement_starttime)));
  Config::SetDefault("ns3::PrioQueue::fluid_model", BooleanValue(false));
  Config::SetDefault("ns3::PrioQueue::running_min", BooleanValue(false));
  Config::SetDefault("ns3::PrioQueue::SubtractPrio", BooleanValue(false));
  Config::SetDefault("ns3::PrioQueue::ReceiverMarks", BooleanValue(false));
  Config::SetDefault("ns3::PrioQueue::MinPrioUpdateTime", TimeValue(Seconds(prio_q_min_update_time)));
  Config::SetDefault("ns3::PrioQueue::PriceUpdateTime", TimeValue(Seconds(price_update_time)));

  Config::SetDefault("ns3::PrioQueue::alpha", DoubleValue(alpha_value));
  Config::SetDefault("ns3::PrioQueue::gamma", DoubleValue(gamma_value));
  Config::SetDefault("ns3::PrioQueue::target_queue", DoubleValue(target_queue));

  Config::SetDefault("ns3::PrioQueue::m_onlydctcp", BooleanValue(onlydctcp));
  Config::SetDefault("ns3::PrioQueue::dctcp_mark", BooleanValue(dctcp_mark));


 Config::SetDefault("ns3::PrioQueue::gamma1", DoubleValue(gamma1_value));
 Config::SetDefault("ns3::PrioQueue::margin_util_price", BooleanValue(margin_util_price));
 Config::SetDefault("ns3::PrioQueue::scen2", BooleanValue(false));
 Config::SetDefault("ns3::PrioQueue::m_pkt_tag",BooleanValue(pkt_tag));
 Config::SetDefault("ns3::PrioQueue::strawmancc",BooleanValue(strawmancc));


  Config::SetDefault ("ns3::DropTailQueue::Mode" , StringValue("QUEUE_MODE_BYTES"));
  Config::SetDefault ("ns3::DropTailQueue::MaxBytes", UintegerValue(max_queue_size));
  
  Config::SetDefault ("ns3::PrioQueue::Mode", StringValue("QUEUE_MODE_BYTES"));
  Config::SetDefault ("ns3::PrioQueue::MaxBytes", UintegerValue (max_queue_size));

  Config::SetDefault ("ns3::PrioQueue::ECNThreshBytes", UintegerValue (max_ecn_thresh));
  Config::SetDefault("ns3::Ipv4L3Protocol::m_pkt_tag", BooleanValue(pkt_tag));
  Config::SetDefault("ns3::Ipv4L3Protocol::m_wfq", BooleanValue(wfq));
  Config::SetDefault("ns3::Ipv4L3Protocol::rate_based", BooleanValue(strawmancc));

  // Here, we will create N nodes in a star.
  NS_LOG_INFO ("Create nodes.");


  NodeContainer bottleneckNodes;
  NodeContainer clientNodes;
  NodeContainer extraNodes;
  bottleneckNodes.Create (2);
  clientNodes.Create (N-2);
  
  NodeContainer allNodes = NodeContainer (bottleneckNodes, clientNodes);

  // Install network stacks on the nodes
  InternetStackHelper internet;
  internet.Install (allNodes);

  //Collect an adjacency list of nodes for the p2p topology
  std::vector<NodeContainer> nodeAdjacencyList1 ((N-2)/2);
  std::vector<NodeContainer> nodeAdjacencyList2 ((N-2)/2);
  NS_LOG_UNCOND("nodeAdjacencyList size "<<nodeAdjacencyList1.size());
  for(uint32_t i=0; i<nodeAdjacencyList1.size (); ++i)
    {
      nodeAdjacencyList1[i] = NodeContainer (bottleneckNodes.Get(0), clientNodes.Get (i));
      nodeAdjacencyList2[i] = NodeContainer (bottleneckNodes.Get(1), clientNodes.Get (i+skip));
    }
  NodeContainer blink(bottleneckNodes.Get(0), bottleneckNodes.Get(1));
  

  // We create the channels first without any IP addressing information
  //
  // Queue, Channel and link characteristics
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2paccess1;
  p2paccess1.SetDeviceAttribute ("DataRate", StringValue (link_rate_string));
  p2paccess1.SetChannelAttribute ("Delay", TimeValue(MicroSeconds(link_delay)));

  PointToPointHelper p2paccess2;
  p2paccess2.SetDeviceAttribute ("DataRate", StringValue (link_rate_string));
  p2paccess2.SetChannelAttribute ("Delay", TimeValue(MicroSeconds(22.0)));

  PointToPointHelper p2pbottleneck;
  p2pbottleneck.SetDeviceAttribute ("DataRate", StringValue (link_rate_string));
  p2pbottleneck.SetChannelAttribute ("Delay", TimeValue(MicroSeconds(1.0)));

  if(queue_type == "PrioQueue") {
    p2pbottleneck.SetQueue("ns3::PrioQueue", "pFabric", StringValue("1"),"DataRate", StringValue(link_rate_string));
    p2paccess1.SetQueue("ns3::PrioQueue", "pFabric", StringValue("1"), "DataRate", StringValue(link_rate_string));
    p2paccess2.SetQueue("ns3::PrioQueue", "pFabric", StringValue("1"), "DataRate", StringValue(link_rate_string));
  }

  std::vector<NetDeviceContainer> deviceAdjacencyList1 ((N-2)/2);
  std::vector<NetDeviceContainer> deviceAdjacencyList2 ((N-2)/2);

  std::vector<Ipv4InterfaceContainer> interfaceAdjacencyList1 ((N-2)/2);
  std::vector<Ipv4InterfaceContainer> interfaceAdjacencyList2 ((N-2)/2);

  NS_LOG_UNCOND("Size of deviceAdjacencyList "<<deviceAdjacencyList1.size());
  Ipv4AddressHelper ipv4;
  uint32_t cur_subnet = 0;
  filePlotQueue << "./" << "queuesize.plotme";
  for(uint32_t i=0; i<deviceAdjacencyList1.size (); ++i)
    {
      if(i == 0) {
        deviceAdjacencyList1[i] = p2paccess1.Install (nodeAdjacencyList1[i].Get(0), nodeAdjacencyList1[i].Get(1));
      } else {
        deviceAdjacencyList1[i] = p2paccess2.Install (nodeAdjacencyList1[i].Get(0), nodeAdjacencyList1[i].Get(1));
      }
          
    /** Verify the attributes that we set **/
    Ptr<PointToPointNetDevice> ptr1(dynamic_cast<PointToPointNetDevice*>(PeekPointer(deviceAdjacencyList1[i].Get(0))));
    Ptr<PointToPointNetDevice> ptr2(dynamic_cast<PointToPointNetDevice*>(PeekPointer(deviceAdjacencyList1[i].Get(1))));
    std::cout<<"Set the access link data rate to "<<ptr1->GetDataRate()<<std::endl;
    std::cout<<"Set the access link data rate to "<<ptr2->GetDataRate()<<std::endl;

    Ptr<PointToPointChannel> chan = StaticCast<PointToPointChannel> (ptr1->GetChannel());
    std::cout<<"Set the access delay to"<<chan->GetDelayPublic()<<std::endl;

    Ptr<PointToPointChannel> chan1 = StaticCast<PointToPointChannel> (ptr2->GetChannel());
    std::cout<<"Set the access delay to"<<chan1->GetDelayPublic()<<std::endl;
    
    //Simulator::Schedule(Seconds(1.5), &change_delay, chan, chan1);

    /** assigining ip address **/

    std::ostringstream subnet;
    NS_LOG_UNCOND("Assigning subnet index "<<i+1);
    subnet<<"10.1."<<i+1<<".0";
    ipv4.SetBase (subnet.str ().c_str (), "255.255.255.0");
    interfaceAdjacencyList1[i] = ipv4.Assign (deviceAdjacencyList1[i]);
    
    if(i==0) {
      deviceAdjacencyList2[i] = p2paccess1.Install (nodeAdjacencyList2[i].Get(0), nodeAdjacencyList2[i].Get(1));
    } else {
      deviceAdjacencyList2[i] = p2paccess2.Install (nodeAdjacencyList2[i].Get(0), nodeAdjacencyList2[i].Get(1));
    }
      
    NS_LOG_UNCOND("Assigning subnet index "<<i+1);
    subnet<<"10.2."<<i+1<<".0";
    ipv4.SetBase (subnet.str ().c_str (), "255.255.255.0");
    interfaceAdjacencyList2[i] = ipv4.Assign (deviceAdjacencyList2[i]);

    NS_LOG_UNCOND("deviceAdjacencyList1 "<<i<<" adjacency list "<<(nodeAdjacencyList1[i].Get(0))->GetId()<<" "<<(nodeAdjacencyList1[i].Get(1))->GetId());
    NS_LOG_UNCOND("deviceAdjacencyList2 "<<i<<" adjacency list "<<(nodeAdjacencyList2[i].Get(0))->GetId()<<" "<<(nodeAdjacencyList2[i].Get(1))->GetId());
    cur_subnet = i;
    }

  NetDeviceContainer bdevice = p2pbottleneck.Install(blink);
  NS_LOG_UNCOND("Assigning subnet index "<<cur_subnet);
  std::ostringstream subnet;
  subnet<<"10.3."<<cur_subnet+1<<".0";
  ipv4.SetBase (subnet.str ().c_str (), "255.255.255.0");
  Ipv4InterfaceContainer bInterface = ipv4.Assign(bdevice);
  NS_LOG_UNCOND("Assigned IP address "<<bInterface.GetAddress(0)<<" "<<bInterface.GetAddress(1)<<" to binterface3");

  //Turn on global static routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Create a packet sink on the star "hub" to receive these packets
  uint16_t port = 50000;
  Address anyAddress = InetSocketAddress (Ipv4Address::GetAny (), port);

//  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", anyAddress);

  ApplicationContainer sinkApps;
  ApplicationContainer sapp1, sapp2;

  for(uint32_t i=0; i<(N-2)/2; i++) 
  {
    if(i == 0) {
      ApplicationContainer sapp1 = sinkHelper.Install (clientNodes.Get(i+skip));
      //sinkApps.Add(sinkHelper.Install (clientNodes.Get(i+skip)));
     NS_LOG_UNCOND("sink apps installed on node "<<(clientNodes.Get(i+skip))->GetId());
    } else {
      ApplicationContainer sapp2 = sinkHelper.Install (clientNodes.Get(i+skip));
      //sinkApps.Add(sinkHelper.Install (clientNodes.Get(i+skip)));
      NS_LOG_UNCOND("sink apps installed on node "<<(clientNodes.Get(i+skip))->GetId());
    }
  }

  sapp1.Start(Seconds(1.0));
  sapp1.Stop(Seconds(sim_time));

  sapp2.Start(Seconds(1.0));
  sapp2.Stop(Seconds(sim_time));

  /* Application configuration */


  ApplicationContainer app[2];
  std::vector< Ptr<Socket> > ns3TcpSockets;

  
  uint32_t flow_id = 1; 
  double start_time[2];
  double stop_time[2];

  start_time[0] = flow1_starttime;
  start_time[1] = flow2_starttime;

  stop_time[0] = flow1_stoptime;
  stop_time[1] = flow2_stoptime;


  for(uint32_t i=0; i < (N-2)/2; i++) 
  {
   
      Address remoteAddress;
      remoteAddress = (InetSocketAddress (interfaceAdjacencyList2[i].GetAddress(1), port));
      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (clientNodes.Get (i), TcpSocketFactory::GetTypeId ());
      ns3TcpSockets.push_back(ns3TcpSocket);
      Ptr<MyApp> SendingApp = CreateObject<MyApp> ();
      SendingApp->Setup (ns3TcpSocket, remoteAddress, pkt_size, 0, DataRate (link_rate_string), start_time[i], stop_time[i]);
      app[i].Add(SendingApp);
      clientNodes.Get(i)->AddApplication(SendingApp);
    
      
      Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((clientNodes.Get(i))->GetObject<Ipv4> ()); // Get Ipv4 instance of the node
      Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal();
      std::cout<<"Config: Client "<<(clientNodes.Get(i))->GetId()<<"  with interface address "<<addr<<" talking to remote server " <<interfaceAdjacencyList2[i].GetAddress(1)<<" flow_id "<<flow_id<<" remote id "<<(clientNodes.Get(i+skip)->GetId())<<std::endl;
      std::stringstream ss;
      ss <<addr<<":"<<interfaceAdjacencyList2[i].GetAddress(1)<<":"<<port;
      std::string s = ss.str(); 
      flowids[s] = flow_id;
      ipv4->setFlow(s, flow_id); 
      std::cout<<"in stop_flow : setting flow "<<s<<" to id "<<flow_id<<std::endl;
      flow_id++;
   }


  /* read the flow utils into a vector and send it to the ipv4module */
  std::vector<double> futils;
  std::ifstream infile(flow_util_file.c_str(), std::ifstream::in);
  std::string util_line;
  while (!infile.eof()) {
    infile >> util_line;
    int flowid = atoi(util_line.c_str());
    infile >> util_line;
    double weight = atof(util_line.c_str());

    futils.push_back(weight);
    NS_LOG_UNCOND("TRACKME : PUSHING "<<weight<<" "<<flowid);
  }



  uint32_t Ntrue = allNodes.GetN(); 
  for(uint32_t nid=0; nid<Ntrue; nid++)
  {
     Ptr<Ipv4> ipv4 = (allNodes.Get(nid))->GetObject<Ipv4> ();
      
     StaticCast<Ipv4L3Protocol> (ipv4)->setFlows(flowids);
     StaticCast<Ipv4L3Protocol> (ipv4)->setQueryTime(rate_update_time);
     StaticCast<Ipv4L3Protocol> (ipv4)->setAlpha(1.0);
     StaticCast<Ipv4L3Protocol> (ipv4)->setEpochUpdate(epoch_update_time);
     StaticCast<Ipv4L3Protocol> (ipv4)->setFlowUtils(futils);
      

//     StaticCast<Ipv4L3Protocol> (ipv4)->setQueryTime(0.0005);
//     StaticCast<Ipv4L3Protocol> (ipv4)->setAlpha(1.0);
  }
     


  //std::vector< Ptr<MyApp> >::iterator appIter;
  //appIter = apps.begin (); ++appIter;
  //(*appIter)->StartApplication();

  for(int i=0; i<2; i++) {
    app[i].Start (Seconds (start_time[i]));
    app[i].Stop (Seconds (stop_time[i]));
  }



  if(queue_type == "PrioQueue") {
    Ptr<PointToPointNetDevice> nd = StaticCast<PointToPointNetDevice> (bdevice.Get(0));
    Ptr<Queue> queue = nd->GetQueue ();
    StaticCast<PrioQueue> (queue)->SetNodeID(0);
    BooleanValue is_switch;
    StaticCast<PrioQueue> (queue)->SetAttribute("is_switch", BooleanValue("true"));
    Simulator::Schedule (Seconds (1.0), &CheckQueueSize, queue);
  }
  Simulator::Schedule (Seconds (1.0), &CheckIpv4Rates, allNodes);


  //configure tracing
  std::string one = ".one";
  std::string two(".two");
  std::string three(".three");

  std::string hname1 = prefix+one;
  
  std::string hname2 = prefix+two;
  std::string hname3 = prefix+three;

  NS_LOG_UNCOND("Name1 "<<hname1<<" name2 "<<hname2.c_str() <<" name3 "<<hname3.c_str());
  
  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream0 = asciiTraceHelper.CreateFileStream (hname1);
  ns3TcpSockets[0]->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream0));

  if(N > 5) {
    AsciiTraceHelper asciiTraceHelper1;
    Ptr<OutputStreamWrapper> stream1 = asciiTraceHelper1.CreateFileStream (hname2.c_str());
    ns3TcpSockets[1]->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream1));
  }

  if(N > 6) {
    AsciiTraceHelper asciiTraceHelper2;
    Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper2.CreateFileStream (hname3.c_str());
    ns3TcpSockets[2]->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream2));
  }

  /* Trace the rto in these nodes */

 /*
  std::string rname1 = prefix+one+".rto";
  std::string rname2 = prefix+two+".rto";
  std::string rname3 = prefix+three+".rto";


  AsciiTraceHelper rtoHelper;
  Ptr<OutputStreamWrapper> rtostream = rtoHelper.CreateFileStream (rname1);
  ns3TcpSockets[0]->TraceConnectWithoutContext ("RTO", MakeBoundCallback (&RTOChange, rtostream));


  if(N > 4) {
    AsciiTraceHelper rtoHelper1;
    Ptr<OutputStreamWrapper> rtostream1 = rtoHelper1.CreateFileStream (rname2);
    ns3TcpSockets[1]->TraceConnectWithoutContext ("RTO", MakeBoundCallback (&RTOChange, rtostream1));
  }

  if(N > 6) {
    AsciiTraceHelper rtoHelper2;
    Ptr<OutputStreamWrapper> rtostream2 = rtoHelper2.CreateFileStream (rname3);
    ns3TcpSockets[2]->TraceConnectWithoutContext ("RTO", MakeBoundCallback (&RTOChange, rtostream2));
  }
*/
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  
  ApplicationContainer::Iterator sapp;
  uint32_t total_bytes = 0;

  double bytes1, bytes2;
  int count = 0;
  for (sapp = sinkApps.Begin (); sapp != sinkApps.End (); ++sapp)
  {
    Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (*sapp);
    NS_LOG_UNCOND("Total Bytes Received: " << sink1->GetTotalRx ());
    if(count == 0) {
      bytes1 = sink1->GetTotalRx();
      count++;
    } else {
      bytes2 = sink1->GetTotalRx();
    }
    total_bytes += sink1->GetTotalRx();
  }

  std::cout<<"Utilization : "<<total_bytes*8.0/(1000000*(sim_time-measurement_starttime))<<" Mbps"<<" ratio "<<bytes2/bytes1<<std::endl;

  return 0;
}
