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
#include "ofc/types.h"
#include "ofc/timer.h"
#include "ofc/handle.h"
#include "ofc/time.h"
#include "ofc/heap.h"

typedef struct
{
  BLUE_MSTIME  expiration_time ;
  BLUE_CCHAR *id ;
} BLUE_TIMER ;

BLUE_CORE_LIB BLUE_HANDLE 
BlueTimerCreate (BLUE_CCHAR *id)
{
  BLUE_TIMER *pTimer ;
  BLUE_HANDLE hTimer ;

  pTimer = BlueHeapMalloc (sizeof (BLUE_TIMER)) ;
  pTimer->expiration_time = 0 ;
  pTimer->id = id ;
  hTimer = BlueHandleCreate (BLUE_HANDLE_TIMER, pTimer) ;
  return (hTimer) ;
}

BLUE_CORE_LIB BLUE_CCHAR *BlueTimerID (BLUE_HANDLE hTimer)
{
  BLUE_TIMER *pTimer ;
  BLUE_CCHAR *id ;

  pTimer = BlueHandleLock (hTimer) ;
  id = BLUE_NULL ;
  if (pTimer != BLUE_NULL)
    {
      id = pTimer->id ;
      BlueHandleUnlock (hTimer) ;
    }
  return (id) ;
}

BLUE_CORE_LIB BLUE_MSTIME 
BlueTimerGetWaitTime (BLUE_HANDLE hTimer)
{
  BLUE_MSTIME now ;
  BLUE_TIMER *pTimer ;
  BLUE_MSTIME ret ;

  ret = 0 ;
  pTimer = BlueHandleLock (hTimer) ;
  if (pTimer != BLUE_NULL)
    {
      BLUE_MSTIME elapsed ;
      now = BlueTimeGetNow() ;
      elapsed = pTimer->expiration_time - now ;
      if (elapsed > 0)
	ret = elapsed ;
      BlueHandleUnlock (hTimer) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueTimerSet (BLUE_HANDLE hTimer, BLUE_MSTIME delta)
{
  BLUE_TIMER *pTimer ;

  pTimer = BlueHandleLock (hTimer) ;
  if (pTimer != BLUE_NULL)
    {
      pTimer->expiration_time = BlueTimeGetNow() + delta ;
      BlueHandleUnlock (hTimer) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueTimerDestroy (BLUE_HANDLE hTimer)
{
  BLUE_TIMER *pTimer ;

  pTimer = BlueHandleLock (hTimer) ;
  if (pTimer != BLUE_NULL)
    {
      BlueHeapFree (pTimer) ;
      BlueHandleDestroy (hTimer) ;
      BlueHandleUnlock (hTimer) ;
    }
}
