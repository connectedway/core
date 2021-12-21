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
#if !defined(__BLUE_HEAP_H__)
#define __BLUE_HEAP_H__

#include "ofc/core.h"
#include "ofc/types.h"

/**
 * \defgroup BluePort Platform Specific Abstraction
 *
 * Blue Share is designed as a portable package that can be ported to
 * most any target platform.  To accomodate this and ease platform 
 * integration, Blue Share provides a porting abstraction.
 *
 * To port Blue Share to a new platform, simple implement these routines
 * and target them at your desired platform
 */

/**
 * \defgroup BlueHeap Heap Facility
 * \ingroup BluePort
 *
 * Blue Share provides routines that abstract heap functions.  A port
 * can map these functions to underlying platform specific heap functions
 * or can use Blue Share's heap management facility.
 *
 * The documentation of these APIs may include specific information relevant
 * only to the Blue Share heap
 *
 * \{ 
 */
#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Initialize the Heap
   *
   * This function is only called by BlueInit.  It will initialize the
   * heap for use.  This may be a noop on many platforms
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueHeapLoad (BLUE_VOID) ;
  /**
   * Unload the heap
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueHeapUnload (BLUE_VOID) ;
  /**
   * Deallocate a chunk of memory
   *
   * This may be mapped to a free on many platforms.
   *
   * \param mem
   * A pointer to the memory to deallocate.
   *
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueHeapFree (BLUE_LPVOID mem) ;
  BLUE_CORE_LIB BLUE_VOID 
  BlueHeapCheckAlloc (BLUE_LPCVOID mem) ;
  BLUE_CORE_LIB BLUE_VOID
  BlueHeapDumpChunk (BLUE_LPVOID mem) ;
  /**
   * Allocate a chunk of memory
   *
   * This may be mapped to a malloc on many platforms
   *
   * \param size
   * size of the memory block to allocate
   *
   * \returns
   * Nothing
   */
  BLUE_CORE_LIB BLUE_LPVOID 
  BlueHeapMalloc (BLUE_SIZET size) ;

  BLUE_CORE_LIB BLUE_LPVOID 
  BlueHeapCalloc (BLUE_SIZET nmemb, BLUE_SIZET size) ;
  /**
   * Change the size (reallocate) a chunk of memory
   *
   * This may be mapped to a realloc on many platforms
   *
   * \param ptr
   * Pointer to the initial chunk of memory
   *
   * \param size
   * size of the new memory block
   *
   * \returns
   * Pointer to the new memory block.
   *
   * \remarks
   * Reallocating a chunk of memory to a size within the same power of two
   * 2^x to the 2^(x+1) will return the pointer passed in.
   */
  BLUE_CORE_LIB BLUE_LPVOID 
  BlueHeapRealloc (BLUE_LPVOID ptr, BLUE_SIZET size) ;
  /**
   * Dump Blue Heap Stats Usuage
   * 
   * This will print to the console the number of bytes used as well as the
   * maximum number of bytes that had been used 
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueHeapDumpStats (BLUE_VOID) ;
  /**
   * Dump Blue Heap Trace
   *
   * This will print to the console a record for each chunk of memory that
   * had been allocated includding who allocated it, the address of the
   * chunk and the size of the chunk.
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueHeapDump (BLUE_VOID) ;
  /**
   * Snap the Blue Heap
   * 
   * Mark all allocated blocks in the heap as previously allocated.  Only
   * those chunks that are 'not snapped' will be printed upon a dump
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueHeapSnap (BLUE_VOID) ;
#if defined(__cplusplus)
}
#endif
/** \} */
#endif
