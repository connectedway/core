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
    WAITQ_TEST_STATE_IDLE,
    WAITQ_TEST_STATE_RUNNING,
  } WAITQ_TEST_STATE ;

#define WAITQ_TEST_INTERVAL 2000
#define WAITQ_TEST_COUNT 5
#define WAITQ_MESSAGE "Wait Queue Test Message\n"

typedef struct
{
  WAITQ_TEST_STATE state ;
  OFC_HANDLE hTimer ;
  OFC_HANDLE hWaitQueue ;
  OFC_HANDLE scheduler ;
  OFC_INT count ;
} OFC_WAITQ_TEST ;

static OFC_VOID WaitQueueTestPreSelect (OFC_HANDLE app) ;
static OFC_HANDLE WaitQueueTestPostSelect (OFC_HANDLE app,
                                           OFC_HANDLE hWaitQueue) ;
static OFC_VOID WaitQueueTestDestroy (OFC_HANDLE app) ;

static OFC_APP_TEMPLATE WaitQueueTestAppDef =
  {
    "Wait Queue Test Application",
    &WaitQueueTestPreSelect,
    &WaitQueueTestPostSelect,
    &WaitQueueTestDestroy,
#if defined(OFC_APP_DEBUG)
    OFC_NULL
#endif
  } ;

static OFC_VOID WaitQueueTestPreSelect (OFC_HANDLE app)
{
  OFC_WAITQ_TEST *waitqTest ;
  WAITQ_TEST_STATE entry_state ;

  waitqTest = ofc_app_get_data (app) ;
  if (waitqTest != OFC_NULL)
    {
      do /* while waitqTest->state != entry_state */
	{
	  entry_state = waitqTest->state ;
	  ofc_sched_clear_wait (waitqTest->scheduler, app) ;

	  switch (waitqTest->state)
	    {
	    default:
	    case WAITQ_TEST_STATE_IDLE:
	      waitqTest->hWaitQueue = ofc_waitq_create() ;
	      if (waitqTest->hWaitQueue != OFC_HANDLE_NULL)
		{
		  waitqTest->hTimer = ofc_timer_create("WQ TEST") ;
		  if (waitqTest->hTimer != OFC_HANDLE_NULL)
		    {
		      ofc_timer_set (waitqTest->hTimer, WAITQ_TEST_INTERVAL) ;
		      waitqTest->state = WAITQ_TEST_STATE_RUNNING ;
		      ofc_sched_add_wait (waitqTest->scheduler, app,
                                  waitqTest->hTimer) ;
		    }
		}
	      break ;

	    case WAITQ_TEST_STATE_RUNNING:
	      ofc_sched_add_wait (waitqTest->scheduler, app,
                              waitqTest->hWaitQueue) ;
	      ofc_sched_add_wait (waitqTest->scheduler, app,
                              waitqTest->hTimer) ;
	      break ;
	    }
	}
      while (waitqTest->state != entry_state) ;
    }
}

static OFC_HANDLE WaitQueueTestPostSelect (OFC_HANDLE app,
                                           OFC_HANDLE hWaitQueue)
{
  OFC_WAITQ_TEST *waitqTest ;
  OFC_CHAR *msg ;
  OFC_HANDLE hNext ;
  OFC_BOOL progress ;

  hNext = OFC_HANDLE_NULL ;
  waitqTest = ofc_app_get_data (app) ;
  if (waitqTest != OFC_NULL)
    {
      for (progress = OFC_TRUE ; progress && !ofc_app_destroying(app);)
	{
	  progress = OFC_FALSE ;

	  switch (waitqTest->state)
	    {
	    default:
	    case WAITQ_TEST_STATE_IDLE:
	      break ;
	  
	    case WAITQ_TEST_STATE_RUNNING:
	      if (hWaitQueue == waitqTest->hWaitQueue)
		{
		  msg = ofc_waitq_dequeue (waitqTest->hWaitQueue) ;
		  if (msg != OFC_NULL)
		    {
		      progress = OFC_TRUE ;
		      ofc_printf (msg) ;
		      ofc_free (msg) ;
		    }
		}
	      else if (hWaitQueue == waitqTest->hTimer)
		{
		  if (waitqTest->count++ < WAITQ_TEST_COUNT)
		    {
		      msg = ofc_malloc (ofc_strlen (WAITQ_MESSAGE) + 1) ;
		      ofc_strcpy (msg, WAITQ_MESSAGE) ;
		      ofc_waitq_enqueue (waitqTest->hWaitQueue, msg) ;
		      ofc_timer_set (waitqTest->hTimer, WAITQ_TEST_INTERVAL) ;
		      ofc_printf ("Wait Queue Timer Triggered\n") ;
		      hNext = waitqTest->hWaitQueue ;
		    }
		  else
		    {
		      ofc_app_kill (app) ;
		      hNext = OFC_HANDLE_NULL ;
		    }
		}
	      break ;
	    }
	}
    }
  return (hNext) ;
}

static OFC_VOID WaitQueueTestDestroy (OFC_HANDLE app)
{
  OFC_WAITQ_TEST *waitqTest ;

  waitqTest = ofc_app_get_data (app) ;
  if (waitqTest != OFC_NULL)
    {
      switch (waitqTest->state)
	{
	default:
	case WAITQ_TEST_STATE_IDLE:
	  break ;

	case WAITQ_TEST_STATE_RUNNING:
	  ofc_timer_destroy (waitqTest->hTimer) ;
	  ofc_waitq_destroy (waitqTest->hWaitQueue) ;
	  break ;
	}
      ofc_free (waitqTest) ;
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
  OFC_WAITQ_TEST *waitqTest ;
  OFC_HANDLE hApp ;

  waitqTest = ofc_malloc (sizeof (OFC_WAITQ_TEST)) ;
  waitqTest->count = 0 ;
  waitqTest->state = WAITQ_TEST_STATE_IDLE ;
  waitqTest->scheduler = hScheduler ;

  hApp = ofc_app_create (hScheduler, &WaitQueueTestAppDef, waitqTest) ;

  if (hDone != OFC_HANDLE_NULL)
    {
      ofc_app_set_wait (hApp, hDone) ;
      ofc_event_wait(hDone);
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
