#include "ns3/tracker.h"

#define FLOW_START 1
#define FLOW_STOP 2

FlowData::FlowData(uint32_t fid)
{
  flow_id = fid;
  flow_running = true; //check if this is useful
}

FlowData::FlowData(uint32_t source, int32_t dest, double fstart, double fsize, uint32_t flw_id, double fweight, uint32_t tcp, uint32_t known)
{
  flow_id = flw_id;
  source_node = source;
  dest_node = dest;
  flow_start = fstart;
  flow_size = fsize;
  flow_weight = fweight;
  flow_tcp = tcp;
  flow_known = known;
  flow_running = false;
  std::cout<<"DEBUG PARAMS FlowData "<<source<<" "<<dest<<" "<<flow_start<<" "<<flow_size<<" "<<flow_id<<" "<<flow_weight<<" "<<flow_tcp<<" "<<flow_known<<std::endl;
}

void Tracker::registered_callback(uint32_t fid)
{
  scheduler_func(fid);
}

void Tracker::register_callback(void (*functocall)(uint32_t))
{
  scheduler_func = functocall;
} 
   

void Tracker::registerEvent(uint32_t eventtype, FlowData fd)
{
  if(eventtype == FLOW_START) {
    std::cout<<"FLOW_START event "<<std::endl;
    // a new flow started - add it to set of flows
    flows_set.push_back(fd);
    dataDump();
  } else {
    //flows_set is a list..search for the right fid to remove
    std::cout<<"FLOW_STOP event fid "<<fd.flow_id<<std::endl;
    std::list<FlowData>::iterator itr;
    itr = flows_set.begin();  
    for(; itr != flows_set.end(); itr++) 
    {
      if(itr->flow_id == fd.flow_id) {
        flows_set.erase(itr);
        break;
      }
    }
    registered_callback(fd.flow_id);  
    
  }
}

void Tracker::dataDump(void)
{
    std::cout<<"flow_set dump "<<std::endl;
    std::list<FlowData>::iterator itr;
    itr = flows_set.begin();  
    for(; itr != flows_set.end(); itr++) 
    {
      std::cout<<"fid "<<itr->flow_id<<" src "<<itr->source_node<<" dst "<<itr->dest_node<<" size "<<itr->flow_size<<std::endl;
    }
}
   

Tracker::Tracker()
{
}  
