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
#if !defined(__BLUE_SCHED_H__)
#define __BLUE_SCHED_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/**
 * \defgroup BlueSched Blue Scheduler Facility
 * \ingroup BlueInternal
 *
 * The Blue Scheduler facility is used to manage the real time application
 * infrastructure.  A Scheduler is a special purpose thread that services
 * applications.  Applications register events with the scheduler.  When
 * the events are ready, the scheduler will dispatch to real-time apps.
 *
 * There are two constructs to the scheduler.  The scheduler itself, and
 * the applications that are managed by the scheduler.
 * \{
 */

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * BlueSchedCreate - Create an application scheduler
   *
   * \returns
   * Handle to Scheduler
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueSchedCreate(BLUE_VOID) ;
  /**
   * Cause a scheduler to quit
   *
   * \param scheduler
   * Handle to scheduler to kill
   *
   * \returns
   * BLUE_TRUE if success, BLUE_FALSE otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueSchedQuit (BLUE_HANDLE scheduler) ;
  /**
   * Execute a preselect pass through all applications
   *
   * \param scheduler
   * Scheduler to run the preselect pass on
   *
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSchedPreselect (BLUE_HANDLE scheduler) ;
  /**
   * Execute a postselect pass through all applications
   *
   * \param scheduler
   * Scheduler to run the postselect pass on
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSchedPostselect (BLUE_HANDLE scheduler) ;
  /**
   * Wait for an event on one of the applications within this scheduler
   *
   * \param scheduler
   * Scheduler to wait for
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSchedWait (BLUE_HANDLE scheduler) ;
  /**
   * Destroy a scheduler
   *
   * \param scheduler
   * Scheduler to destroy
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSchedDestroy (BLUE_HANDLE scheduler) ;
  /**
   * Add an application to a scheduler
   *
   * \param scheduler
   * The scheduler to add the app to
   *
   * \param hApp
   * The app to add to the scheduler
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSchedAdd (BLUE_HANDLE scheduler, BLUE_HANDLE hApp) ;
  /**
   * Trigger a significant event in a schedluer
   *
   * This causes a preselect pass to be run on all applications
   *
   * \param scheduler
   * Scheduler to signal
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSchedSignificantEvent (BLUE_HANDLE scheduler) ;
  /**
   * Wake a scheduler
   *
   * \param scheduler
   * Scheduler to wake
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSchedWake (BLUE_HANDLE scheduler) ;
  /**
   * Add an event for an app to wait for
   *
   * \param hScheduler
   * Scheduler that owns the app
   *
   * \param hApp
   * App that owns the event
   *
   * \param hEvent
   * event to add to the app
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSchedAddWait (BLUE_HANDLE hScheduler, BLUE_HANDLE hApp,
		    BLUE_HANDLE hEvent) ;
  BLUE_CORE_LIB BLUE_VOID
  BlueSchedClearWait (BLUE_HANDLE hScheduler, BLUE_HANDLE hApp) ;
  /**
   * Remove an event from a scheduler
   *
   * \param hScheduler 
   * Scheduler to remove the event from
   *
   * \param hEvent
   * event to remove
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSchedRemoveWait (BLUE_HANDLE hScheduler, BLUE_HANDLE hEvent) ;

  /**
   * Test if the scheduler has any remaining apps
   *
   * \param hScheduler
   * handler to scheduler
   *
   * \returns
   * BLUE_TRUE if more apps exist, BLUE_FALSE otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueSchedEmpty (BLUE_HANDLE hScheduler) ;
#if defined(BLUE_PARAM_APP_DEBUG)
  /**
   * Dump debug info on the scheduler
   *
   * \param hScheduler
   * Scheduler to dump info on
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSchedDump (BLUE_HANDLE hScheduler) ;
#endif
  BLUE_CORE_LIB BLUE_VOID 
  BlueSchedJoin (BLUE_HANDLE hScheduler) ;

  BLUE_CORE_LIB BLUE_BOOL 
  BlueSchedKill (BLUE_HANDLE hScheduler) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueSchedKillAll (BLUE_HANDLE hScheduler) ;

  BLUE_CORE_LIB BLUE_VOID BlueSchedLogMeasure (BLUE_HANDLE hScheduler) ;

#if defined(__cplusplus)
}
#endif
/** \} */
#endif
