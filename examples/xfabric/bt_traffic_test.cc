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

void startFlowEvent(uint32_t sourceN, uint32_t sinkN, double flow_start, double flow_size, uint32_t flow_id, uint32_t flow_weight, uint32_t tcp, uint32_t known)
{

  //std::cout<<"DEBUG params StartFlowEvent "<<sourceN<<" "<<sinkN<<" "<<flow_start<<" "<<flow_size<<" "<<flow_id<<" "<<flow_weight<<" "<<tcp<<" "<<known<<std::endl;

  ports[sinkN]++;
  // Socket at the source
  Ptr<Ipv4L3Protocol> sink_node_ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(sinkN))->GetObject<Ipv4> ());
  Ipv4Address remoteIp = sink_node_ipv4->GetAddress (1,0).GetLocal();
  Address remoteAddress = (InetSocketAddress (remoteIp, ports[sinkN]));
  sinkInstallNodeEvent(sourceN, sinkN, ports[sinkN], flow_id, flow_start, flow_size, tcp);

  // Get source address
  Ptr<Ipv4L3Protocol> source_node_ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(sourceN))->GetObject<Ipv4> ()); 
  Ipv4Address sourceIp = source_node_ipv4->GetAddress (1,0).GetLocal();
  Address sourceAddress = (InetSocketAddress (sourceIp, ports[sinkN]));
  Ptr<MyApp> SendingApp = CreateObject<MyApp> ();
  SendingApp->Setup (remoteAddress, pkt_size, DataRate (application_datarate), flow_size, flow_start, sourceAddress, allNodes.Get(sourceN), flow_id, allNodes.Get(sinkN), tcp, known);

  (allNodes.Get(sourceN))->AddApplication(SendingApp);
      
  Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(sourceN))->GetObject<Ipv4> ()); // Get Ipv4 instance of the node
  Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal();

  std::cout<<"FLOW_INFO source_node "<<(allNodes.Get(sourceN))->GetId()<<" sink_node "<<(allNodes.Get(sinkN))->GetId()<<" "<<addr<<":"<<remoteIp<<" flow_id "<<flow_id<<" start_time "<<flow_start<<" dest_port "<<ports[sinkN]<<" flow_size "<<flow_size<<" flow_weight" <<flow_weight<<std::endl;

  flow_known[flow_id] = known;  // whether this flow is part of known or unknown flow set
  (source_flow[(allNodes.Get(sourceN))->GetId()]).push_back(flow_id);
  (dest_flow[(allNodes.Get(sinkN))->GetId()]).push_back(flow_id);
  std::stringstream ss;
  ss<<addr<<":"<<remoteIp<<":"<<ports[sinkN];
  std::string s = ss.str(); 
  flowids[s] = flow_id;
  
  ipv4->setFlow(s, flow_id, flow_size, flow_weight);
  sink_node_ipv4->setFlow(s, flow_id, flow_size, flow_weight);

  sink_node_ipv4->setSimTime(sim_time);
  
  //flow_id++;
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
  std::cout<<"Getting source address for node "<<sourceN<<" num nodes "<<sourceNodes.GetN()<<std::endl;
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

  // random variables for deadline generation
  Ptr<ExponentialRandomVariable> deadline_exp = getDeadlineRV();
  Ptr<UniformRandomVariable> deadline_decision = getDecisionRV();

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
        
        uint32_t known;
        
        // determine known or not based on flow size
        // if less than 1 MB will be unknown

        if(flow_size <= UNKNOWN_FLOW_SIZE_CUTOFF) {
          known = 0;
          //std::cout<<"unknown flow between "<<(sourceNodes.Get(i))->GetId()<<" and "<<(sinkNodes.Get(j))->GetId()<<" starting at time "<<flow_start_time<<" of size "<<flow_size<<" flow_num "<<flow_num<<std::endl;
          std::cout<<"SC_DCTCP_DEBUG known "<< known <<" UNKNOWN_FLOW_SIZE_CUTOFF "<< UNKNOWN_FLOW_SIZE_CUTOFF <<" flow_size "<< flow_size << " flow_num " << flow_num << std::endl;
        } else {
          known = 1;
          //std::cout<<"known flow between "<<(sourceNodes.Get(i))->GetId()<<" and "<<(sinkNodes.Get(j))->GetId()<<" starting at time "<<flow_start_time<<" of size "<<flow_size<<" flow_num "<<flow_num<<std::endl;
          std::cout<<"SC_DCTCP_DEBUG known "<< known <<" UNKNOWN_FLOW_SIZE_CUTOFF "<< UNKNOWN_FLOW_SIZE_CUTOFF <<" flow_size "<< flow_size << " flow_num " << flow_num << std::endl;

        }

        uint32_t flow_weight = 1.0; // TBD - what weight do they have ? 
        uint32_t snid = (sourceNodes.Get(i))->GetId();
        uint32_t destnid = (sinkNodes.Get(j))->GetId();
        uint32_t uftcp = 1;

        if(flows_tcp == 0 && known == 1) {
          // if flows_tcp == 0, known flows should run UDP
          uftcp = 0;
        }

        // SC change for deadline case
        double local_flow_deadline = 0.0;
        bool flow_has_deadline = false;
        double dd = deadline_decision->GetValue(0, 1.0); 
        double local_flow_duration = 0.0;
        if(deadline_mode && (dd < 2.5) && known) {
          // deadline based flow 
          // get deadline DURATION from random variables
          double local_flow_duration;
          local_flow_duration = getDeadline(flow_start_time, flow_size,deadline_exp, deadline_decision, flow_num);
          local_flow_deadline = flow_start_time + local_flow_duration;
          flow_has_deadline = true;
        }

         // populate flow data with deadline info
        FlowData flowData (snid, destnid, flow_start_time,
        flow_size,flow_num, flow_weight , uftcp, known, flow_size,
        local_flow_deadline, local_flow_duration, flow_has_deadline ); 
        flow_known[flow_num] = known;
          
        std::cout<<"FINAL_DEBUG_DEADLINE flow_num " << flow_num << " flow_start_time " << flow_start_time << " duration " << local_flow_duration << " flow_size " << flow_size << " local_flow_deadline " << local_flow_deadline << " known " << known << " flow_has_deadline  " << flow_has_deadline << std::endl; 

          if(deadline_mode) {
		if(scheduler_mode_edf) {

            		Simulator::Schedule (Seconds (flow_start_time), &run_scheduler_edf, flowData, 1);
            	} else {
			Simulator::Schedule (Seconds (flow_start_time), &run_scheduler, flowData, 1);
		}


        	std::cout<<"ENTER SCHED deadline "<< deadline_mode << " scheduler_mode_edf " << scheduler_mode_edf << std::endl; 

          } else {
            Simulator::Schedule (Seconds (flow_start_time), &run_scheduler, flowData, 1);
          }

          //startFlow(i, j, flow_start_time, flow_size, flow_num, flow_weight, 1, known); 
          flow_num++;
        
      } // end while
    }
  }

}
/*
void scheduler_wrapper(uint32_t fid)
{
  // this is a wrapper called from tracker object
  // a flow_stop event has been registered and flows_set updated at tracker
  // just call run_scheduler with dummy arguments
  //
  if(flow_known[fid] == 1) {
    std::cout<<"known flow "<<fid<<" departed"<<std::endl;
    FlowData fdata(fid);
    if(deadline_mode) {
	if(scheduler_mode_edf){
      		run_scheduler_edf(fdata, 2);
	} else {
      		run_scheduler(fdata, 2);
	}
    } else {
      run_scheduler(fdata, 2);
    }
  } else {
    std::cout<<"unknown flow "<<fid<<" departed"<<std::endl;
  }

}*/

void run_scheduler_edf(FlowData fdata, uint32_t eventType)
{

  std::cout<<"EDF scheduler" <<std::endl;
  
  std::map<uint32_t, double> flow_rate_local;
  std::map<uint32_t, double> flow_weight_local;
      
  /* Deadline based scheduling */
  if(eventType == 1) { //TODO: declare enum FLOW_START
    // add it to the list of flows 
    if(fdata.flow_known == 0) {
      startFlowEvent(fdata.source_node, fdata.dest_node, Simulator::Now().GetSeconds(), fdata.flow_size, fdata.flow_id, 1.0, fdata.flow_tcp, fdata.flow_known);
      setQFlows();
      std::cout<<"EDF Unknown flow "<<fdata.flow_id<<" started.. nothing to be done "<<std::endl;
      return;
    } else {
      std::cout<<"EDF known flow "<<fdata.flow_id<<" started.. run sched "<<std::endl;
    }
    /* adding the flow temporarily so that the scheduler can take it into account */
    flowTracker->registerEvent(1, fdata);
  }
  /* order flows by their deadlines */
  flowTracker->deadline_flows_set.sort(compare_flow_deadlines);
 
  /* sorted in order of their deadlines */
  std::list<FlowData>::iterator itr;
  itr = (flowTracker->deadline_flows_set).begin();

  double total_rate_required = 0.0;
  double available_rate = (1-controller_estimated_unknown_load) * link_rate;
  while(itr != flowTracker->deadline_flows_set.end()) 
  {
    double nanoseconds = 1000000000.0;
    double time_till_deadline = (itr->flow_deadline) * nanoseconds - Simulator::Now().GetNanoSeconds();
    double rate_required = 0.0;
    if(time_till_deadline > 0.0) {
      rate_required = itr->flow_rem_size/(time_till_deadline/nanoseconds);
      total_rate_required += rate_required;
    }
    uint32_t nid = itr->source_node;
    std::cout<<"trying to access node "<<nid<<" fid "<<itr->flow_id<<std::endl;
    Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(nid))->GetObject<Ipv4> ());

    if(rate_required > available_rate) {
      /* this flow cannot complete now - kill it now -- we will just assign 0 rate*/
      rate_required = 0.0;
    } //end of rate required is more than link_rate
    else if((total_rate_required +rate_required) > available_rate) {
      // we can't allocate all of this - but allocate partially
      double partial = available_rate - total_rate_required;
      if(partial < 0.0) {
        partial = 0.0;
      }
      rate_required = partial;
      total_rate_required += partial;
    } 
    flow_rate_local[itr->flow_id] = rate_required;
    std::cout<<"EDF DEADLINE TrueRate "<<Simulator::Now().GetSeconds()<<" "<<itr->flow_id<<" "<<rate_required<<std::endl;
    std::cout<<"EDF DEADLINE setting realrate "<<rate_required<<" for flow "<<itr->flow_id<<" in node "<<itr->source_node<<std::endl;

    std::cout<<"EDF DEBUG rate_required "<< rate_required <<" for flow "<<itr->flow_id<<" in node "<<itr->source_node<< " flow size " << itr->flow_size << " total rate required " << total_rate_required << " available rate " << available_rate << std::endl;

    ipv4->setFlowIdealRate(itr->flow_id, rate_required);

    if(!itr->flow_running) {
      flow_weight_local[itr->flow_id] = 1.0;
      startFlowEvent(itr->source_node, itr->dest_node, Simulator::Now().GetSeconds(), itr->flow_size, itr->flow_id, flow_weight_local[itr->flow_id], itr->flow_tcp, itr->flow_known);
      itr->flow_running = true;
    }
    itr++;
   }

   // we allocated rates to all the deadline bound flows - divide the remaining capacity into non-deadline bound flows
   double remaining_capacity = available_rate - total_rate_required;
   double per_flow_cap = 0.0;
   if(remaining_capacity > 0.0) {
    per_flow_cap = remaining_capacity / (flowTracker->flows_set).size();
   }
   itr = (flowTracker->flows_set).begin();

    while(itr != flowTracker->flows_set.end())  {
      uint32_t nid = itr->source_node;
      Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(nid))->GetObject<Ipv4> ());
      flow_rate_local[itr->flow_id] = per_flow_cap;
      std::cout<<" EDF NO_DEADLINE TrueRate "<<Simulator::Now().GetSeconds()<<" "<<itr->flow_id<<" "<<per_flow_cap<<std::endl;
      std::cout<<" EDF NO_DEADLINE setting realrate "<<per_flow_cap<<" for flow "<<itr->flow_id<<" in node "<<itr->source_node<<std::endl;
      ipv4->setFlowIdealRate(itr->flow_id, per_flow_cap);

      if(!itr->flow_running) {
        flow_weight_local[itr->flow_id] = 1.0;
        startFlowEvent(itr->source_node, itr->dest_node, Simulator::Now().GetSeconds(), itr->flow_size, itr->flow_id, flow_weight_local[itr->flow_id], itr->flow_tcp, itr->flow_known);
        itr->flow_running = true;
      }
      itr++;
   }
    
}    
     
   

void run_scheduler(FlowData fdata, uint32_t eventType)
{

  // scheduler called to add a new flow and reschedule all flows
  // std::cout<<"scheduler called at "<<Simulator::Now().GetSeconds()<<" flow "<<fdata.flow_id<<" eventtype "<<eventType<<std::endl;

  flowTracker->dataDump(); // just for debugging

  if(eventType == 1) { //TODO: declare enum FLOW_START
    // add it to the list of flows 
    if(fdata.flow_known == 0) {
      startFlowEvent(fdata.source_node, fdata.dest_node, Simulator::Now().GetSeconds(), fdata.flow_size, fdata.flow_id, 1.0, fdata.flow_tcp, fdata.flow_known);
      setQFlows();
      // std::cout<<"Unknown flow "<<fdata.flow_id<<" started.. nothing to be done "<<std::endl;
      return;
    } else {
      // std::cout<<"known flow "<<fdata.flow_id<<" started.. run sched "<<std::endl;
    }
    flowTracker->registerEvent(1, fdata);
  }

  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
  double total_weight = 0.0;
  std::map<uint32_t, double> flow_weight_local;
  double min_weight = 100.0;

  std::list<FlowData>::iterator itr;
  itr = (flowTracker->flows_set).begin();

  while(itr != flowTracker->flows_set.end()) 
  {
      uint32_t rand_num = uv->GetInteger(1.0, 10.0);
      double new_weight = rand_num*1.0;
      if(itr->flow_running) {
        /* flow already running - don't reassign weight */ 
        new_weight = itr->flow_weight;
      }
      
      flow_weight_local[itr->flow_id] = new_weight;
      if(new_weight < min_weight) {
        min_weight = new_weight;
      }
      itr++;
  } // end flows_set

  /* start a new loop to normalize weights, if configured to do so */
  itr = flowTracker->flows_set.begin();

  while(itr != flowTracker->flows_set.end()) 
  {
    uint32_t nid = itr->source_node;
    Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(nid))->GetObject<Ipv4> ());
    double new_weight = flow_weight_local[itr->flow_id];
    if(weight_normalized) {
      new_weight = new_weight/min_weight;
    }
    flow_weight_local[itr->flow_id]  = new_weight;
 
    //std::cout<<" setting weight of flow "<<itr->flow_id<<" at node "<<nid<<" to "<<new_weight<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
    total_weight += new_weight;
    ipv4->setFlowWeight(itr->flow_id, new_weight);
    flowweights[itr->flow_id] = new_weight;

    if(itr->flow_running) {
      //nothing to do
      itr++;
      continue;
    } else {
      startFlowEvent(itr->source_node, itr->dest_node, Simulator::Now().GetSeconds(), itr->flow_size, itr->flow_id, flow_weight_local[itr->flow_id], itr->flow_tcp, itr->flow_known);
      itr->flow_running = true;
    }
    itr++;
  } //end flows_set

   std::cout<<"BASE RATE "<<Simulator::Now().GetSeconds()<<" "<<(1.0/total_weight)*link_rate<<std::endl; 
   // get the right allocation of rates - another loop
   //
  itr = flowTracker->flows_set.begin();

  while(itr != flowTracker->flows_set.end()) 
  {
    uint32_t nid = itr->source_node;
    Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(nid))->GetObject<Ipv4> ());

    uint32_t fid = itr->flow_id;
    double weight = flow_weight_local[fid];

    double rate = (weight/total_weight) * (1 - controller_estimated_unknown_load) * link_rate;
    
    if(weight_change == 0) { // UDP with rate control
       Ptr<MyApp> local_SendingApp;
       for(uint32_t aIndx=0; aIndx< (allNodes.Get(itr->source_node))->GetNApplications(); aIndx++) { // check all apps on this node
          local_SendingApp = StaticCast <MyApp> ( (allNodes.Get(nid))->GetApplication(aIndx) ); 
         //  if((rate_based == 0) && (local_SendingApp->getFlowId() == fid)) { //if this is the app associated with this fid, change data rate
           //    local_SendingApp ->ChangeRate(DataRate (rate) ); 
           // }
        } // end for
     }
     std::cout<<"TrueRate "<<Simulator::Now().GetSeconds()<<" "<<fid<<" "<<rate<<" weight "<<weight<<" totalweight "<<total_weight<<std::endl;
     std::cout<<" setting realrate "<<rate<<" for flow "<<fid<<" in node "<<itr->source_node<<std::endl;
        
     ipv4->setFlowIdealRate(fid, rate);
    itr++;
  }
  setQFlows();
  std::cout<<" SCHEDULER DONE "<<Simulator::Now().GetSeconds()<<std::endl;
  // check queue size every 1/1000 of a second
  //Simulator::Schedule (Seconds (0.2), &changeWeights);
}

Ptr<ExponentialRandomVariable> getDeadlineRV() {
  Ptr<ExponentialRandomVariable> exp = CreateObject<ExponentialRandomVariable> ();
  exp->SetAttribute("Mean", DoubleValue(deadline_mean));
  //std::cout<<" create deadline RV with mean " << deadline_mean << std::endl;
  return exp;
}

Ptr<UniformRandomVariable> getDecisionRV() {
  Ptr<UniformRandomVariable> deadline_decision = CreateObject<UniformRandomVariable> ();
  //std::cout<<" create deadline decision uniform 1 - 10 " << deadline_decision << std::endl;
  return deadline_decision;
}

double getDeadline(double start_time, double flow_size, Ptr<ExponentialRandomVariable> exp, Ptr<UniformRandomVariable> deadline_decision, uint32_t flow_num) { 

  // case 1: generate the nominal deadline
  double RV_delta = exp->GetValue();

  // in Bits/sec, convert to bytes/sec
  //double effective_rate = (link_rate * (1.0 - controller_estimated_unknown_load))/8.0;
  double effective_rate = (link_rate * (1.0))/8.0;

  double best_transmit_time = flow_size/effective_rate;
  double min_transmit_time = 1.25 * flow_size/effective_rate;
  // if RV deadline too close, take 1.25 * best transmit (LOWER BOUND)
  // double deadline_delta = std::max(RV_delta + best_transmit_time, min_transmit_time);
  double deadline_delta = std::max(RV_delta * best_transmit_time, min_transmit_time);

  double random_deadline = RV_delta + best_transmit_time;
  double total_deadline = start_time + deadline_delta;

  std::cout<<"created_deadline TRUE flow_num " << flow_num << " duration " << deadline_delta << " min_transit_time " << min_transmit_time << " flow_size " << flow_size << " effective_rate " << effective_rate << " link_rate " << link_rate << " best_transmit_time " << best_transmit_time << " RV_value " << RV_delta << " random_deadline " << random_deadline << " flow_start " << start_time << " total_deadline " << total_deadline << std::endl;
  
  return deadline_delta;
}

void setUpTraffic()
{

  // SC: sample from DCTCP full again
  NS_LOG_UNCOND("EmpiricalRandSetup : file "<<empirical_dist_file);
  Ptr<EmpiricalRandomVariable> x = SetUpEmpirical(empirical_dist_file);
  meanflowsize = x->avg();
  NS_LOG_UNCOND("Avg of empirical values.. "<<meanflowsize);
  //startFlowsStatic();
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

      /* check if this flowid is from this source */
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

  /* start a new loop to normalize weights, if configured to do so */
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

const uint32_t max_flows=10;
void startFlowsStatic(void)
{

  uint32_t flow_num = 1;

  Ptr<UniformRandomVariable> unifRandom = CreateObject<UniformRandomVariable> ();

  std::cout<<"strtflowstatic "<<sourceNodes.GetN()<<" "<<sinkNodes.GetN()<<std::endl;   
  //for (uint32_t i=0; i < sourceNodes.GetN(); i++) 
  //{
    //for(uint32_t j=0; j < sinkNodes.GetN(); j++) 
    //{
  //    uint32_t j = i;
    while(flow_num < max_flows) {
      double flow_start_time = 0.0;
      double time_now = 1.0;
      uint32_t flow_counter = 0;

      unsigned int i = unifRandom->GetInteger (0, (sourceNodes.GetN()-1));
      unsigned int j = unifRandom->GetInteger (0, (sinkNodes.GetN() - 1));
     
        // flow size 
      double flow_size = 12500000000; 
      flow_start_time = time_now + 0.0001;
      std::cout<<"picked a flow between "<<(sourceNodes.Get(i))->GetId()<<" and "<<(sinkNodes.Get(j))->GetId()<<" starting at time "<<flow_start_time<<" of size "<<flow_size<<" flow_num "<<flow_num<<std::endl;
      uint32_t flow_weight = 1.0 * flow_num;
      uint32_t known = 1;
        
      startFlow(i, j, flow_start_time, flow_size, flow_num, flow_weight, flows_tcp, known); 
      flow_num++;
      flow_counter++;
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
    std::string input ("topologies/BT_Europe.txt");

    ns3::TopologyReaderHelper topoHelp;
    topoHelp.SetFileName (input);
    topoHelp.SetFileType (format);
    Ptr<TopologyReader> inFile = topoHelp.GetTopologyReader ();

    //NodeContainer allNodes;

    if (inFile != 0)
    {
      allNodes = inFile->Read ();
      N = allNodes.GetN();
    }

    if (inFile->LinksSize () == 0)
    {
      NS_LOG_ERROR ("Problems reading the topology file. Failing.");
    }

    int totlinks = inFile->LinksSize ();
    std::cout <<" ROCKET : totlinks "<< totlinks << std::endl;

    uint32_t totalNodes = allNodes.GetN ();
    std::cout <<" ROCKET : totalNodes "<< totalNodes << std::endl;

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

      } else {
        // sink node
        source_sink_assignment[i] = 1.0;
        num_sinks++;
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
      std::cout<<"set up nodecontainer between nodes "<<iter->GetFromNode()->GetId()<<" to node "<<iter->GetToNode()->GetId()<<std::endl;
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

    ///////////////////////////////////
    //LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_ALL);

    CommandLine cmd = addCmdOptions();
    cmd.Parse (argc, argv);
    common_config(); 

    std::cout<<*argv<<std::endl;
    std::cout<<"set prefix to "<<prefix<<std::endl;

    rocket_createTopology();

    // createTopology();
    //setUpTraffic();
    startFlowsStatic();
    setUpMonitoring();
    NS_LOG_INFO ("Run Simulation.");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");


 // if(weight_change) {
    setUpWeightChange();
 // }

 // NS_LOG_INFO ("Run Simulation.");
 // Simulator::Run ();
 // Simulator::Destroy ();
 // NS_LOG_INFO ("Done.");
}//
