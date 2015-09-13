/* One file to configure utilities and Inverse of utilities of all flows */

#ifndef FLOWUTILS_H
#define FLOWUTILS_H

#include "ns3/uinteger.h"
#include "ns3/double.h"
#include <vector>
#include<map>

namespace ns3 {

class FlowUtil
{
public:
  double getFCTUtilDerivative(uint32_t fid, double price);
  double getFCTUtilDerivativeInverse(uint32_t fid, double price);
  double getUtilInverseByFlowId(uint32_t fid, double priority);
  double getPrioByFlowID(uint32_t flowid, double rate);
  double getUtilByFlowID(uint32_t flowid, double rate);
  void setWeights(std::map<uint32_t, double> fw);
  double getFCTInverseByFlowId(uint32_t fid, double price);
  void setSizes(std::map<uint32_t, double> fsizes);
  void SetFlow(std::string flow, uint32_t fid, double fsize, double);
  double getFCTUtilByFlowID(uint32_t fid, double price);
  double getAlpha1InverseByFlowId(uint32_t fid, double price);
  double getAlpha1UtilByFlowID(uint32_t fid, double rate);
  void setflowweight(uint32_t fid, double w);

  std::map<uint32_t, double> flow_weights;
  std::map<uint32_t, double> flow_sizes;

  void SetFCTAlpha(double);
  double fct_alpha;

};
}

#endif

