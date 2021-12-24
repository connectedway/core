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
#include "ofc/waitq.h"
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

BLUE_INT test_startup_default(BLUE_VOID)
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

BLUE_INT test_startup(BLUE_VOID)
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

BLUE_VOID test_shutdown(BLUE_VOID)
{
  BlueEventDestroy(hDone);
  BlueSchedQuit(hScheduler);
  BlueFrameworkShutdown();
  BlueFrameworkDestroy();
}

typedef enum
  {
    WAITQ_TEST_STATE_IDLE,
    WAITQ_TEST_STATE_RUNNING,
  } WAITQ_TEST_STATE ;

#define WAITQ_TEST_INTERVAL 2000
#define WAITQ_TEST_COUNT 5
#define WAITQ_MESSAGE "Wait Queue Test Message\n"

typedef struct
{
  WAITQ_TEST_STATE state ;
  BLUE_HANDLE hTimer ;
  BLUE_HANDLE hWaitQueue ;
  BLUE_HANDLE scheduler ;
  BLUE_INT count ;
} BLUE_WAITQ_TEST ;

static BLUE_VOID WaitQueueTestPreSelect (BLUE_HANDLE app) ;
static BLUE_HANDLE WaitQueueTestPostSelect (BLUE_HANDLE app, 
					    BLUE_HANDLE hWaitQueue) ;
static BLUE_VOID WaitQueueTestDestroy (BLUE_HANDLE app) ;

static BLUEAPP_TEMPLATE WaitQueueTestAppDef =
  {
    "Wait Queue Test Application",
    &WaitQueueTestPreSelect,
    &WaitQueueTestPostSelect,
    &WaitQueueTestDestroy,
#if defined(BLUE_PARAM_APP_DEBUG)
    BLUE_NULL
#endif
  } ;

static BLUE_VOID WaitQueueTestPreSelect (BLUE_HANDLE app) 
{
  BLUE_WAITQ_TEST *waitqTest ;
  WAITQ_TEST_STATE entry_state ;

  waitqTest = BlueAppGetData (app) ;
  if (waitqTest != BLUE_NULL)
    {
      do /* while waitqTest->state != entry_state */
	{
	  entry_state = waitqTest->state ;
	  BlueSchedClearWait (waitqTest->scheduler, app) ;

	  switch (waitqTest->state)
	    {
	    default:
	    case WAITQ_TEST_STATE_IDLE:
	      waitqTest->hWaitQueue = BlueWaitQcreate() ;
	      if (waitqTest->hWaitQueue != BLUE_HANDLE_NULL)
		{
		  waitqTest->hTimer = BlueTimerCreate("WQ TEST") ;
		  if (waitqTest->hTimer != BLUE_HANDLE_NULL)
		    {
		      BlueTimerSet (waitqTest->hTimer, WAITQ_TEST_INTERVAL) ;
		      waitqTest->state = WAITQ_TEST_STATE_RUNNING ;
		      BlueSchedAddWait (waitqTest->scheduler, app, 
					waitqTest->hTimer) ;
		    }
		}
	      break ;

	    case WAITQ_TEST_STATE_RUNNING:
	      BlueSchedAddWait (waitqTest->scheduler, app, 
				waitqTest->hWaitQueue) ;
	      BlueSchedAddWait (waitqTest->scheduler, app, 
				waitqTest->hTimer) ;
	      break ;
	    }
	}
      while (waitqTest->state != entry_state) ;
    }
}

static BLUE_HANDLE WaitQueueTestPostSelect (BLUE_HANDLE app, 
					    BLUE_HANDLE hWaitQueue) 
{
  BLUE_WAITQ_TEST *waitqTest ;
  BLUE_CHAR *msg ;
  BLUE_HANDLE hNext ;
  BLUE_BOOL progress ;

  hNext = BLUE_HANDLE_NULL ;
  waitqTest = BlueAppGetData (app) ;
  if (waitqTest != BLUE_NULL)
    {
      for (progress = BLUE_TRUE ; progress && !BlueAppDestroying(app);)
	{
	  progress = BLUE_FALSE ;

	  switch (waitqTest->state)
	    {
	    default:
	    case WAITQ_TEST_STATE_IDLE:
	      break ;
	  
	    case WAITQ_TEST_STATE_RUNNING:
	      if (hWaitQueue == waitqTest->hWaitQueue)
		{
		  msg = BlueWaitQdequeue (waitqTest->hWaitQueue) ;
		  if (msg != BLUE_NULL)
		    {
		      progress = BLUE_TRUE ;
		      BlueCprintf (msg) ;
		      BlueHeapFree (msg) ;
		    }
		}
	      else if (hWaitQueue == waitqTest->hTimer)
		{
		  if (waitqTest->count++ < WAITQ_TEST_COUNT)
		    {
		      msg = BlueHeapMalloc (BlueCstrlen (WAITQ_MESSAGE)+1) ;
		      BlueCstrcpy (msg, WAITQ_MESSAGE) ;
		      BlueWaitQenqueue (waitqTest->hWaitQueue, msg) ;
		      BlueTimerSet (waitqTest->hTimer, WAITQ_TEST_INTERVAL) ;
		      BlueCprintf ("Wait Queue Timer Triggered\n") ;
		      hNext = waitqTest->hWaitQueue ;
		    }
		  else
		    {
		      BlueAppKill (app) ;
		      hNext = BLUE_HANDLE_NULL ;
		    }
		}
	      break ;
	    }
	}
    }
  return (hNext) ;
}

static BLUE_VOID WaitQueueTestDestroy (BLUE_HANDLE app) 
{
  BLUE_WAITQ_TEST *waitqTest ;

  waitqTest = BlueAppGetData (app) ;
  if (waitqTest != BLUE_NULL)
    {
      switch (waitqTest->state)
	{
	default:
	case WAITQ_TEST_STATE_IDLE:
	  break ;

	case WAITQ_TEST_STATE_RUNNING:
	  BlueTimerDestroy (waitqTest->hTimer) ;
	  BlueWaitQdestroy (waitqTest->hWaitQueue) ;
	  break ;
	}
      BlueHeapFree (waitqTest) ;
    }
}

TEST_GROUP(waitq);

TEST_SETUP(waitq)
{
  TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(waitq)
{
  test_shutdown();
}  

TEST(waitq, test_waitq)
{
  BLUE_WAITQ_TEST *waitqTest ;
  BLUE_HANDLE hApp ;

  waitqTest = BlueHeapMalloc (sizeof (BLUE_WAITQ_TEST)) ;
  waitqTest->count = 0 ;
  waitqTest->state = WAITQ_TEST_STATE_IDLE ;
  waitqTest->scheduler = hScheduler ;

  hApp = BlueAppCreate (hScheduler, &WaitQueueTestAppDef, waitqTest) ;

  if (hDone != BLUE_HANDLE_NULL)
    {
      BlueAppSetWait (hApp, hDone) ;
      BlueEventWait(hDone);
    }
}	  

TEST_GROUP_RUNNER(waitq)
{
  RUN_TEST_CASE(waitq, test_waitq);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(waitq);
}

int main(int argc, const char *argv[])
{
  return UnityMain(argc, argv, runAllTests);
}
#endif
