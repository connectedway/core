/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/net.h"
#include "ofc/socket.h"
#include "ofc/libc.h"
#include "ofc/impl/socketimpl.h"

#include "ofc/heap.h"

#define OFC_SCOPE_ALGORITHM

/*
* Forward Delcarations
*/
typedef struct 
{
  OFC_HANDLE impl ;
  OFC_BOOL connected ;
  BLUE_SOCKET_TYPE type ;
  BLUE_IPADDR ip ;		/* For input stream sockets */
  OFC_UINT16 port ;		/* ... */
  OFC_SIZET write_count ;
  OFC_INT write_offset ;
  OFC_HANDLE send_queue ;
} BLUE_SOCKET ;

static BLUE_SOCKET *BlueSocketAlloc (OFC_VOID) ;
static OFC_VOID BlueSocketFree (BLUE_SOCKET *sock) ;

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
BlueSocketAlloc (OFC_VOID)
{
  BLUE_SOCKET *sock ;

  sock = BlueHeapMalloc (sizeof (BLUE_SOCKET)) ;
  sock->impl = OFC_HANDLE_NULL ;
  sock->type = SOCKET_TYPE_NONE;

  return (sock) ;
}

static OFC_VOID
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
OFC_CORE_LIB OFC_VOID
BlueSocketDestroy (OFC_HANDLE hSock)
{
  BLUE_SOCKET *sock ;

  sock = ofc_handle_lock (hSock) ;
  if (sock != OFC_NULL)
    {
      BlueSocketImplClose (sock->impl) ;

      BlueSocketImplDestroy (sock->impl) ;
      BlueSocketFree (sock) ;
      ofc_handle_destroy (hSock) ;
      ofc_handle_unlock (hSock) ;
    }
}

static OFC_INT BlueIPv6GetScope (const BLUE_IPADDR *ip)
{
  OFC_INT scope ;

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

OFC_CORE_LIB OFC_VOID
BlueSocketSourceAddress (const BLUE_IPADDR *dest, 
			 BLUE_IPADDR *source)
{
  OFC_INT count ;
  OFC_INT i ;
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
	  BlueNetInterfaceAddr (i, &ifip, OFC_NULL, &ifmask) ;
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
      OFC_INT dscope ;
      OFC_INT ifscope ;
      OFC_INT sscope ;

      dscope = BlueIPv6GetScope(dest) ;
      source->u.ipv6 = blue_in6addr_any ;
      sscope = BlueIPv6GetScope (source) ;
      for (i = 0 ; i < count ; i++)
	{
	  BlueNetInterfaceAddr (i, &ifip, OFC_NULL, OFC_NULL) ;
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
OFC_CORE_LIB OFC_HANDLE
BlueSocketConnect (const BLUE_IPADDR *ip, OFC_UINT16 port)
{
  OFC_HANDLE hSocket ;
  BLUE_SOCKET *pSock ;
  BLUE_IPADDR myinaddr ;
  BLUE_IPADDR dip ;

  /*
   * Create a socket
   */
  hSocket = OFC_HANDLE_NULL ;
  pSock = BlueSocketAlloc() ;
  pSock->type = SOCKET_TYPE_STREAM;

  dip = *ip ;
  /*
   * Yes, open a stream
   */
  pSock->impl = BlueSocketImplCreate (dip.ip_version, SOCKET_TYPE_STREAM) ;
  if (pSock->impl != OFC_HANDLE_NULL)
    {
      BlueSocketImplReuseAddr (pSock->impl, OFC_TRUE) ;

      BlueSocketSourceAddress (&dip, &myinaddr) ;
      if (BlueSocketImplBind (pSock->impl, &myinaddr, 0))
	{
	  BlueSocketImplNoBlock (pSock->impl, OFC_TRUE) ;
	  if (dip.ip_version == BLUE_FAMILY_IPV6)
	    dip.u.ipv6.blue_scope = myinaddr.u.ipv6.blue_scope ;

	  if (BlueSocketImplConnect (pSock->impl, &dip, port))
	    {
	      hSocket = ofc_handle_create (OFC_HANDLE_SOCKET, pSock) ;
	    }
	}

      if (hSocket == OFC_HANDLE_NULL)
	BlueSocketImplDestroy (pSock->impl) ;
    }

  if (hSocket == OFC_HANDLE_NULL)
    BlueSocketFree (pSock) ;
    
  return (hSocket) ;
}

OFC_CORE_LIB OFC_BOOL
BlueSocketConnected (OFC_HANDLE hSocket)
{
  BLUE_SOCKET *pSocket ;
  OFC_BOOL connected ;

  connected = OFC_FALSE ;
  pSocket = ofc_handle_lock (hSocket) ;
  if (pSocket != OFC_NULL)
    {
      connected = BlueSocketImplConnected (pSocket->impl) ;
      ofc_handle_unlock (hSocket) ;
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
OFC_CORE_LIB OFC_HANDLE
BlueSocketListen (const BLUE_IPADDR *ip, OFC_UINT16 port)
{
  OFC_HANDLE hSocket ;
  BLUE_SOCKET *pSock ;

  /*
   * Create a socket
   */
  hSocket = OFC_HANDLE_NULL ;
  pSock = BlueSocketAlloc() ;
  pSock->type = SOCKET_TYPE_STREAM;

  pSock->impl = BlueSocketImplCreate (ip->ip_version, SOCKET_TYPE_STREAM) ;
  if (pSock->impl != OFC_HANDLE_NULL)
    {
      BlueSocketImplReuseAddr (pSock->impl, OFC_TRUE) ;
      if (BlueSocketImplBind (pSock->impl, ip, port))
	{
	  BlueSocketImplNoBlock (pSock->impl, OFC_TRUE) ;
	  if (BlueSocketImplListen (pSock->impl, 5))
	    {
	      hSocket = ofc_handle_create (OFC_HANDLE_SOCKET, pSock) ;
	    }
	}

      if (hSocket == OFC_HANDLE_NULL)
	BlueSocketImplDestroy (pSock->impl) ;
    }

  if (hSocket == OFC_HANDLE_NULL)
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
OFC_CORE_LIB OFC_HANDLE
BlueSocketDatagram (const BLUE_IPADDR *ip, OFC_UINT16 port)
{
  BLUE_SOCKET *sock ;
  OFC_HANDLE hSocket ;
  OFC_BOOL status ;

  /*
   * Create the socket
   */
  sock = BlueSocketAlloc() ;
  sock->type = SOCKET_TYPE_DGRAM;
  /*
   * Yes, open up a datagram socket
   */
  hSocket = OFC_HANDLE_NULL ;
  sock->impl = BlueSocketImplCreate (ip->ip_version, SOCKET_TYPE_DGRAM) ;
  /*
   * Is it open?
   */
  if (sock->impl != OFC_HANDLE_NULL)
    {
      /*
       * Yes, bind to the ip and port we want to accept datagrams on
       */
      BlueSocketImplReuseAddr (sock->impl, OFC_TRUE) ;
      status = BlueSocketImplBind (sock->impl, ip, port) ;
      if (status == OFC_TRUE)
	{
	  BlueSocketImplNoBlock (sock->impl, OFC_TRUE) ;
	  hSocket = ofc_handle_create (OFC_HANDLE_SOCKET, sock) ;
	}
      else
	BlueSocketImplDestroy (sock->impl) ;
    }

  if (hSocket == OFC_HANDLE_NULL)
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
OFC_CORE_LIB OFC_HANDLE
BlueSocketRaw (const BLUE_IPADDR *ip, BLUE_SOCKET_TYPE socktype)
{
  BLUE_SOCKET *sock ;
  OFC_HANDLE hSocket ;

  /*
   * Create the socket
   */
  hSocket = OFC_HANDLE_NULL ;

  sock = BlueSocketAlloc() ;
  sock->type = socktype ;
  /*
   * Yes, open up a datagram socket
   */
  hSocket = OFC_HANDLE_NULL ;
  sock->impl = BlueSocketImplCreate (ip->ip_version, socktype) ;
  /*
   * Is it open?
   */
  if (sock->impl != OFC_HANDLE_NULL)
    {
      BlueSocketImplNoBlock (sock->impl, OFC_TRUE) ;
      hSocket = ofc_handle_create (OFC_HANDLE_SOCKET, sock) ;
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
OFC_CORE_LIB OFC_HANDLE
BlueSocketAccept (OFC_HANDLE hMasterSocket)
{
  BLUE_SOCKET *socket ;
  BLUE_SOCKET *master_socket ;
  OFC_HANDLE hSocket ;
  
  hSocket = OFC_HANDLE_NULL ;
  master_socket = ofc_handle_lock (hMasterSocket) ;
  if (master_socket != OFC_NULL)
    {
      /*
       * Create a new socket
       */
      socket = BlueSocketAlloc () ;
      socket->type = SOCKET_TYPE_STREAM;

      socket->impl = BlueSocketImplAccept (master_socket->impl,
					   &socket->ip, &socket->port) ;

      if (socket->impl == OFC_HANDLE_NULL)
	{
	  BlueSocketFree (socket) ;
	}
      else
	{
	  BlueSocketImplNoBlock (socket->impl, OFC_TRUE) ;

	  hSocket = ofc_handle_create (OFC_HANDLE_SOCKET, socket) ;
	  if (hSocket == OFC_HANDLE_NULL)
	    BlueSocketFree (socket) ;
	}
      ofc_handle_unlock (hMasterSocket) ;
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
OFC_CORE_LIB OFC_BOOL
BlueSocketWrite (OFC_HANDLE hSocket, BLUE_MESSAGE *msg)
{
  OFC_SIZET nbytes ;
  BLUE_SOCKET *socket ;
  OFC_BOOL progress ;

  socket = ofc_handle_lock (hSocket) ;
  nbytes = -1 ;
  progress = OFC_FALSE ;
  if (socket != OFC_NULL)
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
	  progress = OFC_TRUE ;
	}
      if ((nbytes < 0) ||
	  ((socket->type == SOCKET_TYPE_DGRAM) && (nbytes ==0)))
	{
	  OFC_VOID *handle ;

	  handle = ofc_handle_lock (socket->impl) ;

	  BlueCprintf ("Socket Error on write, type %d, Impl 0x%08x, handle 0x%08x, count %d\n",
		       socket->type, socket->impl, handle, msg->count) ;
	  /*
	   * Force message to retire
	   */
	  msg->count = 0 ;

	  if (handle != OFC_NULL)
	    ofc_handle_unlock (socket->impl) ;
	}

      ofc_handle_unlock (hSocket) ;
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
OFC_CORE_LIB OFC_BOOL
BlueSocketRead (OFC_HANDLE hSocket, BLUE_MESSAGE *msg)
{
  BLUE_SOCKET *socket ;
  OFC_SIZET len ;
  OFC_BOOL progress ;

  socket = ofc_handle_lock (hSocket) ;
  progress = OFC_FALSE ;
  if (socket != OFC_HANDLE_NULL)
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
	  progress = OFC_TRUE ;
	}

      ofc_handle_unlock (hSocket) ;
    }
  return (progress) ;
}

OFC_CORE_LIB OFC_HANDLE
BlueSocketGetImpl (OFC_HANDLE hSocket)
{
  BLUE_SOCKET *sock ;
  OFC_HANDLE impl ;

  impl = OFC_HANDLE_NULL ;
  sock = ofc_handle_lock (hSocket) ;
  if (sock != OFC_NULL)
    {
      impl = sock->impl ;
      ofc_handle_unlock (hSocket) ;
    }
  return (impl) ;
}

OFC_CORE_LIB BLUE_SOCKET_EVENT_TYPE
BlueSocketTest (OFC_HANDLE hSocket)
{
  BLUE_SOCKET_EVENT_TYPE ret ;
  OFC_HANDLE impl ;
  BLUE_SOCKET *sock ;

  ret = OFC_FALSE ;
  impl = OFC_HANDLE_NULL ;
  sock = ofc_handle_lock (hSocket) ;
  if (sock != OFC_NULL)
    {
      ret = BlueSocketImplTest (sock->impl) ;
      ofc_handle_unlock (hSocket) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BlueSocketEnable (OFC_HANDLE hSocket, BLUE_SOCKET_EVENT_TYPE type)
{
  OFC_BOOL ret ;
  OFC_HANDLE impl ;
  BLUE_SOCKET *sock ;

  ret = OFC_FALSE ;
  impl = OFC_HANDLE_NULL ;
  sock = ofc_handle_lock (hSocket) ;
  if (sock != OFC_NULL)
    {
      ret = BlueSocketImplEnable (sock->impl, type) ;
      ofc_handle_unlock (hSocket) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueSocketSetSendSize (OFC_HANDLE hSocket, OFC_INT size)
{
  BLUE_SOCKET *sock ;

  sock = ofc_handle_lock (hSocket) ;
  if (sock != OFC_NULL)
    {
      BlueSocketImplSetSendSize (sock->impl, size) ;
      ofc_handle_unlock (hSocket) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueSocketSetRecvSize (OFC_HANDLE hSocket, OFC_INT size)
{
  BLUE_SOCKET *sock ;

  sock = ofc_handle_lock (hSocket) ;
  if (sock != OFC_NULL)
    {
      BlueSocketImplSetRecvSize (sock->impl, size) ;
      ofc_handle_unlock (hSocket) ;
    }
}

OFC_CORE_LIB OFC_BOOL
BlueSocketGetAddresses (OFC_HANDLE hSock, BLUE_SOCKADDR *local,
                        BLUE_SOCKADDR *remote)
{
  BLUE_SOCKET *sock ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  sock = ofc_handle_lock (hSock) ;
  if (sock != OFC_NULL)
    {
      ret = BlueSocketImplGetAddresses (sock->impl, local, remote) ;
      ofc_handle_unlock (hSock) ;
    }
  return (ret) ;
}
