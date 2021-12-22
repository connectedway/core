/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __BLUE_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/queue.h"
#include "ofc/handle.h"
#include "ofc/heap.h"
#include "ofc/process.h"

/*
 * QUEUE_LINK - The basic list link element.  This is also used as a 
 * list header
 */
typedef struct _QUEUE_LINK
{
  struct _QUEUE_LINK *qNext ;	/* Pointer to the next link */
  struct _QUEUE_LINK *qPrev ;	/* Pointer to the previous link */
  union {
    BLUE_VOID *qElement ;	/* Pointer to the link element if a link */
    struct _QUEUE_LINK *qCache ; /* Pointer to the cache link if head */
  } u ;
} QUEUE_LINK ;

/*
* Forward declarations
*/
static QUEUE_LINK *BlueQfindLink (QUEUE_LINK *qHead, BLUE_VOID *qElement) ;

/*
 * BlueQcreate - create a linked list
 *
 * Accepts:
 *    nothing
 *
 * Returns:
 *    Queue
 */
BLUE_CORE_LIB BLUE_HANDLE 
BlueQcreate (BLUE_VOID)
{
  QUEUE_LINK *qHead ;
  BLUE_HANDLE handle ;

  /*
   * Allocate space for the queue head
   */
  qHead = BlueHeapMalloc (sizeof (QUEUE_LINK)) ;
  /*
   * Did we get a head
   */
  if (qHead != BLUE_NULL)
    {
      /*
       * Yes, so initialize head
       */
      qHead->qNext = qHead ;
      qHead->qPrev = qHead ;
      qHead->u.qCache = qHead ;
      if (qHead->qPrev == BLUE_NULL)
	BlueProcessCrash ("queue corruption\n") ;

      handle = BlueHandleCreate (BLUE_HANDLE_QUEUE, qHead) ;
    }
  else
    /*
     * Couldn't allocate head, set it to null
     */
    handle = BLUE_HANDLE_NULL ;

  return (handle) ;
}

/*
 * BlueQdestroy - Destroy a queue
 *
 * Accepts:
 *    Head of queue
 *
 * Returns:
 *    Nothing
 */
BLUE_CORE_LIB BLUE_VOID 
BlueQdestroy (BLUE_HANDLE qHandle)
{
  QUEUE_LINK *qHead ;

  qHead = BlueHandleLock (qHandle) ;

  if (qHead != BLUE_NULL)
    {
      /*
       * Is the queue empty?
       */
      if (BlueQempty (qHandle))
	{
	  /*
	   * Yes, so deallocate it
	   */
	  BlueHeapFree (qHead) ;
	  BlueHandleDestroy (qHandle) ;
	}
      BlueHandleUnlock (qHandle) ;
    }
}

/*
 * BlueQenqueue - Add an element to the end of the list
 *
 * Accepts:
 *    qHead - Pointer to list header
 *    qElement - Pointer to element to add to list
 */
BLUE_CORE_LIB BLUE_VOID 
BlueQenqueue (BLUE_HANDLE qHandle, BLUE_VOID *qElement)
{
  QUEUE_LINK *qHead ;
  QUEUE_LINK *qLink ;

  qHead = BlueHandleLock (qHandle) ;
  if (qHead != BLUE_NULL)
    {
      if (qHead->qPrev == BLUE_NULL)
	BlueProcessCrash ("queue corruption\n") ;
      qLink = BlueQfindLink (qHead, qElement) ;
      if (qLink != qHead)
	BlueProcessCrash ("Link already queued\n") ;

      /*
       * Allocate a new list element
       */
      qLink = BlueHeapMalloc (sizeof (QUEUE_LINK)) ;

      qLink->u.qElement = qElement ;
      qLink->qPrev = qHead->qPrev ;
      qLink->qNext = qHead ;
      qLink->qPrev->qNext = qLink ;
      qLink->qNext->qPrev = qLink ;
      /*
       * And set the list cache to the new element
       */
      qHead->u.qCache = qLink ;
      if (qHead->qPrev == BLUE_NULL)
	BlueProcessCrash ("queue corruption\n") ;
      BlueHandleUnlock (qHandle) ;
    }
}

/*
 * BlueQdequeue - Remove an element from the front of the list
 *
 * Accepts:
 *    qHead - Pointer to list header
 * 
 * Returns:
 *    Item at the front or NB_NULL
 */
BLUE_CORE_LIB BLUE_VOID *
BlueQdequeue (BLUE_HANDLE qHandle)
{
  QUEUE_LINK *qHead ;
  QUEUE_LINK *qLink ;
  BLUE_VOID *qElement ;

  qElement = BLUE_NULL ;
  qHead = BlueHandleLock (qHandle) ;
  if (qHead != BLUE_NULL)
    {
      if (qHead->qPrev == BLUE_NULL)
	BlueProcessCrash ("queue corruption\n") ;
      /*
       * See if the list is empty
       */
      qLink = qHead->qNext ;
      if (qLink == qHead)
	{
	  /*
	   * Yes, return empty
	   */
	  qElement = BLUE_NULL ;
	}
      else
	{
	  /*
	   * remove the element from the list
	   */
	  qLink->qNext->qPrev = qLink->qPrev ;
	  qLink->qPrev->qNext = qLink->qNext ;
	  qElement = qLink->u.qElement ;
	  if (qHead->u.qCache == qLink)
	    qHead->u.qCache = qHead ;
	  /*
	   * And get rid of the link
	   */
	  BlueHeapFree (qLink) ;
	}
      if (qHead->qPrev == BLUE_NULL)
	BlueProcessCrash ("queue corruption\n") ;
      BlueHandleUnlock (qHandle) ;
    }
  return (qElement) ;
}

/*
 * BlueQempty - See if queue is empty
 *
 * Accepts:
 *    Queue Head
 *
 * Returns:
 *    True if empty, false otherwise
 */
BLUE_CORE_LIB BLUE_BOOL 
BlueQempty (BLUE_HANDLE qHandle) 
{
  QUEUE_LINK *qHead ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  qHead = BlueHandleLock (qHandle) ;
  if (qHead != BLUE_NULL)
    {
      if (qHead->qPrev == BLUE_NULL)
	BlueProcessCrash ("queue corruption\n") ;
      if (qHead->qNext == qHead)
	ret = BLUE_TRUE ;
      BlueHandleUnlock (qHandle) ;
    }
  return (ret) ;
}

/*
 * BlueQfirst - Return the head of the linked list
 *
 * Accepts:
 *    qHead - Pointer to the head of the list
 *
 * Returns:
 *    VOID * - Element that was at the front of the list
 */
BLUE_CORE_LIB BLUE_VOID *
BlueQfirst (BLUE_HANDLE qHandle)
{
  BLUE_VOID *qElement ;
  QUEUE_LINK *qHead ;

  qHead = BlueHandleLock (qHandle) ;
  if (qHead != BLUE_NULL)
    {
      if (qHead->qPrev == BLUE_NULL)
	BlueProcessCrash ("queue corruption\n") ;
      /*
       * See if the list is empty
       */
      if (BlueQempty(qHandle))
	/*
	 * Yes, return empty
	 */
	qElement = BLUE_NULL ;
      else
	{
	  /*
	   * No, so update the cache and return the next element
	   */
	  qHead->u.qCache = qHead->qNext ;
	  qElement = qHead->u.qCache->u.qElement ;
	}
      if (qHead->qPrev == BLUE_NULL)
	BlueProcessCrash ("queue corruption\n") ;
      BlueHandleUnlock (qHandle) ;
    }
  else
    qElement = BLUE_NULL ;

  return (qElement) ;
}

/*
 * BlueQnext - Return the next element on the list
 *
 * Accepts:
 *    qHead - Pointer to the head of the list
 *    qElement - Pointer to the current element
 *
 * Returns:
 *    VOID * - Pointer to the next element
 */
BLUE_CORE_LIB BLUE_VOID *
BlueQnext (BLUE_HANDLE qHandle, BLUE_VOID *qElement) 
{
  QUEUE_LINK *qLink ;
  QUEUE_LINK *qHead ;
  BLUE_VOID *qReturn ;

  qReturn = BLUE_NULL ;
  qHead = BlueHandleLock (qHandle) ;
  if (qHead != BLUE_NULL)
    {
      if (qHead->qPrev == BLUE_NULL)
	BlueProcessCrash ("queue corruption\n") ;
      /*
       * Locate the currrent link
       */
      qLink = BlueQfindLink (qHead, qElement) ;

      /*
       * Did we find the element?
       */
      if (qLink == qHead)
	{
	  /*
	   * No, so return NULL
	   */
	  qReturn = BLUE_NULL ;
	}
      else
	{
	  /*
	   * We found the link, get the next one and update the cache
	   */
	  qLink = qLink->qNext ;
	  qHead->u.qCache = qLink ;
	  /*
	   * Are we at the end of the list?
	   */
	  if (qLink == qHead)
	    /*
	     * Yes, so return NULL
	     */
	    qReturn = BLUE_NULL ;
	  else
	    /*
	     * No, so return the element
	     */
	    qReturn = qLink->u.qElement ;
	}
      if (qHead->qPrev == BLUE_NULL)
	BlueProcessCrash ("queue corruption\n") ;
      BlueHandleUnlock (qHandle) ;
    }
  return (qReturn) ;
}

/*
 * BlueQunlink - Unlink the current element from the list
 *
 * Accepts:
 *    qHead - Pointer to the head of the list
 *    qElement - Pointer to the current element
 *
 * Returns:
 *    nothing
 */
BLUE_CORE_LIB BLUE_VOID 
BlueQunlink (BLUE_HANDLE qHandle, BLUE_VOID *qElement)
{
  QUEUE_LINK *qLink ;
  QUEUE_LINK *qHead ;

  qHead = BlueHandleLock (qHandle) ;
  if (qHead != BLUE_NULL)
    {
      if (qHead->qPrev == BLUE_NULL)
	BlueProcessCrash ("queue corruption\n") ;
      /*
       * See if we can find the current element
       */
      qLink = BlueQfindLink (qHead, qElement) ;

      /*
       * Did we find the element?
       */
      if (qLink != qHead)
	{
	  /*
	   * Yes, so update the cache and remove the element from the list
	   */
	  qHead->u.qCache = qLink->qNext ;
	  qLink->qNext->qPrev = qLink->qPrev ;
	  qLink->qPrev->qNext = qLink->qNext ;
	  if (qHead->u.qCache == qLink)
	    qHead->u.qCache = qHead ;
	  /*
	   * And get rid of the link
	   */
	  BlueHeapFree (qLink) ;
	}
      if (qHead->qPrev == BLUE_NULL)
	BlueProcessCrash ("queue corruption\n") ;
      BlueHandleUnlock (qHandle) ;
    }
}

/*
* BlueQfindLink - Find the link that contains the element
*
* Accepts:
*    qHead - Pointer to the head of the list
*    qElement - Element to locate
*
* Returns:
*    QUEUE_LINK * - Pointer to the link that contains the element
*
*/
static QUEUE_LINK *
BlueQfindLink (QUEUE_LINK *qHead, BLUE_VOID *qElement)
{
  QUEUE_LINK *qLink ;

  /*
   * Start with the cached link
   */
  qLink = qHead->u.qCache ;
  /*
   * Does the cached link point to this element?
   */
  if ((qLink == qHead) || (qLink->u.qElement != qElement))
    {
      /*
       * No, so Search the list for the element
       */
      for (qLink = qHead->qNext ;
	   (qLink != qHead) && (qLink->u.qElement != qElement) ;
	   qLink = qLink->qNext) ;
      /*
       * If we found the element, qlink will point to it.  If we
       * didn't find it, qlink will be NULL
       */
    }
  return (qLink) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueQclear (BLUE_HANDLE qHandle)
{
  BLUE_VOID *entry ;

  for (entry = BlueQdequeue (qHandle) ; entry != BLUE_NULL ; 
       entry = BlueQdequeue (qHandle)) ;
}
