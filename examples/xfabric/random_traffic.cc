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


uint32_t global_flow_id = 1;
std::map<uint32_t, uint32_t> flow_known;

bool compare_flow_deadlines(const FlowData &a, const FlowData &b)
{
  if(a.flow_deadline < b.flow_deadline) {
    return true;
  }
  return false;
}

void config_queue(Ptr<Queue> Q, uint32_t nid, uint32_t vpackets, std::string fkey1)
{
      Q->SetNodeID(nid);
      Q->SetLinkIDString(fkey1);
      Q->SetVPkts(vpackets);
}

void createTopology2(void)
{
  bottleNeckNode.Create(1);
  sourceNodes.Create(N/2);
  sinkNodes.Create(N/2);

  ports = new uint16_t [sinkNodes.GetN()];
   
  for (uint32_t i=0; i <sinkNodes.GetN(); i++) {
    ports[i] = 1;
  }
  allNodes = NodeContainer (bottleNeckNode, sourceNodes, sinkNodes);
  InternetStackHelper internet;
  internet.Install (allNodes);

  // We create the channels first without any IP addressing information
  //
  // Queue, Channel and link characteristics
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2pbottleneck;
  p2pbottleneck.SetDeviceAttribute ("DataRate", StringValue (link_rate_string));
  p2pbottleneck.SetChannelAttribute ("Delay", TimeValue(MicroSeconds(5.0)));
  

  if(queue_type == "WFQ") {
    std::cout<<"setting queue to WFQ"<<std::endl;
    p2pbottleneck.SetQueue("ns3::PrioQueue", "pFabric", StringValue("1"),"DataRate", StringValue(link_rate_string), "MaxBytes", UintegerValue(max_queue_size), "Mode", StringValue("QUEUE_MODE_BYTES"));
  } else if(queue_type == "FifoQueue") {
    std::cout<<"setting queue to FifoQueue"<<std::endl;
    p2pbottleneck.SetQueue("ns3::FifoQueue", "MaxBytes", UintegerValue(max_queue_size), "Mode", StringValue("QUEUE_MODE_BYTES"));
  } 

  // Create links between all sourcenodes and bottleneck switch
  //
  std::vector<NetDeviceContainer> source_links;
  std::vector<NetDeviceContainer> sink_links;
  std::vector<NetDeviceContainer> bnecklinks;

  for(uint32_t nid = 0; nid < sourceNodes.GetN(); nid++) {
    source_links.push_back(p2pbottleneck.Install(sourceNodes.Get(nid), bottleNeckNode.Get(0)));
    printlink(sourceNodes.Get(nid), bottleNeckNode.Get(0));
    Ptr<PointToPointNetDevice> ptr1(dynamic_cast<PointToPointNetDevice*>(PeekPointer(source_links[nid].Get(0))));
    NS_LOG_UNCOND("link data rate set to "<<ptr1->GetDataRate());
  }

  for(uint32_t nid = 0; nid < sinkNodes.GetN(); nid++) {
    sink_links.push_back(p2pbottleneck.Install(bottleNeckNode.Get(0), sinkNodes.Get(nid)));
    printlink(bottleNeckNode.Get(0), sinkNodes.Get(nid));
  }

//  bnecklinks.push_back(p2pbottleneck.Install(bottleNeckNode.Get(0), bottleNeckNode.Get(1))); //bottleneck link

  /* assign ip address */
  std::vector<Ipv4InterfaceContainer> sourceAdj(source_links.size());
  std::vector<Ipv4InterfaceContainer> sinkAdj(sink_links.size());
    
  uint32_t cur_subnet = 0;

  for (uint32_t index=0; index<2; index++) {
    std::vector<NetDeviceContainer> dev_cont;
    if(index == 0) {
      dev_cont = source_links;
    } else if (index==1) {
      dev_cont = sink_links;
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
    
     sourceAdj[i] = assignAddress(dev_cont[i], cur_subnet);
     cur_subnet++;
   }
  }

  //Turn on global static routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
}
  

void createTopology(void)
{
  bottleNeckNode.Create(2);
  sourceNodes.Create(N/2);
  sinkNodes.Create(N/2);

  ports = new uint16_t [sinkNodes.GetN()];
   
  for (uint32_t i=0; i <sinkNodes.GetN(); i++) {
    ports[i] = 1;
  }
  allNodes = NodeContainer (bottleNeckNode, sourceNodes, sinkNodes);
  InternetStackHelper internet;
  internet.Install (allNodes);

  // We create the channels first without any IP addressing information
  //
  // Queue, Channel and link characteristics
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2pbottleneck;
  p2pbottleneck.SetDeviceAttribute ("DataRate", StringValue (link_rate_string));
  p2pbottleneck.SetChannelAttribute ("Delay", TimeValue(MicroSeconds(5.0)));
  

  if(queue_type == "W2FQ") {
    std::cout<<"setting queue to W2FQ"<<std::endl;
    p2pbottleneck.SetQueue("ns3::W2FQ", "DataRate", StringValue(link_rate_string), "MaxBytes", UintegerValue(max_queue_size), "Mode", StringValue("QUEUE_MODE_BYTES"));
  } else if(queue_type == "WFQ") {
    std::cout<<"setting queue to WFQ"<<std::endl;
    p2pbottleneck.SetQueue("ns3::PrioQueue", "pFabric", StringValue("1"),"DataRate", StringValue(link_rate_string), "MaxBytes", UintegerValue(max_queue_size), "Mode", StringValue("QUEUE_MODE_BYTES"));
  } else if(queue_type == "FifoQueue") {
    std::cout<<"setting queue to FifoQueue"<<std::endl;
    p2pbottleneck.SetQueue("ns3::FifoQueue", "MaxBytes", UintegerValue(max_queue_size), "Mode", StringValue("QUEUE_MODE_BYTES"));
  } else if(queue_type == "hybridQ") {
    std::cout<<"setting queue to hybridQueue"<<std::endl;
    p2pbottleneck.SetQueue("ns3::hybridQ", "MaxBytes", UintegerValue(max_queue_size), "Mode", StringValue("QUEUE_MODE_BYTES"), "DataRate", StringValue(link_rate_string));
  } else if(queue_type == "fifo_hybridQ") {
    std::cout<<"setting queue to fifo_hybridQueue"<<std::endl;
    p2pbottleneck.SetQueue("ns3::fifo_hybridQ", "MaxBytes", UintegerValue(max_queue_size), "Mode", StringValue("QUEUE_MODE_BYTES"), "DataRate", StringValue(link_rate_string));
  }

  // Create links between all sourcenodes and bottleneck switch
  //
  std::vector<NetDeviceContainer> source_links;
  std::vector<NetDeviceContainer> sink_links;
  std::vector<NetDeviceContainer> bnecklinks;

  for(uint32_t nid = 0; nid < sourceNodes.GetN(); nid++) {
    source_links.push_back(p2pbottleneck.Install(sourceNodes.Get(nid), bottleNeckNode.Get(0)));
    printlink(sourceNodes.Get(nid), bottleNeckNode.Get(0));
    Ptr<PointToPointNetDevice> ptr1(dynamic_cast<PointToPointNetDevice*>(PeekPointer(source_links[nid].Get(0))));
    NS_LOG_UNCOND("link data rate set to "<<ptr1->GetDataRate());
  }

  for(uint32_t nid = 0; nid < sinkNodes.GetN(); nid++) {
    sink_links.push_back(p2pbottleneck.Install(bottleNeckNode.Get(1), sinkNodes.Get(nid)));
    printlink(bottleNeckNode.Get(0), sinkNodes.Get(nid));
  }

  bnecklinks.push_back(p2pbottleneck.Install(bottleNeckNode.Get(0), bottleNeckNode.Get(1))); //bottleneck link

  /* assign ip address */
  std::vector<Ipv4InterfaceContainer> sourceAdj(source_links.size());
  std::vector<Ipv4InterfaceContainer> sinkAdj(sink_links.size());
    
  uint32_t cur_subnet = 0;

  for (uint32_t index=0; index<3; index++) {
    std::vector<NetDeviceContainer> dev_cont;
    if(index == 0) {
      dev_cont = source_links;
    } else if (index==1) {
      dev_cont = sink_links;
    } else {
      dev_cont = bnecklinks;
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
    
     sourceAdj[i] = assignAddress(dev_cont[i], cur_subnet);
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
        if(queue_type == "W2FQ") {
          StaticCast<W2FQ> (q)->setFlowID(it->first, it->second, flowweights[it->second], flow_known[it->second]);
        }
        if(queue_type == "WFQ") {
          StaticCast<PrioQueue> (q)->setFlowID(it->first, it->second, flowweights[it->second], flow_known[it->second]);
        }
        if(queue_type == "hybridQ") {
          StaticCast<hybridQ> (q)->setFlowID(it->first, it->second, flowweights[it->second], flow_known[it->second]);
        }
        if(queue_type == "fifo_hybridQ") {
          StaticCast<fifo_hybridQ> (q)->setFlowID(it->first, it->second, flowweights[it->second], flow_known[it->second]);
        }
        
          
      }
    }
}


void startFlow(uint32_t sourceN, uint32_t sinkN, double flow_start, uint32_t flow_size, uint32_t flow_id, uint32_t flow_weight, uint32_t tcp, uint32_t known)
{
  ports[sinkN]++;
  // Socket at the source
  Ptr<Ipv4L3Protocol> sink_node_ipv4 = StaticCast<Ipv4L3Protocol> ((sinkNodes.Get(sinkN))->GetObject<Ipv4> ());
  Ipv4Address remoteIp = sink_node_ipv4->GetAddress (1,0).GetLocal();
  Address remoteAddress = (InetSocketAddress (remoteIp, ports[sinkN]));
  sinkInstallNode(sourceN, sinkN, ports[sinkN], flow_id, flow_start, flow_size, tcp);

  // Get source address
  Ptr<Ipv4L3Protocol> source_node_ipv4 = StaticCast<Ipv4L3Protocol> ((sourceNodes.Get(sourceN))->GetObject<Ipv4> ()); 
  Ipv4Address sourceIp = source_node_ipv4->GetAddress (1,0).GetLocal();
  Address sourceAddress = (InetSocketAddress (sourceIp, ports[sinkN]));
  Ptr<MyApp> SendingApp = CreateObject<MyApp> ();
  SendingApp->Setup (remoteAddress, pkt_size, DataRate (application_datarate), flow_size, flow_start, sourceAddress, sourceNodes.Get(sourceN), flow_id, sinkNodes.Get(sinkN), tcp, known);

  (sourceNodes.Get(sourceN))->AddApplication(SendingApp);
      
  Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((sourceNodes.Get(sourceN))->GetObject<Ipv4> ()); // Get Ipv4 instance of the node
  Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal();

  std::cout<<"FLOW_INFO source_node "<<(sourceNodes.Get(sourceN))->GetId()<<" sink_node "<<(sinkNodes.Get(sinkN))->GetId()<<" "<<addr<<":"<<remoteIp<<" flow_id "<<flow_id<<" start_time "<<flow_start<<" dest_port "<<ports[sinkN]<<" flow_size "<<flow_size<<" flow_weight" <<flow_weight<<std::endl;

  flow_known[flow_id] = known;  // whether this flow is part of known or unknown flow set
  (source_flow[(sourceNodes.Get(sourceN))->GetId()]).push_back(flow_id);
  (dest_flow[(sinkNodes.Get(sinkN))->GetId()]).push_back(flow_id);
  std::stringstream ss;
  ss<<addr<<":"<<remoteIp<<":"<<ports[sinkN];
  std::string s = ss.str(); 
  flowids[s] = flow_id;
  
  ipv4->setFlow(s, flow_id, flow_size, flow_weight);
  sink_node_ipv4->setFlow(s, flow_id, flow_size, flow_weight);

  sink_node_ipv4->setSimTime(sim_time);
  
  //flow_id++;
}


// SC new version: if flow size less than 1 MB, classify as unknown
// else consider it a known flow
void startRandomFlows(Ptr<EmpiricalRandomVariable> empirical_rand)
{
  double lambda = (link_rate * load ) / (meanflowsize*8.0);
  std::cout<<"lambda first "<<lambda<<" load "<<load<<std::endl;
  lambda = lambda / (sinkNodes.GetN() * sourceNodes.GetN()); 
  double avg_interarrival = 1/lambda;

  Ptr<ExponentialRandomVariable> exp = CreateObject<ExponentialRandomVariable> ();
  exp->SetAttribute("Mean", DoubleValue(avg_interarrival));

  std::cout<<"lambda is "<<lambda<<" denom "<<sourceNodes.GetN()<<" avg_interarrival "<<avg_interarrival<<" meanflowsize "<<meanflowsize<<" link_rate "<<link_rate<<" load "<<load<<std::endl;

  //uint32_t flow_id_zero = 1000;
  uint32_t flow_num = global_flow_id;
   
  for (uint32_t i=0; i < sourceNodes.GetN(); i++) 
  {
    for(uint32_t j=0; j < sinkNodes.GetN(); j++) 
    {
      double flow_start_time = 0.0;
      double time_now = 1.0;
     
      uint32_t flow_counter = 0;
      while(time_now < (sim_time-0.1))
      {
        // flow size 
        double flow_size = empirical_rand->GetValue(); 
        double inter_arrival = exp->GetValue();
        flow_start_time = time_now + inter_arrival;
        std::cout<<"next arrival after "<<inter_arrival<<" flow_start_time "<<flow_start_time<<std::endl;
        time_now = flow_start_time; // is this right ?

        uint32_t flow_weight = 1.0; 

        flows_tcp=1;
        uint32_t known = 1;

        startFlow(i, j, flow_start_time, flow_size, flow_num, flow_weight, flows_tcp, known); 
        flow_num++;
        flow_counter++;
        
      } // end while
    }
  }

}


void setUpTraffic()
{

  // SC: sample from DCTCP full again
  NS_LOG_UNCOND("EmpiricalRandSetup : file "<<empirical_dist_file);
  Ptr<EmpiricalRandomVariable> x = SetUpEmpirical(empirical_dist_file);
  meanflowsize = x->avg();
  NS_LOG_UNCOND("Avg of empirical values.. "<<meanflowsize);
  startRandomFlows(x);

  /////////////////////////////////////////////////
  // SC: heavy DCTCP traffic for known flows
  /*
  NS_LOG_UNCOND("EmpiricalRandSetup_DCTCP_heavy : file "<< empirical_dist_file_DCTCP_heavy);
  Ptr<EmpiricalRandomVariable> x_DCTCP_heavy = SetUpEmpirical(empirical_dist_file_DCTCP_heavy);
  meanflowsize = x_DCTCP_heavy->avg();
  NS_LOG_UNCOND("Avg of empirical DCTCP heavy values.. "<< meanflowsize);

  // known "random" flows in foreground: sampled from heavy parts of DCTCP
  uint32_t known = 1;
  double random_load = (1.0 - load);
  std::cout<<"HEAVY known "<< known << " random load " << random_load << " load " << load << std::endl;
   
  startRandomFlows(x_DCTCP_heavy, known, random_load);

  /////////////////////////////////////////////////
  // SC: light DCTCP traffic for unknown flows
  // unknown random flows in background, sampled from light parts of DCTCP
  
  NS_LOG_UNCOND("EmpiricalRandSetup_DCTCP_light : file "<< empirical_dist_file_DCTCP_light);
  Ptr<EmpiricalRandomVariable> x_DCTCP_light = SetUpEmpirical(empirical_dist_file_DCTCP_light);
  meanflowsize = x_DCTCP_light->avg();
  NS_LOG_UNCOND("Avg of empirical DCTCP light values.. "<< meanflowsize);

  known = 0;
  random_load = load; 

  std::cout<<"LIGHT known "<< known << " random load " << random_load << " load " << load << std::endl;
  startRandomFlows(x_DCTCP_light, known, random_load);
  */
    
} 
   
Ptr<EmpiricalRandomVariable>  SetUpEmpirical(std::string fname)
{
  Ptr<EmpiricalRandomVariable> x = CreateObject<EmpiricalRandomVariable> ();
  std::ifstream myfile (fname.c_str(),  std::ifstream::in);
  NS_LOG_UNCOND("SetUpEmpirical... ");
  if (myfile.is_open())
  {
    double val, one, prob;

    while(myfile >> val >> one >> prob)
    {
    /*while ( getline (myfile,line) )
      const char *myString = line.c_str();
      NS_LOG_UNCOND("myString is "<<myString);
      char *p = strtok(myString, " ");
      double val =  std::stod(p, &sz);
      char *one = strtok(NULL, " ");
      char *q = strtok(NULL, " ");
      double prob = std::stod(q, &sz);  */
      
      NS_LOG_UNCOND("EmpiricalRandSetup val = "<<val<<" prob = "<<prob<<" one "<<one);
      
      x->CDF(val, prob); 
    }
    myfile.close();
  } else {
    NS_LOG_UNCOND("EmpiricalRandSetup. File not found "<<fname );
  }
  return x;
}
/*
void changeWeights(void)
{
  uint32_t N = allNodes.GetN(); 
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
  double total_weight = 0.0;
  std::map<uint32_t, double> flow_weight_local;
  double min_weight = 100.0;
  for(uint32_t nid=0; nid < N ; nid++)
  {
    Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(nid))->GetObject<Ipv4> ());
    std::map<std::string,uint32_t>::iterator it;
    for (std::map<std::string,uint32_t>::iterator it=ipv4->flowids.begin(); it!=ipv4->flowids.end(); ++it)
    {
       
      uint32_t s = it->second;
      if(flow_known[s] == 0) { //unknown flow - no weight or rate
        continue;
      }

      // check if this flowid is from this source 
      if (std::find((source_flow[nid]).begin(), (source_flow[nid]).end(), s)!=(source_flow[nid]).end()) {
        uint32_t rand_num = uv->GetInteger(1.0, 10.0);
        double new_weight = rand_num*1.0;
        flow_weight_local[s] = new_weight;
        if(new_weight < min_weight) {
          min_weight = new_weight;
        }
      } // end if flow is sourced here
    } // end for all flows registered with this node
  } // end for all nodes

  // start a new loop to normalize weights, if configured to do so 
  for(uint32_t nid=0; nid < N ; nid++)
  {
    Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(nid))->GetObject<Ipv4> ());
    std::map<std::string,uint32_t>::iterator it;
    for (std::map<std::string,uint32_t>::iterator it=ipv4->flowids.begin(); it!=ipv4->flowids.end(); ++it)
    {
       
      uint32_t s = it->second;
      if(flow_known[s] == 0) { //unknown flow - no weight or rate
        continue;
      }

      // check if this flowid is from this source 
      if (std::find((source_flow[nid]).begin(), (source_flow[nid]).end(), s)!=(source_flow[nid]).end()) {
        double new_weight = flow_weight_local[s];
        if(weight_normalized) {
          new_weight = new_weight/min_weight;
        }
        flow_weight_local[s]  = new_weight;
 
        
        std::cout<<" setting weight of flow "<<s<<" at node "<<nid<<" to "<<new_weight<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
        total_weight += new_weight;
        ipv4->setFlowWeight(s, new_weight);
        flowweights[s] = new_weight;
      }
    }
  }

  std::cout<<"BASE RATE "<<Simulator::Now().GetSeconds()<<" "<<(1.0/total_weight)*link_rate<<std::endl; 
  // get the right allocation 
  for(std::map<uint32_t, double>::iterator it = flow_weight_local.begin(); it != flow_weight_local.end(); ++it)
  {
    uint32_t fid = it->first;
    double weight = flow_weight_local[fid];

    double rate = (weight/total_weight) * (1 - controller_estimated_unknown_load) * link_rate;
    for(uint32_t nid=0; nid < N ; nid++)
    {
      Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(nid))->GetObject<Ipv4> ());
      if (std::find((source_flow[nid]).begin(), (source_flow[nid]).end(), fid)!=(source_flow[nid]).end()) {
        std::cout<<"TrueRate "<<Simulator::Now().GetSeconds()<<" "<<fid<<" "<<rate<<std::endl;
        ipv4->setFlowIdealRate(fid, rate);
      }
    }
  }
  setQFlows();
  // check queue size every 1/1000 of a second
  Simulator::Schedule (Seconds (0.2), &changeWeights);
}


void setUpWeightChange(void)
{
  Simulator::Schedule (Seconds (1.0), &changeWeights);
}

*/
void startFlowsStatic(void)
{

  uint32_t flow_num = 1;

   
  for (uint32_t i=0; i < sourceNodes.GetN(); i++) 
  {
    for(uint32_t j=0; j < sinkNodes.GetN(); j++) 
    {
  //    uint32_t j = i;
      double flow_start_time = 0.0;
      double time_now = 1.0;
      uint32_t flow_counter = 0;
     
     while(flow_counter < flows_per_host)
  //    while(flow_num < 3)
      {
        // flow size 
        double flow_size = 12500000000; 
        flow_start_time = time_now + flow_num * 0.0001;
        std::cout<<"flow between "<<(sourceNodes.Get(i))->GetId()<<" and "<<(sinkNodes.Get(j))->GetId()<<" starting at time "<<flow_start_time<<" of size "<<flow_size<<" flow_num "<<flow_num<<std::endl;
        //uint32_t flow_weight = 1.0 * flow_num;
        uint32_t flow_weight = 1.0; // * flow_num;
        uint32_t known = 1;
          
        startFlow(i, j, flow_start_time, flow_size, flow_num, flow_weight, flows_tcp, known); 
        flow_num++;
        flow_counter++;
      }
    }
  }

  uint32_t num_ports = sourceNodes.GetN() + sinkNodes.GetN();
  std::cout<<"num_ports "<<num_ports<<std::endl;
  std::cout<<"num_flows "<<(flow_num-1)<<std::endl;

  global_flow_id = flow_num;

}


void rocket_createTopology(void)
{
    std::string format ("Rocketfuel");
    //std::string input ("src/topology-read/examples/RocketFuel_toposample_1239_weights.txt");
    //std::string input ("src/topology-read/examples/rocket.test.txt");
    
    // telstra
    //std::string input ("src/topology-read/examples/latencies.intra");
    
    // BT Europe
    //std::string input ("topologies/BT_Europe.txt");
    //std::string input ("topologies/small_topo.txt");
    //std::string input ("topologies/ring.txt");
    
    //std::string input ("topologies/Iinet.hand.txt");
    std::string input ("topologies/Iinet.noLoop.txt");
    //std::string input ("topologies/dumbbell.txt");
    //std::string input ("topologies/ring.txt");

    ns3::TopologyReaderHelper topoHelp;
    topoHelp.SetFileName (input);
    topoHelp.SetFileType (format);
    Ptr<TopologyReader> inFile = topoHelp.GetTopologyReader ();

    //NodeContainer allNodes;

    if (inFile != 0)
    {
      allNodes = inFile->Read ();
    }

    if (inFile->LinksSize () == 0)
    {
      NS_LOG_ERROR ("Problems reading the topology file. Failing.");
    }

    int totlinks = inFile->LinksSize ();
    std::cout <<" ROCKET : totlinks "<< totlinks << std::endl;

    uint32_t totalNodes = allNodes.GetN ();
    std::cout <<" ROCKET : totalNodes "<< totalNodes << std::endl;

    N = totalNodes;

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

    for ( unsigned int i = 0; i < allNodes.GetN (); i++ )
    {
      
      unsigned int randomServerNumber = unifRandom->GetInteger (0, 10);

      if (randomServerNumber <= 5) {
         // source node
        source_sink_assignment[i] = 0.0;
        num_sources++;
	std::cout <<"source node: "<< i << std::endl;


      } else {
        // sink node
        source_sink_assignment[i] = 1.0;
        num_sinks++;
	std::cout <<"sink node: "<< i << std::endl;
      
      }
    }

    //NodeContainer* sourceNodes = new NodeContainer[num_sources];
    //NodeContainer* sinkNodes = new NodeContainer[num_sinks];

    // works
    //NodeContainer sourceNodes;
    //NodeContainer sinkNodes;

    std::cout<<"num_sources is "<< num_sources << std::endl;
    std::cout<<"num_sinks is "<< num_sinks << std::endl;
    
    //std::vector<NodeContainer> sourceNodes;
    //std::vector<NodeContainer> sinkNodes;


    // ASSIGNMENT IS A PROBLEM
    for ( unsigned int i = 0; i < allNodes.GetN (); i++ )
    {
      if (source_sink_assignment[i] == 0.0) {
        
        // sourceNodes.push_back(nodes.Get(i));
        // sourceNodes.Get(i) = nodes.Get(i);
        sourceNodes.Add(allNodes.Get(i));
      
      } else {
        
        // sinkNodes.push_back(nodes.Get(i));
        // sinkNodes.Get(i) = nodes.Get(i);
        sinkNodes.Add(allNodes.Get(i));
      }
    }
  
    std::cout<<"assigned sources and sinks "<< sourceNodes.GetN()<<" "<<sinkNodes.GetN()<<" "<<allNodes.GetN()<<std::endl;

//    for ( unsigned int i = 0; i < nodes.GetN (); i++ )
//    {
//      if (source_sink_assignment[i] == 0.0) {
//        sourceNodes[i] = nodes[i];
//      } else {
//        sinkNodes[i] = nodes[i];
//      }
//    }
    
    // kanthis code verbatim: get ports and install internet
    ports = new uint16_t [sinkNodes.GetN()];
    for (uint32_t i=0; i <sinkNodes.GetN(); i++) {
        ports[i] = 1;
    }

    InternetStackHelper internet;
    internet.Install (allNodes);

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
//    std::vector<Ipv4InterfaceContainer> IP_ADDR(totlinks);
//    uint32_t cur_subnet = 0;
//
//    for (int i = 0; i < totlinks; i++)
//    {
//         IP_ADDR[i] = assignAddress(ndc[i], cur_subnet);
//         cur_subnet++;
//    
//    }
    
    // SC: ask kanthi here; DEFAULT TOPOSIM
    // it creates little subnets, one for each couple of nodes.


  NS_LOG_INFO ("creating ip4 addresses");
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.252");

    NS_LOG_INFO ("creating ipv4 interfaces");
    Ipv4InterfaceContainer* ipic = new Ipv4InterfaceContainer[totlinks];
    for (int i = 0; i < totlinks; i++)
    {
      ipic[i] = address.Assign (ndc[i]);
      address.NewNetwork ();
    }

    ////Turn on global static routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
}








int
main(int argc, char *argv[])
{

  //LogComponentEnable("TcpSocketBase", LOG_LEVEL_ALL);
  CommandLine cmd = addCmdOptions();
  cmd.Parse (argc, argv);
  common_config(); 

  std::cout<<"print me "<<std::endl;
  std::cout<<*argv<<std::endl;
   std::cout<<"set prefix to "<<prefix<<std::endl;
 // initAll();

  if(deadline_mode){
    Ptr<ExponentialRandomVariable> deadline_exp;    
    Ptr<UniformRandomVariable> deadline_decision;    
    std::cout<<"DEADLINE TRUE"<<std::endl; 
    //deadline_exp, deadline_decision = generateDeadlineRV();
  }


  std::cout<<"PARAMS load " << load << " deadline_mean " << deadline_mean << " scheduler_mode " << scheduler_mode_edf << std::endl; 

  
  
  createTopology2();
  setUpTraffic();
  setUpMonitoring();


  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
