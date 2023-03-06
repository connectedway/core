/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_NET_H__)
#define __OFC_NET_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/** 
 * \defgroup net Networking APIs
 *
 * The Open Files product leverages the network stack of the underlying 
 * platform.  Currently, all supported network stacks are derivatives of BSD 
 * sockets.  Net provides it's own API that extends sockets so that 
 * the issues of network buffers, flow control, and endianess can be 
 * handled in a consistent manner.  
 */

/** \{ */

/* All strings should be allocated to support IPv6 Addresses */
#define IPSTR_LEN 16
#define IP6STR_LEN 40
/*
 * Socket Families.  Network Handling in the Open Files Stack is 
 * segmented by families.  The main family is IP which covers typical
 * TCP/UDP/IP traffic.  
 */
typedef enum {
    /**
     * IP family.  All RAW TCP/UDP/IP traffic is part of this family
     */
    OFC_FAMILY_IP,
    /**
     * IPv6 Family
     */
    OFC_FAMILY_IPV6,
    /**
     * NetBIOS Family.  Currently not supported
     */
    OFC_FAMILY_NETBIOS,
    /**
     * Number of supported socket families
     */
    OFC_FAMILY_NUM
} OFC_FAMILY_TYPE;

/**
 * A IPv4 IP Address
 */
typedef struct {
    OFC_UINT32 addr;
} OFC_INADDR;
/**
 * An IPv6 address
 */
typedef struct {
    OFC_UINT8 _s6_addr[16];
    OFC_INT scope;
} OFC_IN6ADDR;

typedef struct {
    OFC_FAMILY_TYPE ip_version;
    union {
        OFC_INADDR ipv4;
        OFC_IN6ADDR ipv6;
    } u;
} OFC_IPADDR;

/**
 * Representation of a WildCard IP Address
 */
#define OFC_INADDR_ANY ((OFC_UINT32) 0x00000000)

#define OFC_IN6ADDR_ANY_INIT {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},0}
extern OFC_IN6ADDR ofc_in6addr_any;

/**
 * Representation of the loopback IP Address
 */
#define OFC_INADDR_LOOPBACK ((OFC_UINT32) 0x7f000001)

#define OFCIN6ADDR_LOOPBACK_INIT {{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },0}
extern OFC_IN6ADDR ofc_in6addr_loopback;
/**
 * Representation of the Interface Generic Broadcast Address
 */
#define OFC_INADDR_BROADCAST ((OFC_UINT32) 0xffffffff)
#define OFC_IN6ADDR_BCAST_INIT {{ 0xFF,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },0}
extern OFC_IN6ADDR ofc_in6addr_bcast;
/**
 * Representation of an unspecified IP Address
 */
#define OFC_INADDR_NONE ((OFC_UINT32) 0xffffffff)
#define OFC_IN6ADDR_NONE_INIT {{ 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF },0}
extern OFC_IN6ADDR ofc_in6addr_none;

/**
 * Definition of a socket address.  
 *
 * Used by the Socket APIs
 */
typedef struct _OFC_SOCKADDR {
    OFC_UINT16 sin_family;    /* The Address Family */
    OFC_UINT16 sin_port;    /* The TCP/UDP Port Number */
    OFC_IPADDR sin_addr;    /* The IP Address */
} OFC_SOCKADDR;

#define OFC_IN6_ARE_ADDR_EQUAL(a, b) \
    (((OFC_UINT32 *)(a))[0] == ((OFC_UINT32 *)(b))[0] \
     && ((OFC_UINT32 *)(a))[1] == ((OFC_UINT32 *)(b))[1] \
     && ((OFC_UINT32 *)(a))[2] == ((OFC_UINT32 *)(b))[2] \
     && ((OFC_UINT32 *)(a))[3] == ((OFC_UINT32 *)(b))[3])

#define OFC_IN6_IS_ADDR_UNSPECIFIED(addr) \
    (((OFC_UINT32 *)(addr))[0] == 0 \
     && ((OFC_UINT32 *)(addr))[1] == 0 \
     && ((OFC_UINT32 *)(addr))[2] == 0 \
     && ((OFC_UINT32 *)(addr))[3] == 0)

#define OFC_IN6_IS_ADDR_LOOPBACK(addr) \
    (((OFC_UINT32 *)(addr))[0] == 0 \
     && ((OFC_UINT32 *)(addr))[1] == 0 \
     && ((OFC_UINT32 *)(addr))[2] == 0 \
     && ((OFC_UINT8 *)(addr))[12] == 0 \
     && ((OFC_UINT8 *)(addr))[13] == 0 \
     && ((OFC_UINT8 *)(addr))[14] == 0 \
     && ((OFC_UINT8 *)(addr))[15] == 0x01)

#define OFC_IN6_IS_ADDR_DYNAMIC(addr) \
  (((OFC_UINT8*)(addr))[11] == 0x00 \
   && ((OFC_UINT8*)(addr))[12] == 0x00)

#define OFC_IN6_IS_ADDR_TEMPORARY(addr) \
  (((OFC_UINT8*)(addr))[11] != 0xFF \
   && ((OFC_UINT8*)(addr))[12] != 0xFE \
   && !OFC_IN6_IS_ADDR_DYNAMIC(addr))

#define OFC_IN6_IS_ADDR_MULTICAST(addr) (((OFC_UINT8 *) (addr))[0] == 0xff)

#define OFC_IN6_IS_ADDR_LINKLOCAL(addr) \
  (((OFC_UINT8*)(addr))[0] == 0xFE \
   && (((OFC_UINT8*)(addr))[1] & 0xC0) == 0x80)

#define OFC_IN6_IS_ADDR_SITELOCAL(addr) \
  (((OFC_UINT8*)(addr))[0] == 0xFE \
   && (((OFC_UINT8*)(addr))[1] & 0xC0) == 0xC0)

#define OFC_IN6_IS_ADDR_MC_NODELOCAL(addr) \
    (OFC_IN6_IS_ADDR_MULTICAST(addr) \
     && (((OFC_UINT8 *)(addr))[1] & 0xf) == 0x1)

#define OFC_IN6_IS_ADDR_MC_LINKLOCAL(addr) \
    (OFC_IN6_IS_ADDR_MULTICAST (addr) \
     && (((OFC_UINT8 *)(addr))[1] & 0xf) == 0x2)

#define OFC_IN6_IS_ADDR_MC_SITELOCAL(addr) \
    (OFC_IN6_IS_ADDR_MULTICAST(addr) \
     && (((OFC_UINT8 *)(addr))[1] & 0xf) == 0x5)

#define OFC_IN6_IS_ADDR_MC_ORGLOCAL(addr) \
    (OFC_IN6_IS_ADDR_MULTICAST(addr) \
     && (((OFC_UINT8 *)(addr))[1] & 0xf) == 0x8)

#define OFC_IN6_IS_ADDR_MC_GLOBAL(addr) \
    (OFC_IN6_IS_ADDR_MULTICAST(addr) \
     && (((OFC_UINT8 *)(addr))[1] & 0xf) == 0xe)

#define ICMP_TYPE_PING 0x08
#define ICMP_TYPE_PING_REPLY 0x00

#define ICMP_TYPE 0x00
#define ICMP_CODE 0x01
#define ICMP_CHECKSUM 0x02
#define ICMP_ID 0x04
#define ICMP_SEQUENCE 0x06
#define ICMP_DATA 0x08
#define ICMP_LEN 0x10

#if defined(__cplusplus)
extern "C"
{
#endif
/**
 * Initialize the Network Facility
 *
 * This should only be called by the net_init routine
 */
OFC_CORE_LIB OFC_VOID
ofc_net_init(OFC_VOID);

OFC_CORE_LIB OFC_VOID
ofc_net_destroy(OFC_VOID);

const OFC_CHAR *
ofc_ntop(const OFC_IPADDR *src, OFC_CHAR *dst, OFC_SIZET size);

OFC_INT
ofc_pton(const OFC_CHAR *src, OFC_IPADDR *dst);

/**
 * Return the count of interfaces on the platform
 *
 * \returns Count of interfaces
 */
OFC_CORE_LIB OFC_INT
ofc_net_interface_count(OFC_VOID);
/**
 * Return ip address info for an interface
 *
 * \param index
 * Index of the interface to retrieve info for
 *
 * \param pinaddr Pointer to where to store Ip address
 * \param pbcast Pointer to where to store the broadcast address
 * \param pmask Pointer to where to store the interface mask
 */
OFC_CORE_LIB OFC_VOID
ofc_net_interface_addr(OFC_INT index, OFC_IPADDR *pinaddr,
                       OFC_IPADDR *pbcast, OFC_IPADDR *pmask);
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
ofc_net_interface_wins(OFC_INT index, OFC_INT *num_wins,
                       OFC_IPADDR **winslist);
/**
 * Test whether an ip address is on the same subnet as another address
 *
 * \param fromip IP address to test
 * \param intip IP address of other machine or subnet
 * \param mask Mask of the interface
 * \returns OFC_TRUE if ip addresses are on same subnet
 */
OFC_CORE_LIB OFC_BOOL
ofc_net_subnet_match(OFC_IPADDR *fromip, OFC_IPADDR *intip,
                     OFC_IPADDR *mask);
/**
 * Register a Config Event
 */
OFC_CORE_LIB OFC_VOID
ofc_net_register_config(OFC_HANDLE hEvent);
/**
 * Unregister a Config Event
 */
OFC_CORE_LIB OFC_VOID
ofc_net_unregister_config(OFC_HANDLE hEvent);
/**
 * Resolve a name to an IP address
 *
 * This routine will resolve a name to an IP address using
 * netbios
 *
 * \param name
 * Name to lookup
 *
 * \param ip
 * Pointer to IP structure to return IP address in
 */
OFC_CORE_LIB OFC_VOID
ofc_net_resolve_name(OFC_LPCSTR name, OFC_UINT16 *num_addrs,
                     OFC_IPADDR *ip);
/**
 * Resolve a name to an IP address
 *
 * This routine will resolve a unicode name to an IP address using
 * netbios
 *
 * \param name
 * Name to lookup
 *
 * \param ip
 * Pointer to IP structure to return IP address in
 */
OFC_CORE_LIB OFC_VOID
ofc_net_resolve_dns_name(OFC_LPCSTR name, OFC_UINT16 *num_addrs,
                         OFC_IPADDR *ip);

OFC_CORE_LIB OFC_BOOL ofc_net_is_addr_any(OFC_IPADDR *ip);

OFC_CORE_LIB OFC_BOOL ofc_net_is_addr_loopback(OFC_IPADDR *ip);

OFC_CORE_LIB OFC_BOOL ofc_net_is_addr_bcast(OFC_IPADDR *ip);

OFC_CORE_LIB OFC_BOOL ofc_net_is_addr_none(OFC_IPADDR *ip);

OFC_CORE_LIB OFC_BOOL ofc_net_is_addr_equal(OFC_IPADDR *ip1,
                                            OFC_IPADDR *ip2);

OFC_CORE_LIB OFC_BOOL ofc_net_is_addr_link_local(OFC_IPADDR *ip);

#if defined(__cplusplus)
}
#endif
/** \} */
#endif

