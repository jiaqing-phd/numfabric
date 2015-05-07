#ifndef __DECLARATIONS_H__
#define __DECLARATIONS_H__

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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("pfabric");

std::map<uint32_t, double> flow_sizes;
int checkTimes = 0;

std::map<uint32_t, std::vector<uint32_t> > source_flow;
ApplicationContainer sinkApps;


double sim_time = 2.0;
double measurement_starttime = 1.2;
double prio_q_min_update_time = 0.0001;
double gamma1_value = 10.0;

double price_update_time = 0.0001;
double rate_update_time = 0.0001;
double gamma_value = 0.000001;
double flow2_stoptime=1.5;
double flow1_stoptime=1.5;
double flow3_stoptime=1.5;
double flow2_starttime=0;
double flow1_starttime=0;
double flow3_starttime=0;

double alpha_value = 1.0*1e-10;
double target_queue = 30000.0;


float sampling_interval = 0.0001;
uint32_t pkt_size = 1040;
//uint32_t max_queue_size = 150000;
uint32_t max_ecn_thresh = 0;
uint32_t max_queue_size = 450000000;
//uint32_t max_ecn_thresh = 30000;
//uint32_t max_segment_size = 1440;
uint32_t max_segment_size = 1402;
uint32_t ssthresh_value = 3000;
std::map<std::string, uint32_t> flowids;
uint32_t recv_buf_size = 1310720;
uint32_t send_buf_size = 1310720;
double link_delay = 7.0; //in microseconds
bool margin_util_price = false;

uint32_t N = 6; //number of nodes in the star
uint32_t skip = 2;
std::string queue_type;
double epoch_update_time = 0.001;
bool pkt_tag, onlydctcp, wfq, dctcp_mark;
bool strawmancc = false;
std::string empirical_dist_file="DCTCP_CDF";
Ptr<EmpiricalRandomVariable>  SetUpEmpirical(std::string fname);

std::string link_twice_string = "20Gbps";

NodeContainer allNodes;
NodeContainer bottleNeckNode; // Only 1 switch
NodeContainer sourceNodes;    
NodeContainer sinkNodes;

double ONEG = 1000000000;
double link_rate = ONEG;
std::string link_rate_string = "1Gbps";

double load = 0.5;
double meanflowsize = 1138*1460 ; // what TBD?

/* Application configuration */
//ApplicationContainer apps;
std::vector< Ptr<Socket> > ns3TcpSockets;
std::string prefix;
void setuptracing(uint32_t sindex, Ptr<Socket> skt);
uint32_t util_method =1;
uint16_t *ports;

void sinkInstallNode(uint32_t sourceN, uint32_t sinkN, uint16_t port, uint32_t flow_id, double startTime, uint32_t numBytes);




CommandLine addCmdOptions(CommandLine cmd);
void common_config(void);
void setUpMonitoring(void);
void CheckIpv4Rates (NodeContainer &allNodes);
void printlink(Ptr<Node> n1, Ptr<Node> n2);
Ipv4InterfaceContainer assignAddress(NetDeviceContainer dev, uint32_t subnet_index);
void CheckQueueSize (Ptr<Queue> queue);

#endif 
