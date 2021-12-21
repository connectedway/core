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
#if !defined(__BLUE_APP_H__)
#define __BLUE_APP_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/**
 * The Application Template Definition
 *
 * Each application defines one of these and registers it with the
 * scheduler
 */
typedef struct _BLUEAPP_TEMPLATE
{
  BLUE_CHAR *name ;			    /**< Name of App */
  BLUE_VOID (*preselect)(BLUE_HANDLE app) ; /**< App's preselect routine */
  /** Apps Postselect routine */
  BLUE_HANDLE (*postselect)(BLUE_HANDLE app, BLUE_HANDLE hEvent) ;
  BLUE_VOID (*destroy)(BLUE_HANDLE app) ; /**< App's destroy routine */
  BLUE_VOID (*dump)(BLUE_HANDLE app) ;	  /**< App's dump routine  */
} BLUEAPP_TEMPLATE ;

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
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueAppCreate (BLUE_HANDLE scheduler, BLUEAPP_TEMPLATE *template,
		 BLUE_VOID *app_dat) ;
  /**
   * schedule the application for destruction.
   *
   * \param app
   * Handle to the app to kill
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueAppKill (BLUE_HANDLE app) ;
  /**
   * Destroy an application
   *
   * \param app
   * Application to destroy
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueAppDestroy (BLUE_HANDLE app) ;
  /**
   * Call the apps preselect routine
   *
   * \param app
   * Handle to the app to preselect
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueAppPreselect (BLUE_HANDLE app) ;
  /**
   * Call the app's postselect routine
   *
   * \param hApp
   * Handle to app to postselect
   *
   * \param hEvent
   * Event to pass to app
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueAppPostselect (BLUE_HANDLE hApp, BLUE_HANDLE hEvent) ;
  /**
   * Signal a significant event for the app.  This is passed to the apps
   * scheduler. This will cause all applications on that scheduler to
   * reexecute their preselect routines.
   *
   * \param app
   * Handle to the app to preselect
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueAppEventSig (BLUE_HANDLE app) ;
  /**
   * Return whether we're destroying this app
   *
   * \param hApp
   * Test if an app is being destroyed
   *
   * \returns
   * BLUE_TRUE if it's being destroyed.  BLUE_FALSE otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueAppDestroying (BLUE_HANDLE hApp) ;
  /**
   * Return the apps context
   *
   * \param hApp
   * Handle to app for the context
   *
   * \returns
   * The apps context
   */
  BLUE_CORE_LIB BLUE_VOID *
  BlueAppGetData (BLUE_HANDLE hApp) ;
  /**
   * Set an event to be notified when the app is destroyed
   *
   * \param hApp
   * Handle to the app
   *
   * \param hNotify
   * Event to notify when app exits
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueAppSetWait (BLUE_HANDLE hApp, BLUE_HANDLE hNotify) ;
#if defined(BLUE_PARAM_APP_DEBUG)
  /**
   * Dump the state of the app
   *
   * \param hApp
   * Handle to app to dump
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueAppDump (BLUE_HANDLE hApp) ;
#endif

#if defined(__cplusplus)
}
#endif
#endif

