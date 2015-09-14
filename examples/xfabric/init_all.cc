#ifndef __XFABRIC_DECLARATIONS_H__
#define __XFABRIC_DECLARATIONS_H__

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

#include "ns3/tracker.h"

using namespace ns3;

Ptr<Tracker> flowTracker;

std::map<uint32_t, double> flow_sizes;
int checkTimes = 0;
double kvalue = 50000;

std::map<uint32_t, std::vector<uint32_t> > source_flow;
std::map<uint32_t, std::vector<uint32_t> > dest_flow;
ApplicationContainer sinkApps;

std::map<uint32_t, double> flowweights;
double sim_time = 1.3;
double gamma1_value = 10.0;  //the weight to the rate term


// All 3 variables for DGD price calculation
double dgd_gamma = 0.000000001; 
double dgd_alpha = 3*1e-10;
double target_queue = 30000.0;


double measurement_starttime = 1.2;
double rate_update_time = 0.0005;
double price_update_time = 0.000200; 
double guard_time = 0.000100; 

float sampling_interval = 0.1;
uint32_t pkt_size = 1454;
uint32_t flows_tcp = 1;
uint32_t weight_change = 1;
uint32_t weight_normalized = 0;

uint32_t const max_flows = 5;

/* Queue variables */
uint32_t max_ecn_thresh = 50000;
uint32_t max_queue_size = 450000000;
std::string queue_type = "WFQ";
bool delay_mark_value = true;
uint32_t vpackets = 1;

bool host_compensate;

/* TCP variables */
uint32_t max_segment_size = 1414;
uint32_t ssthresh_value = 3000;
uint32_t recv_buf_size = 1310720;
uint32_t send_buf_size = 1310720;
bool xfabric = true;
bool xfabric_price = true;
bool dctcp = false;
bool strawmancc = false;

/* Deadline variables */
bool deadline_mode = false;
bool scheduler_mode_edf = false;
// 5 ms
double deadline_mean = 2.0;
double fraction_flows_deadline = .5;

/* IP related variables */
std::map<std::string, uint32_t> flowids;
std::vector<Ptr<Queue > > AllQueues;
double link_delay = 5.0; //in microseconds
bool rate_based  = false;
bool pfabric_util = false;


/* Overall simulation parameters */
uint32_t N = 4; //number of nodes in the star
uint32_t flows_per_host = 1;
std::string application_datarate = "10Gbps";

bool pkt_tag = true;

std::string empirical_dist_file_DCTCP_heavy="DCTCP_CDF_HEAVY";
std::string empirical_dist_file="DCTCP_CDF";
std::string empirical_dist_file_DCTCP_light="DCTCP_CDF_LIGHT";
Ptr<EmpiricalRandomVariable>  SetUpEmpirical(std::string fname);

std::string link_twice_string = "20Gbps";

NodeContainer allNodes;
NodeContainer bottleNeckNode; // Only 1 switch
NodeContainer sourceNodes;    
NodeContainer sinkNodes;

double ONEG = 1000000000.0;
double link_rate = ONEG * 10.0;
//std::string link_rate_string = "10Gbps";
std::string link_rate_string = "10Gbps";

double load = 0.05;
double controller_estimated_unknown_load = 0.05;
double UNKNOWN_FLOW_SIZE_CUTOFF = 1000000.0;
double meanflowsize = 1138*1460 ; // what TBD?

/* Application configuration */
//ApplicationContainer apps;
std::vector< Ptr<Socket> > ns3TcpSockets;
std::string prefix ="test";
uint32_t util_method =1;
double fct_alpha=0.1;
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
