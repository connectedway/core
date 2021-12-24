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
#include "ofc/sched.h"
#include "ofc/app.h"
#include "ofc/heap.h"
#include "ofc/event.h"
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
    EVENT_TEST_STATE_IDLE,
    EVENT_TEST_STATE_RUNNING,
  } EVENT_TEST_STATE ;

#define EVENT_TEST_INTERVAL 2000

typedef struct
{
  EVENT_TEST_STATE state ;
  BLUE_HANDLE hTimer ;
  BLUE_HANDLE hEvent ;
  BLUE_HANDLE scheduler ;
  BLUE_INT count ;
} BLUE_EVENT_TEST ;

static BLUE_VOID EventTestPreSelect (BLUE_HANDLE app) ;
static BLUE_HANDLE EventTestPostSelect (BLUE_HANDLE app, BLUE_HANDLE hEvent) ;
static BLUE_VOID EventTestDestroy (BLUE_HANDLE app) ;
#if defined(BLUE_PARAM_APP_DEBUG)
static BLUE_VOID EventTestDump (BLUE_HANDLE app) ;
#endif

static BLUEAPP_TEMPLATE EventTestAppDef =
  {
    "Event Test Application",
    &EventTestPreSelect,
    &EventTestPostSelect,
    &EventTestDestroy,
#if defined(BLUE_PARAM_APP_DEBUG)
    &EventTestDump
#else
    BLUE_NULL
#endif
  } ;

#if defined(BLUE_PARAM_APP_DEBUG)
static BLUE_VOID EventTestDump (BLUE_HANDLE app)
{
  BLUE_EVENT_TEST *eventTest ;

  eventTest = BlueAppGetData (app) ;
  if (eventTest != BLUE_NULL)
    {
      BlueCprintf ("%-20s : %d\n", "Event Test State", 
		   (BLUE_UINT16) eventTest->state) ;
      BlueCprintf ("\n") ;
    }
}
#endif

static BLUE_VOID EventTestPreSelect (BLUE_HANDLE app) 
{
  BLUE_EVENT_TEST *eventTest ;
  EVENT_TEST_STATE entry_state ;

  eventTest = BlueAppGetData (app) ;
  if (eventTest != BLUE_NULL)
    {
      do /* while eventTest->state != entry_state */
	{
	  entry_state = eventTest->state ;
	  BlueSchedClearWait (eventTest->scheduler, app) ;
	  
	  switch (eventTest->state)
	    {
	    default:
	    case EVENT_TEST_STATE_IDLE:
	      eventTest->hEvent = BlueEventCreate(BLUE_EVENT_AUTO) ;
	      if (eventTest->hEvent != BLUE_HANDLE_NULL)
		{
		  BlueSchedAddWait (eventTest->scheduler, app, 
				    eventTest->hEvent) ;
		  eventTest->hTimer = BlueTimerCreate("EVENT") ;
		  if (eventTest->hTimer != BLUE_HANDLE_NULL)
		    {
		      BlueTimerSet (eventTest->hTimer, EVENT_TEST_INTERVAL) ;
		      BlueSchedAddWait (eventTest->scheduler, app, 
					eventTest->hTimer) ;
		      eventTest->state = EVENT_TEST_STATE_RUNNING ;
		    }
		}
	      break ;

	    case EVENT_TEST_STATE_RUNNING:
	      BlueSchedAddWait (eventTest->scheduler, app, eventTest->hEvent) ;
	      BlueSchedAddWait (eventTest->scheduler, app, eventTest->hTimer) ;
	      break ;
	    }
	}
      while (eventTest->state != entry_state) ;
    }
}

static BLUE_HANDLE EventTestPostSelect (BLUE_HANDLE app, BLUE_HANDLE hEvent) 
{
  BLUE_EVENT_TEST *eventTest ;
  BLUE_BOOL progress ;

  eventTest = BlueAppGetData (app) ;
  if (eventTest != BLUE_NULL)
    {
      for (progress = BLUE_TRUE ; progress && !BlueAppDestroying(app);)
	{
	  progress = BLUE_FALSE ;

	  switch (eventTest->state)
	    {
	    default:
	    case EVENT_TEST_STATE_IDLE:
	      break ;
	      
	    case EVENT_TEST_STATE_RUNNING:
	      if (hEvent == eventTest->hEvent)
		{
		  BlueCprintf ("Event Triggered\n") ;
		  eventTest->count++ ;

		  if (eventTest->count >= 10)
		    BlueAppKill (app) ;
		}
	      else if (hEvent == eventTest->hTimer)
		{
		  BlueEventSet (eventTest->hEvent) ;
		  BlueTimerSet (eventTest->hTimer, EVENT_TEST_INTERVAL) ;
		  BlueCprintf ("Timer Triggered\n") ;
		}
	      break ;
	    }
	}
    }
  return (BLUE_HANDLE_NULL) ;
}

static BLUE_VOID EventTestDestroy (BLUE_HANDLE app) 
{
  BLUE_EVENT_TEST *eventTest ;

  eventTest = BlueAppGetData (app) ;
  if (eventTest != BLUE_NULL)
    {
      switch (eventTest->state)
	{
	default:
	case EVENT_TEST_STATE_IDLE:
	  break ;

	case EVENT_TEST_STATE_RUNNING:
	  BlueTimerDestroy (eventTest->hTimer) ;
	  BlueEventDestroy (eventTest->hEvent) ;
	  break ;
	}

      BlueHeapFree (eventTest) ;
    }
}

TEST_GROUP(event);

TEST_SETUP(event)
{
  TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(event)
{
  test_shutdown();
}  

TEST(event, test_event)
{
  BLUE_EVENT_TEST *eventTest ;
  BLUE_HANDLE hApp ;

  eventTest = BlueHeapMalloc (sizeof (BLUE_EVENT_TEST)) ;
  eventTest->count = 0 ;
  eventTest->state = EVENT_TEST_STATE_IDLE ;
  eventTest->scheduler = hScheduler ;

  hApp = BlueAppCreate (hScheduler, &EventTestAppDef, eventTest) ;

  if (hDone != BLUE_HANDLE_NULL)
    {
      BlueAppSetWait (hApp, hDone) ;
      BlueEventWait(hDone);
    }
}	  

TEST_GROUP_RUNNER(event)
{
  RUN_TEST_CASE(event, test_event);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(event);
}

int main(int argc, const char *argv[])
{
  return UnityMain(argc, argv, runAllTests);
}
#endif
