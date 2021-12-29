/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_NET_IMPL_H__)
#define __OFC_NET_IMPL_H__

#include "ofc/core.h"
#include "ofc/types.h"

/** 
 * \defgroup net_impl Platform Dependent Network Implementaion
 * \ingroup port
 *
 * Open Files leverages BSD socket architectures for abstracting the
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
OFC_VOID ofc_net_init_impl(OFC_VOID);

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
ofc_net_interface_count_impl(OFC_VOID);

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
ofc_net_interface_addr_impl(OFC_INT index,
                            OFC_IPADDR *pinaddr,
                            OFC_IPADDR *pbcast,
                            OFC_IPADDR *pmask);
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
 * be freed with ofc_free when no longer needed.
 */
OFC_CORE_LIB OFC_VOID
ofc_net_interface_wins_impl(OFC_INT index, OFC_INT *num_wins,
                            OFC_IPADDR **winslist);

/**
 * Register a configuration event
 */
OFC_VOID ofc_net_register_config_impl(OFC_HANDLE hEvent);

/**
 * Unregister a configuration event
 */
OFC_VOID ofc_net_unregister_config_impl(OFC_HANDLE hEvent);

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
ofc_net_resolve_dns_name_impl(OFC_LPCSTR name,
                              OFC_UINT16 *num_addrs,
                              OFC_IPADDR *ip);

#if defined(__cplusplus)
}
#endif
/** \} */
#endif

