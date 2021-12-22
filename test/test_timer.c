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
   TIMER_TEST_STATE_IDLE,
   TIMER_TEST_STATE_RUNNING,
  } TIMER_TEST_STATE ;

#define TIMER_TEST_INTERVAL 1000
#define TIMER_TEST_COUNT 10

typedef struct
{
  TIMER_TEST_STATE state ;
  BLUE_HANDLE hTimer ;
  BLUE_HANDLE scheduler ;
  BLUE_INT count ;
} BLUE_TIMER_TEST ;

static BLUE_VOID TimerTestPreSelect (BLUE_HANDLE app) ;
static BLUE_HANDLE TimerTestPostSelect (BLUE_HANDLE app, BLUE_HANDLE hEvent) ;
static BLUE_VOID TimerTestDestroy (BLUE_HANDLE app) ;

static BLUEAPP_TEMPLATE TimerTestAppDef =
  {
   "Timer Test Application",
   &TimerTestPreSelect,
   &TimerTestPostSelect,
   &TimerTestDestroy,
#if defined(BLUE_PARAM_APP_DEBUG)
   BLUE_NULL
#endif
} ;

static BLUE_VOID TimerTestPreSelect (BLUE_HANDLE app)
{
  BLUE_TIMER_TEST *timerTest ;
  TIMER_TEST_STATE entry_state ;

  timerTest = BlueAppGetData (app) ;
  if (timerTest != BLUE_NULL)
    {
      do
	{
	  entry_state = timerTest->state ;
	  BlueSchedClearWait (timerTest->scheduler, app) ;

	  switch (timerTest->state)
	    {
	    default:
	    case TIMER_TEST_STATE_IDLE:
	      timerTest->hTimer = BlueTimerCreate("TIMER TEST") ;
	      if (timerTest->hTimer != BLUE_HANDLE_NULL)
		{
		  BlueTimerSet (timerTest->hTimer, TIMER_TEST_INTERVAL) ;
		  timerTest->state = TIMER_TEST_STATE_RUNNING ;
		  BlueSchedAddWait (timerTest->scheduler, app, 
				    timerTest->hTimer) ;
		}
	      break ;

	    case TIMER_TEST_STATE_RUNNING:
	      BlueSchedAddWait (timerTest->scheduler, app, timerTest->hTimer) ;
	      break ;
	    }
	}
      while (timerTest->state != entry_state) ;
    }
}

static BLUE_HANDLE TimerTestPostSelect (BLUE_HANDLE app, BLUE_HANDLE hEvent) 
{
  BLUE_TIMER_TEST *timerTest ;
  BLUE_BOOL progress ;

  timerTest = BlueAppGetData (app) ;
  if (timerTest != BLUE_NULL)
    {
      for (progress = BLUE_TRUE ; progress && !BlueAppDestroying(app);)
	{
	  progress = BLUE_FALSE ;

	  switch (timerTest->state)
	    {
	    default:
	    case TIMER_TEST_STATE_IDLE:
	      break ;
	  
	    case TIMER_TEST_STATE_RUNNING:
	      if (hEvent == timerTest->hTimer)
		{
		  BlueCprintf ("Timer Expired %d\n", timerTest->count) ;
		  timerTest->count++ ;
		  if (timerTest->count >= TIMER_TEST_COUNT)
		    {
		      BlueAppKill (app) ;
		    }
		  else
		    BlueTimerSet (timerTest->hTimer, TIMER_TEST_INTERVAL) ;
		}
	      break ;
	    }
	}
    }
  return (BLUE_HANDLE_NULL) ;
}

static BLUE_VOID TimerTestDestroy (BLUE_HANDLE app) 
{
  BLUE_TIMER_TEST *timerTest ;

  timerTest = BlueAppGetData (app) ;
  if (timerTest != BLUE_NULL)
    {
      switch (timerTest->state)
	{
	default:
	case TIMER_TEST_STATE_IDLE:
	  break ;

	case TIMER_TEST_STATE_RUNNING:
	  BlueTimerDestroy (timerTest->hTimer) ;
	  break ;
	}

      BlueHeapFree (timerTest) ;
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
  BLUE_TIMER_TEST *timerTest ;
  BLUE_HANDLE hApp ;

  timerTest = BlueHeapMalloc (sizeof (BLUE_TIMER_TEST)) ;
  timerTest->count = 0 ;
  timerTest->state = TIMER_TEST_STATE_IDLE ;
  timerTest->scheduler = hScheduler ;

  hApp = BlueAppCreate (hScheduler, &TimerTestAppDef, timerTest) ;

  if (hDone != BLUE_HANDLE_NULL)
    {
      BlueAppSetWait (hApp, hDone) ;
      BlueEventWait(hDone);
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
