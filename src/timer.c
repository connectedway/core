/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/timer.h"
#include "ofc/handle.h"
#include "ofc/time.h"
#include "ofc/heap.h"

typedef struct
{
  OFC_MSTIME  expiration_time ;
  OFC_CCHAR *id ;
} BLUE_TIMER ;

OFC_CORE_LIB BLUE_HANDLE
BlueTimerCreate (OFC_CCHAR *id)
{
  BLUE_TIMER *pTimer ;
  BLUE_HANDLE hTimer ;

  pTimer = BlueHeapMalloc (sizeof (BLUE_TIMER)) ;
  pTimer->expiration_time = 0 ;
  pTimer->id = id ;
  hTimer = BlueHandleCreate (BLUE_HANDLE_TIMER, pTimer) ;
  return (hTimer) ;
}

OFC_CORE_LIB OFC_CCHAR *BlueTimerID (BLUE_HANDLE hTimer)
{
  BLUE_TIMER *pTimer ;
  OFC_CCHAR *id ;

  pTimer = BlueHandleLock (hTimer) ;
  id = OFC_NULL ;
  if (pTimer != OFC_NULL)
    {
      id = pTimer->id ;
      BlueHandleUnlock (hTimer) ;
    }
  return (id) ;
}

OFC_CORE_LIB OFC_MSTIME
BlueTimerGetWaitTime (BLUE_HANDLE hTimer)
{
  OFC_MSTIME now ;
  BLUE_TIMER *pTimer ;
  OFC_MSTIME ret ;

  ret = 0 ;
  pTimer = BlueHandleLock (hTimer) ;
  if (pTimer != OFC_NULL)
    {
      OFC_MSTIME elapsed ;
      now = BlueTimeGetNow() ;
      elapsed = pTimer->expiration_time - now ;
      if (elapsed > 0)
	ret = elapsed ;
      BlueHandleUnlock (hTimer) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueTimerSet (BLUE_HANDLE hTimer, OFC_MSTIME delta)
{
  BLUE_TIMER *pTimer ;

  pTimer = BlueHandleLock (hTimer) ;
  if (pTimer != OFC_NULL)
    {
      pTimer->expiration_time = BlueTimeGetNow() + delta ;
      BlueHandleUnlock (hTimer) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueTimerDestroy (BLUE_HANDLE hTimer)
{
  BLUE_TIMER *pTimer ;

  pTimer = BlueHandleLock (hTimer) ;
  if (pTimer != OFC_NULL)
    {
      BlueHeapFree (pTimer) ;
      BlueHandleDestroy (hTimer) ;
      BlueHandleUnlock (hTimer) ;
    }
}
