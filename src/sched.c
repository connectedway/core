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
ofc_scheduler_loop(OFC_HANDLE hThread, OFC_VOID *context);

/*
* Define the scheduler data structure
*/
typedef struct _SCHEDULER {
    OFC_HANDLE applications;    /* List of applications for this sched */
    OFC_BOOL quit;        /* does this scheduler want to quit */
    OFC_BOOL significant_event;    /* Is there a significant event */
    OFC_HANDLE hEventSet;        /* The pending read events */
    OFC_HANDLE thread;
    OFC_HANDLE hTriggered;
#if defined(OFC_HANDLE_PERF)
    OFC_MSTIME avg_sleep ;
    OFC_UINT32 avg_count ;
#endif
    OFC_HANDLE hEvent;
} SCHEDULER;

static OFC_INT g_instance = 0;

/*
 * ofc_sched_create - Create an application scheduler
 *
 * Accepts:
 *    Nothing
 *
 * Returns:
 *    Scheduler Context
 */
OFC_CORE_LIB OFC_HANDLE
ofc_sched_create(OFC_VOID) {
    SCHEDULER *scheduler;
    OFC_HANDLE hScheduler;
    /*
     * Try to allocate room for the scheduler
     */
    scheduler = ofc_malloc(sizeof(SCHEDULER));

    scheduler->quit = OFC_FALSE;
    scheduler->significant_event = OFC_FALSE;
    scheduler->hEvent = OFC_HANDLE_NULL;

    scheduler->hEventSet = ofc_waitset_create();

    scheduler->applications = ofc_queue_create();
    scheduler->hTriggered = OFC_HANDLE_NULL;
#if defined(OFC_HANDLE_PERF)
    scheduler->avg_sleep = 0 ;
    scheduler->avg_count = 0 ;
#endif
    hScheduler = ofc_handle_create(OFC_HANDLE_SCHED, scheduler);

    /*
     * Create a thread for the scheduler
     */
    scheduler->thread =
            ofc_thread_create(&ofc_scheduler_loop,
                              OFC_THREAD_SCHED, g_instance++,
                              (OFC_VOID *) hScheduler,
                              OFC_THREAD_JOIN, OFC_HANDLE_NULL);
    return (hScheduler);
}

/*
 * ofc_sched_quit - See if the scheduler is supposed to quit
 *
 * Accepts:
 *    Scheduler to check for
 *
 * Returns:
 *    TRUE if we need to quit, FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_sched_quit(OFC_HANDLE hScheduler) {
    SCHEDULER *scheduler;
    OFC_BOOL ret;

    scheduler = ofc_handle_lock(hScheduler);
    ret = OFC_FALSE;
    if (scheduler != OFC_NULL) {
        ofc_thread_delete(scheduler->thread);
        ofc_sched_wake(hScheduler);
        ofc_thread_wait(scheduler->thread);
        ofc_handle_unlock(hScheduler);
        ret = OFC_TRUE;
    }

    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_sched_join(OFC_HANDLE hScheduler) {
    SCHEDULER *scheduler;

    scheduler = ofc_handle_lock(hScheduler);
    if (scheduler != OFC_NULL) {
        if (scheduler->hEvent == OFC_HANDLE_NULL)
            scheduler->hEvent = ofc_event_create(OFC_EVENT_AUTO);

        if (scheduler->hEvent != OFC_HANDLE_NULL) {
            ofc_sched_wake(hScheduler);
            ofc_event_wait(scheduler->hEvent);
            ofc_event_destroy(scheduler->hEvent);
        }
        ofc_handle_unlock(hScheduler);
    }
}

OFC_CORE_LIB OFC_BOOL
ofc_sched_kill(OFC_HANDLE hScheduler) {
    SCHEDULER *scheduler;
    OFC_BOOL ret;

    scheduler = ofc_handle_lock(hScheduler);
    ret = OFC_FALSE;
    if (scheduler != OFC_NULL) {
        ofc_thread_delete(scheduler->thread);
        ofc_sched_join(hScheduler);
        ofc_handle_unlock(hScheduler);
        ret = OFC_TRUE;
    }

    return (ret);
}

/*
 * ofc_sched_preselect - Preselect all the applications
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
ofc_sched_preselect(OFC_HANDLE hScheduler) {
    SCHEDULER *scheduler;
    OFC_HANDLE hApp;

    scheduler = ofc_handle_lock(hScheduler);
    if (scheduler != OFC_NULL) {
        /*
         * Scheduler loop.
         */
        ofc_waitset_clear(scheduler->hEventSet);
        /*
         * Go through all the apps until there are no more or someone
         * set a significant event
         */
        for (hApp = (OFC_HANDLE) ofc_queue_first(scheduler->applications);
             (hApp != OFC_HANDLE_NULL);) {
            /*
             * See if we want to destroy this app
             */
            if (ofc_app_destroying(hApp)) {
                /*
                 * Yes, remove it from the scheduler list
                 */
                ofc_waitset_clear_app(scheduler->hEventSet, hApp);
                ofc_queue_unlink(scheduler->applications, (OFC_VOID *) hApp);
                /*
                 * Destroy the application
                 */
                ofc_app_destroy(hApp);
                /*
                 * And force another scheduler pass
                 */
                hApp = (OFC_HANDLE) ofc_queue_first(scheduler->applications);
            } else
                hApp = (OFC_HANDLE) ofc_queue_next(scheduler->applications,
                                                   (OFC_VOID *) hApp);
        }

        /*
         * Initialize socket event list
         */
        scheduler->significant_event = OFC_FALSE;
        /*
         * Go through all the apps until there are no more or someone
         * set a significant event
         */
        ofc_waitset_clear(scheduler->hEventSet);

        for (hApp = (OFC_HANDLE) ofc_queue_first(scheduler->applications);
             (hApp != OFC_HANDLE_NULL);) {
            ofc_app_preselect(hApp);
            if (scheduler->significant_event) {
                scheduler->significant_event = OFC_FALSE;
                hApp = (OFC_HANDLE) ofc_queue_first(scheduler->applications);
                ofc_waitset_clear(scheduler->hEventSet);
            } else
                hApp = (OFC_HANDLE) ofc_queue_next(scheduler->applications,
                                                   (OFC_VOID *) hApp);
        }
        ofc_handle_unlock(hScheduler);
    }
}

/*
 * ofc_sched_postselect - Process any triggered events
 *
 * Accepts:
 *    Scheduler
 *
 * Returns:
 *    Nothing
 */
OFC_CORE_LIB OFC_VOID
ofc_sched_postselect(OFC_HANDLE hScheduler) {
    OFC_HANDLE hApp;
    SCHEDULER *scheduler;

    scheduler = ofc_handle_lock(hScheduler);
    if (scheduler != OFC_NULL) {
        /*
         * Go through applications until end or some significant event
         */
        scheduler->significant_event = OFC_FALSE;

        while (scheduler->hTriggered != OFC_HANDLE_NULL) {
            hApp = ofc_handle_get_app(scheduler->hTriggered);
            if (hApp != OFC_HANDLE_NULL) {
                scheduler->hTriggered =
                        ofc_app_postselect(hApp, scheduler->hTriggered);
#if !defined(OFC_PRESELECT_PASS)
                if (ofc_app_destroying(hApp)) {
                    /*
                     * Yes, remove it from the scheduler list
                     */
                    ofc_waitset_clear_app(scheduler->hEventSet, hApp);
                    ofc_queue_unlink(scheduler->applications, (OFC_VOID *) hApp);
                    /*
                     * Destroy the application
                     */
                    ofc_app_destroy(hApp);
                } else
                    ofc_app_preselect(hApp);
#endif
            } else
                /*
                 * No one waiting on that handle yet, so can't schedule it.
                 */
                scheduler->hTriggered = OFC_HANDLE_NULL;
        }
        ofc_handle_unlock(hScheduler);
    }
}

/*
 * ofc_sched_wait - Wait for any pending events to trigger
 *
 * Accepts:
 *    scheduler to wait for
 */
OFC_CORE_LIB OFC_VOID
ofc_sched_wait(OFC_HANDLE hScheduler) {
    SCHEDULER *scheduler;
#if defined(OFC_HANDLE_PERF)
    OFC_MSTIME sleep ;
    OFC_MSTIME slept ;
#endif

    scheduler = ofc_handle_lock(hScheduler);
    if (scheduler != OFC_NULL) {
        scheduler->hTriggered = OFC_HANDLE_NULL;
        if (!scheduler->significant_event) {
            /*
             * Wait for an event (preselect set up the events to wait for)
             */
#if defined(OFC_HANDLE_PERF)
            sleep = ofc_time_get_now() ;
#endif
            scheduler->hTriggered = ofc_waitset_wait(scheduler->hEventSet);
#if defined(OFC_HANDLE_PERF)
            slept = ofc_time_get_now() - sleep ;
            if (slept > scheduler->avg_sleep)
              scheduler->avg_sleep +=
                ((slept - scheduler->avg_sleep) /
                 (scheduler->avg_count + 1)) ;
            else
              scheduler->avg_sleep -=
                ((scheduler->avg_sleep - slept) /
                 (scheduler->avg_count + 1)) ;
            scheduler->avg_count++ ;
            ofc_handle_measure (scheduler->hTriggered) ;
#endif
        }
        ofc_handle_unlock(hScheduler);
    }
}

#if defined(OFC_HANDLE_PERF)
OFC_VOID ofc_sched_log_measure (OFC_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;

  scheduler = ofc_handle_lock (hScheduler) ;
  if (scheduler != OFC_NULL)
    {
      ofc_printf ("Average Sleep Time: %lu ms\n", scheduler->avg_sleep) ;

      ofc_waitset_log_measure (scheduler->hEventSet) ;

      scheduler->avg_sleep = 0 ;
      scheduler->avg_count = 0 ;
    }
}
#endif

/*
 * ofc_sched_destroy - Destroy a scheduler
 *
 * Accepts:
 *    scheduler to destroy
 *
 * This doesn't currently get called
 */
OFC_CORE_LIB OFC_VOID
ofc_sched_destroy(OFC_HANDLE hScheduler) {
    OFC_HANDLE hApp;
    SCHEDULER *scheduler;

    scheduler = ofc_handle_lock(hScheduler);
    if (scheduler != OFC_NULL) {
        if (scheduler->hEvent != OFC_HANDLE_NULL)
            ofc_event_set(scheduler->hEvent);
        /*
         * Dequeue all the applications and destroy them
         */
        for (hApp = (OFC_HANDLE) ofc_dequeue(scheduler->applications);
             hApp != OFC_HANDLE_NULL;
             hApp = (OFC_HANDLE) ofc_dequeue(scheduler->applications)) {
            /*
             * Destroy the app
             */
            ofc_waitset_clear_app(scheduler->hEventSet, hApp);
            ofc_app_destroy(hApp);
        }
        /*
         * Get rid of the applications list
         */
        ofc_queue_destroy(scheduler->applications);
        /*
         * And get rid of the scheduler
         */
        ofc_waitset_destroy(scheduler->hEventSet);
        ofc_free(scheduler);
        ofc_handle_destroy(hScheduler);
        ofc_handle_unlock(hScheduler);
    }
}

OFC_CORE_LIB OFC_VOID
ofc_sched_kill_all(OFC_HANDLE hScheduler) {
    OFC_HANDLE hApp;
    SCHEDULER *scheduler;

    scheduler = ofc_handle_lock(hScheduler);
    if (scheduler != OFC_NULL) {
        /*
         * Dequeue all the applications and destroy them
         */
        for (hApp = (OFC_HANDLE) ofc_queue_first(scheduler->applications);
             hApp != OFC_HANDLE_NULL;
             hApp = (OFC_HANDLE) ofc_queue_next(scheduler->applications,
                                                (OFC_VOID *) hApp)) {
            /*
             * Destroy the app
             */
            ofc_app_kill(hApp);
        }
    }
}

/*
 * ofc_sched_significant_event - Schedule a significant event
 *
 * Accepts:
 *    Scheduler Handle
 *
 * Returns:
 *    nothing
 */
OFC_CORE_LIB OFC_VOID
ofc_sched_significant_event(OFC_HANDLE hScheduler) {
    SCHEDULER *scheduler;

    scheduler = ofc_handle_lock(hScheduler);
    if (scheduler != OFC_NULL) {
        scheduler->significant_event = OFC_TRUE;
        ofc_handle_unlock(hScheduler);
    }
}

/*
 * ofc_sched_add - Add an application to the scheduler list
 *
 * Accepts:
 *    hScheduler - The scheduler handle
 *    hState - The application handle
 *
 * Returns:
 *    Nothing
 */
OFC_CORE_LIB OFC_VOID
ofc_sched_add(OFC_HANDLE hScheduler, OFC_HANDLE hApp) {
    SCHEDULER *scheduler;

    scheduler = ofc_handle_lock(hScheduler);
    if (scheduler != OFC_NULL) {
        ofc_enqueue(scheduler->applications, (OFC_VOID *) hApp);
        ofc_handle_unlock(hScheduler);
    }
}

/*
 * ofc_sched_wake - Wake up the scheduler
 *
 * Accepts:
 *    scheduler handle
 */
OFC_CORE_LIB OFC_VOID
ofc_sched_wake(OFC_HANDLE hScheduler) {
    SCHEDULER *scheduler;

    scheduler = ofc_handle_lock(hScheduler);
    if (scheduler != OFC_NULL) {
        ofc_sched_significant_event(hScheduler);
        ofc_waitset_wake(scheduler->hEventSet);
        ofc_handle_unlock(hScheduler);
    }
}

OFC_CORE_LIB OFC_VOID
ofc_sched_clear_wait(OFC_HANDLE hScheduler, OFC_HANDLE hApp) {
    SCHEDULER *scheduler;

    scheduler = ofc_handle_lock(hScheduler);
    if (scheduler != OFC_NULL) {
        ofc_waitset_clear_app(scheduler->hEventSet, hApp);
        ofc_handle_unlock(hScheduler);
    }
}

OFC_CORE_LIB OFC_VOID
ofc_sched_add_wait(OFC_HANDLE hScheduler, OFC_HANDLE hApp, OFC_HANDLE hEvent) {
    SCHEDULER *scheduler;

    scheduler = ofc_handle_lock(hScheduler);
    if (scheduler != OFC_NULL) {
        ofc_waitset_add(scheduler->hEventSet, hApp, hEvent);
        ofc_handle_unlock(hScheduler);
    }
}

OFC_CORE_LIB OFC_VOID
ofc_sched_remove_wait(OFC_HANDLE hScheduler, OFC_HANDLE hEvent) {
    SCHEDULER *scheduler;

    scheduler = ofc_handle_lock(hScheduler);
    if (scheduler != OFC_NULL) {
        ofc_waitset_remove(scheduler->hEventSet, hEvent);
        ofc_handle_unlock(hScheduler);
    }
}

static OFC_DWORD
ofc_scheduler_loop(OFC_HANDLE hThread, OFC_VOID *context) {
    OFC_HANDLE hScheduler;
    SCHEDULER *scheduler;

    hScheduler = (OFC_HANDLE) context;
    scheduler = ofc_handle_lock(hScheduler);

    while (!ofc_thread_is_deleting(hThread)) {
#if defined(OFC_PRESELECT_PASS)
        /*
         * Do a preselect path
         */
        ofc_sched_preselect (hScheduler) ;
#else
        if (scheduler->significant_event)
            ofc_sched_preselect(hScheduler);
#endif
        /*
         * Wait for some significant event
         */
        ofc_sched_wait(hScheduler);
        /*
         * And do a post select pas
         */
        ofc_sched_postselect(hScheduler);
    }

    ofc_sched_destroy(hScheduler);
    ofc_handle_unlock(hScheduler);
    return (0);
}

OFC_CORE_LIB OFC_BOOL
ofc_sched_empty(OFC_HANDLE hScheduler) {
    SCHEDULER *scheduler;
    OFC_BOOL ret;

    ret = OFC_FALSE;
    scheduler = ofc_handle_lock(hScheduler);
    if (scheduler != OFC_NULL) {
        if (ofc_queue_empty(scheduler->applications))
            ret = OFC_TRUE;
        ofc_handle_unlock(hScheduler);
    }
    return (ret);
}

#if defined(OFC_APP_DEBUG)
OFC_CORE_LIB OFC_VOID 
ofc_sched_dump (OFC_HANDLE hScheduler)
{
  SCHEDULER *scheduler ;
  OFC_HANDLE hApp ;

  scheduler = ofc_handle_lock (hScheduler) ;
  if (scheduler != OFC_NULL)
    {
      ofc_printf ("%-20s: %s\n", "Scheduled for Quit", 
           scheduler->quit ? "yes" : "no") ;
      ofc_printf ("%-20s: %s\n", "Significant Event",
           scheduler->quit ? "yes" : "no") ;

      /*
       * Go through all the apps until there are no more or someone
       */
      for (hApp = (OFC_HANDLE) ofc_queue_first (scheduler->applications) ;
       hApp != OFC_HANDLE_NULL ;
       hApp = (OFC_HANDLE) ofc_queue_next (scheduler->applications,
                       (OFC_VOID *) hApp))

    {
      ofc_app_dump (hApp) ;
    }
      ofc_handle_unlock (hScheduler) ;
    }
}
#endif
