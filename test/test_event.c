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
    EVENT_TEST_STATE_IDLE,
    EVENT_TEST_STATE_RUNNING,
  } EVENT_TEST_STATE ;

#define EVENT_TEST_INTERVAL 2000

typedef struct
{
  EVENT_TEST_STATE state ;
  OFC_HANDLE hTimer ;
  OFC_HANDLE hEvent ;
  OFC_HANDLE scheduler ;
  OFC_INT count ;
} OFC_EVENT_TEST ;

static OFC_VOID EventTestPreSelect (OFC_HANDLE app) ;
static OFC_HANDLE EventTestPostSelect (OFC_HANDLE app, OFC_HANDLE hEvent) ;
static OFC_VOID EventTestDestroy (OFC_HANDLE app) ;
#if defined(OFC_APP_DEBUG)
static OFC_VOID EventTestDump (OFC_HANDLE app) ;
#endif

static OFC_APP_TEMPLATE EventTestAppDef =
  {
          "Event Test Application",
          &EventTestPreSelect,
          &EventTestPostSelect,
          &EventTestDestroy,
#if defined(OFC_APP_DEBUG)
    &EventTestDump
#else
          OFC_NULL
#endif
  } ;

#if defined(OFC_APP_DEBUG)
static OFC_VOID EventTestDump (OFC_HANDLE app)
{
  OFC_EVENT_TEST *eventTest ;

  eventTest = ofc_app_get_data (app) ;
  if (eventTest != OFC_NULL)
    {
      ofc_printf ("%-20s : %d\n", "Event Test State", 
		   (OFC_UINT16) eventTest->state) ;
      ofc_printf ("\n") ;
    }
}
#endif

static OFC_VOID EventTestPreSelect (OFC_HANDLE app)
{
  OFC_EVENT_TEST *eventTest ;
  EVENT_TEST_STATE entry_state ;

  eventTest = ofc_app_get_data (app) ;
  if (eventTest != OFC_NULL)
    {
      do /* while eventTest->state != entry_state */
	{
	  entry_state = eventTest->state ;
	  ofc_sched_clear_wait (eventTest->scheduler, app) ;
	  
	  switch (eventTest->state)
	    {
	    default:
	    case EVENT_TEST_STATE_IDLE:
	      eventTest->hEvent = ofc_event_create(OFC_EVENT_AUTO) ;
	      if (eventTest->hEvent != OFC_HANDLE_NULL)
		{
		  ofc_sched_add_wait (eventTest->scheduler, app,
                              eventTest->hEvent) ;
		  eventTest->hTimer = ofc_timer_create("EVENT") ;
		  if (eventTest->hTimer != OFC_HANDLE_NULL)
		    {
		      ofc_timer_set (eventTest->hTimer, EVENT_TEST_INTERVAL) ;
		      ofc_sched_add_wait (eventTest->scheduler, app,
                                  eventTest->hTimer) ;
		      eventTest->state = EVENT_TEST_STATE_RUNNING ;
		    }
		}
	      break ;

	    case EVENT_TEST_STATE_RUNNING:
	      ofc_sched_add_wait (eventTest->scheduler, app, eventTest->hEvent) ;
	      ofc_sched_add_wait (eventTest->scheduler, app, eventTest->hTimer) ;
	      break ;
	    }
	}
      while (eventTest->state != entry_state) ;
    }
}

static OFC_HANDLE EventTestPostSelect (OFC_HANDLE app, OFC_HANDLE hEvent)
{
  OFC_EVENT_TEST *eventTest ;
  OFC_BOOL progress ;

  eventTest = ofc_app_get_data (app) ;
  if (eventTest != OFC_NULL)
    {
      for (progress = OFC_TRUE ; progress && !ofc_app_destroying(app);)
	{
	  progress = OFC_FALSE ;

	  switch (eventTest->state)
	    {
	    default:
	    case EVENT_TEST_STATE_IDLE:
	      break ;
	      
	    case EVENT_TEST_STATE_RUNNING:
	      if (hEvent == eventTest->hEvent)
		{
		  ofc_printf ("Event Triggered\n") ;
		  eventTest->count++ ;

		  if (eventTest->count >= 10)
		    ofc_app_kill (app) ;
		}
	      else if (hEvent == eventTest->hTimer)
		{
		  ofc_event_set (eventTest->hEvent) ;
		  ofc_timer_set (eventTest->hTimer, EVENT_TEST_INTERVAL) ;
		  ofc_printf ("Timer Triggered\n") ;
		}
	      break ;
	    }
	}
    }
  return (OFC_HANDLE_NULL) ;
}

static OFC_VOID EventTestDestroy (OFC_HANDLE app)
{
  OFC_EVENT_TEST *eventTest ;

  eventTest = ofc_app_get_data (app) ;
  if (eventTest != OFC_NULL)
    {
      switch (eventTest->state)
	{
	default:
	case EVENT_TEST_STATE_IDLE:
	  break ;

	case EVENT_TEST_STATE_RUNNING:
	  ofc_timer_destroy (eventTest->hTimer) ;
	  ofc_event_destroy (eventTest->hEvent) ;
	  break ;
	}

      ofc_free (eventTest) ;
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
  OFC_EVENT_TEST *eventTest ;
  OFC_HANDLE hApp ;

  eventTest = ofc_malloc (sizeof (OFC_EVENT_TEST)) ;
  eventTest->count = 0 ;
  eventTest->state = EVENT_TEST_STATE_IDLE ;
  eventTest->scheduler = hScheduler ;

  hApp = ofc_app_create (hScheduler, &EventTestAppDef, eventTest) ;

  if (hDone != OFC_HANDLE_NULL)
    {
      ofc_app_set_wait (hApp, hDone) ;
      ofc_event_wait(hDone);
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
