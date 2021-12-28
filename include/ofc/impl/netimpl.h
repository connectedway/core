/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__BLUE_NET_IMPL_H__)
#define __BLUE_NET_IMPL_H__

#include "ofc/core.h"
#include "ofc/types.h"

/** 
 * \defgroup BlueNetImpl Platform Dependent Network Implementaion
 * \ingroup BluePort
 *
 * Blue Share leverages BSD socket architectures for abstracting the
 * network facility.  There are also functions for obtaining network
 * configuration information and resolving network names
 */

/** \{ */

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Initialize the Network Implementation
   *
   * This is called during system initialization to perform any platform
   * specific initialization necessary
   */
  OFC_VOID BlueNetInitImpl (OFC_VOID) ;
  /**
   * Return the number of interfaces configured on the platform
   *
   * This function should call underlying platform code to determine
   * the number of configured, enabled, and non-loopback inerfaces
   * available
   *
   * \returns 
   * Number of configured interfaces
   */
  OFC_INT
  BlueNetInterfaceCountImpl (OFC_VOID) ;
  /**
   * Return IP information for an interface
   *
   * This function should call underlying platform code to obtain
   * the IP information configured for the interface
   *
   * \param index
   * Index of tghe interface to query
   *
   * \param pinaddr
   * Pointer to where to return the IP address info
   *
   * \param pbcast
   * Pointer to where to return the broadcast info
   * 
   * \param pmask
   * Pointer to where to return the mask info
   */
  OFC_VOID
  BlueNetInterfaceAddrImpl (OFC_INT index,
                            BLUE_IPADDR *pinaddr,
                            BLUE_IPADDR *pbcast,
                            BLUE_IPADDR *pmask) ;
  /**
   * Return the wins configuration for an interface
   *
   * \param index
   * The interface index to return the wins info from
   *
   * \param num_wins
   * Pointer to where to return the number of wins servers configured
   *
   * \param winslist
   * Pointer to where to return the wins ip address list.  This list should
   * be freed with BlueHeapFree when no longer needed.
   */
  OFC_CORE_LIB OFC_VOID
  BlueNetInterfaceWinsImpl (OFC_INT index, OFC_INT *num_wins,
                            BLUE_IPADDR **winslist) ;
  /**
   * Register a configuration event
   */
  OFC_VOID BlueNetRegisterConfigImpl (OFC_HANDLE hEvent) ;
  /**
   * Unregister a configuration event
   */
  OFC_VOID BlueNetUnregisterConfigImpl (OFC_HANDLE hEvent) ;
  /**
   * Resolve a DNS Name on the platform
   *
   * Use the underlying platform stack to translate a name to an IP address
   *
   * \param name
   * Name of the node to translate
   *
   * \param ip
   * Pointer to where to return the ip info for the node
   */
  OFC_VOID
  BlueNetResolveDNSNameImpl (OFC_LPCSTR name,
                             OFC_UINT16 *num_addrs,
                             BLUE_IPADDR *ip) ;
#if defined(__cplusplus)
}
#endif
/** \} */
#endif

