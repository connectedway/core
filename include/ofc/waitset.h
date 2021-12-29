/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_WAIT_SET_H__)
#define __OFC_WAIT_SET_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/**
 * \addtogroup sched
 *
 * Wait Sets are a key component of Open Files and allow for synchronization
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
typedef struct {
    OFC_HANDLE hHandleQueue;    /**< List of events in wait set  */
    OFC_VOID *impl;        /**< Pointer to implementation info  */
} WAIT_SET;

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
OFC_CORE_LIB OFC_HANDLE
ofc_waitset_create(OFC_VOID);
/**
 * Destroy a wait set
 *
 * \param handle
 * Handle of the wait set to destroy
 */
OFC_CORE_LIB OFC_VOID
ofc_waitset_destroy(OFC_HANDLE handle);
/**
 * Wait for an event in a wait set to be ready
 *
 * \param handle
 * Handle of the wait set to wait on
 *
 * \returns
 * Handle to the app with the event that is ready.
 * OFC_HANDLE_NULL if woken by a wake call.
 */
OFC_CORE_LIB OFC_HANDLE
ofc_waitset_wait(OFC_HANDLE handle);
/**
 * Wake up a wait set that is currently waiting.
 *
 * \param handle
 * Handle of the wait set to wake up.
 */
OFC_CORE_LIB OFC_VOID
ofc_waitset_wake(OFC_HANDLE handle);
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
OFC_CORE_LIB OFC_VOID
ofc_waitset_add(OFC_HANDLE hSet, OFC_HANDLE hApp, OFC_HANDLE hEvent);
/**
 * Remove an event from a wait set
 *
 * \param hSet
 * Handle to wait set
 *
 * \param hEvent
 * Handle to event to remove
 */
OFC_CORE_LIB OFC_VOID
ofc_waitset_remove(OFC_HANDLE hSet, OFC_HANDLE hEvent);
/**
 * Remove all events from a wait set
 *
 * \param handle
 * Handle to wait set
 */
OFC_CORE_LIB OFC_VOID
ofc_waitset_clear(OFC_HANDLE handle);

OFC_CORE_LIB OFC_VOID
ofc_waitset_clear_app(OFC_HANDLE handle, OFC_HANDLE hApp);

#if defined(OFC_HANDLE_DEBUG)
OFC_CORE_LIB OFC_VOID
ofc_waitset_log_measure(OFC_HANDLE handle)  ;
#endif

#if defined(__cplusplus)
}
#endif
/** \} */
#endif
