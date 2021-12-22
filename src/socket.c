/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __BLUE_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/net.h"
#include "ofc/socket.h"
#include "ofc/libc.h"
#include "ofc/impl/socketimpl.h"

#include "ofc/heap.h"

#define BLUE_PARAM_SCOPE_ALGORITHM

/*
* Forward Delcarations
*/
typedef struct 
{
  BLUE_HANDLE impl ;
  BLUE_BOOL connected ;
  BLUE_SOCKET_TYPE type ;
  BLUE_IPADDR ip ;		/* For input stream sockets */
  BLUE_UINT16 port ;		/* ... */
  BLUE_SIZET write_count ;
  BLUE_INT write_offset ;
  BLUE_HANDLE send_queue ;
} BLUE_SOCKET ;

static BLUE_SOCKET *BlueSocketAlloc (BLUE_VOID) ;
static BLUE_VOID BlueSocketFree (BLUE_SOCKET *sock) ;

/*
 * SOCKET_create - Create a socket
 *
 * Accepts:
 *    Nothing
 *
 * Returns:
 *    socket
 *
 * This is an internal routine to create a socket structure
 */
static BLUE_SOCKET *
BlueSocketAlloc (BLUE_VOID)
{
  BLUE_SOCKET *sock ;

  sock = BlueHeapMalloc (sizeof (BLUE_SOCKET)) ;
  sock->impl = BLUE_HANDLE_NULL ;
  sock->type = SOCKET_TYPE_NONE;

  return (sock) ;
}

static BLUE_VOID 
BlueSocketFree (BLUE_SOCKET *sock) 
{
  BlueHeapFree (sock) ;
}

/*
 * SOCKET_destroy - destroy a socket
 *
 * Accpets:
 *    The socket to destroy
 *
 * Returns:
 *    nothing
 */
BLUE_CORE_LIB BLUE_VOID 
BlueSocketDestroy (BLUE_HANDLE hSock)
{
  BLUE_SOCKET *sock ;

  sock = BlueHandleLock (hSock) ;
  if (sock != BLUE_NULL)
    {
      BlueSocketImplClose (sock->impl) ;

      BlueSocketImplDestroy (sock->impl) ;
      BlueSocketFree (sock) ;
      BlueHandleDestroy (hSock) ;
      BlueHandleUnlock (hSock) ;
    }
}

static BLUE_INT BlueIPv6GetScope (const BLUE_IPADDR *ip)
{
  BLUE_INT scope ;

  if (BLUE_IN6_IS_ADDR_LOOPBACK (ip->u.ipv6.blue_s6_addr))
    scope = 0x01 ;
  else if ((BLUE_IN6_IS_ADDR_LINKLOCAL (ip->u.ipv6.blue_s6_addr)) ||
	   (BLUE_IN6_IS_ADDR_MC_LINKLOCAL (ip->u.ipv6.blue_s6_addr)))
    scope = 0x02 ;
  else if ((BLUE_IN6_IS_ADDR_SITELOCAL (ip->u.ipv6.blue_s6_addr)) ||
	   (BLUE_IN6_IS_ADDR_MC_SITELOCAL (ip->u.ipv6.blue_s6_addr)))
    scope = 0x05 ;
  else
    scope = 0x0e ;

  return (scope) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueSocketSourceAddress (const BLUE_IPADDR *dest, 
			 BLUE_IPADDR *source)
{
  BLUE_INT count ;
  BLUE_INT i ;
  BLUE_IPADDR ifip ;
  BLUE_IPADDR ifmask ;

  count = BlueNetInterfaceCount () ;
  /*
   * Start with a default
   */
  source->ip_version = dest->ip_version ;
  if (dest->ip_version == BLUE_FAMILY_IP)
    {
      source->u.ipv4.addr = BLUE_INADDR_ANY ;
      /* try to match by subnet */

      for (i = 0 ; i < count && source->u.ipv4.addr == BLUE_INADDR_ANY ; i++)
	{
	  BlueNetInterfaceAddr (i, &ifip, BLUE_NULL, &ifmask) ;
	  if (ifip.ip_version == BLUE_FAMILY_IP)
	    {
	      if ((ifip.u.ipv4.addr & ifmask.u.ipv4.addr) ==
		  (dest->u.ipv4.addr & ifmask.u.ipv4.addr))
		*source = ifip ;
	    }
	}
    }
  else
    {
#if defined(BLUE_SOCKET_SCOPE_ALGORITHM)
      source->u.ipv6 = blue_in6addr_any ;
      for (i = 0 ; i < count ; i++)
	{
	  BlueNetInterfaceAddr (i, &ifip, BLUE_NULL, BLUE_NULL) ;
	  /*
	   * Rule 1, prefer same address
	   */
	  if (BLUE_IN6_ARE_ADDR_EQUAL (ifip.u.ipv6.blue_s6_addr,
				       dest->u.ipv6.blue_s6_addr))
	    *source = ifip ;
	  /*
	   * Rule 2, Prefer scope
	   */
	  else if (BlueIPv6GetScope (&ifip) <= BlueIPv6GetScope (source))
	    {
	      if (BlueIPv6GetScope (&ifip) >=  BlueIPv6GetScope (dest))
		*source = ifip ;
	    }
	  else
	    {
	      if (BlueIPv6GetScope (source) <= BlueIPv6GetScope (dest))
		*source = ifip ;
	    }
	}
#else
      BLUE_INT dscope ;
      BLUE_INT ifscope ;
      BLUE_INT sscope ;

      dscope = BlueIPv6GetScope(dest) ;
      source->u.ipv6 = blue_in6addr_any ;
      sscope = BlueIPv6GetScope (source) ;
      for (i = 0 ; i < count ; i++)
	{
	  BlueNetInterfaceAddr (i, &ifip, BLUE_NULL, BLUE_NULL) ;
	  if (ifip.ip_version == BLUE_FAMILY_IPV6)
	    {
	      ifscope = BlueIPv6GetScope(&ifip) ;
	      if (dscope <= ifscope && ifscope <= sscope)
		{
		  *source = ifip ;
		  sscope = ifscope ;
		}
	    }
	}
#endif
    }
}

/*
 * SOCKET_connect - Connect to a remote socket
 *
 * Accepts:
 *    ip to connect to
 *    port to connect to
 *
 * Returns:
 *    socket structure for connection
 */
BLUE_CORE_LIB BLUE_HANDLE 
BlueSocketConnect (const BLUE_IPADDR *ip, BLUE_UINT16 port)
{
  BLUE_HANDLE hSocket ;
  BLUE_SOCKET *pSock ;
  BLUE_IPADDR myinaddr ;
  BLUE_IPADDR dip ;

  /*
   * Create a socket
   */
  hSocket = BLUE_HANDLE_NULL ;
  pSock = BlueSocketAlloc() ;
  pSock->type = SOCKET_TYPE_STREAM;

  dip = *ip ;
  /*
   * Yes, open a stream
   */
  pSock->impl = BlueSocketImplCreate (dip.ip_version, SOCKET_TYPE_STREAM) ;
  if (pSock->impl != BLUE_HANDLE_NULL)
    {
      BlueSocketImplReuseAddr (pSock->impl, BLUE_TRUE) ;

      BlueSocketSourceAddress (&dip, &myinaddr) ;
      if (BlueSocketImplBind (pSock->impl, &myinaddr, 0))
	{
	  BlueSocketImplNoBlock (pSock->impl, BLUE_TRUE) ;
	  if (dip.ip_version == BLUE_FAMILY_IPV6)
	    dip.u.ipv6.blue_scope = myinaddr.u.ipv6.blue_scope ;

	  if (BlueSocketImplConnect (pSock->impl, &dip, port))
	    {
	      hSocket = BlueHandleCreate (BLUE_HANDLE_SOCKET, pSock) ;
	    }
	}

      if (hSocket == BLUE_HANDLE_NULL)
	BlueSocketImplDestroy (pSock->impl) ;
    }

  if (hSocket == BLUE_HANDLE_NULL)
    BlueSocketFree (pSock) ;
    
  return (hSocket) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueSocketConnected (BLUE_HANDLE hSocket)
{
  BLUE_SOCKET *pSocket ;
  BLUE_BOOL connected ;

  connected = BLUE_FALSE ;
  pSocket = BlueHandleLock (hSocket) ;
  if (pSocket != BLUE_NULL)
    {
      connected = BlueSocketImplConnected (pSocket->impl) ;
      BlueHandleUnlock (hSocket) ;
    }
  return (connected) ;
}

/*
 * SOCK_listen - Open up a socket to listen on
 *
 * Accepts:
 *    interface to accept listens on
 *    port to accept listens on
 *
 * Returns:
 *    listening socket
 */
BLUE_CORE_LIB BLUE_HANDLE 
BlueSocketListen (const BLUE_IPADDR *ip, BLUE_UINT16 port)
{
  BLUE_HANDLE hSocket ;
  BLUE_SOCKET *pSock ;

  /*
   * Create a socket
   */
  hSocket = BLUE_HANDLE_NULL ;
  pSock = BlueSocketAlloc() ;
  pSock->type = SOCKET_TYPE_STREAM;

  pSock->impl = BlueSocketImplCreate (ip->ip_version, SOCKET_TYPE_STREAM) ;
  if (pSock->impl != BLUE_HANDLE_NULL)
    {
      BlueSocketImplReuseAddr (pSock->impl, BLUE_TRUE) ;
      if (BlueSocketImplBind (pSock->impl, ip, port))
	{
	  BlueSocketImplNoBlock (pSock->impl, BLUE_TRUE) ;
	  if (BlueSocketImplListen (pSock->impl, 5))
	    {
	      hSocket = BlueHandleCreate (BLUE_HANDLE_SOCKET, pSock) ;
	    }
	}

      if (hSocket == BLUE_HANDLE_NULL)
	BlueSocketImplDestroy (pSock->impl) ;
    }

  if (hSocket == BLUE_HANDLE_NULL)
    BlueSocketFree (pSock) ;

  return (hSocket) ;
}


/*
* SOCK_datagram - Open up a udp socket to listen on
*
* Accepts:
*    interface to accept datagrams on
*    port to accept datagrams on
*
* Returns:
*    datagram socket
*/
BLUE_CORE_LIB BLUE_HANDLE 
BlueSocketDatagram (const BLUE_IPADDR *ip, BLUE_UINT16 port)
{
  BLUE_SOCKET *sock ;
  BLUE_HANDLE hSocket ;
  BLUE_BOOL status ;

  /*
   * Create the socket
   */
  sock = BlueSocketAlloc() ;
  sock->type = SOCKET_TYPE_DGRAM;
  /*
   * Yes, open up a datagram socket
   */
  hSocket = BLUE_HANDLE_NULL ;
  sock->impl = BlueSocketImplCreate (ip->ip_version, SOCKET_TYPE_DGRAM) ;
  /*
   * Is it open?
   */
  if (sock->impl != BLUE_HANDLE_NULL)
    {
      /*
       * Yes, bind to the ip and port we want to accept datagrams on
       */
      BlueSocketImplReuseAddr (sock->impl, BLUE_TRUE) ;
      status = BlueSocketImplBind (sock->impl, ip, port) ;
      if (status == BLUE_TRUE)
	{
	  BlueSocketImplNoBlock (sock->impl, BLUE_TRUE) ;
	  hSocket = BlueHandleCreate (BLUE_HANDLE_SOCKET, sock) ;
	}
      else
	BlueSocketImplDestroy (sock->impl) ;
    }

  if (hSocket == BLUE_HANDLE_NULL)
    BlueSocketFree (sock) ;

  return (hSocket) ;
}


/*
* SOCK_datagram - Open up a udp socket to listen on
*
* Accepts:
*    interface to accept datagrams on
*    port to accept datagrams on
*
* Returns:
*    datagram socket
*/
BLUE_CORE_LIB BLUE_HANDLE 
BlueSocketRaw (const BLUE_IPADDR *ip, BLUE_SOCKET_TYPE socktype)
{
  BLUE_SOCKET *sock ;
  BLUE_HANDLE hSocket ;

  /*
   * Create the socket
   */
  hSocket = BLUE_HANDLE_NULL ;

  sock = BlueSocketAlloc() ;
  sock->type = socktype ;
  /*
   * Yes, open up a datagram socket
   */
  hSocket = BLUE_HANDLE_NULL ;
  sock->impl = BlueSocketImplCreate (ip->ip_version, socktype) ;
  /*
   * Is it open?
   */
  if (sock->impl != BLUE_HANDLE_NULL)
    {
      BlueSocketImplNoBlock (sock->impl, BLUE_TRUE) ;
      hSocket = BlueHandleCreate (BLUE_HANDLE_SOCKET, sock) ;
    }
  else 
    BlueSocketFree (sock) ;

  return (hSocket) ;
}

/*
 * SOCKET_accept - Accept an incoming connection on a listening socket
 *
 * Accepts:
 *    master socket to accept listen from
 *
 * Returns:
 *    socket structure
 */
BLUE_CORE_LIB BLUE_HANDLE 
BlueSocketAccept (BLUE_HANDLE hMasterSocket)
{
  BLUE_SOCKET *socket ;
  BLUE_SOCKET *master_socket ;
  BLUE_HANDLE hSocket ;
  
  hSocket = BLUE_HANDLE_NULL ;
  master_socket = BlueHandleLock (hMasterSocket) ;
  if (master_socket != BLUE_NULL)
    {
      /*
       * Create a new socket
       */
      socket = BlueSocketAlloc () ;
      socket->type = SOCKET_TYPE_STREAM;

      socket->impl = BlueSocketImplAccept (master_socket->impl,
					   &socket->ip, &socket->port) ;

      if (socket->impl == BLUE_HANDLE_NULL)
	{
	  BlueSocketFree (socket) ;
	}
      else
	{
	  BlueSocketImplNoBlock (socket->impl, BLUE_TRUE) ;

	  hSocket = BlueHandleCreate (BLUE_HANDLE_SOCKET, socket) ;
	  if (hSocket == BLUE_HANDLE_NULL)
	    BlueSocketFree (socket) ;
	}
      BlueHandleUnlock (hMasterSocket) ;
    }

  return (hSocket) ;
}


/*
 * SOCKET_write - Write data to a stream socket
 *
 * Accepts:
 *    socket to write to
 *    buffer to write
 *    where in buffer to start writing from
 *    Number of bytes to write from buffer
 *
 * Returns:
 *   True if we wrote something, false otherwise
 */
BLUE_CORE_LIB BLUE_BOOL 
BlueSocketWrite (BLUE_HANDLE hSocket, BLUE_MESSAGE *msg)
{
  BLUE_SIZET nbytes ;
  BLUE_SOCKET *socket ;
  BLUE_BOOL progress ;

  socket = BlueHandleLock (hSocket) ;
  nbytes = -1 ;
  progress = BLUE_FALSE ;
  if (socket != BLUE_NULL)
    {
      /*
       * Do we have bytes to send?
       */
      if (msg->count > 0)
	{
	  /*
	   * Yes, so try to send it
	   */
	  if (socket->type == SOCKET_TYPE_STREAM)
	    {
	      nbytes = BlueSocketImplSend (socket->impl, 
					   msg->msg + msg->offset,
					   msg->count) ;
	    }
	  else if (socket->type == SOCKET_TYPE_DGRAM ||
		   socket->type == SOCKET_TYPE_ICMP)
	    {
	      nbytes = BlueSocketImplSendTo (socket->impl,
					     msg->msg + msg->offset,
					     msg->count,
					     &msg->ip,
					     msg->port) ;
	    }
	}
      else
	nbytes = 0 ;

      if (nbytes > 0)
	{
	  if (nbytes > msg->count)
	    {
	      BlueCprintf ("nbytes %d is greater then count %d\n",
			   nbytes, msg->count) ;
	      nbytes = msg->count ;
	    }
	  msg->count -= nbytes ;
	  msg->offset += nbytes ;
	  progress = BLUE_TRUE ;
	}
      if ((nbytes < 0) ||
	  ((socket->type == SOCKET_TYPE_DGRAM) && (nbytes ==0)))
	{
	  BLUE_VOID *handle ;

	  handle = BlueHandleLock (socket->impl) ;

	  BlueCprintf ("Socket Error on write, type %d, Impl 0x%08x, handle 0x%08x, count %d\n",
		       socket->type, socket->impl, handle, msg->count) ;
	  /*
	   * Force message to retire
	   */
	  msg->count = 0 ;

	  if (handle != BLUE_NULL)
	    BlueHandleUnlock (socket->impl) ;
	}

      BlueHandleUnlock (hSocket) ;
    }
  return (progress) ;
}

/*
 * SOCKET_read - Read data from a socket
 *
 * Accepts:
 *    socket to read from
 *    buffer to read into
 *    offset within buffer to read to
 *    number of bytes to read
 *
 */
BLUE_CORE_LIB BLUE_BOOL 
BlueSocketRead (BLUE_HANDLE hSocket, BLUE_MESSAGE *msg)
{
  BLUE_SOCKET *socket ;
  BLUE_SIZET len ;
  BLUE_BOOL progress ;

  socket = BlueHandleLock (hSocket) ;
  progress = BLUE_FALSE ;
  if (socket != BLUE_HANDLE_NULL)
    {
      len = 0 ;
      if (msg->count > 0)
	{
	  /*
	   * Yes, so try to receive it.
	   */
	  if (socket->type == SOCKET_TYPE_STREAM)
	    {
	      len = BlueSocketImplRecv (socket->impl,
					msg->msg + msg->offset,
					msg->count) ;
	      msg->ip = socket->ip;
	      msg->port = socket->port;
	    }
	  else if (socket->type == SOCKET_TYPE_DGRAM ||
		   socket->type == SOCKET_TYPE_ICMP)
	    {
	      len = BlueSocketImplRecvFrom (socket->impl,
					    msg->msg + msg->offset,
					    msg->count,
					    &msg->ip, &msg->port) ;
	    }
	}

      if (len > 0)
	{
	  msg->offset += len ;
	  msg->count -= len ;
	  progress = BLUE_TRUE ;
	}

      BlueHandleUnlock (hSocket) ;
    }
  return (progress) ;
}

BLUE_CORE_LIB BLUE_HANDLE 
BlueSocketGetImpl (BLUE_HANDLE hSocket)
{
  BLUE_SOCKET *sock ;
  BLUE_HANDLE impl ;

  impl = BLUE_HANDLE_NULL ;
  sock = BlueHandleLock (hSocket) ;
  if (sock != BLUE_NULL)
    {
      impl = sock->impl ;
      BlueHandleUnlock (hSocket) ;
    }
  return (impl) ;
}

BLUE_CORE_LIB BLUE_SOCKET_EVENT_TYPE 
BlueSocketTest (BLUE_HANDLE hSocket)
{
  BLUE_SOCKET_EVENT_TYPE ret ;
  BLUE_HANDLE impl ;
  BLUE_SOCKET *sock ;

  ret = BLUE_FALSE ;
  impl = BLUE_HANDLE_NULL ;
  sock = BlueHandleLock (hSocket) ;
  if (sock != BLUE_NULL)
    {
      ret = BlueSocketImplTest (sock->impl) ;
      BlueHandleUnlock (hSocket) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueSocketEnable (BLUE_HANDLE hSocket, BLUE_SOCKET_EVENT_TYPE type) 
{
  BLUE_BOOL ret ;
  BLUE_HANDLE impl ;
  BLUE_SOCKET *sock ;

  ret = BLUE_FALSE ;
  impl = BLUE_HANDLE_NULL ;
  sock = BlueHandleLock (hSocket) ;
  if (sock != BLUE_NULL)
    {
      ret = BlueSocketImplEnable (sock->impl, type) ;
      BlueHandleUnlock (hSocket) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueSocketSetSendSize (BLUE_HANDLE hSocket, BLUE_INT size)
{
  BLUE_SOCKET *sock ;

  sock = BlueHandleLock (hSocket) ;
  if (sock != BLUE_NULL)
    {
      BlueSocketImplSetSendSize (sock->impl, size) ;
      BlueHandleUnlock (hSocket) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueSocketSetRecvSize (BLUE_HANDLE hSocket, BLUE_INT size)
{
  BLUE_SOCKET *sock ;

  sock = BlueHandleLock (hSocket) ;
  if (sock != BLUE_NULL)
    {
      BlueSocketImplSetRecvSize (sock->impl, size) ;
      BlueHandleUnlock (hSocket) ;
    }
}

BLUE_CORE_LIB BLUE_BOOL 
BlueSocketGetAddresses (BLUE_HANDLE hSock, BLUE_SOCKADDR *local, 
			BLUE_SOCKADDR *remote)
{
  BLUE_SOCKET *sock ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  sock = BlueHandleLock (hSock) ;
  if (sock != BLUE_NULL)
    {
      ret = BlueSocketImplGetAddresses (sock->impl, local, remote) ;
      BlueHandleUnlock (hSock) ;
    }
  return (ret) ;
}
