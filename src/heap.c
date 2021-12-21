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
#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/lock.h"
#include "ofc/console.h"
#include "ofc/thread.h"
#include "ofc/process.h"

#include "ofc/impl/heapimpl.h"

struct blueheap_chunk
{
  BLUE_SIZET alloc_size ;
#if defined(BLUE_PARAM_HEAP_DEBUG)
  struct blueheap_chunk * dbgnext ;
  struct blueheap_chunk * dbgprev ;
  BLUE_BOOL snap ;
#if defined(__GNUC__) && defined(BLUE_STACK_TRACE)
  BLUE_VOID *caller1 ;
  BLUE_VOID *caller2 ;
  BLUE_VOID *caller3 ;
  BLUE_VOID *caller4 ;
#else
  BLUE_VOID *caller ;
#endif
#endif
} ;

typedef struct
{
  BLUE_LOCK lock ;
  BLUE_UINT32 Max ;
  BLUE_UINT32 Total ;
#if defined(BLUE_PARAM_HEAP_DEBUG)
  struct blueheap_chunk *Allocated ;
#endif
} BLUE_HEAP_STATS ;

static BLUE_HEAP_STATS BlueHeapStats = {0};

static BLUE_VOID 
BlueHeapMallocAcct (BLUE_SIZET size, struct blueheap_chunk *chunk)
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

static BLUE_VOID 
BlueHeapFreeAcct (struct blueheap_chunk *chunk)
{
  BlueLock (BlueHeapStats.lock) ;
  BlueHeapStats.Total -= chunk->alloc_size ;
  BlueUnlock (BlueHeapStats.lock) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueHeapLoad (BLUE_VOID)
{
  BlueHeapStats.lock = BlueLockInit () ;
  BlueHeapStats.Max = 0 ;
  BlueHeapStats.Total = 0 ;
#if defined(BLUE_PARAM_HEAP_DEBUG)
  BlueHeapStats.Allocated = BLUE_NULL ;
#endif
  BlueHeapInitImpl () ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueHeapUnload (BLUE_VOID)
{
  BlueLockDestroy (BlueHeapStats.lock) ;
  BlueHeapUnloadImpl() ;
}

#if defined(BLUE_PARAM_HEAP_DEBUG)
static BLUE_VOID 
BlueHeapDebugAlloc (BLUE_SIZET size, struct blueheap_chunk *chunk,
		    BLUE_VOID *ret)
{
  BlueLock (BlueHeapStats.lock) ;
  chunk->dbgnext = BlueHeapStats.Allocated ;
  if (BlueHeapStats.Allocated != BLUE_NULL)
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
  chunk->snap = BLUE_FALSE ;

  BlueUnlock (BlueHeapStats.lock) ;
}
#endif

#if defined(BLUE_PARAM_HEAP_DEBUG)
static BLUE_VOID 
BlueHeapDebugFree (struct blueheap_chunk *chunk)
{
  /*
   * Pull off the allocation queue
   */
  BlueLock (BlueHeapStats.lock) ;
  if (chunk->dbgprev != BLUE_NULL)
    chunk->dbgprev->dbgnext = chunk->dbgnext ;
  else
    BlueHeapStats.Allocated = chunk->dbgnext ;
  if (chunk->dbgnext != BLUE_NULL)
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
BLUE_CORE_LIB BLUE_VOID 
BlueHeapDumpStats (BLUE_VOID)
{
  BLUE_CHAR obuf[OBUF_SIZE] ;
  BLUE_SIZET len ;
  
  len = BlueCsnprintf (obuf, OBUF_SIZE,
		       "Total Allocated Memory %d, Max Allocated Memory %d\n",
		       BlueHeapStats.Total, BlueHeapStats.Max) ;
  BlueWriteConsole (obuf) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueHeapDump (BLUE_VOID)
{
#if defined(BLUE_PARAM_HEAP_DEBUG)
  struct blueheap_chunk *chunk ;
  BLUE_CHAR obuf[OBUF_SIZE] ;
  BLUE_SIZET len ;
#endif

  BlueHeapDumpStats() ;
#if defined(BLUE_PARAM_HEAP_DEBUG)
#if defined(__GNUC__) && defined(BLUE_STACK_TRACE)
  len = BlueCsnprintf (obuf, OBUF_SIZE, "%-10s %-10s %-10s %-10s %-10s %-10s\n",
		       "Address", "Size", "Caller1", "Caller2", "Caller3", 
		       "Caller4") ;
  BlueWriteConsole (obuf) ;

  BlueLock (BlueHeapStats.lock) ;
  for (chunk = BlueHeapStats.Allocated ;
       chunk != BLUE_NULL ;
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
			       chunk+1, (BLUE_INT) chunk->alloc_size, 
			       chunk->caller1, chunk->caller2, chunk->caller3, 
			       chunk->caller4) ;
#endif
	  BlueWriteConsole (obuf) ;
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
  BlueWriteConsole (obuf) ;
#endif
}

BLUE_CORE_LIB BLUE_VOID
BlueHeapCheckAlloc (BLUE_LPCVOID mem)
{
  const struct blueheap_chunk * chunk ;

  if (mem != BLUE_NULL)
    {
      chunk = mem ;
      chunk-- ;

      BlueHeapCheckAllocImpl (chunk) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueHeapSnap (BLUE_VOID)
{
#if defined(BLUE_PARAM_HEAP_DEBUG)
  struct blueheap_chunk *chunk ;

  BlueLock (BlueHeapStats.lock) ;
  for (chunk = BlueHeapStats.Allocated ;
       chunk != BLUE_NULL ;
       chunk = chunk->dbgnext)
    {
      chunk->snap = BLUE_TRUE ;
    }

  BlueUnlock (BlueHeapStats.lock) ;
#endif
}

BLUE_CORE_LIB BLUE_VOID
BlueHeapDumpChunk (BLUE_LPVOID mem)
{
#if defined(BLUE_PARAM_HEAP_DEBUG)
#if defined(__GNUC__) && defined(BLUE_STACK_TRACE)
  struct blueheap_chunk * chunk ;

  if (mem != BLUE_NULL)
    {
      chunk = mem ;
      chunk-- ;

      BlueCprintf ("%-10p %-10d %-10p %-10p %-10p %-10p\n",
		   chunk+1, (BLUE_INT) chunk->alloc_size, 
		   chunk->caller1, chunk->caller2, chunk->caller3, 
		   chunk->caller4) ;
    }
#endif
#endif
}

BLUE_CORE_LIB BLUE_LPVOID 
BlueHeapMalloc (BLUE_SIZET size)
{
  BLUE_LPVOID mem ;
  struct blueheap_chunk * chunk ;

  mem = BLUE_NULL ;
  chunk = BlueHeapMallocImpl (size + sizeof (struct blueheap_chunk)) ;
  if (chunk != BLUE_NULL)
    {
      BlueHeapMallocAcct (size, chunk) ;
#if defined(BLUE_PARAM_HEAP_DEBUG)
      BlueHeapDebugAlloc (size, chunk, RETURN_ADDRESS()) ;
#endif
      mem = (BLUE_LPVOID) (++chunk) ;
    }
  else
    {
      BlueProcessCrash ("BlueHeapMalloc Allocation Failed\n") ;
    }
  return (mem) ;
}

BLUE_CORE_LIB BLUE_LPVOID 
BlueHeapCalloc (BLUE_SIZET nmemb, BLUE_SIZET size)
{
  return (BlueHeapMalloc (nmemb * size)) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueHeapFree (BLUE_LPVOID mem)
{
  struct blueheap_chunk * chunk ;

  if (mem != BLUE_NULL)
    {
      chunk = mem ;
      chunk-- ;

#if defined(BLUE_PARAM_HEAP_DEBUG)
      BlueHeapDebugFree (chunk) ;
#endif
      BlueHeapFreeAcct (chunk) ;
      BlueHeapFreeImpl (chunk) ;
    }
}

BLUE_CORE_LIB BLUE_LPVOID 
BlueHeapRealloc (BLUE_LPVOID ptr, BLUE_SIZET size)
{
  struct blueheap_chunk * chunk ;
  struct blueheap_chunk * newchunk ;
  BLUE_LPVOID mem ;

  mem = BLUE_NULL ;
  chunk = ptr ;
  if (chunk != BLUE_NULL)
    {
      chunk-- ;

      BlueHeapFreeAcct (chunk) ;

#if defined(BLUE_PARAM_HEAP_DEBUG)
      BlueHeapDebugFree (chunk) ;
#endif

      newchunk = BlueHeapReallocImpl (chunk, 
				      size + sizeof (struct blueheap_chunk)) ;

      if (newchunk != BLUE_NULL)
	{
#if defined(BLUE_PARAM_HEAP_DEBUG)
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

