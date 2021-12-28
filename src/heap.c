/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/lock.h"
#include "ofc/console.h"
#include "ofc/thread.h"
#include "ofc/process.h"
#include "ofc/heap.h"

#include "ofc/impl/heapimpl.h"

struct blueheap_chunk
{
  OFC_SIZET alloc_size ;
#if defined(OFC_HEAP_DEBUG)
  struct blueheap_chunk * dbgnext ;
  struct blueheap_chunk * dbgprev ;
  OFC_BOOL snap ;
#if defined(__GNUC__) && defined(BLUE_STACK_TRACE)
  OFC_VOID *caller1 ;
  OFC_VOID *caller2 ;
  OFC_VOID *caller3 ;
  OFC_VOID *caller4 ;
#else
  BLUE_VOID *caller ;
#endif
#endif
} ;

typedef struct
{
  BLUE_LOCK lock ;
  OFC_UINT32 Max ;
  OFC_UINT32 Total ;
#if defined(OFC_HEAP_DEBUG)
  struct blueheap_chunk *Allocated ;
#endif
} BLUE_HEAP_STATS ;

static BLUE_HEAP_STATS BlueHeapStats = {0};

static OFC_VOID
BlueHeapMallocAcct (OFC_SIZET size, struct blueheap_chunk *chunk)
{
  /*
   * Put on the allocation queue
   */
  chunk->alloc_size = size ;
  BlueLock (BlueHeapStats.lock) ;
  BlueHeapStats.Total += size ;
  if (BlueHeapStats.Total >= BlueHeapStats.Max)
    BlueHeapStats.Max = BlueHeapStats.Total ;
  BlueUnlock (BlueHeapStats.lock) ;
}

static OFC_VOID
BlueHeapFreeAcct (struct blueheap_chunk *chunk)
{
  BlueLock (BlueHeapStats.lock) ;
  BlueHeapStats.Total -= chunk->alloc_size ;
  BlueUnlock (BlueHeapStats.lock) ;
}

OFC_CORE_LIB OFC_VOID
BlueHeapLoad (OFC_VOID)
{
  BlueHeapStats.Max = 0 ;
  BlueHeapStats.Total = 0 ;
#if defined(OFC_HEAP_DEBUG)
  BlueHeapStats.Allocated = OFC_NULL ;
#endif
  BlueHeapInitImpl () ;
  BlueHeapStats.lock = BlueLockInit () ;
}

OFC_CORE_LIB OFC_VOID
BlueHeapUnload (OFC_VOID)
{
  BlueLockDestroy (BlueHeapStats.lock) ;
  BlueHeapStats.lock = OFC_NULL;
  BlueHeapUnloadImpl() ;
  BlueHeapDump();
}

#if defined(OFC_HEAP_DEBUG)
static OFC_VOID
BlueHeapDebugAlloc (OFC_SIZET size, struct blueheap_chunk *chunk,
                    OFC_VOID *ret)
{
  BlueLock (BlueHeapStats.lock) ;
  chunk->dbgnext = BlueHeapStats.Allocated ;
  if (BlueHeapStats.Allocated != OFC_NULL)
    BlueHeapStats.Allocated->dbgprev = chunk ;
  BlueHeapStats.Allocated = chunk ;
  chunk->dbgprev = 0 ;

#if defined(__GNUC__) && defined(BLUE_STACK_TRACE)
#if defined(__cyg_profile)
  chunk->caller1 = __cyg_profile_return_address(1) ;
  chunk->caller2 = __cyg_profile_return_address(2) ;
  chunk->caller3 = __cyg_profile_return_address(3) ;
  chunk->caller4 = __cyg_profile_return_address(4) ;
#else
  chunk->caller1 = __builtin_return_address(1) ;
  chunk->caller2 = __builtin_return_address(2) ;
  chunk->caller3 = __builtin_return_address(3) ;
  chunk->caller4 = __builtin_return_address(4) ;
#endif
#else
  chunk->caller = ret ;
#endif
  chunk->snap = OFC_FALSE ;

  BlueUnlock (BlueHeapStats.lock) ;
}
#endif

#if defined(OFC_HEAP_DEBUG)
static OFC_VOID
BlueHeapDebugFree (struct blueheap_chunk *chunk)
{
  /*
   * Pull off the allocation queue
   */
  BlueLock (BlueHeapStats.lock) ;
  if (chunk->dbgprev != OFC_NULL)
    chunk->dbgprev->dbgnext = chunk->dbgnext ;
  else
    BlueHeapStats.Allocated = chunk->dbgnext ;
  if (chunk->dbgnext != OFC_NULL)
    chunk->dbgnext->dbgprev = chunk->dbgprev ;
#if defined(__GNUC__) && defined(BLUE_STACK_TRACE)
#if defined(__cyg_profile)
  chunk->caller1 = __cyg_profile_return_address(1) ;
  chunk->caller2 = __cyg_profile_return_address(2) ;
  chunk->caller3 = __cyg_profile_return_address(3) ;
  chunk->caller4 = __cyg_profile_return_address(4) ;
#else
  chunk->caller1 = __builtin_return_address(1) ;
  chunk->caller2 = __builtin_return_address(2) ;
  chunk->caller3 = __builtin_return_address(3) ;
  chunk->caller4 = __builtin_return_address(4) ;
#endif
#endif
  BlueUnlock (BlueHeapStats.lock) ;
}
#endif

#define OBUF_SIZE 200
OFC_CORE_LIB OFC_VOID
BlueHeapDumpStats (OFC_VOID)
{
  OFC_CHAR obuf[OBUF_SIZE] ;
  OFC_SIZET len ;
  
  len = BlueCsnprintf (obuf, OBUF_SIZE,
		       "Total Allocated Memory %d, Max Allocated Memory %d\n",
		       BlueHeapStats.Total, BlueHeapStats.Max) ;
  ofc_write_console (obuf) ;
}

OFC_CORE_LIB OFC_VOID
BlueHeapDump (OFC_VOID)
{
#if defined(OFC_HEAP_DEBUG)
  struct blueheap_chunk *chunk ;
  OFC_CHAR obuf[OBUF_SIZE] ;
  OFC_SIZET len ;
#endif

  BlueHeapDumpStats() ;
#if defined(OFC_HEAP_DEBUG)
#if defined(__GNUC__) && defined(BLUE_STACK_TRACE)
  if (BlueHeapStats.Allocated == OFC_NULL)
    {
      len = BlueCsnprintf (obuf, OBUF_SIZE,
			   "\nHeap is Empty, No leaks detected\n");
      ofc_write_console (obuf) ;
    }
  else
    {
      len = BlueCsnprintf (obuf, OBUF_SIZE,
			   "%-10s %-10s %-10s %-10s %-10s %-10s\n",
			   "Address", "Size", "Caller1", "Caller2", "Caller3", 
			   "Caller4") ;
      ofc_write_console (obuf) ;
    }

  BlueLock (BlueHeapStats.lock) ;
  for (chunk = BlueHeapStats.Allocated ;
       chunk != OFC_NULL ;
       chunk = chunk->dbgnext)
    {
      if (!chunk->snap)
	{
#if defined(__cyg_profile)
	  len = BlueCsnprintf (obuf, OBUF_SIZE, 
			       "%-10p %-10d %-10s %-10s %-10s %-10s\n",
			       chunk+1, (BLUE_INT) chunk->alloc_size, 
			       __cyg_profile_addr2sym(chunk->caller1), 
			       __cyg_profile_addr2sym(chunk->caller2), 
			       __cyg_profile_addr2sym(chunk->caller3), 
			       __cyg_profile_addr2sym(chunk->caller4)) ;
#else
	  len = BlueCsnprintf (obuf, OBUF_SIZE, 
			       "%-10p %-10d %-10p %-10p %-10p %-10p\n",
			       chunk+1, (OFC_INT) chunk->alloc_size,
			       chunk->caller1, chunk->caller2, chunk->caller3, 
			       chunk->caller4) ;
#endif
	  ofc_write_console (obuf) ;
	}
    }
  BlueUnlock (BlueHeapStats.lock) ;
#else
  len = BlueCsnprintf (obuf, OBUF_SIZE, "%-20s %-10s %-20s\n",
		       "Address", "Size", "Caller") ;
  BlueWriteConsole (obuf) ;
  BlueLock (BlueHeapStats.lock) ;
  for (chunk = BlueHeapStats.Allocated ;
       chunk != BLUE_NULL ;
       chunk = chunk->dbgnext)
    {
      len = BlueCsnprintf (obuf, OBUF_SIZE, "%-20p %-10d %-20p\n",
			   chunk+1, chunk->alloc_size, chunk->caller) ;
      BlueWriteConsole (obuf) ;
    }
  BlueUnlock (BlueHeapStats.lock) ;
#endif
  len = BlueCsnprintf (obuf, OBUF_SIZE, "\n") ;
  ofc_write_console (obuf) ;
#endif
}

OFC_CORE_LIB OFC_VOID
BlueHeapCheckAlloc (OFC_LPCVOID mem)
{
  const struct blueheap_chunk * chunk ;

  if (mem != OFC_NULL)
    {
      chunk = mem ;
      chunk-- ;

      BlueHeapCheckAllocImpl (chunk) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueHeapSnap (OFC_VOID)
{
#if defined(OFC_HEAP_DEBUG)
  struct blueheap_chunk *chunk ;

  BlueLock (BlueHeapStats.lock) ;
  for (chunk = BlueHeapStats.Allocated ;
       chunk != OFC_NULL ;
       chunk = chunk->dbgnext)
    {
      chunk->snap = OFC_TRUE ;
    }

  BlueUnlock (BlueHeapStats.lock) ;
#endif
}

OFC_CORE_LIB OFC_VOID
BlueHeapDumpChunk (OFC_LPVOID mem)
{
#if defined(OFC_HEAP_DEBUG)
#if defined(__GNUC__) && defined(BLUE_STACK_TRACE)
  struct blueheap_chunk * chunk ;

  if (mem != OFC_NULL)
    {
      chunk = mem ;
      chunk-- ;

      BlueCprintf ("%-10p %-10d %-10p %-10p %-10p %-10p\n",
		   chunk+1, (OFC_INT) chunk->alloc_size,
		   chunk->caller1, chunk->caller2, chunk->caller3, 
		   chunk->caller4) ;
    }
#endif
#endif
}

OFC_CORE_LIB OFC_LPVOID
BlueHeapMalloc (OFC_SIZET size)
{
  OFC_LPVOID mem ;
  struct blueheap_chunk * chunk ;

  mem = OFC_NULL ;
  chunk = BlueHeapMallocImpl (size + sizeof (struct blueheap_chunk)) ;
  if (chunk != OFC_NULL)
    {
      BlueHeapMallocAcct (size, chunk) ;
#if defined(OFC_HEAP_DEBUG)
      BlueHeapDebugAlloc (size, chunk, RETURN_ADDRESS()) ;
#endif
      mem = (OFC_LPVOID) (++chunk) ;
    }
  else
    {
      BlueProcessCrash ("BlueHeapMalloc Allocation Failed\n") ;
    }
  return (mem) ;
}

OFC_CORE_LIB OFC_LPVOID
BlueHeapCalloc (OFC_SIZET nmemb, OFC_SIZET size)
{
  return (BlueHeapMalloc (nmemb * size)) ;
}

OFC_CORE_LIB OFC_VOID
BlueHeapFree (OFC_LPVOID mem)
{
  struct blueheap_chunk * chunk ;

  if (mem != OFC_NULL)
    {
      chunk = mem ;
      chunk-- ;

#if defined(OFC_HEAP_DEBUG)
      BlueHeapDebugFree (chunk) ;
#endif
      BlueHeapFreeAcct (chunk) ;
      BlueHeapFreeImpl (chunk) ;
    }
}

OFC_CORE_LIB OFC_LPVOID
BlueHeapRealloc (OFC_LPVOID ptr, OFC_SIZET size)
{
  struct blueheap_chunk * chunk ;
  struct blueheap_chunk * newchunk ;
  OFC_LPVOID mem ;

  mem = OFC_NULL ;
  chunk = ptr ;
  if (chunk != OFC_NULL)
    {
      chunk-- ;

      BlueHeapFreeAcct (chunk) ;

#if defined(OFC_HEAP_DEBUG)
      BlueHeapDebugFree (chunk) ;
#endif

      newchunk = BlueHeapReallocImpl (chunk, 
				      size + sizeof (struct blueheap_chunk)) ;

      if (newchunk != OFC_NULL)
	{
#if defined(OFC_HEAP_DEBUG)
	  BlueHeapDebugAlloc (size, newchunk, RETURN_ADDRESS()) ;
#endif
	  BlueHeapMallocAcct (size, newchunk) ;
	  chunk = newchunk ;
	  mem = chunk + 1 ;
	}
      else
	{
	  BlueProcessCrash ("BlueHeapRealloc Allocation Failed\n") ;
	}
    }
  else
    mem = BlueHeapMalloc (size) ;

  return (mem) ;
}

