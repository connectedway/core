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
 * The Application Template Definition
 *
 * Each application defines one of these and registers it with the
 * scheduler
 */
typedef struct _OFC_APP_TEMPLATE
{
  OFC_CHAR *name ;			    /**< Name of App */
  OFC_VOID (*preselect)(BLUE_HANDLE app) ; /**< App's preselect routine */
  /** Apps Postselect routine */
  BLUE_HANDLE (*postselect)(BLUE_HANDLE app, BLUE_HANDLE hEvent) ;
  OFC_VOID (*destroy)(BLUE_HANDLE app) ; /**< App's destroy routine */
  OFC_VOID (*dump)(BLUE_HANDLE app) ;	  /**< App's dump routine  */
} OFC_APP_TEMPLATE ;

/**/
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
   * \returns
   * Handle to the application
   */
  OFC_CORE_LIB BLUE_HANDLE
  ofc_app_create (BLUE_HANDLE scheduler, OFC_APP_TEMPLATE *template,
                  OFC_VOID *app_dat) ;
  /**
   * schedule the application for destruction.
   *
   * \param app
   * Handle to the app to kill
   */
  OFC_CORE_LIB OFC_VOID
  ofc_app_kill (BLUE_HANDLE app) ;
  /**
   * Destroy an application
   *
   * \param app
   * Application to destroy
   */
  OFC_CORE_LIB OFC_VOID
  ofc_app_destroy (BLUE_HANDLE app) ;
  /**
   * Call the apps preselect routine
   *
   * \param app
   * Handle to the app to preselect
   */
  OFC_CORE_LIB OFC_VOID
  ofc_app_preselect (BLUE_HANDLE app) ;
  /**
   * Call the app's postselect routine
   *
   * \param hApp
   * Handle to app to postselect
   *
   * \param hEvent
   * Event to pass to app
   */
  OFC_CORE_LIB BLUE_HANDLE
  ofc_app_postselect (BLUE_HANDLE hApp, BLUE_HANDLE hEvent) ;
  /**
   * Signal a significant event for the app.  This is passed to the apps
   * scheduler. This will cause all applications on that scheduler to
   * reexecute their preselect routines.
   *
   * \param app
   * Handle to the app to preselect
   */
  OFC_CORE_LIB OFC_VOID
  ofc_app_sig_event (BLUE_HANDLE app) ;
  /**
   * Return whether we're destroying this app
   *
   * \param hApp
   * Test if an app is being destroyed
   *
   * \returns
   * OFC_TRUE if it's being destroyed.  OFC_FALSE otherwise
   */
  OFC_CORE_LIB OFC_BOOL
  ofc_app_destroying (BLUE_HANDLE hApp) ;
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
  ofc_app_get_data (BLUE_HANDLE hApp) ;
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
  ofc_app_set_wait (BLUE_HANDLE hApp, BLUE_HANDLE hNotify) ;
#if defined(BLUE_PARAM_APP_DEBUG)
  /**
   * Dump the state of the app
   *
   * \param hApp
   * Handle to app to dump
   */
  OFC_CORE_LIB OFC_VOID 
  ofc_app_dump (BLUE_HANDLE hApp) ;
#endif

#if defined(__cplusplus)
}
#endif
#endif

