/* Copyright (c) 2009 Blue Peach Solutions, Inc.
 * All rights reserved.
 *
 * This software is protected by copyright and intellectual 
 * property laws as well as international treaties.  It is to be 
 * used and copied only by authorized licensees under the 
 * conditions described in their licenses.  
 *
 * Title to and ownership of the software shall at all times 
 * remain with Blue Peach Solutions.
 */
#if !defined(__BLUE_NET_H__)
#define __BLUE_NET_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/** 
 * \defgroup BlueNet Network Utility Functions
 * \ingroup BlueInternal
 *
 * The Blue Share product leverages the network stack of the underlying 
 * platform.  Currently, all supported network stacks are derivatives of BSD 
 * sockets.  Blue Net provides it's own API that extends sockets so that 
 * the issues of network buffers, flow control, and endianess can be 
 * handled in a consistent manner.  
 */

/** \{ */

/* All strings should be allocated to support IPv6 Addresses */
#define IPSTR_LEN 16
#define IP6STR_LEN 40
/*
 * Socket Families.  Network Handling in the Blue Real Stack is 
 * segmented by families.  The main family is IP which covers typical
 * TCP/UDP/IP traffic.  
 */
typedef enum
  {
    /**
     * IP family.  All RAW TCP/UDP/IP traffic is part of this family
     */
    BLUE_FAMILY_IP,
    /**
     * IPv6 Family
     */
    BLUE_FAMILY_IPV6,
    /**
     * NetBIOS Family.  Currently not supported
     */
    BLUE_FAMILY_NETBIOS,
    /**
     * Number of supported socket families
     */
    BLUE_FAMILY_NUM
  } BLUE_FAMILY_TYPE ;

/**
 * A IPv4 IP Address
 */
typedef struct
  {
    BLUE_UINT32 addr;
  } BLUE_INADDR ;
/**
 * An IPv6 address
 */
typedef struct
{
  BLUE_UINT8 blue_s6_addr[16] ;
  BLUE_INT blue_scope ;
} BLUE_IN6ADDR ;

typedef struct
{
  BLUE_FAMILY_TYPE ip_version ;
  union 
  {
    BLUE_INADDR ipv4 ;
    BLUE_IN6ADDR ipv6 ;
  } u ;
} BLUE_IPADDR ;

/**
 * Representation of a WildCard IP Address
 */
#define BLUE_INADDR_ANY ((BLUE_UINT32) 0x00000000)

#define BLUE_IN6ADDR_ANY_INIT {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
extern BLUE_IN6ADDR blue_in6addr_any ;

/**
 * Representation of the loopback IP Address
 */
#define BLUE_INADDR_LOOPBACK ((BLUE_UINT32) 0x7f000001)

#define BLUE_IN6ADDR_LOOPBACK_INIT { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 }
extern BLUE_IN6ADDR blue_in6addr_loopback ;
/**
 * Representation of the Interface Generic Broadcast Address
 */
#define BLUE_INADDR_BROADCAST ((BLUE_UINT32) 0xffffffff)
#define BLUE_IN6ADDR_BCAST_INIT { 0xFF,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,1 }
extern BLUE_IN6ADDR blue_in6addr_bcast ;
/**
 * Representation of an unspecified IP Address
 */
#define BLUE_INADDR_NONE ((BLUE_UINT32) 0xffffffff)
#define BLUE_IN6ADDR_NONE_INIT { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF }
extern BLUE_IN6ADDR blue_in6addr_none ;

/**
 * Definition of a socket address.  
 *
 * Used by the Socket APIs
 */
typedef struct _BLUE_SOCKADDR
{
  BLUE_UINT16 sin_family ;	/* The Address Family */
  BLUE_UINT16 sin_port ;	/* The TCP/UDP Port Number */
  BLUE_IPADDR sin_addr ;	/* The IP Address */
} BLUE_SOCKADDR ;

/**
 * \latexonly
 */
/*
 * u8 - Reference a byte at a location.  The location is typically a short or
 * long member of some structure with a byte position.
 *
 * Accepts:
 *    a - reference to short or long value
 *    o - Byte position of the structure
 *
 * For example:
 *    mybyte = u8(myword, 1)    - return byte 1 of myword
 *    u8(mylong,3) = mybyte ;   - Set byte 3 of mylong to mybyte
 */
#define u8(a,o) *((BLUE_UINT8*)(a)+o)
/*
 * utou8 - Map a byte from one location to another
 *
 * Take a byte from some position in a word or long and map it to a
 * some other short or long member of a structure.
 *
 * Accepts:
 *    d - reference to the destination short or long value
 *    a - short or long value as the source
 *    o - Byte position in the destination and source
 *
 * For example:
 *    utou8(smbmsg.wordvalue, myword, 1) - Map byte 1 of myword to wordvalue
 */
#define u16tou8(d,a,o) u8(d,(1-o))=((BLUE_UINT8)((a>>(o*8))&0xFF))
#define u32tou8(d,a,o) u8(d,(3-o))=((BLUE_UINT8)((a>>(o*8))&0xFF))
#if defined(BLUE_PARAM_64BIT_INTEGER)
#define u64tou8(d,a,o) u8(d,(7-o))=((BLUE_UINT8)((a>>(o*8))&0xFF))
#endif
#define u16toxu8(d,a,o) u8(d,o)=((BLUE_UINT8)((a>>(o*8))&0xFF))
#define u32toxu8(d,a,o) u8(d,o)=((BLUE_UINT8)((a>>(o*8))&0xFF))
#if defined(BLUE_PARAM_64BIT_INTEGER)
#define u64toxu8(d,a,o) u8(d,o)=((BLUE_UINT8)((a>>(o*8))&0xFF))
#endif
/*
 * u8tou16 - Return a word containing a byte from the source
 * u8tou32 - Return a long word containing a byte from the source
 *
 * Mask off a byte from a short or long
 *
 * Accepts:
 *    a - Reference short or long
 *    o - Byte position in short or long
 *
 * For Example:
 *    myword = u8tou16(smbmsg.word, 1) - Mask off the first byte of word
 *    mylong = u8tou32(smbmsg.long, 3) - Mask off the third byte
 */
#define u8tou16(a,o) ((BLUE_UINT16)u8(a,o)<<((1-o)*8))
#define u8tou32(a,o) ((BLUE_UINT32)u8(a,o)<<((3-o)*8))
#if defined(BLUE_PARAM_64BIT_INTEGER)
#define u8tou64(a,o) ((BLUE_UINT64)u8(a,o)<<((7-o)*8))
#endif
#define u8toxu16(a,o) ((BLUE_UINT16)u8(a,o)<<(o*8))
#define u8toxu32(a,o) ((BLUE_UINT32)u8(a,o)<<(o*8))
#if defined(BLUE_PARAM_64BIT_INTEGER)
#define u8toxu64(a,o) ((BLUE_UINT64)u8(a,o)<<(o*8))
#endif

/**
 * BLUE_NET_NTOS - dereference a smb short value
 *
 * Accepts:
 *    a - reference to smb short value
 *
 * For Example:
 *    myshort = smbtos(smb.word) - Return the smb short value
 */
#define BLUE_NET_NTOS(a,o) (u8tou16((BLUE_UINT8*)a+o,0)|u8tou16((BLUE_UINT8*)a+o,1))
/**
 * BLUE_NET_NTOS - dereference a smb short value
 *
 * Accepts:
 *    a - reference to smb short value
 *
 * For Example:
 *    myshort = smbtos(smb.word) - Return the smb short value
 */

#define BLUE_NET_NTOL(a,o) (u8tou32((BLUE_UINT8*)a+o,0)|u8tou32((BLUE_UINT8*)a+o,1)|u8tou32((BLUE_UINT8*)a+o,2)|u8tou32((BLUE_UINT8*)a+o,3))
#if defined(BLUE_PARAM_64BIT_INTEGER)
#define BLUE_NET_NTOLL(a,o) (u8tou64((BLUE_UINT8*)a+o,0)|u8tou64((BLUE_UINT8*)a+o,1)|u8tou64((BLUE_UINT8*)a+o,2)|u8tou64((BLUE_UINT8*)a+o,3)|u8tou64((BLUE_UINT8*)a+o,4)|u8tou64((BLUE_UINT8*)a+o,5)|u8tou64((BLUE_UINT8*)a+o,6)|u8tou64((BLUE_UINT8*)a+o,7))
#endif
#define BLUE_NET_NTOC(a,o) (u8((BLUE_UINT8*)a+o,0))
#define BLUE_NET_SMBTOS(a,o) (u8toxu16((BLUE_UINT8*)a+o,0)|u8toxu16((BLUE_UINT8*)a+o,1))
#define BLUE_NET_SMBTOL(a,o) (u8toxu32((BLUE_UINT8*)a+o,0)|u8toxu32((BLUE_UINT8*)a+o,1)|u8toxu32((BLUE_UINT8*)a+o,2)|u8toxu32((BLUE_UINT8*)a+o,3))
#if defined(BLUE_PARAM_64BIT_INTEGER)
#define BLUE_NET_SMBTOLL(a,o) (u8toxu64((BLUE_UINT8*)a+o,0)|u8toxu64((BLUE_UINT8*)a+o,1)|u8toxu64((BLUE_UINT8*)a+o,2)|u8toxu64((BLUE_UINT8*)a+o,3)|u8toxu64((BLUE_UINT8*)a+o,4)|u8toxu64((BLUE_UINT8*)a+o,5)|u8toxu64((BLUE_UINT8*)a+o,6)|u8toxu64((BLUE_UINT8*)a+o,7))
#endif
#define BLUE_NET_SMBTOC(a,o) (u8((BLUE_UINT8*)a+o,0))
/*
 * BLUE_NET_STON - Store an smb short value
 * BLUE_NET_LTON - Store an smb long value
 *
 * Accepts:
 *    d - destination smb value
 *    a - Value to store
 *
 * For Example:
 *    stosmb (smb.word, myword) ;
 *    ltosmb (smb.long, mylong) ;
 */
#define BLUE_NET_STON(d,o,a) {u16tou8(((BLUE_UINT8*)d+o),(a),0);u16tou8(((BLUE_UINT8*)d+o),(a),1);}
#define BLUE_NET_LTON(d,o,a) {u32tou8(((BLUE_UINT8*)d+o),(a),0);u32tou8(((BLUE_UINT8*)d+o),(a),1);u32tou8(((BLUE_UINT8*)d+o),(a),2);u32tou8(((BLUE_UINT8*)d+o),(a),3);}
#define BLUE_NET_LLTON(d,o,a) {u64tou8(((BLUE_UINT8*)d+o),(a),0);u64tou8(((BLUE_UINT8*)d+o),(a),1);u64tou8(((BLUE_UINT8*)d+o),(a),2);u64tou8(((BLUE_UINT8*)d+o),(a),3);u64tou8(((BLUE_UINT8*)d+o),(a),4);u64tou8(((BLUE_UINT8*)d+o),(a),5);u64tou8(((BLUE_UINT8*)d+o),(a),6);u64tou8(((BLUE_UINT8*)d+o),(a),7);}
#define BLUE_NET_CTON(d,o,a) (u8((BLUE_UINT8*)d+o,0)=((BLUE_UINT8)(a)&0xFF))
#define BLUE_NET_STOSMB(d,o,a) {u16toxu8(((BLUE_UINT8*)d+o),(a),0);u16toxu8(((BLUE_UINT8*)d+o),(a),1);}
#define BLUE_NET_LTOSMB(d,o,a) {u32toxu8(((BLUE_UINT8*)d+o),(a),0);u32toxu8(((BLUE_UINT8*)d+o),(a),1);u32toxu8(((BLUE_UINT8*)d+o),(a),2);u32toxu8(((BLUE_UINT8*)d+o),(a),3);}
#define BLUE_NET_LLTOSMB(d,o,a) {u64toxu8(((BLUE_UINT8*)d+o),(a),0);u64toxu8(((BLUE_UINT8*)d+o),(a),1);u64toxu8(((BLUE_UINT8*)d+o),(a),2);u64toxu8(((BLUE_UINT8*)d+o),(a),3);u64toxu8(((BLUE_UINT8*)d+o),(a),4);u64toxu8(((BLUE_UINT8*)d+o),(a),5);u64toxu8(((BLUE_UINT8*)d+o),(a),6);u64toxu8(((BLUE_UINT8*)d+o),(a),7);}
#define BLUE_NET_CTOSMB(d,o,a) (u8((BLUE_UINT8*)d+o,0)=((BLUE_UINT8)(a)&0xFF))

#define BLUE_IN6_ARE_ADDR_EQUAL(a, b) \
	(((BLUE_UINT32 *)(a))[0] == ((BLUE_UINT32 *)(b))[0] \
	 && ((BLUE_UINT32 *)(a))[1] == ((BLUE_UINT32 *)(b))[1] \
	 && ((BLUE_UINT32 *)(a))[2] == ((BLUE_UINT32 *)(b))[2] \
	 && ((BLUE_UINT32 *)(a))[3] == ((BLUE_UINT32 *)(b))[3])

#define BLUE_IN6_IS_ADDR_UNSPECIFIED(addr) \
	(((BLUE_UINT32 *)(addr))[0] == 0 \
	 && ((BLUE_UINT32 *)(addr))[1] == 0 \
	 && ((BLUE_UINT32 *)(addr))[2] == 0 \
	 && ((BLUE_UINT32 *)(addr))[3] == 0)

#define BLUE_IN6_IS_ADDR_LOOPBACK(addr) \
	(((BLUE_UINT32 *)(addr))[0] == 0 \
	 && ((BLUE_UINT32 *)(addr))[1] == 0 \
	 && ((BLUE_UINT32 *)(addr))[2] == 0 \
	 && ((BLUE_UINT8 *)(addr))[12] == 0 \
	 && ((BLUE_UINT8 *)(addr))[13] == 0 \
	 && ((BLUE_UINT8 *)(addr))[14] == 0 \
	 && ((BLUE_UINT8 *)(addr))[15] == 0x01)

/* These macros are not validated.  I figure they are right, and help
 * determine which IP addresses we want to use.
 */
/* A temporary address is one that does not have our MAC address in it.
 * If the 11th and 12th bytes are 00:00, then it is dynamic (DHCPv6)
 * If the 11th and 12th bytes are not 00:00 nor FF:FE then it is temporary
 */
#define BLUE_IN6_IS_ADDR_DYNAMIC(addr) \
  (((BLUE_UINT8*)(addr))[11] == 0x00 \
   && ((BLUE_UINT8*)(addr))[12] == 0x00)

#define BLUE_IN6_IS_ADDR_TEMPORARY(addr) \
  (((BLUE_UINT8*)(addr))[11] != 0xFF \
   && ((BLUE_UINT8*)(addr))[12] != 0xFE \
   && !BLUE_IN6_IS_ADDR_DYNAMIC(addr))

#define BLUE_IN6_IS_ADDR_MULTICAST(addr) (((BLUE_UINT8 *) (addr))[0] == 0xff)

#define BLUE_IN6_IS_ADDR_LINKLOCAL(addr) \
  (((BLUE_UINT8*)(addr))[0] == 0xFE \
   && (((BLUE_UINT8*)(addr))[1] & 0xC0) == 0x80)

#define BLUE_IN6_IS_ADDR_SITELOCAL(addr) \
  (((BLUE_UINT8*)(addr))[0] == 0xFE \
   && (((BLUE_UINT8*)(addr))[1] & 0xC0) == 0xC0)

#define BLUE_IN6_IS_ADDR_MC_NODELOCAL(addr) \
	(BLUE_IN6_IS_ADDR_MULTICAST(addr) \
	 && (((BLUE_UINT8 *)(addr))[1] & 0xf) == 0x1)

#define BLUE_IN6_IS_ADDR_MC_LINKLOCAL(addr) \
	(BLUE_IN6_IS_ADDR_MULTICAST (addr) \
	 && (((BLUE_UINT8 *)(addr))[1] & 0xf) == 0x2)

#define BLUE_IN6_IS_ADDR_MC_SITELOCAL(addr) \
	(BLUE_IN6_IS_ADDR_MULTICAST(addr) \
	 && (((BLUE_UINT8 *)(addr))[1] & 0xf) == 0x5)

#define BLUE_IN6_IS_ADDR_MC_ORGLOCAL(addr) \
	(BLUE_IN6_IS_ADDR_MULTICAST(addr) \
	 && (((BLUE_UINT8 *)(addr))[1] & 0xf) == 0x8)

#define BLUE_IN6_IS_ADDR_MC_GLOBAL(addr) \
	(BLUE_IN6_IS_ADDR_MULTICAST(addr) \
	 && (((BLUE_UINT8 *)(addr))[1] & 0xf) == 0xe)

#define ICMP_TYPE_PING 0x08
#define ICMP_TYPE_PING_REPLY 0x00

#define ICMP_TYPE 0x00
#define ICMP_CODE 0x01
#define ICMP_CHECKSUM 0x02
#define ICMP_ID 0x04
#define ICMP_SEQUENCE 0x06
#define ICMP_DATA 0x08
#define ICMP_LEN 0x10

/** \endlatexonly */

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Initialize the Network Facility
   *
   * This should only be called by the BlueInit routine
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueNetInit (BLUE_VOID) ;
  BLUE_CORE_LIB BLUE_VOID 
  BlueNetDestroy (BLUE_VOID) ;
#if 0
  /**
   * Convert a IP Address to a string
   *
   * \param inaddr
   * The ip address
   *
   * \returns
   * Pointer to a statically allocated string.  No other calls can be made
   * to BlueNETntoa before completely using the output of this call.
   */
  BLUE_CORE_LIB BLUE_CHAR *
  BlueNETntoa (BLUE_INADDR inaddr) ;
  BLUE_CORE_LIB BLUE_CHAR *
  BlueNETntoa_t (BLUE_INADDR ip_addr) ;
  /**
   * Convert a string in dot notation to an IP address
   *
   * \param str
   * String to convert
   *
   * \param pinaddr Pointer to an IP address to store result
   * \returns Nothing
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueNETaton (BLUE_CCHAR *str, BLUE_INADDR *pinaddr) ;
#endif
  const BLUE_CHAR *
  BlueNETntop (const BLUE_IPADDR *src, BLUE_CHAR *dst, BLUE_SIZET size) ;
  BLUE_INT
  BlueNETpton (const BLUE_CHAR *src, BLUE_IPADDR *dst) ;

  /**
   * Return the count of interfaces on the platform
   *
   * \returns Count of interfaces
   */
  BLUE_CORE_LIB BLUE_INT 
  BlueNetInterfaceCount (BLUE_VOID) ;
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
  BLUE_CORE_LIB BLUE_VOID 
  BlueNetInterfaceAddr (BLUE_INT index, BLUE_IPADDR *pinaddr,
			BLUE_IPADDR *pbcast, BLUE_IPADDR *pmask) ;
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
  BLUE_CORE_LIB BLUE_VOID
  BlueNetInterfaceWins (BLUE_INT index, BLUE_INT *num_wins, 
			BLUE_IPADDR **winslist) ;
  /**
   * Test whether an ip address is on the same subnet as another address
   *
   * \param fromip IP address to test
   * \param intip IP address of other machine or subnet
   * \param mask Mask of the interface
   * \returns BLUE_TRUE if ip addresses are on same subnet
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueNetSubnetMatch (BLUE_IPADDR *fromip, BLUE_IPADDR *intip,
		      BLUE_IPADDR *mask) ;
  /**
   * Register a Config Event
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueNetRegisterConfig (BLUE_HANDLE hEvent) ;
  /**
   * Unregister a Config Event
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueNetUnregisterConfig (BLUE_HANDLE hEvent) ;
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
  BLUE_CORE_LIB BLUE_VOID 
  BlueNetResolveName (BLUE_LPCSTR name, BLUE_UINT16 *num_addrs,
		      BLUE_IPADDR *ip) ;
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
  BLUE_CORE_LIB BLUE_VOID 
  BlueNetResolveDNSName (BLUE_LPCSTR name, BLUE_UINT16 *num_addrs,
			 BLUE_IPADDR *ip) ;

  BLUE_CORE_LIB BLUE_BOOL BlueNETIsAddrAny (BLUE_IPADDR *ip) ;
  BLUE_CORE_LIB BLUE_BOOL BlueNETIsAddrLoopback (BLUE_IPADDR *ip) ;
  BLUE_CORE_LIB BLUE_BOOL BlueNETIsAddrBcast (BLUE_IPADDR *ip) ;
  BLUE_CORE_LIB BLUE_BOOL BlueNETIsAddrNone (BLUE_IPADDR *ip) ;
  BLUE_CORE_LIB BLUE_BOOL BlueNETIsAddrEqual (BLUE_IPADDR *ip1,
					      BLUE_IPADDR *ip2) ;
  BLUE_CORE_LIB BLUE_BOOL BlueNETIsAddrLinkLocal (BLUE_IPADDR *ip) ;

#if defined(__cplusplus)
}
#endif
/** \} */
#endif

