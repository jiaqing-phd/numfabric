
#include "ns3/assert.h"
#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "prio-header.h"

NS_LOG_COMPONENT_DEFINE ("PrioHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PrioHeader);

//PriHeader::PriHeader(double p, double r)
PriHeader::PriHeader(double wfq_w)
    {
      wfq_weight = wfq_w;
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
  //return 3*sizeof(double) + sizeof(uint32_t);
  return sizeof(double);
}
void 
PrioHeader::Serialize (Buffer::Iterator start) const
{
     // The data.
     uint8_t a[sizeof(double)];
     memcpy((void *)a, (void *)&m_wfq_weight, sizeof(double));
  
     start.Write(a, sizeof(double));
}


uint32_t 
PrioHeader::Deserialize (Buffer::Iterator start)
{
  uint8_t a[sizeof(double)];
  start.Read(a, sizeof(double)); //prio
  memcpy((void *)&m_wfq_weight, (void *)a, sizeof(double));
  return sizeof(double);
}

void 
PrioHeader::Print (std::ostream &os) const
{
  os <<" wfq_weight = "<<m_wfq_weight;
}

void PrioHeader::SetData(PriHeader priheader)
{
  m_wfq_weight = priheader.wfq_weight;
}

PriHeader PrioHeader::GetData(void) const
{
  PriHeader priheader (m_wfq_weight);
  return priheader;
}

}
