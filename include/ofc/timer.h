/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

#if !defined(__BLUE_TIMER_H__)
#define __BLUE_TIMER_H__

/**
 * \defgroup BlueTimer Timer Facility
 *
 * \ingroup BlueUtil
 */

/** \{ */

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Create a timer
   *
   * \returns 
   * A handle to the timer
   */
  OFC_CORE_LIB BLUE_HANDLE
  BlueTimerCreate (OFC_CCHAR *id) ;
  /**
   * Get the remaining wait time for a timer
   *
   * \param hTimer
   * The handle to the timer to return the wait time for
   *
   * \returns
   * The number of milliseconds that are left on the timer.  If the timer
   * is expired, a zero is returned.
   */
  OFC_CORE_LIB OFC_MSTIME
  BlueTimerGetWaitTime (BLUE_HANDLE hTimer) ;
  /**
   * Set a timer with a millisecond count
   * 
   * \param hTimer
   * The timer to set
   *
   * \param delta
   * The number of milliseconds to tick for
   */
  OFC_CORE_LIB OFC_VOID
  BlueTimerSet (BLUE_HANDLE hTimer, OFC_MSTIME delta) ;
  /**
   * Destroy a timer
   *
   * \param hTimer
   * The handle to the timer to destroy
   */
  OFC_CORE_LIB OFC_VOID
  BlueTimerDestroy (BLUE_HANDLE hTimer) ;

  OFC_CORE_LIB OFC_CCHAR *BlueTimerID (BLUE_HANDLE hTimer);
#if defined(__cplusplus)
}
#endif

#endif

/** \} */
