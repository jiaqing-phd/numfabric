
#include "ns3/assert.h"
#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "prio-header.h"

NS_LOG_COMPONENT_DEFINE ("PrioHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PrioHeader);

//PriHeader::PriHeader(double p, double r)
PriHeader::PriHeader(double wfq_w, double residue_, double netw_price_)
    {
      wfq_weight = wfq_w;
	  residue = residue_;
	  netw_price = netw_price_;
    }

TypeId
PrioHeader::GetTypeId (void)
{
   static TypeId tid = TypeId ("ns3::PrioHeader")
       .SetParent<Header> ()
       .AddConstructor<PrioHeader> ()
     ;
     return tid;
}

TypeId
PrioHeader::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

uint32_t 
PrioHeader::GetSerializedSize (void) const
{
  return 3 * sizeof(double); //kn - increased size to include 3 doubles now 
}
void 
PrioHeader::Serialize (Buffer::Iterator start) const
{
     // The data.
     uint8_t a[sizeof(double)];
     memcpy((void *)a, (void *)&m_wfq_weight, sizeof(double));
  
     start.Write(a, sizeof(double));

	 // kn - the next field - the residue
     uint8_t b[sizeof(double)];
     memcpy((void *)b, (void *)&m_residue, sizeof(double));
  
     start.Write(b, sizeof(double));

	 // kn - the next field - the residue
     uint8_t c[sizeof(double)];
     memcpy((void *)c, (void *)&m_netw_price, sizeof(double));
  
     start.Write(c, sizeof(double));
}


uint32_t 
PrioHeader::Deserialize (Buffer::Iterator start)
{
  uint8_t a[sizeof(double)];
  start.Read(a, sizeof(double)); 
  memcpy((void *)&m_wfq_weight, (void *)a, sizeof(double));

  uint8_t b[sizeof(double)];
  start.Read(b, sizeof(double)); 
  memcpy((void *)&m_residue, (void *)b, sizeof(double));

  uint8_t c[sizeof(double)];
  start.Read(c, sizeof(double)); 
  memcpy((void *)&m_netw_price, (void *)c, sizeof(double));

  return 3 * sizeof(double);
}

void 
PrioHeader::Print (std::ostream &os) const
{
  os <<" wfq_weight = "<<m_wfq_weight;
}

void PrioHeader::SetData(PriHeader priheader)
{
  m_wfq_weight = priheader.wfq_weight;
  m_netw_price = priheader.netw_price;
  m_residue = priheader.residue;
}

PriHeader PrioHeader::GetData(void) const
{
  PriHeader priheader (m_wfq_weight, m_residue, m_netw_price);
  return priheader;
}

}
