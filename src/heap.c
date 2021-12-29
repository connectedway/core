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

struct heap_chunk
{
  OFC_SIZET alloc_size ;
#if defined(OFC_HEAP_DEBUG)
  struct heap_chunk * dbgnext ;
  struct heap_chunk * dbgprev ;
  OFC_BOOL snap ;
#if defined(__GNUC__) && defined(OFC_STACK_TRACE)
  OFC_VOID *caller1 ;
  OFC_VOID *caller2 ;
  OFC_VOID *caller3 ;
  OFC_VOID *caller4 ;
#else
  OFC_VOID *caller ;
#endif
#endif
} ;

typedef struct
{
  OFC_LOCK lock ;
  OFC_UINT32 Max ;
  OFC_UINT32 Total ;
#if defined(OFC_HEAP_DEBUG)
  struct heap_chunk *Allocated ;
#endif
} OFC_HEAP_STATS ;

static OFC_HEAP_STATS ofc_heap_stats = {0};

static OFC_VOID
ofc_heap_malloc_acct (OFC_SIZET size, struct heap_chunk *chunk)
{
  /*
   * Put on the allocation queue
   */
  chunk->alloc_size = size ;
  ofc_lock (ofc_heap_stats.lock) ;
  ofc_heap_stats.Total += size ;
  if (ofc_heap_stats.Total >= ofc_heap_stats.Max)
    ofc_heap_stats.Max = ofc_heap_stats.Total ;
  ofc_unlock (ofc_heap_stats.lock) ;
}

static OFC_VOID
ofc_heap_free_acct (struct heap_chunk *chunk)
{
  ofc_lock (ofc_heap_stats.lock) ;
  ofc_heap_stats.Total -= chunk->alloc_size ;
  ofc_unlock (ofc_heap_stats.lock) ;
}

OFC_CORE_LIB OFC_VOID
ofc_heap_load (OFC_VOID)
{
  ofc_heap_stats.Max = 0 ;
  ofc_heap_stats.Total = 0 ;
#if defined(OFC_HEAP_DEBUG)
  ofc_heap_stats.Allocated = OFC_NULL ;
#endif
  ofc_heap_init_impl () ;
  ofc_heap_stats.lock = ofc_lock_init () ;
}

OFC_CORE_LIB OFC_VOID
ofc_heap_unload (OFC_VOID)
{
  ofc_lock_destroy (ofc_heap_stats.lock) ;
  ofc_heap_stats.lock = OFC_NULL;
  ofc_heap_unload_impl() ;
  ofc_heap_dump();
}

#if defined(OFC_HEAP_DEBUG)
static OFC_VOID
ofc_heap_debug_alloc (OFC_SIZET size, struct heap_chunk *chunk,
                    OFC_VOID *ret)
{
  ofc_lock (ofc_heap_stats.lock) ;
  chunk->dbgnext = ofc_heap_stats.Allocated ;
  if (ofc_heap_stats.Allocated != OFC_NULL)
    ofc_heap_stats.Allocated->dbgprev = chunk ;
  ofc_heap_stats.Allocated = chunk ;
  chunk->dbgprev = 0 ;

#if defined(__GNUC__) && defined(OFC_STACK_TRACE)
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

  ofc_unlock (ofc_heap_stats.lock) ;
}
#endif

#if defined(OFC_HEAP_DEBUG)
static OFC_VOID
ofc_heap_debug_free (struct heap_chunk *chunk)
{
  /*
   * Pull off the allocation queue
   */
  ofc_lock (ofc_heap_stats.lock) ;
  if (chunk->dbgprev != OFC_NULL)
    chunk->dbgprev->dbgnext = chunk->dbgnext ;
  else
    ofc_heap_stats.Allocated = chunk->dbgnext ;
  if (chunk->dbgnext != OFC_NULL)
    chunk->dbgnext->dbgprev = chunk->dbgprev ;
#if defined(__GNUC__) && defined(OFC_STACK_TRACE)
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
  ofc_unlock (ofc_heap_stats.lock) ;
}
#endif

#define OBUF_SIZE 200
OFC_CORE_LIB OFC_VOID
ofc_heap_dump_stats (OFC_VOID)
{
  OFC_CHAR obuf[OBUF_SIZE] ;
  OFC_SIZET len ;
  
  len = ofc_snprintf (obuf, OBUF_SIZE,
                      "Total Allocated Memory %d, Max Allocated Memory %d\n",
                      ofc_heap_stats.Total, ofc_heap_stats.Max) ;
  ofc_write_console (obuf) ;
}

OFC_CORE_LIB OFC_VOID
ofc_heap_dump (OFC_VOID)
{
#if defined(OFC_HEAP_DEBUG)
  struct heap_chunk *chunk ;
  OFC_CHAR obuf[OBUF_SIZE] ;
  OFC_SIZET len ;
#endif

  ofc_heap_dump_stats() ;
#if defined(OFC_HEAP_DEBUG)
#if defined(__GNUC__) && defined(OFC_STACK_TRACE)
  if (ofc_heap_stats.Allocated == OFC_NULL)
    {
      len = ofc_snprintf (obuf, OBUF_SIZE,
                          "\nHeap is Empty, No leaks detected\n");
      ofc_write_console (obuf) ;
    }
  else
    {
      len = ofc_snprintf (obuf, OBUF_SIZE,
                          "%-10s %-10s %-10s %-10s %-10s %-10s\n",
                          "Address", "Size", "Caller1", "Caller2", "Caller3",
                          "Caller4") ;
      ofc_write_console (obuf) ;
    }

  ofc_lock (ofc_heap_stats.lock) ;
  for (chunk = ofc_heap_stats.Allocated ;
       chunk != OFC_NULL ;
       chunk = chunk->dbgnext)
    {
      if (!chunk->snap)
	{
#if defined(__cyg_profile)
	  len = ofc_snprintf (obuf, OBUF_SIZE, 
			       "%-10p %-10d %-10s %-10s %-10s %-10s\n",
			       chunk+1, (OFC_INT) chunk->alloc_size, 
			       __cyg_profile_addr2sym(chunk->caller1), 
			       __cyg_profile_addr2sym(chunk->caller2), 
			       __cyg_profile_addr2sym(chunk->caller3), 
			       __cyg_profile_addr2sym(chunk->caller4)) ;
#else
	  len = ofc_snprintf (obuf, OBUF_SIZE,
                          "%-10p %-10d %-10p %-10p %-10p %-10p\n",
			       chunk+1, (OFC_INT) chunk->alloc_size,
                          chunk->caller1, chunk->caller2, chunk->caller3,
                          chunk->caller4) ;
#endif
	  ofc_write_console (obuf) ;
	}
    }
  ofc_unlock (ofc_heap_stats.lock) ;
#else
  len = ofc_snprintf (obuf, OBUF_SIZE, "%-20s %-10s %-20s\n",
		       "Address", "Size", "Caller") ;
  ofc_write_console(obuf) ;
  ofc_lock (ofc_heap_stats.lock) ;
  for (chunk = ofc_heap_stats.Allocated ;
       chunk != OFC_NULL ;
       chunk = chunk->dbgnext)
    {
      len = ofc_snprintf (obuf, OBUF_SIZE, "%-20p %-10d %-20p\n",
			   chunk+1, chunk->alloc_size, chunk->caller) ;
      ofc_write_console (obuf) ;
    }
  ofc_unlock (ofc_heap_stats.lock) ;
#endif
  len = ofc_snprintf (obuf, OBUF_SIZE, "\n") ;
  ofc_write_console (obuf) ;
#endif
}

OFC_CORE_LIB OFC_VOID
ofc_heap_check_alloc (OFC_LPCVOID mem)
{
  const struct heap_chunk * chunk ;

  if (mem != OFC_NULL)
    {
      chunk = mem ;
      chunk-- ;

      ofc_heap_check_alloc_impl (chunk) ;
    }
}

OFC_CORE_LIB OFC_VOID
ofc_heap_snap (OFC_VOID)
{
#if defined(OFC_HEAP_DEBUG)
  struct heap_chunk *chunk ;

  ofc_lock (ofc_heap_stats.lock) ;
  for (chunk = ofc_heap_stats.Allocated ;
       chunk != OFC_NULL ;
       chunk = chunk->dbgnext)
    {
      chunk->snap = OFC_TRUE ;
    }

  ofc_unlock (ofc_heap_stats.lock) ;
#endif
}

OFC_CORE_LIB OFC_VOID
ofc_heap_dump_chunk (OFC_LPVOID mem)
{
#if defined(OFC_HEAP_DEBUG)
#if defined(__GNUC__) && defined(OFC_STACK_TRACE)
  struct heap_chunk * chunk ;

  if (mem != OFC_NULL)
    {
      chunk = mem ;
      chunk-- ;

      ofc_printf ("%-10p %-10d %-10p %-10p %-10p %-10p\n",
		   chunk+1, (OFC_INT) chunk->alloc_size,
                  chunk->caller1, chunk->caller2, chunk->caller3,
                  chunk->caller4) ;
    }
#endif
#endif
}

OFC_CORE_LIB OFC_LPVOID
ofc_malloc (OFC_SIZET size)
{
  OFC_LPVOID mem ;
  struct heap_chunk * chunk ;

  mem = OFC_NULL ;
  chunk = ofc_malloc_impl (size + sizeof (struct heap_chunk)) ;
  if (chunk != OFC_NULL)
    {
      ofc_heap_malloc_acct (size, chunk) ;
#if defined(OFC_HEAP_DEBUG)
      ofc_heap_debug_alloc (size, chunk, RETURN_ADDRESS()) ;
#endif
      mem = (OFC_LPVOID) (++chunk) ;
    }
  else
    {
      ofc_process_crash ("ofc_malloc Allocation Failed\n") ;
    }
  return (mem) ;
}

OFC_CORE_LIB OFC_LPVOID
ofc_calloc (OFC_SIZET nmemb, OFC_SIZET size)
{
  return (ofc_malloc (nmemb * size)) ;
}

OFC_CORE_LIB OFC_VOID
ofc_free (OFC_LPVOID mem)
{
  struct heap_chunk * chunk ;

  if (mem != OFC_NULL)
    {
      chunk = mem ;
      chunk-- ;

#if defined(OFC_HEAP_DEBUG)
      ofc_heap_debug_free (chunk) ;
#endif
      ofc_heap_free_acct (chunk) ;
      ofc_free_impl (chunk) ;
    }
}

OFC_CORE_LIB OFC_LPVOID
ofc_realloc (OFC_LPVOID ptr, OFC_SIZET size)
{
  struct heap_chunk * chunk ;
  struct heap_chunk * newchunk ;
  OFC_LPVOID mem ;

  mem = OFC_NULL ;
  chunk = ptr ;
  if (chunk != OFC_NULL)
    {
      chunk-- ;

      ofc_heap_free_acct (chunk) ;

#if defined(OFC_HEAP_DEBUG)
      ofc_heap_debug_free (chunk) ;
#endif

      newchunk = ofc_realloc_impl (chunk,
				      size + sizeof (struct heap_chunk)) ;

      if (newchunk != OFC_NULL)
	{
#if defined(OFC_HEAP_DEBUG)
	  ofc_heap_debug_alloc (size, newchunk, RETURN_ADDRESS()) ;
#endif
	  ofc_heap_malloc_acct (size, newchunk) ;
	  chunk = newchunk ;
	  mem = chunk + 1 ;
	}
      else
	{
	  ofc_process_crash ("ofc_realloc Allocation Failed\n") ;
	}
    }
  else
    mem = ofc_malloc (size) ;

  return (mem) ;
}

