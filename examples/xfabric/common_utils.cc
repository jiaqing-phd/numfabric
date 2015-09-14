#include "declarations.h"
using namespace ns3;

void sinkInstallNodeEvent(uint32_t sourceN, uint32_t sinkN, uint16_t port, uint32_t flow_id, double startTime, uint32_t numBytes, uint32_t tcp)
{
  // Create a packet sink on the star "hub" to receive these packets
  Address anyAddress = InetSocketAddress (Ipv4Address::GetAny (), port);

  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", anyAddress);
  if(tcp) {
    sinkHelper.SetAttribute("Protocol", StringValue("ns3::TcpSocketFactory"));
    sinkHelper.SetAttribute("Local", AddressValue(anyAddress));
//    sinkHelper ("ns3::TcpSocketFactory", anyAddress);
  }
  ApplicationContainer sinkAppContainer = sinkHelper.Install (allNodes.Get(sinkN));
  sinkAppContainer.Start(Seconds(0.0));
  sinkApps.Add(sinkAppContainer);


  NS_LOG_UNCOND("sink apps installed on node "<<(allNodes.Get(sinkN))->GetId());
  Ptr<PacketSink> pSink = StaticCast <PacketSink> (sinkAppContainer.Get(0));
  pSink->SetAttribute("numBytes", UintegerValue(numBytes));
  pSink->SetAttribute("flowid", UintegerValue(flow_id));
  pSink->SetAttribute("nodeid", UintegerValue(allNodes.Get(sinkN)->GetId()));
  pSink->SetAttribute("peernodeid", UintegerValue(allNodes.Get(sourceN)->GetId()));
  pSink->setTracker(flowTracker);


  /* Debug... Check what we set */
  UintegerValue nb, fid, n1, n2;
  pSink->GetAttribute("numBytes", nb);
  pSink->GetAttribute("flowid", fid);
  pSink->GetAttribute("nodeid", n1);
  pSink->GetAttribute("peernodeid", n2);
  NS_LOG_UNCOND("sink attributed set : numbytes "<<nb.Get()<<" flowid "<<fid.Get()<<" nodeid "<<n1.Get()<<" source nodeid "<<n2.Get());
  
}
void sinkInstallNode(uint32_t sourceN, uint32_t sinkN, uint16_t port, uint32_t flow_id, double startTime, uint32_t numBytes, uint32_t tcp)
{
  // Create a packet sink on the star "hub" to receive these packets
  Address anyAddress = InetSocketAddress (Ipv4Address::GetAny (), port);

  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", anyAddress);
  if(tcp) {
    sinkHelper.SetAttribute("Protocol", StringValue("ns3::TcpSocketFactory"));
    sinkHelper.SetAttribute("Local", AddressValue(anyAddress));
//    sinkHelper ("ns3::TcpSocketFactory", anyAddress);
  }
  ApplicationContainer sinkAppContainer = sinkHelper.Install (sinkNodes.Get(sinkN));
  sinkAppContainer.Start(Seconds(0.0));
  sinkApps.Add(sinkAppContainer);


  NS_LOG_UNCOND("sink apps installed on node "<<(sinkNodes.Get(sinkN))->GetId());
  Ptr<PacketSink> pSink = StaticCast <PacketSink> (sinkAppContainer.Get(0));
  pSink->SetAttribute("numBytes", UintegerValue(numBytes));
  pSink->SetAttribute("flowid", UintegerValue(flow_id));
  pSink->SetAttribute("nodeid", UintegerValue(sinkNodes.Get(sinkN)->GetId()));
  pSink->SetAttribute("peernodeid", UintegerValue(sourceNodes.Get(sourceN)->GetId()));
  pSink->setTracker(flowTracker);


  /* Debug... Check what we set */
  UintegerValue nb, fid, n1, n2;
  pSink->GetAttribute("numBytes", nb);
  pSink->GetAttribute("flowid", fid);
  pSink->GetAttribute("nodeid", n1);
  pSink->GetAttribute("peernodeid", n2);
  NS_LOG_UNCOND("sink attributed set : numbytes "<<nb.Get()<<" flowid "<<fid.Get()<<" nodeid "<<n1.Get()<<" source nodeid "<<n2.Get());
  
}

void
CheckQueueSize (Ptr<Queue> queue)
{
  if(queue_type == "WFQ") {
    uint32_t qSize = StaticCast<PrioQueue> (queue)->GetCurSize ();
    uint32_t nid = StaticCast<PrioQueue> (queue)->nodeid;
    std::string qname = StaticCast<PrioQueue> (queue)->GetLinkIDString();
    double cur_price = StaticCast<PrioQueue> (queue)->getCurrentPrice();
    checkTimes++;
    std::cout<<"QueueStats "<<qname<<" "<<Simulator::Now ().GetSeconds () << " " << qSize<<" "<<nid<<" "<<cur_price<<std::endl;
    std::map<std::string, uint32_t>::iterator it;
/*    for (std::map<std::string,uint32_t>::iterator it= flowids.begin(); it!= flowids.end(); ++it) {
      double dline = StaticCast<PrioQueue> (queue)->get_stored_deadline(it->first);
      double virtual_time = StaticCast<PrioQueue> (queue)->get_virtualtime();
      double current_slope = StaticCast<PrioQueue> (queue)->getCurrentSlope();
      std::cout<<"QueueStats1 "<<qname<<" "<<Simulator::Now().GetSeconds()<<" "<<it->second<<" "<<dline<<" "<<virtual_time<<" "<<" "<<current_slope<<" "<<cur_price<<" "<<nid<<std::endl;
    } */
  } 
  if(queue_type == "W2FQ") {
    uint32_t qSize = StaticCast<W2FQ> (queue)->GetCurSize (0);
    uint32_t nid = StaticCast<W2FQ> (queue)->nodeid;
    std::string qname = StaticCast<W2FQ> (queue)->GetLinkIDString();
    checkTimes++;
    std::cout<<"QueueStats "<<qname<<" "<<Simulator::Now ().GetSeconds () << " " << qSize<<" "<<nid<<std::endl;
/*    std::map<std::string, uint32_t>::iterator it;
    for (std::map<std::string,uint32_t>::iterator it= flowids.begin(); it!= flowids.end(); ++it) {
      uint64_t virtual_time = StaticCast<W2FQ> (queue)->get_virtualtime();
      double dline = 0.0;
      std::cout<<"QueueStats1 "<<qname<<" "<<Simulator::Now().GetSeconds()<<" "<<it->second<<" "<<dline<<" "<<virtual_time<<" "<<" "<<nid<<std::endl;
    } */
  }

  if(queue_type == "hybridQ") {
    uint32_t qSize = StaticCast<hybridQ> (queue)->GetCurSize (0);
    uint32_t nid = StaticCast<hybridQ> (queue)->nodeid;
    std::string qname = StaticCast<hybridQ> (queue)->GetLinkIDString();
    uint32_t fifosize = StaticCast<hybridQ> (queue)->GetFifoSize();
    checkTimes++;
    std::cout<<"QueueStats "<<qname<<" "<<Simulator::Now ().GetSeconds () << " " << qSize<<" "<<nid<<" "<<fifosize<<std::endl;
/*    std::map<std::string, uint32_t>::iterator it;
    for (std::map<std::string,uint32_t>::iterator it= flowids.begin(); it!= flowids.end(); ++it) {
      uint64_t virtual_time = StaticCast<hybridQ> (queue)->get_virtualtime();
      double dline = 0.0;
      std::cout<<"QueueStats1 "<<qname<<" "<<Simulator::Now().GetSeconds()<<" "<<it->second<<" "<<dline<<" "<<virtual_time<<" "<<" "<<nid<<std::endl;
    }
*/
  }

  if(queue_type == "fifo_hybridQ") {
    uint32_t nid = StaticCast<fifo_hybridQ> (queue)->nodeid;
    std::string qname = StaticCast<fifo_hybridQ> (queue)->GetLinkIDString();
    uint32_t fifo_1_size = StaticCast<fifo_hybridQ> (queue)->GetFifo_1_Size();
    uint32_t fifo_2_size = StaticCast<fifo_hybridQ> (queue)->GetFifo_2_Size();
    checkTimes++;
    // example line
    // QueueStats 2_2_0 1.5 fifo_2_size 1078 fifo_1_size 2 0
    std::cout<<"QueueStats "<< qname <<" "<<Simulator::Now ().GetSeconds () << " fifo_2_size " << fifo_2_size << " fifo_1_size " << fifo_1_size << " node_id " << nid <<std::endl;
  }

  if(queue_type == "FifoQueue") {
    uint32_t qSize = StaticCast<FifoQueue> (queue)->GetCurSize ();
    uint32_t nid = StaticCast<FifoQueue> (queue)->nodeid;
    std::string qname = StaticCast<FifoQueue> (queue)->GetLinkIDString();
    checkTimes++;
    std::cout<<"QueueStats "<<qname<<" "<<Simulator::Now ().GetSeconds () << " " << qSize<<" "<<nid<<std::endl;
  }

  
    Simulator::Schedule (Seconds (sampling_interval), &CheckQueueSize, queue);
    if(Simulator::Now().GetSeconds() >= sim_time) {
      Simulator::Stop();
    }
 
}

CommandLine addCmdOptions(void)
{
  
  CommandLine cmd;  
  cmd.AddValue ("fct_alpha", "fctalpha for utility", fct_alpha);
  cmd.AddValue ("nNodes", "Number of nodes", N);
  cmd.AddValue ("prefix", "Output prefix", prefix);
  cmd.AddValue ("queuetype", "Queue Type", queue_type);
  cmd.AddValue ("pkt_tag","pkt_tag",pkt_tag);
  cmd.AddValue ("sim_time", "sim_time", sim_time);
  cmd.AddValue ("pkt_size", "pkt_size", pkt_size);
  cmd.AddValue ("link_rate","link_rate",link_rate);
  cmd.AddValue ("link_delay","link_delay",link_delay);
  cmd.AddValue ("ecn_thresh", "ecn_thresh", max_ecn_thresh);
  cmd.AddValue ("load", "load",load);
  cmd.AddValue ("controller_estimated_unknown_load", "controller_estimated_unknown_load",controller_estimated_unknown_load);
  cmd.AddValue ("rate_update_time", "rate_update_time", rate_update_time);
  cmd.AddValue ("sampling_interval", "sampling_interval", sampling_interval);
  cmd.AddValue ("kay", "kay", kvalue);
  cmd.AddValue ("vpackets", "vpackets", vpackets);
  cmd.AddValue ("xfabric", "xfabric", xfabric);
  cmd.AddValue ("dctcp", "dctcp", dctcp);
  cmd.AddValue ("hostflows", "hostflows",flows_per_host);
  cmd.AddValue ("flows_tcp", "flows_tcp", flows_tcp);
  cmd.AddValue ("weight_change", "weight_change", weight_change);
  //cmd.AddValue ("weight_norm", "weight_norm", weight_normalized);
  cmd.AddValue ("rate_based", "rate_based", rate_based);
  //cmd.AddValue ("UNKNOWN_FLOW_SIZE_CUTOFF", "unknown_flow_size_cutoff", UNKNOWN_FLOW_SIZE_CUTOFF);
  cmd.AddValue ("scheduler_mode_edf", "scheduler_mode_edf", scheduler_mode_edf);
  cmd.AddValue ("deadline_mode", "deadline_mode", deadline_mode);
  cmd.AddValue ("deadline_mean", "deadline_mean", deadline_mean);
  cmd.AddValue ("price_update_time", "price_update_time", price_update_time);
  cmd.AddValue ("host_compensate", "host_compensate", host_compensate);
  cmd.AddValue ("util_method", "util_method", util_method);
  cmd.AddValue ("strawmancc", "strawmancc", strawmancc);
  cmd.AddValue ("dgd_alpha", "dgd_alpha", dgd_alpha);
  cmd.AddValue ("dgd_gamma", "dgd_gamma", dgd_gamma);
  cmd.AddValue ("guardtime", "guardtime", guard_time);
  cmd.AddValue ("pfabric_util", "pfabric_util",pfabric_util);

  return cmd;
}

void common_config(void)
{
  double total_rtt = link_delay * 6.0;
  uint32_t bdproduct = link_rate *total_rtt/(1000000.0* 8.0);
  uint32_t initcwnd = (bdproduct / max_segment_size) +1;
  uint32_t ssthresh = initcwnd * max_segment_size;

  std::cout<<"Setting ssthresh = "<<ssthresh<<" initcwnd = "<<initcwnd<<" link_delay  "<<link_delay<<" bdproduct "<<bdproduct<<std::endl;  

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
  Config::SetDefault("ns3::TcpNewReno::dctcp", BooleanValue(dctcp));
  Config::SetDefault("ns3::TcpNewReno::xfabric", BooleanValue(xfabric));

  Config::SetDefault("ns3::PacketSink::StartMeasurement",TimeValue(Seconds(measurement_starttime)));

  Config::SetDefault("ns3::PrioQueue::m_pkt_tag",BooleanValue(pkt_tag));
  Config::SetDefault ("ns3::PrioQueue::Mode", StringValue("QUEUE_MODE_BYTES"));
  Config::SetDefault ("ns3::PrioQueue::MaxBytes", UintegerValue (max_queue_size));
  Config::SetDefault ("ns3::PrioQueue::ECNThreshBytes", UintegerValue (max_ecn_thresh));
  Config::SetDefault ("ns3::PrioQueue::delay_mark", BooleanValue(delay_mark_value));
  Config::SetDefault("ns3::PrioQueue::xfabric_price",BooleanValue(xfabric));
  Config::SetDefault("ns3::PrioQueue::dgd_gamma", DoubleValue(dgd_gamma));
  Config::SetDefault("ns3::PrioQueue::dgd_alpha",DoubleValue(dgd_alpha));
  Config::SetDefault("ns3::PrioQueue::guardTime",TimeValue(Seconds(guard_time)));
  Config::SetDefault("ns3::PrioQueue::PriceUpdateTime",TimeValue(Seconds(price_update_time)));


  Config::SetDefault ("ns3::FifoQueue::Mode", StringValue("QUEUE_MODE_BYTES"));
  Config::SetDefault ("ns3::FifoQueue::MaxBytes", UintegerValue (max_queue_size));
  Config::SetDefault ("ns3::FifoQueue::ECNThreshBytes", UintegerValue (max_ecn_thresh));

  Config::SetDefault ("ns3::fifo_hybridQ::Mode", StringValue("QUEUE_MODE_BYTES"));
  Config::SetDefault ("ns3::fifo_hybridQ::MaxBytes", UintegerValue (max_queue_size));
  Config::SetDefault ("ns3::fifo_hybridQ::ECNThreshBytes", UintegerValue (max_ecn_thresh));

  Config::SetDefault ("ns3::hybridQ::Mode", StringValue("QUEUE_MODE_BYTES"));
  Config::SetDefault ("ns3::hybridQ::MaxBytes", UintegerValue (max_queue_size));
  Config::SetDefault ("ns3::hybridQ::ECNThreshBytes", UintegerValue (max_ecn_thresh));

  Config::SetDefault("ns3::Ipv4L3Protocol::m_pkt_tag", BooleanValue(pkt_tag));
  Config::SetDefault ("ns3::PrioQueue::host_compensate", BooleanValue(host_compensate));
  Config::SetDefault("ns3::Ipv4L3Protocol::host_compensate", BooleanValue(host_compensate));
  Config::SetDefault("ns3::Ipv4L3Protocol::m_pfabric", BooleanValue(pfabric_util));
  Config::SetDefault("ns3::Ipv4L3Protocol::UtilFunction", UintegerValue(util_method));
  
  
  // rate_based 
  Config::SetDefault("ns3::Ipv4L3Protocol::rate_based", BooleanValue(strawmancc));
  Config::SetDefault("ns3::TcpNewReno::strawman", BooleanValue(strawmancc));

  flowTracker = new Tracker();
  //flowTracker->register_callback(scheduler_wrapper);

  return;

}

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
//  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
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

void setUpMonitoring(void)
{
  
  uint32_t Ntrue = allNodes.GetN(); 
  for(uint32_t nid=0; nid<Ntrue; nid++)
  {
     Ptr<Ipv4> ipv4 = (allNodes.Get(nid))->GetObject<Ipv4> ();
     NS_LOG_UNCOND("Setting flows up... "); 
     //StaticCast<Ipv4L3Protocol> (ipv4)->setFlows(flowids);
     //StaticCast<Ipv4L3Protocol> (ipv4)->setQueryTime(0.000004);
     //StaticCast<Ipv4L3Protocol> (ipv4)->setAlpha(1.0/128.0);
     StaticCast<Ipv4L3Protocol> (ipv4)->setQueryTime(rate_update_time);
     StaticCast<Ipv4L3Protocol> (ipv4)->setAlpha(1.0);

     StaticCast<Ipv4L3Protocol> (ipv4)->setKay(kvalue);
     
     //StaticCast<Ipv4L3Protocol> (ipv4)->setEpochUpdate(epoch_update_time);
     StaticCast<Ipv4L3Protocol> (ipv4)->setfctAlpha(fct_alpha);
  }
     
  //apps.Start (Seconds (1.0));
  //apps.Stop (Seconds (sim_time));

  Simulator::Schedule (Seconds (1.0), &CheckIpv4Rates, allNodes);
}

void
CheckIpv4Rates (NodeContainer &allNodes)
{
  double current_rate = 0.0;
  uint32_t N = allNodes.GetN(); 
  for(uint32_t nid=0; nid < N ; nid++)
  {
    Ptr<Ipv4L3Protocol> ipv4 = StaticCast<Ipv4L3Protocol> ((allNodes.Get(nid))->GetObject<Ipv4> ());
    std::map<std::string,uint32_t>::iterator it;
    for (std::map<std::string,uint32_t>::iterator it=ipv4->flowids.begin(); it!=ipv4->flowids.end(); ++it)
    {
    
//      double rate = ipv4->GetStoreRate (it->first);
      double csfq_rate = ipv4->GetCSFQRate (it->first);
      double short_rate = ipv4->GetShortTermRate(it->first);

      uint32_t s = it->second;

      /* check if this flowid is from this source */
      if (std::find((source_flow[nid]).begin(), (source_flow[nid]).end(), s)!=(source_flow[nid]).end()) {
         std::cout<<"DestRate flowid "<<it->second<<" "<<Simulator::Now ().GetSeconds () << " " << csfq_rate <<" "<<short_rate<<std::endl;
         current_rate += short_rate;
      }
//      std::cout<<"finding flow "<<s<<" in destination node "<<nid<<std::endl;
/*
      if (std::find((dest_flow[nid]).begin(), (dest_flow[nid]).end(), s)!=(dest_flow[nid]).end()) {
         //
         std::cout<<"DestRate flowid "<<it->second<<" "<<Simulator::Now ().GetSeconds () << " " << destRate <<" "<<csfq_rate<<" "<< nid << " " << N << " " << short_rate << std::endl;
      
        current_dest_rate += destRate;

      }
*/
    }
  }
  std::cout<<Simulator::Now().GetSeconds()<<" TotalRate "<<current_rate<<std::endl;
  
  // check queue size every 1/1000 of a second
  Simulator::Schedule (Seconds (sampling_interval), &CheckIpv4Rates, allNodes);
  if(Simulator::Now().GetSeconds() >= sim_time) {
    Simulator::Stop();
  }
}

void printlink(Ptr<Node> n1, Ptr<Node> n2)
{
  NS_LOG_UNCOND("printlink: link setup between node "<<n1->GetId()<<" and node "<<n2->GetId());
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


