/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
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
  OFC_HANDLE BlueWaitSetWaitImpl (OFC_HANDLE handle) ;
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
  OFC_VOID BlueWaitSetSetAssocImpl (OFC_HANDLE hHandle,
                                    OFC_HANDLE hApp, OFC_HANDLE hSet) ;
  OFC_VOID
  BlueWaitSetSignalImpl (OFC_HANDLE handle, OFC_HANDLE hEvent) ;
  /**
   * Wake up a wait set
   *
   * \param handle
   * Handle of wake set to wake
   */
  OFC_VOID BlueWaitSetWakeImpl (OFC_HANDLE handle) ;
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
  OFC_VOID BlueWaitSetCreateImpl (WAIT_SET *pWaitSet) ;
  /**
   * Destroy a wait set
   *
   * \param pWaitSet
   * Pointer to wait set context to destroy
   */
  OFC_VOID BlueWaitSetDestroyImpl (WAIT_SET *pWaitSet)  ;
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
  OFC_VOID
  BlueWaitSetAddImpl (OFC_HANDLE handle, OFC_HANDLE hApp,
                      OFC_HANDLE hEvent) ;

#if defined(__cplusplus)
}
#endif
 
/** \} */

#endif
