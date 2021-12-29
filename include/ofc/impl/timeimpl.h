/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_TIME_IMPL_H__)
#define __OFC_TIME_IMPL_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/file.h"

/**
 * \defgroup time_impl Time Management Implementation
 * \ingroup port
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
OFC_MSTIME ofc_time_get_now_impl(OFC_VOID);

/**
 * Get Time of Day in OFC_FILETIME format
 *
 * \param now
 * Pointer to where to store the current file time
 */
OFC_VOID ofc_time_get_file_time_impl(OFC_FILETIME *now);

/**
 * Get The local time zone offset from UTC
 *
 * \returns
 * The number of minutes from UTC that this timezone is in
 */
OFC_UINT16 ofc_time_get_timezone_impl(OFC_VOID);

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
 * OFC_TRUE if successful, OFC_FALSE otherwise
 */
OFC_BOOL
ofc_file_time_to_dos_date_time_impl(const OFC_FILETIME *lpFileTime,
                                    OFC_WORD *lpFatDate,
                                    OFC_WORD *lpFatTime);

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
 * OFC_TRUE if success, OFC_FALSE otherwise
 */
OFC_BOOL
ofc_dos_date_time_to_file_time_impl(OFC_WORD FatDate,
                                    OFC_WORD FatTime,
                                    OFC_FILETIME *lpFileTime);

/**
 * Returns Number of US of runtime
 *
 * \returns
 * Runtime in us
 */
OFC_MSTIME ofc_get_runtime_impl(OFC_VOID);

#if defined(__cplusplus)
}
#endif
/** \} */
#endif

