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
#include "sending_app.h"

#define SOME_LARGE_VALUE 1250000000

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("pfabric");
uint32_t global_flowid = 0;

Ptr<MyApp> startFlow(uint32_t sourceN, uint32_t sinkN, double flow_start, uint32_t flow_size, uint32_t flow_id)
{
  ports[sinkN]++;
  // Socket at the source
  Ptr<Ipv4L3Protocol> sink_node_ipv4 = StaticCast<Ipv4L3Protocol> ((clientNodes.Get(sinkN))->GetObject<Ipv4> ());
  Ipv4Address remoteIp = sink_node_ipv4->GetAddress (1,0).GetLocal();
  Address remoteAddress = (InetSocketAddress (remoteIp, ports[sinkN]));
  sinkInstallNode(sourceN, sinkN, ports[sinkN], flow_id, flow_start, flow_size, 1);

  // Get source address
  Ptr<Ipv4> source_node_ipv4 = (clientNodes.Get(sourceN))->GetObject<Ipv4> (); 
  Ipv4Address sourceIp = source_node_ipv4->GetAddress (1,0).GetLocal();
  Address sourceAddress = (InetSocketAddress (sourceIp, ports[sinkN]));

  Ptr<MyApp> SendingApp = CreateObject<MyApp> ();
  SendingApp->Setup (remoteAddress, pkt_size, DataRate (link_rate_string), flow_size, flow_start, sourceAddress, clientNodes.Get(sourceN), flow_id, clientNodes.Get(sinkN), 1, 1);
  (clientNodes.Get(sourceN))->AddApplication(SendingApp);
      
  Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((clientNodes.Get(sourceN))->GetObject<Ipv4> ()); // Get Ipv4 instance of the node
  Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal();

  (source_flow[(clientNodes.Get(sourceN))->GetId()]).push_back(flow_id);
  (dest_flow[(clientNodes.Get(sinkN))->GetId()]).push_back(flow_id);
  std::stringstream ss;
  ss<<addr<<":"<<remoteIp<<":"<<ports[sinkN];
  std::string s = ss.str(); 
  flowids[s] = flow_id;

  ipv4->setFlow(s, flow_id, flow_size, 1.0);
  sink_node_ipv4->setFlow(s, flow_id, flow_size, 1.0);
  std::cout<<"FLOW_INFO source_node "<<(clientNodes.Get(sourceN))->GetId()<<" sink_node "<<(clientNodes.Get(sinkN))->GetId()<<" "<<addr<<":"<<remoteIp<<" flow_id "<<flow_id<<" start_time "<<flow_start<<" dest_port "<<ports[sinkN]<<" flow_size "<<flow_size<<" "<<std::endl;
  //flow_id++;
  return SendingApp;
}

const uint32_t max_flows = 5;
uint32_t flow_started[max_flows];
Ptr<MyApp> sending_apps[max_flows];
uint32_t num_flows = 0;
uint32_t min_flows=3, max_flows_allowed=4;

void stop_a_flow(void)
{
  while (true) {
    //a = std::mt19937 g2(rand_seed);
    //i = a % max_flows;
    //uint32_t i = (g1()*1.0/g1.max()) * (max_flows);
//    uint32_t i = distribution2(generator2);
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
  

void start_a_flow(void)
{
    UniformVariable urand;
    uint32_t i = urand.GetInteger(0, max_flows-1);
    
    std::cout<<"picked "<<i<<" to start"<<std::endl;
    if(flow_started[i] == 0) {
      uint32_t source_node = (sourceNodes.Get(i))->GetId();
      uint32_t sink_node = (sinkNodes.Get(i))->GetId();

      double flow_size = SOME_LARGE_VALUE;
      double flow_start = Simulator::Now().GetSeconds();
  
  //    double rand_weight = (g1()*1.0/g1.max()) * 8.0;
//      double rand_weight = 1.0*(g1() % (8));
//      UniformVariable randt(1,8);
      Ptr<MyApp> sendingApp = startFlow(source_node, sink_node, flow_start, flow_size, global_flowid);
      sending_apps[i] = sendingApp;
      global_flowid++;
      flow_started[i] = 1;
      std::cout<<Simulator::Now().GetSeconds()<<" starting flow "<<i<<std::endl;
      num_flows++;
   }
}

void startflowwrapper(void) 
{
  std::cout<<"Entered startflowwrapper at "<<Simulator::Now().GetSeconds()<<std::endl;
  
  if(num_flows >= max_flows_allowed) {
    stop_a_flow();
  } else if(num_flows <= min_flows) {
    start_a_flow();
  } else {
     Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
     double rand_num = uv->GetValue(0.0, 1.0);
     if(rand_num < 0.5) {
        start_a_flow();
      } else {
        stop_a_flow();
      }
  }

  Simulator::Schedule (Seconds (0.1), &startflowwrapper);
}

void config_queue(Ptr<Queue> Q, uint32_t nid, uint32_t vpackets, std::string fkey1)
{
      Q->SetNodeID(nid);
      Q->SetLinkIDString(fkey1);
      Q->SetVPkts(vpackets);
}

void createTopology()
{



  NodeContainer bottleneckNodes;

  bottleneckNodes.Create (6);
  clientNodes.Create (12);
  
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
  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(1), bottleneckNodes.Get(2)));
  printlink(bottleneckNodes.Get(1), bottleneckNodes.Get(2));
  
  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(1), bottleneckNodes.Get(3)));
  printlink( bottleneckNodes.Get(1), bottleneckNodes.Get(3));
  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(3), bottleneckNodes.Get(4)));
  printlink( bottleneckNodes.Get(3), bottleneckNodes.Get(4));
  bdevice.push_back(p2pbottleneck.Install(bottleneckNodes.Get(3), bottleneckNodes.Get(5)));
  printlink( bottleneckNodes.Get(3), bottleneckNodes.Get(5));

  // Attach other nodes

  access.push_back(p2paccess.Install(bottleneckNodes.Get(0), clientNodes.Get(0)));
  printlink (bottleneckNodes.Get(0), clientNodes.Get(0));
  access.push_back(p2paccess.Install(bottleneckNodes.Get(0), clientNodes.Get(1)));
  printlink( bottleneckNodes.Get(0), clientNodes.Get(1));
  access.push_back(p2paccess.Install(bottleneckNodes.Get(0), clientNodes.Get(2)));
  printlink( bottleneckNodes.Get(0), clientNodes.Get(2));

  access.push_back(p2paccess.Install(bottleneckNodes.Get(1), clientNodes.Get(3)));
  printlink( bottleneckNodes.Get(1), clientNodes.Get(3));

  access.push_back(p2paccess.Install(bottleneckNodes.Get(2), clientNodes.Get(4)));
  printlink( bottleneckNodes.Get(2), clientNodes.Get(4));
  access.push_back(p2paccess.Install(bottleneckNodes.Get(2), clientNodes.Get(5)));
  printlink( bottleneckNodes.Get(2), clientNodes.Get(5));

  access.push_back(p2paccess.Install(bottleneckNodes.Get(4), clientNodes.Get(6)));
  printlink( bottleneckNodes.Get(4), clientNodes.Get(6));
  access.push_back(p2paccess.Install(bottleneckNodes.Get(4), clientNodes.Get(7)));
  printlink( bottleneckNodes.Get(4), clientNodes.Get(7));
  access.push_back(p2paccess.Install(bottleneckNodes.Get(4), clientNodes.Get(8)));
  printlink( bottleneckNodes.Get(4), clientNodes.Get(8));


  access.push_back(p2paccess.Install(bottleneckNodes.Get(5), clientNodes.Get(9)));
  printlink( bottleneckNodes.Get(5), clientNodes.Get(9));
  access.push_back(p2paccess.Install(bottleneckNodes.Get(5), clientNodes.Get(10)));
  printlink( bottleneckNodes.Get(5), clientNodes.Get(10));
  access.push_back(p2paccess.Install(bottleneckNodes.Get(5), clientNodes.Get(11)));
  printlink(bottleneckNodes.Get(5), clientNodes.Get(11));

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

     // get the string version of names of the queues 
     std::stringstream ss;
     ss<<nid<<"_"<<nid<<"_"<<nid1;
     std::string fkey1 = ss.str(); 

     std::cout<<"fkey1 "<<fkey1<<std::endl;

     std::stringstream ss1;
     ss1<<nid1<<"_"<<nid<<"_"<<nid1;
     std::string fkey2 = ss1.str(); 
     std::cout<<"fkey2 "<<fkey2<<std::endl;

     config_queue(queue, nid, vpackets, fkey1);
     config_queue(queue1, nid1, vpackets, fkey2);

     Simulator::Schedule (Seconds (1.0), &CheckQueueSize, queue);
     Simulator::Schedule (Seconds (1.0), &CheckQueueSize, queue1);
     // assign ip address


    //
    // assign ip address
    
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

     // get the string version of names of the queues 
     std::stringstream ss;
     ss<<nid<<"_"<<nid<<"_"<<nid1;
     std::string fkey1 = ss.str(); 

     std::cout<<"fkey1 "<<fkey1<<std::endl;

     std::stringstream ss1;
     ss1<<nid1<<"_"<<nid<<"_"<<nid1;
     std::string fkey2 = ss1.str(); 
     std::cout<<"fkey2 "<<fkey2<<std::endl;

     config_queue(queue, nid, vpackets, fkey1);
     config_queue(queue1, nid1, vpackets, fkey2);

    //
    // assign ip address
  }

  for(uint32_t i=0; i < access.size(); ++i)
  {
    aAdj[i] = assignAddress(access[i], cur_subnet);
    cur_subnet++;
  }
  
   ports = new uint16_t [clientNodes.GetN()];
   for (uint32_t i=0; i <clientNodes.GetN(); i++) {
       ports[i] = 1;
   }

  //Turn on global static routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

}

void setUpTraffic()      
{

  static const uint32_t source_arr[] = {4,6,9,10,11};
  static const uint32_t sink_arr[] = {0,1,8,2,3}; 
  for ( unsigned int i = 0; i < num_flows; i++ )
  {
    sourceNodes.Add(allNodes.Get(source_arr[i]));
    sinkNodes.Add(allNodes.Get(sink_arr[i]));
  }
   
  Simulator::Schedule (Seconds (1.0), &startflowwrapper);
}


int 
main (int argc, char *argv[])
{
  CommandLine cmd = addCmdOptions();
  cmd.Parse (argc, argv);
  common_config(); 

  createTopology();
  //setUpTraffic();
  startflowwrapper();
  setUpMonitoring();


  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}
