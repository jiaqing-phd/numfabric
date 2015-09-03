#include <ctime>
#include <sstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/topology-read-module.h"
#include <list>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Rfuel");


// take from NS3 rocketfuel example: topology-example-sim.cc
std::string format ("Rocketfuel");
std::string input ("src/topology-read/examples/RocketFuel_toposample_1239_weights.txt");

ns3::TopologyReaderHelper topoHelp;
topoHelp.SetFileName (input);
topoHelp.SetFileType (format);
Ptr<TopologyReader> inFile = topoHelp.GetTopologyReader ();

NodeContainer nodes;

if (inFile != 0)
{
  nodes = inFile->Read ();
}

if (inFile->LinksSize () == 0)
{
  NS_LOG_ERROR ("Problems reading the topology file. Failing.");
  return -1;
}

int totlinks = inFile->LinksSize ();
std::cout <<" ROCKET : totlinks "<< totlinks << std::endl;

uint32_t totalNodes = nodes.GetN ();
std::cout <<" ROCKET : totalNodes"<< totalNodes << std::endl;

// mixture of kanthi + rocketfuel example
////////////////////////////////////////////////////////
// steps to do
// 1. randomly assign 1/2 nodes to source others to sink
// 2. create sourceNodes, sinkNodes containers
// 3. create ports and allNodes
// 4. install internet on allNodes
// 5. create channels, queues 

// rv to choose source or sink
////////////////////////////////////////////////////////
Ptr<UniformRandomVariable> unifRandom = CreateObject<UniformRandomVariable> ();

// step 1: randomly assign 1/2 the nodes
// create a map from node ID to 0 for source, 1 to sink
std::map<uint32_t, double> source_sink_assignment;
int num_sources = 0;
int num_sinks = 0;

for ( unsigned int i = 0; i < nodes.GetN (); i++ )
{
  
  unsigned int randomServerNumber = unifRandom->GetInteger (0, 10);

  if (randomServerNumber <= 5) {
     // source node
    source_sink_assignment[i] = 0.0;
    num_sources++;

  } else {
    // sink node
    source_sink_assignment[i] = 1.0;
    num_sinks++;
  }
}

NodeContainer* sourceNodes = new NodeContainer[num_sources];
NodeContainer* sinkNodes = new NodeContainer[num_sinks];

for ( unsigned int i = 0; i < nodes.GetN (); i++ )
{
  if (source_sink_assignment[i] == 0.0) {
    sourceNodes.push_back(nodes[i]);
  } else {
    sinkNodes.push_back(nodes[i]);
  }
}

// kanthis code verbatim: get ports and install internet
ports = new uint16_t [sinkNodes.GetN()];

for (uint32_t i=0; i <sinkNodes.GetN(); i++) {
    ports[i] = 1;
}

InternetStackHelper internet;
internet.Install (nodes);

////////////////////////////////////////////////////////

// nc is same as allNodes in Kanthis code
NS_LOG_INFO ("creating node containers");
NodeContainer* nc = new NodeContainer[totlinks];
TopologyReader::ConstLinksIterator iter;
int i = 0;
for ( iter = inFile->LinksBegin (); iter != inFile->LinksEnd (); iter++, i++ )
{
  nc[i] = NodeContainer (iter->GetFromNode (), iter->GetToNode ());
}


// set up queues links etc
//////////////////////////////////////////////////////
NS_LOG_INFO ("creating net device containers");
NetDeviceContainer* ndc = new NetDeviceContainer[totlinks];
PointToPointHelper p2p;
for (int i = 0; i < totlinks; i++)
{
 
  p2p.SetDeviceAttribute ("DataRate", StringValue (link_rate_string));
  p2p.SetChannelAttribute ("Delay", TimeValue(MicroSeconds(5.0)));
  
  // SC: set the queue here
  //////////////////////////
  if(queue_type == "W2FQ") {
    std::cout<<"setting queue to W2FQ"<<std::endl;
    p2p.SetQueue("ns3::W2FQ", "DataRate", StringValue(link_rate_string), "MaxBytes", UintegerValue(max_queue_size), "Mode", StringValue("QUEUE_MODE_BYTES"));
  } else if(queue_type == "WFQ") {
    std::cout<<"setting queue to WFQ"<<std::endl;
    p2p.SetQueue("ns3::PrioQueue", "pFabric", StringValue("1"),"DataRate", StringValue(link_rate_string), "MaxBytes", UintegerValue(max_queue_size), "Mode", StringValue("QUEUE_MODE_BYTES"));
  } else if(queue_type == "FifoQueue") {
    std::cout<<"setting queue to FifoQueue"<<std::endl;
    p2p.SetQueue("ns3::FifoQueue", "MaxBytes", UintegerValue(max_queue_size), "Mode", StringValue("QUEUE_MODE_BYTES"));
  } else if(queue_type == "hybridQ") {
    std::cout<<"setting queue to hybridQueue"<<std::endl;
    p2p.SetQueue("ns3::hybridQ", "MaxBytes", UintegerValue(max_queue_size), "Mode", StringValue("QUEUE_MODE_BYTES"), "DataRate", StringValue(link_rate_string));
  } else if(queue_type == "fifo_hybridQ") {
    std::cout<<"setting queue to fifo_hybridQueue"<<std::endl;
    p2p.SetQueue("ns3::fifo_hybridQ", "MaxBytes", UintegerValue(max_queue_size), "Mode", StringValue("QUEUE_MODE_BYTES"), "DataRate", StringValue(link_rate_string));
  }
  //////////////////////////

  // source links and sink_links in kanthi's code are net device containers
  ndc[i] = p2p.Install (nc[i]);
  // create them here

  // set it as switch
  Ptr<PointToPointNetDevice> nd = StaticCast<PointToPointNetDevice> ((ndc[i]).Get(0));
  Ptr<Queue> queue = nd->GetQueue ();
  uint32_t nid = (nd->GetNode())->GetId(); 
  std::cout<<"Node id is "<<(nd->GetNode())->GetId()<<std::endl;
  AllQueues.push_back(queue);

  // the other end
  Ptr<PointToPointNetDevice> nd1 = StaticCast<PointToPointNetDevice> ((ndc[i]).Get(1));
  Ptr<Queue> queue1 = nd1->GetQueue ();
  uint32_t nid1 = (nd1->GetNode())->GetId(); 
  std::cout<<"Node id is "<<(nd1->GetNode())->GetId()<<std::endl;

  AllQueues.push_back(queue1);
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

}

// KN version
/* assign ip address */
std::vector<Ipv4InterfaceContainer> IP_ADDR(totlinks);
uint32_t cur_subnet = 0;

for (int i = 0; i < totlinks; i++)
{
     IP_ADDR[i] = assignAddress(dev_cont[i], cur_subnet);
}

// SC: ask kanthi here; DEFAULT TOPOSIM
// it creates little subnets, one for each couple of nodes.

//NS_LOG_INFO ("creating ipv4 interfaces");
//Ipv4InterfaceContainer* ipic = new Ipv4InterfaceContainer[totlinks];
//for (int i = 0; i < totlinks; i++)
//{
//  ipic[i] = address.Assign (ndc[i]);
//  address.NewNetwork ();
//}


//Turn on global static routing
Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
