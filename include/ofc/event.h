/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_EVENT_H__)
#define __OFC_EVENT_H__

#include "ofc/core.h"
#include "ofc/types.h"

/*
 * The type of event
 *
 * Events can be one shot, or auto trigger
 */
typedef enum
  {
    OFC_EVENT_MANUAL,		/**< Arming of the event is manual  */
    OFC_EVENT_AUTO		/**< Arming of the event is automatic
				   It is reenabled after processing */
  } OFC_EVENT_TYPE ;

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
  OFC_CORE_LIB OFC_HANDLE
  ofc_event_create (OFC_EVENT_TYPE eventType) ;
  /**
   * Return the type of event
   *
   * \param hEvent
   * The event handle whose type to return
   *
   * \returns
   * The event type
   */
  OFC_CORE_LIB OFC_EVENT_TYPE
  ofc_event_get_type (OFC_HANDLE hEvent) ;
  /**
   * Set an event.  
   *
   * Cause an event to be signalled
   *
   * \param hEvent
   * The event to signal
   */
  OFC_CORE_LIB OFC_VOID
  ofc_event_set (OFC_HANDLE hEvent) ;
  /**
   * Reset an Event
   *
   * Rearm a manual event
   *
   * \param hEvent
   * The event to rearm
   */
  OFC_CORE_LIB OFC_VOID
  ofc_event_reset (OFC_HANDLE hEvent) ;
  /**
   * Test if an event has been signalled
   *
   * \param hEvent
   * The event handle to test
   *
   * \returns
   * OFC_TRUE if set, OFC_FALSE otherwise
   */
  OFC_CORE_LIB OFC_BOOL
  ofc_event_test (OFC_HANDLE hEvent) ;
  /**
   * Destroy an Event
   *
   * \param hEvent
   * The handle to the event to destroy
   * Nothing
   */
  OFC_CORE_LIB OFC_VOID
  ofc_event_destroy (OFC_HANDLE hEvent) ;
  /**
   * Wait for an event to fire.
   *
   * If the event is an automatic event, it is automatically reset after
   * this return returns
   */
  OFC_CORE_LIB OFC_VOID
  ofc_event_wait (OFC_HANDLE hEvent) ;
#if defined(__cplusplus)
}
#endif
#endif

