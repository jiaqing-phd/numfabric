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
#include "declarations.h"

using namespace ns3;

typedef struct OptDataRate_ {
  uint32_t flowid;
  double datarate;
  //OptDataRate(uint32_t f, double d) {flowid = f; datarate = d;}
} OptDataRate;


Ptr<Tracker> flowTracker;

std::map<uint32_t, double> flow_sizes;
int checkTimes = 0;
double kvalue = 50000;
double kvalue_price = 50000;
double kvalue_rate = 10000;
double kvalue_measurement = 80000;

std::map<uint32_t, std::vector<uint32_t> > source_flow;
std::map<uint32_t, std::vector<uint32_t> > dest_flow;
ApplicationContainer sinkApps;
std::vector<Ptr<PacketSink> >sink_objects;

std::map<uint32_t, double> flowweights;
double sim_time = 1.3;
double gamma1_value = 10.0;  //the weight to the rate term
double xfabric_beta = 0.5;

// All 3 variables for DGD price calculation
//
// 
/*double dgd_gamma = 0.000000001;
double dgd_alpha = 3*1e-10;
*/

double multiplier = 0.0000000001;
double dgd_gamma = 1.0;
double dgd_alpha = 0.3;


double dt_val = 0.000012;  // should be in seconds
double target_queue = 15000.0; // DGD parameter

double measurement_starttime = 1.2;
double rate_update_time = 0.0005;
double price_update_time = 0.000200; 
double xfabric_eta=10.0;
double guard_time = 0.000100; 

float sampling_interval = 0.0001;
uint32_t pkt_size = 1446; // reduced further to allow for 1 byte counter
uint32_t flows_tcp = 1;
uint32_t weight_change = 1;
uint32_t weight_normalized = 0;

const uint32_t max_flows=5;
uint32_t arg_max_flows = 5;

/* Queue variables */
uint32_t max_ecn_thresh = 50000;
uint32_t max_queue_size = 450000000;
std::string queue_type = "WFQ";
bool delay_mark_value = false;
uint32_t vpackets = 1;

bool host_compensate;

/* TCP variables */
uint32_t max_segment_size = 1410; // reduced further to allow for 1 bytes counter
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
double link_delay = 2.0; //in microseconds
bool rate_based  = false;
bool pfabric_util = false;
bool flow_ecmp = false;
bool packet_spraying = true;


/* Overall simulation parameters */
uint32_t N = 4; //number of nodes in the star
uint32_t flows_per_host = 1;
std::string application_datarate = "10Gbps";
std::string link_rate_string = "10Gbps";
double ONEG = 1000000000.0;
double link_rate = ONEG * 10.0;

// data rates and delays for leaf-spine
std::string fabric_datarate = "10Gbps";
std::string edge_datarate = "10Gbps";
double fabricdelay=2.0, edgedelay=2.0;

// number of nodes for leaf-spine
//uint32_t num_spines = 4, num_leafs = 9, num_hosts_per_leaf = 16;
uint32_t num_spines = 1, num_leafs = 2, num_hosts_per_leaf = 2;

bool pkt_tag = true;

std::string empirical_dist_file_DCTCP_heavy="DCTCP_CDF_HEAVY";
std::string empirical_dist_file_DCTCP_light="DCTCP_CDF_LIGHT";
Ptr<EmpiricalRandomVariable>  SetUpEmpirical(std::string fname);
std::string empirical_dist_file="DCTCP_CDF";
//std::string opt_rates_file="opt_rates_small_highspeed";
std::string opt_rates_file="opt_rates_nonexistent";

//std::map<uint32_t, std::vector<OptDataRate> > opt_drates;
std::map<uint32_t, std::map<uint32_t, double> > opt_drates;
EventId next_epoch_event;

std::string link_twice_string = "40Gbps";

NodeContainer bottleNeckNode;
NodeContainer allNodes;
NodeContainer spines, leafnodes, hosts; // for leaf-spine
NodeContainer sourceNodes;    
NodeContainer sinkNodes;
NodeContainer clientNodes;

double load = 0.05;
double controller_estimated_unknown_load = 0.05;
double UNKNOWN_FLOW_SIZE_CUTOFF = 1000000.0;
double meanflowsize = 1138*1460 ; // what TBD?

/* Application configuration */
//ApplicationContainer apps;
std::vector< Ptr<Socket> > ns3TcpSockets;
std::string prefix ="test";
uint32_t util_method =2;
double fct_alpha=0.1;
uint16_t *ports;

bool wfq;

Ptr<PacketSink> sinkInstallNode(uint32_t sourceN, uint32_t sinkN, uint16_t port, uint32_t flow_id, double startTime, uint32_t numBytes);
CommandLine addCmdOptions(CommandLine cmd);
void common_config(void);
void setUpMonitoring(void);
void CheckIpv4Rates (NodeContainer &allNodes);
void printlink(Ptr<Node> n1, Ptr<Node> n2);
Ipv4InterfaceContainer assignAddress(NetDeviceContainer dev, uint32_t subnet_index);
void CheckQueueSize (Ptr<Queue> queue);
bool price_multiply = false;

uint32_t number_flows = 100;
bool desynchronize = false;
uint32_t epoch_number = 0;
std::vector<uint32_t> sourcenodes;//(max_system_flows, 0);
std::vector<uint32_t> sinknodes;//(max_system_flows, 0);
uint32_t ninety_fifth;
double LastEventTime;
std::list<uint32_t> flows_to_start;
std::list<uint32_t> flows_to_stop;
std::list<uint32_t> event_list;


#endif 
