/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Adrian Sai-wah Tam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Adrian Sai-wah Tam <adrian.sw.tam@gmail.com>
 */

#ifndef TCP_NEWRENO_H
#define TCP_NEWRENO_H

#include "tcp-socket-base.h"

namespace ns3 {

/**
 * \ingroup socket
 * \ingroup tcp
 *
 * \brief An implementation of a stream socket using TCP.
 *
 * This class contains the NewReno implementation of TCP, as of \RFC{2582}.
 */
class Ipv4EndPoint;

class TcpNewReno : public TcpSocketBase
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * Create an unbound tcp socket.
   */
  TcpNewReno (void);
  /**
   * \brief Copy constructor
   * \param sock the object to copy
   */
  TcpNewReno (const TcpNewReno& sock);
  virtual ~TcpNewReno (void);

  // From TcpSocketBase
  virtual int Connect (const Address &address);
  virtual int Listen (void);
  void setdctcp(bool);
  void setxfabric(bool);
  void resetSSThresh(uint32_t ssthresh);
  void resetInitCwnd(uint32_t cwnd);
  uint32_t getBDP(void);
  bool link_underutilized(const TcpHeader &tcpHeader);
  

protected:
  virtual uint32_t Window (void); // Return the max possible number of unacked bytes
  virtual Ptr<TcpSocketBase> Fork (void); // Call CopyObject<TcpNewReno> to clone me
  virtual void NewAck (SequenceNumber32 const& seq); // Inc cwnd and call NewAck() of parent
  virtual void DupAck (const TcpHeader& t, uint32_t count);  // Halving cwnd and reset nextTxSequence
  virtual void Retransmit (void); // Exit fast recovery upon retransmit timeout

  virtual void resetCW(double);
  virtual void ProcessECN(const TcpHeader &tcpheader);
  virtual void processRate(const TcpHeader &tcpheader);
  uint32_t getFlowId(std::string fkey);
  double getFlowIdealRate(std::string fkey);
  SequenceNumber32 ecn_highest;
  SequenceNumber32 last_outstanding_num;
  double d0, m_dt;
  uint32_t getBytesAcked(const TcpHeader &tcpheader);
  void init_values(void);

  // Implementing ns3::TcpSocket -- Attribute get/set
  virtual void     SetSegSize (uint32_t size);
  virtual void     SetInitialSSThresh (uint32_t threshold);
  virtual uint32_t GetInitialSSThresh (void) const;
  virtual void     SetInitialCwnd (uint32_t cwnd);
  virtual uint32_t GetInitialCwnd (void) const;
  /**
   * \brief Set the congestion window when connection starts
   */
  void InitializeCwnd (void);
private:

protected:
  double                 unquantized_window;
  TracedValue<uint32_t>  m_cWnd;         //!< Congestion window
  TracedValue<uint32_t>  m_ssThresh;     //!< Slow Start Threshold
  uint32_t               m_initialCWnd;  //!< Initial cWnd value
  uint32_t               m_initialSsThresh;  //!< Initial Slow Start Threshold value
  SequenceNumber32       m_recover;      //!< Previous highest Tx seqnum for fast recovery
  uint32_t               m_retxThresh;   //!< Fast Retransmit threshold
  bool                   m_inFastRec;    //!< currently in fast recovery
  bool                   m_limitedTx;    //!< perform limited transmit
  bool                   m_dctcp;    //!< enable dctcp
  bool                   dctcp_reacted; 
  bool                   xfabric_reacted;
  int32_t                bytes_with_ecn, total_bytes_acked;
  double                 beta, dctcp_alpha;
  SequenceNumber32       highest_ack_recvd;
  bool                   m_xfabric;   //enable xfabric like behavior
  bool                   m_strawmancc;
  bool                   utilize_link;   //enable xfabric like behavior
  SequenceNumber32       one_rtt;
  double utilInverse(std::string, double lp);
   
};

} // namespace ns3

#endif /* TCP_NEWRENO_H */
