/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/queue.h"
#include "ofc/libc.h"
#include "ofc/lock.h"
#include "ofc/time.h"
#include "ofc/waitset.h"
#include "ofc/thread.h"
#include "ofc/process.h"

#include "ofc/impl/waitsetimpl.h"

#include "ofc/heap.h"

#if defined(DISABLED)
typedef enum
  {
    HANDLE_EVENT_LOCK,
    HANDLE_EVENT_UNLOCK,
    HANDLE_EVENT_DESTROY
  } BLUE_HANDLE_TRACE_EVENT ;

typedef struct _STACK
{
  BLUE_HANDLE_TRACE_EVENT event ;
  BLUE_INT reference ;
  BLUE_BOOL destroy ;
  BLUE_VOID *caller1 ;
  BLUE_VOID *caller2 ;
  BLUE_VOID *caller3 ;
  BLUE_VOID *caller4 ;
} STACK ;
#endif

typedef struct _HANDLE_CONTEXT
{
  BLUE_HANDLE_TYPE type ;
  OFC_INT reference ;
  OFC_BOOL destroy ;
  OFC_VOID *context ;
  BLUE_HANDLE wait_app ;
  BLUE_HANDLE wait_set ;
#if defined(DISABLED)
  STACK trace[10] ;
  BLUE_INT trace_idx ;
#endif
#if defined(OFC_HANDLE_PERF)
  BLUE_MSTIME last_triggered ;
  BLUE_MSTIME avg_interval ;
  BLUE_UINT32 avg_count ;
#endif
#if defined(OFC_HANDLE_DEBUG)
  struct _HANDLE_CONTEXT * dbgnext ;
  struct _HANDLE_CONTEXT * dbgprev ;
#if defined(__GNUC__)
  BLUE_VOID *caller1 ;
  BLUE_VOID *caller2 ;
  BLUE_VOID *caller3 ;
  BLUE_VOID *caller4 ;
#else
  BLUE_VOID *caller ;
#endif
#endif
} HANDLE_CONTEXT ;

typedef struct _HANDLE16_CONTEXT
{
  OFC_UCHAR id ;
  OFC_UCHAR instance ;
  OFC_BOOL alloc ;
  OFC_VOID *context ;
  OFC_INT ref ;
#if defined(OFC_HANDLE_DEBUG)
  struct _HANDLE16_CONTEXT * dbgnext ;
  struct _HANDLE16_CONTEXT * dbgprev ;
#if defined(__GNUC__)
  BLUE_VOID *caller1 ;
  BLUE_VOID *caller2 ;
  BLUE_VOID *caller3 ;
  BLUE_VOID *caller4 ;
#else
  BLUE_VOID *caller ;
#endif
#endif
} HANDLE16_CONTEXT ;


BLUE_LOCK BlueHandle16Mutex ;
BLUE_LOCK HandleLock ;

#if defined(OFC_HANDLE_DEBUG)
static HANDLE_CONTEXT *BlueHandleAlloc ;
static HANDLE16_CONTEXT *BlueHandle16Alloc ;

BLUE_VOID BlueHandleDebugInit (BLUE_VOID)
{
  BlueHandleAlloc = BLUE_NULL ;
  BlueHandle16Alloc = BLUE_NULL ;
}

BLUE_VOID BlueHandleDebugAlloc (HANDLE_CONTEXT *handle, BLUE_VOID *ret)
{
  /*
   * Put on the allocation queue
   */
  BlueLock (HandleLock) ;
  handle->dbgnext = BlueHandleAlloc ;
  if (BlueHandleAlloc != BLUE_NULL)
    BlueHandleAlloc->dbgprev = handle ;
  BlueHandleAlloc = handle ;
  handle->dbgprev = BLUE_NULL ;
  BlueUnlock (HandleLock) ;
#if defined(__GNUC__) && defined(BLUE_STACK_TRACE)
  handle->caller1 = __builtin_return_address(1) ;
  handle->caller2 = __builtin_return_address(2) ;
  handle->caller3 = __builtin_return_address(3) ;
  handle->caller4 = __builtin_return_address(4) ;
#else
  handle->caller = ret ;
#endif
}

BLUE_VOID BlueHandleDebugFree (HANDLE_CONTEXT *handle)
{
  /*
   * Pull off the allocation queue
   */
  BlueLock (HandleLock) ;
  if (handle->dbgprev != BLUE_NULL)
    handle->dbgprev->dbgnext = handle->dbgnext ;
  else
    BlueHandleAlloc = handle->dbgnext ;
  if (handle->dbgnext != BLUE_NULL)
    handle->dbgnext->dbgprev = handle->dbgprev ;
  BlueUnlock (HandleLock) ;
}

BLUE_VOID BlueHandleDebugDump (BLUE_VOID)
{
  HANDLE_CONTEXT *handle ;

  BlueCprintf ("Handle Dump\n") ;
#if defined(__GNUC__)
  BlueCprintf ("%-10s %-10s %-10s %-10s %-10s\n", "Address", 
	       "Caller1", "Caller2", "Caller3", "Caller4") ;

  for (handle = BlueHandleAlloc ; handle != BLUE_NULL ; 
       handle = handle->dbgnext)
    {
      BlueCprintf ("%-10p %-10p %-10p %-10p %-10p\n", handle, 
		   handle->caller1, handle->caller2, handle->caller3,
		   handle->caller4) ;
    }
#else
  BlueCprintf ("%-20s %-20s\n", "Address", "Caller") ;

  for (handle = BlueHandleAlloc ; handle != BLUE_NULL ; 
       handle = handle->dbgnext)
    {
      BlueCprintf ("%-20p %-20p\n", handle, handle->caller) ;
    }
#endif
  BlueCprintf ("\n") ;
}

BLUE_VOID BlueHandle16DebugAlloc (HANDLE16_CONTEXT *handle, BLUE_VOID *ret)
{
  /*
   * Put on the allocation queue
   */
  BlueLock (HandleLock) ;
  handle->dbgnext = BlueHandle16Alloc ;
  if (BlueHandle16Alloc != BLUE_NULL)
    BlueHandle16Alloc->dbgprev = handle ;
  BlueHandle16Alloc = handle ;
  handle->dbgprev = BLUE_NULL ;
  BlueUnlock (HandleLock) ;
#if defined(__GNUC__) && defined(BLUE_STACK_TRACE)
  handle->caller1 = __builtin_return_address(1) ;
  handle->caller2 = __builtin_return_address(2) ;
  handle->caller3 = __builtin_return_address(3) ;
  handle->caller4 = __builtin_return_address(4) ;
#else
  handle->caller = ret ;
#endif
}

BLUE_VOID BlueHandle16DebugFree (HANDLE16_CONTEXT *handle)
{
  /*
   * Pull off the allocation queue
   */
  BlueLock (HandleLock) ;
  if (handle->dbgprev != BLUE_NULL)
    handle->dbgprev->dbgnext = handle->dbgnext ;
  else
    BlueHandle16Alloc = handle->dbgnext ;
  if (handle->dbgnext != BLUE_NULL)
    handle->dbgnext->dbgprev = handle->dbgprev ;
#if defined(__GNUC__) && defined(BLUE_STACK_TRACE)
  handle->caller1 = __builtin_return_address(1) ;
  handle->caller2 = __builtin_return_address(2) ;
  handle->caller3 = __builtin_return_address(3) ;
  handle->caller4 = __builtin_return_address(4) ;
#endif
  BlueUnlock (HandleLock) ;
}

BLUE_VOID BlueHandle16DebugDump (BLUE_VOID)
{
  HANDLE16_CONTEXT *handle ;

  BlueCprintf ("Handle 16 Dump\n") ;
  BlueCprintf ("Address of BlueHandle16DebugDump 0x%08x\n", BlueHandle16DebugDump) ;

#if defined(__GNUC__)
  BlueCprintf ("%-10s %-10s %-10s %-10s %-10s\n", "Address", 
	       "Caller1", "Caller2", "Caller3", "Caller4") ;

  for (handle = BlueHandle16Alloc ; handle != BLUE_NULL ; 
       handle = handle->dbgnext)
    {
      BlueCprintf ("%-10p %-10p %-10p %-10p %-10p\n", handle, 
		   handle->caller1, handle->caller2, handle->caller3,
		   handle->caller4) ;
    }
#else
  BlueCprintf ("%-20s %-20s\n", "Address", "Caller") ;

  for (handle = BlueHandle16Alloc ; handle != BLUE_NULL ; 
       handle = handle->dbgnext)
    {
      BlueCprintf ("%-20p %-20p\n", handle, handle->caller) ;
    }
#endif
  BlueCprintf ("\n") ;
}

#endif

OFC_CORE_LIB BLUE_HANDLE
BlueHandleCreate (BLUE_HANDLE_TYPE hType, OFC_VOID *context)
{
  HANDLE_CONTEXT *handle_context ;

  handle_context = BlueHeapMalloc (sizeof (HANDLE_CONTEXT)) ;
  handle_context->reference = 0 ;
  handle_context->destroy = OFC_FALSE ;
  handle_context->type = hType ;
  handle_context->context = context ;
#if defined(DISABLED)
  handle_context->trace_idx = 0 ;
#endif

  handle_context->wait_set = BLUE_HANDLE_NULL ;
  handle_context->wait_app = BLUE_HANDLE_NULL ;

#if defined(OFC_HANDLE_PERF)
  handle_context->last_triggered = 0 ;
  handle_context->avg_interval = 0 ;
  handle_context->avg_count = 0 ;
#endif

#if defined(OFC_HANDLE_DEBUG)
  BlueHandleDebugAlloc (handle_context, RETURN_ADDRESS()) ;
#endif

  return ((BLUE_HANDLE) handle_context) ;
}

OFC_CORE_LIB BLUE_HANDLE_TYPE BlueHandleGetType (BLUE_HANDLE hHandle)
{
  HANDLE_CONTEXT *handle ;
  BLUE_HANDLE_TYPE type ;

  type = BLUE_HANDLE_UNKNOWN ;
  handle = (HANDLE_CONTEXT *) hHandle ;
  if (handle != OFC_NULL)
    {
      type = handle->type ;
    }
  return (type) ;
}

OFC_CORE_LIB OFC_VOID *BlueHandleLock (BLUE_HANDLE handle)
{
  HANDLE_CONTEXT *handle_context ;
  OFC_VOID *ret ;

  handle_context = (HANDLE_CONTEXT *) handle ;
  ret = OFC_NULL ;

  BlueLock (HandleLock) ;
#if defined(DISABLED)
  handle_context->trace[handle_context->trace_idx].event =
    HANDLE_EVENT_LOCK ;
  handle_context->trace[handle_context->trace_idx].reference =
    handle_context->reference ;
  handle_context->trace[handle_context->trace_idx].destroy =
    handle_context->lock) ;
handle_context->destroy ;
  handle_context->trace[handle_context->trace_idx].caller1 = 
    __builtin_return_address(1) ;
  handle_context->trace[handle_context->trace_idx].caller2 = 
    __builtin_return_address(2) ;
  handle_context->trace[handle_context->trace_idx].caller3 = 
    __builtin_return_address(3) ;
  handle_context->trace[handle_context->trace_idx].caller4 = 
    __builtin_return_address(4) ;
  handle_context->trace_idx = (handle_context->trace_idx+1) % 10 ;
#endif

  if (!handle_context->destroy)
    {
      handle_context->reference++ ;
      ret = handle_context->context ;
    }
  BlueUnlock (HandleLock) ;

  return (ret) ;
}

OFC_CORE_LIB OFC_VOID *
BlueHandleLockEx (BLUE_HANDLE handle, BLUE_HANDLE_TYPE type)
{
  OFC_VOID *ret ;

  ret = OFC_NULL ;
  if (BlueHandleGetType (handle) == type)
    ret = BlueHandleLock (handle) ;
  else
    BlueProcessCrash ("Handle Mismatch\n") ;
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID BlueHandleUnlock (BLUE_HANDLE handle)
{
  HANDLE_CONTEXT *handle_context ;

  handle_context = (HANDLE_CONTEXT *) handle ;
  BlueLock (HandleLock) ;

  handle_context->reference-- ;
#if defined(DISABLED)
  handle_context->trace[handle_context->trace_idx].event =
    HANDLE_EVENT_UNLOCK ;
  handle_context->trace[handle_context->trace_idx].reference =
    handle_context->reference ;
  handle_context->trace[handle_context->trace_idx].destroy =
    handle_context->destroy ;
  handle_context->trace[handle_context->trace_idx].caller1 = 
    __builtin_return_address(1) ;
  handle_context->trace[handle_context->trace_idx].caller2 = 
    __builtin_return_address(2) ;
  handle_context->trace[handle_context->trace_idx].caller3 = 
    __builtin_return_address(3) ;
  handle_context->trace[handle_context->trace_idx].caller4 = 
    __builtin_return_address(4) ;
  handle_context->trace_idx = (handle_context->trace_idx+1) % 100 ;
#endif

  if (handle_context->destroy && handle_context->reference == 0)
    {
#if defined(OFC_HANDLE_DEBUG)
      BlueHandleDebugFree (handle_context) ;
#endif
#if defined(OFC_HANDLE_PERF)
      BlueHandlePrintInterval ("Handle: ", handle) ;
#endif
      BlueUnlock (HandleLock) ;
      BlueHeapFree (handle_context) ;
    }
  else
    BlueUnlock (HandleLock) ;
}

OFC_CORE_LIB OFC_VOID BlueHandleDestroy (BLUE_HANDLE handle)
{
  HANDLE_CONTEXT *handle_context ;

  handle_context = (HANDLE_CONTEXT *) handle ;

  if (handle_context->wait_set != BLUE_HANDLE_NULL)
    BlueWaitSetRemove (handle_context->wait_set, handle) ;

  BlueLock (HandleLock) ;
#if defined(DISABLED)
  handle_context->trace[handle_context->trace_idx].event =
    HANDLE_EVENT_DESTROY ;
  handle_context->trace[handle_context->trace_idx].reference =
    handle_context->reference ;
  handle_context->trace[handle_context->trace_idx].destroy =
    handle_context->destroy ;
  handle_context->trace[handle_context->trace_idx].caller1 = 
    __builtin_return_address(1) ;
  handle_context->trace[handle_context->trace_idx].caller2 = 
    __builtin_return_address(2) ;
  handle_context->trace[handle_context->trace_idx].caller3 = 
    __builtin_return_address(3) ;
  handle_context->trace[handle_context->trace_idx].caller4 = 
    __builtin_return_address(4) ;
  handle_context->trace_idx = (handle_context->trace_idx+1) % 100 ;
#endif

  BlueUnlock (HandleLock) ;
  if (handle_context->reference == 0)
    {

#if defined(OFC_HANDLE_DEBUG)
      BlueHandleDebugFree (handle_context) ;
#endif
#if defined(OFC_HANDLE_PERF)
      BlueHandlePrintInterval ("Handle: ", handle) ;
#endif
      BlueHeapFree (handle_context) ;
    }
  else
    handle_context->destroy = OFC_TRUE ;
}

OFC_CORE_LIB OFC_VOID
BlueHandleSetApp (BLUE_HANDLE hHandle, BLUE_HANDLE hApp, BLUE_HANDLE hSet)
{
  HANDLE_CONTEXT *handle_context ;

  if (hHandle != BLUE_HANDLE_NULL)
    {
      handle_context = (HANDLE_CONTEXT *) hHandle ;

      handle_context->wait_app = hApp ;
      handle_context->wait_set = hSet ;
      BlueWaitSetSetAssocImpl (hHandle, hApp, hSet) ;
    }
}

OFC_CORE_LIB BLUE_HANDLE BlueHandleGetApp (BLUE_HANDLE hHandle)
{
  HANDLE_CONTEXT *handle_context ;

  handle_context = (HANDLE_CONTEXT *) hHandle ;

  return (handle_context->wait_app) ;
}

OFC_CORE_LIB BLUE_HANDLE BlueHandleGetWaitSet (BLUE_HANDLE hHandle)
{
  HANDLE_CONTEXT *handle_context ;

  handle_context = (HANDLE_CONTEXT *) hHandle ;

  return (handle_context->wait_set) ;
}

static HANDLE16_CONTEXT Handle16Array[OFC_MAX_HANDLE16] ;
static BLUE_HANDLE Handle16Free ;

#define HANDLE16_MAKE(ix, id) ((BLUE_HANDLE16) (ix & 0xFF) << 8 | (id & 0xFF))
#define HANDLE16_INSTANCE(handle) ((OFC_UCHAR) ((handle >> 8) & 0xFF))
#define HANDLE16_ID(handle) ((OFC_UCHAR) (handle & 0xFF))

OFC_CORE_LIB OFC_VOID BlueHandle16Init (OFC_VOID)
{
  OFC_INT i ;

#if defined (OFC_HANDLE_DEBUG)
  BlueHandleDebugInit() ;
#endif

  BlueHandle16Mutex = BlueLockInit () ;
  HandleLock = BlueLockInit () ;

  Handle16Free = BlueQcreate() ;

  for (i = 0 ; i < OFC_MAX_HANDLE16 ; i++)
    {
      Handle16Array[i].id = i ;
      Handle16Array[i].instance = 0xFF ;
      Handle16Array[i].context = OFC_NULL ;
      Handle16Array[i].alloc = OFC_FALSE ;
      Handle16Array[i].ref = 0 ;
      BlueQenqueue (Handle16Free, &Handle16Array[i]) ;
    }
}

OFC_CORE_LIB OFC_VOID BlueHandle16Free (OFC_VOID)
{
  BlueQclear (Handle16Free) ;
  BlueQdestroy (Handle16Free) ;

  BlueLockDestroy (BlueHandle16Mutex) ;
  BlueLockDestroy (HandleLock) ;
}

OFC_CORE_LIB OFC_VOID *BlueHandle16Lock (BLUE_HANDLE16 hHandle)
{
  OFC_VOID *ret ;
  OFC_UCHAR id ;
  HANDLE16_CONTEXT *handle ;

  ret = OFC_NULL ;
  BlueLock (BlueHandle16Mutex) ;
  id = HANDLE16_ID (hHandle) ;
  if (id < OFC_MAX_HANDLE16)
    {
      handle = &Handle16Array[id] ;    
      if (handle->instance == HANDLE16_INSTANCE(hHandle) &&
          handle->alloc == OFC_TRUE)
        {
          ret = handle->context ;
          handle->ref++ ;
        }
    }
      
  BlueUnlock (BlueHandle16Mutex) ;
  return (ret) ;
}

OFC_CORE_LIB BLUE_HANDLE16 BlueHandle16Create (OFC_VOID *context)
{
  HANDLE16_CONTEXT *handle ;  
  BLUE_HANDLE16 ret ;

  ret = BLUE_HANDLE16_INVALID ;
  BlueLock (BlueHandle16Mutex) ;
  handle = BlueQdequeue (Handle16Free) ;
  if (handle != OFC_NULL)
    {
      handle->instance++ ;
      if (handle->instance == 0xFF || handle->instance == 0)
        handle->instance = 1 ;
      handle->alloc = OFC_TRUE ;
      handle->ref++ ;
      handle->context = context ;
#if defined(OFC_HANDLE_DEBUG)
      BlueHandle16DebugAlloc (handle, RETURN_ADDRESS()) ;
#endif
      ret = HANDLE16_MAKE (handle->instance, handle->id) ;
    }
#if defined(OFC_HANDLE_DEBUG)
  else
    BlueHandle16DebugDump() ;
#endif
  
  if (ret == BLUE_HANDLE16_INVALID)
    {
      BlueProcessCrash ("Handle16 Pool Exhausted\n") ;
    }

  BlueUnlock (BlueHandle16Mutex) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID BlueHandle16Destroy (BLUE_HANDLE16 hHandle)
{
  OFC_UCHAR id ;
  HANDLE16_CONTEXT *handle ;

  BlueLock (BlueHandle16Mutex) ;
  id = HANDLE16_ID (hHandle) ;
  if (id < OFC_MAX_HANDLE16)
    {
      handle = &Handle16Array[id] ;    
      if (handle->instance == HANDLE16_INSTANCE(hHandle) &&
          handle->alloc == OFC_TRUE)
        {
          handle->context = OFC_NULL ;
	  BlueUnlock (BlueHandle16Mutex) ;
          BlueHandle16Unlock (hHandle) ;
        }
      else
	{
	  /*
	   * Already destroyed 
	   */
	  BlueUnlock (BlueHandle16Mutex) ;
	}
    }
  else
    {
      BlueUnlock (BlueHandle16Mutex) ;
      BlueCprintf ("Bad Handle being destroyed 0x%08x, %d\n",
		   hHandle, id) ;
      BlueProcessCrash ("Bad Handle being destroyed\n") ;
    }
}

OFC_CORE_LIB OFC_VOID BlueHandle16Unlock (BLUE_HANDLE16 hHandle)
{
  OFC_UCHAR id ;
  HANDLE16_CONTEXT *handle ;

  BlueLock (BlueHandle16Mutex) ;
  id = HANDLE16_ID (hHandle) ;
  if (id < OFC_MAX_HANDLE16)
    {
      handle = &Handle16Array[id] ;    
      if (handle->instance == HANDLE16_INSTANCE(hHandle) &&
          handle->alloc == OFC_TRUE)
        {
          handle->ref-- ;
          if (handle->ref == 0)
            {
#if defined(OFC_HANDLE_DEBUG)
	      BlueHandle16DebugFree (handle) ;
#endif
	      handle->alloc = OFC_FALSE ;
              BlueQenqueue (Handle16Free, handle) ;
            }
        }
    }
  else
    BlueProcessCrash ("Bad Handle being unlocked\n") ;
  BlueUnlock (BlueHandle16Mutex) ;
}

#if defined(OFC_HANDLE_PERF)
BLUE_CORE_LIB BLUE_VOID BlueHandleMeasure (BLUE_HANDLE hHandle)
{
  HANDLE_CONTEXT *handle_context ;
  BLUE_MSTIME now ;
  BLUE_MSTIME interval ;

  if (hHandle != BLUE_HANDLE_NULL)
    {
      now = BlueTimeGetNow() ;
      handle_context = (HANDLE_CONTEXT *) hHandle ;
      if (handle_context->last_triggered != 0)
	{
	  interval = now - handle_context->last_triggered ;
	  if (interval > handle_context->avg_interval)
	    handle_context->avg_interval += 
	      ((interval - handle_context->avg_interval) /
	       (handle_context->avg_count + 1)) ;
	  else
	    handle_context->avg_interval -= 
	      ((handle_context->avg_interval - interval) /
	       (handle_context->avg_count + 1)) ;
	  handle_context->avg_count++ ;
	}
      handle_context->last_triggered = now ;
    }
}

BLUE_CORE_LIB BLUE_MSTIME BlueHandleGetAvgInterval (BLUE_HANDLE hHandle,
						    BLUE_UINT32 *count,
						    BLUE_HANDLE_TYPE *type)
{
  HANDLE_CONTEXT *handle_context ;
  BLUE_MSTIME interval ;

  if (hHandle != BLUE_HANDLE_NULL)
    {
      handle_context = (HANDLE_CONTEXT *) hHandle ;
      interval = handle_context->avg_interval ;
      *count = handle_context->avg_count ;
      *type = handle_context->type ;
      handle_context->avg_interval = 0 ;
      handle_context->avg_count = 0 ;
    }
  return (interval) ;
}

static BLUE_CHAR *type2str (BLUE_HANDLE_TYPE type)
{
  static struct 
  {
    BLUE_HANDLE_TYPE type; 
    BLUE_CHAR *str;
  } typearray[] =
      {
	{ BLUE_HANDLE_UNKNOWN, "Unknown" },
	{ BLUE_HANDLE_WAIT_SET, "Wait Set" },
	{ BLUE_HANDLE_QUEUE, "Queue" },
	{ BLUE_HANDLE_FILE,	"File" },
	{ BLUE_HANDLE_FSWIN32_FILE,	"Win32 File" },
	{ BLUE_HANDLE_FSWIN32_OVERLAPPED, "Win32 Buffer" },
	{ BLUE_HANDLE_FSCIFS_OVERLAPPED, "Cifs Buffer" },
	{ BLUE_HANDLE_FSDARWIN_OVERLAPPED, "Darwin Buffer" },
	{ BLUE_HANDLE_FSLINUX_OVERLAPPED, "Linux Buffer" },
	{ BLUE_HANDLE_FSFILEX_OVERLAPPED, "FILEX Buffer" },
	{ BLUE_HANDLE_FSNUFILE_OVERLAPPED, "NUFILE Buffer" },
	{ BLUE_HANDLE_FSANDROID_OVERLAPPED, "Android Buffer" },
	{ BLUE_HANDLE_FSDARWIN_FILE, "Darwin File" },
	{ BLUE_HANDLE_FSLINUX_FILE,	"Linux File" },
	{ BLUE_HANDLE_FSFILEX_FILE,	"FILEX File" },
	{ BLUE_HANDLE_FSANDROID_FILE, "Android File" },
	{ BLUE_HANDLE_FSNUFILE_FILE, "NUFILE File" },
	{ BLUE_HANDLE_FSOTHER_FILE, "Other File" },
	{ BLUE_HANDLE_FSBROWSER_FILE, "Browser File" },
	{ BLUE_HANDLE_FSBOOKMARK_FILE, "Bookmark File" },
	{ BLUE_HANDLE_SCHED, "Scheduler" },
	{ BLUE_HANDLE_APP, "App" },
	{ BLUE_HANDLE_THREAD, "Thread" },
	{ BLUE_HANDLE_SOCKET, "Socket" },
	{ BLUE_HANDLE_SOCKET_IMPL, "Socket Impl" },
	{ BLUE_HANDLE_EVENT, "Event" },
	{ BLUE_HANDLE_TIMER, "Timer" },
	{ BLUE_HANDLE_PIPE,	"Pipe" },
	{ BLUE_HANDLE_WAIT_QUEUE, "Wait Queue" },
	{ BLUE_HANDLE_TRANSACTION, "Transaction" },
	{ BLUE_HANDLE_CIFS_FILE, "CIFS File" },
	{ BLUE_HANDLE_MAILSLOT, "Mailslot" },
	{ BLUE_HANDLE_PROCESS, "Process" },
	{ BLUE_HANDLE_NUM, BLUE_NULL }
      } ;
  BLUE_INT i ;

  for (i = 0 ; i < BLUE_HANDLE_NUM && typearray[i].type != type; i++) ;

  return (typearray[i].str) ;
}

BLUE_CORE_LIB BLUE_VOID BlueHandlePrintIntervalHeader (BLUE_VOID)
{
  BlueCprintf ("%-15.15s  %15.15s  %15.15s\n", 
	       "Handle Type", "Avg Interval", "Count") ;
}

BLUE_CORE_LIB BLUE_VOID
BlueHandlePrintInterval (BLUE_CHAR *prefix, BLUE_HANDLE hHandle)
{
  BLUE_MSTIME interval ;
  BLUE_UINT32 count ;
  BLUE_HANDLE_TYPE type ;

  interval = BlueHandleGetAvgInterval (hHandle, &count, &type) ;

  if (count > 0)
    {
      if (type == BLUE_HANDLE_TIMER)
	{
	  BlueCprintf ("%s%-15.15s  %12lu ms  %15lu  %s\n", prefix,
		       type2str(type), 
		       interval, count, BlueTimerID (hHandle)) ;
	}
      else
	{
	  BlueCprintf ("%s%-15.15s  %12lu ms  %15lu\n", prefix,
		       type2str(type), interval, count) ;
	}
    }
}
#endif
