#include "declarations.h"
#include "sending_app.h"

NS_LOG_COMPONENT_DEFINE ("pfabric");

uint32_t global_flow_id = 0;
std::map<uint32_t, uint32_t> flow_known;

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
        if(queue_type == "W2FQ") {
          StaticCast<W2FQ> (q)->setFlowID(it->first, it->second, flowweights[it->second]);
        }
        if(queue_type == "WFQ") {
          StaticCast<PrioQueue> (q)->setFlowID(it->first, it->second, flowweights[it->second]);
        }
        if(queue_type == "hybridQ") {
          StaticCast<hybridQ> (q)->setFlowID(it->first, it->second, flowweights[it->second]);
        }
        if(queue_type == "fifo_hybridQ") {
          StaticCast<fifo_hybridQ> (q)->setFlowID(it->first, it->second, flowweights[it->second]);
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
  SendingApp->Setup (remoteAddress, pkt_size, DataRate (application_datarate), flow_size, flow_start, sourceAddress, sourceNodes.Get(sourceN), flow_id, sinkNodes.Get(sinkN), tcp);
  //apps.Add(SendingApp);
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
void changeAppRates(void)
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
      if(flow_known[s] == 0) { //unknown flow - no weight or rate
        continue;
      }

      /* check if this flowid is from this source */
      if (std::find((source_flow[nid]).begin(), (source_flow[nid]).end(), s)!=(source_flow[nid]).end()) {
        uint32_t rand_num = uv->GetInteger(1.0, 10.0);
        double new_weight = rand_num*1.0;
        std::cout<<" setting weight of flow "<<s<<" at node "<<nid<<" to "<<new_weight<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
        flow_weight_local[s] = new_weight;
        total_weight += new_weight;
        ipv4->setFlowWeight(s, new_weight);
        flowweights[s] = new_weight;
      }
    }
  }
 
  // get the right allocation 
  for(std::map<uint32_t, double>::iterator it = flow_weight_local.begin(); it != flow_weight_local.end(); ++it)
  {
    uint32_t fid = it->first;
    double weight = flow_weight_local[fid];

    double new_rate = (weight/total_weight) * (link_rate - 0.1*link_rate);
    for(uint32_t nid=0; nid < N ; nid++)
    {
      Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(nid))->GetObject<Ipv4> ());
      if (std::find((source_flow[nid]).begin(), (source_flow[nid]).end(), fid)!=(source_flow[nid]).end()) {
        
        std::stringstream ss;
        ss<<new_rate<<"bps";
        std::string datarate_str = ss.str(); 
      
        Ptr<MyApp> local_SendingApp;
        for(uint32_t aIndx=0; aIndx< (allNodes.Get(nid))->GetNApplications(); aIndx++) { // check all apps on this node
          local_SendingApp = StaticCast <MyApp> ( (allNodes.Get(nid))->GetApplication(aIndx) ); 
          if(local_SendingApp->getFlowId() == fid) { //if this is the app associated with this fid, change data rate
            local_SendingApp ->ChangeRate(DataRate (new_rate) ); 
            std::cout<<"TrueRate "<<Simulator::Now().GetSeconds()<<" "<<fid<<" "<<new_rate<<" source "<<nid<<std::endl;
            ipv4->setFlowIdealRate(fid, new_rate);
          } // end if flowid matches
        }  //end for iterating all applications
      } // end if flow belongs to the source
    }
  }
  setQFlows();
  // check queue size every 1/1000 of a second
  Simulator::Schedule (Seconds (0.2), &changeAppRates);
}

#if 0
// SC: follows form of change weights
// loop through all sources, assign a random 'weight' and scale the rate accordingly
// change the rate by modifying MyApp data rate for each application at host
void changeAppRates(void)
{
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
  double total_weight = 0.0;
  std::map<uint32_t, double> flow_weight_local;

  // loop thru senders and get the relative rate (as a weight) per sender
  for (uint32_t i=0; i < sourceNodes.GetN(); i++) {
        uint32_t rand_num = uv->GetInteger(1.0, 10.0);
        double new_weight = rand_num*1.0;

        flow_weight_local[i] = new_weight;
        total_weight += new_weight;
  }

  // iterate through all the senders
  // get the myApp pointer for currently running app
  // change its rate, continue
  for (uint32_t i=0; i < sourceNodes.GetN(); i++) {

      double weight = flow_weight_local[i];
      double new_rate = (weight/total_weight) * link_rate;

      std::stringstream ss;
      ss<<new_rate<<"bps";
      std::string datarate_str = ss.str(); 
      
      Ptr<MyApp> local_SendingApp;
      local_SendingApp = StaticCast <MyApp> ( (sourceNodes.Get(i))->GetApplication(0) );
      // local_SendingApp ->ChangeRate(DataRate ("4.1Gbps") ); 
      local_SendingApp ->ChangeRate(DataRate (new_rate) ); 
      
      std::cout<<"SC: changing application rate at node "<<i<<" to "<<new_rate<<" at "<<Simulator::Now().GetSeconds()<<" weight "<< weight <<std::endl;
 
      // SC: change idealRate based on rate of APP
      // SC: example cout debug statements
      // SC TrueRate debug 1 flow id 1 rate 5e+09 node id 2
      // SC TrueRate debug 1 flow id 2 rate 5e+09 node id 3
      // SC: changing application rate at node 0 to 5.71429e+09 at 1 weight 4
      // SC: changing application rate at node 1 to 4.28571e+09 at 1 weight 3
     
      // SC change to more general code for N > 2 
      // first sender, node 2 has fid 1
      if(i == 0) {
            Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(2))->GetObject<Ipv4> ());
            ipv4->setFlowIdealRate(1, new_rate);      
      } 
      //  sender 2, node 3 has fid 2
      if(i == 1) {
            Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(3))->GetObject<Ipv4> ());
            ipv4->setFlowIdealRate(2, new_rate);      
      }
 
  }
 Simulator::Schedule (Seconds (0.10), &changeAppRates);

}
#endif

// SC 
void setUpRateChange(void)
{
  
  Simulator::Schedule (Seconds (1.0), &changeAppRates);
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
      if(flow_known[s] == 0) { //unknown flow - no weight or rate
        continue;
      }

      /* check if this flowid is from this source */
      if (std::find((source_flow[nid]).begin(), (source_flow[nid]).end(), s)!=(source_flow[nid]).end()) {
        uint32_t rand_num = uv->GetInteger(1.0, 10.0);
        double new_weight = rand_num*1.0;
/*        double new_weight = s*1.0;
        double ns_time_now = Simulator::Now().GetSeconds();
        if((ns_time_now <= 1.19) || (ns_time_now > 1.2 && ns_time_now < 1.5)) {
          if(s == 1) {
            new_weight = 10.0;
          } else {
            new_weight = 1.0;
          }
        } else {
          if(s == 1) {
            new_weight = 10.0;
          } else {
            new_weight = 10.0;
          }
        }
  */         
        std::cout<<" setting weight of flow "<<s<<" at node "<<nid<<" to "<<new_weight<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
        flow_weight_local[s] = new_weight;
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
  setQFlows();
  // check queue size every 1/1000 of a second
  Simulator::Schedule (Seconds (0.2), &changeWeights);
}


void startRandomFlows(Ptr<EmpiricalRandomVariable> empirical_rand)
{
  double lambda = (link_rate * load ) / (meanflowsize*8.0);
  std::cout<<"lambda first "<<lambda<<std::endl;
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
     
      while(time_now < (sim_time-0.1))
      {
        // flow size 
        double flow_size = empirical_rand->GetValue(); 
        double inter_arrival = exp->GetValue();
        flow_start_time = time_now + inter_arrival;
        NS_LOG_UNCOND("next arrival after "<<inter_arrival<<" flow_start_time "<<flow_start_time);
        time_now = flow_start_time; // is this right ?
        std::cout<<"unknown flow between "<<(sourceNodes.Get(i))->GetId()<<" and "<<(sinkNodes.Get(j))->GetId()<<" starting at time "<<flow_start_time<<" of size "<<flow_size<<" flow_num "<<flow_num<<std::endl;
        uint32_t flow_weight = 1.0; // TBD - what weight do they have ? 

        uint32_t known = 0;
        startFlow(i, j, flow_start_time, flow_size, flow_num, flow_weight, 1, known); 
        flow_num++;
      }
    }
  }

  global_flow_id = flow_num;

}


void startFlowsStatic(void)
{

  uint32_t flow_num = 1;

   
  for (uint32_t i=0; i < sourceNodes.GetN(); i++) 
  {
    for(uint32_t j=0; j < sinkNodes.GetN(); j++) 
    {
    //  uint32_t j = i;
      double flow_start_time = 0.0;
      double time_now = 1.0;
      uint32_t flow_counter = 0;
     
      while(flow_counter < flows_per_host)
      //while(flow_num < 2)
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

void setUpWeightChange(void)
{
  
  Simulator::Schedule (Seconds (1.0), &changeWeights);
}

void setUpTraffic()
{
  NS_LOG_UNCOND("EmpiricalRandSetup : file "<<empirical_dist_file);
  Ptr<EmpiricalRandomVariable> x = SetUpEmpirical(empirical_dist_file);
  meanflowsize = x->avg();
  NS_LOG_UNCOND("Avg of empirical values.. "<<meanflowsize);
  startFlowsStatic();
  startRandomFlows(x);
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

  if(weight_change) {
    setUpWeightChange();
  } else {
    //
    // SC added
    setUpRateChange();
  }
  
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
