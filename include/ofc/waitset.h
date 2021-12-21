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
#if !defined(__BLUE_WAIT_SET_H__)
#define __BLUE_WAIT_SET_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/**
 * \addtogroup BlueSched
 *
 * Wait Sets are a key component of Blue Share and allow for synchronization
 * of events between applications and platform threads.  The events can
 * be for network sockets, files, wait queues, semaphores, or any 
 * synchonizable abstraction supported by the platform.
 * \{ 
 */

/**
 * Visible definition of Wait Set
 *
 * Used by platform scheduling code
 */
typedef struct
{
  BLUE_HANDLE hHandleQueue ;	/**< List of events in wait set  */
  BLUE_VOID *impl ;		/**< Pointer to implementation info  */
} WAIT_SET ;

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Create a wait set
   *
   * A Wait Set is just a container that events are added to.  Events
   * are specified as handles.  Multiple events can be added and a subsequent
   * wait will wait for any one of the events to be triggered.
   *
   * \returns
   * A handle to the wait set
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueWaitSetCreate (BLUE_VOID) ;
  /**
   * Destroy a wait set
   *
   * \param handle
   * Handle of the wait set to destroy
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueWaitSetDestroy (BLUE_HANDLE handle) ;
  /**
   * Wait for an event in a wait set to be ready
   *
   * \param handle
   * Handle of the wait set to wait on
   *
   * \returns
   * Handle to the app with the event that is ready.  
   * BLUE_HANDLE_NULL if woken by a wake call.
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueWaitSetWait (BLUE_HANDLE handle) ;
  /**
   * Wake up a wait set that is currently waiting.
   *
   * \param handle
   * Handle of the wait set to wake up.
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueWaitSetWake (BLUE_HANDLE handle) ;
  /**
   * Add an event to the wait set
   *
   * \param hSet
   * Handle to the wait set to add the event to
   *
   * \param hApp
   * Handle to the app that owns the event
   *
   * \param hEvent
   * Handle to the event to add
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueWaitSetAdd (BLUE_HANDLE hSet, BLUE_HANDLE hApp, BLUE_HANDLE hEvent) ;
  /**
   * Remove an event from a wait set
   *
   * \param hSet
   * Handle to wait set
   *
   * \param hEvent
   * Handle to event to remove
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueWaitSetRemove (BLUE_HANDLE hSet, BLUE_HANDLE hEvent) ;
  /**
   * Remove all events from a wait set
   *
   * \param handle
   * Handle to wait set
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueWaitSetClear (BLUE_HANDLE handle) ;
  BLUE_CORE_LIB BLUE_VOID 
  BlueWaitSetClearApp (BLUE_HANDLE handle, BLUE_HANDLE hApp) ;
#if defined(BLUE_PARAM_HANDLE_DEBUG)
  BLUE_CORE_LIB BLUE_VOID 
  BlueWaitSetLogMeasure (BLUE_HANDLE handle)  ;
#endif

#if defined(__cplusplus)
}
#endif
/** \} */
#endif
