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
#include "ofc/sched.h"
#include "ofc/app.h"
#include "ofc/event.h"

#include "ofc/heap.h"

/**
 * The Application Descriptor
 */
typedef struct _BLUEAPP
{
  BLUEAPP_TEMPLATE *def ;	/* Pointer to app definition */
  BLUE_HANDLE scheduler ;	/* Scheduler Hanlder */
  BLUE_BOOL destroy ;		/* Flag to destroy app */
  BLUE_VOID *app_data ;
  BLUE_HANDLE hNotify ;
} BLUEAPP ;

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
BLUE_CORE_LIB BLUE_HANDLE 
BlueAppCreate (BLUE_HANDLE scheduler, BLUEAPP_TEMPLATE *templatep, 
	       BLUE_VOID *app_data)
{
  BLUEAPP *app ;
  BLUE_HANDLE hApp ;

  /*
   * Allocate structure for app
   */
  app = BlueHeapMalloc (sizeof (BLUEAPP)) ;

  /*
   * Initialize the application
   */
  app->def = templatep;
  app->scheduler = scheduler ;
  app->destroy = BLUE_FALSE ;
  app->app_data = app_data;
  app->hNotify = BLUE_HANDLE_NULL ;

  hApp = BlueHandleCreate (BLUE_HANDLE_APP, app) ;
  /*
   * Application was initialized, add to scheduler
   */
  BlueSchedAdd (scheduler, hApp) ;
#if defined(BLUE_PARAM_PRESELECT_PASS)
  BlueAppEventSig (hApp) ;
#else
  BlueAppPreselect (hApp) ;
#endif
  /*
   * Return the application pointer
   */
  return (hApp) ;
}

/*
 * BlueAppKill - schedule the application for destruction.
 *
 * Accepts:
 *    The application to kill
 *
 * The application will be killed by the scheduler
 */
BLUE_CORE_LIB BLUE_VOID 
BlueAppKill (BLUE_HANDLE hApp)
{
  BLUEAPP *app ;

  app = BlueHandleLock (hApp) ;
  if (app != BLUE_NULL)
    {
      /*
       * Set the flag to kill the app
       */
      app->destroy = BLUE_TRUE ;
      /*
       * And notify the scheduler
       */
      BlueHandleUnlock (hApp) ;
      BlueAppEventSig (hApp) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueAppSetWait (BLUE_HANDLE hApp, BLUE_HANDLE hNotify)
{
  BLUEAPP *app ;

  app = BlueHandleLock (hApp) ;
  if (app != BLUE_NULL)
    {
      app->hNotify = hNotify ;
      BlueHandleUnlock (hApp) ;
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
BLUE_CORE_LIB BLUE_VOID 
BlueAppDestroy (BLUE_HANDLE hApp)
{
  BLUEAPP *app ;

  app = BlueHandleLock (hApp) ;
  if (app != BLUE_NULL)
    {
      /*
       * Call the application specific routine
       */
      (*app->def->destroy)(hApp) ;

      if (app->hNotify != BLUE_HANDLE_NULL)
	BlueEventSet  (app->hNotify) ;

      BlueHandleDestroy (hApp) ;
      BlueHandleUnlock (hApp) ;
      BlueHeapFree (app) ;
    }
}

/*
 * STATE_preselect - Call the apps preselect routine
 *
 * Accepts: hApp - The handle of the app to do the preselect for
 */
BLUE_CORE_LIB BLUE_VOID 
BlueAppPreselect (BLUE_HANDLE hApp)
{
  BLUEAPP *app ;

  app = BlueHandleLock (hApp) ;
  if (app != BLUE_NULL)
    {
      if (!app->destroy)
	{
	  (*app->def->preselect)(hApp) ;
	}
      BlueHandleUnlock (hApp) ;
    }
}

/*
 * STATE_postselect - Call the app's postselect routine
 *
 * Accepts:
 *    hApp - The handle of the app doing the postselect for
 *    events - The events that have occured
 */
BLUE_CORE_LIB BLUE_HANDLE 
BlueAppPostselect (BLUE_HANDLE hApp, BLUE_HANDLE hEvent)
{
  BLUEAPP *app ;
  BLUE_HANDLE ret ;

  ret = BLUE_HANDLE_NULL ;
  app = BlueHandleLock (hApp) ;
  if (app != BLUE_NULL)
    {
      if (!app->destroy)
	{
	  ret = (*app->def->postselect)(hApp, hEvent) ;
	}
      BlueHandleUnlock (hApp) ;
    }
  return (ret) ;
}

/*
 * STATE_event_sig - Signal a significant event for the app
 *
 * Accepts:
 *    Handle to app that has generated a significant event
 */
BLUE_CORE_LIB BLUE_VOID 
BlueAppEventSig (BLUE_HANDLE hApp)
{
  BLUEAPP *app ;

  app = BlueHandleLock (hApp) ;
  if (app != BLUE_NULL)
    {
      /*
       * Pass the significant event on to the scheduler
       */
      BlueSchedSignificantEvent (app->scheduler) ;
      BlueHandleUnlock (hApp) ;
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
BLUE_CORE_LIB BLUE_BOOL 
BlueAppDestroying (BLUE_HANDLE hApp)
{
  BLUEAPP *app ;
  BLUE_BOOL ret ;

  app = BlueHandleLock (hApp) ;
  ret = BLUE_FALSE ;
  if (app != BLUE_NULL)
    {
      ret = app->destroy ;
      BlueHandleUnlock (hApp) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID *
BlueAppGetData (BLUE_HANDLE hApp)
{
  BLUEAPP *app ;
  BLUE_VOID *ret ;

  ret = BLUE_NULL ;
  app = BlueHandleLock (hApp) ;
  if (app != BLUE_NULL)
    {
      ret = app->app_data ;
      BlueHandleUnlock (hApp) ;
    }
  return (ret) ;
}

#if defined(BLUE_PARAM_APP_DEBUG)
BLUE_CORE_LIB BLUE_VOID 
BlueAppDump (BLUE_HANDLE hApp)
{
  BLUEAPP *app ;

  app = BlueHandleLock (hApp) ;
  if (app != BLUE_NULL)
    {
      if (*app->def->dump != BLUE_NULL)
	(*app->def->dump)(hApp) ;
      BlueHandleUnlock (hApp) ;
    }
}
#endif
