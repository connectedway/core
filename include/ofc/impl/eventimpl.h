/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__BLUE_EVENT_IMPL_H__)
#define __BLUE_EVENT_IMPL_H__

#include "ofc/core.h"
#include "ofc/types.h"

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Create a Platform Specific Event
   *
   * This function is called to create a platform specific event. 
   * There are two types of events, AUTO and MANUAL.  Auto events
   * are automatically reset after waiting on them.  Manual events stay
   * set until explicitly reset.
   *
   * \param eventType
   * Type of event
   *
   * \returns
   * Handle to event
   */
  BLUE_HANDLE 
  BlueEventCreateImpl (BLUE_EVENT_TYPE eventType) ;
  /**
   * Return the event type
   *
   * \param hEvent
   * The handle to the event to obtail the type for
   *
   * \returns
   * The event type
   */
  BLUE_EVENT_TYPE BlueEventGetTypeImpl (BLUE_HANDLE hEvent) ;
  /**
   * Set an event.  
   *
   * Cause an event to be signalled in the platform
   *
   * \param hEvent
   * The event to signal
   */
  BLUE_VOID BlueEventSetImpl (BLUE_HANDLE hEvent) ;
  /**
   * Reset an Event
   *
   * Rearm a manual event in the platform
   *
   * \param hEvent
   * The event to rearm
   */
  BLUE_VOID BlueEventResetImpl (BLUE_HANDLE hEvent) ;
  /**
   * Test if an event has been signalled
   *
   * \param hEvent
   * The event handle to test
   *
   * \returns
   * BLUE_TRUE if set, BLUE_FALSE otherwise
   */
  BLUE_BOOL BlueEventTestImpl (BLUE_HANDLE hEvent) ;
  /**
   * Destroy an Event
   *
   * \param hEvent
   * The handle to the event to destroy
   */
  BLUE_VOID BlueEventDestroyImpl (BLUE_HANDLE hEvent) ;
  /**
   * Wait for an event to fire.
   *
   * If the event is an automatic event, it is automatically reset after
   * this return returns
   */
  BLUE_VOID BlueEventWaitImpl (BLUE_HANDLE hEvent) ;
#if defined(__cplusplus)
}
#endif

#endif

