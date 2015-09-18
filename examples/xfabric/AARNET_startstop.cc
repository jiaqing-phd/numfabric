/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


// Default Network topology, 9 nodes in a star
/*
          n2 n3 n4
           \ | /
            \|/
       n1---n0----n9---n5
            /| \
           / | \
          n8 n7 n6
*/
// - CBR Traffic goes from the star "arms" to the "hub"




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
#include <ns3/node.h>

#include "declarations.h"

using namespace ns3;
class MyApp;

NS_LOG_COMPONENT_DEFINE ("pfabric");

std::map<uint32_t, uint16_t> flow_dest_port;
uint16_t port = 5000;
const uint32_t max_flows=8;
uint32_t flow_started[max_flows] = {0};
Ptr<MyApp> sending_apps[max_flows];
uint32_t global_flowid = 1;
uint32_t num_flows = 0;
uint32_t min_flows=3, max_flows_allowed=7;

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
//  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

double SOME_LARGE_VALUE = 12500000000000;


/* Application configuration */
//ApplicationContainer apps;

class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  //void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate, uint32_t maxbytes, double flow_start, Address address1);
  void Setup (Address address, uint32_t packetSize, DataRate dataRate, uint32_t maxbytes, double flow_start, Address address1, Ptr<Node> pnode, uint32_t fid, Ptr<Node> dnode, uint32_t w);
  void Setup();
  virtual void StartApplication (void);
  virtual void StopApplication (void);
    
private:

  void ScheduleTx (void);
  void SendPacket (void);


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
  uint32_t        m_weight;
};

std::vector< Ptr<MyApp> > apps;
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

void MyApp::Setup()
{
  StartApplication();
/*  NS_LOG_UNCOND("Scheduling start of flow "<<fid<<" at time "<<Time(tNext).GetSeconds());
  m_startEvent = Simulator::ScheduleNow ( &MyApp::StartApplication, this);*/
  std::cout<<"flow_start "<<m_fid<<" "<<srcNode->GetId()<<" "<<destNode->GetId()<<" at "<<(Simulator::Now()).GetNanoSeconds()<<" "<<m_maxBytes<<" port "<< InetSocketAddress::ConvertFrom (m_peer).GetPort () <<" weight "<<m_weight<<std::endl;
}
  
void
//MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate, Ptr<RandomVariableStream> interArrival)
//MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate, uint32_t maxBytes, double start_time, Address ownaddress, Ptr<Node> sNode)
MyApp::Setup (Address address, uint32_t packetSize, DataRate dataRate, uint32_t maxBytes, double start_time, Address ownaddress, Ptr<Node> sNode, uint32_t fid, Ptr<Node> dNode, uint32_t weight)
{
  //m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_dataRate = dataRate;
  m_totBytes = 0;
  m_maxBytes = maxBytes;
  m_startTime = start_time;
  m_fid = fid;
  m_weight = weight;
  
  Time tNext = Time(Seconds(m_startTime));
  myAddress = ownaddress;
  srcNode = sNode;
  destNode = dNode;

  StartApplication();
/*  NS_LOG_UNCOND("Scheduling start of flow "<<fid<<" at time "<<Time(tNext).GetSeconds());
  m_startEvent = Simulator::ScheduleNow ( &MyApp::StartApplication, this);*/
  std::cout<<"flow_start "<<m_fid<<" "<<srcNode->GetId()<<" "<<destNode->GetId()<<" at "<<(Simulator::Now()).GetNanoSeconds()<<" "<<m_maxBytes<<" port "<< InetSocketAddress::ConvertFrom (m_peer).GetPort () <<" weight "<<m_weight<<std::endl;
}

void setuptracing(uint32_t sindex, Ptr<Socket> skt)
{
  
    //configure tracing
    std::string one = ".cwnd";
    std::stringstream ss;
    ss << "."<<sindex;
    std::string str = ss.str();
    std::string hname1 = prefix+one+str;
    std::cout<<"cwnd output in "<<hname1<<std::endl;
   
    AsciiTraceHelper asciiTraceHelper;
    Ptr<OutputStreamWrapper> stream0 = asciiTraceHelper.CreateFileStream (hname1);
    skt->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream0));
  
}

void
MyApp::StartApplication (void)
{
/*
  if(Simulator::Now().GetNanoSeconds() < Time(Seconds(m_startTime)).GetNanoSeconds()) {
//    std::cout<<"Time "<<Simulator::Now().GetNanoSeconds()<<" spurious call flowid "<<m_fid<<" returning before start_time "<<  Time(Seconds(m_startTime)).GetNanoSeconds()<<std::endl;
    if(Simulator::IsExpired(m_startEvent)) {
      Time tNext = Time(Seconds(m_startTime));
      m_startEvent = Simulator::Schedule (tNext, &MyApp::StartApplication, this);
//      std::cout<<"Time "<<Simulator::Now().GetSeconds()<<" spurious call flowid "<<m_fid<<" rescheduling at  "<<tNext.GetSeconds()<<std::endl;
      
    }
      
    return;

  } */

  m_running = true;
  m_packetsSent = 0;
  m_totBytes = 0;

  if(m_socket == 0) {
    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (srcNode, TcpSocketFactory::GetTypeId ());
    ns3TcpSockets.push_back(ns3TcpSocket);
    m_socket = ns3TcpSocket;
    setuptracing(m_fid, m_socket);
    NS_LOG_UNCOND("number of sockets at node "<<srcNode->GetId()<<" = "<<ns3TcpSockets.size());
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
  }
//  Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> (srcNode->GetObject<Ipv4> ());
//  ipv4->removeFromDropList(m_fid);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

/*  if (m_socket)
    {
      m_socket->Close ();
    }
*/
  std::cout<<"flow_stop "<<m_fid<<" "<<srcNode->GetId()<<" "<<destNode->GetId()<<" at "<<(Simulator::Now()).GetNanoSeconds()<<" "<<m_maxBytes<<" port "<< InetSocketAddress::ConvertFrom (m_peer).GetPort () <<" weight "<<m_weight<<std::endl;
//  Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> (srcNode->GetObject<Ipv4> ());
//  ipv4->addToDropList(m_fid);
}

void
MyApp::SendPacket (void)
{
  
  Ptr<Packet> packet = Create<Packet> (m_packetSize);

  int ret_val = m_socket->Send( packet ); 
  
  if(ret_val != -1) {
    m_totBytes += packet->GetSize();
  } else {
 //   NS_LOG_UNCOND(Simulator::Now().GetSeconds()<<" flowid "<<m_fid<<" Tcp buffer is overflowing.. trying later");
  }

//  NS_LOG_UNCOND(Simulator::Now().GetSeconds()<<" flowid "<<m_fid<<" bytes sent "<<m_totBytes<<" maxBytes "<<m_maxBytes);

  //if (++m_packetsSent < m_nPackets)
  //if (m_totBytes < m_maxBytes)
  //  {
      ScheduleTx ();
   // }
  
}

void
MyApp::ScheduleTx (void)
{
  if (m_running) {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
  } else {
      StopApplication();
  }
}



void
CheckQueueSize (Ptr<Queue> queue)
{
  uint32_t qSize = StaticCast<PrioQueue> (queue)->GetCurSize ();
//  uint32_t nid = StaticCast<PrioQueue> (queue)->linkid;
  double qPrice = StaticCast<PrioQueue> (queue)->getCurrentPrice ();

  std::cout<<"QueueStats "<<StaticCast<PrioQueue> (queue)->GetLinkIDString()<<" "<<Simulator::Now ().GetSeconds () << " " << qSize<<" 1.0 "<<qPrice<<std::endl;

  // check queue size every 1/1000 of a second
  Simulator::Schedule (Seconds (sampling_interval), &CheckQueueSize, queue);
  if(Simulator::Now().GetSeconds() >= sim_time) {
    Simulator::Stop();
  }
}

uint32_t getLinkid(uint32_t n1, uint32_t n2)
{

  //if(n1 == 0 && n2 == 1) {
  //  return 0;
  //}
  //
  //if(n1 == 1 && n2 == 2) {
  //  return 2;
  //}

  //if(n1 == 1 && n2 == 3) {
  //  return 1;
  //}
  //if(n1 == 3 && n2 == 4) {
  //  return 3;
  //}
  //if(n1 == 4 && n2 == 5) {
  //  return 5;
  //}
  //if(n1 == 3 && n2 == 5) {
  //  return 4;
  //}
 
  //return 1000*n1 + n2; 


  if(n1 == 0 && n2 == 1) {
    return 0;
  }
  
  if(n1 == 1 && n2 == 4) {
    return 1;
  }

  if(n1 == 2 && n2 == 1) {
    return 2;
  }
  
  if(n1 == 3 && n2 == 1) {
    return 3;
  }
  
  if(n1 == 4 && n2 == 5) {
    return 4;
  }
  
  if(n1 == 4 && n2 == 6) {
    return 5;
  }


  if(n1 == 6 && n2 == 7) {
    return 2;
  }
  
  if(n1 == 7 && n2 == 8) {
    return 3;
  }
  
  if(n1 == 7 && n2 == 9) {
    return 4;
  }
  
  if(n1 == 10 && n2 == 11) {
    return 5;
  }

  if(n1 == 9 && n2 == 10) {
    return 4;
  }
  
  if(n1 == 11 && n2 == 12) {
    return 5;
  }
 
  return 1000*n1 + n2; 
  
}

void
CheckIpv4Rates (NodeContainer &allNodes)
{
  double current_rate = 0.0;
  uint32_t N = allNodes.GetN(); 
  std::cout<<" called "<<std::endl;
  for(uint32_t nid=0; nid < N ; nid++)
  {
    Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(nid))->GetObject<Ipv4> ());
    std::map<std::string,uint32_t>::iterator it;
    for (std::map<std::string,uint32_t>::iterator it=ipv4->flowids.begin(); it!=ipv4->flowids.end(); ++it)
    {
    
      double rate = ipv4->GetStoreDestRate (it->first);  // this is instantaneous rate
   //   double rate = ipv4->GetCSFQRate (it->first); //this is the real long term rate
      double prio = ipv4->GetStorePrio (it->first);
      //double netw_price = ipv4->getCurrentNetwPrice(it->first);
      //double destrate = ipv4->GetCSFQRate (it->first);
      //double destprio = ipv4->GetStoreDestPrio (it->first);
      uint32_t s = it->second;

      /* check if this flowid is from this source */
  /*    if (std::find((source_flow[nid]).begin(), (source_flow[nid]).end(), s)!=(source_flow[nid]).end()) {
         std::cout<<"DestRate flowid "<<it->second<<" "<<Simulator::Now ().GetSeconds () << " " << rate << " "<<prio<<" N "<< N << " node "<<nid<<std::endl;
         current_rate += rate;
      } 
*/
      
      if (std::find((dest_flow[nid]).begin(), (dest_flow[nid]).end(), s)!=(dest_flow[nid]).end()) {
         std::cout<<"DestRate flowid "<<it->second<<" "<<Simulator::Now ().GetSeconds () << " " << rate << " "<<prio<<std::endl;
         current_rate += rate;
      }
      
    }
  }
  std::cout<<Simulator::Now().GetSeconds()<<" TotalRate "<<current_rate<<std::endl;
  
  // check queue size every 1/1000 of a second
  Simulator::Schedule (Seconds (sampling_interval), &CheckIpv4Rates, allNodes);
  if(Simulator::Now().GetSeconds() >= sim_time) {
    Simulator::Stop();
  }
}
Ipv4InterfaceContainer assignAddress(NetDeviceContainer dev, uint32_t subnet_index)
{
    /** assigining ip address **/

    Ipv4InterfaceContainer intf;

    std::ostringstream subnet;
    Ipv4AddressHelper ipv4;
    NS_LOG_UNCOND("Assigning subnet index "<<subnet_index);
    subnet<<"10.1."<<subnet_index<<".0";
    ipv4.SetBase (subnet.str ().c_str (), "255.255.255.0");
    intf = ipv4.Assign (dev);
    return intf;

}


void printlink(Ptr<Node> n1, Ptr<Node> n2)
{
  std::cout<<"printlink: link setup between node "<<n1->GetId()<<" and node "<<n2->GetId()<<std::endl;
}

void sinkInstallNode(uint32_t sourceN, uint32_t sinkN, uint16_t port, uint32_t flow_id, double startTime, uint32_t numBytes, NodeContainer clientNodes)
{
  // Create a packet sink on the star "hub" to receive these packets
  Address anyAddress = InetSocketAddress (Ipv4Address::GetAny (), port);
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", anyAddress);
  ApplicationContainer sinkAppContainer = sinkHelper.Install (clientNodes.Get(sinkN));
  sinkAppContainer.Start(Seconds(0.0));
  sinkApps.Add(sinkAppContainer);


  std::cout<<"sink apps installed on node "<<(clientNodes.Get(sinkN))->GetId()<<std::endl;
  Ptr<PacketSink> pSink = StaticCast <PacketSink> (sinkAppContainer.Get(0));
  pSink->SetAttribute("numBytes", UintegerValue(numBytes));
  pSink->SetAttribute("flowid", UintegerValue(flow_id));
  pSink->SetAttribute("nodeid", UintegerValue(clientNodes.Get(sinkN)->GetId()));
  pSink->SetAttribute("peernodeid", UintegerValue(clientNodes.Get(sourceN)->GetId()));


  /* Debug... Check what we set */
  UintegerValue nb, fid, n1, n2;
  pSink->GetAttribute("numBytes", nb);
  pSink->GetAttribute("flowid", fid);
  pSink->GetAttribute("nodeid", n1);
  pSink->GetAttribute("peernodeid", n2);
  NS_LOG_UNCOND("sink attributed set : numbytes "<<nb.Get()<<" flowid "<<fid.Get()<<" nodeid "<<n1.Get()<<" source nodeid "<<n2.Get());
  
  /* Debug end */

//  Config::SetDefault("ns3::PacketSink::StartMeasurement",TimeValue(Seconds(measurement_starttime)));
}

void change_weight(uint32_t sourceN, uint32_t sinkN, NodeContainer clientNodes, uint32_t rand_weight)
{
  // Socket at the source
  Ptr<Ipv4L3Protocol> sink_node_ipv4 = StaticCast<Ipv4L3Protocol> ((clientNodes.Get(sinkN))->GetObject<Ipv4> ());
  Ipv4Address remoteIp = sink_node_ipv4->GetAddress (1,0).GetLocal();
  // Get source address
  Ptr<Ipv4> source_node_ipv4 = (clientNodes.Get(sourceN))->GetObject<Ipv4> (); 
  Ipv4Address sourceIp = source_node_ipv4->GetAddress (1,0).GetLocal();
  Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((clientNodes.Get(sourceN))->GetObject<Ipv4> ()); // Get Ipv4 instance of the node
  Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal();

  port = flow_dest_port[clientNodes.Get(sourceN)->GetId()];
  std::stringstream ss;
  ss<<addr<<":"<<remoteIp<<":"<<port;
  std::string s = ss.str(); 

  uint32_t fid = flowids[s];

  ipv4->setFlowWeight(fid, rand_weight*1.0);
  sink_node_ipv4->setFlowWeight(fid, rand_weight*1.0);
  std::cout<<"Changing weight of flow "<<fid<<" to "<<rand_weight<<" "<<Simulator::Now().GetSeconds()<<std::endl;

  std::cout<<"flow_start "<<fid<<" "<<(clientNodes.Get(sourceN))->GetId()<<" "<<(clientNodes.Get(sinkN))->GetId()<<" at "<<(Simulator::Now()).GetNanoSeconds()<<" "<<SOME_LARGE_VALUE<<" port "<< port <<" weight "<<rand_weight<<std::endl;

}

Ptr<MyApp> startFlow(uint32_t sourceN, uint32_t sinkN, double flow_start, uint32_t flow_size, uint32_t flow_id, NodeContainer clientNodes, uint32_t rand_weight, Ptr<MyApp> sApp)
{

  Ptr<MyApp> SendingApp = sApp;
  if(sApp == NULL) {
    port++;
    Ptr<Ipv4L3Protocol> sink_node_ipv4 = StaticCast<Ipv4L3Protocol> ((clientNodes.Get(sinkN))->GetObject<Ipv4> ());
    Ipv4Address remoteIp = sink_node_ipv4->GetAddress (1,0).GetLocal();
    Address remoteAddress = (InetSocketAddress (remoteIp, port));
    Ptr<Ipv4> source_node_ipv4 = (clientNodes.Get(sourceN))->GetObject<Ipv4> (); 
    Ipv4Address sourceIp = source_node_ipv4->GetAddress (1,0).GetLocal();
    Address sourceAddress = (InetSocketAddress (sourceIp, port));
    std::cout<<Simulator::Now().GetSeconds()<<" startFlow first time "<<std::endl;
    // Socket at the source
    sinkInstallNode(sourceN, sinkN, port, flow_id, flow_start, flow_size, clientNodes);

    // Get source address

    SendingApp = CreateObject<MyApp> ();
    SendingApp->Setup (remoteAddress, pkt_size, DataRate (link_rate_string), flow_size, flow_start, sourceAddress, clientNodes.Get(sourceN), flow_id, clientNodes.Get(sinkN), rand_weight);
    (clientNodes.Get(sourceN))->AddApplication(SendingApp);
    Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((clientNodes.Get(sourceN))->GetObject<Ipv4> ()); // Get Ipv4 instance of the node
    Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal();

    (source_flow[(clientNodes.Get(sourceN))->GetId()]).push_back(flow_id);
    (dest_flow[(clientNodes.Get(sinkN))->GetId()]).push_back(flow_id);
    std::stringstream ss;
    ss<<addr<<":"<<remoteIp<<":"<<port;
    std::string s = ss.str(); 
    flowids[s] = flow_id;

    flow_dest_port[clientNodes.Get(sourceN)->GetId()] = port;

    ipv4->setFlow(s, flow_id, flow_size, rand_weight);
    sink_node_ipv4->setFlow(s, flow_id, flow_size, rand_weight);
  } else {
    sApp->Setup();
  }
      
  //std::cout<<"FLOW_INFO source_node "<<(clientNodes.Get(sourceN))->GetId()<<" sink_node "<<(clientNodes.Get(sinkN))->GetId()<<" "<<addr<<":"<<remoteIp<<" flow_id "<<flow_id<<" start_time "<<flow_start<<" dest_port "<<port<<" flow_size "<<flow_size<<" "<<rand_weight<<std::endl;
  //flow_id++;
  return SendingApp;
}

//const uint32_t max_flows=5;

void start_a_flow(std::vector<uint32_t>sourcenodes, std::vector<uint32_t>sinknodes, NodeContainer clientNodes)
{

  while(true) 
  {
     UniformVariable urand;
     uint32_t i = urand.GetInteger(0, max_flows-1);
     if(flow_started[i] == 0) { 
      uint32_t source_node = sourcenodes[i];
      uint32_t sink_node = sinknodes[i];

      double flow_size = SOME_LARGE_VALUE;
      double flow_start = Simulator::Now().GetSeconds();
      double rand_weight = 1.0;
      Ptr<MyApp> sendingApp = startFlow(source_node, sink_node, flow_start, flow_size, i, clientNodes, rand_weight, sending_apps[i]);

      if(sending_apps[i] == NULL) {
        sending_apps[i] = sendingApp;
      }
        
      flow_started[i] = 1;
      std::cout<<Simulator::Now().GetSeconds()<<" starting flow "<<i<<" source "<<source_node<<" sink_node "<<sink_node<<std::endl;
      num_flows++;
      break;
     }
  }
}


void stop_a_flow(void)
{
  while (true) {
    UniformVariable urand;
    uint32_t i = urand.GetInteger(0, max_flows-1);
    std::cout<<"picked "<<i<<" to stop"<<std::endl;
    if(flow_started[i] == 1) {
      sending_apps[i]->StopApplication();
      //delete sending_apps[i];
      std::cout<<Simulator::Now().GetSeconds()<<" stopping flow "<<i<<std::endl;
      num_flows--;
      flow_started[i] = 0;
      break;
    }
  }
}
  
void startflowwrapper( std::vector<uint32_t> sourcenodes, std::vector<uint32_t> sinknodes, NodeContainer clientNodes)
{

  std::cout<<"Entered startflowwrapper at "<<Simulator::Now().GetSeconds()<<" nf "<<num_flows<<" maxf "<<max_flows_allowed<<" minf "<<min_flows<<std::endl;
  if(num_flows >= max_flows_allowed) {
    stop_a_flow();
  } else if(num_flows <= min_flows) {
    start_a_flow(sourcenodes, sinknodes, clientNodes);
  } else {
     Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
     double rand_num = uv->GetValue(0.0, 1.0);
     if(rand_num < 0.5) {
        start_a_flow(sourcenodes, sinknodes, clientNodes);
      } else {
        stop_a_flow();
      }
  }
  Simulator::Schedule (Seconds (0.01), &startflowwrapper, sourcenodes, sinknodes, clientNodes);


}


int 
main (int argc, char *argv[])
{

  //LogComponentEnable ("TcpSocketBase", LOG_LEVEL_ALL);
/*  LogComponentEnable ("TcpNewReno", LOG_LEVEL_ALL);*/
/*  LogComponentEnable ("PrioQueue", LOG_LEVEL_ALL);
  LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
*/


  uint32_t N = 4; //number of nodes in the star
  std::string queue_type;
  std::string empirical_dist_file = "DCTCP_CDF"; //file name containing empirical dist
  double epoch_update_time = 0.01;
  std::string flow_util_file;
  bool pkt_tag = true, onlydctcp, wfq, dctcp_mark;
  bool margin_util_price, m_pfabric = false;
  bool strawmancc = false;
  

  // Allow the user to override any of the defaults and the above
  // Config::SetDefault()s at run-time, via command-line arguments
  CommandLine cmd;
  cmd.AddValue("fct_alpha", "fct_alpha",  fct_alpha);
  cmd.AddValue("util_method", "util_method",  util_method);
  cmd.AddValue("m_pfabric", "m_pfabric",  m_pfabric);
  cmd.AddValue("pkt_tag","pkt_tag",pkt_tag);
  cmd.AddValue("dctcp_mark","dctcp_mark",dctcp_mark);
  cmd.AddValue ("nNodes", "Number of nodes to place in the star", N);
  cmd.AddValue ("prefix", "Output prefix", prefix);
  cmd.AddValue ("queuetype", "Queue Type", queue_type);
  cmd.AddValue ("load", "Fabric load", load);
  cmd.AddValue ("empirical_dist_file", "name of the file that contains empirical dist of packets", empirical_dist_file);
  cmd.AddValue ("epoch_update_time", "Epoch Update", epoch_update_time);
  cmd.AddValue ("flow_util_file", "flow_util_file", flow_util_file);
  cmd.AddValue ("onlydctcp", "onlydctcp", onlydctcp);
  cmd.AddValue ("wfq", "wfq", wfq);
  cmd.AddValue ("sim_time", "sim_time", sim_time);
  cmd.AddValue ("pkt_size", "pkt_size", pkt_size);
  cmd.AddValue ("link_rate","link_rate",link_rate);
  cmd.AddValue ("link_delay","link_delay",link_delay);
  cmd.AddValue ("ecn_thresh", "ecn_thresh", max_ecn_thresh);

  cmd.AddValue("dgd_gamma", "dgd_gamma", dgd_gamma);
  cmd.AddValue("margin_util_price", "margin_util_price", margin_util_price);
  cmd.AddValue("strawmancc", "strawmancc", strawmancc);
  cmd.AddValue ("price_update_time", "price_update_time", price_update_time);
  cmd.AddValue ("rate_update_time", "rate_update_time", rate_update_time);
  cmd.AddValue ("sampling_interval", "sampling_interval", sampling_interval);
  cmd.AddValue ("controller_estimated_unknown_load", "controller_estimated_unknown_load",controller_estimated_unknown_load);
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
  cmd.AddValue ("guardtime", "guardtime to ignore mins", guard_time);

  cmd.Parse (argc, argv);

  if(link_rate == 10000000000) {
    link_rate_string = "10Gbps";
  } else if(link_rate == 100000000000) {
    link_rate_string = "100Gbps";
  }

  NS_LOG_UNCOND("file prefix "<<prefix);

  /* print all command line parameters */
  NS_LOG_UNCOND("num_nodes "<<N<<" prefix "<<prefix<<" queuetype "<<queue_type<<" load "<<load<<" empirical_dist_file "<<empirical_dist_file);
  
  /* command line parameters print end */
  
  double total_rtt = link_delay * 4.0;
  uint32_t bdproduct = link_rate *total_rtt/(1000000.0* 8.0);
  uint32_t initcwnd = (bdproduct / max_segment_size)+1;
  uint32_t ssthresh = initcwnd * max_segment_size;

  if(strawmancc) {
    initcwnd = initcwnd*4.0;
  }

  NS_LOG_UNCOND("Setting ssthresh = "<<ssthresh<<" initcwnd = "<<initcwnd);  
  

  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue(max_segment_size));
//  Config::SetDefault ("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue(ssthresh_value));
  Config::SetDefault ("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue(ssthresh));
  Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue(initcwnd));
  Config::SetDefault ("ns3::TcpSocketBase::Timestamp", BooleanValue(false));
//  Config::SetDefault ("ns3::RttEstimator::MinRTO", TimeValue(Seconds(0.001)));
  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (recv_buf_size));
  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (send_buf_size));
  // Disable delayed ack
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  Config::SetDefault("ns3::TcpNewReno::dctcp", BooleanValue(false));
  Config::SetDefault("ns3::TcpNewReno::xfabric", BooleanValue(true));

  Config::SetDefault("ns3::PacketSink::StartMeasurement",TimeValue(Seconds(measurement_starttime)));
  Config::SetDefault("ns3::PrioQueue::PriceUpdateTime", TimeValue(Seconds(price_update_time)));
  Config::SetDefault("ns3::PrioQueue::guardTime", TimeValue(Seconds(guard_time)));

  Config::SetDefault("ns3::PrioQueue::m_pkt_tag", BooleanValue(pkt_tag));
  Config::SetDefault("ns3::PrioQueue::m_pfabricdequeue",BooleanValue(m_pfabric));

  Config::SetDefault("ns3::PrioQueue::dgd_alpha", DoubleValue(dgd_alpha));
  Config::SetDefault("ns3::PrioQueue::target_queue", DoubleValue(target_queue));

  Config::SetDefault("ns3::PrioQueue::dgd_gamma", DoubleValue(dgd_gamma));


  Config::SetDefault ("ns3::DropTailQueue::Mode" , StringValue("QUEUE_MODE_BYTES"));
  Config::SetDefault ("ns3::DropTailQueue::MaxBytes", UintegerValue(max_queue_size));
  
  Config::SetDefault ("ns3::PrioQueue::Mode", StringValue("QUEUE_MODE_BYTES"));
  Config::SetDefault ("ns3::PrioQueue::MaxBytes", UintegerValue (max_queue_size));

  Config::SetDefault ("ns3::PrioQueue::host_compensate", BooleanValue(host_compensate));

  Config::SetDefault ("ns3::PrioQueue::ECNThreshBytes", UintegerValue (max_ecn_thresh));
  NS_LOG_UNCOND("Set max_ecn_thresh at "<<max_ecn_thresh);

  Config::SetDefault("ns3::Ipv4L3Protocol::m_pkt_tag", BooleanValue(pkt_tag));
  Config::SetDefault("ns3::Ipv4L3Protocol::m_wfq", BooleanValue(wfq));
  Config::SetDefault("ns3::Ipv4L3Protocol::UtilFunction", UintegerValue(util_method));
  Config::SetDefault("ns3::Ipv4L3Protocol::m_pfabric", BooleanValue(m_pfabric));

  Config::SetDefault("ns3::Ipv4L3Protocol::rate_based", BooleanValue(strawmancc));
  Config::SetDefault("ns3::Ipv4L3Protocol::host_compensate", BooleanValue(host_compensate));




  // Here, we will create N nodes in a star.
  NS_LOG_INFO ("Create nodes.");


  NodeContainer bottleneckNodes;
  NodeContainer clientNodes;
  bottleneckNodes.Create (13);
  clientNodes.Create (18);
  
  NodeContainer allNodes = NodeContainer (bottleneckNodes, clientNodes);
  
  N = allNodes.GetN();

  // Install network stacks on the nodes
  InternetStackHelper internet;
  internet.Install (allNodes);

  // We create the channels first without any IP addressing information
  //
  // Queue, Channel and link characteristics
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2paccess;
  p2paccess.SetDeviceAttribute ("DataRate", StringValue (link_rate_string));
  p2paccess.SetChannelAttribute ("Delay", TimeValue(MicroSeconds(link_delay)));
  p2paccess.SetQueue("ns3::PrioQueue", "pFabric", StringValue("1"), "DataRate", StringValue(link_rate_string));

  PointToPointHelper p2pbottleneck;
  p2pbottleneck.SetDeviceAttribute ("DataRate", StringValue (link_rate_string));
  p2pbottleneck.SetChannelAttribute ("Delay", TimeValue(MicroSeconds(link_delay)));
  p2pbottleneck.SetQueue("ns3::PrioQueue", "pFabric", StringValue("1"),"DataRate", StringValue(link_rate_string));


  std::vector<NetDeviceContainer> bdevice;
  std::vector<NetDeviceContainer> access;

  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(0), bottleneckNodes.Get(1)));
  printlink(bottleneckNodes.Get(0), bottleneckNodes.Get(1));
  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(1), bottleneckNodes.Get(4)));
  printlink(bottleneckNodes.Get(1), bottleneckNodes.Get(4));


  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(2), bottleneckNodes.Get(1)));
  printlink(bottleneckNodes.Get(2), bottleneckNodes.Get(1));
  // LOOP
  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(3), bottleneckNodes.Get(1)));
  printlink(bottleneckNodes.Get(3), bottleneckNodes.Get(1));

  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(3), bottleneckNodes.Get(2)));
  printlink(bottleneckNodes.Get(3), bottleneckNodes.Get(2));
 
  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(4), bottleneckNodes.Get(5)));
  printlink(bottleneckNodes.Get(4), bottleneckNodes.Get(5));
  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(4), bottleneckNodes.Get(6)));
  printlink(bottleneckNodes.Get(4), bottleneckNodes.Get(6));

 
  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(6), bottleneckNodes.Get(7)));
  printlink(bottleneckNodes.Get(6), bottleneckNodes.Get(7));
  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(7), bottleneckNodes.Get(8)));
  printlink(bottleneckNodes.Get(7), bottleneckNodes.Get(8));
  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(7), bottleneckNodes.Get(9)));
  printlink(bottleneckNodes.Get(7), bottleneckNodes.Get(9));

  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(10), bottleneckNodes.Get(11)));
  printlink(bottleneckNodes.Get(10), bottleneckNodes.Get(11));
  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(9), bottleneckNodes.Get(10)));
  printlink(bottleneckNodes.Get(9), bottleneckNodes.Get(10));
  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(11), bottleneckNodes.Get(12)));
  printlink(bottleneckNodes.Get(11), bottleneckNodes.Get(12));

  // Attach other nodes

  access.push_back(p2paccess.Install(bottleneckNodes.Get(0), clientNodes.Get(0)));
  printlink (bottleneckNodes.Get(0), clientNodes.Get(0));
  access.push_back(p2paccess.Install(bottleneckNodes.Get(0), clientNodes.Get(1)));
  printlink( bottleneckNodes.Get(0), clientNodes.Get(1));
  access.push_back(p2paccess.Install(bottleneckNodes.Get(0), clientNodes.Get(2)));
  printlink( bottleneckNodes.Get(0), clientNodes.Get(2));

  access.push_back(p2paccess.Install(bottleneckNodes.Get(1), clientNodes.Get(6)));
  printlink( bottleneckNodes.Get(1), clientNodes.Get(6));

  access.push_back(p2paccess.Install(bottleneckNodes.Get(2), clientNodes.Get(5)));
  printlink( bottleneckNodes.Get(2), clientNodes.Get(5));

  access.push_back(p2paccess.Install(bottleneckNodes.Get(3), clientNodes.Get(3)));
  printlink( bottleneckNodes.Get(3), clientNodes.Get(3));

  access.push_back(p2paccess.Install(bottleneckNodes.Get(3), clientNodes.Get(4)));
  printlink( bottleneckNodes.Get(3), clientNodes.Get(4));

  access.push_back(p2paccess.Install(bottleneckNodes.Get(4), clientNodes.Get(8)));
  printlink( bottleneckNodes.Get(4), clientNodes.Get(8));

  access.push_back(p2paccess.Install(bottleneckNodes.Get(5), clientNodes.Get(9)));
  printlink( bottleneckNodes.Get(5), clientNodes.Get(9));

  access.push_back(p2paccess.Install(bottleneckNodes.Get(6), clientNodes.Get(11)));
  printlink( bottleneckNodes.Get(6), clientNodes.Get(11));

  access.push_back(p2paccess.Install(bottleneckNodes.Get(7), clientNodes.Get(12)));
  printlink( bottleneckNodes.Get(7), clientNodes.Get(12));

  access.push_back(p2paccess.Install(bottleneckNodes.Get(8), clientNodes.Get(13)));
  printlink( bottleneckNodes.Get(8), clientNodes.Get(13));

  access.push_back(p2paccess.Install(bottleneckNodes.Get(9), clientNodes.Get(14)));
  printlink( bottleneckNodes.Get(9), clientNodes.Get(14));

  access.push_back(p2paccess.Install(bottleneckNodes.Get(10), clientNodes.Get(15)));
  printlink( bottleneckNodes.Get(10), clientNodes.Get(15));


  access.push_back(p2paccess.Install(bottleneckNodes.Get(11), clientNodes.Get(16)));
  printlink( bottleneckNodes.Get(11), clientNodes.Get(16));

  access.push_back(p2paccess.Install(bottleneckNodes.Get(12), clientNodes.Get(17)));
  printlink( bottleneckNodes.Get(12), clientNodes.Get(17));

  //access.push_back(p2paccess.Install(bottleneckNodes.Get(2), clientNodes.Get(4)));
  //printlink( bottleneckNodes.Get(2), clientNodes.Get(4));
  //access.push_back(p2paccess.Install(bottleneckNodes.Get(2), clientNodes.Get(5)));
  //printlink( bottleneckNodes.Get(2), clientNodes.Get(5));

  //access.push_back(p2paccess.Install(bottleneckNodes.Get(4), clientNodes.Get(6)));
  //printlink( bottleneckNodes.Get(4), clientNodes.Get(6));
  //access.push_back(p2paccess.Install(bottleneckNodes.Get(4), clientNodes.Get(7)));
  //printlink( bottleneckNodes.Get(4), clientNodes.Get(7));
  //access.push_back(p2paccess.Install(bottleneckNodes.Get(4), clientNodes.Get(8)));
  //printlink( bottleneckNodes.Get(4), clientNodes.Get(8));


  //access.push_back(p2paccess.Install(bottleneckNodes.Get(5), clientNodes.Get(9)));
  //printlink( bottleneckNodes.Get(5), clientNodes.Get(9));
  //access.push_back(p2paccess.Install(bottleneckNodes.Get(5), clientNodes.Get(10)));
  //printlink( bottleneckNodes.Get(5), clientNodes.Get(10));
  //access.push_back(p2paccess.Install(bottleneckNodes.Get(5), clientNodes.Get(11)));
  //printlink(bottleneckNodes.Get(5), clientNodes.Get(11));

  std::vector<Ipv4InterfaceContainer> bAdj (bdevice.size());
  std::vector<Ipv4InterfaceContainer> aAdj (access.size());

  uint32_t cur_subnet = 0;

  for(uint32_t i=0; i < bdevice.size(); ++i)
  {
    // set it as switch
    //

    Ptr<PointToPointNetDevice> nd = StaticCast<PointToPointNetDevice> ((bdevice[i]).Get(0));
    Ptr<Queue> queue = nd->GetQueue ();
    uint32_t nid = (nd->GetNode())->GetId(); 
    Ptr<PointToPointNetDevice> nd1 = StaticCast<PointToPointNetDevice> ((bdevice[i]).Get(1));
    Ptr<Queue> queue1 = nd1->GetQueue ();
    uint32_t nid1 = (nd1->GetNode())->GetId(); 

    uint32_t link_id = getLinkid(nid, nid1); 


    NS_LOG_UNCOND("Node id is "<<(nd->GetNode())->GetId());
    StaticCast<PrioQueue> (queue)->SetNodeID(nid);
    StaticCast<PrioQueue> (queue)->SetLinkID(link_id);


    std::stringstream ss;
     ss<<nid<<"_"<<nid<<"_"<<nid1;
     std::string fkey1 = ss.str(); 

     std::cout<<"fkey1 "<<fkey1<<std::endl;

     std::stringstream ss1;
     ss1<<nid1<<"_"<<nid<<"_"<<nid1;
     std::string fkey2 = ss1.str(); 
     std::cout<<"fkey2 "<<fkey2<<std::endl;


    std::cout<<"Set node "<<nid<<" as switch";
    Simulator::Schedule (Seconds (1.0), &CheckQueueSize, queue);

    // the other end too
    NS_LOG_UNCOND("Node id is "<<(nd1->GetNode())->GetId());
    StaticCast<PrioQueue> (queue1)->SetNodeID(nid1);
    StaticCast<PrioQueue> (queue1)->SetLinkID(10000+link_id);
    std::cout<<"Set node "<<nid1<<" as switch"<<std::endl;
    Simulator::Schedule (Seconds (1.0), &CheckQueueSize, queue1);
    //
     std::cout<<"fkey1 "<<fkey1<<std::endl;
     std::cout<<"fkey2 "<<fkey2<<std::endl;
    // assign ip address
    StaticCast<PrioQueue> (queue)->SetLinkIDString(fkey1);
    StaticCast<PrioQueue> (queue1)->SetLinkIDString(fkey2);
    
    bAdj[i] = assignAddress(bdevice[i], cur_subnet);
    cur_subnet++;
  }

  for(uint32_t i=0; i < access.size(); ++i)
  {
    // set it as switch
    //

    Ptr<PointToPointNetDevice> nd = StaticCast<PointToPointNetDevice> ((access[i]).Get(0));
    Ptr<Queue> queue = nd->GetQueue ();
    uint32_t nid = (nd->GetNode())->GetId(); 
    Ptr<PointToPointNetDevice> nd1 = StaticCast<PointToPointNetDevice> ((access[i]).Get(1));
    Ptr<Queue> queue1 = nd1->GetQueue ();
    uint32_t nid1 = (nd1->GetNode())->GetId(); 

    std::stringstream ss;
     ss<<nid<<"_"<<nid<<"_"<<nid1;
     std::string fkey1 = ss.str(); 

     std::cout<<"fkey1 "<<fkey1<<std::endl;

     std::stringstream ss1;
     ss1<<nid1<<"_"<<nid<<"_"<<nid1;
     std::string fkey2 = ss1.str(); 
     std::cout<<"fkey2 "<<fkey2<<std::endl;

    NS_LOG_UNCOND("Node id is "<<(nd->GetNode())->GetId());
    StaticCast<PrioQueue> (queue)->SetNodeID(nid);

    // the other end too
    NS_LOG_UNCOND("Node id is "<<(nd1->GetNode())->GetId());
    StaticCast<PrioQueue> (queue1)->SetNodeID(nid1);
    // assign ip address
    StaticCast<PrioQueue> (queue)->SetLinkIDString(fkey1);
    StaticCast<PrioQueue> (queue1)->SetLinkIDString(fkey2);

    Simulator::Schedule (Seconds (1.0), &CheckQueueSize, queue);
    Simulator::Schedule (Seconds (1.0), &CheckQueueSize, queue1);
    //
    // assign ip address
  }

  for(uint32_t i=0; i < access.size(); ++i)
  {
    aAdj[i] = assignAddress(access[i], cur_subnet);
    cur_subnet++;
  }
  

  //Turn on global static routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  //std::vector<uint32_t> sinknodes {4,5,6,9,10,11};
//  std::vector<uint32_t> sinknodes(4,6,9,10,11); //this works
  static const uint32_t arr[] = {6,8,11,5,13,14,16,17}; 
  //static const uint32_t arr[] = {3};
  
  std::vector<uint32_t> sinknodes (arr, arr + sizeof(arr) / sizeof(arr[0]) );

  sinkApps.Start (Seconds (1.0));
  sinkApps.Stop (Seconds (sim_time));

  //static const uint32_t arr1[] = {0,1,8,2,3}; 
  static const uint32_t arr1[] = {0,1,2,9,3,4,12,15};
  
  std::vector<uint32_t> sourcenodes (arr1, arr1 + sizeof(arr1) / sizeof(arr1[0]) );
  uint32_t flow_id = 1; 
  global_flowid = flow_id;
   
   Simulator::Schedule (Seconds (1.0), &startflowwrapper, sourcenodes, sinknodes, clientNodes);

  uint32_t Ntrue = allNodes.GetN(); 
  for(uint32_t nid=0; nid<Ntrue; nid++)
  {
     Ptr<Ipv4> ipv4 = (allNodes.Get(nid))->GetObject<Ipv4> ();
      
     //StaticCast<Ipv4L3Protocol> (ipv4)->setFlows(flowids);
     //StaticCast<Ipv4L3Protocol> (ipv4)->setQueryTime(0.000004);
     //StaticCast<Ipv4L3Protocol> (ipv4)->setAlpha(1.0/128.0);
     StaticCast<Ipv4L3Protocol> (ipv4)->setQueryTime(rate_update_time);
     StaticCast<Ipv4L3Protocol> (ipv4)->setAlpha(1.0);
     StaticCast<Ipv4L3Protocol> (ipv4)->setKay(kvalue);
     //StaticCast<Ipv4L3Protocol> (ipv4)->setFlowUtils(futils);
     StaticCast<Ipv4L3Protocol> (ipv4)->setfctAlpha(fct_alpha);
     

  }
  Simulator::Schedule (Seconds (1.0), &CheckIpv4Rates, allNodes);

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
