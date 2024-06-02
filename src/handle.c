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
  } OFC_HANDLE_TRACE_EVENT ;

typedef struct _STACK
{
  OFC_HANDLE_TRACE_EVENT event ;
  OFC_INT reference ;
  OFC_BOOL destroy ;
  OFC_VOID *caller1 ;
  OFC_VOID *caller2 ;
  OFC_VOID *caller3 ;
  OFC_VOID *caller4 ;
} STACK ;
#endif

typedef struct _HANDLE_CONTEXT {
    OFC_HANDLE_TYPE type;
    OFC_INT reference;
    OFC_BOOL destroy;
    OFC_VOID *context;
    OFC_HANDLE wait_app;
    OFC_HANDLE wait_set;
#if defined(DISABLED)
    STACK trace[10] ;
    OFC_INT trace_idx ;
#endif
#if defined(OFC_HANDLE_PERF)
    OFC_MSTIME last_triggered ;
    OFC_MSTIME avg_interval ;
    OFC_UINT32 avg_count ;
#endif
#if defined(OFC_HANDLE_DEBUG)
    struct _HANDLE_CONTEXT * dbgnext ;
    struct _HANDLE_CONTEXT * dbgprev ;
#if defined(__GNUC__)
    OFC_VOID *caller1 ;
    OFC_VOID *caller2 ;
    OFC_VOID *caller3 ;
    OFC_VOID *caller4 ;
#else
    OFC_VOID *caller ;
#endif
#endif
} HANDLE_CONTEXT;

typedef struct _HANDLE16_CONTEXT {
    OFC_UCHAR id;
    OFC_UCHAR instance;
    OFC_BOOL alloc;
    OFC_VOID *context;
    OFC_INT ref;
#if defined(OFC_HANDLE_DEBUG)
    struct _HANDLE16_CONTEXT * dbgnext ;
    struct _HANDLE16_CONTEXT * dbgprev ;
#if defined(__GNUC__)
    OFC_VOID *caller1 ;
    OFC_VOID *caller2 ;
    OFC_VOID *caller3 ;
    OFC_VOID *caller4 ;
#else
    OFC_VOID *caller ;
#endif
#endif
} HANDLE16_CONTEXT;


OFC_LOCK OfcHandle16Mutex;
OFC_LOCK HandleLock;

#if defined(OFC_HANDLE_DEBUG)
static HANDLE_CONTEXT *OfcHandleAlloc ;
static HANDLE16_CONTEXT *OfcHandle16Alloc ;

OFC_VOID OfcHandleDebugInit (OFC_VOID)
{
  OfcHandleAlloc = OFC_NULL ;
  OfcHandle16Alloc = OFC_NULL ;
}

OFC_VOID OfcHandleDebugAlloc (HANDLE_CONTEXT *handle, OFC_VOID *ret)
{
  /*
   * Put on the allocation queue
   */
  ofc_lock (HandleLock) ;
  handle->dbgnext = OfcHandleAlloc ;
  if (OfcHandleAlloc != OFC_NULL)
    OfcHandleAlloc->dbgprev = handle ;
  OfcHandleAlloc = handle ;
  handle->dbgprev = OFC_NULL ;
  ofc_unlock (HandleLock) ;
#if defined(__GNUC__) && defined(OFC_STACK_TRACE)
  handle->caller1 = __builtin_return_address(1) ;
  handle->caller2 = __builtin_return_address(2) ;
  handle->caller3 = __builtin_return_address(3) ;
  handle->caller4 = __builtin_return_address(4) ;
#else
  handle->caller = ret ;
#endif
}

OFC_VOID ofc_handle_debug_free (HANDLE_CONTEXT *handle)
{
  /*
   * Pull off the allocation queue
   */
  ofc_lock (HandleLock) ;
  if (handle->dbgprev != OFC_NULL)
    handle->dbgprev->dbgnext = handle->dbgnext ;
  else
    OfcHandleAlloc = handle->dbgnext ;
  if (handle->dbgnext != OFC_NULL)
    handle->dbgnext->dbgprev = handle->dbgprev ;
  ofc_unlock (HandleLock) ;
}

OFC_VOID ofc_handle_debug_dump (OFC_VOID)
{
  HANDLE_CONTEXT *handle ;

  ofc_printf ("Handle Dump\n") ;
#if defined(__GNUC__)
  ofc_printf ("%-10s %-10s %-10s %-10s %-10s\n", "Address", 
           "Caller1", "Caller2", "Caller3", "Caller4") ;

  for (handle = OfcHandleAlloc ; handle != OFC_NULL ;
       handle = handle->dbgnext)
    {
      ofc_printf ("%-10p %-10p %-10p %-10p %-10p\n", handle,
           handle->caller1, handle->caller2, handle->caller3,
           handle->caller4) ;
    }
#else
  ofc_printf ("%-20s %-20s\n", "Address", "Caller") ;

  for (handle = OfcHandleAlloc ; handle != OFC_NULL ;
       handle = handle->dbgnext)
    {
      ofc_printf ("%-20p %-20p\n", handle, handle->caller) ;
    }
#endif
  ofc_printf ("\n") ;
}

OFC_VOID OfcHandle16DebugAlloc (HANDLE16_CONTEXT *handle, OFC_VOID *ret)
{
  /*
   * Put on the allocation queue
   */
  ofc_lock (HandleLock) ;
  handle->dbgnext = OfcHandle16Alloc ;
  if (OfcHandle16Alloc != OFC_NULL)
    OfcHandle16Alloc->dbgprev = handle ;
  OfcHandle16Alloc = handle ;
  handle->dbgprev = OFC_NULL ;
  ofc_unlock (HandleLock) ;
#if defined(__GNUC__) && defined(OFC_STACK_TRACE)
  handle->caller1 = __builtin_return_address(1) ;
  handle->caller2 = __builtin_return_address(2) ;
  handle->caller3 = __builtin_return_address(3) ;
  handle->caller4 = __builtin_return_address(4) ;
#else
  handle->caller = ret ;
#endif
}

OFC_VOID OfcHandle16DebugFree (HANDLE16_CONTEXT *handle)
{
  /*
   * Pull off the allocation queue
   */
  ofc_lock (HandleLock) ;
  if (handle->dbgprev != OFC_NULL)
    handle->dbgprev->dbgnext = handle->dbgnext ;
  else
    OfcHandle16Alloc = handle->dbgnext ;
  if (handle->dbgnext != OFC_NULL)
    handle->dbgnext->dbgprev = handle->dbgprev ;
#if defined(__GNUC__) && defined(OFC_STACK_TRACE)
  handle->caller1 = __builtin_return_address(1) ;
  handle->caller2 = __builtin_return_address(2) ;
  handle->caller3 = __builtin_return_address(3) ;
  handle->caller4 = __builtin_return_address(4) ;
#endif
  ofc_unlock (HandleLock) ;
}

OFC_VOID OfcHandle16DebugDump (OFC_VOID)
{
  HANDLE16_CONTEXT *handle ;

  ofc_log (OFC_LOG_DEBUG, "Handle 16 Dump\n") ;
  ofc_log (OFC_LOG_DEBUG, "Handle 16 Count %d\n", handle16_count);
  ofc_log (OFC_LOG_DEBUG, "Address of OfcHandle16DebugDump 0x%08x\n", OfcHandle16DebugDump) ;

#if defined(__GNUC__)
  ofc_log (OFC_LOG_DEBUG, "%-10s %-10s %-10s %-10s %-10s\n", "Address",
           "Caller1", "Caller2", "Caller3", "Caller4") ;

  for (handle = OfcHandle16Alloc ; handle != OFC_NULL ;
       handle = handle->dbgnext)
    {
      ofc_log (OFC_LOG_DEBUG, "%-10p %-10p %-10p %-10p %-10p %-10p\n", handle,
           handle->caller1, handle->caller2, handle->caller3,
               handle->caller4, handle->ref) ;
    }
#else
  ofc_log (OFC_LOG_DEBUG, "%-20s %-20s\n", "Address", "Caller") ;

  for (handle = OfcHandle16Alloc ; handle != OFC_NULL ; 
       handle = handle->dbgnext)
    {
      ofc_log (OFC_LOG_DEBUG, "%-20p %-20p\n", handle, handle->caller) ;
    }
#endif
  ofc_log (OFC_LOG_DEBUG, "\n") ;
}

#endif

OFC_CORE_LIB OFC_HANDLE
ofc_handle_create(OFC_HANDLE_TYPE hType, OFC_VOID *context) {
    HANDLE_CONTEXT *handle_context;

    handle_context = ofc_malloc(sizeof(HANDLE_CONTEXT));
    handle_context->reference = 0;
    handle_context->destroy = OFC_FALSE;
    handle_context->type = hType;
    handle_context->context = context;
#if defined(DISABLED)
    handle_context->trace_idx = 0 ;
#endif

    handle_context->wait_set = OFC_HANDLE_NULL;
    handle_context->wait_app = OFC_HANDLE_NULL;

#if defined(OFC_HANDLE_PERF)
    handle_context->last_triggered = 0 ;
    handle_context->avg_interval = 0 ;
    handle_context->avg_count = 0 ;
#endif

#if defined(OFC_HANDLE_DEBUG)
    OfcHandleDebugAlloc (handle_context, RETURN_ADDRESS()) ;
#endif

    return ((OFC_HANDLE) handle_context);
}

OFC_CORE_LIB OFC_HANDLE_TYPE ofc_handle_get_type(OFC_HANDLE hHandle) {
    HANDLE_CONTEXT *handle;
    OFC_HANDLE_TYPE type;

    type = OFC_HANDLE_UNKNOWN;
    handle = (HANDLE_CONTEXT *) hHandle;
    if (handle != OFC_NULL) {
        type = handle->type;
    }
    return (type);
}

OFC_CORE_LIB OFC_VOID *ofc_handle_lock(OFC_HANDLE handle) {
    HANDLE_CONTEXT *handle_context;
    OFC_VOID *ret;

    handle_context = (HANDLE_CONTEXT *) handle;
    ret = OFC_NULL;

    ofc_lock(HandleLock);
#if defined(DISABLED)
    handle_context->trace[handle_context->trace_idx].event =
      HANDLE_EVENT_LOCK ;
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
    handle_context->trace_idx = (handle_context->trace_idx+1) % 10 ;
#endif

    if (!handle_context->destroy) {
        handle_context->reference++;
        ret = handle_context->context;
    }
    ofc_unlock(HandleLock);

    return (ret);
}

OFC_CORE_LIB OFC_VOID *
ofc_handle_lock_ex(OFC_HANDLE handle, OFC_HANDLE_TYPE type) {
    OFC_VOID *ret;

    ret = OFC_NULL;
    if (ofc_handle_get_type(handle) == type)
        ret = ofc_handle_lock(handle);
    else
        ofc_process_crash("Handle Mismatch\n");
    return (ret);
}

OFC_CORE_LIB OFC_VOID ofc_handle_unlock(OFC_HANDLE handle) {
    HANDLE_CONTEXT *handle_context;

    handle_context = (HANDLE_CONTEXT *) handle;
    ofc_lock(HandleLock);

    handle_context->reference--;
    ofc_assert(handle_context->reference >= 0,
	       "Invalid handle reference count");

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
	ofc_unlock(HandleLock);
#if defined(OFC_HANDLE_DEBUG)
        ofc_handle_debug_free(handle_context) ;
#endif
#if defined(OFC_HANDLE_PERF)
        ofc_handle_print_interval("Handle: ", handle) ;
#endif
        ofc_free(handle_context);
    }
    else
      ofc_unlock(HandleLock);
}

OFC_CORE_LIB OFC_VOID ofc_handle_destroy(OFC_HANDLE handle) {
    HANDLE_CONTEXT *handle_context;

    handle_context = (HANDLE_CONTEXT *) handle;

    if (handle_context->wait_set != OFC_HANDLE_NULL)
        ofc_waitset_remove(handle_context->wait_set, handle);

    ofc_lock(HandleLock);
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

    handle_context->destroy = OFC_TRUE;

    if (handle_context->reference == 0)
      {
	ofc_unlock(HandleLock);

#if defined(OFC_HANDLE_DEBUG)
        ofc_handle_debug_free(handle_context) ;
#endif
#if defined(OFC_HANDLE_PERF)
        ofc_handle_print_interval("Handle: ", handle) ;
#endif
        ofc_free(handle_context);
      }
    else
      ofc_unlock(HandleLock);
}

OFC_CORE_LIB OFC_VOID
ofc_handle_set_app(OFC_HANDLE hHandle, OFC_HANDLE hApp, OFC_HANDLE hSet) {
    HANDLE_CONTEXT *handle_context;

    if (hHandle != OFC_HANDLE_NULL) {
        handle_context = (HANDLE_CONTEXT *) hHandle;

        handle_context->wait_app = hApp;
        handle_context->wait_set = hSet;
        ofc_waitset_set_assoc_impl(hHandle, hApp, hSet);
    }
}

OFC_CORE_LIB OFC_HANDLE ofc_handle_get_app(OFC_HANDLE hHandle) {
    HANDLE_CONTEXT *handle_context;

    handle_context = (HANDLE_CONTEXT *) hHandle;

    return (handle_context->wait_app);
}

OFC_CORE_LIB OFC_HANDLE ofc_handle_get_wait_set(OFC_HANDLE hHandle) {
    HANDLE_CONTEXT *handle_context;

    handle_context = (HANDLE_CONTEXT *) hHandle;

    return (handle_context->wait_set);
}

static HANDLE16_CONTEXT Handle16Array[OFC_MAX_HANDLE16];
static OFC_HANDLE Handle16Free;

#define HANDLE16_MAKE(ix, id) ((OFC_HANDLE16) (ix & 0xFF) << 8 | (id & 0xFF))
#define HANDLE16_INSTANCE(handle) ((OFC_UCHAR) ((handle >> 8) & 0xFF))
#define HANDLE16_ID(handle) ((OFC_UCHAR) (handle & 0xFF))

OFC_CORE_LIB OFC_VOID ofc_handle16_init(OFC_VOID) {
    OFC_INT i;

#if defined (OFC_HANDLE_DEBUG)
    OfcHandleDebugInit() ;
#endif

    OfcHandle16Mutex = ofc_lock_init();
    HandleLock = ofc_lock_init();

    Handle16Free = ofc_queue_create();

    for (i = 0; i < OFC_MAX_HANDLE16; i++) {
        Handle16Array[i].id = i;
        Handle16Array[i].instance = 0xFF;
        Handle16Array[i].context = OFC_NULL;
        Handle16Array[i].alloc = OFC_FALSE;
        Handle16Array[i].ref = 0;
        ofc_enqueue(Handle16Free, &Handle16Array[i]);
    }
}

OFC_CORE_LIB OFC_VOID ofc_handle16_free(OFC_VOID) {
    ofc_queue_clear(Handle16Free);
    ofc_queue_destroy(Handle16Free);

    ofc_lock_destroy(OfcHandle16Mutex);
    ofc_lock_destroy(HandleLock);
}

OFC_CORE_LIB OFC_VOID *ofc_handle16_lock(OFC_HANDLE16 hHandle) {
    OFC_VOID *ret;
    OFC_UCHAR id;
    HANDLE16_CONTEXT *handle;

    ret = OFC_NULL;
    ofc_lock(OfcHandle16Mutex);
    id = HANDLE16_ID (hHandle);
    if (id < OFC_MAX_HANDLE16) {
        handle = &Handle16Array[id];
        if (handle->instance == HANDLE16_INSTANCE(hHandle) &&
            handle->alloc == OFC_TRUE) {
            ret = handle->context;
            handle->ref++;
        }
    }

    ofc_unlock(OfcHandle16Mutex);
    return (ret);
}

OFC_CORE_LIB OFC_HANDLE16 ofc_handle16_create(OFC_VOID *context) {
    HANDLE16_CONTEXT *handle;
    OFC_HANDLE16 ret;

    ret = OFC_HANDLE16_INVALID;
    ofc_lock(OfcHandle16Mutex);
    handle = ofc_dequeue(Handle16Free);
    if (handle != OFC_NULL) {
        handle->instance++;
        if (handle->instance == 0xFF || handle->instance == 0)
            handle->instance = 1;
        handle->alloc = OFC_TRUE;
        ofc_assert(handle->ref == 0, "Handle16 ref not zero");
        handle->ref++;
        handle->context = context;
#if defined(OFC_HANDLE_DEBUG)
        OfcHandle16DebugAlloc (handle, RETURN_ADDRESS()) ;
#endif
        ret = HANDLE16_MAKE (handle->instance, handle->id);
    }
#if defined(OFC_HANDLE_DEBUG)
    else
      OfcHandle16DebugDump() ;
#endif

    if (ret == OFC_HANDLE16_INVALID) {
        ofc_process_crash("Handle16 Pool Exhausted\n");
    }

    ofc_unlock(OfcHandle16Mutex);
    return (ret);
}

OFC_CORE_LIB OFC_VOID ofc_handle16_destroy(OFC_HANDLE16 hHandle) {
    OFC_UCHAR id;
    HANDLE16_CONTEXT *handle;

    ofc_lock(OfcHandle16Mutex);
    id = HANDLE16_ID (hHandle);
    if (id < OFC_MAX_HANDLE16) {
        handle = &Handle16Array[id];
        if (handle->instance == HANDLE16_INSTANCE(hHandle) &&
            handle->alloc == OFC_TRUE) {
            handle->context = OFC_NULL;
            ofc_unlock(OfcHandle16Mutex);
            ofc_handle16_unlock(hHandle);
        } else {
            /*
             * Already destroyed
             */
            ofc_unlock(OfcHandle16Mutex);
        }
    } else {
        ofc_unlock(OfcHandle16Mutex);
        ofc_log(OFC_LOG_FATAL, "Bad Handle being destroyed 0x%08x, %d\n",
                   hHandle, id);
        ofc_process_crash("Bad Handle being destroyed\n");
    }
}

OFC_CORE_LIB OFC_VOID ofc_handle16_unlock(OFC_HANDLE16 hHandle) {
    OFC_UCHAR id;
    HANDLE16_CONTEXT *handle;

    ofc_lock(OfcHandle16Mutex);
    id = HANDLE16_ID (hHandle);
    if (id < OFC_MAX_HANDLE16) {
        handle = &Handle16Array[id];
        if (handle->instance == HANDLE16_INSTANCE(hHandle) &&
            handle->alloc == OFC_TRUE) {
            handle->ref--;
            if (handle->ref == 0) {
#if defined(OFC_HANDLE_DEBUG)
                OfcHandle16DebugFree (handle) ;
#endif
                handle->alloc = OFC_FALSE;
                ofc_enqueue(Handle16Free, handle);
            }
        }
    } else
        ofc_process_crash("Bad Handle being unlocked\n");
    ofc_unlock(OfcHandle16Mutex);
}

#if defined(OFC_HANDLE_PERF)
OFC_CORE_LIB OFC_VOID OfcHandleMeasure (OFC_HANDLE hHandle)
{
  HANDLE_CONTEXT *handle_context ;
  OFC_MSTIME now ;
  OFC_MSTIME interval ;

  if (hHandle != OFC_HANDLE_NULL)
    {
      now = ofc_time_get_now() ;
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

OFC_CORE_LIB OFC_MSTIME OfcHandleGetAvgInterval (OFC_HANDLE hHandle,
                            OFC_UINT32 *count,
                            OFC_HANDLE_TYPE *type)
{
  HANDLE_CONTEXT *handle_context ;
  OFC_MSTIME interval ;

  if (hHandle != OFC_HANDLE_NULL)
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

static OFC_CHAR *type2str (OFC_HANDLE_TYPE type)
{
  static struct
  {
    OFC_HANDLE_TYPE type;
    OFC_CHAR *str;
  } typearray[] =
      {
    { OFC_HANDLE_UNKNOWN, "Unknown" },
    { OFC_HANDLE_WAIT_SET, "Wait Set" },
    { OFC_HANDLE_QUEUE, "Queue" },
    { OFC_HANDLE_FILE,	"File" },
    { OFC_HANDLE_FSWIN32_FILE,	"Win32 File" },
    { OFC_HANDLE_FSWIN32_OVERLAPPED, "Win32 Buffer" },
    { OFC_HANDLE_FSSMB_OVERLAPPED, "Cifs Buffer" },
    { OFC_HANDLE_FSDARWIN_OVERLAPPED, "Darwin Buffer" },
    { OFC_HANDLE_FSRESOLVER_OVERLAPPED, "Resolver Buffer" },
    { OFC_HANDLE_FSLINUX_OVERLAPPED, "Linux Buffer" },
    { OFC_HANDLE_FSFILEX_OVERLAPPED, "FILEX Buffer" },
    { OFC_HANDLE_FSNUFILE_OVERLAPPED, "NUFILE Buffer" },
    { OFC_HANDLE_FSANDROID_OVERLAPPED, "Android Buffer" },
    { OFC_HANDLE_FSDARWIN_FILE, "Darwin File" },
    { OFC_HANDLE_FSLINUX_FILE,	"Linux File" },
    { OFC_HANDLE_FSFILEX_FILE,	"FILEX File" },
    { OFC_HANDLE_FSANDROID_FILE, "Android File" },
    { OFC_HANDLE_FSNUFILE_FILE, "NUFILE File" },
    { OFC_HANDLE_FSOTHER_FILE, "Other File" },
    { OFC_HANDLE_FSBROWSER_FILE, "Browser File" },
    { OFC_HANDLE_FSBOOKMARK_FILE, "Bookmark File" },
    { OFC_HANDLE_SCHED, "Scheduler" },
    { OFC_HANDLE_APP, "App" },
    { OFC_HANDLE_THREAD, "Thread" },
    { OFC_HANDLE_SOCKET, "Socket" },
    { OFC_HANDLE_SOCKET_IMPL, "Socket Impl" },
    { OFC_HANDLE_EVENT, "Event" },
    { OFC_HANDLE_TIMER, "Timer" },
    { OFC_HANDLE_PIPE,	"Pipe" },
    { OFC_HANDLE_WAIT_QUEUE, "Wait Queue" },
    { OFC_HANDLE_TRANSACTION, "Transaction" },
    { OFC_HANDLE_SMB_FILE, "SMB File" },
    { OFC_HANDLE_MAILSLOT, "Mailslot" },
    { OFC_HANDLE_PROCESS, "Process" },
    { OFC_HANDLE_NUM, OFC_NULL }
      } ;
  OFC_INT i ;

  for (i = 0 ; i < OFC_HANDLE_NUM && typearray[i].type != type; i++) ;

  return (typearray[i].str) ;
}

OFC_CORE_LIB OFC_VOID ofc_handle_print_interval_header(OFC_VOID)
{
  ofc_printf ("%-15.15s  %15.15s  %15.15s\n",
           "Handle Type", "Avg Interval", "Count") ;
}

OFC_CORE_LIB OFC_VOID
ofc_handle_print_interval(OFC_CHAR *prefix, OFC_HANDLE hHandle)
{
  OFC_MSTIME interval ;
  OFC_UINT32 count ;
  OFC_HANDLE_TYPE type ;

  interval = OfcHandleGetAvgInterval (hHandle, &count, &type) ;

  if (count > 0)
    {
      if (type == OFC_HANDLE_TIMER)
	{
	  ofc_printf ("%s%-15.15s  %12lu ms  %15lu  %s\n", prefix,
		      type2str(type),
		      interval, count, ofc_timer_id (hHandle)) ;
	}
      else
	{
	  ofc_printf ("%s%-15.15s  %12lu ms  %15lu\n", prefix,
		      type2str(type), interval, count) ;
	}
    }
}
#endif
