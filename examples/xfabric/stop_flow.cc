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
#include "declarations.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("pfabric");

std::stringstream filePlotQueue;

double flow1_stoptime=1.3;
double flow2_stoptime=1.3;
double flow3_stoptime=1.3;

double flow1_starttime=1.0;
double flow2_starttime=1.0;
double flow3_starttime=1.0;



//float sampling_interval = 0.0001;
//uint32_t pkt_size = 1040;
//uint32_t max_ecn_thresh = 0;
//uint32_t max_queue_size = 450000000;
//uint32_t max_segment_size = 1402;
//uint32_t ssthresh_value = 3000;
//std::map<std::string, uint32_t> flowids;
//uint32_t recv_buf_size = 1310720;
//uint32_t send_buf_size = 1310720;
//double link_delay = 7.0;
//double link_delay = 7.5e-6;
//double link_rate = 1000000000; //1G
//std::string link_rate_string = "1Gbps";
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
/*  m_socket->Send (packet);
  m_socket->Send (packet);
  m_socket->Send (packet);
  m_socket->Send (packet);
*/  

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

void
CheckQueueSize (Ptr<Queue> queue)
{
  uint32_t qSize = StaticCast<PrioQueue> (queue)->GetCurSize ();
  uint32_t nid = StaticCast<PrioQueue> (queue)->nodeid;
  double qPrice = StaticCast<PrioQueue> (queue)->getCurrentPrice ();
  
  std::cout<<"QueueStats "<<nid<<" "<<Simulator::Now ().GetSeconds () << " " << qSize<<" 1.0 "<<qPrice<<std::endl;
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
    
      //double rate = ipv4->GetStoreRate (it->first);
      //double prio = ipv4->GetStorePrio (it->first);
//      double destrate = ipv4->GetStoreDestRate (it->first);
      double destrate = ipv4->GetCSFQRate (it->first);
      double srate = ipv4->GetShortRate (it->first);
      
/*      if(((it->second == nid-2) && (it->second != 3)) || (it->second == 3 && nid == 8)) {
        std::cout<<"RatePrio flowid "<<it->second<<" "<<Simulator::Now ().GetSeconds () << " " << rate << " "<<prio<<" "<<flow_price<<" "<<increments<<std::endl;
      } */
      //if(((it->second == 1) && (nid == 5)) || ((it->second == 2) && (nid == 7)) || (it->second == 3 && nid == 6)) {
      if(((it->second == 1) && (nid == 3)) || ((it->second == 2) && (nid == 4)) || (it->second == 3 && nid == 8)) {
        std::cout<<"DestRate flowid "<<it->second<<" "<<Simulator::Now ().GetSeconds () << " " << destrate<<" "<<srate<<std::endl;
      }

    }
  }
  
  // check queue size every 1/1000 of a second
  Simulator::Schedule (Seconds (sampling_interval), &CheckIpv4Rates, allNodes);
  if(Simulator::Now().GetSeconds() >= sim_time) {
    Simulator::Stop();
  }
}

void config_queue(Ptr<Queue> Q, uint32_t nid, uint32_t vpackets, std::string fkey1)
{
      Q->SetNodeID(nid);
      Q->SetLinkIDString(fkey1);
      Q->SetVPkts(vpackets);
}
int 
main (int argc, char *argv[])
{

  // Set up some default values for the simulation.
  //Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("5000kb/s"));
  uint32_t N = 7; //number of nodes in the star
  uint32_t skip = 2;
  std::string prefix;
  std::string queue_type;
  double epoch_update_time = 0.001;
  std::string flow_util_file;
  bool pkt_tag = true, onlydctcp, wfq;
  bool strawmancc = false;
  
  dctcp = true; 

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
  cmd.AddValue ("load", "load",load);
  cmd.AddValue ("controller_estimated_unknown_load", "controller_estimated_unknown_load",controller_estimated_unknown_load);
  cmd.AddValue ("sampling_interval", "sampling_interval", sampling_interval);
  cmd.AddValue ("kay", "kay", kvalue);
  cmd.AddValue ("vpackets", "vpackets", vpackets);
  cmd.AddValue ("xfabric", "xfabric", xfabric);
  cmd.AddValue ("dctcp", "dctcp", dctcp);
  cmd.AddValue ("hostflows", "hostflows",flows_per_host);
  cmd.AddValue ("flows_tcp", "flows_tcp", flows_tcp);
  cmd.AddValue ("weight_change", "weight_change", weight_change);
  cmd.AddValue ("weight_norm", "weight_norm", weight_normalized);
  cmd.AddValue ("rate_based", "rate_based", rate_based);
  cmd.AddValue ("UNKNOWN_FLOW_SIZE_CUTOFF", "unknown_flow_size_cutoff", UNKNOWN_FLOW_SIZE_CUTOFF);
  cmd.AddValue ("scheduler_mode_edf", "scheduler_mode_edf", scheduler_mode_edf);
  cmd.AddValue ("deadline_mode", "deadline_mode", deadline_mode);
  cmd.AddValue ("deadline_mean", "deadline_mean", deadline_mean);
  cmd.AddValue ("host_compensate", "host_compensate", host_compensate);


  cmd.Parse (argc, argv);

  if(link_rate == 10000000000) {
    link_rate_string = "10Gbps";
  } else if(link_rate == 100000000000) {
    link_rate_string = "100Gbps";
  }


  NS_LOG_UNCOND("file prefix "<<prefix);

  double total_rtt = link_delay * 6.0;
  uint32_t bdproduct = link_rate *total_rtt/(1000000.0 * 8.0);
  //uint32_t bdproduct = link_rate *total_rtt/(8.0);
  uint32_t initcwnd = (bdproduct / max_segment_size)+1;
  uint32_t ssthresh = initcwnd * max_segment_size;

  std::cout<<"initcwnd "<<initcwnd<<" ssthresh "<<ssthresh<<std::endl;

  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue(max_segment_size));
//  Config::SetDefault ("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue(ssthresh_value));
  Config::SetDefault ("ns3::TcpSocketBase::Timestamp", BooleanValue(false));
  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (recv_buf_size));
  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (send_buf_size));

  Config::SetDefault ("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue(ssthresh));
  Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue(initcwnd));

  // Disable delayed ack
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  Config::SetDefault("ns3::TcpNewReno::dctcp", BooleanValue(false));
  Config::SetDefault("ns3::TcpNewReno::xfabric", BooleanValue(true));

  Config::SetDefault("ns3::PacketSink::StartMeasurement",TimeValue(Seconds(measurement_starttime)));
  Config::SetDefault("ns3::PrioQueue::PriceUpdateTime", TimeValue(Seconds(price_update_time)));

  Config::SetDefault("ns3::PrioQueue::m_pkt_tag", BooleanValue(pkt_tag));
 Config::SetDefault("ns3::PrioQueue::m_pkt_tag",BooleanValue(pkt_tag));


  Config::SetDefault ("ns3::DropTailQueue::Mode" , StringValue("QUEUE_MODE_BYTES"));
  Config::SetDefault ("ns3::DropTailQueue::MaxBytes", UintegerValue(max_queue_size));
  
  Config::SetDefault ("ns3::PrioQueue::Mode", StringValue("QUEUE_MODE_BYTES"));
  Config::SetDefault ("ns3::PrioQueue::MaxBytes", UintegerValue (max_queue_size));

  Config::SetDefault ("ns3::PrioQueue::ECNThreshBytes", UintegerValue (max_ecn_thresh));
  Config::SetDefault("ns3::Ipv4L3Protocol::m_pkt_tag", BooleanValue(pkt_tag));
  Config::SetDefault("ns3::Ipv4L3Protocol::m_wfq", BooleanValue(false));

  Config::SetDefault ("ns3::PrioQueue::host_compensate", BooleanValue(host_compensate));
  Config::SetDefault("ns3::Ipv4L3Protocol::host_compensate", BooleanValue(host_compensate));
  // Here, we will create N nodes in a star.
  NS_LOG_INFO ("Create nodes.");



  NodeContainer bottleneckNodes;
  NodeContainer clientNodes;
  NodeContainer extraNodes;
  bottleneckNodes.Create (3);
  clientNodes.Create (N-3);
  extraNodes.Create(2);
  
  NodeContainer allNodes = NodeContainer (bottleneckNodes, clientNodes, extraNodes);

  // Install network stacks on the nodes
  InternetStackHelper internet;
  internet.Install (allNodes);

  //Collect an adjacency list of nodes for the p2p topology
  std::vector<NodeContainer> nodeAdjacencyList1 ((N-3)/2);
  std::vector<NodeContainer> nodeAdjacencyList2 ((N-3)/2);
  NS_LOG_UNCOND("nodeAdjacencyList size "<<nodeAdjacencyList1.size());
  for(uint32_t i=0; i<nodeAdjacencyList1.size (); ++i)
    {
      nodeAdjacencyList1[i] = NodeContainer (bottleneckNodes.Get(0), clientNodes.Get (i));
      nodeAdjacencyList2[i] = NodeContainer (bottleneckNodes.Get(2), clientNodes.Get (i+skip));
    }
  NodeContainer blink1(bottleneckNodes.Get(0), bottleneckNodes.Get(1));
  NodeContainer blink2(bottleneckNodes.Get(1), bottleneckNodes.Get(2));
  
  NodeContainer blink3(bottleneckNodes.Get(1),extraNodes.Get(0));
  NodeContainer blink4(bottleneckNodes.Get(1),extraNodes.Get(1));


  // We create the channels first without any IP addressing information
  //
  // Queue, Channel and link characteristics
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2paccess;
  p2paccess.SetDeviceAttribute ("DataRate", StringValue (link_rate_string));
  p2paccess.SetChannelAttribute ("Delay", TimeValue(MicroSeconds(link_delay)));

  PointToPointHelper p2pbottleneck;
  p2pbottleneck.SetDeviceAttribute ("DataRate", StringValue (link_rate_string));
  p2pbottleneck.SetChannelAttribute ("Delay", TimeValue(MicroSeconds(0.5)));

//  if(queue_type == "PrioQueue") {
    p2pbottleneck.SetQueue("ns3::PrioQueue", "pFabric", StringValue("1"),"DataRate", StringValue(link_rate_string));
    p2paccess.SetQueue("ns3::PrioQueue", "pFabric", StringValue("1"), "DataRate", StringValue(link_rate_string));
    //p2pbottleneck.SetQueue("ns3::PrioQueue", "pFabric", StringValue("1"),"DataRate", StringValue(link_speed));
    //p2paccess.SetQueue("ns3::PrioQueue", "pFabric", StringValue("1"), "DataRate", StringValue("20Gbps"));
//  }

  std::vector<NetDeviceContainer> deviceAdjacencyList1 ((N-3)/2);
  std::vector<NetDeviceContainer> deviceAdjacencyList2 ((N-3)/2);

  std::vector<Ipv4InterfaceContainer> interfaceAdjacencyList1 ((N-3)/2);
  std::vector<Ipv4InterfaceContainer> interfaceAdjacencyList2 ((N-3)/2);

  NS_LOG_UNCOND("Size of deviceAdjacencyList "<<deviceAdjacencyList1.size());
  Ipv4AddressHelper ipv4;
  uint32_t cur_subnet = 0;
  filePlotQueue << "./" << "queuesize.plotme";
  for(uint32_t i=0; i<deviceAdjacencyList1.size (); ++i)
    {
      deviceAdjacencyList1[i] = p2paccess.Install (nodeAdjacencyList1[i].Get(0), nodeAdjacencyList1[i].Get(1));
    /** Verify the attributes that we set **/
    Ptr<PointToPointNetDevice> ptr1(dynamic_cast<PointToPointNetDevice*>(PeekPointer(deviceAdjacencyList1[i].Get(0))));
    Ptr<PointToPointNetDevice> ptr2(dynamic_cast<PointToPointNetDevice*>(PeekPointer(deviceAdjacencyList1[i].Get(1))));
    NS_LOG_UNCOND("Set the access link data rate to "<<ptr1->GetDataRate());
    NS_LOG_UNCOND("Set the access link data rate to "<<ptr2->GetDataRate());

    Ptr<PointToPointChannel> chan = StaticCast<PointToPointChannel> (ptr2->GetChannel());
    std::cout<<"Set the access delay to"<<chan->GetDelayPublic()<<std::endl;

    /** assigining ip address **/

    std::ostringstream subnet;
    NS_LOG_UNCOND("Assigning subnet index "<<i+1);
    subnet<<"10.1."<<i+1<<".0";
    ipv4.SetBase (subnet.str ().c_str (), "255.255.255.0");
    interfaceAdjacencyList1[i] = ipv4.Assign (deviceAdjacencyList1[i]);

    deviceAdjacencyList2[i] = p2paccess.Install (nodeAdjacencyList2[i].Get(0), nodeAdjacencyList2[i].Get(1));
    NS_LOG_UNCOND("Assigning subnet index "<<i+1);
    subnet<<"10.2."<<i+1<<".0";
    ipv4.SetBase (subnet.str ().c_str (), "255.255.255.0");
    interfaceAdjacencyList2[i] = ipv4.Assign (deviceAdjacencyList2[i]);

    NS_LOG_UNCOND("deviceAdjacencyList1 "<<i<<" adjacency list "<<(nodeAdjacencyList1[i].Get(0))->GetId()<<" "<<(nodeAdjacencyList1[i].Get(1))->GetId());
    NS_LOG_UNCOND("deviceAdjacencyList2 "<<i<<" adjacency list "<<(nodeAdjacencyList2[i].Get(0))->GetId()<<" "<<(nodeAdjacencyList2[i].Get(1))->GetId());
    cur_subnet = i;
    }

  NetDeviceContainer bdevice1 = p2pbottleneck.Install(blink1);
  NetDeviceContainer bdevice2 = p2pbottleneck.Install(blink2);
  NetDeviceContainer bdevice3 = p2paccess.Install(blink3);
  NetDeviceContainer bdevice4 = p2paccess.Install(blink4);
  NS_LOG_UNCOND("Assigning subnet index "<<cur_subnet);
  std::ostringstream subnet;
  subnet<<"10.3."<<cur_subnet+1<<".0";
  ipv4.SetBase (subnet.str ().c_str (), "255.255.255.0");
  Ipv4InterfaceContainer bInterface3 = ipv4.Assign(bdevice3);
  NS_LOG_UNCOND("Assigned IP address "<<bInterface3.GetAddress(0)<<" "<<bInterface3.GetAddress(1)<<" to binterface3");
  subnet<<"10.4."<<cur_subnet+1<<".0";
  ipv4.SetBase (subnet.str ().c_str (), "255.255.255.0");
  Ipv4InterfaceContainer bInterface4 = ipv4.Assign(bdevice4);
  NS_LOG_UNCOND("Assigned IP address "<<bInterface4.GetAddress(0)<<" "<<bInterface4.GetAddress(1)<<" to binterface4");


  Ptr<PointToPointNetDevice> ptr3(dynamic_cast<PointToPointNetDevice*>(PeekPointer(bdevice1.Get(1))));
  NS_LOG_UNCOND("Set the bottleneck link 1 data rate to "<<ptr3->GetDataRate());
  
  Ptr<PointToPointNetDevice> ptr4(dynamic_cast<PointToPointNetDevice*>(PeekPointer(bdevice2.Get(1))));
  NS_LOG_UNCOND("Set the bottleneck link 2 data rate to "<<ptr4->GetDataRate());

  Ptr<PointToPointChannel> chan = StaticCast<PointToPointChannel> (ptr3->GetChannel());
  NS_LOG_UNCOND("Set the bottleneck delay to"<<chan->GetDelayPublic());

  subnet<<"10.5.1.0";
  ipv4.SetBase (subnet.str ().c_str (), "255.255.255.0");
  Ipv4InterfaceContainer bInterface1 = ipv4.Assign(bdevice1);
  Ipv4InterfaceContainer bInterface2 = ipv4.Assign(bdevice2);
  

  //Turn on global static routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Create a packet sink on the star "hub" to receive these packets
  uint16_t port = 50000;
  Address anyAddress = InetSocketAddress (Ipv4Address::GetAny (), port);

//  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", anyAddress);

  ApplicationContainer sinkApps;
  ApplicationContainer sapp2, sapp3;

  for(uint32_t i=0; i<(N-3)/2; i++) 
  {
    if(i == 0) {
      ApplicationContainer sapp2 = sinkHelper.Install (clientNodes.Get(i+skip));
      //sinkApps.Add(sinkHelper.Install (clientNodes.Get(i+skip)));
     NS_LOG_UNCOND("sink apps installed on node "<<(clientNodes.Get(i+skip))->GetId());
    } else {
      ApplicationContainer sapp3 = sinkHelper.Install (clientNodes.Get(i+skip));
      //sinkApps.Add(sinkHelper.Install (clientNodes.Get(i+skip)));
      NS_LOG_UNCOND("sink apps installed on node "<<(clientNodes.Get(i+skip))->GetId());
    }
  }

  // Add a sink to the extraNode
  //sinkApps.Add(sinkHelper.Install (extraNodes.Get(0)));
  ApplicationContainer sapp1 = sinkHelper.Install (extraNodes.Get(0));
  Ptr<Ipv4> ip = (extraNodes.Get(0))->GetObject<Ipv4> (); // Get Ipv4 instance of the node
  Ipv4Address addr1 = ip->GetAddress (1, 0).GetLocal();
  NS_LOG_UNCOND("sink apps installed on node "<<(extraNodes.Get(0))->GetId()<<" at addr "<<addr1);
  sapp1.Start(Seconds(1.0));
  sapp1.Stop(Seconds(sim_time));

  sapp2.Start(Seconds(1.0));
  sapp2.Stop(Seconds(sim_time));
  sapp3.Start(Seconds(1.0));
  sapp3.Stop(Seconds(sim_time));



 // sinkApps.Start (Seconds (1.0));
 // sinkApps.Stop (Seconds (sim_time));

  
  /* Application configuration */



  ApplicationContainer app1, app2, app3;
  std::vector< Ptr<Socket> > ns3TcpSockets;

  
  uint32_t flow_id = 1; 


  for(uint32_t i=0; i < (N-3)/2; i++) 
  {
   
      Address remoteAddress;
      if(i == 1) { 
        remoteAddress =  (InetSocketAddress (bInterface3.GetAddress(1), port));
      } else {
        remoteAddress = (InetSocketAddress (interfaceAdjacencyList2[i].GetAddress(1), port));
      }
      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (clientNodes.Get (i), TcpSocketFactory::GetTypeId ());
      ns3TcpSockets.push_back(ns3TcpSocket);
      Ptr<MyApp> SendingApp = CreateObject<MyApp> ();
      double stop_time = sim_time;
      if(i == 1) {
        stop_time = flow2_stoptime;
        SendingApp->Setup (ns3TcpSocket, remoteAddress, pkt_size, 0, DataRate (link_rate_string), flow2_starttime, flow2_stoptime);
        app2.Add(SendingApp);
      } else {
        stop_time = flow1_stoptime;
        SendingApp->Setup (ns3TcpSocket, remoteAddress, pkt_size, 0, DataRate (link_rate_string), flow1_starttime, flow1_stoptime);
        app1.Add(SendingApp);
      }
        
      clientNodes.Get(i)->AddApplication(SendingApp);
    
      
      Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((clientNodes.Get(i))->GetObject<Ipv4> ()); // Get Ipv4 instance of the node
      Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal();


      if(i == 1) {
        std::cout<<"Config: Client "<<(clientNodes.Get(i))->GetId()<<"  with interface address "<<addr<<" talking to remote server " <<bInterface3.GetAddress(1)<<" flow_id "<<flow_id<<" stopping at "<<stop_time<<" remote id "<<extraNodes.Get(0)->GetId()<<std::endl;
        std::stringstream ss;
        ss <<addr<<":"<<bInterface3.GetAddress(1)<<":"<<port;
        std::string s = ss.str(); 
        flowids[s] = flow_id;
        ipv4->setFlow(s, flow_id); 
        std::cout<<"in stop_flow : setting flow "<<s<<" to id "<<flow_id<<std::endl;
      } else {
        std::cout<<"Config: Client "<<(clientNodes.Get(i))->GetId()<<"  with interface address "<<addr<<" talking to remote server " <<interfaceAdjacencyList2[i].GetAddress(1)<<" flow_id "<<flow_id<<" remote id "<<(clientNodes.Get(2)->GetId())<<std::endl;
        std::stringstream ss;
        ss <<addr<<":"<<interfaceAdjacencyList2[i].GetAddress(1)<<":"<<port;
        std::string s = ss.str(); 
        flowids[s] = flow_id;
        ipv4->setFlow(s, flow_id, 0.0, 1.0); 
        std::cout<<"in stop_flow : setting flow "<<s<<" to id "<<flow_id<<" weight 1.0"<<std::endl;
      }

      
      flow_id++;
      
   }

    /* One more manual setup */
    Address remoteAddress;
    remoteAddress = (InetSocketAddress (interfaceAdjacencyList2[1].GetAddress(1), port));
    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (extraNodes.Get (1), TcpSocketFactory::GetTypeId ());
    ns3TcpSockets.push_back(ns3TcpSocket);
    Ptr<MyApp> SendingApp = CreateObject<MyApp> ();
    SendingApp->Setup (ns3TcpSocket, remoteAddress, pkt_size, 0, DataRate (link_rate_string), flow3_starttime, flow3_stoptime);
    app3.Add(SendingApp);
    extraNodes.Get(1)->AddApplication(SendingApp);
      
    Ptr<Ipv4L3Protocol> ipv4_1 = StaticCast<Ipv4L3Protocol> ((extraNodes.Get(1))->GetObject<Ipv4> ()); // Get Ipv4 instance of the node
    Ipv4Address addr = ipv4_1->GetAddress (1, 0).GetLocal();

    NS_LOG_UNCOND("Config: Client "<<(extraNodes.Get(1))->GetId()<<"  with interface address "<<addr<<" talking to remote server " <<interfaceAdjacencyList2[1].GetAddress(1)<<" flow_id "<<flow_id);

      std::stringstream ss;
      ss <<addr<<":"<<interfaceAdjacencyList2[1].GetAddress(1)<<":"<<port;
      std::string s = ss.str(); 
      
      flowids[s] = flow_id;
      ipv4_1->setFlow(s, flow_id, 0.0, 1.0); 
      std::cout<<"in stop_flow : setting flow "<<s<<" to id "<<flow_id<<std::endl;

  /* Manual setup end */


  uint32_t Ntrue = allNodes.GetN(); 
  for(uint32_t nid=0; nid<Ntrue; nid++)
  {
     Ptr<Ipv4> ipv4 = (allNodes.Get(nid))->GetObject<Ipv4> ();
      
     StaticCast<Ipv4L3Protocol> (ipv4)->setFlows(flowids);
     StaticCast<Ipv4L3Protocol> (ipv4)->setQueryTime(rate_update_time);
     StaticCast<Ipv4L3Protocol> (ipv4)->setAlpha(1.0);
     StaticCast<Ipv4L3Protocol> (ipv4)->setEpochUpdate(epoch_update_time);
     StaticCast<Ipv4L3Protocol> (ipv4)->setKay(kvalue);

//     StaticCast<Ipv4L3Protocol> (ipv4)->setQueryTime(0.0005);
//     StaticCast<Ipv4L3Protocol> (ipv4)->setAlpha(1.0);
  }
     


  //std::vector< Ptr<MyApp> >::iterator appIter;
  //appIter = apps.begin (); ++appIter;
  //(*appIter)->StartApplication();

  app1.Start (Seconds (flow1_starttime));
  app1.Stop (Seconds (flow1_stoptime));

  app2.Start (Seconds (flow2_starttime));
  app2.Stop (Seconds (flow2_stoptime));

  app3.Start (Seconds (flow3_starttime));
  app3.Stop (Seconds (flow3_stoptime));



  Ptr<PointToPointNetDevice> nd2 = StaticCast<PointToPointNetDevice> (deviceAdjacencyList1[0].Get(1));
  Ptr<Queue> queue2 = nd2->GetQueue ();
  uint32_t nid = (nd2->GetNode())->GetId();
  StaticCast<PrioQueue> (queue2)->SetNodeID(nid);


  Simulator::Schedule (Seconds (1.0), &CheckQueueSize, queue2);
  

  Ptr<PointToPointNetDevice> nd = StaticCast<PointToPointNetDevice> (bdevice1.Get(0));
  Ptr<Queue> queue = nd->GetQueue ();
  nid = (nd->GetNode())->GetId();
  StaticCast<PrioQueue> (queue)->SetNodeID(nid);

  Ptr<PointToPointNetDevice> nd1 = StaticCast<PointToPointNetDevice> (bdevice2.Get(0));
  Ptr<Queue> queue1 = nd1->GetQueue ();
  nid = (nd1->GetNode())->GetId();
  StaticCast<PrioQueue> (queue1)->SetNodeID(nid);

  Ptr<PointToPointNetDevice> nd4 = StaticCast<PointToPointNetDevice> (bdevice1.Get(1));
  Ptr<Queue> queue4 = nd4->GetQueue ();
  nid = (nd4->GetNode())->GetId();
  StaticCast<PrioQueue> (queue4)->SetNodeID(nid);

  Ptr<PointToPointNetDevice> nd3 = StaticCast<PointToPointNetDevice> (bdevice2.Get(1));
  Ptr<Queue> queue3 = nd3->GetQueue ();
  nid = (nd3->GetNode())->GetId();
  StaticCast<PrioQueue> (queue3)->SetNodeID(nid);


  for(uint32_t i=0; i<deviceAdjacencyList1.size (); ++i)
  {
    Ptr<PointToPointNetDevice> nd = StaticCast<PointToPointNetDevice> (deviceAdjacencyList1[i].Get(0));
    Ptr<Queue> queue = nd->GetQueue ();
    uint32_t nid = (nd->GetNode())->GetId();
    StaticCast<PrioQueue> (queue)->SetNodeID(nid);
    Ptr<PointToPointNetDevice> nd1 = StaticCast<PointToPointNetDevice> (deviceAdjacencyList1[i].Get(1));
    Ptr<Queue> queue1 = nd1->GetQueue ();
    uint32_t nid1 = (nd1->GetNode())->GetId();
    StaticCast<PrioQueue> (queue1)->SetNodeID(nid1);
    //queue_id++;

    nd = StaticCast<PointToPointNetDevice> (deviceAdjacencyList2[i].Get(0));
    queue = nd->GetQueue ();
    nid = (nd->GetNode())->GetId();
    NS_LOG_UNCOND("Setting node id "<<nid<<" in prioqueue");
    StaticCast<PrioQueue> (queue)->SetNodeID(nid);

    // other end
    nd1 = StaticCast<PointToPointNetDevice> (deviceAdjacencyList2[i].Get(1));
    queue1 = nd1->GetQueue ();
    nid1 = (nd1->GetNode())->GetId();
    StaticCast<PrioQueue> (queue1)->SetNodeID(nid1);
    //queue_id++;
  }


  Simulator::Schedule (Seconds (1.0), &CheckQueueSize, queue);
  Simulator::Schedule (Seconds (1.0), &CheckQueueSize, queue1);
  Simulator::Schedule (Seconds (1.0), &CheckIpv4Rates, allNodes);


  //uint32_t qSize = StaticCast<PrioQueue> (queue)->GetCurCount ();
  //NS_LOG_UNCOND("max queue size is "<<qSize);
  

  //configure tracing
  std::string one = ".cwnd.1";
  std::string two(".cwnd.2");
  std::string three(".cwnd.3");

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

  NS_LOG_UNCOND("Utilization : "<<total_bytes*8.0/(1000000*(sim_time-measurement_starttime))<<" Mbps"<<" ratio "<<bytes2/bytes1);

  return 0;
}
