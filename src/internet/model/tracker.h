#ifndef __FLOW_TRACKER__
#define __FLOW_TRACKER__

#include "ns3/object.h"

using namespace ns3;

class FlowData {
 public:
  uint32_t flow_id;
  double flow_size;
  uint32_t source_node;
  uint32_t dest_node;
  double flow_weight;
  uint32_t flow_tcp;
  uint32_t flow_known;
  double flow_start;
  double flow_rem_size;
  double flow_deadline;
  double flow_deadline_delta;

  bool flow_running;

  FlowData(uint32_t fid);
  FlowData(uint32_t source, int32_t dest, double flw_start, double
  flw_size, uint32_t flw_id, double fweight, uint32_t tcp, uint32_t
  flw_known, double flw_rem_size, double flw_deadline, double
  flw_deadline_delta);

};
  
class Tracker : public Object
{
  public:
    std::list<FlowData> flows_set; 
    void (*scheduler_func)(uint32_t);
    Tracker();
    void registerEvent(uint32_t, FlowData);
    void dataDump();
    void register_callback(void (*f)(uint32_t));
    void registered_callback(uint32_t);
    void UpdateFlowRemainingSize(FlowData, double, uint32_t);

}; 

#endif
