/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__BLUE_QUEUE_H__)
#define __BLUE_QUEUE_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/**
 * \defgroup BlueQueue Queue Management Facility
 * \ingroup BlueInternal
 *
 * Routines that provide for queue management
 * \{ 
 */

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Create a linked list
   *
   * \returns
   * Handle to the linked list
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueQcreate (BLUE_VOID) ;
  /**
   * Destroy a queue
   *
   * \param qHead
   * Head of queue
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueQdestroy (BLUE_HANDLE qHead) ;
  /**
   * Add an element to the end of the list
   *
   * \param qHead
   * Pointer to list header
   *
   * \param qElement 
   * Pointer to element to add to list
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueQenqueue (BLUE_HANDLE qHead, BLUE_VOID *qElement) ;
  /**
   * Remove an element from the front of the list
   *
   * \param qHead
   * Pointer to list header
   * 
   * \returns
   * Item at the front or BLUE_NULL
   */
  BLUE_CORE_LIB BLUE_VOID *
  BlueQdequeue (BLUE_HANDLE qHead) ;
  /**
   * See if queue is empty
   *
   * \param qHead
   * Queue Head
   *
   * \returns
   * BLUE_TRUE if empty, BLUE_FALSE otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueQempty (BLUE_HANDLE qHead) ;
  /**
   * Return the head of the linked list
   *
   * \param qHead
   * Pointer to the head of the list
   *
   * \returns
   * Element that was at the front of the list
   */
  BLUE_CORE_LIB BLUE_VOID *
  BlueQfirst (BLUE_HANDLE qHead) ;
  /**
   * Return the next element on the list
   *
   * \param qHead
   * Pointer to the head of the list
   *
   * \param qElement
   * Pointer to the current element
   *
   * \returns
   * Pointer to the next element
   */
  BLUE_CORE_LIB BLUE_VOID *
  BlueQnext (BLUE_HANDLE qHead, BLUE_VOID *qElement) ;
  /**
   * Unlink the current element from the list
   *
   * \param qHead 
   * Pointer to the head of the list
   *
   * \param qElement
   * Pointer to the current element
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueQunlink (BLUE_HANDLE qHead, BLUE_VOID *qElement) ;
  /**
   * Clear the contents of a linked list
   *
   * \param qHandle
   * Handle to the linked list
   *
   * \remarks
   * All items on the list will be removed.  If those items contain elements
   * allocated from the heap, or if the items themselves were allocated from
   * the heap, a leak will likely occur.  Care should be taken to insure that
   * either only static information is contained in the list, or the dynamic
   * information is freed someother way.
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueQclear (BLUE_HANDLE qHandle) ;

#if defined(__cplusplus)
}
#endif
/** \} */
#endif
