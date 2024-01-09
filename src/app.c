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
#include "ofc/sched.h"
#include "ofc/app.h"
#include "ofc/event.h"

#include "ofc/heap.h"

/**
 * The Application Descriptor
 */
typedef struct _OFC_APP {
    OFC_APP_TEMPLATE *def;    /* Pointer to app definition */
    OFC_HANDLE scheduler;    /* Scheduler Handler */
    OFC_BOOL destroy;        /* Flag to destroy app */
    OFC_VOID *app_data;
    OFC_HANDLE hNotify;
} OFC_APP;

/*
 * STATE_create - Create an application
 * 
 * Accepts:
 *    Scheduler that app belongs to
 *    Definition of application
 *
 * Returns:
 *    The application created (Null if error)
 */
OFC_CORE_LIB OFC_HANDLE
ofc_app_create(OFC_HANDLE scheduler, OFC_APP_TEMPLATE *templatep,
               OFC_VOID *app_data) {
    OFC_APP *app;
    OFC_HANDLE hApp;

    /*
     * Allocate structure for app
     */
    app = ofc_malloc(sizeof(OFC_APP));

    /*
     * Initialize the application
     */
    app->def = templatep;
    app->scheduler = scheduler;
    app->destroy = OFC_FALSE;
    app->app_data = app_data;
    app->hNotify = OFC_HANDLE_NULL;

    hApp = ofc_handle_create(OFC_HANDLE_APP, app);
    /*
     * Application was initialized, add to scheduler
     */
    ofc_sched_add(scheduler, hApp);
#if defined(OFC_PRESELECT_PASS)
    ofc_app_event_sig(hApp) ;
#else
    ofc_sched_wake(scheduler);
#endif
    /*
     * Return the application pointer
     */
    return (hApp);
}

/*
 * ofc_app_kill - schedule the application for destruction.
 *
 * Accepts:
 *    The application to kill
 *
 * The application will be killed by the scheduler
 */
OFC_CORE_LIB OFC_VOID
ofc_app_kill(OFC_HANDLE hApp) {
    OFC_APP *app;

    app = ofc_handle_lock(hApp);
    if (app != OFC_NULL) {
        /*
         * Set the flag to kill the app
         */
        app->destroy = OFC_TRUE;
        /*
         * And notify the scheduler
         */
        ofc_handle_unlock(hApp);
        ofc_app_sig_event(hApp);
    }
}

OFC_CORE_LIB OFC_VOID
ofc_app_set_wait(OFC_HANDLE hApp, OFC_HANDLE hNotify) {
    OFC_APP *app;

    app = ofc_handle_lock(hApp);
    if (app != OFC_NULL) {
        app->hNotify = hNotify;
        ofc_handle_unlock(hApp);
    }
}

/*
 * STATE_destroy - Destroy an application
 *
 * Accepts:
 *    Application to destroy
 *
 * This should only be called by the scheduler
 */
OFC_CORE_LIB OFC_VOID
ofc_app_destroy(OFC_HANDLE hApp) {
    OFC_APP *app;

    app = ofc_handle_lock(hApp);
    if (app != OFC_NULL) {
        /*
         * Call the application specific routine
         */
        (*app->def->destroy)(hApp);

        if (app->hNotify != OFC_HANDLE_NULL)
            ofc_event_set(app->hNotify);

        ofc_handle_destroy(hApp);
        ofc_handle_unlock(hApp);
        ofc_free(app);
    }
}

/*
 * STATE_preselect - Call the apps preselect routine
 *
 * Accepts: hApp - The handle of the app to do the preselect for
 */
OFC_CORE_LIB OFC_VOID
ofc_app_preselect(OFC_HANDLE hApp) {
    OFC_APP *app;

    app = ofc_handle_lock(hApp);
    if (app != OFC_NULL) {
        if (!app->destroy) {
            (*app->def->preselect)(hApp);
        }
        ofc_handle_unlock(hApp);
    }
}

/*
 * STATE_postselect - Call the app's postselect routine
 *
 * Accepts:
 *    hApp - The handle of the app doing the postselect for
 *    events - The events that have occurred
 */
OFC_CORE_LIB OFC_HANDLE
ofc_app_postselect(OFC_HANDLE hApp, OFC_HANDLE hEvent) {
    OFC_APP *app;
    OFC_HANDLE ret;

    ret = OFC_HANDLE_NULL;
    app = ofc_handle_lock(hApp);
    if (app != OFC_NULL) {
        if (!app->destroy) {
            ret = (*app->def->postselect)(hApp, hEvent);
        }
        ofc_handle_unlock(hApp);
    }
    return (ret);
}

/*
 * STATE_event_sig - Signal a significant event for the app
 *
 * Accepts:
 *    Handle to app that has generated a significant event
 */
OFC_CORE_LIB OFC_VOID
ofc_app_sig_event(OFC_HANDLE hApp) {
    OFC_APP *app;

    app = ofc_handle_lock(hApp);
    if (app != OFC_NULL) {
        /*
         * Pass the significant event on to the scheduler
         */
        ofc_sched_significant_event(app->scheduler);
        ofc_handle_unlock(hApp);
    }
}

/*
 * STATE_destroying - Return whether we're destroying this app
 *
 * Accepts:
 *    Handle to app
 *
 * Returns:
 *    whether we're destroying it or not
 */
OFC_CORE_LIB OFC_BOOL
ofc_app_destroying(OFC_HANDLE hApp) {
    OFC_APP *app;
    OFC_BOOL ret;

    app = ofc_handle_lock(hApp);
    ret = OFC_FALSE;
    if (app != OFC_NULL) {
        ret = app->destroy;
        ofc_handle_unlock(hApp);
    }
    return (ret);
}

OFC_CORE_LIB OFC_VOID *
ofc_app_get_data(OFC_HANDLE hApp) {
    OFC_APP *app;
    OFC_VOID *ret;

    ret = OFC_NULL;
    app = ofc_handle_lock(hApp);
    if (app != OFC_NULL) {
        ret = app->app_data;
        ofc_handle_unlock(hApp);
    }
    return (ret);
}

#if defined(OFC_APP_DEBUG)
OFC_CORE_LIB OFC_VOID 
ofc_app_dump(OFC_HANDLE hApp)
{
  OFC_APP *app ;

  app = ofc_handle_lock (hApp) ;
  if (app != OFC_NULL)
    {
      if (*app->def->dump != OFC_NULL)
    (*app->def->dump)(hApp) ;
      ofc_handle_unlock(hApp) ;
    }
}
#endif
