/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

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

static OFC_DWORD
BlueSchedulerLoop (BLUE_HANDLE hThread, OFC_VOID *context) ;

/*
* Define the scheduler data structure
*/
typedef struct _SCHEDULER
{
  BLUE_HANDLE applications ;	/* List of applications for this sched */
  OFC_BOOL quit ;		/* does this scheduler want to quit */
  OFC_BOOL significant_event ;	/* Is there a significant event */
  BLUE_HANDLE hEventSet ;		/* The pending read events */
  BLUE_HANDLE thread ;
  BLUE_HANDLE hTriggered ;
#if defined(OFC_HANDLE_PERF)
  BLUE_MSTIME avg_sleep ;
  BLUE_UINT32 avg_count ;
#endif
  BLUE_HANDLE hEvent ;
} SCHEDULER ;

static OFC_INT g_instance = 0 ;

/*
 * BlueSchedcreate - Create an application scheduler
 *
 * Accepts:
 *    Nothing
 *
 * Returns:
 *    Scheduler Context
 */
OFC_CORE_LIB BLUE_HANDLE
BlueSchedCreate(OFC_VOID)
{
  SCHEDULER *scheduler ;
  BLUE_HANDLE hScheduler ;
  /*
   * Try to allocate room for the scheduler
   */
  scheduler = BlueHeapMalloc (sizeof (SCHEDULER)) ;

  scheduler->quit = OFC_FALSE ;
  scheduler->significant_event = OFC_FALSE ;
  scheduler->hEvent = BLUE_HANDLE_NULL ;

  scheduler->hEventSet = BlueWaitSetCreate() ;

  scheduler->applications = BlueQcreate() ;
  scheduler->hTriggered = BLUE_HANDLE_NULL ;
#if defined(OFC_HANDLE_PERF)
  scheduler->avg_sleep = 0 ;
  scheduler->avg_count = 0 ;
#endif
  hScheduler = BlueHandleCreate (BLUE_HANDLE_SCHED, scheduler) ;

  /*
   * Create a thread for the scheduler
   */
  scheduler->thread =
    BlueThreadCreate (&BlueSchedulerLoop, 
		      BLUE_THREAD_SCHED, g_instance++,
		      (OFC_VOID *) hScheduler,
		      BLUE_THREAD_JOIN, BLUE_HANDLE_NULL) ;
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
OFC_CORE_LIB OFC_BOOL
BlueSchedQuit (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;
  OFC_BOOL ret ;

  scheduler = BlueHandleLock (hScheduler) ;
  ret = OFC_FALSE ;
  if (scheduler != OFC_NULL)
    {
      BlueThreadDelete (scheduler->thread) ;
      BlueSchedWake (hScheduler) ;
      BlueThreadWait (scheduler->thread);
      BlueHandleUnlock (hScheduler) ;
      ret = OFC_TRUE ;
    }

  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueSchedJoin (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;
  
  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != OFC_NULL)
    {
      if (scheduler->hEvent == BLUE_HANDLE_NULL)
	scheduler->hEvent = ofc_event_create (OFC_EVENT_AUTO) ;

      if (scheduler->hEvent != BLUE_HANDLE_NULL)
	{
	  BlueSchedWake (hScheduler) ;
	  ofc_event_wait (scheduler->hEvent) ;
	  ofc_event_destroy (scheduler->hEvent) ;
	}
      BlueHandleUnlock (hScheduler) ;
    }
}

OFC_CORE_LIB OFC_BOOL
BlueSchedKill (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;
  OFC_BOOL ret ;

  scheduler = BlueHandleLock (hScheduler) ;
  ret = OFC_FALSE ;
  if (scheduler != OFC_NULL)
    {
      BlueThreadDelete (scheduler->thread) ;
      BlueSchedJoin (hScheduler) ;
      BlueHandleUnlock (hScheduler) ;
      ret = OFC_TRUE ;
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
OFC_CORE_LIB OFC_VOID
BlueSchedPreselect (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;
  BLUE_HANDLE hApp ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != OFC_NULL)
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
	  if (ofc_app_destroying (hApp))
	    {
	      /*
	       * Yes, remove it from the scheduler list
	       */
	      BlueWaitSetClearApp (scheduler->hEventSet, hApp) ;
	      BlueQunlink (scheduler->applications, (OFC_VOID *) hApp) ;
	      /*
	       * Destroy the application
	       */
	      ofc_app_destroy (hApp) ;
	      /*
	       * And force another scheduler pass
	       */
	      hApp = (BLUE_HANDLE) BlueQfirst (scheduler->applications) ;
	    }
	  else
	    hApp = (BLUE_HANDLE) BlueQnext (scheduler->applications, 
					    (OFC_VOID *) hApp) ;
	}
      
      /*
       * Initialize socket event list
       */
      scheduler->significant_event = OFC_FALSE ;
      /*
       * Go through all the apps until there are no more or someone
       * set a significant event
       */
      BlueWaitSetClear (scheduler->hEventSet) ;

      for (hApp = (BLUE_HANDLE) BlueQfirst (scheduler->applications) ;
	   (hApp != BLUE_HANDLE_NULL) ; )
	{
	  ofc_app_preselect (hApp) ;
	  if (scheduler->significant_event)
	    {
	      scheduler->significant_event = OFC_FALSE ;
	      hApp = (BLUE_HANDLE) BlueQfirst (scheduler->applications) ;
	      BlueWaitSetClear (scheduler->hEventSet) ;
	    }
	  else
	    hApp = (BLUE_HANDLE) BlueQnext (scheduler->applications, 
					    (OFC_VOID *) hApp) ;
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
OFC_CORE_LIB OFC_VOID
BlueSchedPostselect (BLUE_HANDLE hScheduler)
{
  BLUE_HANDLE hApp ;
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != OFC_NULL)
    {
      /*
       * Go through applications until end or some significant event
       */
      scheduler->significant_event = OFC_FALSE ;
      
      while (scheduler->hTriggered != BLUE_HANDLE_NULL)
	{
	  hApp = BlueHandleGetApp (scheduler->hTriggered) ;
	  if (hApp != BLUE_HANDLE_NULL)
	    {
	      scheduler->hTriggered = 
		ofc_app_postselect (hApp, scheduler->hTriggered) ;
#if !defined(OFC_PRESELECT_PASS)
	      if (ofc_app_destroying (hApp))
		{
		  /*
		   * Yes, remove it from the scheduler list
		   */
		  BlueWaitSetClearApp (scheduler->hEventSet, hApp) ;
		  BlueQunlink (scheduler->applications, (OFC_VOID *) hApp) ;
		  /*
		   * Destroy the application
		   */
		  ofc_app_destroy (hApp) ;
		}
	      else
		ofc_app_preselect (hApp) ;
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
OFC_CORE_LIB OFC_VOID
BlueSchedWait (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;
#if defined(OFC_HANDLE_PERF)
  BLUE_MSTIME sleep ;
  BLUE_MSTIME slept ;
#endif

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != OFC_NULL)
    {
      scheduler->hTriggered = BLUE_HANDLE_NULL ;
      if (!scheduler->significant_event)
	{
	  /*
	   * Wait for an event (preselect set up the events to wait for)
	   */
#if defined(OFC_HANDLE_PERF)
	  sleep = BlueTimeGetNow() ;
#endif
	  scheduler->hTriggered = BlueWaitSetWait (scheduler->hEventSet) ;
#if defined(OFC_HANDLE_PERF)
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

#if defined(OFC_HANDLE_PERF)
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
OFC_CORE_LIB OFC_VOID
BlueSchedDestroy (BLUE_HANDLE hScheduler)
{
  BLUE_HANDLE hApp ;
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != OFC_NULL)
    {
      if (scheduler->hEvent != BLUE_HANDLE_NULL)
	ofc_event_set (scheduler->hEvent) ;
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
	  ofc_app_destroy (hApp) ;
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

OFC_CORE_LIB OFC_VOID
BlueSchedKillAll (BLUE_HANDLE hScheduler)
{
  BLUE_HANDLE hApp ;
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != OFC_NULL)
    {
      /*
       * Dequeue all the applications and destroy them
       */
      for (hApp = (BLUE_HANDLE) BlueQfirst (scheduler->applications) ;
	   hApp != BLUE_HANDLE_NULL ;
	   hApp = (BLUE_HANDLE) BlueQnext (scheduler->applications, 
					   (OFC_VOID *) hApp))
	{
	  /*
	   * Destroy the app
	   */
	  ofc_app_kill (hApp) ;
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
OFC_CORE_LIB OFC_VOID
BlueSchedSignificantEvent (BLUE_HANDLE hScheduler) 
{
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != OFC_NULL)
    {
      scheduler->significant_event = OFC_TRUE ;
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
OFC_CORE_LIB OFC_VOID
BlueSchedAdd (BLUE_HANDLE hScheduler, BLUE_HANDLE hApp)
{
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != OFC_NULL)
    {
      BlueQenqueue (scheduler->applications, (OFC_VOID *) hApp) ;
      BlueHandleUnlock (hScheduler) ;
    }
}

/*
 * BlueSchedWake - Wake up the scheduler
 *
 * Accepts:
 *    scheduler handle
 */
OFC_CORE_LIB OFC_VOID
BlueSchedWake (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != OFC_NULL)
    {
      BlueSchedSignificantEvent (hScheduler);
      BlueWaitSetWake (scheduler->hEventSet) ;
      BlueHandleUnlock (hScheduler) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueSchedClearWait (BLUE_HANDLE hScheduler, BLUE_HANDLE hApp)
{
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != OFC_NULL)
    {
      BlueWaitSetClearApp (scheduler->hEventSet, hApp) ;
      BlueHandleUnlock (hScheduler) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueSchedAddWait (BLUE_HANDLE hScheduler, BLUE_HANDLE hApp, BLUE_HANDLE hEvent)
{
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != OFC_NULL)
    {
      BlueWaitSetAdd (scheduler->hEventSet, hApp, hEvent) ;
      BlueHandleUnlock (hScheduler) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueSchedRemoveWait (BLUE_HANDLE hScheduler, BLUE_HANDLE hEvent)
{
  SCHEDULER *scheduler ;

  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != OFC_NULL)
    {
      BlueWaitSetRemove (scheduler->hEventSet, hEvent) ;
      BlueHandleUnlock (hScheduler) ;
    }
}

static OFC_DWORD
BlueSchedulerLoop (BLUE_HANDLE hThread, OFC_VOID *context)
{
  BLUE_HANDLE hScheduler ;
  SCHEDULER *scheduler ;

  hScheduler = (BLUE_HANDLE) context ;
  scheduler = BlueHandleLock (hScheduler) ;

  while (!BlueThreadIsDeleting (hThread))
    {
#if defined(OFC_PRESELECT_PASS)
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

  BlueSchedDestroy (hScheduler) ;
  BlueHandleUnlock (hScheduler) ;
  return (0) ;
}

OFC_CORE_LIB OFC_BOOL
BlueSchedEmpty (BLUE_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  scheduler = BlueHandleLock (hScheduler) ;
  if (scheduler != OFC_NULL)
    {
      if (BlueQempty (scheduler->applications))
	ret = OFC_TRUE ;
      BlueHandleUnlock (hScheduler) ;
    }
  return (ret) ;
}

#if defined(OFC_APP_DEBUG)
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
