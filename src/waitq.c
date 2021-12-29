/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/queue.h"
#include "ofc/waitq.h"
#include "ofc/handle.h"
#include "ofc/event.h"
#include "ofc/lock.h"

#include "ofc/heap.h"

typedef struct
{
  OFC_HANDLE hEvent ;
  OFC_HANDLE hQueue ;
  OFC_LOCK lock ;
} WAIT_QUEUE ;

/*
 * ofc_waitq_create - create a linked list
 *
 * Accepts:
 *    nothing
 *
 * Returns:
 *    Queue
 */
OFC_CORE_LIB OFC_HANDLE
ofc_waitq_create (OFC_VOID)
{
  WAIT_QUEUE *wait_queue ;
  OFC_HANDLE hWaitQueue ;

  /*
   * Allocate space for the queue head
   */
  hWaitQueue = OFC_HANDLE_NULL ;
  wait_queue = ofc_malloc (sizeof (WAIT_QUEUE)) ;
  
  if (wait_queue != OFC_NULL)
    {
      wait_queue->hQueue = ofc_queue_create () ;
      wait_queue->hEvent = ofc_event_create (OFC_EVENT_MANUAL) ;
      hWaitQueue = ofc_handle_create (OFC_HANDLE_WAIT_QUEUE, wait_queue) ;
      wait_queue->lock = ofc_lock_init () ;
    }
  return (hWaitQueue) ;
}

/*
 * ofc_waitq_destroy - Destroy a queue
 *
 * Accepts:
 *    Head of queue
 *
 * Returns:
 *    Nothing
 */
OFC_CORE_LIB OFC_VOID
ofc_waitq_destroy (OFC_HANDLE qHandle)
{
  WAIT_QUEUE *pWaitQueue ;


  pWaitQueue = ofc_handle_lock (qHandle) ;

  if (pWaitQueue != OFC_NULL)
    {
      ofc_lock_destroy (pWaitQueue->lock) ;
      ofc_queue_destroy (pWaitQueue->hQueue) ;
      ofc_event_destroy (pWaitQueue->hEvent) ;

      ofc_free (pWaitQueue) ;
      ofc_handle_destroy (qHandle) ;
      ofc_handle_unlock (qHandle) ;
    }
}

/*
 * ofc_waitq_enqueue - Add an element to the end of the list
 *
 * Accepts:
 *    qHead - Pointer to list header
 *    qElement - Pointer to element to add to list
 */
OFC_CORE_LIB OFC_VOID
ofc_waitq_enqueue (OFC_HANDLE qHandle, OFC_VOID *qElement)
{
  WAIT_QUEUE *pWaitQueue ;

  pWaitQueue = ofc_handle_lock (qHandle) ;
  if (pWaitQueue != OFC_NULL)
    {
      ofc_lock (pWaitQueue->lock) ;
      ofc_enqueue (pWaitQueue->hQueue, qElement) ;
      ofc_event_set (pWaitQueue->hEvent) ;
      ofc_unlock (pWaitQueue->lock) ;
      ofc_handle_unlock (qHandle) ;
    }
}

/*
 * ofc_waitq_dequeue - Remove an element from the front of the list
 *
 * Accepts:
 *    qHead - Pointer to list header
 * 
 * Returns:
 *    Item at the front or NB_NULL
 */
OFC_CORE_LIB OFC_VOID *
ofc_waitq_dequeue (OFC_HANDLE qHandle)
{
  WAIT_QUEUE *pWaitQueue ;
  OFC_VOID *qElement ;

  pWaitQueue = ofc_handle_lock (qHandle) ;
  qElement = OFC_NULL ;
  if (pWaitQueue != OFC_NULL)
    {
      ofc_lock (pWaitQueue->lock) ;
      qElement = ofc_dequeue (pWaitQueue->hQueue) ;
      if (ofc_queue_empty (pWaitQueue->hQueue))
	ofc_event_reset (pWaitQueue->hEvent) ;
      ofc_unlock (pWaitQueue->lock) ;
      ofc_handle_unlock (qHandle) ;
    }
  return (qElement) ;
}

/*
 * ofc_waitq_empty - See if queue is empty
 *
 * Accepts:
 *    Queue Head
 *
 * Returns:
 *    True if empty, false otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_waitq_empty (OFC_HANDLE qHandle)
{
  WAIT_QUEUE *pWaitQueue ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;

  pWaitQueue = ofc_handle_lock (qHandle) ;
  if (pWaitQueue != OFC_NULL)
    {
      ofc_lock (pWaitQueue->lock) ;
      ret = ofc_queue_empty (pWaitQueue->hQueue) ;
      ofc_unlock (pWaitQueue->lock) ;
      ofc_handle_unlock (qHandle) ;
    }
  return (ret) ;
}

/*
 * ofc_waitq_first - Return the head of the linked list
 *
 * Accepts:
 *    qHead - Pointer to the head of the list
 *
 * Returns:
 *    VOID * - Element that was at the front of the list
 */
OFC_CORE_LIB OFC_VOID *
ofc_waitq_first (OFC_HANDLE qHandle)
{
  OFC_VOID *qElement ;
  WAIT_QUEUE *pWaitQueue ;

  qElement = OFC_NULL ;
  pWaitQueue = ofc_handle_lock (qHandle) ;
  if (pWaitQueue != OFC_NULL)
    {
      ofc_lock (pWaitQueue->lock) ;
      qElement = ofc_queue_first (pWaitQueue->hQueue) ;
      ofc_unlock (pWaitQueue->lock) ;
      ofc_handle_unlock (qHandle) ;
    }
  return (qElement) ;
}

/*
 * ofc_waitq_next - Return the next element on the list
 *
 * Accepts:
 *    qHead - Pointer to the head of the list
 *    qElement - Pointer to the current element
 *
 * Returns:
 *    VOID * - Pointer to the next element
 */
OFC_CORE_LIB OFC_VOID *
ofc_waitq_next (OFC_HANDLE qHandle, OFC_VOID *qElement)
{
  WAIT_QUEUE *pWaitQueue ;
  OFC_VOID *qReturn ;

  qReturn = OFC_NULL ;
  pWaitQueue = ofc_handle_lock (qHandle) ;
  if (pWaitQueue != OFC_NULL)
    {
      ofc_lock (pWaitQueue->lock) ;
      qReturn = ofc_queue_next (pWaitQueue->hQueue, qElement) ;
      ofc_unlock (pWaitQueue->lock) ;
      ofc_handle_unlock (qHandle) ;
    }
  return (qReturn) ;
}

/*
 * ofc_waitq_unlink - Unlink the current element from the list
 *
 * Accepts:
 *    qHead - Pointer to the head of the list
 *    qElement - Pointer to the current element
 *
 * Returns:
 *    nothing
 */
OFC_CORE_LIB OFC_VOID
ofc_waitq_unlink (OFC_HANDLE qHandle, OFC_VOID *qElement)
{
  WAIT_QUEUE *pWaitQueue ;

  pWaitQueue = ofc_handle_lock (qHandle) ;
  if (pWaitQueue != OFC_NULL)
    {
      ofc_lock (pWaitQueue->lock) ;
      ofc_queue_unlink (pWaitQueue->hQueue, qElement) ;
      if (ofc_queue_empty (pWaitQueue->hQueue))
	ofc_event_reset (pWaitQueue->hEvent) ;
      ofc_unlock (pWaitQueue->lock) ;
      ofc_handle_unlock (qHandle) ;
    }
}

OFC_CORE_LIB OFC_VOID
ofc_waitq_clear (OFC_HANDLE qHandle)
{
  WAIT_QUEUE *pWaitQueue ;
  OFC_VOID *entry ;

  pWaitQueue = ofc_handle_lock (qHandle) ;
  if (pWaitQueue != OFC_NULL)
    {
      ofc_lock (pWaitQueue->lock) ;
      for (entry = ofc_dequeue (pWaitQueue->hQueue) ;
           entry != OFC_NULL ;
	   entry = ofc_dequeue (pWaitQueue->hQueue)) ;
      ofc_unlock (pWaitQueue->lock) ;

      ofc_handle_unlock (qHandle) ;
    }
}

OFC_CORE_LIB OFC_HANDLE
ofc_waitq_get_event_handle (OFC_HANDLE qHandle)
{
  WAIT_QUEUE *pWaitQueue ;
  OFC_HANDLE hEvent ;

  hEvent = OFC_HANDLE_NULL ;

  pWaitQueue = ofc_handle_lock (qHandle) ;
  if (pWaitQueue != OFC_NULL)
    {
      hEvent = pWaitQueue->hEvent ;
      ofc_handle_unlock (qHandle) ;
    }
  return (hEvent) ;
}

OFC_CORE_LIB OFC_VOID
ofc_waitq_block (OFC_HANDLE waitq)
{
  WAIT_QUEUE *pWaitQueue ;

  pWaitQueue = ofc_handle_lock (waitq) ;
  if (pWaitQueue != OFC_NULL)
    {
      if (ofc_queue_empty (pWaitQueue->hQueue))
	ofc_event_wait (pWaitQueue->hEvent) ;
      ofc_handle_unlock (waitq) ;
    }
}
