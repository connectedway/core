/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__BLUE_TIME_IMPL_H__)
#define __BLUE_TIME_IMPL_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/file.h"

/**
 * \defgroup BlueTimeImpl Time Management Implementation
 * \ingroup BluePort
 *
 * This facility provides the platform dependent implementation of time
 * functions
 */

/** \{ */

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Return a millisecond tick count
   *
   * \returns
   * A Millisecond Tick Count
   */
  BLUE_MSTIME BlueTimeGetNowImpl(BLUE_VOID) ;
  /**
   * Get Time of Day in BLUE_FILETIME format
   *
   * \param now
   * Pointer to where to store the current file time
   */
  BLUE_VOID BlueTimeGetFileTimeImpl (BLUE_FILETIME *now) ;
  /**
   * Get The local time zone offset from UTC
   *
   * \returns
   * The number of minutes from UTC that this timezone is in
   */
  BLUE_UINT16 BlueTimeGetTimeZoneImpl (BLUE_VOID) ;
  /**
   * Convert a File Time to a DOS Date Time
   *
   * \param lpFileTime
   * File Time to convert
   *
   * \param lpFatDate
   * Pointer to Fat Day Word
   *
   * \param lpFatTime
   * Pointer to Fat Seconds Word
   *
   * \returns
   * BLUE_TRUE if successful, BLUE_FALSE otherwise
   */
  BLUE_BOOL 
  BlueFileTimeToDosDateTimeImpl (const BLUE_FILETIME *lpFileTime,
				 BLUE_WORD *lpFatDate,
				 BLUE_WORD *lpFatTime) ;
  /**
   * Convert a DOS Date Time to a File Time
   *
   * \param FatDate
   * DOS Day Word
   *
   * \param FatTime
   * DOS Seconds Word
   *
   * \param lpFileTime
   * Pointer to where to store the file time
   *
   * \returns
   * BLUE_TRUE if success, BLUE_FALSE otherwise
   */
  BLUE_BOOL 
  BlueDosDateTimeToFileTimeImpl (BLUE_WORD FatDate, 
				 BLUE_WORD FatTime,
				 BLUE_FILETIME *lpFileTime) ;
  /**
   * Returns Number of US of runtime
   *
   * \returns
   * Runtime in us
   */
  BLUE_MSTIME BlueTimeGetRuntimeImpl (BLUE_VOID) ;

#if defined(__cplusplus)
}
#endif
/** \} */
#endif

