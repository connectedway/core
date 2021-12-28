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

BLUE_IN6ADDR blue_in6addr_any = BLUE_IN6ADDR_ANY_INIT ;
BLUE_IN6ADDR blue_in6addr_loopback = BLUE_IN6ADDR_LOOPBACK_INIT ;
BLUE_IN6ADDR blue_in6addr_bcast = BLUE_IN6ADDR_BCAST_INIT ;
BLUE_IN6ADDR blue_in6addr_none = BLUE_IN6ADDR_NONE_INIT ;

OFC_CORE_LIB OFC_VOID
BlueNetInit (OFC_VOID)
{
#if defined(OFC_MESSAGE_DEBUG)
  BlueMessageDebugInit() ;
#endif
  BlueNetInitImpl () ;
}

OFC_CORE_LIB OFC_VOID
BlueNetDestroy (OFC_VOID)
{
#if defined(OFC_MESSAGE_DEBUG)
  BlueMessageDebugDestroy() ;
#endif
}

OFC_CORE_LIB OFC_INT
BlueNetInterfaceCount (OFC_VOID)
{
  OFC_INT ret ;

  ret = BlueNetInterfaceCountImpl() ;
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueNetInterfaceAddr (OFC_INT index, BLUE_IPADDR *pinaddr,
                      BLUE_IPADDR *pbcast, BLUE_IPADDR *pmask)
{
  BlueNetInterfaceAddrImpl (index, pinaddr, pbcast, pmask) ;
}

OFC_CORE_LIB OFC_VOID
BlueNetInterfaceWins (OFC_INT index, OFC_INT *num_wins,
                      BLUE_IPADDR **winslist)
{
  BlueNetInterfaceWinsImpl (index, num_wins, winslist) ;
}

OFC_CORE_LIB OFC_VOID
BlueNetRegisterConfig (BLUE_HANDLE hEvent)
{
  BlueNetRegisterConfigImpl (hEvent) ;
}

OFC_CORE_LIB OFC_VOID
BlueNetUnregisterConfig (BLUE_HANDLE hEvent)
{
  BlueNetUnregisterConfigImpl (hEvent) ;
}

OFC_CORE_LIB OFC_BOOL
BlueNetSubnetMatch (BLUE_IPADDR *fromip, BLUE_IPADDR *intip, BLUE_IPADDR *mask)
{
  OFC_BOOL ret ;

  ret = OFC_TRUE ;
  if (fromip->ip_version == intip->ip_version)
    {
      if (fromip->ip_version == BLUE_FAMILY_IP)
	{
	  if ((fromip->u.ipv4.addr & mask->u.ipv4.addr) != 
	      (intip->u.ipv4.addr & mask->u.ipv4.addr))
	    ret = OFC_FALSE ;
	}
      else
	{
	  if (!BlueNETIsAddrEqual (fromip, intip))
	    ret = OFC_FALSE ;
	}
    }
  return (ret) ;
}

#if 0
BLUE_CORE_LIB BLUE_CHAR *
BlueNETntoa_s (BLUE_IPADDR *ip_addr, BLUE_CHAR *ipstr) 
{
  BLUE_UINT32 ipx ;
  BLUE_UINT32 dot[4] ;
  BLUE_INT i ;

  if (ip_addr->ip_version == BLUE_FAMILY_IP)
    {
      ipx = ip_addr->u.ipv4.addr ;
  
      for (i = 0 ; i < 4 ; i++)
	{
	  dot[i] = (ipx >> (24 - (i*8))) & 0xFF ;
	}
      BlueCsnprintf (ipstr, IPSTR_LEN, 
		     "%d.%d.%d.%d", dot[0], dot[1], dot[2], dot[3]) ;
    }
  else
    {
      /* Don't worry about squelching zeros */
      BlueCsnprintf (ipstr, IP6STR_LEN,
		     "0x%02x0x%02x:0x%02x0x%02x:0x%02x0x%02x:0x%02x0x%02x:0x%02x0x%02x:0x%02x0x%02x:0x%02x0x%02x:0x%02x0x%02x",
		     ip_addr->u.ipv6.s6_addr[0],
		     ip_addr->u.ipv6.s6_addr[1],
		     ip_addr->u.ipv6.s6_addr[2],
		     ip_addr->u.ipv6.s6_addr[3],
		     ip_addr->u.ipv6.s6_addr[4],
		     ip_addr->u.ipv6.s6_addr[5],
		     ip_addr->u.ipv6.s6_addr[6],
		     ip_addr->u.ipv6.s6_addr[7],
		     ip_addr->u.ipv6.s6_addr[8],
		     ip_addr->u.ipv6.s6_addr[9],
		     ip_addr->u.ipv6.s6_addr[10],
		     ip_addr->u.ipv6.s6_addr[11],
		     ip_addr->u.ipv6.s6_addr[12],
		     ip_addr->u.ipv6.s6_addr[13],
		     ip_addr->u.ipv6.s6_addr[14],
		     ip_addr->u.ipv6.s6_addr[15]) ;
    }
  return (ipstr) ;
}
      
BLUE_CORE_LIB BLUE_CHAR *
BlueNETntoa (BLUE_IPADDR *ip_addr) 
{
  static char ipstr[IP6STR_LEN] ;

  BlueNETntoa_s (BLUE_IPADDR *ip_addr, ipstr) 

  return (ipstr) ;
}

BLUE_CORE_LIB BLUE_CHAR *
BlueNETntoa_t (BLUE_INADDR ip_addr) 
{
  char *ipstr ;

  ipstr = BlueHeapMalloc (IP6STR_LEN+1) ;

  BlueNETntoa_s (ip_addr, ipstr) ;
  
  return (ipstr) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueNETaton (BLUE_CCHAR *str, BLUE_INADDR *ip_addr)
{
  BLUE_CCHAR *p ;
  BLUE_INT i ;
  BLUE_INT dot[4] ;
  BLUE_UINT32 ipx ;

  for (p = str, i = 0, ipx = 0 ; (*p != '\0') && (*p != ';') && (i < 4) ; i++ )
    {
      dot[i] = 0 ;
      for ( ; (*p != '\0') && (*p != '.') && (*p >= '0') && (*p <= '9') ; p++) 
	{
	  if ((*p >= '0') && (*p <= '9'))
	    dot[i] = (dot[i] * 10) + (*p - '0') ;
	}
      ipx |= dot[i] << (24 - (i*8)) ;

      if (*p == '.')
	p++ ;
    }

  ip_addr->addr = ipx ;
}
#endif

OFC_CORE_LIB OFC_VOID
BlueNetResolveName (OFC_LPCSTR name, OFC_UINT16 *num_addrs,
                    BLUE_IPADDR *ip)
{
  OFC_BOOL none ;

  none = OFC_TRUE ;
#if defined (OFC_NETBIOS)
  BlueNETpton (name, ip) ;

  /*
   * Is it still null?
   */
  if (none)
    {
      BlueNameResolveName (name, num_addrs, ip) ;
      if (!BlueNETIsAddrNone (ip))
	none = BLUE_FALSE ;
    }
#endif

  if (none)
    {
      BlueNetResolveDNSName (name, num_addrs, ip) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueNetResolveDNSName (OFC_LPCSTR name, OFC_UINT16 *num_addrs,
                       BLUE_IPADDR *ip)
{
  BlueNetResolveDNSNameImpl (name, num_addrs, ip) ;
}

OFC_CORE_LIB OFC_BOOL BlueNETIsAddrAny (BLUE_IPADDR *ip)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (ip->ip_version == BLUE_FAMILY_IP)
    {
      if (ip->u.ipv4.addr == BLUE_INADDR_ANY)
	ret = OFC_TRUE ;
    }
  else
    {
      if (BLUE_IN6_ARE_ADDR_EQUAL(ip->u.ipv6.blue_s6_addr, &blue_in6addr_any))
	ret = OFC_TRUE ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL BlueNETIsAddrLinkLocal (BLUE_IPADDR *ip)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (ip->ip_version == BLUE_FAMILY_IP)
    ret = OFC_TRUE ;
  else
    {
      if (BLUE_IN6_IS_ADDR_LINKLOCAL(ip->u.ipv6.blue_s6_addr))
	ret = OFC_TRUE ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL BlueNETIsAddrLoopback (BLUE_IPADDR *ip)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (ip->ip_version == BLUE_FAMILY_IP)
    {
      if (ip->u.ipv4.addr == BLUE_INADDR_LOOPBACK)
	ret = OFC_TRUE ;
    }
  else
    {
      if (BLUE_IN6_ARE_ADDR_EQUAL(ip->u.ipv6.blue_s6_addr, 
				  &blue_in6addr_loopback))
	ret = OFC_TRUE ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL BlueNETIsAddrBcast (BLUE_IPADDR *ip)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (ip->ip_version == BLUE_FAMILY_IP)
    {
      if (ip->u.ipv4.addr == BLUE_INADDR_BROADCAST)
	ret = OFC_TRUE ;
    }
  else
    {
      if (BLUE_IN6_ARE_ADDR_EQUAL(ip->u.ipv6.blue_s6_addr, 
				  &blue_in6addr_bcast))
	ret = OFC_TRUE ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL BlueNETIsAddrNone (BLUE_IPADDR *ip)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (ip->ip_version == BLUE_FAMILY_IP)
    {
      if (ip->u.ipv4.addr == BLUE_INADDR_NONE)
	ret = OFC_TRUE ;
    }
  else
    {
      if (BLUE_IN6_ARE_ADDR_EQUAL(ip->u.ipv6.blue_s6_addr, &blue_in6addr_none))
	ret = OFC_TRUE ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL BlueNETIsAddrEqual (BLUE_IPADDR *ip1,
                                          BLUE_IPADDR *ip2)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (ip1->ip_version == BLUE_FAMILY_IP)
    {
      if (ip1->u.ipv4.addr == ip2->u.ipv4.addr)
	ret = OFC_TRUE ;
    }
  else
    {
      if (BLUE_IN6_ARE_ADDR_EQUAL(ip1->u.ipv6.blue_s6_addr, 
				  ip2->u.ipv6.blue_s6_addr))
	ret = OFC_TRUE ;
    }
  return (ret) ;
}
