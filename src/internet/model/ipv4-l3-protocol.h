// -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*-
//
// Copyright (c) 2006 Georgia Tech Research Corporation
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: George F. Riley<riley@ece.gatech.edu>
//

#ifndef IPV4_L3_PROTOCOL_H
#define IPV4_L3_PROTOCOL_H

#include <list>
#include <map>
#include <vector>
#include <stdint.h>
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/net-device.h"
#include "ns3/ipv4.h"
#include "ns3/traced-callback.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/flow_utils.h"
#include<cstring>
#include<iostream>
#include<sstream>
#include<queue>


class Ipv4L3ProtocolTestCase;

namespace ns3 {

class Packet;
class NetDevice;
class Ipv4Interface;
class Ipv4Address;
class Ipv4Header;
class Ipv4RoutingTableEntry;
class Ipv4Route;
class Node;
class Socket;
class Ipv4RawSocketImpl;
class IpL4Protocol;
class Icmpv4L4Protocol;
class PriHeader;
class FlowUtil;

//using namespace std;

typedef struct flowkey {
  Ipv4Address src;
  Ipv4Address dst;
  uint16_t protocol;
  uint16_t sport;
  uint16_t dport;
} flowkey_t;


/**
 * \brief Implement the Ipv4 layer.
 * 
 * This is the actual implementation of IP.  It contains APIs to send and
 * receive packets at the IP layer, as well as APIs for IP routing.
 *
 * This class contains two distinct groups of trace sources.  The
 * trace sources 'Rx' and 'Tx' are called, respectively, immediately
 * after receiving from the NetDevice and immediately before sending
 * to a NetDevice for transmitting a packet.  These are low level
 * trace sources that include the Ipv4Header already serialized into
 * the packet.  In contrast, the Drop, SendOutgoing, UnicastForward,
 * and LocalDeliver trace sources are slightly higher-level and pass
 * around the Ipv4Header as an explicit parameter and not as part of
 * the packet.
 *
 * IP fragmentation and reassembly is handled at this level.
 * At the moment the fragmentation does not handle IP option headers,
 * and in particular the ones that shall not be fragmented.
 * Moreover, the actual implementation does not mimic exactly the Linux
 * kernel. Hence it is not possible, for instance, to test a fragmentation
 * attack.
 */
class Ipv4L3Protocol : public Ipv4
{
public:

  uint32_t m_method;

  enum UTILITIES {
    LOGUTILITY=1,
    FCTUTILITY=2,
    ALPHA1UTILITY=3
  };
  enum Term
  {
    LONGER = 1,
    SHORTER = 2
  };


  /* Data structures required for priority kanthi */
  typedef std::map< std::string, uint32_t> FlowId_;
  typedef std::map< std::string, uint32_t>::iterator FlowIdIter_;
  typedef std::map< std::string, double> FlowRP_;
  typedef std::map< std::string, double>::iterator FlowRPIter_;
  bool m_pfabric;
  
  std::map<uint32_t, double> fsizes_copy;
  std::map<uint32_t, double> fweights_copy;
  
  double stop_time;
  void addToDropList(uint32_t id);
  void removeFromDropList(uint32_t id);
  std::map<uint32_t, uint32_t> drop_list;

  double long_ewma_const, short_ewma_const;
  std::map<std::string, EventId> m_sendEvent;
//  EventId m_sendEvent;
  
  bool known_flow(std::string flowkey);
  bool issource(Ipv4Address s);
  void setFlowIdealRate(uint32_t, double);
  std::map<uint32_t, double> flow_idealrate;
  void setAlpha(double alpha);
  void setlong_ewma_const(double kvalue);
  void setshort_ewma_const(double kvalue);

  void setKay(double kvalue);
  void updateAverages(std::string flowkey, double inter_arrival, double pktsize);
  
  double GetStoreRate(std::string fkey);
  double GetStoreDestRate(std::string fkey);
  double GetStorePrio(std::string fkey);
  double GetStoreDestPrio(std::string fkey);
  double GetCSFQRate(std::string fkey);
  double GetShortRate(std::string fkey);
  void setFlow(std::string flow, uint32_t fid, double size=0.0, uint32_t weight = 1.0);
  void setPriceValid(std::string);
  void setNumHops(std::string, uint32_t h);
  void removeFlow(uint32_t fid);
  void setFlows(FlowId_ flowid_set);
  void setFlowUtils(std::vector<double> futils);
  void setFlowWeight(uint32_t fid, double weight);
  void setFlowSizes(std::map<uint32_t, double> fsizes);
  void setQueryTime(double qtime);
  void DoSend (Ptr<Packet> packet, Ipv4Address source, 
             Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route);
  void QueueWithUs (Ptr<Packet> packet, Ipv4Address source, 
             Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route);
  void setEpochUpdate(double qtime);
  double getflowsize(std::string flowkey);
  uint64_t current_epoch;
  double getVirtualPktLength(Ptr<Packet> packet, Ipv4Header &ipHeader);
  double getInterArrival(std::string fkey);
  


  void printSumThr(void);
  void setSimTime(double sim_time);
 
  std::map<std::string, uint32_t > data_recvd; 
  std::map<std::string, double > inter_arrival;

  std::map<std::string, std::queue<Ptr<Packet> > > our_packets;
  std::map<std::string, std::queue<Ipv4Address> >src_address;
  std::map<std::string, std::queue<Ipv4Address> >dst_address;
  std::map<std::string, std::queue<uint8_t> >prot_q;
  std::map<std::string, std::queue<Ptr<Ipv4Route> > >route_q;

  void CheckToSend(std::string flowkey);

  FlowId_ flowids;
  FlowRP_ flow_prios;
  FlowRP_ flow_rates;
  PriHeader AddPrioHeader(Ptr<Packet> packet, Ipv4Header &ipHeader);
  double pre_set_wfq_weight;
  double pre_set_netw_price;
  double pre_set_residue;
  void setfctAlpha(double fct);

  std::vector<std::string> sort_by_priority(FlowRP_ prios);
  FlowUtil flowutil;
  uint32_t next_deadline;
  double last_deadline;
  bool epoch_changed;
  bool rate_based;
  bool host_compensate;
  std::map<std::string, bool> price_valid;
  std::map<std::string, uint32_t> num_hops;
  uint32_t bytes_in_queue;
  double deq_bytes;
  std::map<std::string, double> cnp;
  std::map<std::string, double> flow_target_rate;
  double sample_deadline;
  double utilInverse(std::string s, double y, int method);
  double utilInverse(std::string s, double y);
  double getDeadline(Ptr<Packet> packet, Ipv4Header &iph);
  double getCurrentNetwPrice(std::string fkey);
  double getCurrentDeadline(void);
  double getCurrentTargetRate(void);

  double GetRate(std::string, Term t);
  double GetShortTermRate(std::string);

  
  void setCurrentNetwPrice(double cnp_sample, std::string fkey);
  double getAlternateDeadline(Ptr<Packet> packet, Ipv4Header &iph);
  double getAlternateDeadline1(Ptr<Packet> packet, Ipv4Header &iph);
  uint64_t getCurrentEpoch(void);
  std::vector<std::string> flow_dests;
  bool isDestination(std::string node);
  uint32_t last_drain_time;
  double virtual_queue_size;
  bool m_pkt_tag;
  bool m_wfq;
  double get_wfq_weight(Ptr<Packet> packet, Ipv4Header &iph);

  /* kanthi end */
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  static const uint16_t PROT_NUMBER; //!< Protocol number (0x0800)

  Ipv4L3Protocol();
  virtual ~Ipv4L3Protocol ();

  /**
   * \enum DropReason
   * \brief Reason why a packet has been dropped.
   */
  enum DropReason 
  {
    DROP_TTL_EXPIRED = 1,   /**< Packet TTL has expired */
    DROP_NO_ROUTE,   /**< No route to host */
    DROP_BAD_CHECKSUM,   /**< Bad checksum */
    DROP_INTERFACE_DOWN,   /**< Interface is down so can not send packet */
    DROP_ROUTE_ERROR,   /**< Route error */
    DROP_FRAGMENT_TIMEOUT /**< Fragment timeout exceeded */
  };

  /**
   * \brief Set node associated with this stack.
   * \param node node to set
   */
  void SetNode (Ptr<Node> node);
  Ptr<Node> GetNode (void);

  // functions defined in base class Ipv4

  void SetRoutingProtocol (Ptr<Ipv4RoutingProtocol> routingProtocol);
  Ptr<Ipv4RoutingProtocol> GetRoutingProtocol (void) const;

  Ptr<Socket> CreateRawSocket (void);
  void DeleteRawSocket (Ptr<Socket> socket);

  /**
   * \param protocol a template for the protocol to add to this L4 Demux.
   * \returns the L4Protocol effectively added.
   *
   * Invoke Copy on the input template to get a copy of the input
   * protocol which can be used on the Node on which this L4 Demux 
   * is running. The new L4Protocol is registered internally as
   * a working L4 Protocol and returned from this method.
   * The caller does not get ownership of the returned pointer.
   */
  void Insert (Ptr<IpL4Protocol> protocol);
  /**
   * \param protocolNumber number of protocol to lookup
   *        in this L4 Demux
   * \returns a matching L4 Protocol
   *
   * This method is typically called by lower layers
   * to forward packets up the stack to the right protocol.
   */
  virtual Ptr<IpL4Protocol> GetProtocol (int protocolNumber) const;
  /**
   * \param protocol protocol to remove from this demux.
   *
   * The input value to this method should be the value
   * returned from the Ipv4L4Protocol::Insert method.
   */
  void Remove (Ptr<IpL4Protocol> protocol);

  /**
   * \param ttl default ttl to use
   *
   * When we need to send an ipv4 packet, we use this default
   * ttl value.
   */
  void SetDefaultTtl (uint8_t ttl);

  /**
   * Lower layer calls this method after calling L3Demux::Lookup
   * The ARP subclass needs to know from which NetDevice this
   * packet is coming to:
   *    - implement a per-NetDevice ARP cache
   *    - send back arp replies on the right device
   * \param device network device
   * \param p the packet
   * \param protocol protocol value
   * \param from address of the correspondant
   * \param to address of the destination
   * \param packetType type of the packet
   */
  void Receive ( Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from,
                 const Address &to, NetDevice::PacketType packetType);

  /**
   * \param packet packet to send
   * \param source source address of packet
   * \param destination address of packet
   * \param protocol number of packet
   * \param route route entry
   *
   * Higher-level layers call this method to send a packet
   * down the stack to the MAC and PHY layers.
   */
  void Send (Ptr<Packet> packet, Ipv4Address source, 
             Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route);
  /**
   * \param packet packet to send
   * \param ipHeader IP Header
   * \param route route entry
   *
   * Higher-level layers call this method to send a packet with IPv4 Header
   * (Intend to be used with IpHeaderInclude attribute.)
   */
  void SendWithHeader (Ptr<Packet> packet, Ipv4Header ipHeader, Ptr<Ipv4Route> route);

  uint32_t AddInterface (Ptr<NetDevice> device);
  /**
   * \brief Get an interface.
   * \param i interface index
   * \return IPv4 interface pointer
   */
  Ptr<Ipv4Interface> GetInterface (uint32_t i) const;
  uint32_t GetNInterfaces (void) const;

  int32_t GetInterfaceForAddress (Ipv4Address addr) const;
  int32_t GetInterfaceForPrefix (Ipv4Address addr, Ipv4Mask mask) const;
  int32_t GetInterfaceForDevice (Ptr<const NetDevice> device) const;
  bool IsDestinationAddress (Ipv4Address address, uint32_t iif) const;

  bool AddAddress (uint32_t i, Ipv4InterfaceAddress address);
  Ipv4InterfaceAddress GetAddress (uint32_t interfaceIndex, uint32_t addressIndex) const;
  uint32_t GetNAddresses (uint32_t interface) const;
  bool RemoveAddress (uint32_t interfaceIndex, uint32_t addressIndex);
  bool RemoveAddress (uint32_t interface, Ipv4Address address);
  Ipv4Address SelectSourceAddress (Ptr<const NetDevice> device,
                                   Ipv4Address dst, Ipv4InterfaceAddress::InterfaceAddressScope_e scope);


  void SetMetric (uint32_t i, uint16_t metric);
  uint16_t GetMetric (uint32_t i) const;
  uint16_t GetMtu (uint32_t i) const;
  bool IsUp (uint32_t i) const;
  void SetUp (uint32_t i);
  void SetDown (uint32_t i);
  bool IsForwarding (uint32_t i) const;
  void SetForwarding (uint32_t i, bool val);

  Ptr<NetDevice> GetNetDevice (uint32_t i);

  /**
   * \brief Check if an IPv4 address is unicast according to the node.
   *
   * This function checks all the node's interfaces and the respective subnet masks.
   * An address is considered unicast if it's not broadcast, subnet-broadcast or multicast.
   *
   * \param ad address
   *
   * \return true if the address is unicast
   */
  bool IsUnicast (Ipv4Address ad) const;

protected:

  virtual void DoDispose (void);
  /**
   * This function will notify other components connected to the node that a new stack member is now connected
   * This will be used to notify Layer 3 protocol of layer 4 protocol stack to connect them together.
   */
  virtual void NotifyNewAggregate ();
private:
  friend class ::Ipv4L3ProtocolTestCase;

  /**
   * \brief Copy constructor.
   *
   * Defined but not implemented to avoid misuse
   */
  Ipv4L3Protocol(const Ipv4L3Protocol &);

  /**
   * \brief Copy constructor.
   *
   * Defined but not implemented to avoid misuse
   * \returns the copied object
   */
  Ipv4L3Protocol &operator = (const Ipv4L3Protocol &);

  // class Ipv4 attributes
  virtual void SetIpForward (bool forward);
  virtual bool GetIpForward (void) const;
  virtual void SetWeakEsModel (bool model);
  virtual bool GetWeakEsModel (void) const;

  /**
   * \brief Construct an IPv4 header.
   * \param source source IPv4 address
   * \param destination destination IPv4 address
   * \param protocol L4 protocol
   * \param payloadSize payload size
   * \param ttl Time to Live
   * \param tos Type of Service
   * \param mayFragment true if the packet can be fragmented
   * \return newly created IPv4 header
   */
  Ipv4Header BuildHeader (
    Ipv4Address source,
    Ipv4Address destination,
    uint8_t protocol,
    uint16_t payloadSize,
    uint8_t ttl,
    uint8_t tos,
    bool mayFragment);

  /**
   * \brief Send packet with route.
   * \param route route
   * \param packet packet to send
   * \param ipHeader IPv4 header to add to the packet
   */
  void
  SendRealOut (Ptr<Ipv4Route> route,
               Ptr<Packet> packet,
               Ipv4Header &ipHeader);

  /**
   * \brief Forward a packet.
   * \param rtentry route
   * \param p packet to forward
   * \param header IPv4 header to add to the packet
   */
  void 
  IpForward (Ptr<Ipv4Route> rtentry, 
             Ptr<const Packet> p, 
             const Ipv4Header &header);

  /**
   * \brief Forward a multicast packet.
   * \param mrtentry route
   * \param p packet to forward
   * \param header IPv6 header to add to the packet
   */
  void
  IpMulticastForward (Ptr<Ipv4MulticastRoute> mrtentry, 
                      Ptr<const Packet> p, 
                      const Ipv4Header &header);

  /**
   * \brief Deliver a packet.
   * \param p packet delivered
   * \param ip IPv4 header
   * \param iif input interface packet was received
   */
  void LocalDeliver (Ptr<const Packet> p, Ipv4Header const&ip, uint32_t iif);

  /**
   * \brief Fallback when no route is found.
   * \param p packet
   * \param ipHeader IPv4 header
   * \param sockErrno error number
   */
  void RouteInputError (Ptr<const Packet> p, const Ipv4Header & ipHeader, Socket::SocketErrno sockErrno);

  /**
   * \brief Add an IPv4 interface to the stack.
   * \param interface interface to add
   * \return index of newly added interface
   */
  uint32_t AddIpv4Interface (Ptr<Ipv4Interface> interface);

  /**
   * \brief Setup loopback interface.
   */
  void SetupLoopback (void);

  /**
   * \brief Get ICMPv4 protocol.
   * \return Icmpv4L4Protocol pointer
   */
  Ptr<Icmpv4L4Protocol> GetIcmp (void) const;

  /**
   * \brief Check if an IPv4 address is unicast.
   * \param ad address
   * \param interfaceMask the network mask
   * \return true if the address is unicast
   */
  bool IsUnicast (Ipv4Address ad, Ipv4Mask interfaceMask) const;

  /**
   * \brief Fragment a packet
   * \param packet the packet
   * \param outIfaceMtu the MTU of the interface
   * \param listFragments the list of fragments
   */
  void DoFragmentation (Ptr<Packet> packet, uint32_t outIfaceMtu, std::list<Ptr<Packet> >& listFragments);

  /**
   * \brief Process a packet fragment
   * \param packet the packet
   * \param ipHeader the IP header
   * \param iif Input Interface
   * \return true is the fragment completed the packet
   */
  bool ProcessFragment (Ptr<Packet>& packet, Ipv4Header & ipHeader, uint32_t iif);

  /**
   * \brief Process the timeout for packet fragments
   * \param key representing the packet fragments
   * \param ipHeader the IP header of the original packet
   * \param iif Input Interface
   */
  void HandleFragmentsTimeout ( std::pair<uint64_t, uint32_t> key, Ipv4Header & ipHeader, uint32_t iif);
  
  /**
   * \brief Container of the IPv4 Interfaces.
   */
  typedef std::vector<Ptr<Ipv4Interface> > Ipv4InterfaceList;
  /**
   * \brief Container of the IPv4 Raw Sockets.
   */
  typedef std::list<Ptr<Ipv4RawSocketImpl> > SocketList;
  /**
   * \brief Container of the IPv4 L4 instances.
   */
   typedef std::list<Ptr<IpL4Protocol> > L4List_t;

  bool m_ipForward;      //!< Forwarding packets (i.e. router mode) state.
  bool m_weakEsModel;    //!< Weak ES model state
  L4List_t m_protocols;  //!< List of transport protocol.
  Ipv4InterfaceList m_interfaces; //!< List of IPv4 interfaces.
  uint8_t m_defaultTos;  //!< Default TOS
  uint8_t m_defaultTtl;  //!< Default TTL
  std::map<std::pair<uint64_t, uint8_t>, uint16_t> m_identification; //!< Identification (for each {src, dst, proto} tuple)
  Ptr<Node> m_node; //!< Node attached to stack.

  std::map<std::string, uint32_t> lastPacket;
  std::map<std::string, double> lastPacketSize;
  std::map<std::string, double> totalbytes;
  std::map<std::string, double> destination_bytes;
  std::map<std::string, double> store_dest_rate;
  std::map<std::string, double> store_rate;
  std::map<std::string, double> store_prio;
  std::map<std::string, double> store_dest_prio;

  std::map<std::string, double> current_residue;
  std::map<std::string, double> last_residue;
  std::map<std::string, int> total_samples;

  std::map<std::string, double> last_arrival;
  std::map<std::string, double> long_term_ewma_rate;
  std::map<std::string, double> short_term_ewma_rate;
  std::map<std::string, double>instant_rate_store;


  double QUERY_TIME;
  double alpha;
  double previousRate;
  double epoch_update_time;
  void updateRate(std::string fkey);
  void updateAllRates(void);
  void updateCurrentEpoch(void);
  void updateInterArrival(std::string flowkey);
  void updateMarginalUtility(std::string fkey, double cur_rate);

  /// Trace of sent packets
  TracedCallback<const Ipv4Header &, Ptr<const Packet>, uint32_t> m_sendOutgoingTrace;
  /// Trace of unicast forwarded packets
  TracedCallback<const Ipv4Header &, Ptr<const Packet>, uint32_t> m_unicastForwardTrace;
  /// Trace of locally delivered packets
  TracedCallback<const Ipv4Header &, Ptr<const Packet>, uint32_t> m_localDeliverTrace;

  // The following two traces pass a packet with an IP header
  /// Trace of transmitted packets
  TracedCallback<Ptr<const Packet>, Ptr<Ipv4>,  uint32_t> m_txTrace;
  /// Trace of received packets
  TracedCallback<Ptr<const Packet>, Ptr<Ipv4>, uint32_t> m_rxTrace;
  // <ip-header, payload, reason, ifindex> (ifindex not valid if reason is DROP_NO_ROUTE)
  /// Trace of dropped packets
  TracedCallback<const Ipv4Header &, Ptr<const Packet>, DropReason, Ptr<Ipv4>, uint32_t> m_dropTrace;

  Ptr<Ipv4RoutingProtocol> m_routingProtocol; //!< Routing protocol associated with the stack

  SocketList m_sockets; //!< List of IPv4 raw sockets.


  

  /**
   * \class Fragments
   * \brief A Set of Fragment belonging to the same packet (src, dst, identification and proto)
   */
  class Fragments : public SimpleRefCount<Fragments>
  {
public:
    /**
     * \brief Constructor.
     */
    Fragments ();

    /**
     * \brief Destructor.
     */
    ~Fragments ();

    /**
     * \brief Add a fragment.
     * \param fragment the fragment
     * \param fragmentOffset the offset of the fragment
     * \param moreFragment the bit "More Fragment"
     */
    void AddFragment (Ptr<Packet> fragment, uint16_t fragmentOffset, bool moreFragment);

    /**
     * \brief If all fragments have been added.
     * \returns true if the packet is entire
     */
    bool IsEntire () const;

    /**
     * \brief Get the entire packet.
     * \return the entire packet
     */
    Ptr<Packet> GetPacket () const;

    /**
     * \brief Get the complete part of the packet.
     * \return the part we have comeplete
     */
    Ptr<Packet> GetPartialPacket () const;

private:
    /**
     * \brief True if other fragments will be sent.
     */
    bool m_moreFragment;

    /**
     * \brief The current fragments.
     */
    std::list<std::pair<Ptr<Packet>, uint16_t> > m_fragments;

  };

  /// Container of fragments, stored as pairs(src+dst addr, src+dst port) / fragment
  typedef std::map< std::pair<uint64_t, uint32_t>, Ptr<Fragments> > MapFragments_t;
  /// Container of fragment timeout event, stored as pairs(src+dst addr, src+dst port) / EventId
  typedef std::map< std::pair<uint64_t, uint32_t>, EventId > MapFragmentsTimers_t;

  MapFragments_t       m_fragments; //!< Fragmented packets.
  Time                 m_fragmentExpirationTimeout; //!< Expiration timeout
  MapFragmentsTimers_t m_fragmentsTimers; //!< Expiration events.



};

} // Namespace ns3

#endif /* IPV4_L3_PROTOCOL_H */
