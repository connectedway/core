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

OFC_CORE_LIB OFC_HANDLE
BlueTimerCreate (OFC_CCHAR *id)
{
  BLUE_TIMER *pTimer ;
  OFC_HANDLE hTimer ;

  pTimer = ofc_malloc (sizeof (BLUE_TIMER)) ;
  pTimer->expiration_time = 0 ;
  pTimer->id = id ;
  hTimer = ofc_handle_create (OFC_HANDLE_TIMER, pTimer) ;
  return (hTimer) ;
}

OFC_CORE_LIB OFC_CCHAR *BlueTimerID (OFC_HANDLE hTimer)
{
  BLUE_TIMER *pTimer ;
  OFC_CCHAR *id ;

  pTimer = ofc_handle_lock (hTimer) ;
  id = OFC_NULL ;
  if (pTimer != OFC_NULL)
    {
      id = pTimer->id ;
      ofc_handle_unlock (hTimer) ;
    }
  return (id) ;
}

OFC_CORE_LIB OFC_MSTIME
BlueTimerGetWaitTime (OFC_HANDLE hTimer)
{
  OFC_MSTIME now ;
  BLUE_TIMER *pTimer ;
  OFC_MSTIME ret ;

  ret = 0 ;
  pTimer = ofc_handle_lock (hTimer) ;
  if (pTimer != OFC_NULL)
    {
      OFC_MSTIME elapsed ;
      now = BlueTimeGetNow() ;
      elapsed = pTimer->expiration_time - now ;
      if (elapsed > 0)
	ret = elapsed ;
      ofc_handle_unlock (hTimer) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueTimerSet (OFC_HANDLE hTimer, OFC_MSTIME delta)
{
  BLUE_TIMER *pTimer ;

  pTimer = ofc_handle_lock (hTimer) ;
  if (pTimer != OFC_NULL)
    {
      pTimer->expiration_time = BlueTimeGetNow() + delta ;
      ofc_handle_unlock (hTimer) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueTimerDestroy (OFC_HANDLE hTimer)
{
  BLUE_TIMER *pTimer ;

  pTimer = ofc_handle_lock (hTimer) ;
  if (pTimer != OFC_NULL)
    {
      ofc_free (pTimer) ;
      ofc_handle_destroy (hTimer) ;
      ofc_handle_unlock (hTimer) ;
    }
}
