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

static BLUE_HANDLE hScheduler;
static BLUE_HANDLE hDone;

static BLUE_INT
test_startup_persist(BLUE_VOID)
{
  BLUE_INT ret = 1;
  BLUE_TCHAR path[BLUE_MAX_PATH] ;

  if (BlueEnvGet (BLUE_ENV_HOME, path, BLUE_MAX_PATH) == BLUE_TRUE)
    {
      BlueFrameworkLoad (path);
      ret = 0;
    }
  return (ret);
}

static BLUE_INT test_startup_default(BLUE_VOID)
{
  static BLUE_UUID uuid =
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

static BLUE_INT test_startup(BLUE_VOID)
{
  BLUE_INT ret;
  BlueFrameworkInit();
#if defined(BLUE_PARAM_PERSIST)
  ret = test_startup_persist();
#else
  ret = test_startup_default();
#endif
  hScheduler = BlueSchedCreate();
  hDone = BlueEventCreate(BLUE_EVENT_AUTO);

  return(ret);
}

static BLUE_VOID test_shutdown(BLUE_VOID)
{
  BlueEventDestroy(hDone);
  BlueSchedQuit(hScheduler);
  BlueFrameworkShutdown();
  BlueFrameworkDestroy();
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
  BLUE_HANDLE hTimer ;
  BLUE_HANDLE scheduler ;
  BLUE_HANDLE hSocket ;
  BLUE_MESSAGE *recv_msg ;
} BLUE_DGRAM_TEST_SERVER ;

static BLUE_VOID DGramTestServerPreSelect (BLUE_HANDLE app) ;
static BLUE_HANDLE DGramTestServerPostSelect (BLUE_HANDLE app, 
					      BLUE_HANDLE hSocket) ;
static BLUE_VOID DGramTestServerDestroy (BLUE_HANDLE app) ;
#if defined(BLUE_PARAM_APP_DEBUG)
static BLUE_VOID DGramTestServerDump (BLUE_HANDLE app) ;
#endif

static BLUEAPP_TEMPLATE DGramTestServerAppDef =
  {
    "Datagram Test Server Application",
    &DGramTestServerPreSelect,
    &DGramTestServerPostSelect,
    &DGramTestServerDestroy,
#if defined(BLUE_PARAM_APP_DEBUG)
    &DGramTestServerDump
#else
    BLUE_NULL
#endif
  } ;

#if defined(BLUE_PARAM_APP_DEBUG)
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

static BLUE_VOID DGramTestServerPreSelect (BLUE_HANDLE app) 
{
  BLUE_DGRAM_TEST_SERVER *DGramTestServer ;
  BLUE_SOCKET_EVENT_TYPE event_types ;
  DGRAM_TEST_SERVER_STATE entry_state ;

  DGramTestServer = BlueAppGetData (app) ;
  if (DGramTestServer != BLUE_NULL)
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
	      if (DGramTestServer->hTimer != BLUE_HANDLE_NULL)
		{
		  BlueTimerSet (DGramTestServer->hTimer, 
				DGRAM_TEST_SERVER_INTERVAL) ;
		  DGramTestServer->recv_msg = BLUE_NULL ;
		  DGramTestServer->state = DGRAM_TEST_SERVER_STATE_PRIMING ;

		  BlueSchedAddWait (DGramTestServer->scheduler, app, 
				    DGramTestServer->hTimer) ;
		}
	      else
		BlueAppKill (app) ;

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

static BLUE_BOOL ServiceRead (BLUE_DGRAM_TEST_SERVER *DGramTestServer)
{
  BLUE_BOOL progress ;
  BLUE_IPADDR ip ;
  BLUE_UINT16 port ;
  BLUE_CHAR packet_ip[IP6STR_LEN] ;
  BLUE_CHAR interface_ip[IP6STR_LEN] ;

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
	BlueMessageCreate (MSG_ALLOC_HEAP, 1000, BLUE_NULL) ;
    }
  return (progress);
}


static BLUE_HANDLE DGramTestServerPostSelect (BLUE_HANDLE app, 
					      BLUE_HANDLE hSocket) 
{
  BLUE_DGRAM_TEST_SERVER *DGramTestServer ;
  BLUE_BOOL progress ;
  BLUE_CHAR ip_addr[IP6STR_LEN] ;

  DGramTestServer = BlueAppGetData (app) ;
  if (DGramTestServer != BLUE_NULL)
    {
      for (progress = BLUE_TRUE ; progress && !BlueAppDestroying(app);)
	{
	  progress = BLUE_FALSE ;

	  switch (DGramTestServer->state)
	    {
	    default:
	    case DGRAM_TEST_SERVER_STATE_IDLE:
	      break ;
	  
	    case DGRAM_TEST_SERVER_STATE_PRIMING:
	      if (hSocket == DGramTestServer->hTimer)
		{
		  if (DGramTestServer->recv_msg == BLUE_NULL)
		    {
		      DGramTestServer->hSocket = 
			BlueSocketDatagram (&DGramTestServer->ip, 
					    DGRAM_TEST_PORT) ;
		      if (DGramTestServer->hSocket == BLUE_HANDLE_NULL)
			{
			  BlueAppKill (app) ;
			}
		      else
			{
			  BlueCprintf ("Listening for Datagrams on Interface "
				       "on %s\n", 
				       BlueNETntop (&DGramTestServer->ip,
						    ip_addr, IP6STR_LEN)) ;
			  DGramTestServer->recv_msg = 
			    BlueMessageCreate (MSG_ALLOC_HEAP, 1000,
					       BLUE_NULL) ;
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
  return (BLUE_HANDLE_NULL) ;
}

static BLUE_VOID DGramTestServerDestroy (BLUE_HANDLE app) 
{
  BLUE_DGRAM_TEST_SERVER *DGramTestServer ;

  BlueCprintf ("Destroying Datagram Test Server Application\n") ;
  DGramTestServer = BlueAppGetData (app) ;
  if (DGramTestServer != BLUE_NULL)
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
	  if (DGramTestServer->hSocket != BLUE_HANDLE_NULL)
	    BlueSocketDestroy (DGramTestServer->hSocket) ;
	  break ;
	}
      if (DGramTestServer->recv_msg != BLUE_NULL)
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
  BLUE_HANDLE hSocket ;
  BLUE_HANDLE scheduler ;
  BLUE_MESSAGE *send_msg ;
  BLUE_HANDLE hTimer ;
  BLUE_INT count ;
  BLUE_HANDLE hAppQueue ;
  BLUE_HANDLE hDestroy;
  BLUE_HANDLE hListenApp ;
  BLUE_HANDLE hCurrApp ;
  BLUE_HANDLE hConfigUpdate ;
} BLUE_DGRAM_TEST_CLIENT ;

static BLUE_VOID DGramTestClientPreSelect (BLUE_HANDLE app) ;
static BLUE_HANDLE DGramTestClientPostSelect (BLUE_HANDLE app, 
					      BLUE_HANDLE hSocket) ;
static BLUE_VOID DGramTestClientDestroy (BLUE_HANDLE app) ;
#if defined(BLUE_PARAM_APP_DEBUG)
static BLUE_VOID DGramTestClientDump (BLUE_HANDLE app) ;
#endif

static BLUEAPP_TEMPLATE DGramTestClientAppDef =
  {
    "Datagram Test Client Application",
    &DGramTestClientPreSelect,
    &DGramTestClientPostSelect,
    &DGramTestClientDestroy,
#if defined(BLUE_PARAM_APP_DEBUG)
    &DGramTestClientDump
#else
    BLUE_NULL
#endif
  } ;

#if defined(BLUE_PARAM_APP_DEBUG)
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

static BLUE_VOID DGramTestClientPreSelect (BLUE_HANDLE app) 
{
  BLUE_DGRAM_TEST_CLIENT *DGramTestClient ;
  BLUE_SOCKET_EVENT_TYPE event_types ;
#if !defined(BLUE_PARAM_MULTI_UDP)
  BLUE_DGRAM_TEST_SERVER *DGramTestServer ;
  BLUE_CHAR ip_addr[IP6STR_LEN] ;
#endif
  BLUE_IPADDR ipaddr ;
  DGRAM_TEST_CLIENT_STATE entry_state ;

  DGramTestClient = BlueAppGetData (app) ;
  if (DGramTestClient != BLUE_NULL)
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
	      DGramTestClient->hDestroy = BlueEventCreate(BLUE_EVENT_AUTO);
	      DGramTestClient->hConfigUpdate = BlueEventCreate(BLUE_EVENT_AUTO) ;
	      if (DGramTestClient->hConfigUpdate != BLUE_HANDLE_NULL)
		{
		  BlueConfigRegisterUpdate (DGramTestClient->hConfigUpdate) ;
		}

	      DGramTestClient->hCurrApp = 
		(BLUE_HANDLE) BlueQfirst (DGramTestClient->hAppQueue) ;
#if !defined(BLUE_PARAM_MULTI_UDP)
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
#if defined(BLUE_PARAM_DISCOVER_IPV6)
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
		BlueAppCreate (DGramTestServer->scheduler, 
			       &DGramTestServerAppDef, DGramTestServer) ;
#endif

	      DGramTestClient->send_msg = BLUE_NULL ;
	      if (DGramTestClient->family == BLUE_FAMILY_IP)
		{
		  ipaddr.ip_version = BLUE_FAMILY_IP ;
		  ipaddr.u.ipv4.addr = BLUE_INADDR_ANY ;
		}
#if defined(BLUE_PARAM_DISCOVER_IPV6)
	      else
		{
		  ipaddr.ip_version = BLUE_FAMILY_IPV6 ;
		  ipaddr.u.ipv6 = blue_in6addr_any ;
		}
#endif
	      DGramTestClient->hSocket = 
		BlueSocketDatagram (&ipaddr, 0) ;

	      if (DGramTestClient->hSocket != BLUE_HANDLE_NULL)
		{
		  DGramTestClient->hTimer = BlueTimerCreate("DG CLIENT") ;
		  if (DGramTestClient->hTimer != BLUE_HANDLE_NULL)
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
		    BlueAppKill(app) ;
		}
	      else
		BlueAppKill (app) ;

	      break ;

	    case DGRAM_TEST_CLIENT_STATE_CONNECTED:
	      event_types = BLUE_SOCKET_EVENT_CLOSE ;
	      if (DGramTestClient->send_msg != BLUE_NULL)
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

static BLUE_VOID DGramTestReconfig (BLUE_DGRAM_TEST_CLIENT *DGramTestClient)
{
  int i ;
  BLUE_IPADDR ip ;
  BLUE_IPADDR mask ;
  BLUE_IPADDR bcast ;
  BLUE_HANDLE next_app ;
  BLUE_HANDLE hServerApp ;
  BLUE_BOOL found ;
  BLUE_DGRAM_TEST_SERVER *DGramTestServer ;
  BLUE_CHAR ip_addr[IP6STR_LEN] ;

  for (hServerApp = (BLUE_HANDLE) BlueQfirst (DGramTestClient->hAppQueue) ;
       hServerApp != BLUE_HANDLE_NULL ;
       hServerApp = next_app)
    {
      next_app = (BLUE_HANDLE) BlueQnext (DGramTestClient->hAppQueue, 
					  (BLUE_VOID *) hServerApp) ;
      DGramTestServer = BlueAppGetData (hServerApp) ;
      found = BLUE_FALSE ;

      for (i = 0 ; i < BlueConfigInterfaceCount() && !found ;)
	{
	  BlueConfigInterfaceAddr (i, &ip, &bcast, &mask) ;
	  if (BlueNETIsAddrEqual (&DGramTestServer->ip, &ip) &&
	      BlueNETIsAddrEqual (&DGramTestServer->bcast, &bcast) &&
	      BlueNETIsAddrEqual (&DGramTestServer->mask, &mask))
	    found = BLUE_TRUE ;
	  else
	    i++ ;
	}
      if (!found)
	{
	  if (DGramTestClient->hCurrApp == hServerApp)
	    DGramTestClient->hCurrApp = next_app ;
	  BlueQunlink (DGramTestClient->hAppQueue, (BLUE_VOID *) hServerApp) ;
	  BlueAppKill (hServerApp) ;
	}
    }

  /*
   * Now look for added interfaces
   */
  for (i = 0 ; i < BlueConfigInterfaceCount() ; i++)
    {
      BlueConfigInterfaceAddr (i, &ip, &bcast, &mask) ;

      found = BLUE_FALSE ;
      for (hServerApp = (BLUE_HANDLE) BlueQfirst (DGramTestClient->hAppQueue) ;
	   hServerApp != BLUE_HANDLE_NULL && !found ;)
	{
	  DGramTestServer = BlueAppGetData (hServerApp) ;
	  if (BlueNETIsAddrEqual (&DGramTestServer->ip, &ip) &&
	      BlueNETIsAddrEqual (&DGramTestServer->bcast, &bcast) &&
	      BlueNETIsAddrEqual (&DGramTestServer->mask, &mask))
	    found = BLUE_TRUE ;
	  else
	    hServerApp = (BLUE_HANDLE) BlueQnext (DGramTestClient->hAppQueue, 
						  (BLUE_VOID *) hServerApp) ;
	}

      if (!found && (!BlueNETIsAddrLinkLocal (&ip)))
	/*
	 * Only configure link local interfaces for dg test
	 */
	found = BLUE_TRUE ;
      /*
       * If our device does not loopback broadcast datagrams to the
       * sending interface, then don't configure server's on anything
       * but the loopback interface 
       */
#if defined(BLUE_PARAM_LOOPBACK)
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
	  if (DGramTestServer != BLUE_NULL)
	    {
	      DGramTestServer->ip = ip ;
	      DGramTestServer->bcast = bcast ;
	      DGramTestServer->mask = mask ;
	      DGramTestServer->state = DGRAM_TEST_SERVER_STATE_IDLE ;
	      DGramTestServer->scheduler = DGramTestClient->scheduler ;

	      BlueCprintf ("Creating Datagram Test Server Application on %s\n",
			   BlueNETntop (&DGramTestServer->ip,
					ip_addr, IP6STR_LEN)) ;

	      hServerApp = BlueAppCreate (DGramTestServer->scheduler, 
					  &DGramTestServerAppDef, 
					  DGramTestServer) ;
	      if (hServerApp != BLUE_HANDLE_NULL)
		BlueQenqueue (DGramTestClient->hAppQueue, 
			      (BLUE_VOID *) hServerApp) ;
	      else
		BlueHeapFree (DGramTestServer) ;
	    }
	}
    }
}

static BLUE_HANDLE DGramTestClientPostSelect (BLUE_HANDLE app, 
					      BLUE_HANDLE hSocket) 
{
  BLUE_DGRAM_TEST_CLIENT *DGramTestClient ;
  BLUE_DGRAM_TEST_SERVER *DGramTestServer ;
  BLUE_CHAR *buffer ;
  BLUE_BOOL progress ;
  BLUE_CHAR packet_ip[IP6STR_LEN] ;
  BLUE_CHAR interface_ip[IP6STR_LEN] ;

  DGramTestClient = BlueAppGetData (app) ;
  if (DGramTestClient != BLUE_NULL)
    {
      for (progress = BLUE_TRUE ; progress && !BlueAppDestroying(app);)
	{
	  progress = BLUE_FALSE ;
	  switch (DGramTestClient->state)
	    {
	    default:
	    case DGRAM_TEST_CLIENT_STATE_IDLE:
	      break ;
	  
	    case DGRAM_TEST_CLIENT_STATE_CONNECTED:
	      if (hSocket == DGramTestClient->hTimer)
		{
		  if (DGramTestClient->send_msg == BLUE_NULL)
		    {
		      /*
		       * We will service the server queue round robin
		       */
		      DGramTestClient->hCurrApp = (BLUE_HANDLE)
			BlueQnext (DGramTestClient->hAppQueue, 
				   (BLUE_VOID *) DGramTestClient->hCurrApp) ;
		      if (DGramTestClient->hCurrApp == BLUE_HANDLE_NULL)
			DGramTestClient->hCurrApp = (BLUE_HANDLE) 
			  BlueQfirst (DGramTestClient->hAppQueue) ;

		      if (DGramTestClient->hCurrApp == BLUE_HANDLE_NULL)
			BlueCprintf ("No %s Targets to Send message To\n",
				     DGramTestClient->family ==
				     BLUE_FAMILY_IP ? "IPv4" : "IPv6") ;
		      else
			{
			  DGramTestServer = 
			    BlueAppGetData (DGramTestClient->hCurrApp) ;
			  DGramTestClient->send_msg = 
			    BlueDatagramCreate (MSG_ALLOC_HEAP,
						BlueCstrlen (CLIENT_MSG_DATA) + 
						1,
						BLUE_NULL,
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
			      DGramTestClient->send_msg = BLUE_NULL ;
			    }
			}
		    }
		  BlueTimerSet (DGramTestClient->hTimer, 
				DGRAM_TEST_CLIENT_INTERVAL) ;
		  DGramTestClient->count++ ;
		  if (DGramTestClient->count >= DGRAM_TEST_COUNT)
		    BlueEventSet(DGramTestClient->hDestroy);
		}
	      else if (hSocket == DGramTestClient->hSocket)
		{
		  if (DGramTestClient->send_msg != BLUE_NULL)
		    {
		      progress |= BlueSocketWrite (DGramTestClient->hSocket, 
						   DGramTestClient->send_msg) ;
		      if (BlueMessageDone (DGramTestClient->send_msg))
			{
			  BlueMessageDestroy (DGramTestClient->send_msg) ;
			  DGramTestClient->send_msg = BLUE_NULL ;
			}
		    }
		}
	      else if (hSocket == DGramTestClient->hConfigUpdate)
		{
		  DGramTestReconfig (DGramTestClient) ;
		}
	      else if (hSocket == DGramTestClient->hDestroy)
		{
		  BLUE_HANDLE hDestroyApp;
		  DGramTestClient->state = DGRAM_TEST_CLIENT_STATE_DESTROYING;
		  hDestroyApp =
		    (BLUE_HANDLE) BlueQdequeue (DGramTestClient->hAppQueue);
		  if (hDestroyApp == BLUE_HANDLE_NULL)
		    {
		      hDestroyApp = DGramTestClient->hListenApp;
		      DGramTestClient->hListenApp = BLUE_HANDLE_NULL;
		    }
		  if (hDestroyApp != BLUE_HANDLE_NULL)
		    {
		      BlueAppSetWait (hDestroyApp, DGramTestClient->hDestroy) ;
		      BlueAppKill (hDestroyApp);
		    }
		  else
		    {
		      BlueAppKill(app);
		    }
		}
	      else
		{
		  BlueAppKill (app);
		}
	      break;

	    case DGRAM_TEST_CLIENT_STATE_DESTROYING:
	      if (hSocket == DGramTestClient->hDestroy)
		{
		  BLUE_HANDLE hDestroyApp;
		  hDestroyApp =
		    (BLUE_HANDLE) BlueQdequeue (DGramTestClient->hAppQueue);
		  if (hDestroyApp == BLUE_HANDLE_NULL)
		    {
		      hDestroyApp = DGramTestClient->hListenApp;
		      DGramTestClient->hListenApp = BLUE_HANDLE_NULL;
		    }
		  if (hDestroyApp != BLUE_HANDLE_NULL)
		    {
		      BlueAppSetWait (hDestroyApp, DGramTestClient->hDestroy) ;
		      BlueAppKill (hDestroyApp);
		    }
		  else
		    {
		      BlueAppKill(app);
		    }
		}
	      else
		{
		  BlueAppKill(app);
		}
	      break ;
	    }
	}
    }
  return (BLUE_HANDLE_NULL) ;
}

static BLUE_VOID DGramTestClientDestroy (BLUE_HANDLE app) 
{
  BLUE_DGRAM_TEST_CLIENT *DGramTestClient ;
  BLUE_HANDLE hAppServer ;

  BlueCprintf ("Destroying DGram Test Client Application\n") ;
  DGramTestClient = BlueAppGetData (app) ;
  if (DGramTestClient != BLUE_NULL)
    {
      switch (DGramTestClient->state)
	{
	default:
	case DGRAM_TEST_CLIENT_STATE_IDLE:
	  break ;

	case DGRAM_TEST_CLIENT_STATE_DESTROYING:
	case DGRAM_TEST_CLIENT_STATE_CONNECTED:
	  BlueSocketDestroy (DGramTestClient->hSocket) ;
	  if (DGramTestClient->send_msg != BLUE_NULL)
	    BlueMessageDestroy (DGramTestClient->send_msg) ;
	  BlueTimerDestroy (DGramTestClient->hTimer) ;
	  BlueConfigUnregisterUpdate (DGramTestClient->hConfigUpdate) ;
	  BlueEventDestroy (DGramTestClient->hConfigUpdate) ;
	  BlueEventDestroy (DGramTestClient->hDestroy);
	  break ;
	}

      for (hAppServer = 
	     (BLUE_HANDLE) BlueQdequeue (DGramTestClient->hAppQueue) ;
	   hAppServer != BLUE_HANDLE_NULL ;
	   hAppServer = (BLUE_HANDLE) 
	     BlueQdequeue (DGramTestClient->hAppQueue))
	{
	  BlueCprintf ("App Server still queued at destroy");
	  BlueAppKill (hAppServer) ;
	}
      BlueQdestroy (DGramTestClient->hAppQueue) ;

      if (DGramTestClient->hListenApp != BLUE_HANDLE_NULL)
	{
	  BlueCprintf ("Listener still active at destroy");
	  BlueAppKill (DGramTestClient->hListenApp) ;
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
  BLUE_HANDLE hApp ;
  /*
   * Let's create a client
   */
  DGramTestClient = BlueHeapMalloc (sizeof (BLUE_DGRAM_TEST_CLIENT)) ;
  DGramTestClient->family = BLUE_FAMILY_IP ;
  DGramTestClient->hAppQueue = BlueQcreate () ;
  DGramTestClient->hListenApp = BLUE_HANDLE_NULL ;
  DGramTestClient->count = 0 ;
  DGramTestClient->state = DGRAM_TEST_CLIENT_STATE_IDLE ;
  DGramTestClient->scheduler = hScheduler ;

  BlueCprintf ("Creating Datagram IPV4 Client Application\n") ; 
  hApp = BlueAppCreate (hScheduler, &DGramTestClientAppDef, DGramTestClient) ;

  if (hDone != BLUE_HANDLE_NULL)
    {
      BlueAppSetWait (hApp, hDone) ;
      BlueEventWait(hDone);
    }

#if defined (BLUE_PARAM_DISCOVER_IPV6)
  DGramTestClient = BlueHeapMalloc (sizeof (BLUE_DGRAM_TEST_CLIENT)) ;
  DGramTestClient->family = BLUE_FAMILY_IPV6 ;
  DGramTestClient->hAppQueue = BlueQcreate () ;
  DGramTestClient->hListenApp = BLUE_HANDLE_NULL ;
  DGramTestClient->count = 0 ;
  DGramTestClient->state = DGRAM_TEST_CLIENT_STATE_IDLE ;
  DGramTestClient->scheduler = hScheduler ;

  BlueCprintf ("Creating Datagram IPV6 Client Application\n") ; 
  hApp = BlueAppCreate (hScheduler, &DGramTestClientAppDef, DGramTestClient) ;
#endif

  if (hDone != BLUE_HANDLE_NULL)
    {
      BlueAppSetWait (hApp, hDone) ;
      BlueEventWait(hDone);
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
