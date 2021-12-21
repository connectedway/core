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
#if !defined(__BLUE_WAIT_SET_IMPL_H__)
#define __BLUE_WAIT_SET_IMPL_H__

#include "ofc/core.h"
#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/waitset.h"

/**
 * \defgroup BlueWaitSetImpl Platform Dependent Scheduler Handling
 * \ingroup BluePort
 *
 * This facility provides the necessary platform specific support for
 * synchronizing daemon threads on multiple events
 */

/** \{ */

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Wait on a Wait Set
   *
   * \param handle
   * Implementation Specific Wait Set Handle
   *
   * \returns
   * Handle to member that is ready to run
   */
  BLUE_HANDLE BlueWaitSetWaitImpl (BLUE_HANDLE handle) ;
  /**
   * Wake up a Wait Set on behalf of an event
   *
   * This is a necessary function in order to multiplex sockets, file 
   * descriptors and semaphores in the same wait set.  Some platforms do
   * not support a unified view of synchronization handles.  This
   * function gives the platform specific code an opportunity to signal the
   * wait set in some using some other mechanism.
   *
   * \param hHandle
   * Handle to Wait Set
   */
  BLUE_VOID BlueWaitSetSetAssocImpl (BLUE_HANDLE hHandle, 
				     BLUE_HANDLE hApp, BLUE_HANDLE hSet) ;
  BLUE_VOID 
  BlueWaitSetSignalImpl (BLUE_HANDLE handle, BLUE_HANDLE hEvent) ;
  /**
   * Wake up a wait set
   *
   * \param handle
   * Handle of wake set to wake
   */
  BLUE_VOID BlueWaitSetWakeImpl (BLUE_HANDLE handle) ;
  /**
   * Create a wait set
   *
   * A Wait Set is just a container that events are added to.  Events
   * are specified as handles.  Multiple events can be added and a subsequent
   * wait will wait for any one of the events to be triggered.
   *
   * \param pWaitSet
   * Pointer to wait set abstraction that is returned
   */
  BLUE_VOID BlueWaitSetCreateImpl (WAIT_SET *pWaitSet) ;
  /**
   * Destroy a wait set
   *
   * \param pWaitSet
   * Pointer to wait set context to destroy
   */
  BLUE_VOID BlueWaitSetDestroyImpl (WAIT_SET *pWaitSet)  ;
  /**
   * Add an event to the wait set
   *
   * \param handle
   * Handle to the wait set to add the event to
   *
   * \param hApp
   * Handle to the app that owns the event
   *
   * \param hEvent
   * Handle to the event to add
   */
  BLUE_VOID 
  BlueWaitSetAddImpl (BLUE_HANDLE handle, BLUE_HANDLE hApp,
		      BLUE_HANDLE hEvent) ;

#if defined(__cplusplus)
}
#endif
 
/** \} */

#endif
