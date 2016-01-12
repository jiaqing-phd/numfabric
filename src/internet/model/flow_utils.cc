#include "ns3/flow_utils.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

//NS_LOG_COMPONENT_DEFINE("FlowUtil");
/* TBD */

namespace ns3 {

//NS_OBJECT_ENSURE_REGISTERED (FlowUtil);
//
//

void FlowUtil::setflowweight(uint32_t fid, double w)
{
  flow_weights[fid] = w;
} 

void FlowUtil::SetFCTAlpha(double fctalpha)
{
  fct_alpha = fctalpha;
}

void FlowUtil::setSizes(std::map<uint32_t, double> fsizes)
{
 std::map<uint32_t, double>::iterator it;
 for (std::map<uint32_t, double>::iterator it=fsizes.begin(); it!=fsizes.end(); ++it) {
    flow_sizes[it->first] = it->second;
 }
}

void FlowUtil::SetFlow(std::string flow, uint32_t fid, double fsize, double weight)
{
  flow_sizes[fid] = fsize;
  flow_weights[fid] = weight;
}

 
void
FlowUtil::setWeights(std::map<uint32_t, double> fweights)
{
 std::map<uint32_t, double>::iterator it;
 for (std::map<uint32_t, double>::iterator it=fweights.begin(); it!=fweights.end(); ++it)
  {
    flow_weights[int(it->first)] = fweights[int(it->first)];
//    w.push_back(fweights[i]);
  }
  
}



double 
FlowUtil::getAlpha1InverseByFlowId(uint32_t fid, double price)
{
/*  double flow_size = flow_sizes[fid]/1000000.0;
  if(flow_size == 0) {
    flow_size = 0.000688; //size of the smallest pkt I have seen 
  }
*/
  if(price == 0) {
    price = 0.000000000000000001;
  }
//  double inverse = 1.0 / (price * flow_size);
  double inverse = 1.0 / price;
  double utilinverse = pow(inverse, 1.0/fct_alpha);
  return utilinverse;
}

  
double 
FlowUtil::getUtilInverseByFlowId(uint32_t fid, double priority)
{
  double ret_val = 1.0/(priority);
  if (flow_weights.find( fid ) != flow_weights.end()){
      ret_val = flow_weights[fid]/(priority);
  }
  //std::cout<<" getUtilInverseByFlowID fid "<<fid<<" weight "<<flow_weights[fid]<<" priority "<<priority<<" c "<<c<<std::endl;
  return ret_val;
}

double 
FlowUtil::getFCTInverseByFlowId(uint32_t fid, double price)
{
  //double flow_size = flow_sizes[fid] /1000000.0;
  double flow_size = flow_sizes[fid]/10000000000.0;
  if(flow_size == 0) {
    //flow_size = 688/1000000.0; //size of the smallest pkt I have seen 
    flow_size = 688.0; //size of the smallest pkt I have seen 
  }
  double utilinverse = 1.0 / (price * flow_size);
  std::cout<<"getFCTInverseByFlowid fid "<<fid<<" size "<<flow_size<<" price "<<price<<" utilinverse "<<utilinverse<<std::endl;
  return utilinverse;
}
double 
FlowUtil::getFCTUtilByFlowID(uint32_t fid, double price)
{
  //double flow_size = flow_sizes[fid] /1000000.0;
  double flow_size = flow_sizes[fid]/10000000000.0;
  if(flow_size == 0) {
    //flow_size = 688/1000000.0; //size of the smallest pkt I have seen 
    flow_size = 688.0; 
  }
  if(price == 0) {
    price = 0.000000000000000001;
  }
  double utilinverse = 1.0 / (price * flow_size);
  std::cout<<"getFCTUtilByFlowid fid "<<fid<<" size "<<flow_size<<" price "<<price<<" utilinverse "<<utilinverse<<std::endl;
  return utilinverse;
}

double
FlowUtil::getFCTUtilDerivative(uint32_t fid, double price)
{
  double flow_size = flow_sizes[fid]/10000000000.0;
  if(flow_size == 0) {
    flow_size = 688.0/10000000000.0; //smallest flow size i have seen
  }
  if(price == 0) {
    price = 0.0000000000000001;
  }
  double p = pow(price, 7.0/8.0);
  double derivative = 1.0/(8.0 * flow_size * p);
  return derivative;
}

double
FlowUtil::getFCTUtilDerivativeInverse(uint32_t fid, double price)
{
  double flow_size = flow_sizes[fid]/10000000000.0;
  if(flow_size == 0) {
    flow_size = 688.0/10000000000.0; //smallest flow size i have seen
  }
  if(price == 0) {
    price = 0.0000000000000001;
  }
  double denom = (8.0 * flow_size * price);
  double inverse = 1.0/pow(denom, 8.0/7.0);
  return inverse;
}



double
FlowUtil::getAlpha1UtilByFlowID(uint32_t fid, double rate)
{
/*  double flow_size = flow_sizes[fid]/1000000.0;
  if(flow_size == 0) {
    flow_size = 0.000688; //size of the smallest pkt I have seen 
  }
*/
  if(rate == 0) {
    rate = 0.000000000000000001;
  }
  double xpower = pow(rate, fct_alpha);
//  double inverse = 1.0 / (xpower* flow_size);
  double inverse = 1.0 / (xpower);
  return inverse;
}
   

double
FlowUtil::getUtilByFlowID(uint32_t fid, double rate)
{
  double c = 0;
  if(rate == 0.0) {
    rate = 0.000000000000001;
  }
  if (flow_weights.find( fid ) != flow_weights.end()){
//      std::cout<<"adding "<<c<<" to rate "<<rate<<std::endl;
      return flow_weights[fid]/(rate + c);
  }
  return 1.0/(rate + c);
}

double 
FlowUtil::getPrioByFlowID(uint32_t fid, double rate)
{
  // prio is same as derivative of util * weight
  return getUtilByFlowID(fid, rate);
}

} //ns3 namespace
