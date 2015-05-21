#include "declarations.h"
#include "sending_app.h"

NS_LOG_COMPONENT_DEFINE ("pfabric");

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
  uint32_t queue_id = 1;

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

      // first queue 
     if(queue_type == "WFQ") {
      StaticCast<PrioQueue> (queue)->SetNodeID(nid);
      StaticCast<PrioQueue> (queue)->SetLinkIDString(fkey1);
     } else if(queue_type == "W2FQ") {
      StaticCast<W2FQ> (queue)->SetNodeID(nid);
      StaticCast<W2FQ> (queue)->SetLinkIDString(fkey1);
     }
      
      queue_id++;
//      BooleanValue is_switch;
//      StaticCast<PrioQueue> (queue)->SetAttribute("is_switch", BooleanValue("true"));
//      std::cout<<"Set node "<<nid<<" as switch"<<std::endl;
      Simulator::Schedule (Seconds (1.0), &CheckQueueSize, queue);
//      StaticCast<PrioQueue> (queue)->GetAttribute("is_switch", is_switch);
      std::cout<<"Set the queue id "<<queue_id<<" to queue between "<<nid<<" and "<<nid1<<std::endl;
     if(queue_type == "WFQ") {
      StaticCast<PrioQueue> (queue1)->SetNodeID(nid1);
      StaticCast<PrioQueue> (queue1)->SetLinkIDString(fkey2);
     } else if(queue_type == "W2FQ") {
      StaticCast<W2FQ> (queue1)->SetNodeID(nid1);
      StaticCast<W2FQ> (queue1)->SetLinkIDString(fkey2);
     }

 //    StaticCast<PrioQueue> (queue1)->SetAttribute("is_switch", BooleanValue("true"));
 //    NS_LOG_UNCOND("Set node "<<nid1<<" as switch");
     Simulator::Schedule (Seconds (1.0), &CheckQueueSize, queue1);
//     StaticCast<PrioQueue> (queue1)->GetAttribute("is_switch", is_switch);
     //
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
  if(queue_type == "W2FQ") {
    for(uint32_t i=0; i<AllQueues.size(); i++) {
      Ptr<Queue> q = AllQueues[i];
      for(std::map<std::string, uint32_t>::iterator it=flowids.begin(); it != flowids.end(); ++it) {
        StaticCast<W2FQ> (q)->setFlowID(it->first, it->second);
      }
    }
  }
}

void startFlow(uint32_t sourceN, uint32_t sinkN, double flow_start, uint32_t flow_size, uint32_t flow_id, uint32_t flow_weight)
{
  ports[sinkN]++;
  // Socket at the source
  Ptr<Ipv4L3Protocol> sink_node_ipv4 = StaticCast<Ipv4L3Protocol> ((sinkNodes.Get(sinkN))->GetObject<Ipv4> ());
  Ipv4Address remoteIp = sink_node_ipv4->GetAddress (1,0).GetLocal();
  Address remoteAddress = (InetSocketAddress (remoteIp, ports[sinkN]));
  sinkInstallNode(sourceN, sinkN, ports[sinkN], flow_id, flow_start, flow_size);

  // Get source address
  Ptr<Ipv4L3Protocol> source_node_ipv4 = StaticCast<Ipv4L3Protocol> ((sourceNodes.Get(sourceN))->GetObject<Ipv4> ()); 
  Ipv4Address sourceIp = source_node_ipv4->GetAddress (1,0).GetLocal();
  Address sourceAddress = (InetSocketAddress (sourceIp, ports[sinkN]));

  //Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (sourceNodes.Get(sourceN), TcpSocketFactory::GetTypeId ());
//  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (sourceNodes.Get(sourceN), TcpSocketFactory::GetTypeId ());
//  ns3TcpSockets.push_back(ns3TcpSocket);
//  NS_LOG_UNCOND("number of sockets at node "<<sourceNodes.Get(sourceN)->GetId()<<" = "<<ns3TcpSockets.size());
  Ptr<MyApp> SendingApp = CreateObject<MyApp> ();
  //SendingApp->Setup (ns3TcpSocket, remoteAddress, pkt_size, DataRate ("1Gbps"), flow_size, flow_start, sourceAddress, sourceNodes.Get(sourceN));
  SendingApp->Setup (remoteAddress, pkt_size, DataRate (application_datarate), flow_size, flow_start, sourceAddress, sourceNodes.Get(sourceN), flow_id, sinkNodes.Get(sinkN));
  //apps.Add(SendingApp);
  (sourceNodes.Get(sourceN))->AddApplication(SendingApp);
      
  Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((sourceNodes.Get(sourceN))->GetObject<Ipv4> ()); // Get Ipv4 instance of the node
  Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal();

  std::cout<<"FLOW_INFO source_node "<<(sourceNodes.Get(sourceN))->GetId()<<" sink_node "<<(sinkNodes.Get(sinkN))->GetId()<<" "<<addr<<":"<<remoteIp<<" flow_id "<<flow_id<<" start_time "<<flow_start<<" dest_port "<<ports[sinkN]<<" flow_size "<<flow_size<<" flow_weight" <<flow_weight<<std::endl;
  (source_flow[(sourceNodes.Get(sourceN))->GetId()]).push_back(flow_id);
  (dest_flow[(sinkNodes.Get(sinkN))->GetId()]).push_back(flow_id);
  std::stringstream ss;
  ss<<addr<<":"<<remoteIp<<":"<<ports[sinkN];
  std::string s = ss.str(); 
  flowids[s] = flow_id;
  
  ipv4->setFlow(s, flow_id, flow_size, flow_weight);
  sink_node_ipv4->setFlow(s, flow_id, flow_size, flow_weight);
  
  //flow_id++;
}

void changeWeights(void)
{
  uint32_t N = allNodes.GetN(); 
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
  double total_weight = 0.0;
  std::map<uint32_t, double> flow_weight_local;
  for(uint32_t nid=0; nid < N ; nid++)
  {
    Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(nid))->GetObject<Ipv4> ());
    std::map<std::string,uint32_t>::iterator it;
    for (std::map<std::string,uint32_t>::iterator it=ipv4->flowids.begin(); it!=ipv4->flowids.end(); ++it)
    {
       
      uint32_t s = it->second;

      /* check if this flowid is from this source */
      if (std::find((source_flow[nid]).begin(), (source_flow[nid]).end(), s)!=(source_flow[nid]).end()) {
        uint32_t rand_num = uv->GetInteger(1.0, 10.0);
        double new_weight = rand_num*1.0;
        //double new_weight = s*1.0;
        std::cout<<" setting weight of flow "<<s<<" at node "<<nid<<" to "<<new_weight<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
        flow_weight_local[s] = new_weight;
        total_weight += new_weight;
        ipv4->setFlowWeight(s, new_weight);
      }
    }
  }
 
  // get the right allocation 
  for(std::map<uint32_t, double>::iterator it = flow_weight_local.begin(); it != flow_weight_local.end(); ++it)
  {
    uint32_t fid = it->first;
    double weight = flow_weight_local[fid];

    double rate = (weight/total_weight) * link_rate;
    for(uint32_t nid=0; nid < N ; nid++)
    {
      Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(nid))->GetObject<Ipv4> ());
      if (std::find((source_flow[nid]).begin(), (source_flow[nid]).end(), fid)!=(source_flow[nid]).end()) {
        std::cout<<"TrueRate "<<Simulator::Now().GetSeconds()<<" "<<fid<<" "<<rate<<std::endl;
        ipv4->setFlowIdealRate(fid, rate);
      }
    }
  }

  // check queue size every 1/1000 of a second
  Simulator::Schedule (Seconds (0.2), &changeWeights);
}


void startFlowsDynamic(Ptr<EmpiricalRandomVariable> empirical_rand)
{
  double lambda = (link_rate * load ) / (meanflowsize*8.0);
  lambda = lambda / sourceNodes.GetN(); 
  double avg_interarrival = 1/lambda;

  Ptr<ExponentialRandomVariable> exp = CreateObject<ExponentialRandomVariable> ();
  exp->SetAttribute("Mean", DoubleValue(avg_interarrival));
  NS_LOG_UNCOND("lambda is "<<lambda<<" denom "<<sourceNodes.GetN()<<" avg_interarrival "<<avg_interarrival);

  uint32_t flow_num = 1;

   
  for (uint32_t i=0; i < sourceNodes.GetN(); i++) 
  {
    for(uint32_t j=0; j < sinkNodes.GetN(); j++) 
    {
      double flow_start_time = 0.0;
      double time_now = 1.0;
     
      while(time_now < (sim_time-1.0))
      {
        // flow size 
        double flow_size = empirical_rand->GetValue(); 
        double inter_arrival = exp->GetValue();
        flow_start_time = time_now + inter_arrival;
        NS_LOG_UNCOND("next arrival after "<<inter_arrival<<" flow_start_time "<<flow_start_time);
        time_now = flow_start_time; // is this right ?
        NS_LOG_UNCOND("flow between "<<(sourceNodes.Get(i))->GetId()<<" and "<<(sinkNodes.Get(j))->GetId()<<" starting at time "<<flow_start_time<<" of size "<<flow_size<<" flow_num "<<flow_num);
        uint32_t flow_weight = 1.0 * flow_num;

        startFlow(i, j, flow_start_time, flow_size, flow_num, flow_weight); 
        flow_num++;
      }
    }
  }

  uint32_t num_ports = sourceNodes.GetN() + sinkNodes.GetN();
  std::cout<<"num_ports "<<num_ports<<std::endl;
  std::cout<<"num_flows "<<(flow_num-1)<<std::endl;
}


void startFlowsStatic(void)
{

  uint32_t flow_num = 1;

   
  for (uint32_t i=0; i < sourceNodes.GetN(); i++) 
  {
    for(uint32_t j=0; j < sinkNodes.GetN(); j++) 
    {
      double flow_start_time = 0.0;
      double time_now = 1.0;
      uint32_t flow_counter = 0;
     
    while(flow_counter < flows_per_host)
 //   while(flow_num < 3)
      {
        // flow size 
        double flow_size = 12500000000; 
        flow_start_time = time_now + 0.0001*flow_counter;
        NS_LOG_UNCOND("flow between "<<(sourceNodes.Get(i))->GetId()<<" and "<<(sinkNodes.Get(j))->GetId()<<" starting at time "<<flow_start_time<<" of size "<<flow_size<<" flow_num "<<flow_num);
        uint32_t flow_weight = 1.0 * flow_num;

        startFlow(i, j, flow_start_time, flow_size, flow_num, flow_weight); 
        flow_num++;
        flow_counter++;
      }
    }
  }

  uint32_t num_ports = sourceNodes.GetN() + sinkNodes.GetN();
  std::cout<<"num_ports "<<num_ports<<std::endl;
  std::cout<<"num_flows "<<(flow_num-1)<<std::endl;

  Simulator::Schedule (Seconds (1.0), &changeWeights);
}

void setUpTraffic()
{
  NS_LOG_UNCOND("EmpiricalRandSetup : file "<<empirical_dist_file);
  Ptr<EmpiricalRandomVariable> x = SetUpEmpirical(empirical_dist_file);
  meanflowsize = x->avg();
  NS_LOG_UNCOND("Avg of empirical values.. "<<meanflowsize);
  startFlowsStatic();
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

int
main(int argc, char *argv[])
{

  CommandLine cmd = addCmdOptions();
  cmd.Parse (argc, argv);
  common_config(); 

  std::cout<<*argv<<std::endl;
   std::cout<<"set prefix to "<<prefix<<std::endl;
 // initAll();
  createTopology();
  setUpTraffic();
  setUpMonitoring();
  setQFlows();
  
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
