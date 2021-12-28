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
#include "ofc/thread.h"
#include "ofc/libc.h"
#include "ofc/sched.h"
#include "ofc/app.h"
#include "ofc/heap.h"
#include "ofc/core.h"
#include "ofc/framework.h"
#include "ofc/env.h"
#include "ofc/persist.h"
#include "ofc/event.h"

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

/*
 * The loop interval. 5 seconds
 */
#define THREAD_TEST_RUN_INTERVAL 5000

/*
 * Forward Declaration of a Daemon App for the test thread
 *
 * Daemon apps are internal constructs to Blue Share that are leveraged
 * by the BlueThreadTest.  It may be educational to review the deamon
 * code structure, but it is not necessarily required to understand it
 * unless you will be writing daemon apps.
 */
static OFC_DWORD ThreadTestApp (BLUE_HANDLE hThread, OFC_VOID *context) ;

/*
 * States that the deamon can be in
 */
typedef enum
  {
    THREAD_TEST_STATE_IDLE,	/* The Daemon is idle */
    THREAD_TEST_STATE_RUNNING	/* The Daemon is running */
  } THREAD_TEST_STATE ;

/*
 * The deamon's context
 */
typedef struct
{
  THREAD_TEST_STATE state ;	/* The state that the deamon is in */
  BLUE_HANDLE hThread ;		/* The handle to the created thread */
  BLUE_HANDLE hScheduler ;	/* The scheduler that the deamon is on */
  BLUE_HANDLE hTimer ;		/* The timer for thread creation */
} THREAD_TEST_APP ;
/*
 * Forward declaration to the deamon apps preselect routine
 */
static OFC_VOID ThreadTestPreSelect (BLUE_HANDLE app) ;
/*
 * Forward declaration to the deamon apps postselect routine
 */
static BLUE_HANDLE ThreadTestPostSelect (BLUE_HANDLE app, 
					 BLUE_HANDLE hSocket) ;
/*
 * Forward declaration to the deamon apps destroy routine
 */
static OFC_VOID ThreadTestDestroy (BLUE_HANDLE app) ;

#if defined(OFC_APP_DEBUG)
/*
 * Forward declearation to the deamon apps dump routine
 */
static BLUE_VOID ThreadTestDump (BLUE_HANDLE app) ;
#endif

/*
 * Definition of the thread test deamon application
 */
static OFC_APP_TEMPLATE ThreadTestAppDef =
  {
          "Thread Test App",		/* The deamon name */
    &ThreadTestPreSelect,	/* The preselect routine */
    &ThreadTestPostSelect,	/* The post select routine */
    &ThreadTestDestroy,		/* The destroy routine */
#if defined(OFC_APP_DEBUG)
    &ThreadTestDump		/* The dump routine */
#else
          OFC_NULL
#endif
  } ;

#if defined(OFC_APP_DEBUG)
/*
 * The deamon's dump routine.
 *
 * The routine will dump the context on the console
 *
 * \param app
 * The dump callback is given a handle to the app.  The context variable
 * can be obtained from this handle
 */
static BLUE_VOID ThreadTestDump (BLUE_HANDLE app)
{
  THREAD_TEST_APP *ThreadApp ;	/* The deamon's context */

  /*
   * Get the deamon context from the application
   */
  ThreadApp = BlueAppGetData (app) ;
  if (ThreadApp != BLUE_NULL)
    {
      /*
       * Just dump it's state
       */
      BlueCprintf ("%-20s : %d\n", "Thread Test State", 
		   (BLUE_UINT16) ThreadApp->state) ;
    }
}
#endif

/*
 * The test thread's run routine
 *
 * \param hThread
 * Handle to the thread
 *
 * \param context
 * Pointer to the thread's context
 *
 * \returns
 * A status.  0 is success
 */
static OFC_DWORD ThreadTestApp (BLUE_HANDLE hThread, OFC_VOID *context)
{
  /*
   * The test thread should continue running until it is deleted
   */
  while (!BlueThreadIsDeleting (hThread))
    {
      BlueCprintf ("Test Thread is Running\n") ;
      /*
       * Wait for a second
       */
      BlueSleep (1000) ;
    }
  BlueCprintf ("Test Thread is Exiting\n") ;
  return (0) ;
}

/*
 * The thread deamon's preselect routine
 *
 * \param app
 * Handle of the application
 *
 * \returns
 * Nothing
 */
static OFC_VOID ThreadTestPreSelect (BLUE_HANDLE app)
{
  THREAD_TEST_APP *ThreadApp ;	/* The deamon's context */
  THREAD_TEST_STATE entry_state ;
  /*
   * Obtain the deamon's context
   */
  ThreadApp = ofc_app_get_data (app) ;
  if (ThreadApp != OFC_NULL)
    {
      do /* while ThreadApp->state != entry_state */
	{
	  entry_state = ThreadApp->state ;
	  BlueSchedClearWait (ThreadApp->hScheduler, app) ;
	  /*
	   * dispatch on the deamon's state
	   */
	  switch (ThreadApp->state)
	    {
	    default:
	    case THREAD_TEST_STATE_IDLE:
	      /*
	       * deamons are initialy created in the IDLE state.  A
	       * call to the deamon's preselect routine in the IDLE state
	       * is an indication of the deamon startup
	       *
	       * Create a timer event for the deamon.  We use this timer
	       * event to send a delete to the test thread
	       */
	      BlueTimerSet (ThreadApp->hTimer, THREAD_TEST_RUN_INTERVAL) ;
	      /*
	       * Add the timer event to scheduler for our deamon
	       */
	      BlueSchedAddWait (ThreadApp->hScheduler, app, 
				ThreadApp->hTimer) ;
	      /*
	       * And set our state to running
	       */
	      ThreadApp->state = THREAD_TEST_STATE_RUNNING ;
	      break ;

	    case THREAD_TEST_STATE_RUNNING:
	      /*
	       * Keep the timer event active
	       */
	      BlueSchedAddWait (ThreadApp->hScheduler, app, 
				ThreadApp->hTimer) ;
	      break ;
	    }
	}
      while (ThreadApp->state != entry_state) ;
    }
}

/*
 * The post select routine for the thread deamon
 *
 * \param app
 * Handle to the deamon app
 *
 * \param hEvent
 * Event that we should service
 *
 * \returns Cascaded event
 */
static BLUE_HANDLE ThreadTestPostSelect (BLUE_HANDLE app, BLUE_HANDLE hEvent) 
{
  THREAD_TEST_APP *ThreadApp ;	/* The deamon's context */
  OFC_BOOL progress ;		/* Whether we have serviced anything */

  /*
   * Get the deamon's context
   */
  ThreadApp = ofc_app_get_data (app) ;
  if (ThreadApp != OFC_NULL)
    {
      /*
       * While we have just serviced something and while we are not
       * being destroyed
       */
      for (progress = OFC_TRUE ; progress && !ofc_app_destroying(app);)
	{
	  /*
	   * Say we haven't processed anything
	   */
	  progress = OFC_FALSE ;
	  /*
	   * Switch on our state
	   */
	  switch (ThreadApp->state)
	    {
	    default:
	    case THREAD_TEST_STATE_IDLE:
	      /*
	       * We should never be idle anymore
	       */
	      break ;
	  
	    case THREAD_TEST_STATE_RUNNING:
	      /*
	       * The only event we expect is the timer event.  Let's make
	       * sure that's what it is
	       */
	      if (hEvent == ThreadApp->hTimer)
		{
		  /*
		   * Let's schedule us to be destroyed
		   */
		  ofc_app_kill (app) ;
		  /*
		   * And say we've serviced something
		   */
		  progress = OFC_TRUE ;
		}
	      break ;
	    }
	}
    }
  return (BLUE_HANDLE_NULL) ;
}

/*
 * The destroy callback for the deamon
 *
 * \param app
 * The deamon's handle
 *
 * \returns Nothing
 */
static OFC_VOID ThreadTestDestroy (BLUE_HANDLE app)
{
  THREAD_TEST_APP *ThreadApp ;	/* Our deamon's context */

  /*
   * Get the deamon's context
   */
  ThreadApp = ofc_app_get_data (app) ;
  if (ThreadApp != OFC_NULL)
    {
      /*
       * Switch on our state
       */
      switch (ThreadApp->state)
	{
	default:
	case THREAD_TEST_STATE_IDLE:
	  /*
	   * We should never be idle
	   */
	  break ;

	case THREAD_TEST_STATE_RUNNING:
	  /*
	   * We just have a timer to delete
	   */
	  BlueTimerDestroy (ThreadApp->hTimer) ;
	  break ;
	}
    }
}

TEST_GROUP(thread);

TEST_SETUP(thread)
{
  TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(thread)
{
  test_shutdown();
}  

TEST(thread, test_thread)
{
  THREAD_TEST_APP *ThreadApp;
  BLUE_HANDLE hThread;
  BLUE_HANDLE hApp ;

  /*
   * Create the Thread that this deamon will interact with
   */
  hThread = BlueThreadCreate (&ThreadTestApp,
                              BLUE_THREAD_THREAD_TEST, BLUE_THREAD_SINGLETON,
                              OFC_NULL,
                              BLUE_THREAD_JOIN, BLUE_HANDLE_NULL) ;
  if (hThread == BLUE_HANDLE_NULL)
    BlueCprintf ("Could not create ThreadTestApp\n") ;
  else
    {
      /*
       * Created the thread, so let's create the deamon application.
       *
       * Allocate the deamon's context
       */
      ThreadApp = BlueHeapMalloc (sizeof (THREAD_TEST_APP)) ;
      ThreadApp->hScheduler = hScheduler ;
      /*
       * Create a timer that the thread will wait on
       */
      ThreadApp->hTimer = BlueTimerCreate ("THREAD") ;
      ThreadApp->state = THREAD_TEST_STATE_IDLE ;
      ThreadApp->hThread = hThread ;
      /*
       * Create the deamon application
       */
      hApp = ofc_app_create (ThreadApp->hScheduler, &ThreadTestAppDef,
                             ThreadApp) ;
      if (hDone != BLUE_HANDLE_NULL)
	{
	  ofc_app_set_wait (hApp, hDone) ;
	  ofc_event_wait(hDone);

	  BlueCprintf ("Deleting Thread\n") ;
	  /*
	   * Delete the thread
	   */
	  BlueThreadDelete (ThreadApp->hThread) ;
	  BlueThreadWait (ThreadApp->hThread);
	  /*
	   * And delete our deamon's context
	   */
	  BlueHeapFree (ThreadApp) ;
	}
    }
}	  

TEST_GROUP_RUNNER(thread)
{
  RUN_TEST_CASE(thread, test_thread);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(thread);
}

int main(int argc, const char *argv[])
{
  return UnityMain(argc, argv, runAllTests);
}
#endif
