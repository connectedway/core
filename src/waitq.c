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
#include "ofc/waitq.h"
#include "ofc/handle.h"
#include "ofc/event.h"
#include "ofc/lock.h"

#include "ofc/heap.h"

typedef struct
{
  BLUE_HANDLE hEvent ;
  BLUE_HANDLE hQueue ;
  BLUE_LOCK lock ;
} WAIT_QUEUE ;

/*
 * BlueWaitQcreate - create a linked list
 *
 * Accepts:
 *    nothing
 *
 * Returns:
 *    Queue
 */
BLUE_CORE_LIB BLUE_HANDLE 
BlueWaitQcreate (BLUE_VOID)
{
  WAIT_QUEUE *wait_queue ;
  BLUE_HANDLE hWaitQueue ;

  /*
   * Allocate space for the queue head
   */
  hWaitQueue = BLUE_HANDLE_NULL ;
  wait_queue = BlueHeapMalloc (sizeof (WAIT_QUEUE)) ;
  
  if (wait_queue != BLUE_NULL)
    {
      wait_queue->hQueue = BlueQcreate () ;
      wait_queue->hEvent = BlueEventCreate (BLUE_EVENT_MANUAL) ;
      hWaitQueue = BlueHandleCreate (BLUE_HANDLE_WAIT_QUEUE, wait_queue) ;
      wait_queue->lock = BlueLockInit () ;
    }
  return (hWaitQueue) ;
}

/*
 * BlueWaitQdestroy - Destroy a queue
 *
 * Accepts:
 *    Head of queue
 *
 * Returns:
 *    Nothing
 */
BLUE_CORE_LIB BLUE_VOID 
BlueWaitQdestroy (BLUE_HANDLE qHandle)
{
  WAIT_QUEUE *pWaitQueue ;


  pWaitQueue = BlueHandleLock (qHandle) ;

  if (pWaitQueue != BLUE_NULL)
    {
      BlueLockDestroy (pWaitQueue->lock) ;
      BlueQdestroy (pWaitQueue->hQueue) ;
      BlueEventDestroy (pWaitQueue->hEvent) ;

      BlueHeapFree (pWaitQueue) ;
      BlueHandleDestroy (qHandle) ;
      BlueHandleUnlock (qHandle) ;
    }
}

/*
 * BlueWaitQenqueue - Add an element to the end of the list
 *
 * Accepts:
 *    qHead - Pointer to list header
 *    qElement - Pointer to element to add to list
 */
BLUE_CORE_LIB BLUE_VOID 
BlueWaitQenqueue (BLUE_HANDLE qHandle, BLUE_VOID *qElement)
{
  WAIT_QUEUE *pWaitQueue ;

  pWaitQueue = BlueHandleLock (qHandle) ;
  if (pWaitQueue != BLUE_NULL)
    {
      BlueLock (pWaitQueue->lock) ;
      BlueQenqueue (pWaitQueue->hQueue, qElement) ;
      BlueEventSet (pWaitQueue->hEvent) ;
      BlueUnlock (pWaitQueue->lock) ;
      BlueHandleUnlock (qHandle) ;
    }
}

/*
 * BlueWaitQdequeue - Remove an element from the front of the list
 *
 * Accepts:
 *    qHead - Pointer to list header
 * 
 * Returns:
 *    Item at the front or NB_NULL
 */
BLUE_CORE_LIB BLUE_VOID *
BlueWaitQdequeue (BLUE_HANDLE qHandle)
{
  WAIT_QUEUE *pWaitQueue ;
  BLUE_VOID *qElement ;

  pWaitQueue = BlueHandleLock (qHandle) ;
  qElement = BLUE_NULL ;
  if (pWaitQueue != BLUE_NULL)
    {
      BlueLock (pWaitQueue->lock) ;
      qElement = BlueQdequeue (pWaitQueue->hQueue) ;
      if (BlueQempty (pWaitQueue->hQueue))
	BlueEventReset (pWaitQueue->hEvent) ;
      BlueUnlock (pWaitQueue->lock) ;
      BlueHandleUnlock (qHandle) ;
    }
  return (qElement) ;
}

/*
 * BlueWaitQempty - See if queue is empty
 *
 * Accepts:
 *    Queue Head
 *
 * Returns:
 *    True if empty, false otherwise
 */
BLUE_CORE_LIB BLUE_BOOL 
BlueWaitQempty (BLUE_HANDLE qHandle) 
{
  WAIT_QUEUE *pWaitQueue ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;

  pWaitQueue = BlueHandleLock (qHandle) ;
  if (pWaitQueue != BLUE_NULL)
    {
      BlueLock (pWaitQueue->lock) ;
      ret = BlueQempty (pWaitQueue->hQueue) ;
      BlueUnlock (pWaitQueue->lock) ;
      BlueHandleUnlock (qHandle) ;
    }
  return (ret) ;
}

/*
 * BlueWaitQfirst - Return the head of the linked list
 *
 * Accepts:
 *    qHead - Pointer to the head of the list
 *
 * Returns:
 *    VOID * - Element that was at the front of the list
 */
BLUE_CORE_LIB BLUE_VOID *
BlueWaitQfirst (BLUE_HANDLE qHandle)
{
  BLUE_VOID *qElement ;
  WAIT_QUEUE *pWaitQueue ;

  qElement = BLUE_NULL ;
  pWaitQueue = BlueHandleLock (qHandle) ;
  if (pWaitQueue != BLUE_NULL)
    {
      BlueLock (pWaitQueue->lock) ;
      qElement = BlueQfirst (pWaitQueue->hQueue) ;
      BlueUnlock (pWaitQueue->lock) ;
      BlueHandleUnlock (qHandle) ;
    }
  return (qElement) ;
}

/*
 * BlueWaitQnext - Return the next element on the list
 *
 * Accepts:
 *    qHead - Pointer to the head of the list
 *    qElement - Pointer to the current element
 *
 * Returns:
 *    VOID * - Pointer to the next element
 */
BLUE_CORE_LIB BLUE_VOID *
BlueWaitQnext (BLUE_HANDLE qHandle, BLUE_VOID *qElement) 
{
  WAIT_QUEUE *pWaitQueue ;
  BLUE_VOID *qReturn ;

  qReturn = BLUE_NULL ;
  pWaitQueue = BlueHandleLock (qHandle) ;
  if (pWaitQueue != BLUE_NULL)
    {
      BlueLock (pWaitQueue->lock) ;
      qReturn = BlueQnext (pWaitQueue->hQueue, qElement) ;
      BlueUnlock (pWaitQueue->lock) ;
      BlueHandleUnlock (qHandle) ;
    }
  return (qReturn) ;
}

/*
 * BlueWaitQunlink - Unlink the current element from the list
 *
 * Accepts:
 *    qHead - Pointer to the head of the list
 *    qElement - Pointer to the current element
 *
 * Returns:
 *    nothing
 */
BLUE_CORE_LIB BLUE_VOID 
BlueWaitQunlink (BLUE_HANDLE qHandle, BLUE_VOID *qElement)
{
  WAIT_QUEUE *pWaitQueue ;

  pWaitQueue = BlueHandleLock (qHandle) ;
  if (pWaitQueue != BLUE_NULL)
    {
      BlueLock (pWaitQueue->lock) ;
      BlueQunlink (pWaitQueue->hQueue, qElement) ;
      if (BlueQempty (pWaitQueue->hQueue))
	BlueEventReset (pWaitQueue->hEvent) ;
      BlueUnlock (pWaitQueue->lock) ;
      BlueHandleUnlock (qHandle) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueWaitQclear (BLUE_HANDLE qHandle)
{
  WAIT_QUEUE *pWaitQueue ;
  BLUE_VOID *entry ;

  pWaitQueue = BlueHandleLock (qHandle) ;
  if (pWaitQueue != BLUE_NULL)
    {
      BlueLock (pWaitQueue->lock) ;
      for (entry = BlueQdequeue (pWaitQueue->hQueue) ; 
	   entry != BLUE_NULL ; 
	   entry = BlueQdequeue (pWaitQueue->hQueue)) ;
      BlueUnlock (pWaitQueue->lock) ;

      BlueHandleUnlock (qHandle) ;
    }
}

BLUE_CORE_LIB BLUE_HANDLE 
BlueWaitQGetEventHandle (BLUE_HANDLE qHandle)
{
  WAIT_QUEUE *pWaitQueue ;
  BLUE_HANDLE hEvent ;

  hEvent = BLUE_HANDLE_NULL ;

  pWaitQueue = BlueHandleLock (qHandle) ;
  if (pWaitQueue != BLUE_NULL)
    {
      hEvent = pWaitQueue->hEvent ;
      BlueHandleUnlock (qHandle) ;
    }
  return (hEvent) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueWaitQBlock (BLUE_HANDLE waitq)
{
  WAIT_QUEUE *pWaitQueue ;

  pWaitQueue = BlueHandleLock (waitq) ;
  if (pWaitQueue != BLUE_NULL)
    {
      if (BlueQempty (pWaitQueue->hQueue))
	BlueEventWait (pWaitQueue->hEvent) ;
      BlueHandleUnlock (waitq) ;
    }
}
