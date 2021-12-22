/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__BLUE_EVENT_H__)
#define __BLUE_EVENT_H__

#include "ofc/core.h"
#include "ofc/types.h"

/*
 * The type of event
 *
 * Events can be one shot, or auto trigger
 */
typedef enum
  {
    BLUE_EVENT_MANUAL,		/**< Arming of the event is manual  */
    BLUE_EVENT_AUTO		/**< Arming of the event is automatic
				   It is reenabled after processing */
  } BLUE_EVENT_TYPE ;

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Create an Event
   *
   * \param eventType
   * The type of event to create
   *
   * \returns
   * Handle to the event
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueEventCreate (BLUE_EVENT_TYPE eventType) ;
  /**
   * Return the type of event
   *
   * \param hEvent
   * The event handle whose type to return
   *
   * \returns
   * The event type
   */
  BLUE_CORE_LIB BLUE_EVENT_TYPE 
  BlueEventGetType (BLUE_HANDLE hEvent) ;
  /**
   * Set an event.  
   *
   * Cause an event to be signalled
   *
   * \param hEvent
   * The event to signal
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueEventSet (BLUE_HANDLE hEvent) ;
  /**
   * Reset an Event
   *
   * Rearm a manual event
   *
   * \param hEvent
   * The event to rearm
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueEventReset (BLUE_HANDLE hEvent) ;
  /**
   * Test if an event has been signalled
   *
   * \param hEvent
   * The event handle to test
   *
   * \returns
   * BLUE_TRUE if set, BLUE_FALSE otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueEventTest (BLUE_HANDLE hEvent) ;
  /**
   * Destroy an Event
   *
   * \param hEvent
   * The handle to the event to destroy
   * Nothing
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueEventDestroy (BLUE_HANDLE hEvent) ;
  /**
   * Wait for an event to fire.
   *
   * If the event is an automatic event, it is automatically reset after
   * this return returns
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueEventWait (BLUE_HANDLE hEvent) ;
#if defined(__cplusplus)
}
#endif
#endif

