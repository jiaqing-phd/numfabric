#include "declarations.h"
#include "sending_app.h"

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
//      while(flow_num < 3)
      {
        // flow size 
        double flow_size = 12500000000; 
        flow_start_time = time_now + 0.0001;
        NS_LOG_UNCOND("flow between "<<(sourceNodes.Get(i))->GetId()<<" and "<<(sinkNodes.Get(j))->GetId()<<" starting at time "<<flow_start_time<<" of size "<<flow_size<<" flow_num "<<flow_num);
        uint32_t flow_weight = 1.0 * flow_num;
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

void setUpTraffic()
{

  // SC: sample from DCTCP full again
  NS_LOG_UNCOND("EmpiricalRandSetup : file "<<empirical_dist_file);
  NS_LOG_UNCOND("Avg of empirical values.. "<<meanflowsize);
  startFlowsStatic();
} 
   

int
main(int argc, char *argv[])
{

  CommandLine cmd = addCmdOptions();
  cmd.Parse (argc, argv);
  common_config(); 

  std::cout<<"print me "<<std::endl;
  std::cout<<*argv<<std::endl;
   std::cout<<"set prefix to "<<prefix<<std::endl;
 // initAll();

  createTopology();
  setUpTraffic();
  setUpMonitoring();

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
