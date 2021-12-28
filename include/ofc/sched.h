/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
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
  OFC_CORE_LIB OFC_HANDLE
  BlueSchedCreate(OFC_VOID) ;
  /**
   * Cause a scheduler to quit
   *
   * \param scheduler
   * Handle to scheduler to kill
   *
   * \returns
   * OFC_TRUE if success, OFC_FALSE otherwise
   */
  OFC_CORE_LIB OFC_BOOL
  BlueSchedQuit (OFC_HANDLE scheduler) ;
  /**
   * Execute a preselect pass through all applications
   *
   * \param scheduler
   * Scheduler to run the preselect pass on
   *
   */
  OFC_CORE_LIB OFC_VOID
  BlueSchedPreselect (OFC_HANDLE scheduler) ;
  /**
   * Execute a postselect pass through all applications
   *
   * \param scheduler
   * Scheduler to run the postselect pass on
   */
  OFC_CORE_LIB OFC_VOID
  BlueSchedPostselect (OFC_HANDLE scheduler) ;
  /**
   * Wait for an event on one of the applications within this scheduler
   *
   * \param scheduler
   * Scheduler to wait for
   */
  OFC_CORE_LIB OFC_VOID
  BlueSchedWait (OFC_HANDLE scheduler) ;
  /**
   * Destroy a scheduler
   *
   * \param scheduler
   * Scheduler to destroy
   */
  OFC_CORE_LIB OFC_VOID
  BlueSchedDestroy (OFC_HANDLE scheduler) ;
  /**
   * Add an application to a scheduler
   *
   * \param scheduler
   * The scheduler to add the app to
   *
   * \param hApp
   * The app to add to the scheduler
   */
  OFC_CORE_LIB OFC_VOID
  BlueSchedAdd (OFC_HANDLE scheduler, OFC_HANDLE hApp) ;
  /**
   * Trigger a significant event in a schedluer
   *
   * This causes a preselect pass to be run on all applications
   *
   * \param scheduler
   * Scheduler to signal
   */
  OFC_CORE_LIB OFC_VOID
  BlueSchedSignificantEvent (OFC_HANDLE scheduler) ;
  /**
   * Wake a scheduler
   *
   * \param scheduler
   * Scheduler to wake
   */
  OFC_CORE_LIB OFC_VOID
  BlueSchedWake (OFC_HANDLE scheduler) ;
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
  OFC_CORE_LIB OFC_VOID
  BlueSchedAddWait (OFC_HANDLE hScheduler, OFC_HANDLE hApp,
                    OFC_HANDLE hEvent) ;
  OFC_CORE_LIB OFC_VOID
  BlueSchedClearWait (OFC_HANDLE hScheduler, OFC_HANDLE hApp) ;
  /**
   * Remove an event from a scheduler
   *
   * \param hScheduler 
   * Scheduler to remove the event from
   *
   * \param hEvent
   * event to remove
   */
  OFC_CORE_LIB OFC_VOID
  BlueSchedRemoveWait (OFC_HANDLE hScheduler, OFC_HANDLE hEvent) ;

  /**
   * Test if the scheduler has any remaining apps
   *
   * \param hScheduler
   * handler to scheduler
   *
   * \returns
   * OFC_TRUE if more apps exist, OFC_FALSE otherwise
   */
  OFC_CORE_LIB OFC_BOOL
  BlueSchedEmpty (OFC_HANDLE hScheduler) ;
#if defined(OFC_APP_DEBUG)
  /**
   * Dump debug info on the scheduler
   *
   * \param hScheduler
   * Scheduler to dump info on
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSchedDump (BLUE_HANDLE hScheduler) ;
#endif
  OFC_CORE_LIB OFC_VOID
  BlueSchedJoin (OFC_HANDLE hScheduler) ;

  OFC_CORE_LIB OFC_BOOL
  BlueSchedKill (OFC_HANDLE hScheduler) ;

  OFC_CORE_LIB OFC_VOID
  BlueSchedKillAll (OFC_HANDLE hScheduler) ;

  OFC_CORE_LIB OFC_VOID BlueSchedLogMeasure (OFC_HANDLE hScheduler) ;

#if defined(__cplusplus)
}
#endif
/** \} */
#endif
