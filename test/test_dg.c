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

  BlueConfigDefault();
  BlueConfigSetInterfaceType(BLUE_CONFIG_ICONFIG_AUTO);
  BlueConfigSetNodeName(TSTR("localhost"), TSTR("WORKGROUP"),
			TSTR("OpenFiles Unit Test"));
  BlueConfigSetUUID(&uuid);
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
  hScheduler = BlueSchedCreate();
  hDone = ofc_event_create(OFC_EVENT_AUTO);

  return(ret);
}

static OFC_VOID test_shutdown(OFC_VOID)
{
  ofc_event_destroy(hDone);
  BlueSchedQuit(hScheduler);
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
  BLUE_IPADDR ip ;
  BLUE_IPADDR bcast ;
  BLUE_IPADDR mask ;
  OFC_HANDLE hTimer ;
  OFC_HANDLE scheduler ;
  OFC_HANDLE hSocket ;
  BLUE_MESSAGE *recv_msg ;
} BLUE_DGRAM_TEST_SERVER ;

static OFC_VOID DGramTestServerPreSelect (OFC_HANDLE app) ;
static OFC_HANDLE DGramTestServerPostSelect (OFC_HANDLE app,
                                             OFC_HANDLE hSocket) ;
static OFC_VOID DGramTestServerDestroy (OFC_HANDLE app) ;
#if defined(OFC_APP_DEBUG)
static BLUE_VOID DGramTestServerDump (BLUE_HANDLE app) ;
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
BLUE_VOID DGramTestServerDump (BLUE_HANDLE app)
{
  BLUE_DGRAM_TEST_SERVER *DGramTestServer ;
  BLUE_CHAR ip_addr[IP6STR_LEN] ;

  DGramTestServer = BlueAppGetData (app) ;
  if (DGramTestServer != BLUE_NULL)
    {
      BlueCprintf ("%-20s : %d\n", "DGram Server State", 
		   DGramTestServer->state) ;
      BlueCprintf ("%-20s : %s\n", "IP Address", 
		   BlueNETntop (&DGramTestServer->ip, ip_addr, IP6STR_LEN)) ;
      BlueCprintf ("\n") ;
    }
}
#endif

static OFC_VOID DGramTestServerPreSelect (OFC_HANDLE app)
{
  BLUE_DGRAM_TEST_SERVER *DGramTestServer ;
  BLUE_SOCKET_EVENT_TYPE event_types ;
  DGRAM_TEST_SERVER_STATE entry_state ;

  DGramTestServer = ofc_app_get_data (app) ;
  if (DGramTestServer != OFC_NULL)
    {
      do /* while the state is different */
	{
	  entry_state = DGramTestServer->state ;
	  BlueSchedClearWait (DGramTestServer->scheduler, app) ;
	  
	  switch (DGramTestServer->state)
	    {
	    default:
	    case DGRAM_TEST_SERVER_STATE_IDLE:
	      DGramTestServer->hTimer = BlueTimerCreate("DG SRV") ;
	      if (DGramTestServer->hTimer != OFC_HANDLE_NULL)
		{
		  BlueTimerSet (DGramTestServer->hTimer, 
				DGRAM_TEST_SERVER_INTERVAL) ;
		  DGramTestServer->recv_msg = OFC_NULL ;
		  DGramTestServer->state = DGRAM_TEST_SERVER_STATE_PRIMING ;

		  BlueSchedAddWait (DGramTestServer->scheduler, app, 
				    DGramTestServer->hTimer) ;
		}
	      else
		ofc_app_kill (app) ;

	      break ;

	    case DGRAM_TEST_SERVER_STATE_PRIMING:
	      BlueSchedAddWait (DGramTestServer->scheduler, app, 
				DGramTestServer->hTimer) ;
	      break ;

	    case DGRAM_TEST_SERVER_STATE_BODY:
	      event_types = BLUE_SOCKET_EVENT_READ ;
	      BlueSocketEnable (DGramTestServer->hSocket, event_types) ;

	      BlueSchedAddWait (DGramTestServer->scheduler, app, 
				DGramTestServer->hSocket) ;
	      break ;
	    }
	}
      while (DGramTestServer->state != entry_state) ;
    }
}

static OFC_BOOL ServiceRead (BLUE_DGRAM_TEST_SERVER *DGramTestServer)
{
  OFC_BOOL progress ;
  BLUE_IPADDR ip ;
  OFC_UINT16 port ;
  OFC_CHAR packet_ip[IP6STR_LEN] ;
  OFC_CHAR interface_ip[IP6STR_LEN] ;

  progress = 
    BlueSocketRead (DGramTestServer->hSocket, DGramTestServer->recv_msg) ;
	      
  if (BlueMessageOffset(DGramTestServer->recv_msg) != 0)
    {
      BlueMessageAddr (DGramTestServer->recv_msg, &ip, &port) ;
      BlueNETntop (&ip, packet_ip, IP6STR_LEN) ;
      BlueNETntop (&DGramTestServer->ip, interface_ip, IP6STR_LEN) ;
      BlueCprintf ("Read %d Bytes in Message from %s:%d on interface %s\n",
		   BlueMessageOffset(DGramTestServer->recv_msg),
		   packet_ip, port, interface_ip) ;
      BlueCprintf ("%s\n", BlueMessageData (DGramTestServer->recv_msg)) ;
      BlueMessageDestroy (DGramTestServer->recv_msg) ;
      DGramTestServer->recv_msg = 
	BlueMessageCreate (MSG_ALLOC_HEAP, 1000, OFC_NULL) ;
    }
  return (progress);
}


static OFC_HANDLE DGramTestServerPostSelect (OFC_HANDLE app,
                                             OFC_HANDLE hSocket)
{
  BLUE_DGRAM_TEST_SERVER *DGramTestServer ;
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
			BlueSocketDatagram (&DGramTestServer->ip, 
					    DGRAM_TEST_PORT) ;
		      if (DGramTestServer->hSocket == OFC_HANDLE_NULL)
			{
			  ofc_app_kill (app) ;
			}
		      else
			{
			  BlueCprintf ("Listening for Datagrams on Interface "
				       "on %s\n", 
				       BlueNETntop (&DGramTestServer->ip,
						    ip_addr, IP6STR_LEN)) ;
			  DGramTestServer->recv_msg = 
			    BlueMessageCreate (MSG_ALLOC_HEAP, 1000,
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
  BLUE_DGRAM_TEST_SERVER *DGramTestServer ;

  BlueCprintf ("Destroying Datagram Test Server Application\n") ;
  DGramTestServer = ofc_app_get_data (app) ;
  if (DGramTestServer != OFC_NULL)
    {
      switch (DGramTestServer->state)
	{
	default:
	case DGRAM_TEST_SERVER_STATE_IDLE:
	  break ;

	case DGRAM_TEST_SERVER_STATE_PRIMING:
	  BlueTimerDestroy (DGramTestServer->hTimer) ;
	  break ;

	case DGRAM_TEST_SERVER_STATE_BODY:
	  BlueTimerDestroy (DGramTestServer->hTimer) ;
	  if (DGramTestServer->hSocket != OFC_HANDLE_NULL)
	    BlueSocketDestroy (DGramTestServer->hSocket) ;
	  break ;
	}
      if (DGramTestServer->recv_msg != OFC_NULL)
	BlueMessageDestroy (DGramTestServer->recv_msg) ;
      BlueHeapFree (DGramTestServer) ;
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
  BLUE_FAMILY_TYPE family ;
  DGRAM_TEST_CLIENT_STATE state ;
  OFC_HANDLE hSocket ;
  OFC_HANDLE scheduler ;
  BLUE_MESSAGE *send_msg ;
  OFC_HANDLE hTimer ;
  OFC_INT count ;
  OFC_HANDLE hAppQueue ;
  OFC_HANDLE hDestroy;
  OFC_HANDLE hListenApp ;
  OFC_HANDLE hCurrApp ;
  OFC_HANDLE hConfigUpdate ;
} BLUE_DGRAM_TEST_CLIENT ;

static OFC_VOID DGramTestClientPreSelect (OFC_HANDLE app) ;
static OFC_HANDLE DGramTestClientPostSelect (OFC_HANDLE app,
                                             OFC_HANDLE hSocket) ;
static OFC_VOID DGramTestClientDestroy (OFC_HANDLE app) ;
#if defined(OFC_APP_DEBUG)
static BLUE_VOID DGramTestClientDump (BLUE_HANDLE app) ;
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
BLUE_VOID DGramTestClientDump (BLUE_HANDLE app)
{
  BLUE_DGRAM_TEST_CLIENT *DGramTestClient ;

  DGramTestClient = BlueAppGetData (app) ;
  if (DGramTestClient != BLUE_NULL)
    {
      BlueCprintf ("%-20s : %d\n", "DGram Client State", 
		   DGramTestClient->state) ;
      BlueCprintf ("\n") ;
    }
}
#endif

static OFC_VOID DGramTestClientPreSelect (OFC_HANDLE app)
{
  BLUE_DGRAM_TEST_CLIENT *DGramTestClient ;
  BLUE_SOCKET_EVENT_TYPE event_types ;
#if !defined(OFC_MULTI_UDP)
  BLUE_DGRAM_TEST_SERVER *DGramTestServer ;
  OFC_CHAR ip_addr[IP6STR_LEN] ;
#endif
  BLUE_IPADDR ipaddr ;
  DGRAM_TEST_CLIENT_STATE entry_state ;

  DGramTestClient = ofc_app_get_data (app) ;
  if (DGramTestClient != OFC_NULL)
    {
      do /* while entry state != DGramTestClient->state */
	{
	  entry_state = DGramTestClient->state ;
	  BlueSchedClearWait (DGramTestClient->scheduler, app) ;

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
		  BlueConfigRegisterUpdate (DGramTestClient->hConfigUpdate) ;
		}

	      DGramTestClient->hCurrApp = 
		(OFC_HANDLE) BlueQfirst (DGramTestClient->hAppQueue) ;
#if !defined(OFC_MULTI_UDP)
	      /*
	       * On some systems, we need to listen on the any address
	       */
	      DGramTestServer = BlueHeapMalloc (sizeof (BLUE_DGRAM_TEST_SERVER)) ;
	      if (DGramTestClient->family == BLUE_FAMILY_IP)
		{
		  DGramTestServer->ip.ip_version = BLUE_FAMILY_IP ;
		  DGramTestServer->ip.u.ipv4.addr = BLUE_INADDR_ANY ;
		  DGramTestServer->bcast.ip_version = BLUE_FAMILY_IP ;
		  DGramTestServer->bcast.u.ipv4.addr = BLUE_INADDR_BROADCAST ;
		  DGramTestServer->mask.ip_version = BLUE_FAMILY_IP ;
		  DGramTestServer->mask.u.ipv4.addr = BLUE_INADDR_ANY ;
		}
#if defined(OFC_DISCOVER_IPV6)
	      else
		{
		  DGramTestServer->ip.ip_version = BLUE_FAMILY_IPV6 ;
		  DGramTestServer->ip.u.ipv6 = blue_in6addr_any ;
		  DGramTestServer->bcast.ip_version = BLUE_FAMILY_IPV6 ;
		  DGramTestServer->bcast.u.ipv6 = blue_in6addr_bcast ;
		  DGramTestServer->mask.ip_version = BLUE_FAMILY_IPV6 ;
		  DGramTestServer->mask.u.ipv6 = blue_in6addr_any ;
		}
#endif
	      DGramTestServer->state = DGRAM_TEST_SERVER_STATE_IDLE ;
	      DGramTestServer->scheduler = DGramTestClient->scheduler ;

	      BlueCprintf ("Creating Datagram Test Server Application on %s\n",
			   BlueNETntop (&DGramTestServer->ip,
					ip_addr, IP6STR_LEN)) ;

	      DGramTestClient->hListenApp = 
		ofc_app_create (DGramTestServer->scheduler,
                        &DGramTestServerAppDef, DGramTestServer) ;
#endif

	      DGramTestClient->send_msg = OFC_NULL ;
	      if (DGramTestClient->family == BLUE_FAMILY_IP)
		{
		  ipaddr.ip_version = BLUE_FAMILY_IP ;
		  ipaddr.u.ipv4.addr = BLUE_INADDR_ANY ;
		}
#if defined(OFC_DISCOVER_IPV6)
	      else
		{
		  ipaddr.ip_version = BLUE_FAMILY_IPV6 ;
		  ipaddr.u.ipv6 = blue_in6addr_any ;
		}
#endif
	      DGramTestClient->hSocket = 
		BlueSocketDatagram (&ipaddr, 0) ;

	      if (DGramTestClient->hSocket != OFC_HANDLE_NULL)
		{
		  DGramTestClient->hTimer = BlueTimerCreate("DG CLIENT") ;
		  if (DGramTestClient->hTimer != OFC_HANDLE_NULL)
		    {
		      BlueTimerSet (DGramTestClient->hTimer, 
				    DGRAM_TEST_CLIENT_INTERVAL) ;

		      DGramTestClient->state = DGRAM_TEST_CLIENT_STATE_CONNECTED ;

		      event_types = BLUE_SOCKET_EVENT_CLOSE ;
		      BlueSocketEnable (DGramTestClient->hSocket, event_types) ;

		      BlueSchedAddWait (DGramTestClient->scheduler, app, 
					DGramTestClient->hSocket) ;

		      BlueSchedAddWait (DGramTestClient->scheduler, app, 
					DGramTestClient->hTimer) ;
		      BlueSchedAddWait (DGramTestClient->scheduler, app, 
					DGramTestClient->hConfigUpdate) ;
		      BlueSchedAddWait (DGramTestClient->scheduler, app, 
					DGramTestClient->hDestroy) ;
		    }
		  else
		    ofc_app_kill(app) ;
		}
	      else
		ofc_app_kill (app) ;

	      break ;

	    case DGRAM_TEST_CLIENT_STATE_CONNECTED:
	      event_types = BLUE_SOCKET_EVENT_CLOSE ;
	      if (DGramTestClient->send_msg != OFC_NULL)
		event_types |= BLUE_SOCKET_EVENT_WRITE ;
	      BlueSocketEnable (DGramTestClient->hSocket, event_types) ;
	      BlueSchedAddWait (DGramTestClient->scheduler, app, 
				DGramTestClient->hSocket) ;
	      BlueSchedAddWait (DGramTestClient->scheduler, app, 
				DGramTestClient->hTimer) ;
	      BlueSchedAddWait (DGramTestClient->scheduler, app, 
				DGramTestClient->hConfigUpdate) ;
	      BlueSchedAddWait(DGramTestClient->scheduler, app,
			       DGramTestClient->hDestroy);
	      break ;

	    case DGRAM_TEST_CLIENT_STATE_DESTROYING:
	      BlueSchedAddWait(DGramTestClient->scheduler, app,
			       DGramTestClient->hDestroy);
	      break;
	    }
	}
      while (DGramTestClient->state != entry_state) ;
    }
}

static OFC_VOID DGramTestReconfig (BLUE_DGRAM_TEST_CLIENT *DGramTestClient)
{
  int i ;
  BLUE_IPADDR ip ;
  BLUE_IPADDR mask ;
  BLUE_IPADDR bcast ;
  OFC_HANDLE next_app ;
  OFC_HANDLE hServerApp ;
  OFC_BOOL found ;
  BLUE_DGRAM_TEST_SERVER *DGramTestServer ;
  OFC_CHAR ip_addr[IP6STR_LEN] ;

  for (hServerApp = (OFC_HANDLE) BlueQfirst (DGramTestClient->hAppQueue) ;
       hServerApp != OFC_HANDLE_NULL ;
       hServerApp = next_app)
    {
      next_app = (OFC_HANDLE) BlueQnext (DGramTestClient->hAppQueue,
                                         (OFC_VOID *) hServerApp) ;
      DGramTestServer = ofc_app_get_data (hServerApp) ;
      found = OFC_FALSE ;

      for (i = 0 ; i < BlueConfigInterfaceCount() && !found ;)
	{
	  BlueConfigInterfaceAddr (i, &ip, &bcast, &mask) ;
	  if (BlueNETIsAddrEqual (&DGramTestServer->ip, &ip) &&
	      BlueNETIsAddrEqual (&DGramTestServer->bcast, &bcast) &&
	      BlueNETIsAddrEqual (&DGramTestServer->mask, &mask))
	    found = OFC_TRUE ;
	  else
	    i++ ;
	}
      if (!found)
	{
	  if (DGramTestClient->hCurrApp == hServerApp)
	    DGramTestClient->hCurrApp = next_app ;
	  BlueQunlink (DGramTestClient->hAppQueue, (OFC_VOID *) hServerApp) ;
	  ofc_app_kill (hServerApp) ;
	}
    }

  /*
   * Now look for added interfaces
   */
  for (i = 0 ; i < BlueConfigInterfaceCount() ; i++)
    {
      BlueConfigInterfaceAddr (i, &ip, &bcast, &mask) ;

      found = OFC_FALSE ;
      for (hServerApp = (OFC_HANDLE) BlueQfirst (DGramTestClient->hAppQueue) ;
           hServerApp != OFC_HANDLE_NULL && !found ;)
	{
	  DGramTestServer = ofc_app_get_data (hServerApp) ;
	  if (BlueNETIsAddrEqual (&DGramTestServer->ip, &ip) &&
	      BlueNETIsAddrEqual (&DGramTestServer->bcast, &bcast) &&
	      BlueNETIsAddrEqual (&DGramTestServer->mask, &mask))
	    found = OFC_TRUE ;
	  else
	    hServerApp = (OFC_HANDLE) BlueQnext (DGramTestClient->hAppQueue,
                                             (OFC_VOID *) hServerApp) ;
	}

      if (!found && (!BlueNETIsAddrLinkLocal (&ip)))
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
      if (!found && (!BlueNETIsAddrLoopback(&ip)))
	/*
	 * Say found, so we don't configure this interface.  It's not the
	 * loopback one
	 */
	found = BLUE_TRUE ;
#endif

      if (!found && (DGramTestClient->family == ip.ip_version))
	{
	  DGramTestServer = BlueHeapMalloc (sizeof (BLUE_DGRAM_TEST_SERVER)) ;
	  if (DGramTestServer != OFC_NULL)
	    {
	      DGramTestServer->ip = ip ;
	      DGramTestServer->bcast = bcast ;
	      DGramTestServer->mask = mask ;
	      DGramTestServer->state = DGRAM_TEST_SERVER_STATE_IDLE ;
	      DGramTestServer->scheduler = DGramTestClient->scheduler ;

	      BlueCprintf ("Creating Datagram Test Server Application on %s\n",
			   BlueNETntop (&DGramTestServer->ip,
					ip_addr, IP6STR_LEN)) ;

	      hServerApp = ofc_app_create (DGramTestServer->scheduler,
                                       &DGramTestServerAppDef,
                                       DGramTestServer) ;
	      if (hServerApp != OFC_HANDLE_NULL)
		BlueQenqueue (DGramTestClient->hAppQueue, 
			      (OFC_VOID *) hServerApp) ;
	      else
		BlueHeapFree (DGramTestServer) ;
	    }
	}
    }
}

static OFC_HANDLE DGramTestClientPostSelect (OFC_HANDLE app,
                                             OFC_HANDLE hSocket)
{
  BLUE_DGRAM_TEST_CLIENT *DGramTestClient ;
  BLUE_DGRAM_TEST_SERVER *DGramTestServer ;
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
			BlueQnext (DGramTestClient->hAppQueue, 
				   (OFC_VOID *) DGramTestClient->hCurrApp) ;
		      if (DGramTestClient->hCurrApp == OFC_HANDLE_NULL)
			DGramTestClient->hCurrApp = (OFC_HANDLE)
			  BlueQfirst (DGramTestClient->hAppQueue) ;

		      if (DGramTestClient->hCurrApp == OFC_HANDLE_NULL)
			BlueCprintf ("No %s Targets to Send message To\n",
				     DGramTestClient->family ==
				     BLUE_FAMILY_IP ? "IPv4" : "IPv6") ;
		      else
			{
			  DGramTestServer = 
			    ofc_app_get_data (DGramTestClient->hCurrApp) ;
			  DGramTestClient->send_msg = 
			    BlueDatagramCreate (MSG_ALLOC_HEAP,
						BlueCstrlen (CLIENT_MSG_DATA) + 
						1,
                                    OFC_NULL,
                                    &DGramTestServer->bcast,
                                    DGRAM_TEST_PORT) ;
			  buffer = BlueMessageData(DGramTestClient->send_msg) ;
			  BlueCstrncpy (&buffer[OFFSET_CLIENT_MSG_DATA],
					CLIENT_MSG_DATA, 
					BlueCstrlen (CLIENT_MSG_DATA) + 1) ;
			  BlueSocketWrite (DGramTestClient->hSocket, 
					   DGramTestClient->send_msg) ;
			  if (BlueMessageDone (DGramTestClient->send_msg))
			    {
			      BlueNETntop (&DGramTestServer->bcast,
					   packet_ip, IP6STR_LEN) ;
			      BlueNETntop (&DGramTestServer->ip,
					   interface_ip, IP6STR_LEN) ;
			      BlueCprintf 
				("Wrote Message to %s on interface %s\n",
				 packet_ip, interface_ip) ;
			      BlueMessageDestroy (DGramTestClient->send_msg) ;
			      DGramTestClient->send_msg = OFC_NULL ;
			    }
			}
		    }
		  BlueTimerSet (DGramTestClient->hTimer, 
				DGRAM_TEST_CLIENT_INTERVAL) ;
		  DGramTestClient->count++ ;
		  if (DGramTestClient->count >= DGRAM_TEST_COUNT)
		    ofc_event_set(DGramTestClient->hDestroy);
		}
	      else if (hSocket == DGramTestClient->hSocket)
		{
		  if (DGramTestClient->send_msg != OFC_NULL)
		    {
		      progress |= BlueSocketWrite (DGramTestClient->hSocket, 
						   DGramTestClient->send_msg) ;
		      if (BlueMessageDone (DGramTestClient->send_msg))
			{
			  BlueMessageDestroy (DGramTestClient->send_msg) ;
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
		    (OFC_HANDLE) BlueQdequeue (DGramTestClient->hAppQueue);
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
		    (OFC_HANDLE) BlueQdequeue (DGramTestClient->hAppQueue);
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
  BLUE_DGRAM_TEST_CLIENT *DGramTestClient ;
  OFC_HANDLE hAppServer ;

  BlueCprintf ("Destroying DGram Test Client Application\n") ;
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
	  BlueSocketDestroy (DGramTestClient->hSocket) ;
	  if (DGramTestClient->send_msg != OFC_NULL)
	    BlueMessageDestroy (DGramTestClient->send_msg) ;
	  BlueTimerDestroy (DGramTestClient->hTimer) ;
	  BlueConfigUnregisterUpdate (DGramTestClient->hConfigUpdate) ;
	  ofc_event_destroy (DGramTestClient->hConfigUpdate) ;
	  ofc_event_destroy (DGramTestClient->hDestroy);
	  break ;
	}

      for (hAppServer = 
	     (OFC_HANDLE) BlueQdequeue (DGramTestClient->hAppQueue) ;
           hAppServer != OFC_HANDLE_NULL ;
	   hAppServer = (OFC_HANDLE)
	     BlueQdequeue (DGramTestClient->hAppQueue))
	{
	  BlueCprintf ("App Server still queued at destroy");
	  ofc_app_kill (hAppServer) ;
	}
      BlueQdestroy (DGramTestClient->hAppQueue) ;

      if (DGramTestClient->hListenApp != OFC_HANDLE_NULL)
	{
	  BlueCprintf ("Listener still active at destroy");
	  ofc_app_kill (DGramTestClient->hListenApp) ;
	}
      BlueHeapFree (DGramTestClient) ;
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
  BLUE_DGRAM_TEST_CLIENT *DGramTestClient ;
  OFC_HANDLE hApp ;
  /*
   * Let's create a client
   */
  DGramTestClient = BlueHeapMalloc (sizeof (BLUE_DGRAM_TEST_CLIENT)) ;
  DGramTestClient->family = BLUE_FAMILY_IP ;
  DGramTestClient->hAppQueue = BlueQcreate () ;
  DGramTestClient->hListenApp = OFC_HANDLE_NULL ;
  DGramTestClient->count = 0 ;
  DGramTestClient->state = DGRAM_TEST_CLIENT_STATE_IDLE ;
  DGramTestClient->scheduler = hScheduler ;

  BlueCprintf ("Creating Datagram IPV4 Client Application\n") ; 
  hApp = ofc_app_create (hScheduler, &DGramTestClientAppDef, DGramTestClient) ;

  if (hDone != OFC_HANDLE_NULL)
    {
      ofc_app_set_wait (hApp, hDone) ;
      ofc_event_wait(hDone);
    }

#if defined (OFC_DISCOVER_IPV6)
  DGramTestClient = BlueHeapMalloc (sizeof (BLUE_DGRAM_TEST_CLIENT)) ;
  DGramTestClient->family = BLUE_FAMILY_IPV6 ;
  DGramTestClient->hAppQueue = BlueQcreate () ;
  DGramTestClient->hListenApp = OFC_HANDLE_NULL ;
  DGramTestClient->count = 0 ;
  DGramTestClient->state = DGRAM_TEST_CLIENT_STATE_IDLE ;
  DGramTestClient->scheduler = hScheduler ;

  BlueCprintf ("Creating Datagram IPV6 Client Application\n") ; 
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
