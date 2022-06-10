/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_APP_H__)
#define __OFC_APP_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/**
 * \defgroup app Event Driven Application Handling
 *
 * Open Files event driven applications are tasks that are defined by 
 * simple callbacks.  Each callback is non-blocking and state driven.
 * Applications can create events for many conditions and multiplex those
 * events on a wait list.  As events are fired and actions are to be
 * performed, the application will be callbacked.  Depending on state
 * and event, applications can dispatch to appropriate handling.
 */

/** \{ */

/**
 * \struct _OFC_APP_TEMPLATE
 * The Application Template Definition
 *
 * Each application defines one of these and registers it with the
 * scheduler
 *
 * \var _OFC_APP_TEMPLATE:name
 * Name of App 
 *
 * \var _OFC_APP_TEMPLATE:preselect
 * App's preselect routine 
 * 
 * \var _OFC_APP_TEMPLATE:postselect
 * Apps Postselect routine 
 *
 * \var _OFC_APP_TEMPLATE:destroy
 * App's destroy routine 
 * 
 * \var _OFC_APP_TEMPLATE:dump
 * App's dump routine (used in debug builds)
 */
typedef struct _OFC_APP_TEMPLATE {
    OFC_CHAR *name;
    OFC_VOID (*preselect)(OFC_HANDLE app);
    OFC_HANDLE (*postselect)(OFC_HANDLE app, OFC_HANDLE hEvent);
    OFC_VOID (*destroy)(OFC_HANDLE app); 
    OFC_VOID (*dump)(OFC_HANDLE app);
} OFC_APP_TEMPLATE;

#if defined(__cplusplus)
extern "C"
{
#endif
/**
 * Create an application
 *
 * \param scheduler
 * Scheduler that app belongs to
 *
 * \param template
 * Definition of application
 *
 * \param data
 * Context for application
 *
 * \returns
 * Handle to the application
 */
OFC_CORE_LIB OFC_HANDLE
ofc_app_create(OFC_HANDLE scheduler, OFC_APP_TEMPLATE *template,
               OFC_VOID *app_data);
/**
 * Schedule the application for destruction.
 *
 * \param app
 * Handle to the app to kill
 */
OFC_CORE_LIB OFC_VOID
ofc_app_kill(OFC_HANDLE app);
/**
 * \protected
 * Destroy an application
 *
 * Note: This is called only by the applications scheduler.
 *
 * \param app
 * Application to destroy
 */
OFC_CORE_LIB OFC_VOID
ofc_app_destroy(OFC_HANDLE app);
/**
 * \protected
 * Call the apps preselect routine
 *
 * NOTE: This is called only by the applications scheduler
 *
 * \param app
 * Handle to the app to preselect
 */
OFC_CORE_LIB OFC_VOID
ofc_app_preselect(OFC_HANDLE app);
/**
 * \protected
 * Call the app's postselect routine
 *
 * NOTE: This is called only by the applications scheduler
 *
 * \param hApp
 * Handle to app to postselect
 *
 * \param hEvent
 * App's event that fired.
 */
OFC_CORE_LIB OFC_HANDLE
ofc_app_postselect(OFC_HANDLE hApp, OFC_HANDLE hEvent);
/**
 * Signal a significant event for the app.  This is passed to the apps
 * scheduler. This will cause all applications on that scheduler to
 * reexecute their preselect routines.
 *
 * \param app
 * Handle to the app to preselect
 */
OFC_CORE_LIB OFC_VOID
ofc_app_sig_event(OFC_HANDLE app);
/**
 * Return whether this app is scheduled for destruction
 *
 * \param hApp
 * Test if an app is being destroyed
 *
 * \returns
 * OFC_TRUE if it's being destroyed.  OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_app_destroying(OFC_HANDLE hApp);
/**
 * Return the apps context
 *
 * \param hApp
 * Handle to app for the context
 *
 * \returns
 * The apps context
 */
OFC_CORE_LIB OFC_VOID *
ofc_app_get_data(OFC_HANDLE hApp);
/**
 * Set an event to be notified when the app is destroyed
 *
 * \param hApp
 * Handle to the app
 *
 * \param hNotify
 * Event to notify when app exits
 */
OFC_CORE_LIB OFC_VOID
ofc_app_set_wait(OFC_HANDLE hApp, OFC_HANDLE hNotify);

#if defined(OFC_APP_DEBUG)
/**
 * Dump the state of the app
 *
 * NOTE: This is a debug routine 
 *
 * \param hApp
 * Handle to app to dump
 */
OFC_CORE_LIB OFC_VOID
ofc_app_dump (OFC_HANDLE hApp) ;
#endif

#if defined(__cplusplus)
}
#endif
/** \} */
#endif

