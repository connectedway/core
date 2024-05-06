/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_HEAP_H__)
#define __OFC_HEAP_H__

#include "ofc/core.h"
#include "ofc/types.h"

/** 
 * \{
 * \defgroup heap Open Files Heap Handling
 *
 * Open Files provides routines that abstract heap functions.  A port
 * can map these functions to underlying platform specific heap functions
 * or can use Open Files's heap management facility.
 *
 * The documentation of these APIs may include specific information relevant
 * only to the Open Files heap
 *
 * Heap Functions:
 *
 * Function | Description
 * ---------|-------------
 * \ref ofc_heap_load | Initialize Heap Subsystem
 * \ref ofc_heap_unload | Deinitialize Heap Subsystem
 * \ref ofc_free | Free a chunk of memory
 * \ref ofc_heap_check_alloc | Check if a heap block is allocated
 * \ref ofc_heap_dump_chunk | Dump info on heap chunk
 * \ref ofc_malloc | Allocate a chunk
 * \ref ofc_calloc | Allocate and initialize chunk
 * \ref ofc_realloc | Realloc a chunk of memory
 * \ref ofc_heap_dump_stats | Dump Heap Statistics
 * \ref ofc_heap_dump | Dump info all all allocated chunks
 * \ref ofc_heap_snap | Mark currently allocated memory as valid
 */

#if defined(__cplusplus)
extern "C"
{
#endif
/**
 * Initialize the Heap
 *
 * This function is only called by ofc_init.  It will initialize the
 * heap for use.  This may be a noop on many platforms
 */
OFC_CORE_LIB OFC_VOID
ofc_heap_load(OFC_VOID);
/**
 * Unload the heap
 */
OFC_CORE_LIB OFC_VOID
ofc_heap_unload(OFC_VOID);
/**
 * Deallocate a chunk of memory
 *
 * This may be mapped to a free on many platforms.
 *
 * \param mem
 * A pointer to the memory to deallocate.
 *
 */
OFC_CORE_LIB OFC_VOID
ofc_free(OFC_LPVOID mem);
  /**
   * Check the allocation of a heap.
   *
   * This will crash if the memory is not allocated
   *
   * \param mem
   * Pointer to memory chunk
   */
OFC_CORE_LIB OFC_VOID
ofc_heap_check_alloc(OFC_LPCVOID mem);
  /**
   * Dump the debug status of a memory chunk
   *
   * \param mem
   * Pointer to memory chunk
   */
OFC_CORE_LIB OFC_VOID
ofc_heap_dump_chunk(OFC_LPVOID mem);
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
OFC_CORE_LIB OFC_LPVOID
ofc_malloc(OFC_SIZET size);
  /**
   * Allocate and initiaize chunk of memory
   *
   * \param nmemb
   * Number of structure to allocate
   *
   * \param size
   * size of structure
   *
   * \returns
   * Pointer to chunk
   */
OFC_CORE_LIB OFC_LPVOID
ofc_calloc(OFC_SIZET nmemb, OFC_SIZET size);
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
OFC_CORE_LIB OFC_LPVOID
ofc_realloc(OFC_LPVOID ptr, OFC_SIZET size);
/**
 * Dump Heap Stats Usuage
 *
 * This will print to the console the number of bytes used as well as the
 * maximum number of bytes that had been used
 */
OFC_CORE_LIB OFC_VOID
ofc_heap_dump_stats(OFC_VOID);
/**
 * Dump Heap Trace
 *
 * This will print to the console a record for each chunk of memory that
 * had been allocated includding who allocated it, the address of the
 * chunk and the size of the chunk.
 */
OFC_CORE_LIB OFC_VOID
ofc_heap_dump(OFC_VOID);
/**
 * Snap the Heap
 *
 * Mark all allocated blocks in the heap as previously allocated.  Only
 * those chunks that are 'not snapped' will be printed upon a dump
 */
OFC_CORE_LIB OFC_VOID
ofc_heap_snap(OFC_VOID);

#if defined(__cplusplus)
}
#endif
/** \} */
#endif
