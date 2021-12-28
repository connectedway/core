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
  OFC_CORE_LIB OFC_HANDLE
  BlueQcreate (OFC_VOID) ;
  /**
   * Destroy a queue
   *
   * \param qHead
   * Head of queue
   */
  OFC_CORE_LIB OFC_VOID
  BlueQdestroy (OFC_HANDLE qHead) ;
  /**
   * Add an element to the end of the list
   *
   * \param qHead
   * Pointer to list header
   *
   * \param qElement 
   * Pointer to element to add to list
   */
  OFC_CORE_LIB OFC_VOID
  BlueQenqueue (OFC_HANDLE qHead, OFC_VOID *qElement) ;
  /**
   * Remove an element from the front of the list
   *
   * \param qHead
   * Pointer to list header
   * 
   * \returns
   * Item at the front or OFC_NULL
   */
  OFC_CORE_LIB OFC_VOID *
  BlueQdequeue (OFC_HANDLE qHead) ;
  /**
   * See if queue is empty
   *
   * \param qHead
   * Queue Head
   *
   * \returns
   * OFC_TRUE if empty, OFC_FALSE otherwise
   */
  OFC_CORE_LIB OFC_BOOL
  BlueQempty (OFC_HANDLE qHead) ;
  /**
   * Return the head of the linked list
   *
   * \param qHead
   * Pointer to the head of the list
   *
   * \returns
   * Element that was at the front of the list
   */
  OFC_CORE_LIB OFC_VOID *
  BlueQfirst (OFC_HANDLE qHead) ;
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
  OFC_CORE_LIB OFC_VOID *
  BlueQnext (OFC_HANDLE qHead, OFC_VOID *qElement) ;
  /**
   * Unlink the current element from the list
   *
   * \param qHead 
   * Pointer to the head of the list
   *
   * \param qElement
   * Pointer to the current element
   */
  OFC_CORE_LIB OFC_VOID
  BlueQunlink (OFC_HANDLE qHead, OFC_VOID *qElement) ;
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
  OFC_CORE_LIB OFC_VOID
  BlueQclear (OFC_HANDLE qHandle) ;

#if defined(__cplusplus)
}
#endif
/** \} */
#endif
