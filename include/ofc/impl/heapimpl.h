/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_HEAP_IMPL_H__)
#define __OFC_HEAP_IMPL_H__

#include "ofc/types.h"

/**
 * \defgroup port Platform Specific Abstraction
 *
 * Open Files is designed as a portable package that can be ported to
 * most any target platform.  To accomodate this and ease platform 
 * integration, Open Files provides a porting abstraction.
 *
 * To port Open Files to a new platform, simple implement these routines
 * and target them at your desired platform
 */

/**
 * \defgroup heap Heap Facility
 * \ingroup port
 *
 * Open Files provides routines that abstract heap functions.  A port
 * can map these functions to underlying platform specific heap functions
 * or can use Open Files's heap management facility.
 *
 * The documentation of these APIs may include specific information relevant
 * only to the heap
 *
 * The binary heap algorithm is described below:
 *
 * The heap algorithm used is simple, quick and relatively efficient and is
 * designed to provide deterministic behavior.  The heap is organized as an
 * array of linked lists, each list containing blocks of the same size.  The
 * size of the blocks is a power of 2, where the power is the index into
 * the array.  So, the blocks on the list at offset 5 into the array are 
 * 2^5 or 32 bytes in size.  
 *
 * The number of entries in the array determines the maximum size of a
 * memory block and is defined in the configuration as 
 * OFC_HEAP_POWER.
 *
 * The heap is statically allocated in the file heap.c and is a power of 2
 * bytes in size.  This power of this size must be less then 
 * OFC_HEAP_POWER.  To initialize the heap, all entries in the array
 * are cleared and the static heap is added to block list corresponding to
 * to the power of it's size.
 *
 * To allocate from the heap, the requested size is rounded up to the 
 * nearest power of two and a block from the power's respective linked list
 * is returned.  If no blocks at that power are available, then a block
 * from the next higher power is removed from that power, split in two,
 * one block is added to the power's list and the other is allocated.
 * This transformation of a higher power into two lower powers occurs in
 * a recursive fashion.  So in the initial state, the heap has only 
 * one entry for one congiuous block at the top of the power table. 
 * The first allocation will cause the heap to be split up into a collection
 * of blocks of all powers of two greater then then the power of the
 * allocation.  In other words, if the first allocation is for 72 bytes, it
 * is rounded up to 128 bytes (2^7).  The initial block at entry
 * OFC_HEAP_POWER will be split into two, one is added to the list
 * at entry OFC_HEAP_POWER-1 and the other is split and added to
 * list at OFC_HEAP_POWER-2 and so on until a block of 2^7th is 
 * available and able to satisfy the allocation request.
 *
 * The heap will converge on a steady state where sufficient blocks of each
 * power size are available.  The algorithm becomes deterministic because
 * after steady state has been achieved, the heap will no longer fragment.
 * The algorithm is effecient because after steady state has been achieved,
 * an allocation is a simple removal of a corresponding block from the power
 * table.  The memory wasted in this algorithm is estimated at 25%.  On
 * average a memory allocation of a chunk greater then 2^(x-1) but less then 
 * 2^x, will be 1.5 * 2^x.  For example, the average alloction for blocks
 * between 64 and 128 bytes is 96 bytes.  So on average, an allocation of 
 * 128 bytes will be used to service 96 bytes.  This requires the heap 
 * to be 33% greater then needed.
 *
 * Another efficiency to the algorithm is that memory reallocations within
 * the same power of two size is essentially a no-op.  This is especially
 * useful in network protocol algorithms where messages are reallocated
 * as they are processed.
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
 * This function is only called by ofc_init.  It will initialize the
 * heap for use.  This may be a noop on many platforms
 */
OFC_VOID ofc_heap_init_impl(OFC_VOID);

/**
 * Unload the heap implementation
 */
OFC_VOID ofc_heap_unload_impl(OFC_VOID);

/**
 * Unmap the heap implementation
 * This should be called by the heap manager after calling unload and
 * after checking for any left over crumps
 */
OFC_VOID ofc_heap_unmap_impl(OFC_VOID);

/**
 * Deallocate a chunk of memory
 *
 * This may be mapped to a free on many platforms.
 *
 * \param mem
 * A pointer to the memory to deallocate.
 */
OFC_VOID ofc_free_impl(OFC_LPVOID mem);

OFC_VOID ofc_heap_check_alloc_impl(OFC_LPCVOID mem);

/**
 * Allocate a chunk of memory
 *
 * This may be mapped to a malloc on many platforms
 *
 * \param size
 * size of the memory block to allocate
 */
OFC_LPVOID ofc_malloc_impl(OFC_SIZET size);

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
OFC_LPVOID ofc_realloc_impl(OFC_LPVOID ptr,
                            OFC_SIZET size);

#if defined(__cplusplus)
}
#endif
/** \} */
#endif
