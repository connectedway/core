/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/net.h"
#include "ofc/impl/netimpl.h"

#include "ofc/heap.h"

OFC_IN6ADDR ofc_in6addr_any = OFC_IN6ADDR_ANY_INIT;
OFC_IN6ADDR ofc_in6addr_loopback = OFCIN6ADDR_LOOPBACK_INIT;
OFC_IN6ADDR ofc_in6addr_bcast = OFC_IN6ADDR_BCAST_INIT;
OFC_IN6ADDR ofc_in6addr_none = OFC_IN6ADDR_NONE_INIT;

OFC_CORE_LIB OFC_VOID
ofc_net_init(OFC_VOID) {
#if defined(OFC_MESSAGE_DEBUG)
    ofc_message_debug_init();
#endif
    ofc_net_init_impl();
}

OFC_CORE_LIB OFC_VOID
ofc_net_destroy(OFC_VOID) {
#if defined(OFC_MESSAGE_DEBUG)
    ofc_message_debug_destroy();
#endif
}

OFC_CORE_LIB OFC_VOID
ofc_net_set_handle(OFC_UINT64 network_handle)
{
  ofc_net_set_handle_impl(network_handle);
}

OFC_CORE_LIB OFC_INT
ofc_net_interface_count(OFC_VOID) {
    OFC_INT ret;

    ret = ofc_net_interface_count_impl();
    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_net_interface_addr(OFC_INT index, OFC_IPADDR *pinaddr,
                       OFC_IPADDR *pbcast, OFC_IPADDR *pmask) {
    ofc_net_interface_addr_impl(index, pinaddr, pbcast, pmask);
}

OFC_CORE_LIB OFC_VOID
ofc_net_interface_wins(OFC_INT index, OFC_INT *num_wins,
                       OFC_IPADDR **winslist) {
    ofc_net_interface_wins_impl(index, num_wins, winslist);
}

OFC_CORE_LIB OFC_VOID
ofc_net_register_config(OFC_HANDLE hEvent) {
    ofc_net_register_config_impl(hEvent);
}

OFC_CORE_LIB OFC_VOID
ofc_net_unregister_config(OFC_HANDLE hEvent) {
    ofc_net_unregister_config_impl(hEvent);
}

OFC_CORE_LIB OFC_BOOL
ofc_net_subnet_match(OFC_IPADDR *fromip, OFC_IPADDR *intip, OFC_IPADDR *mask) {
    OFC_BOOL ret;

    ret = OFC_TRUE;
    if (fromip->ip_version == intip->ip_version) {
        if (fromip->ip_version == OFC_FAMILY_IP) {
            if ((fromip->u.ipv4.addr & mask->u.ipv4.addr) !=
                (intip->u.ipv4.addr & mask->u.ipv4.addr))
                ret = OFC_FALSE;
        } else {
            if (!ofc_net_is_addr_equal(fromip, intip))
                ret = OFC_FALSE;
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_net_resolve_name(OFC_LPCSTR name, OFC_UINT16 *num_addrs,
                     OFC_IPADDR *ip)
{
  OFC_BOOL resolved = OFC_FALSE;
  
  if (ofc_pton (name, ip) == 1)
    {
      *num_addrs = 1;
      resolved = OFC_TRUE;
    }

  if (!resolved)
    {
      ofc_net_resolve_dns_name(name, num_addrs, ip);
    }

#if defined (OFC_NETBIOS)
  if (!resolved)
    {
      ofc_name_resolve_name(name, num_addrs, ip) ;
      if (!ofc_net_is_addr_none (&ip[0]))
	resolved = OFC_TRUE;
    }
#endif
}

OFC_CORE_LIB OFC_VOID
ofc_net_resolve_dns_name(OFC_LPCSTR name, OFC_UINT16 *num_addrs,
                         OFC_IPADDR *ip) {
    ofc_net_resolve_dns_name_impl(name, num_addrs, ip);
}

OFC_CORE_LIB OFC_BOOL ofc_net_is_addr_any(OFC_IPADDR *ip) {
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (ip->ip_version == OFC_FAMILY_IP) {
        if (ip->u.ipv4.addr == OFC_INADDR_ANY)
            ret = OFC_TRUE;
    } else {
        if (OFC_IN6_ARE_ADDR_EQUAL(ip->u.ipv6._s6_addr, &ofc_in6addr_any))
            ret = OFC_TRUE;
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL ofc_net_is_addr_link_local(OFC_IPADDR *ip) {
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (ip->ip_version == OFC_FAMILY_IP)
        ret = OFC_TRUE;
    else {
        if (OFC_IN6_IS_ADDR_LINKLOCAL(ip->u.ipv6._s6_addr))
            ret = OFC_TRUE;
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL ofc_net_is_addr_loopback(OFC_IPADDR *ip) {
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (ip->ip_version == OFC_FAMILY_IP) {
        if (ip->u.ipv4.addr == OFC_INADDR_LOOPBACK)
            ret = OFC_TRUE;
    } else {
        if (OFC_IN6_ARE_ADDR_EQUAL(ip->u.ipv6._s6_addr,
                                   &ofc_in6addr_loopback))
            ret = OFC_TRUE;
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL ofc_net_is_addr_bcast(OFC_IPADDR *ip) {
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (ip->ip_version == OFC_FAMILY_IP) {
        if (ip->u.ipv4.addr == OFC_INADDR_BROADCAST)
            ret = OFC_TRUE;
    } else {
        if (OFC_IN6_ARE_ADDR_EQUAL(ip->u.ipv6._s6_addr,
                                   &ofc_in6addr_bcast))
            ret = OFC_TRUE;
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL ofc_net_is_addr_none(OFC_IPADDR *ip) {
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (ip->ip_version == OFC_FAMILY_IP) {
        if (ip->u.ipv4.addr == OFC_INADDR_NONE)
            ret = OFC_TRUE;
    } else {
        if (OFC_IN6_ARE_ADDR_EQUAL(ip->u.ipv6._s6_addr, &ofc_in6addr_none))
            ret = OFC_TRUE;
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL ofc_net_is_addr_equal(OFC_IPADDR *ip1,
                                            OFC_IPADDR *ip2) {
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (ip1->ip_version == OFC_FAMILY_IP) {
        if (ip1->u.ipv4.addr == ip2->u.ipv4.addr)
            ret = OFC_TRUE;
    } else {
        if (OFC_IN6_ARE_ADDR_EQUAL(ip1->u.ipv6._s6_addr,
                                   ip2->u.ipv6._s6_addr))
            ret = OFC_TRUE;
    }
    return (ret);
}
