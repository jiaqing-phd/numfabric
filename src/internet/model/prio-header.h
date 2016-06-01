
#ifndef PRIO_HEADER_H
#define PRIO_HEADER_H

#include "ns3/header.h"

namespace ns3 {
class PriHeader
{
  public:
          double wfq_weight;
	  double residue;
	  double netw_price; // or the fairshare rate
    PriHeader(double wfq_weight, double res, double price);
};


/**
 * \brief Packet header for IPv4
 */
class PrioHeader : public Header 
{
public:
  static TypeId GetTypeId (void);
     virtual TypeId GetInstanceTypeId (void) const;
     virtual uint32_t GetSerializedSize (void) const;
     virtual void Serialize (Buffer::Iterator start) const;
     virtual uint32_t Deserialize (Buffer::Iterator start);
     virtual void Print (std::ostream &os) const;
     // allow protocol-specific access to the header data.
     void SetData (PriHeader priheader);
     PriHeader GetData (void) const;
private:
     double m_wfq_weight;
	 double m_residue;
	 double m_netw_price;
};
}
#endif

