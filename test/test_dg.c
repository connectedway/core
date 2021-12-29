/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#include "unity.h"
#include "unity_fixture.h"

#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/timer.h"
#include "ofc/event.h"
#include "ofc/libc.h"
#include "ofc/message.h"
#include "ofc/net.h"
#include "ofc/socket.h"
#include "ofc/sched.h"
#include "ofc/app.h"
#include "ofc/heap.h"
#include "ofc/queue.h"
#include "ofc/core.h"
#include "ofc/framework.h"
#include "ofc/env.h"
#include "ofc/persist.h"

static OFC_HANDLE hScheduler;
static OFC_HANDLE hDone;

static OFC_INT
test_startup_persist(OFC_VOID)
{
  OFC_INT ret = 1;
  OFC_TCHAR path[OFC_MAX_PATH] ;

  if (ofc_env_get (OFC_ENV_HOME, path, OFC_MAX_PATH) == OFC_TRUE)
    {
      ofc_framework_load (path);
      ret = 0;
    }
  return (ret);
}

static OFC_INT test_startup_default(OFC_VOID)
{
  static OFC_UUID uuid =
    {
     0x04, 0x5d, 0x88, 0x8a, 0xeb, 0x1c, 0xc9, 0x11,
     0x9f, 0xe8, 0x08, 0x00, 0x2b, 0x10, 0x48, 0x60
    } ;

  ofc_persist_default();
  ofc_persist_set_interface_type(OFC_CONFIG_ICONFIG_AUTO);
  ofc_persist_set_node_name(TSTR("localhost"), TSTR("WORKGROUP"),
                            TSTR("OpenFiles Unit Test"));
  ofc_persist_set_uuid(&uuid);
  return(0);
}

static OFC_INT test_startup(OFC_VOID)
{
  OFC_INT ret;
  ofc_framework_init();
#if defined(OFC_PERSIST)
  ret = test_startup_persist();
#else
  ret = test_startup_default();
#endif
  hScheduler = ofc_sched_create();
  hDone = ofc_event_create(OFC_EVENT_AUTO);

  return(ret);
}

static OFC_VOID test_shutdown(OFC_VOID)
{
  ofc_event_destroy(hDone);
  ofc_sched_quit(hScheduler);
  ofc_framework_shutdown();
  ofc_framework_destroy();
}

typedef enum
  {
    DGRAM_TEST_SERVER_STATE_IDLE,
    DGRAM_TEST_SERVER_STATE_PRIMING,
    DGRAM_TEST_SERVER_STATE_BODY
  } DGRAM_TEST_SERVER_STATE ;

#define DGRAM_TEST_SERVER_INTERVAL 1000
#define DGRAM_TEST_PORT 7543

typedef struct
{
  DGRAM_TEST_SERVER_STATE state ;
  OFC_IPADDR ip ;
  OFC_IPADDR bcast ;
  OFC_IPADDR mask ;
  OFC_HANDLE hTimer ;
  OFC_HANDLE scheduler ;
  OFC_HANDLE hSocket ;
  OFC_MESSAGE *recv_msg ;
} OFC_DGRAM_TEST_SERVER ;

static OFC_VOID DGramTestServerPreSelect (OFC_HANDLE app) ;
static OFC_HANDLE DGramTestServerPostSelect (OFC_HANDLE app,
                                             OFC_HANDLE hSocket) ;
static OFC_VOID DGramTestServerDestroy (OFC_HANDLE app) ;
#if defined(OFC_APP_DEBUG)
static OFC_VOID DGramTestServerDump (OFC_HANDLE app) ;
#endif

static OFC_APP_TEMPLATE DGramTestServerAppDef =
  {
          "Datagram Test Server Application",
          &DGramTestServerPreSelect,
          &DGramTestServerPostSelect,
          &DGramTestServerDestroy,
#if defined(OFC_APP_DEBUG)
    &DGramTestServerDump
#else
          OFC_NULL
#endif
  } ;

#if defined(OFC_APP_DEBUG)
OFC_VOID DGramTestServerDump (OFC_HANDLE app)
{
  OFC_DGRAM_TEST_SERVER *DGramTestServer ;
  OFC_CHAR ip_addr[IP6STR_LEN] ;

  DGramTestServer = ofc_app_get_data (app) ;
  if (DGramTestServer != OFC_NULL)
    {
      ofc_printf ("%-20s : %d\n", "DGram Server State", 
		  DGramTestServer->state) ;
      ofc_printf ("%-20s : %s\n", "IP Address", 
		   ofc_ntop (&DGramTestServer->ip, ip_addr, IP6STR_LEN)) ;
      ofc_printf ("\n") ;
    }
}
#endif

static OFC_VOID DGramTestServerPreSelect (OFC_HANDLE app)
{
  OFC_DGRAM_TEST_SERVER *DGramTestServer ;
  OFC_SOCKET_EVENT_TYPE event_types ;
  DGRAM_TEST_SERVER_STATE entry_state ;

  DGramTestServer = ofc_app_get_data (app) ;
  if (DGramTestServer != OFC_NULL)
    {
      do /* while the state is different */
	{
	  entry_state = DGramTestServer->state ;
	  ofc_sched_clear_wait (DGramTestServer->scheduler, app) ;
	  
	  switch (DGramTestServer->state)
	    {
	    default:
	    case DGRAM_TEST_SERVER_STATE_IDLE:
	      DGramTestServer->hTimer = ofc_timer_create("DG SRV") ;
	      if (DGramTestServer->hTimer != OFC_HANDLE_NULL)
		{
		  ofc_timer_set (DGramTestServer->hTimer,
                         DGRAM_TEST_SERVER_INTERVAL) ;
		  DGramTestServer->recv_msg = OFC_NULL ;
		  DGramTestServer->state = DGRAM_TEST_SERVER_STATE_PRIMING ;

		  ofc_sched_add_wait (DGramTestServer->scheduler, app,
                              DGramTestServer->hTimer) ;
		}
	      else
		ofc_app_kill (app) ;

	      break ;

	    case DGRAM_TEST_SERVER_STATE_PRIMING:
	      ofc_sched_add_wait (DGramTestServer->scheduler, app,
                              DGramTestServer->hTimer) ;
	      break ;

	    case DGRAM_TEST_SERVER_STATE_BODY:
	      event_types = OFC_SOCKET_EVENT_READ ;
	      ofc_socket_enable (DGramTestServer->hSocket, event_types) ;

	      ofc_sched_add_wait (DGramTestServer->scheduler, app,
                              DGramTestServer->hSocket) ;
	      break ;
	    }
	}
      while (DGramTestServer->state != entry_state) ;
    }
}

static OFC_BOOL ServiceRead (OFC_DGRAM_TEST_SERVER *DGramTestServer)
{
  OFC_BOOL progress ;
  OFC_IPADDR ip ;
  OFC_UINT16 port ;
  OFC_CHAR packet_ip[IP6STR_LEN] ;
  OFC_CHAR interface_ip[IP6STR_LEN] ;

  progress = 
    ofc_socket_read (DGramTestServer->hSocket, DGramTestServer->recv_msg) ;
	      
  if (ofc_message_offset(DGramTestServer->recv_msg) != 0)
    {
      ofc_message_addr (DGramTestServer->recv_msg, &ip, &port) ;
      ofc_ntop (&ip, packet_ip, IP6STR_LEN) ;
      ofc_ntop (&DGramTestServer->ip, interface_ip, IP6STR_LEN) ;
      ofc_printf ("Read %d Bytes in Message from %s:%d on interface %s\n",
                  ofc_message_offset(DGramTestServer->recv_msg),
                  packet_ip, port, interface_ip) ;
      ofc_printf ("%s\n", ofc_message_data (DGramTestServer->recv_msg)) ;
      ofc_message_destroy (DGramTestServer->recv_msg) ;
      DGramTestServer->recv_msg = 
	ofc_message_create (MSG_ALLOC_HEAP, 1000, OFC_NULL) ;
    }
  return (progress);
}


static OFC_HANDLE DGramTestServerPostSelect (OFC_HANDLE app,
                                             OFC_HANDLE hSocket)
{
  OFC_DGRAM_TEST_SERVER *DGramTestServer ;
  OFC_BOOL progress ;
  OFC_CHAR ip_addr[IP6STR_LEN] ;

  DGramTestServer = ofc_app_get_data (app) ;
  if (DGramTestServer != OFC_NULL)
    {
      for (progress = OFC_TRUE ; progress && !ofc_app_destroying(app);)
	{
	  progress = OFC_FALSE ;

	  switch (DGramTestServer->state)
	    {
	    default:
	    case DGRAM_TEST_SERVER_STATE_IDLE:
	      break ;
	  
	    case DGRAM_TEST_SERVER_STATE_PRIMING:
	      if (hSocket == DGramTestServer->hTimer)
		{
		  if (DGramTestServer->recv_msg == OFC_NULL)
		    {
		      DGramTestServer->hSocket = 
			ofc_socket_datagram (&DGramTestServer->ip,
                                 DGRAM_TEST_PORT) ;
		      if (DGramTestServer->hSocket == OFC_HANDLE_NULL)
			{
			  ofc_app_kill (app) ;
			}
		      else
			{
			  ofc_printf ("Listening for Datagrams on Interface "
				       "on %s\n",
                          ofc_ntop (&DGramTestServer->ip,
                                    ip_addr, IP6STR_LEN)) ;
			  DGramTestServer->recv_msg = 
			    ofc_message_create (MSG_ALLOC_HEAP, 1000,
                                    OFC_NULL) ;
			}
		    }
		  DGramTestServer->state = DGRAM_TEST_SERVER_STATE_BODY ;
		}
	      break ;

	    case DGRAM_TEST_SERVER_STATE_BODY:
	      if (hSocket == DGramTestServer->hSocket)
		{
		  progress |= ServiceRead (DGramTestServer) ;
		}
	      break ;
	    }
	}
    }
  return (OFC_HANDLE_NULL) ;
}

static OFC_VOID DGramTestServerDestroy (OFC_HANDLE app)
{
  OFC_DGRAM_TEST_SERVER *DGramTestServer ;

  ofc_printf ("Destroying Datagram Test Server Application\n") ;
  DGramTestServer = ofc_app_get_data (app) ;
  if (DGramTestServer != OFC_NULL)
    {
      switch (DGramTestServer->state)
	{
	default:
	case DGRAM_TEST_SERVER_STATE_IDLE:
	  break ;

	case DGRAM_TEST_SERVER_STATE_PRIMING:
	  ofc_timer_destroy (DGramTestServer->hTimer) ;
	  break ;

	case DGRAM_TEST_SERVER_STATE_BODY:
	  ofc_timer_destroy (DGramTestServer->hTimer) ;
	  if (DGramTestServer->hSocket != OFC_HANDLE_NULL)
	    ofc_socket_destroy (DGramTestServer->hSocket) ;
	  break ;
	}
      if (DGramTestServer->recv_msg != OFC_NULL)
	ofc_message_destroy (DGramTestServer->recv_msg) ;
      ofc_free (DGramTestServer) ;
    }
}

typedef enum
  {
    DGRAM_TEST_CLIENT_STATE_IDLE,
    DGRAM_TEST_CLIENT_STATE_CONNECTED,
    DGRAM_TEST_CLIENT_STATE_DESTROYING,
  } DGRAM_TEST_CLIENT_STATE ;

#define DGRAM_TEST_CLIENT_INTERVAL 1000
#define DGRAM_TEST_COUNT 10

#define OFFSET_CLIENT_MSG_DATA 0
#define CLIENT_MSG_DATA "This is my test message"

typedef struct
{
  OFC_FAMILY_TYPE family ;
  DGRAM_TEST_CLIENT_STATE state ;
  OFC_HANDLE hSocket ;
  OFC_HANDLE scheduler ;
  OFC_MESSAGE *send_msg ;
  OFC_HANDLE hTimer ;
  OFC_INT count ;
  OFC_HANDLE hAppQueue ;
  OFC_HANDLE hDestroy;
  OFC_HANDLE hListenApp ;
  OFC_HANDLE hCurrApp ;
  OFC_HANDLE hConfigUpdate ;
} OFC_DGRAM_TEST_CLIENT ;

static OFC_VOID DGramTestClientPreSelect (OFC_HANDLE app) ;
static OFC_HANDLE DGramTestClientPostSelect (OFC_HANDLE app,
                                             OFC_HANDLE hSocket) ;
static OFC_VOID DGramTestClientDestroy (OFC_HANDLE app) ;
#if defined(OFC_APP_DEBUG)
static OFC_VOID DGramTestClientDump (OFC_HANDLE app) ;
#endif

static OFC_APP_TEMPLATE DGramTestClientAppDef =
  {
          "Datagram Test Client Application",
          &DGramTestClientPreSelect,
          &DGramTestClientPostSelect,
          &DGramTestClientDestroy,
#if defined(OFC_APP_DEBUG)
    &DGramTestClientDump
#else
          OFC_NULL
#endif
  } ;

#if defined(OFC_APP_DEBUG)
OFC_VOID DGramTestClientDump (OFC_HANDLE app)
{
  OFC_DGRAM_TEST_CLIENT *DGramTestClient ;

  DGramTestClient = ofc_app_get_data (app) ;
  if (DGramTestClient != OFC_NULL)
    {
      ofc_printf ("%-20s : %d\n", "DGram Client State", 
		   DGramTestClient->state) ;
      ofc_printf ("\n") ;
    }
}
#endif

static OFC_VOID DGramTestClientPreSelect (OFC_HANDLE app)
{
  OFC_DGRAM_TEST_CLIENT *DGramTestClient ;
  OFC_SOCKET_EVENT_TYPE event_types ;
#if !defined(OFC_MULTI_UDP)
  OFC_DGRAM_TEST_SERVER *DGramTestServer ;
  OFC_CHAR ip_addr[IP6STR_LEN] ;
#endif
  OFC_IPADDR ipaddr ;
  DGRAM_TEST_CLIENT_STATE entry_state ;

  DGramTestClient = ofc_app_get_data (app) ;
  if (DGramTestClient != OFC_NULL)
    {
      do /* while entry state != DGramTestClient->state */
	{
	  entry_state = DGramTestClient->state ;
	  ofc_sched_clear_wait (DGramTestClient->scheduler, app) ;

	  switch (DGramTestClient->state)
	    {
	    default:
	      break ;

	    case DGRAM_TEST_CLIENT_STATE_IDLE:
	      /*
	       * First we want to register for configuration events
	       */
	      DGramTestClient->hDestroy = ofc_event_create(OFC_EVENT_AUTO);
	      DGramTestClient->hConfigUpdate = ofc_event_create(OFC_EVENT_AUTO) ;
	      if (DGramTestClient->hConfigUpdate != OFC_HANDLE_NULL)
		{
		  ofc_persist_register_update (DGramTestClient->hConfigUpdate) ;
		}

	      DGramTestClient->hCurrApp = 
		(OFC_HANDLE) ofc_queue_first (DGramTestClient->hAppQueue) ;
#if !defined(OFC_MULTI_UDP)
	      /*
	       * On some systems, we need to listen on the any address
	       */
	      DGramTestServer = ofc_malloc (sizeof (OFC_DGRAM_TEST_SERVER)) ;
	      if (DGramTestClient->family == OFC_FAMILY_IP)
		{
		  DGramTestServer->ip.ip_version = OFC_FAMILY_IP ;
		  DGramTestServer->ip.u.ipv4.addr = OFC_INADDR_ANY ;
		  DGramTestServer->bcast.ip_version = OFC_FAMILY_IP ;
		  DGramTestServer->bcast.u.ipv4.addr = OFC_INADDR_BROADCAST ;
		  DGramTestServer->mask.ip_version = OFC_FAMILY_IP ;
		  DGramTestServer->mask.u.ipv4.addr = OFC_INADDR_ANY ;
		}
#if defined(OFC_DISCOVER_IPV6)
	      else
		{
		  DGramTestServer->ip.ip_version = OFC_FAMILY_IPV6 ;
		  DGramTestServer->ip.u.ipv6 = ofc_in6addr_any ;
		  DGramTestServer->bcast.ip_version = OFC_FAMILY_IPV6 ;
		  DGramTestServer->bcast.u.ipv6 = ofc_in6addr_bcast ;
		  DGramTestServer->mask.ip_version = OFC_FAMILY_IPV6 ;
		  DGramTestServer->mask.u.ipv6 = ofc_in6addr_any ;
		}
#endif
	      DGramTestServer->state = DGRAM_TEST_SERVER_STATE_IDLE ;
	      DGramTestServer->scheduler = DGramTestClient->scheduler ;

	      ofc_printf ("Creating Datagram Test Server Application on %s\n",
                      ofc_ntop (&DGramTestServer->ip,
                                ip_addr, IP6STR_LEN)) ;

	      DGramTestClient->hListenApp = 
		ofc_app_create (DGramTestServer->scheduler,
                        &DGramTestServerAppDef, DGramTestServer) ;
#endif

	      DGramTestClient->send_msg = OFC_NULL ;
	      if (DGramTestClient->family == OFC_FAMILY_IP)
		{
		  ipaddr.ip_version = OFC_FAMILY_IP ;
		  ipaddr.u.ipv4.addr = OFC_INADDR_ANY ;
		}
#if defined(OFC_DISCOVER_IPV6)
	      else
		{
		  ipaddr.ip_version = OFC_FAMILY_IPV6 ;
		  ipaddr.u.ipv6 = ofc_in6addr_any ;
		}
#endif
	      DGramTestClient->hSocket = 
		ofc_socket_datagram (&ipaddr, 0) ;

	      if (DGramTestClient->hSocket != OFC_HANDLE_NULL)
		{
		  DGramTestClient->hTimer = ofc_timer_create("DG CLIENT") ;
		  if (DGramTestClient->hTimer != OFC_HANDLE_NULL)
		    {
		      ofc_timer_set (DGramTestClient->hTimer,
                             DGRAM_TEST_CLIENT_INTERVAL) ;

		      DGramTestClient->state = DGRAM_TEST_CLIENT_STATE_CONNECTED ;

		      event_types = OFC_SOCKET_EVENT_CLOSE ;
		      ofc_socket_enable (DGramTestClient->hSocket, event_types) ;

		      ofc_sched_add_wait (DGramTestClient->scheduler, app,
                                  DGramTestClient->hSocket) ;

		      ofc_sched_add_wait (DGramTestClient->scheduler, app,
                                  DGramTestClient->hTimer) ;
		      ofc_sched_add_wait (DGramTestClient->scheduler, app,
                                  DGramTestClient->hConfigUpdate) ;
		      ofc_sched_add_wait (DGramTestClient->scheduler, app,
                                  DGramTestClient->hDestroy) ;
		    }
		  else
		    ofc_app_kill(app) ;
		}
	      else
		ofc_app_kill (app) ;

	      break ;

	    case DGRAM_TEST_CLIENT_STATE_CONNECTED:
	      event_types = OFC_SOCKET_EVENT_CLOSE ;
	      if (DGramTestClient->send_msg != OFC_NULL)
		event_types |= OFC_SOCKET_EVENT_WRITE ;
	      ofc_socket_enable (DGramTestClient->hSocket, event_types) ;
	      ofc_sched_add_wait (DGramTestClient->scheduler, app,
                              DGramTestClient->hSocket) ;
	      ofc_sched_add_wait (DGramTestClient->scheduler, app,
                              DGramTestClient->hTimer) ;
	      ofc_sched_add_wait (DGramTestClient->scheduler, app,
                              DGramTestClient->hConfigUpdate) ;
	      ofc_sched_add_wait(DGramTestClient->scheduler, app,
                             DGramTestClient->hDestroy);
	      break ;

	    case DGRAM_TEST_CLIENT_STATE_DESTROYING:
	      ofc_sched_add_wait(DGramTestClient->scheduler, app,
                             DGramTestClient->hDestroy);
	      break;
	    }
	}
      while (DGramTestClient->state != entry_state) ;
    }
}

static OFC_VOID DGramTestReconfig (OFC_DGRAM_TEST_CLIENT *DGramTestClient)
{
  int i ;
  OFC_IPADDR ip ;
  OFC_IPADDR mask ;
  OFC_IPADDR bcast ;
  OFC_HANDLE next_app ;
  OFC_HANDLE hServerApp ;
  OFC_BOOL found ;
  OFC_DGRAM_TEST_SERVER *DGramTestServer ;
  OFC_CHAR ip_addr[IP6STR_LEN] ;

  for (hServerApp = (OFC_HANDLE) ofc_queue_first (DGramTestClient->hAppQueue) ;
       hServerApp != OFC_HANDLE_NULL ;
       hServerApp = next_app)
    {
      next_app = (OFC_HANDLE) ofc_queue_next (DGramTestClient->hAppQueue,
                                              (OFC_VOID *) hServerApp) ;
      DGramTestServer = ofc_app_get_data (hServerApp) ;
      found = OFC_FALSE ;

      for (i = 0 ; i < ofc_persist_interface_count() && !found ;)
	{
	  ofc_persist_interface_addr (i, &ip, &bcast, &mask) ;
	  if (ofc_net_is_addr_equal (&DGramTestServer->ip, &ip) &&
		  ofc_net_is_addr_equal (&DGramTestServer->bcast, &bcast) &&
		  ofc_net_is_addr_equal (&DGramTestServer->mask, &mask))
	    found = OFC_TRUE ;
	  else
	    i++ ;
	}
      if (!found)
	{
	  if (DGramTestClient->hCurrApp == hServerApp)
	    DGramTestClient->hCurrApp = next_app ;
	  ofc_queue_unlink (DGramTestClient->hAppQueue, (OFC_VOID *) hServerApp) ;
	  ofc_app_kill (hServerApp) ;
	}
    }

  /*
   * Now look for added interfaces
   */
  for (i = 0 ; i < ofc_persist_interface_count() ; i++)
    {
      ofc_persist_interface_addr (i, &ip, &bcast, &mask) ;

      found = OFC_FALSE ;
      for (hServerApp = (OFC_HANDLE) ofc_queue_first (DGramTestClient->hAppQueue) ;
           hServerApp != OFC_HANDLE_NULL && !found ;)
	{
	  DGramTestServer = ofc_app_get_data (hServerApp) ;
	  if (ofc_net_is_addr_equal (&DGramTestServer->ip, &ip) &&
		  ofc_net_is_addr_equal (&DGramTestServer->bcast, &bcast) &&
		  ofc_net_is_addr_equal (&DGramTestServer->mask, &mask))
	    found = OFC_TRUE ;
	  else
	    hServerApp = (OFC_HANDLE) ofc_queue_next (DGramTestClient->hAppQueue,
                                                  (OFC_VOID *) hServerApp) ;
	}

      if (!found && (!ofc_net_is_addr_link_local (&ip)))
	/*
	 * Only configure link local interfaces for dg test
	 */
	found = OFC_TRUE ;
      /*
       * If our device does not loopback broadcast datagrams to the
       * sending interface, then don't configure server's on anything
       * but the loopback interface 
       */
#if defined(OFC_LOOPBACK)
      if (!found && (!ofc_net_is_addr_loopback(&ip)))
	/*
	 * Say found, so we don't configure this interface.  It's not the
	 * loopback one
	 */
	found = OFC_TRUE ;
#endif

      if (!found && (DGramTestClient->family == ip.ip_version))
	{
	  DGramTestServer = ofc_malloc (sizeof (OFC_DGRAM_TEST_SERVER)) ;
	  if (DGramTestServer != OFC_NULL)
	    {
	      DGramTestServer->ip = ip ;
	      DGramTestServer->bcast = bcast ;
	      DGramTestServer->mask = mask ;
	      DGramTestServer->state = DGRAM_TEST_SERVER_STATE_IDLE ;
	      DGramTestServer->scheduler = DGramTestClient->scheduler ;

	      ofc_printf ("Creating Datagram Test Server Application on %s\n",
                      ofc_ntop (&DGramTestServer->ip,
                                ip_addr, IP6STR_LEN)) ;

	      hServerApp = ofc_app_create (DGramTestServer->scheduler,
                                       &DGramTestServerAppDef,
                                       DGramTestServer) ;
	      if (hServerApp != OFC_HANDLE_NULL)
		ofc_enqueue (DGramTestClient->hAppQueue,
                     (OFC_VOID *) hServerApp) ;
	      else
		ofc_free (DGramTestServer) ;
	    }
	}
    }
}

static OFC_HANDLE DGramTestClientPostSelect (OFC_HANDLE app,
                                             OFC_HANDLE hSocket)
{
  OFC_DGRAM_TEST_CLIENT *DGramTestClient ;
  OFC_DGRAM_TEST_SERVER *DGramTestServer ;
  OFC_CHAR *buffer ;
  OFC_BOOL progress ;
  OFC_CHAR packet_ip[IP6STR_LEN] ;
  OFC_CHAR interface_ip[IP6STR_LEN] ;

  DGramTestClient = ofc_app_get_data (app) ;
  if (DGramTestClient != OFC_NULL)
    {
      for (progress = OFC_TRUE ; progress && !ofc_app_destroying(app);)
	{
	  progress = OFC_FALSE ;
	  switch (DGramTestClient->state)
	    {
	    default:
	    case DGRAM_TEST_CLIENT_STATE_IDLE:
	      break ;
	  
	    case DGRAM_TEST_CLIENT_STATE_CONNECTED:
	      if (hSocket == DGramTestClient->hTimer)
		{
		  if (DGramTestClient->send_msg == OFC_NULL)
		    {
		      /*
		       * We will service the server queue round robin
		       */
		      DGramTestClient->hCurrApp = (OFC_HANDLE)
			ofc_queue_next (DGramTestClient->hAppQueue,
                            (OFC_VOID *) DGramTestClient->hCurrApp) ;
		      if (DGramTestClient->hCurrApp == OFC_HANDLE_NULL)
			DGramTestClient->hCurrApp = (OFC_HANDLE)
			  ofc_queue_first (DGramTestClient->hAppQueue) ;

		      if (DGramTestClient->hCurrApp == OFC_HANDLE_NULL)
			ofc_printf ("No %s Targets to Send message To\n",
                        DGramTestClient->family ==
                        OFC_FAMILY_IP ? "IPv4" : "IPv6") ;
		      else
			{
			  DGramTestServer = 
			    ofc_app_get_data (DGramTestClient->hCurrApp) ;
			  DGramTestClient->send_msg = 
			    ofc_datagram_create (MSG_ALLOC_HEAP,
                                    ofc_strlen (CLIENT_MSG_DATA) +
                                    1,
                                     OFC_NULL,
                                     &DGramTestServer->bcast,
                                     DGRAM_TEST_PORT) ;
			  buffer = ofc_message_data(DGramTestClient->send_msg) ;
			  ofc_strncpy (&buffer[OFFSET_CLIENT_MSG_DATA],
                           CLIENT_MSG_DATA,
                            ofc_strlen (CLIENT_MSG_DATA) + 1) ;
			  ofc_socket_write (DGramTestClient->hSocket,
                                DGramTestClient->send_msg) ;
			  if (ofc_message_done (DGramTestClient->send_msg))
			    {
			      ofc_ntop (&DGramTestServer->bcast,
                            packet_ip, IP6STR_LEN) ;
			      ofc_ntop (&DGramTestServer->ip,
                            interface_ip, IP6STR_LEN) ;
			      ofc_printf
				("Wrote Message to %s on interface %s\n",
				 packet_ip, interface_ip) ;
			      ofc_message_destroy (DGramTestClient->send_msg) ;
			      DGramTestClient->send_msg = OFC_NULL ;
			    }
			}
		    }
		  ofc_timer_set (DGramTestClient->hTimer,
                         DGRAM_TEST_CLIENT_INTERVAL) ;
		  DGramTestClient->count++ ;
		  if (DGramTestClient->count >= DGRAM_TEST_COUNT)
		    ofc_event_set(DGramTestClient->hDestroy);
		}
	      else if (hSocket == DGramTestClient->hSocket)
		{
		  if (DGramTestClient->send_msg != OFC_NULL)
		    {
		      progress |= ofc_socket_write (DGramTestClient->hSocket,
                                            DGramTestClient->send_msg) ;
		      if (ofc_message_done (DGramTestClient->send_msg))
			{
			  ofc_message_destroy (DGramTestClient->send_msg) ;
			  DGramTestClient->send_msg = OFC_NULL ;
			}
		    }
		}
	      else if (hSocket == DGramTestClient->hConfigUpdate)
		{
		  DGramTestReconfig (DGramTestClient) ;
		}
	      else if (hSocket == DGramTestClient->hDestroy)
		{
		  OFC_HANDLE hDestroyApp;
		  DGramTestClient->state = DGRAM_TEST_CLIENT_STATE_DESTROYING;
		  hDestroyApp =
		    (OFC_HANDLE) ofc_dequeue (DGramTestClient->hAppQueue);
		  if (hDestroyApp == OFC_HANDLE_NULL)
		    {
		      hDestroyApp = DGramTestClient->hListenApp;
		      DGramTestClient->hListenApp = OFC_HANDLE_NULL;
		    }
		  if (hDestroyApp != OFC_HANDLE_NULL)
		    {
		      ofc_app_set_wait (hDestroyApp, DGramTestClient->hDestroy) ;
		      ofc_app_kill (hDestroyApp);
		    }
		  else
		    {
		      ofc_app_kill(app);
		    }
		}
	      else
		{
		  ofc_app_kill (app);
		}
	      break;

	    case DGRAM_TEST_CLIENT_STATE_DESTROYING:
	      if (hSocket == DGramTestClient->hDestroy)
		{
		  OFC_HANDLE hDestroyApp;
		  hDestroyApp =
		    (OFC_HANDLE) ofc_dequeue (DGramTestClient->hAppQueue);
		  if (hDestroyApp == OFC_HANDLE_NULL)
		    {
		      hDestroyApp = DGramTestClient->hListenApp;
		      DGramTestClient->hListenApp = OFC_HANDLE_NULL;
		    }
		  if (hDestroyApp != OFC_HANDLE_NULL)
		    {
		      ofc_app_set_wait (hDestroyApp, DGramTestClient->hDestroy) ;
		      ofc_app_kill (hDestroyApp);
		    }
		  else
		    {
		      ofc_app_kill(app);
		    }
		}
	      else
		{
		  ofc_app_kill(app);
		}
	      break ;
	    }
	}
    }
  return (OFC_HANDLE_NULL) ;
}

static OFC_VOID DGramTestClientDestroy (OFC_HANDLE app)
{
  OFC_DGRAM_TEST_CLIENT *DGramTestClient ;
  OFC_HANDLE hAppServer ;

  ofc_printf ("Destroying DGram Test Client Application\n") ;
  DGramTestClient = ofc_app_get_data (app) ;
  if (DGramTestClient != OFC_NULL)
    {
      switch (DGramTestClient->state)
	{
	default:
	case DGRAM_TEST_CLIENT_STATE_IDLE:
	  break ;

	case DGRAM_TEST_CLIENT_STATE_DESTROYING:
	case DGRAM_TEST_CLIENT_STATE_CONNECTED:
	  ofc_socket_destroy (DGramTestClient->hSocket) ;
	  if (DGramTestClient->send_msg != OFC_NULL)
	    ofc_message_destroy (DGramTestClient->send_msg) ;
	  ofc_timer_destroy (DGramTestClient->hTimer) ;
	  ofc_persist_unregister_update (DGramTestClient->hConfigUpdate) ;
	  ofc_event_destroy (DGramTestClient->hConfigUpdate) ;
	  ofc_event_destroy (DGramTestClient->hDestroy);
	  break ;
	}

      for (hAppServer = 
	     (OFC_HANDLE) ofc_dequeue (DGramTestClient->hAppQueue) ;
           hAppServer != OFC_HANDLE_NULL ;
	   hAppServer = (OFC_HANDLE)
	     ofc_dequeue (DGramTestClient->hAppQueue))
	{
	  ofc_printf ("App Server still queued at destroy");
	  ofc_app_kill (hAppServer) ;
	}
      ofc_queue_destroy (DGramTestClient->hAppQueue) ;

      if (DGramTestClient->hListenApp != OFC_HANDLE_NULL)
	{
	  ofc_printf ("Listener still active at destroy");
	  ofc_app_kill (DGramTestClient->hListenApp) ;
	}
      ofc_free (DGramTestClient) ;
    }
}

TEST_GROUP(dg);

TEST_SETUP(dg)
{
  TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(dg)
{
  test_shutdown();
}  

TEST(dg, test_dg)
{
  OFC_DGRAM_TEST_CLIENT *DGramTestClient ;
  OFC_HANDLE hApp ;
  /*
   * Let's create a client
   */
  DGramTestClient = ofc_malloc (sizeof (OFC_DGRAM_TEST_CLIENT)) ;
  DGramTestClient->family = OFC_FAMILY_IP ;
  DGramTestClient->hAppQueue = ofc_queue_create () ;
  DGramTestClient->hListenApp = OFC_HANDLE_NULL ;
  DGramTestClient->count = 0 ;
  DGramTestClient->state = DGRAM_TEST_CLIENT_STATE_IDLE ;
  DGramTestClient->scheduler = hScheduler ;

  ofc_printf ("Creating Datagram IPV4 Client Application\n") ;
  hApp = ofc_app_create (hScheduler, &DGramTestClientAppDef, DGramTestClient) ;

  if (hDone != OFC_HANDLE_NULL)
    {
      ofc_app_set_wait (hApp, hDone) ;
      ofc_event_wait(hDone);
    }

#if defined (OFC_DISCOVER_IPV6)
  DGramTestClient = ofc_malloc (sizeof (OFC_DGRAM_TEST_CLIENT)) ;
  DGramTestClient->family = OFC_FAMILY_IPV6 ;
  DGramTestClient->hAppQueue = ofc_queue_create () ;
  DGramTestClient->hListenApp = OFC_HANDLE_NULL ;
  DGramTestClient->count = 0 ;
  DGramTestClient->state = DGRAM_TEST_CLIENT_STATE_IDLE ;
  DGramTestClient->scheduler = hScheduler ;

  ofc_printf ("Creating Datagram IPV6 Client Application\n") ;
  hApp = ofc_app_create (hScheduler, &DGramTestClientAppDef, DGramTestClient) ;
#endif

  if (hDone != OFC_HANDLE_NULL)
    {
      ofc_app_set_wait (hApp, hDone) ;
      ofc_event_wait(hDone);
    }
}	  

TEST_GROUP_RUNNER(dg)
{
  RUN_TEST_CASE(dg, test_dg);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(dg);
}

int main(int argc, const char *argv[])
{
  return UnityMain(argc, argv, runAllTests);
}
#endif
