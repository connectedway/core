/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

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
 *   OFC_HANDLE_WAIT_QUEUE
 *   OFC_HANDLE_FILE
 *   OFC_HANDLE_SOCKET
 *   OFC_HANDLE_EVENT
 *   OFC_HANDLE_TIMER
 *   OFC_HANDLE_PIPE
 *   OFC_HANDLE_MAILSLOT
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
 * actual number is build specific (OFC_NUM_OVERLAPPED_IOS).  Each 
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

OFC_CORE_LIB OFC_HANDLE
BlueWaitSetCreate (OFC_VOID)
{
  WAIT_SET *pWaitSet ;
  OFC_HANDLE handle ;

  pWaitSet = ofc_malloc (sizeof (WAIT_SET)) ;
  pWaitSet->hHandleQueue = BlueQcreate() ;
  BlueWaitSetCreateImpl (pWaitSet) ;
  handle = ofc_handle_create (OFC_HANDLE_WAIT_SET, pWaitSet) ;
  /* extra for create */
  ofc_handle_lock (handle) ;
  return (handle) ;
}
  
OFC_CORE_LIB OFC_VOID
BlueWaitSetClear (OFC_HANDLE handle)
{
  WAIT_SET *pWaitSet ;
  OFC_HANDLE hEventHandle ;

  pWaitSet = ofc_handle_lock (handle) ;
  if (pWaitSet != OFC_NULL)
    {
      for (hEventHandle = 
	     (OFC_HANDLE) BlueQdequeue (pWaitSet->hHandleQueue) ;
           hEventHandle != OFC_HANDLE_NULL ;
	   hEventHandle = 
	     (OFC_HANDLE) BlueQdequeue (pWaitSet->hHandleQueue) )
	{
	  ofc_handle_set_app (hEventHandle, OFC_HANDLE_NULL, OFC_HANDLE_NULL) ;
	}
      ofc_handle_unlock (handle) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueWaitSetClearApp (OFC_HANDLE handle, OFC_HANDLE hApp)
{
  WAIT_SET *pWaitSet ;
  OFC_HANDLE hEventHandle ;
  OFC_HANDLE hNext ;

  pWaitSet = ofc_handle_lock (handle) ;
  if (pWaitSet != OFC_NULL)
    {
      for (hEventHandle = 
	     (OFC_HANDLE) BlueQfirst (pWaitSet->hHandleQueue) ;
           hEventHandle != OFC_HANDLE_NULL ;)
	{
	  hNext = (OFC_HANDLE) BlueQnext (pWaitSet->hHandleQueue,
                                      (OFC_VOID *) hEventHandle) ;

	  if (ofc_handle_get_app (hEventHandle) == hApp)
	    {
	      BlueQunlink (pWaitSet->hHandleQueue, 
			   (OFC_VOID *) hEventHandle) ;
	      ofc_handle_set_app (hEventHandle,
                              OFC_HANDLE_NULL, OFC_HANDLE_NULL) ;
	    }

	  hEventHandle = hNext ;
	}
      ofc_handle_unlock (handle) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueWaitSetDestroy (OFC_HANDLE handle)
{
  WAIT_SET *pWaitSet ;
  OFC_HANDLE hEventHandle ;

  pWaitSet = ofc_handle_lock (handle) ;
  if (pWaitSet != OFC_NULL)
    {
      for (hEventHandle = 
	     (OFC_HANDLE) BlueQdequeue (pWaitSet->hHandleQueue) ;
           hEventHandle != OFC_HANDLE_NULL ;
	   hEventHandle = 
	     (OFC_HANDLE) BlueQdequeue (pWaitSet->hHandleQueue) )
	{
	  ofc_handle_set_app (hEventHandle, OFC_HANDLE_NULL, OFC_HANDLE_NULL) ;
	}
      BlueQdestroy (pWaitSet->hHandleQueue) ;
      ofc_handle_destroy (handle) ;
      ofc_handle_unlock (handle) ;
      BlueWaitSetDestroyImpl (pWaitSet) ;
      ofc_free (pWaitSet) ;
      /* second unlock to balance extra on create */
      ofc_handle_unlock (handle) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueWaitSetAdd (OFC_HANDLE hSet, OFC_HANDLE hApp, OFC_HANDLE hEvent)
{
  WAIT_SET *pWaitSet ;

  pWaitSet = ofc_handle_lock (hSet) ;
  if (pWaitSet != OFC_NULL)
    {
      BlueWaitSetAddImpl (hSet, hApp, hEvent) ;
      ofc_handle_set_app (hEvent, hApp, hSet) ;
      BlueQenqueue (pWaitSet->hHandleQueue, (OFC_VOID *) hEvent) ;
      ofc_handle_unlock (hSet) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueWaitSetRemove (OFC_HANDLE hSet, OFC_HANDLE hEvent)
{
  WAIT_SET *pWaitSet ;

  pWaitSet = ofc_handle_lock (hSet) ;
  if (pWaitSet != OFC_NULL)
    {
      BlueQunlink (pWaitSet->hHandleQueue, (OFC_VOID *) hEvent) ;
      ofc_handle_set_app (hEvent, OFC_HANDLE_NULL, OFC_HANDLE_NULL) ;
      ofc_handle_unlock (hSet) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueWaitSetWake (OFC_HANDLE handle)
{
  BlueWaitSetWakeImpl (handle) ;
}

OFC_CORE_LIB OFC_HANDLE
BlueWaitSetWait (OFC_HANDLE handle)
{
  return (BlueWaitSetWaitImpl (handle)) ;
}
 
#if defined(OFC_HANDLE_PERF)
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
