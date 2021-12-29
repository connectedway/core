/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#include "unity.h"
#include "unity_fixture.h"

#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/framework.h"
#include "ofc/persist.h"
#include "ofc/env.h"
#include "ofc/handle.h"
#include "ofc/app.h"
#include "ofc/sched.h"
#include "ofc/timer.h"
#include "ofc/libc.h"
#include "ofc/heap.h"
#include "ofc/event.h"

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
   TIMER_TEST_STATE_IDLE,
   TIMER_TEST_STATE_RUNNING,
  } TIMER_TEST_STATE ;

#define TIMER_TEST_INTERVAL 1000
#define TIMER_TEST_COUNT 10

typedef struct
{
  TIMER_TEST_STATE state ;
  OFC_HANDLE hTimer ;
  OFC_HANDLE scheduler ;
  OFC_INT count ;
} OFC_TIMER_TEST ;

static OFC_VOID TimerTestPreSelect (OFC_HANDLE app) ;
static OFC_HANDLE TimerTestPostSelect (OFC_HANDLE app, OFC_HANDLE hEvent) ;
static OFC_VOID TimerTestDestroy (OFC_HANDLE app) ;

static OFC_APP_TEMPLATE TimerTestAppDef =
  {
   "Timer Test Application",
   &TimerTestPreSelect,
   &TimerTestPostSelect,
   &TimerTestDestroy,
#if defined(OFC_APP_DEBUG)
   OFC_NULL
#endif
} ;

static OFC_VOID TimerTestPreSelect (OFC_HANDLE app)
{
  OFC_TIMER_TEST *timerTest ;
  TIMER_TEST_STATE entry_state ;

  timerTest = ofc_app_get_data (app) ;
  if (timerTest != OFC_NULL)
    {
      do
	{
	  entry_state = timerTest->state ;
	  ofc_sched_clear_wait (timerTest->scheduler, app) ;

	  switch (timerTest->state)
	    {
	    default:
	    case TIMER_TEST_STATE_IDLE:
	      timerTest->hTimer = ofc_timer_create("TIMER TEST") ;
	      if (timerTest->hTimer != OFC_HANDLE_NULL)
		{
		  ofc_timer_set (timerTest->hTimer, TIMER_TEST_INTERVAL) ;
		  timerTest->state = TIMER_TEST_STATE_RUNNING ;
		  ofc_sched_add_wait (timerTest->scheduler, app,
                              timerTest->hTimer) ;
		}
	      break ;

	    case TIMER_TEST_STATE_RUNNING:
	      ofc_sched_add_wait (timerTest->scheduler, app, timerTest->hTimer) ;
	      break ;
	    }
	}
      while (timerTest->state != entry_state) ;
    }
}

static OFC_HANDLE TimerTestPostSelect (OFC_HANDLE app, OFC_HANDLE hEvent)
{
  OFC_TIMER_TEST *timerTest ;
  OFC_BOOL progress ;

  timerTest = ofc_app_get_data (app) ;
  if (timerTest != OFC_NULL)
    {
      for (progress = OFC_TRUE ; progress && !ofc_app_destroying(app);)
	{
	  progress = OFC_FALSE ;

	  switch (timerTest->state)
	    {
	    default:
	    case TIMER_TEST_STATE_IDLE:
	      break ;
	  
	    case TIMER_TEST_STATE_RUNNING:
	      if (hEvent == timerTest->hTimer)
		{
		  ofc_printf ("Timer Expired %d\n", timerTest->count) ;
		  timerTest->count++ ;
		  if (timerTest->count >= TIMER_TEST_COUNT)
		    {
		      ofc_app_kill (app) ;
		    }
		  else
		    ofc_timer_set (timerTest->hTimer, TIMER_TEST_INTERVAL) ;
		}
	      break ;
	    }
	}
    }
  return (OFC_HANDLE_NULL) ;
}

static OFC_VOID TimerTestDestroy (OFC_HANDLE app)
{
  OFC_TIMER_TEST *timerTest ;

  timerTest = ofc_app_get_data (app) ;
  if (timerTest != OFC_NULL)
    {
      switch (timerTest->state)
	{
	default:
	case TIMER_TEST_STATE_IDLE:
	  break ;

	case TIMER_TEST_STATE_RUNNING:
	  ofc_timer_destroy (timerTest->hTimer) ;
	  break ;
	}

      ofc_free (timerTest) ;
    }
}

TEST_GROUP(timer);

TEST_SETUP(timer)
{
  TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(timer)
{
  test_shutdown();
}  

TEST(timer, test_timer)
{
  OFC_TIMER_TEST *timerTest ;
  OFC_HANDLE hApp ;

  timerTest = ofc_malloc (sizeof (OFC_TIMER_TEST)) ;
  timerTest->count = 0 ;
  timerTest->state = TIMER_TEST_STATE_IDLE ;
  timerTest->scheduler = hScheduler ;

  hApp = ofc_app_create (hScheduler, &TimerTestAppDef, timerTest) ;

  if (hDone != OFC_HANDLE_NULL)
    {
      ofc_app_set_wait (hApp, hDone) ;
      ofc_event_wait(hDone);
    }
}	  

TEST_GROUP_RUNNER(timer)
{
  RUN_TEST_CASE(timer, test_timer);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(timer);
}

int main(int argc, const char *argv[])
{
  return UnityMain(argc, argv, runAllTests);
}
#endif
