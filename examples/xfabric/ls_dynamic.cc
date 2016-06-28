#include "declarations.h"
#include "sending_app.h"
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


NS_LOG_COMPONENT_DEFINE ("pfabric");

class MyApp;

const uint32_t max_system_flows = 10;
const uint32_t maxx = max_system_flows+1;
uint32_t flow_started[maxx] = {0};
Ptr<MyApp> sending_apps[maxx];
uint32_t num_flows = 0;
uint32_t min_flows_allowed = 7;
uint32_t max_flows_allowed = 9;

std::map<uint32_t, std::string> flowkeys;

//uint32_t min_flows_allowed = 8;
//uint32_t max_flows_allowed = 10;


void dropFlowFromQueues(uint32_t f)
{
    for(uint32_t i=0; i<AllQueues.size(); i++) {
      Ptr<Queue> q = AllQueues[i];
      StaticCast<PrioQueue> (q)->dropFlowPackets(flowkeys[f]);
    }
}
    

void config_queue(Ptr<Queue> Q, uint32_t nid, uint32_t vpackets, std::string fkey1)
{
      Q->SetNodeID(nid);
      Q->SetLinkIDString(fkey1);
      Q->SetVPkts(vpackets);
}

void createTopology(void)
{

  std::cout<<"Creating "<<num_spines<<" spines "<<num_leafs<<" leaves "<<num_hosts_per_leaf<<" hosts  per leaf "<<std::endl;
  hosts.Create(num_hosts_per_leaf*num_leafs);
  leafnodes.Create(num_leafs);
  spines.Create(num_spines);
  
  ports = new uint16_t [hosts.GetN()];
   
  for (uint32_t i=0; i <hosts.GetN(); i++) {
    ports[i] = 1;
  }
  allNodes = NodeContainer (hosts,  leafnodes, spines);
  InternetStackHelper internet;
  internet.Install (allNodes);

  // We create the channels first without any IP addressing information
  //
  // Queue, Channel and link characteristics
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper fabriclink;
  fabriclink.SetDeviceAttribute ("DataRate", StringValue (fabric_datarate));
  fabriclink.SetChannelAttribute ("Delay", TimeValue(MicroSeconds(fabricdelay)));
  
  PointToPointHelper edgelink;
  edgelink.SetDeviceAttribute ("DataRate", StringValue (edge_datarate));
  edgelink.SetChannelAttribute ("Delay", TimeValue(MicroSeconds(edgedelay)));


  if(queue_type == "WFQ") {
    std::cout<<"setting queue to WFQ"<<std::endl;
    fabriclink.SetQueue("ns3::PrioQueue", "pFabric", StringValue("1"),"DataRate", StringValue(fabric_datarate), "MaxBytes", UintegerValue(max_queue_size), "Mode", StringValue("QUEUE_MODE_BYTES"));
    edgelink.SetQueue("ns3::PrioQueue", "pFabric", StringValue("1"),"DataRate", StringValue(edge_datarate), "MaxBytes", UintegerValue(max_queue_size), "Mode", StringValue("QUEUE_MODE_BYTES"));
  }

  // Create links between all sourcenodes and bottleneck switch
  //
  std::vector<NetDeviceContainer> fabric_devs;
  std::vector<NetDeviceContainer> edge_devs;

  std::cout<<"Creating fabric links.."<<std::endl;

  for(uint32_t sidx = 0; sidx < spines.GetN(); sidx++) {
    for(uint32_t lidx = 0; lidx < leafnodes.GetN(); lidx++) {
      NetDeviceContainer net_dev = fabriclink.Install(spines.Get(sidx), leafnodes.Get(lidx));
      fabric_devs.push_back(net_dev);

      // debug information
      printlink(spines.Get(sidx), leafnodes.Get(lidx));
      Ptr<PointToPointNetDevice> ptr1(dynamic_cast<PointToPointNetDevice*>(PeekPointer(net_dev.Get(0))));
      std::cout<<" fabric link datarate "<<ptr1->GetDataRate()<<std::endl;
      
    }
  }

  std::cout<<"Creating edge links.."<<std::endl;

  
  for(uint32_t lidx = 0; lidx < leafnodes.GetN(); lidx++) {
    uint32_t start_index = lidx * num_hosts_per_leaf;
    for(uint32_t hidx = start_index; hidx < start_index + num_hosts_per_leaf; hidx++) {
      NetDeviceContainer net_dev = edgelink.Install(leafnodes.Get(lidx), hosts.Get(hidx));
      edge_devs.push_back(net_dev);
      // debug information
      printlink(leafnodes.Get(lidx), hosts.Get(hidx));
      Ptr<PointToPointNetDevice> ptr1(dynamic_cast<PointToPointNetDevice*>(PeekPointer(net_dev.Get(0))));
      std::cout<<" edge link datarate "<<ptr1->GetDataRate()<<std::endl;
      
    }
  }

  uint32_t cur_subnet = 0;

  for (uint32_t index=0; index<2; index++) {
    std::vector<NetDeviceContainer> dev_cont;
    if(index == 0) {
      dev_cont = fabric_devs;
    } else if (index==1) {
      dev_cont = edge_devs;
    }

    for(uint32_t i=0; i < dev_cont.size(); ++i)
    {
      // set it as switch
      Ptr<PointToPointNetDevice> nd = StaticCast<PointToPointNetDevice> ((dev_cont[i]).Get(0));
      Ptr<Queue> queue = nd->GetQueue ();
      uint32_t nid = (nd->GetNode())->GetId(); 
      std::cout<<"Node id is "<<(nd->GetNode())->GetId()<<std::endl;
      AllQueues.push_back(queue);

      // the other end
      Ptr<PointToPointNetDevice> nd1 = StaticCast<PointToPointNetDevice> ((dev_cont[i]).Get(1));
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
     

     // assign ip address
     assignAddress(dev_cont[i], cur_subnet);
     cur_subnet++;
   }
  }

  //Turn on global static routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
}

void setQFlows()
{
    for(uint32_t i=0; i<AllQueues.size(); i++) {
      Ptr<Queue> q = AllQueues[i];
      for(std::map<std::string, uint32_t>::iterator it=flowids.begin(); it != flowids.end(); ++it) {
      //  std::cout<<"setQFlows flow "<<it->second<<" is "<<flow_known[it->second]<<std::endl;
        /*if(queue_type == "W2FQ") {
          StaticCast<W2FQ> (q)->setFlowID(it->first, it->second, flowweights[it->second])
        }*/
        if(queue_type == "WFQ") {
          StaticCast<PrioQueue> (q)->setFlowID(it->first, it->second, flowweights[it->second]);
        }
          
      }
    }
}


Ptr<MyApp> startFlow(uint32_t sourceN, uint32_t sinkN, double flow_start, uint32_t flow_size, uint32_t flow_id, uint32_t rand_weight)
{

    ports[sinkN]++;
    Ptr<Ipv4L3Protocol> sink_node_ipv4 = StaticCast<Ipv4L3Protocol> ((sinkNodes.Get(sinkN))->GetObject<Ipv4> ());
    Ipv4Address remoteIp = sink_node_ipv4->GetAddress (1,0).GetLocal();
    Address remoteAddress = (InetSocketAddress (remoteIp, ports[sinkN]));
    Ptr<Ipv4> source_node_ipv4 = (sourceNodes.Get(sourceN))->GetObject<Ipv4> (); 
    Ipv4Address sourceIp = source_node_ipv4->GetAddress (1,0).GetLocal();
    Address sourceAddress = (InetSocketAddress (sourceIp, ports[sinkN]));
    // Socket at the source
    //sinkInstallNode(sourceN, sinkN, ports[sinkN], flow_id, flow_start, flow_size, clientNodes);
    sinkInstallNode(sourceN, sinkN, ports[sinkN], flow_id, flow_start, flow_size, 1);


    Ptr<MyApp> SendingApp = CreateObject<MyApp> ();
    SendingApp->Setup (remoteAddress, pkt_size, DataRate (application_datarate), flow_size, flow_start, sourceAddress, sourceNodes.Get(sourceN), flow_id, sinkNodes.Get(sinkN), rand_weight);
    (sourceNodes.Get(sourceN))->AddApplication(SendingApp);
    Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((sourceNodes.Get(sourceN))->GetObject<Ipv4> ()); // Get Ipv4 instance of the node
    Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal();

    (source_flow[(sourceNodes.Get(sourceN))->GetId()]).push_back(flow_id);
    (dest_flow[(sinkNodes.Get(sinkN))->GetId()]).push_back(flow_id);
    std::stringstream ss;
    ss<<addr<<":"<<remoteIp<<":"<<ports[sinkN];
    std::string s = ss.str(); 
    flowids[s] = flow_id;
    flowkeys[flow_id] = s;

    //flow_dest_port[clientNodes.Get(sourceN)->GetId()] = ports[sinkN];
    //

    ipv4->setFlow(s, flow_id, flow_size, rand_weight);
    sink_node_ipv4->setFlow(s, flow_id, flow_size, rand_weight);
      
  std::cout<<"FLOW_INFO source_node "<<(sourceNodes.Get(sourceN))->GetId()<<" sink_node "<<(sinkNodes.Get(sinkN))->GetId()<<" "<<addr<<":"<<remoteIp<<" flow_id "<<flow_id<<" start_time "<<flow_start<<" dest_port "<<ports[sinkN]<<" flow_size "<<flow_size<<" "<<rand_weight<<std::endl;
  //flow_id++;
  return SendingApp;
}

void splitHosts(void)
{

 /*   sourceNodes = hosts;
    sinkNodes = hosts;
    uint32_t num_sources = sourceNodes.GetN(), num_sinks = sinkNodes.GetN();
*/

/*    sourceNodes.Add(hosts.Get(0));
    sourceNodes.Add(hosts.Get(1));

    sinkNodes.Add(hosts.Get(2));
    sinkNodes.Add(hosts.Get(3));
*/
  
    uint32_t num_sources = 0, num_sinks = 0;
    Ptr<UniformRandomVariable> unifRandom = CreateObject<UniformRandomVariable> ();

    for ( unsigned int i = 0; i < hosts.GetN (); i++ )
    {
      //double randomServerNumber = unifRandom->GetValue(0,1.0);
      //if (randomServerNumber <= 0.5) {
      if (i < hosts.GetN()/2) {
         // source node
        sourceNodes.Add(hosts.Get(i));
        num_sources++;
      } else {
        sinkNodes.Add(hosts.Get(i));
        num_sinks++;
      }
    }  
    std::cout<<"num_sources is "<< num_sources << std::endl;
    std::cout<<"num_sinks is "<< num_sinks << std::endl; 
}
  
/*
void startFlowsStatic(void)
{

  uint32_t flow_num = 1;
  
  // pick a random source ; a random destination
  // and send traffic from source to destination

  // TBD - we may not need this but do this anyway
  // divide into sources and sinks
  splitHosts();

  Ptr<UniformRandomVariable> unifRandom = CreateObject<UniformRandomVariable> ();
  while(flow_num < max_system_flows) 
  {

    uint32_t src_index = unifRandom->GetInteger (0, sourceNodes.GetN()-1);
    uint32_t dst_index = src_index;

    uint32_t src_nid = sourceNodes.Get(src_index)->GetId(); 
    uint32_t dst_nid = src_nid; 
    
    

    while(dst_nid == src_nid) {
      std::cout<<" picked destination "<<dst_index<<std::endl;
      dst_index = unifRandom->GetInteger (0, sinkNodes.GetN()-1);
      dst_nid = sinkNodes.Get(dst_index)->GetId(); 
    }
    

    std::cout<<" randomly picked source "<<src_index<<" randomly picked destination "<<dst_index<<std::endl;  
    double flow_start_time = 0.0;
    double time_now = 1.0;
  
    double flow_size = 12500000000; 
    flow_start_time = time_now + 0.0001;

    std::cout<<"flow between hosts"<<(sourceNodes.Get(src_index))->GetId()<<" and "<<(sinkNodes.Get(dst_index))->GetId()<<" starting at time "<<flow_start_time<<" of size "<<flow_size<<" flow_num "<<flow_num<<std::endl;
    uint32_t flow_weight = 1.0;
      
    startFlow(src_index, dst_index, flow_start_time, flow_size, flow_num, flow_weight); 
    flow_num++;
   }
  std::cout<<"num_flows "<<(flow_num-1)<<std::endl;

}
*/

void stop_a_flow(std::vector<uint32_t> sourcenodes, std::vector<uint32_t> sinknodes)
{
  while (true) {
    UniformVariable urand;
    uint32_t i = urand.GetInteger(1, max_system_flows);
    std::cout<<"picked "<<i<<" to stop"<<std::endl;
    if(flow_started[i] == 1) {
      // stop the application
      sending_apps[i]->StopApplication();
      std::cout<<Simulator::Now().GetSeconds()<<" stopping flow "<<i<<std::endl;
      num_flows--;
      flow_started[i] = 0;

      // remove flow from ipv4 object to stop tracking rate
      uint32_t source_node = sourcenodes[i];
      uint32_t sink_node = sinknodes[i];
      Ptr<Ipv4L3Protocol> sink_node_ipv4 = StaticCast<Ipv4L3Protocol> ((sinkNodes.Get(sink_node))->GetObject<Ipv4> ());
      Ptr<Ipv4L3Protocol> src_node_ipv4 = StaticCast<Ipv4L3Protocol> ((sourceNodes.Get(source_node))->GetObject<Ipv4> ());
      sink_node_ipv4->removeFlow(i);
      src_node_ipv4->removeFlow(i);
      dropFlowFromQueues(i);
      break;
    }
  }
}

uint32_t start_flow_index = 1;

void start_a_flow(std::vector<uint32_t> sourcenodes, std::vector<uint32_t> sinknodes)
{
  
    while(true)
    {
     UniformVariable urand;
     //uint32_t i = start_flow_index; //urand.GetInteger(1, max_system_flows-1);
     //start_flow_index++;
     //start_flow_index = start_flow_index % max_flows_allowed;
     //
     uint32_t i = urand.GetInteger(1, max_system_flows-1);
     std::cout<<Simulator::Now()<<" start_a_flow "<<i<<" max_system_flows "<<max_system_flows<<std::endl;
     if(flow_started[i] == 0) { 

       double flow_size = 0;
       double flow_start = Simulator::Now().GetSeconds();
       double rand_weight = 1.0;
       Ptr<MyApp> sendingApp = startFlow(sourcenodes[i], sinknodes[i],flow_start, flow_size, i, rand_weight);
       sending_apps[i] = sendingApp;
        
       flow_started[i] = 1;
       num_flows++;
       break;
     }
  }
}


void startflowwrapper( std::vector<uint32_t> sourcenodes, std::vector<uint32_t> sinknodes)
{
  if(num_flows >= max_flows_allowed) {
    stop_a_flow(sourcenodes, sinknodes);
  } else if(num_flows < min_flows_allowed) {
    start_a_flow(sourcenodes, sinknodes);
  } else {
     Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
     double rand_num = uv->GetValue(0.0, 1.0);
     if(rand_num < 0.5) {
        start_a_flow(sourcenodes, sinknodes);
      } else {
        stop_a_flow(sourcenodes, sinknodes);
      }
  }


/*  double delay = 0.0;
  if(num_flows >= 7) {
     delay = 0.05; //start up the first 30 flows very fast
  }
  if(num_flows >=8) {
    delay = 5.0;
  } */
  double delay = 0.01;
  Simulator::Schedule (Seconds (delay), &startflowwrapper, sourcenodes, sinknodes);
}

Ptr<EmpiricalRandomVariable>  SetUpEmpirical(std::string fname)
{
  Ptr<EmpiricalRandomVariable> x = CreateObject<EmpiricalRandomVariable> ();
  std::ifstream myfile (fname.c_str(),  std::ifstream::in);
  if (myfile.is_open())
  {
    double val, one, prob;
    if(fname.compare("ENTERPRISE_CDF") == 0) {
        while(myfile >> val >> prob)
        {
          std::cout<<"EmpiricalRandSetup val = "<<val<<" prob = "<<prob<<std::endl;
          x->CDF(val, prob); 
        }
     } else {

        while(myfile >> val >> one >> prob)
        {
      
          std::cout<<"EmpiricalRandSetup val = "<<val<<" prob = "<<prob<<" one "<<one<<std::endl;
          x->CDF(val, prob); 
        }
     }
    myfile.close();
  } else {
    NS_LOG_UNCOND("EmpiricalRandSetup. File not found "<<fname );
  }
  return x;
}

void startRandomFlows()
{
  
  NS_LOG_UNCOND("EmpiricalRandSetup : file "<<empirical_dist_file);
  Ptr<EmpiricalRandomVariable> empirical_rand = SetUpEmpirical(empirical_dist_file);
  meanflowsize = empirical_rand->avg();
  NS_LOG_UNCOND("Avg of empirical values.. "<<meanflowsize);
  double lambda = (link_rate * load ) / (meanflowsize*8.0);
  std::cout<<"lambda first "<<lambda<<" load "<<load<<std::endl;
  //lambda = lambda / (sinkNodes.GetN() * sourceNodes.GetN()); 
  lambda = lambda / sourceNodes.GetN();
  double avg_interarrival = 1/lambda;

  Ptr<ExponentialRandomVariable> exp = CreateObject<ExponentialRandomVariable> ();
  exp->SetAttribute("Mean", DoubleValue(avg_interarrival));

  std::cout<<"lambda is "<<lambda<<" denom "<<sourceNodes.GetN()<<" avg_interarrival "<<avg_interarrival<<" meanflowsize "<<meanflowsize<<" link_rate "<<link_rate<<" load "<<load<<std::endl;

  uint32_t flow_num = 1;

  std::cout<<"num "<<sourceNodes.GetN()<<" "<<sinkNodes.GetN()<<std::endl;
   
  for (uint32_t i=0; i < sourceNodes.GetN(); i++) 
  {
    for(uint32_t j=0; j < sinkNodes.GetN(); j++) 
    {
      double flow_start_time = 0.0;
      double time_now = 1.0;
     
      uint32_t flow_counter = 0;
      while(time_now < (sim_time-3.0))
//      while(flow_num<2)
      {
        // flow size 
        uint32_t flow_size = empirical_rand->GetValue(); 
	while(flow_size == 0) {
		flow_size = empirical_rand->GetValue(); 
	}
        double inter_arrival = exp->GetValue();
        flow_start_time = time_now + inter_arrival;
        std::cout<<"next arrival after "<<inter_arrival<<" flow_start_time "<<flow_start_time<<" flow size "<<flow_size<<std::endl;
        time_now = flow_start_time;

        uint32_t flow_weight = 1.0; 

        startFlow(i, j, flow_start_time, flow_size, flow_num, flow_weight);
        flow_num++;
      } // end while
    }
  }

}

void setUpTraffic()
{
  splitHosts();
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();

  std::vector<uint32_t> sourcenodes;//(max_system_flows, 0);
  std::vector<uint32_t> sinknodes;//(max_system_flows, 0);

  std::cout<<" generating random source and destination pairs "<<std::endl;
  /* Generate max_system_flows number of random source destination pairs */
  for(uint32_t flow_idx=1; flow_idx<max_system_flows+1; flow_idx++) {
    
    uint32_t source_idx = uv->GetInteger(0, sourceNodes.GetN()-1);
    uint32_t sink_idx = uv->GetInteger(0, sinkNodes.GetN()-1);

    std::cout<<" added source "<<source_idx<<" sink "<<sink_idx<<" source nodes "<<sourceNodes.GetN()<<" flow_id "<<flow_idx<<std::endl;

    sourcenodes.push_back(source_idx);
    sinknodes.push_back(sink_idx);
  }

  std::cout<<" generated source node "<<sourcenodes.size()<<" sink nodes "<<sinknodes.size()<<std::endl;
  //Simulator::Schedule (Seconds (1.0), &startflowwrapper, sourcenodes, sinknodes);
  //startFlowsStatic();
  startRandomFlows();
}
 
int
main(int argc, char *argv[])
{
  CommandLine cmd = addCmdOptions();
  cmd.Parse (argc, argv);
  common_config(); 

  dump_config();

  std::cout<<"print me "<<std::endl;
  std::cout<<*argv<<std::endl;
  std::cout<<"set prefix to "<<prefix<<std::endl;
  
  //LogComponentEnable("TcpSocketBase", LOG_LEVEL_ALL);
 // initAll();

  if(deadline_mode){
    Ptr<ExponentialRandomVariable> deadline_exp;    
    Ptr<UniformRandomVariable> deadline_decision;    
    std::cout<<"DEADLINE TRUE"<<std::endl; 
    //deadline_exp, deadline_decision = generateDeadlineRV();
  }


  std::cout<<"PARAMS load " << load << " deadline_mean " << deadline_mean << " scheduler_mode " << scheduler_mode_edf << std::endl; 

  
//  rocket_createTopology();
  
  createTopology();
  setUpTraffic();
  setUpMonitoring();


//  if(weight_change) {
//    setUpWeightChange();
//  }

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
