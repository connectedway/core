/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_SCHED_H__)
#define __OFC_SCHED_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/**
 * \defgroup sched Open Files Sheduler Facility
 *
 * The Open Files Scheduler facility is used to manage the real time application
 * infrastructure.  A Scheduler is a special purpose thread that services
 * applications.  Applications register events with the scheduler.  When
 * the events are ready, the scheduler will dispatch to real-time apps.
 *
 * There are two constructs to the scheduler.  The scheduler itself, and
 * the applications that are managed by the scheduler.
 */

/** \{ */

#if defined(__cplusplus)
extern "C"
{
#endif
/**
 * ofc_sched_create - Create an application scheduler
 *
 * \returns
 * Handle to Scheduler
 */
OFC_CORE_LIB OFC_HANDLE
ofc_sched_create(OFC_VOID);
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
ofc_sched_quit(OFC_HANDLE scheduler);
/**
 * Execute a preselect pass through all applications
 *
 * \param scheduler
 * Scheduler to run the preselect pass on
 *
 */
OFC_CORE_LIB OFC_VOID
ofc_sched_preselect(OFC_HANDLE scheduler);
/**
 * Execute a postselect pass through all applications
 *
 * \param scheduler
 * Scheduler to run the postselect pass on
 */
OFC_CORE_LIB OFC_VOID
ofc_sched_postselect(OFC_HANDLE scheduler);
/**
 * Wait for an event on one of the applications within this scheduler
 *
 * \param scheduler
 * Scheduler to wait for
 */
OFC_CORE_LIB OFC_VOID
ofc_sched_wait(OFC_HANDLE scheduler);
/**
 * Destroy a scheduler
 *
 * \param scheduler
 * Scheduler to destroy
 */
OFC_CORE_LIB OFC_VOID
ofc_sched_destroy(OFC_HANDLE scheduler);
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
ofc_sched_add(OFC_HANDLE scheduler, OFC_HANDLE hApp);
/**
 * Trigger a significant event in a schedluer
 *
 * This causes a preselect pass to be run on all applications
 *
 * \param scheduler
 * Scheduler to signal
 */
OFC_CORE_LIB OFC_VOID
ofc_sched_significant_event(OFC_HANDLE scheduler);
/**
 * Wake a scheduler
 *
 * \param scheduler
 * Scheduler to wake
 */
OFC_CORE_LIB OFC_VOID
ofc_sched_wake(OFC_HANDLE scheduler);
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
ofc_sched_add_wait(OFC_HANDLE hScheduler, OFC_HANDLE hApp,
                   OFC_HANDLE hEvent);

OFC_CORE_LIB OFC_VOID
ofc_sched_clear_wait(OFC_HANDLE hScheduler, OFC_HANDLE hApp);
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
ofc_sched_remove_wait(OFC_HANDLE hScheduler, OFC_HANDLE hEvent);

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
ofc_sched_empty(OFC_HANDLE hScheduler);

#if defined(OFC_APP_DEBUG)
/**
 * Dump debug info on the scheduler
 *
 * \param hScheduler
 * Scheduler to dump info on
 */
OFC_CORE_LIB OFC_VOID
ofc_sched_dump (OFC_HANDLE hScheduler) ;
#endif
OFC_CORE_LIB OFC_VOID
ofc_sched_join(OFC_HANDLE hScheduler);

OFC_CORE_LIB OFC_BOOL
ofc_sched_kill(OFC_HANDLE hScheduler);

OFC_CORE_LIB OFC_VOID
ofc_sched_kill_all(OFC_HANDLE hScheduler);

OFC_CORE_LIB OFC_VOID ofc_sched_log_measuer(OFC_HANDLE hScheduler);

#if defined(__cplusplus)
}
#endif
/** \} */
#endif
