/* Copyright (c) 2009 Blue Peach Solutions, Inc. 
 * All rights reserved.
 *
 * This software is protected by copyright and intellectual 
 * property laws as well as international treaties.  It is to be 
 * used and copied only by authorized licensees under the 
 * conditions described in their licenses.  
 *
 * Title to and ownership of the software shall at all times
 * remain with Blue Peach Solutions.
 */
#define __BLUE_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/queue.h"
#include "ofc/handle.h"
#include "ofc/libc.h"
#include "ofc/thread.h"
#include "ofc/waitset.h"
#include "ofc/app.h"
#include "ofc/sched.h"
#include "ofc/event.h"
#include "ofc/time.h"

#include "ofc/heap.h"

static BLUE_DWORD 
BlueSchedulerLoop (BLUE_HANDLE hThread, BLUE_VOID *context) ;

/*
* Define the scheduler data structure
*/
typedef struct _SCHEDULER
{
  BLUE_HANDLE applications ;	/* List of applications for this sched */
  BLUE_BOOL quit ;		/* does this scheduler want to quit */
  BLUE_BOOL significant_event ;	/* Is there a significant event */
  BLUE_HANDLE hEventSet ;		/* The pending read events */
  BLUE_HANDLE thread ;
  BLUE_HANDLE hTriggered ;
#if defined(BLUE_PARAM_HANDLE_PERF)
  BLUE_MSTIME avg_sleep ;
  BLUE_UINT32 avg_count ;
#endif
#if !defined(SHUTDOWN)
  BLUE_HANDLE hEvent ;
#endif
} SCHEDULER ;

static BLUE_INT g_instance = 0 ;

/*
 * BlueSchedcreate - Create an application scheduler
 *
 * Accepts:
 *    Nothing
 *
 * Returns:
 *    Scheduler Context
 */
BLUE_CORE_LIB BLUE_HANDLE 
BlueSchedCreate(BLUE_VOID)
{
  SCHEDULER *scheduler ;
  BLUE_HANDLE hScheduler ;
  /*
   * Try to allocate room for the scheduler
   */
  scheduler = BlueHeapMalloc (sizeof (SCHEDULER)) ;

  scheduler->quit = BLUE_FALSE ;
  scheduler->significant_event = BLUE_FALSE ;
#if !defined(SHUTDOWN)
  scheduler->hEvent = BLUE_HANDLE_NULL ;
#endif

  scheduler->hEventSet = BlueWaitSetCreate() ;

  scheduler->applications = BlueQcreate() ;
  scheduler->hTriggered = BLUE_HANDLE_NULL ;
#if defined(BLUE_PARAM_HANDLE_PERF)
  scheduler->avg_sleep = 0 ;
  scheduler->avg_count = 0 ;
#endif
  hScheduler = BlueHandleCreate (BLUE_HANDLE_SCHED, scheduler) ;

  /*
   * Create a thread for the scheduler
   */
#if defined(SHUTDOWN)
  scheduler->thread =
    BlueThreadCreate (&BlueSchedulerLoop, 
		      BLUE_THREAD_SCHED, g_instance++,
		      (BLUE_VOID *) hScheduler,
		      BLUE_THREAD_JOIN, BLUE_HANDLE_NULL) ;
#else
  scheduler->thread =
    BlueThreadCreate (&BlueSchedulerLoop, 
		      BLUE_THREAD_SCHED, g_instance++,
		      (BLUE_VOID *) hScheduler,
		      BLUE_THREAD_JOIN, BLUE_HANDLE_NULL) ;
#endif
  return (hScheduler) ;
}

/*
 * BlueSchedquit - See if the scheduler is supposed to quit
 *
 * Accepts:
 *    Scheduler to check for
 *
 * Returns:
 *    TRUE if we need to quit, FALSE otherwise
 */
BLUE_CORE_LIB BLUE_BOOL 
BlueSchedQuit (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;
  BLUE_BOOL ret ;

  scheduler = BlueHandleLock (hScheduler) ;
  ret = BLUE_FALSE ;
  if (scheduler != BLUE_NULL)
    {
      BlueThreadDelete (scheduler->thread) ;
      BlueSchedWake (hScheduler) ;
      BlueThreadWait (scheduler->thread);
      BlueHandleUnlock (hScheduler) ;
      ret = BLUE_TRUE ;
    }

  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueSchedJoin (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;
  
  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != BLUE_NULL)
    {
#if !defined(SHUTDOWN)
      if (scheduler->hEvent == BLUE_HANDLE_NULL)
	scheduler->hEvent = BlueEventCreate (BLUE_EVENT_AUTO) ;

      if (scheduler->hEvent != BLUE_HANDLE_NULL)
	{
	  BlueSchedWake (hScheduler) ;
	  BlueEventWait (scheduler->hEvent) ;
	  BlueEventDestroy (scheduler->hEvent) ;
	}
#else
      BlueThreadWait (scheduler->thread) ;
#endif
      BlueHandleUnlock (hScheduler) ;
    }
}

BLUE_CORE_LIB BLUE_BOOL 
BlueSchedKill (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;
  BLUE_BOOL ret ;

  scheduler = BlueHandleLock (hScheduler) ;
  ret = BLUE_FALSE ;
  if (scheduler != BLUE_NULL)
    {
      BlueThreadDelete (scheduler->thread) ;
      BlueSchedJoin (hScheduler) ;
      BlueHandleUnlock (hScheduler) ;
      ret = BLUE_TRUE ;
    }

  return (ret) ;
}

/*
 * BlueSchedPreselect - Preselect all the applications
 *
 * Accepts:
 *    Scheduler
 *
 * Returns:
 *    Nothing
 *
 * This will also clean up applications being deleted
 */
BLUE_CORE_LIB BLUE_VOID 
BlueSchedPreselect (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;
  BLUE_HANDLE hApp ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != BLUE_NULL)
    {
      /*
       * Scheduler loop.
       */
      BlueWaitSetClear (scheduler->hEventSet) ;
      /*
       * Go through all the apps until there are no more or someone
       * set a significant event
       */
      for (hApp = (BLUE_HANDLE) BlueQfirst (scheduler->applications) ;
	   (hApp != BLUE_HANDLE_NULL) ; )
	{
	  /*
	   * See if we want to destroy this app
	   */
	  if (BlueAppDestroying (hApp))
	    {
	      /*
	       * Yes, remove it from the scheduler list
	       */
	      BlueWaitSetClearApp (scheduler->hEventSet, hApp) ;
	      BlueQunlink (scheduler->applications, (BLUE_VOID *) hApp) ;
	      /*
	       * Destroy the application
	       */
	      BlueAppDestroy (hApp) ;
	      /*
	       * And force another scheduler pass
	       */
	      hApp = (BLUE_HANDLE) BlueQfirst (scheduler->applications) ;
	    }
	  else
	    hApp = (BLUE_HANDLE) BlueQnext (scheduler->applications, 
					    (BLUE_VOID *) hApp) ;
	}
      
      /*
       * Initialize socket event list
       */
      scheduler->significant_event = BLUE_FALSE ;
      /*
       * Go through all the apps until there are no more or someone
       * set a significant event
       */
      BlueWaitSetClear (scheduler->hEventSet) ;

      for (hApp = (BLUE_HANDLE) BlueQfirst (scheduler->applications) ;
	   (hApp != BLUE_HANDLE_NULL) ; )
	{
	  BlueAppPreselect (hApp) ;
	  if (scheduler->significant_event)
	    {
	      scheduler->significant_event = BLUE_FALSE ;
	      hApp = (BLUE_HANDLE) BlueQfirst (scheduler->applications) ;
	      BlueWaitSetClear (scheduler->hEventSet) ;
	    }
	  else
	    hApp = (BLUE_HANDLE) BlueQnext (scheduler->applications, 
					    (BLUE_VOID *) hApp) ;
	}
      BlueHandleUnlock (hScheduler) ;
    }
}

/*
 * BlueSchedPostselect - Process any triggered events
 *
 * Accepts:
 *    Scheduler
 *
 * Returns:
 *    Nothing
 */
BLUE_CORE_LIB BLUE_VOID 
BlueSchedPostselect (BLUE_HANDLE hScheduler)
{
  BLUE_HANDLE hApp ;
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != BLUE_NULL)
    {
      /*
       * Go through applications until end or some significant event
       */
      scheduler->significant_event = BLUE_FALSE ;
      
      while (scheduler->hTriggered != BLUE_HANDLE_NULL)
	{
	  hApp = BlueHandleGetApp (scheduler->hTriggered) ;
	  if (hApp != BLUE_HANDLE_NULL)
	    {
	      scheduler->hTriggered = 
		BlueAppPostselect (hApp, scheduler->hTriggered) ;
#if !defined(BLUE_PARAM_PRESELECT_PASS)
	      if (BlueAppDestroying (hApp))
		{
		  /*
		   * Yes, remove it from the scheduler list
		   */
		  BlueWaitSetClearApp (scheduler->hEventSet, hApp) ;
		  BlueQunlink (scheduler->applications, (BLUE_VOID *) hApp) ;
		  /*
		   * Destroy the application
		   */
		  BlueAppDestroy (hApp) ;
		}
	      else
		BlueAppPreselect (hApp) ;
#endif
	    }
	  else
	    /*
	     * No one waiting on that handle yet, so can't schedule it.
	     */
	    scheduler->hTriggered = BLUE_HANDLE_NULL ;
	}
      BlueHandleUnlock (hScheduler) ;
    }
}

/*
 * BlueSchedWait - Wait for any pending events to trigger
 *
 * Accepts:
 *    scheduler to wait for
 */
BLUE_CORE_LIB BLUE_VOID 
BlueSchedWait (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;
#if defined(BLUE_PARAM_HANDLE_PERF)
  BLUE_MSTIME sleep ;
  BLUE_MSTIME slept ;
#endif

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != BLUE_NULL)
    {
      scheduler->hTriggered = BLUE_HANDLE_NULL ;
      if (!scheduler->significant_event)
	{
	  /*
	   * Wait for an event (preselect set up the events to wait for)
	   */
#if defined(BLUE_PARAM_HANDLE_PERF)
	  sleep = BlueTimeGetNow() ;
#endif
	  scheduler->hTriggered = BlueWaitSetWait (scheduler->hEventSet) ;
#if defined(BLUE_PARAM_HANDLE_PERF)
	  slept = BlueTimeGetNow() - sleep ;
	  if (slept > scheduler->avg_sleep)
	    scheduler->avg_sleep += 
	      ((slept - scheduler->avg_sleep) /
	       (scheduler->avg_count + 1)) ;
	  else
	    scheduler->avg_sleep -= 
	      ((scheduler->avg_sleep - slept) /
	       (scheduler->avg_count + 1)) ;
	  scheduler->avg_count++ ;
	  BlueHandleMeasure (scheduler->hTriggered) ;
#endif
	}
      BlueHandleUnlock (hScheduler) ;
    }
}

#if defined(BLUE_PARAM_HANDLE_PERF)
BLUE_VOID BlueSchedLogMeasure (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != BLUE_NULL)
    {
      BlueCprintf ("Average Sleep Time: %lu ms\n", scheduler->avg_sleep) ;

      BlueWaitSetLogMeasure (scheduler->hEventSet) ;

      scheduler->avg_sleep = 0 ;
      scheduler->avg_count = 0 ;
    }
}
#endif

/*
 * BlueSchedDestroy - Destroy a scheduler
 *
 * Accepts:
 *    scheduler to destroy
 *
 * This doesn't currently get called
 */
BLUE_CORE_LIB BLUE_VOID 
BlueSchedDestroy (BLUE_HANDLE hScheduler)
{
  BLUE_HANDLE hApp ;
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != BLUE_NULL)
    {
#if !defined(SHUTDOWN)
      if (scheduler->hEvent != BLUE_HANDLE_NULL)
	BlueEventSet (scheduler->hEvent) ;
#endif
      /*
       * Dequeue all the applications and destroy them
       */
      for (hApp = (BLUE_HANDLE) BlueQdequeue (scheduler->applications) ;
	   hApp != BLUE_HANDLE_NULL ;
	   hApp = (BLUE_HANDLE) BlueQdequeue (scheduler->applications))
	{
	  /*
	   * Destroy the app
	   */
	  BlueWaitSetClearApp (scheduler->hEventSet, hApp) ;
	  BlueAppDestroy (hApp) ;
	}
      /*
       * Get rid of the applications list
       */
      BlueQdestroy (scheduler->applications) ;
      /*
       * And get rid of the scheduler
       */
      BlueWaitSetDestroy (scheduler->hEventSet) ;
      BlueHeapFree (scheduler) ;
      BlueHandleDestroy (hScheduler) ;
      BlueHandleUnlock (hScheduler) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueSchedKillAll (BLUE_HANDLE hScheduler)
{
  BLUE_HANDLE hApp ;
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != BLUE_NULL)
    {
      /*
       * Dequeue all the applications and destroy them
       */
      for (hApp = (BLUE_HANDLE) BlueQfirst (scheduler->applications) ;
	   hApp != BLUE_HANDLE_NULL ;
	   hApp = (BLUE_HANDLE) BlueQnext (scheduler->applications, 
					   (BLUE_VOID *) hApp))
	{
	  /*
	   * Destroy the app
	   */
	  BlueAppKill (hApp) ;
	}
    }
}

/*
 * BlueSchedSignificantEvent - Schedule a significant event
 *
 * Accepts:
 *    Scheduler Handle
 *
 * Returns:
 *    nothing
 */
BLUE_CORE_LIB BLUE_VOID 
BlueSchedSignificantEvent (BLUE_HANDLE hScheduler) 
{
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != BLUE_NULL)
    {
      scheduler->significant_event = BLUE_TRUE ;
      BlueHandleUnlock (hScheduler) ;
    }
}

/*
 * BlueSchedAdd - Add an application to the scheduler list
 *
 * Accepts:
 *    hScheduler - The scheduler handle
 *    hState - The application handle
 *
 * Returns:
 *    Nothing
 */
BLUE_CORE_LIB BLUE_VOID 
BlueSchedAdd (BLUE_HANDLE hScheduler, BLUE_HANDLE hApp)
{
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != BLUE_NULL)
    {
      BlueQenqueue (scheduler->applications, (BLUE_VOID *) hApp) ;
      BlueHandleUnlock (hScheduler) ;
    }
}

/*
 * BlueSchedWake - Wake up the scheduler
 *
 * Accepts:
 *    scheduler handle
 */
BLUE_CORE_LIB BLUE_VOID 
BlueSchedWake (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != BLUE_NULL)
    {
      BlueSchedSignificantEvent (hScheduler);
      BlueWaitSetWake (scheduler->hEventSet) ;
      BlueHandleUnlock (hScheduler) ;
    }
}

BLUE_CORE_LIB BLUE_VOID
BlueSchedClearWait (BLUE_HANDLE hScheduler, BLUE_HANDLE hApp)
{
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != BLUE_NULL)
    {
      BlueWaitSetClearApp (scheduler->hEventSet, hApp) ;
      BlueHandleUnlock (hScheduler) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueSchedAddWait (BLUE_HANDLE hScheduler, BLUE_HANDLE hApp, BLUE_HANDLE hEvent)
{
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != BLUE_NULL)
    {
      BlueWaitSetAdd (scheduler->hEventSet, hApp, hEvent) ;
      BlueHandleUnlock (hScheduler) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueSchedRemoveWait (BLUE_HANDLE hScheduler, BLUE_HANDLE hEvent)
{
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != BLUE_NULL)
    {
      BlueWaitSetRemove (scheduler->hEventSet, hEvent) ;
      BlueHandleUnlock (hScheduler) ;
    }
}

static BLUE_DWORD 
BlueSchedulerLoop (BLUE_HANDLE hThread, BLUE_VOID *context) 
{
  BLUE_HANDLE hScheduler ;
  SCHEDULER *scheduler ;

  hScheduler = (BLUE_HANDLE) context ;
  scheduler = BlueHandleLock (hScheduler) ;

  while (!BlueThreadIsDeleting (hThread))
    {
#if defined(SHUTDOWN)
      /*
       * If we're shutting down, we want to kill all our apps and
       * schedule us for deletion.  The Preslect routine will
       * do the actual cleanup of the apps
       */
      if (BlueGetShutdown())
	{
	  BlueSchedKillAll (hScheduler) ;
	  BlueSchedPreselect (hScheduler) ;
	  BlueThreadDelete (hThread) ;
	}
      else
	{
#if defined(BLUE_PARAM_PRESELECT_PASS)
	  /*
	   * Do a preselect path
	   */
	  BlueSchedPreselect (hScheduler) ;
#else
	  if (scheduler->significant_event)
	    BlueSchedPreselect (hScheduler) ;
#endif
	  /*
	   * Wait for some significant event
	   */
	  BlueSchedWait (hScheduler) ;
	  /*
	   * And do a post select path
	   */
	  BlueSchedPostselect (hScheduler) ;
	}
#else
#if defined(BLUE_PARAM_PRESELECT_PASS)
      /*
       * Do a preselect path
       */
      BlueSchedPreselect (hScheduler) ;
#else
      if (scheduler->significant_event)
	BlueSchedPreselect (hScheduler) ;
#endif
      /*
       * Wait for some significant event
       */
      BlueSchedWait (hScheduler) ;
      /*
       * And do a post select path
       */
      BlueSchedPostselect (hScheduler) ;
#endif
    }

#if !defined(SHUTDOWN)
  BlueSchedDestroy (hScheduler) ;
#endif
  BlueHandleUnlock (hScheduler) ;
  return (0) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueSchedEmpty (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != BLUE_NULL)
    {
      if (BlueQempty (scheduler->applications))
	ret = BLUE_TRUE ;
      BlueHandleUnlock (hScheduler) ;
    }
  return (ret) ;
}

#if defined(BLUE_PARAM_APP_DEBUG)
BLUE_CORE_LIB BLUE_VOID 
BlueSchedDump (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;
  BLUE_HANDLE hApp ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != BLUE_NULL)
    {
      BlueCprintf ("%-20s: %s\n", "Scheduled for Quit", 
		   scheduler->quit ? "yes" : "no") ;
      BlueCprintf ("%-20s: %s\n", "Significant Event", 
		   scheduler->quit ? "yes" : "no") ;

      /*
       * Go through all the apps until there are no more or someone
       */
      for (hApp = (BLUE_HANDLE) BlueQfirst (scheduler->applications) ;
	   hApp != BLUE_HANDLE_NULL ; 
	   hApp = (BLUE_HANDLE) BlueQnext (scheduler->applications, 
					   (BLUE_VOID *) hApp))

	{
	  BlueAppDump (hApp) ;
	}
      BlueHandleUnlock (hScheduler) ;
    }
}
#endif
