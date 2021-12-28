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

static OFC_INT
test_startup_persist(OFC_VOID)
{
  OFC_INT ret = 1;
  OFC_TCHAR path[OFC_MAX_PATH] ;

  if (ofc_env_get (OFC_ENV_HOME, path, OFC_MAX_PATH) == OFC_TRUE)
    {
      BlueFrameworkLoad (path);
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
  BlueFrameworkInit();
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
  BlueFrameworkShutdown();
  BlueFrameworkDestroy();
}

typedef enum
  {
    /** 
     * The Socket Server application has just been created
     */
    STREAM_TEST_STATE_IDLE,
    /** 
     * The Socket Server application is running
     */
    STREAM_TEST_STATE_RUNNING,
  } STREAM_TEST_STATE ;

#define STREAM_TEST_INTERVAL 2000
#define STREAM_TEST_COUNT 5
#define STREAM_TEST_PORT 7542

typedef struct
{
  /**
   * The IP Address of the Interface
   */
  BLUE_IPADDR ip ;
  /**
   * The Handle of the Socket that is listening on the interface
   */
  BLUE_HANDLE hListen ;
} BLUE_STREAM_INTERFACE ;

/**
 * The management information for the Stream Socket Server Application
 */
typedef struct
{
  /**
   * The state that the StreamSocket Server Application is in
   */
  STREAM_TEST_STATE state ;
  /**
   * A Handle to the Applications timer.
   */
  BLUE_HANDLE hTimer ;
#if defined(OFC_MULTI_TCP)
  /**
   * A Handle to the configuration update wait queue
   */
  BLUE_HANDLE hConfigUpdate ;
#endif
  /**
   * A reference to the Application's scheduler
   */
  BLUE_HANDLE scheduler ;
  /**
   * A handle to the list of interfaces that this socket server manages.
   */
  BLUE_HANDLE interfaceList ;
  /**
   * Number of runs through the test
   */
  OFC_INT count ;
  BLUE_FAMILY_TYPE family ;
} BLUE_STREAM_TEST ;

static OFC_VOID StreamTestPreSelect (BLUE_HANDLE app) ;
static BLUE_HANDLE StreamTestPostSelect (BLUE_HANDLE app, 
					 BLUE_HANDLE hSocket) ;
static OFC_VOID StreamTestDestroy (BLUE_HANDLE app) ;

static OFC_APP_TEMPLATE StreamTestAppDef =
  {
    "Stream Test Application",
    &StreamTestPreSelect,
    &StreamTestPostSelect,
    &StreamTestDestroy,
#if defined(OFC_APP_DEBUG)
    OFC_NULL
#endif
  } ;

/**
 * \enum SERVER_TEST_STATE
 * Possible states of the Stream Server Application
 *
 * A Stream Server is an application that accepts incoming connections,
 * processes messages, and exits once the message has been received and
 * processed.
 *
 * \remark
 * Additional states can be added for a more socket server application.
 */
typedef enum
  {
    /**
     * The Application is Idle and has just been created
     */
    SERVER_TEST_STATE_IDLE,
    /**
     * The application is running and is receiving the header of a message
     */
    SERVER_TEST_STATE_HEADER,
    /**
     * The application is running and is receiving the body of a message
     */
    SERVER_TEST_STATE_BODY
  } SERVER_TEST_STATE ;

typedef struct
{
  /**
   * The Application's State
   */
  SERVER_TEST_STATE state ;
  /**
   * The socket that is listening for incoming connections.
   */
  BLUE_HANDLE masterSocket ;
  /**
   * The socket of the accepted connection
   */
  BLUE_HANDLE hSocket ;
  /**
   * The handle of this application's scheduler
   */
  BLUE_HANDLE scheduler ;
  /**
   * The message that is being received
   */
  BLUE_MESSAGE *recv_msg ;
  /**
   * The header of the received message (contains the message length)
   */
  OFC_UINT32 header ;
} BLUE_SERVER_TEST ;

static OFC_VOID ServerTestPreSelect (BLUE_HANDLE app) ;
static BLUE_HANDLE ServerTestPostSelect (BLUE_HANDLE app, BLUE_HANDLE hSocket) ;
static OFC_VOID ServerTestDestroy (BLUE_HANDLE app) ;

static OFC_APP_TEMPLATE ServerTestAppDef =
  {
    "Stream Server Test Application",
    &ServerTestPreSelect,
    &ServerTestPostSelect,
    &ServerTestDestroy,
#if defined(OFC_APP_DEBUG)
    OFC_NULL
#endif
  } ;

/**
 * The states that a Stream Test Client Application can be in
 */
typedef enum
  {
    /**
     * The client is idle and just created.
     */
    CLIENT_TEST_STATE_IDLE,
    /**
     * The client is attempting to connect to the server
     */
    CLIENT_TEST_STATE_CONNECTING,
    /**
     * The client is connected to the server and attempting to send a message.
     */
    CLIENT_TEST_STATE_CONNECTED
  } CLIENT_TEST_STATE ;

/**
 * The client application's management information
 */
typedef struct
{
  /**
   * The client application's state
   */
  CLIENT_TEST_STATE state ;
  /**
   * The handle to the socket to send the client's message on.
   */
  BLUE_HANDLE hSocket ;
  /**
   * The handle to the application's scheduler
   */
  BLUE_HANDLE scheduler ;
  /**
   * The message that the application is sending
   */
  BLUE_MESSAGE *write_msg ;
  BLUE_HANDLE app ;
  BLUE_IPADDR ip ;
} BLUE_CLIENT_TEST ;

static OFC_VOID ClientTestPreSelect (BLUE_HANDLE app) ;
static BLUE_HANDLE ClientTestPostSelect (BLUE_HANDLE app, 
					 BLUE_HANDLE hSocket) ;
static OFC_VOID ClientTestDestroy (BLUE_HANDLE app) ;

/**
 * The Stream Client's Application Scheduler Information
 */
static OFC_APP_TEMPLATE ClientTestAppDef =
  {
    "Stream Client Test Application",
    &ClientTestPreSelect,
    &ClientTestPostSelect,
    &ClientTestDestroy,
#if defined(OFC_APP_DEBUG)
    OFC_NULL
#endif
  } ;

/**
 * The offset in a message where the message size is located.
 */
#define OFFSET_CLIENT_MSG_SIZE 0
/**
 * The offset in a message where the message's data is located.
 */
#define OFFSET_CLIENT_MSG_DATA 4
/**
 * The size of the message header
 */
#define CLIENT_MSG_HEADER_LEN 4
/**
 * The data portion of the message that is sent
 */
#define CLIENT_MSG_DATA "This is my test message"

/**
 * A Routine to Set up an interface for listening.
 *
 * The routine will allocate an interface structure, and create a server
 * socket that is listening for incoming connection requests.
 *
 * \param ip
 * The IP address of the associated interface to startup
 *
 * \returns
 * A pointer to a BLUE_STREAM_INTERFACE structure
 */
static BLUE_STREAM_INTERFACE *StartupInterface (BLUE_FAMILY_TYPE family,
						BLUE_IPADDR *ip)
{
  BLUE_STREAM_INTERFACE *interface ;
  OFC_CHAR ip_addr[IP6STR_LEN] ;

  /*
   * Allocate a context for this interface
   */
  interface = BlueHeapMalloc (sizeof (BLUE_STREAM_INTERFACE)) ;
  /*
   * Initialize the IP address of the interface
   */
  BlueCmemcpy (&interface->ip, ip, sizeof (BLUE_IPADDR)) ;
  /*
   * Create a Listen Socket for this interface
   */
  interface->hListen = BlueSocketListen (ip, STREAM_TEST_PORT) ;
  if (interface->hListen == BLUE_HANDLE_NULL)
    {
      /*
       * If we had trouble issueing a listen, free up the interface and
       * return an error indication (OFC_NULL)
       */
      BlueHeapFree (interface) ;
      interface = OFC_NULL ;
    }
  else
    {
      /*
       * We have a listening socket, let's notify the user that the
       * interface is started
       */
      BlueCprintf ("Started Interface for %s on %s\n", 
		   (family == BLUE_FAMILY_IP) ? "IPV4" : "IPV6", 
		   BlueNETntop (&interface->ip, ip_addr, IP6STR_LEN)) ;
    }
	      
  return (interface) ;
}

/**
 * Routine to shutdown an interface
 *
 * The routine will destroy the listening socket and free the resources 
 * associated with the interface
 *
 * \param interface
 * A pointer to the interface structure associated with the interface to 
 * shutdown.
 */
static OFC_VOID ShutdownInterface (BLUE_STREAM_INTERFACE *interface)
{
  /*
   * Destroy the listening socket
   */
  BlueSocketDestroy (interface->hListen) ;
  /*
   * And free the interface context
   */
  BlueHeapFree (interface) ;
}

/**
 * Internal Routine to Inialize the Stream Test.
 *
 * The routine will startup all interfaces and add them to the stream
 * test's interface list.  It will also create a timer to be used for
 * creating client applications.
 *
 * \param streamTest
 * A pointer to the Stream Socket Server's management data
 */
static OFC_VOID StreamTestInitialize (BLUE_STREAM_TEST *streamTest)
{
#if !defined(OFC_MULTI_TCP)
  BLUE_IPADDR ip ;
  BLUE_STREAM_INTERFACE *interface ;
#endif

  /*
   * Create a queue for our interface list
   */
  streamTest->interfaceList = BlueQcreate () ;

  /*
   * If we are multi-homed and listen on multiple interfaces, we want to hold
   * off configuring those interfaces until we get configuration updates.
   * If we are going to listen on the IP_ANY address, we can do that now.
   */
#if !defined(OFC_MULTI_TCP)
  /*
   * Get it's IP address
   */
  if (streamTest->family == BLUE_FAMILY_IP)
    {
      ip.ip_version = BLUE_FAMILY_IP ;
      ip.u.ipv4.addr = BLUE_INADDR_ANY ;
    }
  else
    {
      ip.ip_version = BLUE_FAMILY_IPV6 ;
      ip.u.ipv6 = blue_in6addr_any ;
    }

  /*
   * Startup the interface
   */
  interface = StartupInterface (streamTest->family, &ip) ;
  if (interface != OFC_NULL)
    /*
     * If we successfully started the interface, add it to our list
     */
    BlueQenqueue (streamTest->interfaceList, interface) ;
#endif
  /*
   * Create a timer for the socket server application.  We create the
   * clients from within the socket server.  We could create a client
   * for each stream server application (interface specific), but this
   * implementation creates only one client regardless of the number of
   * interfaces we've started.
   */
  streamTest->hTimer = BlueTimerCreate("SOCKET") ;
  if (streamTest->hTimer != BLUE_HANDLE_NULL)
    {
      /*
       * The timer is created, so set it with our test interval.  When it
       * fires, we'll create the client
       */
      BlueTimerSet (streamTest->hTimer, STREAM_TEST_INTERVAL) ;
    }
#if defined(OFC_MULTI_TCP)
  /*
   * Create an event to get configuration updates
   */
  streamTest->hConfigUpdate = BlueEventCreate(BLUE_EVENT_AUTO) ;
  if (streamTest->hConfigUpdate != BLUE_HANDLE_NULL)
    BlueConfigRegisterUpdate (streamTest->hConfigUpdate) ;
#endif
}

#if defined(OFC_MULTI_TCP)
static BLUE_VOID StreamTestReconfig (BLUE_STREAM_TEST *streamTest)
{
  int i ;
  BLUE_IPADDR ip ;
  BLUE_STREAM_INTERFACE *interface ;
  BLUE_STREAM_INTERFACE *next_interface ;
  OFC_BOOL found ;
  /*
   * First look for deleted interfaces
   */
  for (interface = BlueQfirst (streamTest->interfaceList) ;
       interface != OFC_NULL ;
       interface = next_interface)
    {
      next_interface = BlueQnext (streamTest->interfaceList, interface) ;
      /*
       * Is this interface in the update message with the same values
       */
      found = BLUE_FALSE ;
      for (i = 0 ; i < BlueConfigInterfaceCount() && !found ;)
	{
	  BlueConfigInterfaceAddr (i, &ip, OFC_NULL, OFC_NULL) ;
	  if (BlueNETIsAddrEqual (&interface->ip, &ip))
	    found = BLUE_TRUE ;
	  else
	    i++ ;
	}
      if (!found)
	{
	  /*
	   * Delete the interface
	   */
	  BlueQunlink (streamTest->interfaceList, interface) ;
	  ShutdownInterface (interface) ;
	}
    }

  /*
   * Now look for added interfaces
   */
  for (i = 0 ; i < BlueConfigInterfaceCount() ; i++)
    {
      BlueConfigInterfaceAddr (i, &ip, OFC_NULL, OFC_NULL) ;

      found = BLUE_FALSE ;
      for (interface = BlueQfirst (streamTest->interfaceList) ;
	   interface != OFC_NULL && !found ;)
	{
	  /*
	   * Is this interface in the update message with the same values
	   */
	  if (BlueNETIsAddrEqual (&interface->ip, &ip))
	    found = BLUE_TRUE ;
	  else
	    interface = BlueQnext (streamTest->interfaceList, interface) ;
	}

      if (!found && (streamTest->family == ip.ip_version))
	{
	  /*
	   * Add the interface
	   */
	  interface = StartupInterface (streamTest->family, &ip) ;
	  if (interface != OFC_NULL)
	    /*
	     * If we successfully started the interface, add it to our list
	     */
	    BlueQenqueue (streamTest->interfaceList, interface) ;
	}
    }
}
#endif

/**
 * The Scheduler Preselect Routine for the Stream Socket Server
 *
 * This routine will initialize the application if necessary and will 
 * schedule events that the application wishes to receive notifications of.
 *
 * \param app
 * The Application's Handle
 */
static OFC_VOID StreamTestPreSelect (BLUE_HANDLE app)
{
  BLUE_STREAM_TEST *streamTest ;
  BLUE_STREAM_INTERFACE *interface ;
  BLUE_SOCKET_EVENT_TYPE event_types ;
  STREAM_TEST_STATE entry_state ;
  /*
   * Get our application data.  This is the structure that was passed into
   * the ofc_app_create call
   */
  streamTest = ofc_app_get_data (app) ;
  if (streamTest != OFC_NULL)
    {
      do /* while streamTest->state != entry_state */
	{
	  entry_state = streamTest->state ;
	  BlueSchedClearWait (streamTest->scheduler, app) ;

	  /*
	   * Dispatch on our state
	   */
	  switch (streamTest->state)
	    {
	    default:
	    case STREAM_TEST_STATE_IDLE:
	      /*
	       * We have just been created.  So initialize the socket server
	       */
	      StreamTestInitialize (streamTest) ;
	      /*
	       * Now enable the timer so we'll get events to create a client
	       */
	      BlueSchedAddWait (streamTest->scheduler, app, 
				streamTest->hTimer) ;
#if defined(OFC_MULTI_TCP)
	      BlueSchedAddWait (streamTest->scheduler, app, 
				streamTest->hConfigUpdate) ;
#endif
	      /*
	       * For each interface we've started, enable the event on that 
	       * interface's socket.
	       */
	      for (interface = BlueQfirst (streamTest->interfaceList) ;
               interface != OFC_NULL ;
		   interface = BlueQnext (streamTest->interfaceList, 
					  interface))
		{
		  event_types = BLUE_SOCKET_EVENT_ACCEPT ;
		  BlueSocketEnable (interface->hListen, event_types) ;
		  BlueSchedAddWait (streamTest->scheduler, app, 
				    interface->hListen) ;
		}
	      /*
	       * And switch our state to running
	       */
	      streamTest->state = STREAM_TEST_STATE_RUNNING ;
	      break ;

	    case STREAM_TEST_STATE_RUNNING:
	      /*
	       * We are running, so let's just enable the events we're 
	       * interested in.
	       *
	       * Enable the timer to create clients
	       */
	      BlueSchedAddWait (streamTest->scheduler, app, 
				streamTest->hTimer) ;
#if defined(OFC_MULTI_TCP)
	      BlueSchedAddWait (streamTest->scheduler, app, 
				streamTest->hConfigUpdate) ;
#endif
	      /*
	       * And enable each interface we've started
	       */
	      for (interface = BlueQfirst (streamTest->interfaceList) ;
               interface != OFC_NULL ;
		   interface = BlueQnext (streamTest->interfaceList, 
					  interface))
		{
		  event_types = BLUE_SOCKET_EVENT_ACCEPT ;
		  BlueSocketEnable (interface->hListen, event_types) ;
		  BlueSchedAddWait (streamTest->scheduler, app, 
				    interface->hListen) ;
		}
	      break ;
	    }
	}
      while (streamTest->state != entry_state) ;
    }
}

/**
 * The Stream Socket Server's Post Select Routine
 *
 * This routine will be called when one of the events selected in the preselect
 * phase has been triggered.
 *
 * \param app
 * The handle to the application
 *
 * \param hSocket
 * The handle of the event that triggered this activity.
 */
static BLUE_HANDLE StreamTestPostSelect (BLUE_HANDLE app, BLUE_HANDLE hSocket) 
{
  BLUE_STREAM_TEST *streamTest ;
  BLUE_SERVER_TEST *serverTest ;
  BLUE_CLIENT_TEST *clientTest ;
  BLUE_STREAM_INTERFACE *interface ;
  OFC_BOOL progress ;
  BLUE_SOCKET_EVENT_TYPE event_types ;
  OFC_CHAR ip_str[IP6STR_LEN] ;

  /*
   * Get the socket server's context
   */
  streamTest = ofc_app_get_data (app) ;
  if (streamTest != OFC_NULL)
    {
      for (progress = OFC_TRUE ; progress && !ofc_app_destroying(app);)
	{
	  progress = OFC_FALSE ;
	  /*
	   * Dispatch on our state
	   */
	  switch (streamTest->state)
	    {
	    default:
	      break;

	    case STREAM_TEST_STATE_IDLE:
	      /*
	       * We should no longer be idle.
	       */
	      break ;
	  
	    case STREAM_TEST_STATE_RUNNING:
	      /*
	       * Find a matching interface socket
	       */
	      for (interface = BlueQfirst (streamTest->interfaceList) ;
               interface != OFC_NULL && hSocket != interface->hListen ;
		   interface = BlueQnext (streamTest->interfaceList, 
					  interface)) ;

	      if (interface != OFC_NULL && hSocket == interface->hListen)
		{
		  /*
		   * Found an Interface, Determine Event
		   */
		  event_types = BlueSocketTest (hSocket) ;
		  if (event_types & BLUE_SOCKET_EVENT_ACCEPT)
		    {
		      /*
		       * We've received a listen event
		       *
		       * Let's create an app to accept the listen
		       */
		      serverTest = BlueHeapMalloc (sizeof (BLUE_SERVER_TEST)) ;
		      /*
		       * Initialize it's state to idle and set up the info
		       * we know about
		       */
		      serverTest->state = SERVER_TEST_STATE_IDLE ;
		      serverTest->scheduler = streamTest->scheduler ;
		      serverTest->masterSocket = interface->hListen ;
		      /*
		       * Need to do the accept now to clear the event
		       * Some implementations will not clear the listen event
		       * until after the listen has been serviced.  So we
		       * need to service the listen now.
		       */
		      serverTest->hSocket = 
			BlueSocketAccept (serverTest->masterSocket) ;
		      if (serverTest->hSocket == BLUE_HANDLE_NULL)
			{
			  BlueHeapFree (serverTest) ;
			}
		      else
			{
			  progress = OFC_TRUE ;
			  /*
			   * Create the server application for this socket
			   */
			  ofc_app_create (streamTest->scheduler,
                              &ServerTestAppDef,
                              serverTest) ;
			}
		    }
		}
	      else if (hSocket == streamTest->hTimer)
		{
		  if (streamTest->count++ >= STREAM_TEST_COUNT)
		    {
		      /*
		       * we want to kill ourselves after a number of times
		       */
		      ofc_app_kill (app) ;
		    }
		  else
		    {
		      for (interface = BlueQfirst (streamTest->interfaceList) ;
                   interface != OFC_NULL ;
			   interface = BlueQnext (streamTest->interfaceList, 
						  interface)) 
			{
			  /*
			   * This is a timer that we use to create a client.
			   * Let's create an app to send a message
			   */
			  clientTest = 
			    BlueHeapMalloc (sizeof (BLUE_CLIENT_TEST)) ;
			  /*
			   * Set it's state and the info we know
			   */
			  clientTest->state = CLIENT_TEST_STATE_IDLE ;
			  clientTest->scheduler = streamTest->scheduler ;
			  clientTest->ip = interface->ip ;

			  BlueCprintf ("Creating Stream Client for %s "
				       "Application on %s\n",
				       streamTest->family == BLUE_FAMILY_IP ?
				       "IPv4" : "IPv6", 
				       BlueNETntop (&interface->ip,
						    ip_str, IP6STR_LEN)) ;
			  /*
			   * Create the application
			   */
			  clientTest->app =
			    ofc_app_create (streamTest->scheduler,
                                &ClientTestAppDef,
                                clientTest) ;
			}
		      /*
		       * And reset the timer so we'll get notification to 
		       * create another client in the future.
		       */
		      BlueTimerSet (streamTest->hTimer, STREAM_TEST_INTERVAL) ;
		    }
		}
#if defined(OFC_MULTI_TCP)
	      else if (hSocket == streamTest->hConfigUpdate)
		StreamTestReconfig (streamTest) ;
#endif
	      break ;
	    }
	}
    }
  return (BLUE_HANDLE_NULL) ;
}

/**
 * The Stream Socket Server's Destructor
 *
 * This routine will destroy the stream socket server's state.  This includes
 * timers, sockets, and other resources
 *
 * \param app
 * The application's handle
 */
static OFC_VOID StreamTestDestroy (BLUE_HANDLE app)
{
  BLUE_STREAM_TEST *streamTest ;
  BLUE_STREAM_INTERFACE *interface ;

  BlueCprintf ("Destroying Stream Test Application\n") ;

  /*
   * Get the application's data
   */
  streamTest = ofc_app_get_data (app) ;
  if (streamTest != OFC_NULL)
    {
      /*
       * And dispatch on it's state
       */
      switch (streamTest->state)
	{
	default:
	case STREAM_TEST_STATE_IDLE:
	  /*
	   * We should never be idle
	   */
	  break ;

	case STREAM_TEST_STATE_RUNNING:
	  /*
	   * Destroy the timer
	   */
	  BlueTimerDestroy (streamTest->hTimer) ;
#if defined(OFC_MULTI_TCP)
	  BlueConfigUnregisterUpdate (streamTest->hConfigUpdate) ;
	  BlueEventDestroy (streamTest->hConfigUpdate) ;
#endif
	  /*
	   * And Shutdown all our configured interfaces
	   */
	  for (interface = BlueQdequeue (streamTest->interfaceList) ;
           interface != OFC_NULL ;
	       interface = BlueQdequeue (streamTest->interfaceList))
	    {
	      ShutdownInterface (interface) ;
	    }
	  /*
	   * Now destroy the interface list
	   */
	  BlueQdestroy (streamTest->interfaceList) ;
	  break ;
	}
      /*
       * Destroy our context
       */
      BlueHeapFree (streamTest) ;
    }
}

/**
 * The Scheduler Preselect Routine for the Stream Server
 *
 * This routine will initialize the application if necessary and will 
 * schedule events that the application wishes to receive notifications of.
 *
 * \param app
 * The Application's Handle
 */
static OFC_VOID ServerTestPreSelect (BLUE_HANDLE app)
{
  BLUE_SERVER_TEST *serverTest ;
  BLUE_SOCKET_EVENT_TYPE event_types ;
  SERVER_TEST_STATE entry_state ;

  /*
   * Get the application's data
   */
  serverTest = ofc_app_get_data (app) ;
  if (serverTest != OFC_NULL)
    {
      do /* while serverTest->state != entry_state */
	{
	  entry_state = serverTest->state ;
	  BlueSchedClearWait (serverTest->scheduler, app) ;

	  /*
	   * Dispatch on state
	   */
	  switch (serverTest->state)
	    {
	    default:
	    case SERVER_TEST_STATE_IDLE:
	      /*
	       * We are idle, and the listening application already accepted 
	       * the connection.  So initialize some of our context and set 
	       * our state to header.  There's nothing we need to do except 
	       * await an incoming message */
	      serverTest->recv_msg = OFC_NULL ;
	      serverTest->state = SERVER_TEST_STATE_HEADER ;
	      /*
	       * And enable socket events
	       */
	      event_types = BLUE_SOCKET_EVENT_READ | BLUE_SOCKET_EVENT_CLOSE ;
	      BlueSocketEnable (serverTest->hSocket, event_types) ;
	      BlueSchedAddWait (serverTest->scheduler, app, 
				serverTest->hSocket) ;
	      break ;

	    case SERVER_TEST_STATE_HEADER:
	    case SERVER_TEST_STATE_BODY:
	      /*
	       * If we're awaiting a header or a body, simply enable socket
	       * events
	       */
	      event_types = BLUE_SOCKET_EVENT_READ | BLUE_SOCKET_EVENT_CLOSE ;
	      BlueSocketEnable (serverTest->hSocket, event_types) ;
	      BlueSchedAddWait (serverTest->scheduler, app, 
				serverTest->hSocket) ;
	      break ;
	    }
	}
      while (serverTest->state != entry_state) ;
    }
}

/**
 * The Stream Server's Post Select Routine
 *
 * This routine will be called when one of the events selected in the preselect
 * phase has been triggered.
 *
 * \param app
 * The handle to the application
 *
 * \param hSocket
 * The handle of the event that triggered this activity.
 */
static BLUE_HANDLE ServerTestPostSelect (BLUE_HANDLE app, BLUE_HANDLE hSocket) 
{
  BLUE_SERVER_TEST *serverTest ;
  OFC_SIZET count ;
  OFC_BOOL progress ;
  BLUE_SOCKADDR sockaddr_local ;
  BLUE_SOCKADDR sockaddr_remote ;
  OFC_CHAR local_ip_str[IP6STR_LEN] ;
  OFC_CHAR remote_ip_str[IP6STR_LEN] ;

  /*
   * Get the application's data
   */
  serverTest = ofc_app_get_data (app) ;
  if (serverTest != OFC_NULL)
    {
      for (progress = OFC_TRUE ; progress && !ofc_app_destroying(app);)
	{
	  progress = OFC_FALSE ;
	  /*
	   * Dispatch on state
	   */
	  switch (serverTest->state)
	    {
	    default:
	    case SERVER_TEST_STATE_IDLE:
	      /*
	       * We should never be idle
	       */
	      break ;
	  
	    case SERVER_TEST_STATE_HEADER:
	      /*
	       * If we're in the header state, let's see if we've gotten 
	       * a socket event.  It's the only event we expect
	       */
	      if (hSocket == serverTest->hSocket)
		{
		  /*
		   * We have received an event.  We could check whether it's a 
		   * read event or not, but since that's the only thing we 
		   * expect, we'll just go ahead and assume it's a read.
		   */
		  if (serverTest->recv_msg == OFC_NULL)
		    {
		      /*
		       * If we haven't created a message context for the read,
		       * then do so now.  Since we're reading into our 
		       * context info, we set the message type to 
		       * MSG_ALLOC_AUTO. This tells the message handler 
		       * not to dealloc the header but also not to expect 
		       * it to stay in scope.
		       */
		      serverTest->recv_msg = 
			BlueMessageCreate (MSG_ALLOC_AUTO, 
					   CLIENT_MSG_HEADER_LEN,
					   (OFC_CHAR *) &serverTest->header) ;
		    }
		  /*
		   * Now Service the read using recv_msg
		   */
		  progress |= BlueSocketRead (serverTest->hSocket, 
					      serverTest->recv_msg) ;
		  /*
		   * Check if we've received the entire message (or expected
		   * portion)
		   */
		  if (BlueMessageDone (serverTest->recv_msg))
		    {
		      /*
		       * In this case, we were only expecting the header, and
		       * we've received the entire header, so let's figure out
		       * how many bytes are in the message.
		       */
		      count = BLUE_NET_NTOL (&serverTest->header, 0) ;
		      /*
		       * Destroy the message context
		       */
		      BlueMessageDestroy (serverTest->recv_msg) ;
		      /*
		       * And create a new one to read the body.  This one is
		       * created using MSG_ALLOC_HEAP which directs the
		       * message handler to allocate room for the message from
		       * the heap.  
		       */
		      serverTest->recv_msg = 
			BlueMessageCreate (MSG_ALLOC_HEAP, count, OFC_NULL) ;
		      /*
		       * And switch our state to body
		       */
		      serverTest->state = SERVER_TEST_STATE_BODY ;
		    }
		}
	      break ;
	    case SERVER_TEST_STATE_BODY:
	      /*
	       * We are reading in the body.  So let's see if we have a
	       * socket event
	       */
	      if (hSocket == serverTest->hSocket)
		{
		  /*
		   * Yes, since we're in the body state, we know we have a 
		   * recv_msg context, so use it to service the read.
		   */
		  progress |= BlueSocketRead (serverTest->hSocket, 
					      serverTest->recv_msg) ;
		  /*
		   * Have we read in the entire message body?
		   */
		  if (BlueMessageDone (serverTest->recv_msg))
		    {
		      /*
		       * Entire message read in
		       */
		      BlueSocketGetAddresses (serverTest->hSocket,
					      &sockaddr_local,
					      &sockaddr_remote) ;
		      BlueNETntop(&sockaddr_local.sin_addr,
				  local_ip_str, IP6STR_LEN) ;
		      BlueNETntop(&sockaddr_remote.sin_addr,
				  remote_ip_str, IP6STR_LEN) ;
		      BlueCprintf ("Read %d Bytes on %s(%d) from %s(%d)\n",
				   BlueMessageOffset(serverTest->recv_msg),
				   local_ip_str,
				   sockaddr_local.sin_port,
				   remote_ip_str,
				   sockaddr_remote.sin_port) ;
		      /*
		       * We persist only for receipt of one message.  So
		       * start to tear down the application
		       */
		      ofc_app_kill (app) ;
		    }
		}
	      break ;
	    }
	}
    }
  return (BLUE_HANDLE_NULL) ;
}

/**
 * The Stream Server's Destructor
 *
 * This routine will destroy the stream socket server's state.  This includes
 * timers, sockets, and other resources
 *
 * \param app
 * The application's handle
 */
static OFC_VOID ServerTestDestroy (BLUE_HANDLE app)
{
  BLUE_SERVER_TEST *serverTest ;

  BlueCprintf ("Destroying Stream Server Application\n") ;
  /*
   * Get our context
   */
  serverTest = ofc_app_get_data (app) ;
  if (serverTest != OFC_NULL)
    {
      /*
       * And switch on our state
       */
      switch (serverTest->state)
	{
	default:
	case SERVER_TEST_STATE_IDLE:
	  /*
	   * We should not be in the idle state
	   */
	  break ;

	case SERVER_TEST_STATE_HEADER:
	case SERVER_TEST_STATE_BODY:
	  /*
	   * If we are reading (or have read) the header and/or body,
	   * destroy the socket
	   */
	  BlueSocketDestroy (serverTest->hSocket) ;
	  /*
	   * If we have a message allocated, destroy it
	   */
	  if (serverTest->recv_msg != OFC_NULL)
	    BlueMessageDestroy (serverTest->recv_msg) ;
	  break ;
	}
      /*
       * And free the server application's context
       */
      BlueHeapFree (serverTest) ;
    }
}

OFC_BOOL ServiceWrite(BLUE_CLIENT_TEST *clientTest)
{
  OFC_BOOL progress ;
  OFC_SIZET size ;
  OFC_CHAR *buffer ;
  BLUE_SOCKADDR sockaddr_local ;
  BLUE_SOCKADDR sockaddr_remote ;
  OFC_CHAR local_ip_str[IP6STR_LEN] ;
  OFC_CHAR remote_ip_str[IP6STR_LEN] ;

  progress = OFC_FALSE;

  /*
   * Create a message to hold the header and data
   */
  if (clientTest->write_msg == BLUE_HANDLE_NULL)
    {
      size = BlueCstrlen (CLIENT_MSG_DATA) ;
      clientTest->write_msg = BlueMessageCreate (MSG_ALLOC_HEAP,
						 CLIENT_MSG_HEADER_LEN + size,
                                                 OFC_NULL) ;
      /*
       * Since it's allocated from the heap, let's get the
       * address of the buffer to fill with the message, then
       * let's construct the message
       */
      buffer = BlueMessageData(clientTest->write_msg) ;

      BLUE_NET_LTON(buffer, OFFSET_CLIENT_MSG_SIZE, size) ;
      BlueCstrncpy (&buffer[OFFSET_CLIENT_MSG_DATA],
		    CLIENT_MSG_DATA, 
		    BlueCstrlen (CLIENT_MSG_DATA)) ;
    }
  /*
   * Now write it to the server
   */
  progress = BlueSocketWrite (clientTest->hSocket, clientTest->write_msg) ;

  if (BlueMessageDone (clientTest->write_msg))
    {
      BlueSocketGetAddresses (clientTest->hSocket,
			      &sockaddr_local,
			      &sockaddr_remote) ;
      BlueNETntop(&sockaddr_local.sin_addr, local_ip_str, IP6STR_LEN) ;
      BlueNETntop(&sockaddr_remote.sin_addr, remote_ip_str, IP6STR_LEN) ;
      BlueCprintf ("Wrote Message on %s(%d) to %s(%d)\n",
		   local_ip_str,
		   sockaddr_local.sin_port,
		   remote_ip_str,
		   sockaddr_remote.sin_port) ;
      /*
       * We've written the entire message, so since 
       * we're not persistent, let's clean up the 
       * application. If we haven't written the entire 
       * message, we'll get another event when we're in the
       * connected state.  We'll complete the write
       * servicing then.
       */
      ofc_app_kill (clientTest->app) ;
    }
  return (progress) ;
}

/**
 * The Scheduler Preselect Routine for the Stream Client
 *
 * This routine will initialize the application if necessary and will 
 * schedule events that the application wishes to receive notifications of.
 *
 * \param app
 * The Application's Handle
 */
static OFC_VOID ClientTestPreSelect (BLUE_HANDLE app)
{
  BLUE_CLIENT_TEST *clientTest ;
  BLUE_SOCKET_EVENT_TYPE event_types ;
  CLIENT_TEST_STATE entry_state ;

  /*
   * Get our context
   */
  clientTest = ofc_app_get_data (app) ;
  if (clientTest != OFC_NULL)
    {
      do /* while clientTest->state != entry_state */
	{
	  entry_state = clientTest->state ;
	  BlueSchedClearWait (clientTest->scheduler, app) ;
	  /*
	   * And dispatch on our state
	   */
	  switch (clientTest->state)
	    {
	    default:
	    case CLIENT_TEST_STATE_IDLE:
	      /*
	       * We are idle when we are first created.  So set up the client
	       * application.
	       */
	      clientTest->write_msg = OFC_NULL ;
	      clientTest->hSocket = BlueSocketConnect (&clientTest->ip, 
						       STREAM_TEST_PORT) ;
	      if (clientTest->hSocket != BLUE_HANDLE_NULL)
		{
		  clientTest->state = CLIENT_TEST_STATE_CONNECTING ;
	      
		  event_types = BLUE_SOCKET_EVENT_CLOSE | 
		    BLUE_SOCKET_EVENT_WRITE ;

		  BlueSocketEnable (clientTest->hSocket, event_types) ;
		  BlueSchedAddWait (clientTest->scheduler, app, 
				    clientTest->hSocket) ;
		}
	      else
		{
		  /*
		   * We were not able to issue the connect, so we should just
		   * clean up
		   */
		  ofc_app_kill (app) ;
		}

	      break ;

	    case CLIENT_TEST_STATE_CONNECTING:
	    case CLIENT_TEST_STATE_CONNECTED:
	      /*
	       * If we're connecting or connected, we're interested in socket
	       * events
	       */
	      event_types = BLUE_SOCKET_EVENT_CLOSE ;
	      if (clientTest->write_msg == BLUE_HANDLE_NULL)
		event_types |= BLUE_SOCKET_EVENT_WRITE ;
	      BlueSocketEnable (clientTest->hSocket, event_types) ;

	      BlueSchedAddWait (clientTest->scheduler, app, 
				clientTest->hSocket) ;
	      break ;
	    }
	}
      while (clientTest->state != entry_state) ;
    }
}

/**
 * The Stream Client's Post Select Routine
 *
 * This routine will be called when one of the events selected in the preselect
 * phase has been triggered.
 *
 * \param app
 * The handle to the application
 *
 * \param hSocket
 * The handle of the event that triggered this activity.
 */
static BLUE_HANDLE ClientTestPostSelect (BLUE_HANDLE app, BLUE_HANDLE hSocket) 
{
  BLUE_CLIENT_TEST *clientTest ;
  OFC_BOOL progress ;
  BLUE_SOCKET_EVENT_TYPE event_types ;

  /*
   * Get our context
   */
  clientTest = ofc_app_get_data (app) ;
  if (clientTest != OFC_NULL)
    {
      for (progress = OFC_TRUE ; progress && !ofc_app_destroying(app);)
	{
	  progress = OFC_FALSE ;
	  /*
	   * Dispatch on our state
	   */
	  switch (clientTest->state)
	    {
	    default:
	    case CLIENT_TEST_STATE_IDLE:
	      /*
	       * We should not be idle
	       */
	      break ;
	      
	    case CLIENT_TEST_STATE_CONNECTING:
	      /*
	       * If we're connecting, let's see if we have a socket event
	       */
	      if (hSocket == clientTest->hSocket)
		{
		  event_types = BlueSocketTest (hSocket) ;
		  if (event_types & BLUE_SOCKET_EVENT_CLOSE)
		    {
		      ofc_app_kill (app) ;
		    }
		  else if (BlueSocketConnected (clientTest->hSocket))
		    {
		      /*
		       * Yes, so let's say we're connected.
		       */
		      clientTest->state = CLIENT_TEST_STATE_CONNECTED ;
		      progress |= ServiceWrite (clientTest) ;
		    }
		}
	      break ;

	    case CLIENT_TEST_STATE_CONNECTED:
	      /*
	       * We're connected, so let's see if we have a socket event
	       */
	      if (hSocket == clientTest->hSocket)
		progress |= ServiceWrite (clientTest) ;
	      break ;
	    }
	}
    }
  return (BLUE_HANDLE_NULL) ;
}

/**
 * The Stream Client's Destructor
 *
 * This routine will destroy the stream socket server's state.  This includes
 * timers, sockets, and other resources
 *
 * \param app
 * The application's handle
 */
static OFC_VOID ClientTestDestroy (BLUE_HANDLE app)
{
  BLUE_CLIENT_TEST *clientTest ;

  BlueCprintf ("Destroying Stream Client Application\n") ;

  /*
   * Get the app's context
   */
  clientTest = ofc_app_get_data (app) ;
  if (clientTest != OFC_NULL)
    {
      /*
       * And switch on state
       */
      switch (clientTest->state)
	{
	default:
	case CLIENT_TEST_STATE_IDLE:
	  /*
	   * We shouldn't be idle
	   */
	  break ;

	case CLIENT_TEST_STATE_CONNECTING:
	case CLIENT_TEST_STATE_CONNECTED:
	  /*
	   * If we're connected or connecting, let's destroy the client's
	   * socket
	   */
	  BlueSocketDestroy (clientTest->hSocket) ;
	  /*
	   * And free the message if we have one
	   */
	  if (clientTest->write_msg != OFC_NULL)
	    BlueMessageDestroy (clientTest->write_msg) ;
	  break ;
	}
      /*
       * And free the application's context.
       */
      BlueHeapFree (clientTest) ;
    }
}

TEST_GROUP(stream);

TEST_SETUP(stream)
{
  TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(stream)
{
  test_shutdown();
}  

TEST(stream, test_stream)
{
  BLUE_STREAM_TEST *streamTest ;
  BLUE_HANDLE hApp ;

  /*
   * Allocate a management context for the socket server application
   */
  streamTest = BlueHeapMalloc (sizeof (BLUE_STREAM_TEST)) ;
  /*
   * Initialize the state and note the scheduler the app is part of
   */
  streamTest->family = BLUE_FAMILY_IP ;
  streamTest->count = 0 ;
  streamTest->state = STREAM_TEST_STATE_IDLE ;
  streamTest->scheduler = hScheduler ;

  BlueCprintf ("Creating Stream Test Application for IPv4\n") ;
  /*
   * Create the Application using the scheduler definition and the
   * management context
   */
  hApp = ofc_app_create (hScheduler, &StreamTestAppDef, streamTest) ;

  if (hDone != BLUE_HANDLE_NULL)
    {
      ofc_app_set_wait (hApp, hDone) ;
      ofc_event_wait (hDone);
    }

#if defined(OFC_DISCOVER_IPV6)
  /*
   * Now IPv6. Allocate a management context for the socket server application
   */
  streamTest = BlueHeapMalloc (sizeof (BLUE_STREAM_TEST)) ;
  /*
   * Initialize the state and note the scheduler the app is part of
   */
  streamTest->family = BLUE_FAMILY_IPV6 ;
  streamTest->count = 0 ;
  streamTest->state = STREAM_TEST_STATE_IDLE ;
  streamTest->scheduler = hScheduler ;

  BlueCprintf ("Creating Stream Test Application for IPv6\n") ;
  /*
   * Create the Application using the scheduler definition and the
   * management context
   */
  hApp = ofc_app_create (hScheduler, &StreamTestAppDef, streamTest) ;

  if (hDone != BLUE_HANDLE_NULL)
    {
      ofc_app_set_wait (hApp, hDone) ;
      ofc_event_wait (hDone);
    }
#endif
}	  

TEST_GROUP_RUNNER(stream)
{
  RUN_TEST_CASE(stream, test_stream);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(stream);
}

int main(int argc, const char *argv[])
{
  return UnityMain(argc, argv, runAllTests);
}
#endif
