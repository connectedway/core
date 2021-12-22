/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __BLUE_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/queue.h"
#include "ofc/waitset.h"
#include "ofc/libc.h"
#include "ofc/impl/waitsetimpl.h"

#include "ofc/heap.h"

/*
 * pass the handle set off to a platform dependent layer
 *
 * The platform layer needs to decide on a scheduling scheme.
 * Each handle has a type (see BlueHandle.h).  Some Types are 'scheduling 
 * entities'.  Which are scheduling entities and which are not is platform 
 * dependent but if an appis trying to schedule on a handle which is not a 
 * scheduling entity for the platform, well, the app may not wake up when 
 * expected.  Therefore, an app should be designed to only wait on handles 
 * that are defined as scheduling entities for all platforms that the app 
 * is being targeted to.  For the BlueShare product, the handles that are
 * scheduling entities are:
 *
 *   BLUE_HANDLE_WAIT_QUEUE
 *   BLUE_HANDLE_FILE
 *   BLUE_HANDLE_SOCKET
 *   BLUE_HANDLE_EVENT
 *   BLUE_HANDLE_TIMER
 *   BLUE_HANDLE_PIPE
 *   BLUE_HANDLE_MAILSLOT
 *
 * How these are handled is a platform dependent decision.  Some thoughts 
 * about the three platforms currently supported by BlueShare: win32, and 
 * posix.
 *
 * Some platforms support the notion of a scheduling entity of a FIFO or 
 * Queue.  Win32 for instance has the SList facitlity.  So that behavior of
 * queues is consistent across platforms, we will associate an event with a 
 * queue on all platforms.  Therefore, for the purpose of handle scheduling, 
 * a queue is equivalent to an event an all platforms.
 *
 * For Win32, FILE, PIPe, SOCKET, EVENT, and TIMERs all map to platform 
 * handles that are referred to as synchronization objects.  This implies 
 * that they can all be used by the WaitForMultipleObjects call.  This makes 
 * Win32 the easiest platform to implement.
 *
 * Posix is a more interesting case.  There is no single posix call that 
 * will allow a thread to wait on some combination of PIPE, FILE, SOCKET, 
 * EVENT, and TIMER.  What there is though is a select or poll call which 
 * allows synchronization on FILE, PIPEs and SOCKET events.  It also takes a 
 * timeout argument which allows the wait routine to collapse all the timer 
 * objects to a single expiration time and use with the select or poll call.
 *
 * This leaves the event object.  Each handle set will have one PIPE 
 * associated with it.  Whan a handle set attemts to wait on an EVENT, the 
 * EVENT is associated with the handle set's PIPE.  When an EVENT is 
 * triggered, the handle to the EVENT is passed through the PIPE.  Part of 
 * the post select handling is to drain the pipe of all handles.  The post 
 * processing adds the handle to the list of handles that triggered from the 
 * select call.
 *
 * Some RTOSes are a little more difficult.  There are a few synchronization 
 * methods that are available (a wait on events and a 
 * selet call for network sockets) but there's no call that allows a hybrid 
 * of waitable objects.  So the scheme we use is that the scheduling call 
 * is a select which works for SOCKET and TIMERs.
 *
 * Some RTOSes allow a Resume call to wake up a threaad waiting on a select so
 * setting an event will add the event to the triggered list and resume the 
 * thread.  
 *
 * This facility consists of a number of threads that perform I/O.  The 
 * actual number is build specific (BLUE_PARAM_NUM_OVERLAPPED_IOS).  Each 
 * thread is a BlueScheduler and each of these schedulers have one app.  
 * There's a queue associated with each of these apps.  When a file I/O 
 * occurs, a message is built defining the I/O and queued to a free I/O app.  
 * If no free I/O app is found, it is queued to a round robin app.  
 * Queing the message to the app will trigger the associated event for that 
 * app.  This wakes the app, it performs the I/O, queues the response the 
 * FILE handle, adds the file handle to the triggered list and resumes the 
 * initiating scheduler.
 *
 * This is expected to be the most difficult aspect of porting BlueShare 
 * to new platforms.  We provide PSPs for all three platforms so a 
 * developer should be able to use the techniques available to design a 
 * scheme that works for the target platform.
 */

BLUE_CORE_LIB BLUE_HANDLE 
BlueWaitSetCreate (BLUE_VOID) 
{
  WAIT_SET *pWaitSet ;
  BLUE_HANDLE handle ;

  pWaitSet = BlueHeapMalloc (sizeof (WAIT_SET)) ;
  pWaitSet->hHandleQueue = BlueQcreate() ;
  BlueWaitSetCreateImpl (pWaitSet) ;
  handle = BlueHandleCreate (BLUE_HANDLE_WAIT_SET, pWaitSet) ;
  /* extra for create */
  BlueHandleLock (handle) ;
  return (handle) ;
}
  
BLUE_CORE_LIB BLUE_VOID 
BlueWaitSetClear (BLUE_HANDLE handle) 
{
  WAIT_SET *pWaitSet ;
  BLUE_HANDLE hEventHandle ;

  pWaitSet = BlueHandleLock (handle) ;
  if (pWaitSet != BLUE_NULL)
    {
      for (hEventHandle = 
	     (BLUE_HANDLE) BlueQdequeue (pWaitSet->hHandleQueue) ;
	   hEventHandle != BLUE_HANDLE_NULL ;
	   hEventHandle = 
	     (BLUE_HANDLE) BlueQdequeue (pWaitSet->hHandleQueue) ) 
	{
	  BlueHandleSetApp (hEventHandle, BLUE_HANDLE_NULL, BLUE_HANDLE_NULL) ;
	}
      BlueHandleUnlock (handle) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueWaitSetClearApp (BLUE_HANDLE handle, BLUE_HANDLE hApp) 
{
  WAIT_SET *pWaitSet ;
  BLUE_HANDLE hEventHandle ;
  BLUE_HANDLE hNext ;

  pWaitSet = BlueHandleLock (handle) ;
  if (pWaitSet != BLUE_NULL)
    {
      for (hEventHandle = 
	     (BLUE_HANDLE) BlueQfirst (pWaitSet->hHandleQueue) ;
	   hEventHandle != BLUE_HANDLE_NULL ;)
	{
	  hNext = (BLUE_HANDLE) BlueQnext (pWaitSet->hHandleQueue, 
					   (BLUE_VOID *) hEventHandle) ;

	  if (BlueHandleGetApp (hEventHandle) == hApp)
	    {
	      BlueQunlink (pWaitSet->hHandleQueue, 
			   (BLUE_VOID *) hEventHandle) ;
	      BlueHandleSetApp (hEventHandle, 
				BLUE_HANDLE_NULL, BLUE_HANDLE_NULL) ;
	    }

	  hEventHandle = hNext ;
	}
      BlueHandleUnlock (handle) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueWaitSetDestroy (BLUE_HANDLE handle)
{
  WAIT_SET *pWaitSet ;
  BLUE_HANDLE hEventHandle ;

  pWaitSet = BlueHandleLock (handle) ;
  if (pWaitSet != BLUE_NULL)
    {
      for (hEventHandle = 
	     (BLUE_HANDLE) BlueQdequeue (pWaitSet->hHandleQueue) ;
	   hEventHandle != BLUE_HANDLE_NULL ;
	   hEventHandle = 
	     (BLUE_HANDLE) BlueQdequeue (pWaitSet->hHandleQueue) ) 
	{
	  BlueHandleSetApp (hEventHandle, BLUE_HANDLE_NULL, BLUE_HANDLE_NULL) ;
	}
      BlueQdestroy (pWaitSet->hHandleQueue) ;
      BlueHandleDestroy (handle) ;
      BlueHandleUnlock (handle) ;
      BlueWaitSetDestroyImpl (pWaitSet) ;
      BlueHeapFree (pWaitSet) ;
      /* second unlock to balance extra on create */
      BlueHandleUnlock (handle) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueWaitSetAdd (BLUE_HANDLE hSet, BLUE_HANDLE hApp, BLUE_HANDLE hEvent)
{
  WAIT_SET *pWaitSet ;

  pWaitSet = BlueHandleLock (hSet) ;
  if (pWaitSet != BLUE_NULL)
    {
      BlueWaitSetAddImpl (hSet, hApp, hEvent) ;
      BlueHandleSetApp (hEvent, hApp, hSet) ;
      BlueQenqueue (pWaitSet->hHandleQueue, (BLUE_VOID *) hEvent) ;
      BlueHandleUnlock (hSet) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueWaitSetRemove (BLUE_HANDLE hSet, BLUE_HANDLE hEvent)
{
  WAIT_SET *pWaitSet ;

  pWaitSet = BlueHandleLock (hSet) ;
  if (pWaitSet != BLUE_NULL)
    {
      BlueQunlink (pWaitSet->hHandleQueue, (BLUE_VOID *) hEvent) ;
      BlueHandleSetApp (hEvent, BLUE_HANDLE_NULL, BLUE_HANDLE_NULL) ;
      BlueHandleUnlock (hSet) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueWaitSetWake (BLUE_HANDLE handle)
{
  BlueWaitSetWakeImpl (handle) ;
}

BLUE_CORE_LIB BLUE_HANDLE 
BlueWaitSetWait (BLUE_HANDLE handle)
{
  return (BlueWaitSetWaitImpl (handle)) ;
}
 
#if defined(BLUE_PARAM_HANDLE_PERF)
BLUE_CORE_LIB BLUE_VOID 
BlueWaitSetLogMeasure (BLUE_HANDLE handle) 
{
  WAIT_SET *pWaitSet ;
  BLUE_HANDLE hEventHandle ;

  pWaitSet = BlueHandleLock (handle) ;
  if (pWaitSet != BLUE_NULL)
    {
      BlueHandlePrintIntervalHeader() ;
      for (hEventHandle = 
	     (BLUE_HANDLE) BlueQfirst (pWaitSet->hHandleQueue) ;
	   hEventHandle != BLUE_HANDLE_NULL ;
	   hEventHandle = 
	     (BLUE_HANDLE) BlueQnext (pWaitSet->hHandleQueue, hEventHandle) ) 
	{
	  BlueHandlePrintInterval ("", hEventHandle) ;
	}
      BlueHandleUnlock (handle) ;
    }
}
#endif
