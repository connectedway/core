/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_TIME_H__)
#define __OFC_TIME_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/file.h"

/**
 * \defgroup Open Files Time Management
 *
 * This facility provides Time conversion functions
 */

/** \{ */

#if defined(OFC_PERF_STATS)
/**
 * A structure for helping us measure queing delay
 */
typedef enum {
    OFC_PERF_FSSMB_READ = 0,
    OFC_PERF_FSSMB_WRITE = 1,
    OFC_PERF_CLIENT_READ = 2,
    OFC_PERF_CLIENT_WRITE = 3,
    OFC_PERF_SERVER_READ = 4,
    OFC_PERF_SERVER_WRITE = 5,
    OFC_PERF_SESSION_READ = 6,
    OFC_PERF_SESSION_WRITE = 7,
    OFC_PERF_NUM = 8
} OFC_TIME_PERF_ID;

typedef struct {
    OFC_TIME_PERF_ID id;    /**< This perf id */
    OFC_MSTIME elapsed;        /**< The total wait time  */
    OFC_ULONG depthsum;        /**< The cumulative queue depth  */
    OFC_ULONG depth;        /**< The current queue depth  */
    OFC_ULONG count;        /**< The number of I/Os */
    OFC_LONG totalbytes;    /**< Sum of bytes  */
    OFC_MSTIME runtime_start;    /**< Runtime at the last reset */
    OFC_MSTIME runtime_end;    /**< Runtime at the last measurement */
} OFC_PERF_STAT;

#endif

/**
 * Definition for DOS Day Field in DOS Day Word
 */
#define OFC_DOS_DAY 0x001F
/**
 * Bit position for DOS Day Field
 */
#define OFC_DOS_DAY_SHIFT 0
/**
 * Definition for DOS Month Field in DOS Day Word
 */
#define OFC_DOS_MONTH 0x01E0
/**
 * Bit position for DOS Month Field
 */
#define OFC_DOS_MONTH_SHIFT 5
/**
 * Definition for DOS Year Field in DOS Day Word
 */
#define OFC_DOS_YEAR 0xFE00
/**
 * Bit Position for DOS Year Field
 */
#define OFC_DOS_YEAR_SHIFT 9
/**
 * Relative Offset of DOS Year
 */
#define OFC_DOS_YEAR_BASE 1980
/**
 * Defition of DOS Seconds Field in DOS Seconds Word
 */
#define OFC_DOS_SECS 0x001F
/**
 * Bit position of DOS Seconds Field
 */
#define OFC_DOS_SECS_SHIFT 0
/**
 * Definition of DOS Minute Field in DOS Seconds Word
 */
#define OFC_DOS_MINS 0x07E0
/**
 * Bit Position of DOS Minute Field
 */
#define OFC_DOS_MINS_SHIFT 5
/**
 * Definition of DOS Hours Field in DOS Seconds Word
 */
#define OFC_DOS_HRS 0xF800
/**
 * Bit position of DOS Hours Field
 */
#define OFC_DOS_HRS_SHIFT 11

/*
 * This brings us to 1900
 */
#define OFC_TIME_S_EPOCH_OFFSET_1900 11644473600LL
/*
 * This brings us to 1970
 */
#define OFC_TIME_S_EPOCH_OFFSET_1970 9435484800LL

#if defined(__cplusplus)
extern "C"
{
#endif

OFC_VOID epoch_time_to_file_time(const OFC_ULONG tv_sec,
                                 const OFC_ULONG tv_nsec,
                                 OFC_FILETIME *filetime);

OFC_VOID file_time_to_epoch_time(const OFC_FILETIME *filetime,
                                 OFC_ULONG *tv_sec,
                                 OFC_ULONG *tv_nsec);
/**
 * Get a millisecond tick count
 *
 * \returns
 * A Millisecond Tick Count
 */
OFC_CORE_LIB OFC_MSTIME
ofc_time_get_now(OFC_VOID);
/**
 * Get Time of Day in OFC_FILETIME format
 *
 * \param now
 * Pointer to where to store the current file time
 */
OFC_CORE_LIB OFC_VOID
ofc_time_get_file_time(OFC_FILETIME *now);
/**
 * Get The local time zone offset from UTC
 *
 * \returns
 * The number of minutes from UTC that this timezone is in
 */
OFC_CORE_LIB OFC_UINT16
ofc_time_get_time_zone(OFC_VOID);
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
OFC_CORE_LIB OFC_BOOL
ofc_file_time_to_dos_date_time(const OFC_FILETIME *lpFileTime,
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
OFC_CORE_LIB OFC_BOOL
ofc_dos_date_time_to_file_time(OFC_WORD FatDate, OFC_WORD FatTime,
                               OFC_FILETIME *lpFileTime);
/**
 * Build A DOS Date Time from components
 *
 * \param month
 * The month where January is 1
 *
 * \param day
 * The day of month (first day is 1)
 *
 * \param year
 * Year based at 1, in other words, the regular year.  For instance, 2008.
 *
 * \param hour
 * The hour in a 24 hour clock.  Midnight is 0, Noon is 12.
 *
 * \param min
 * The minute in the hour (0 - 59)
 *
 * \param sec
 * The second in a minute (0 - 59)
 *
 * \param lpFatDate
 * A pointer to where to store the date in DOS format
 *
 * \param lpFatTime
 * A pointer to where to store the Time in DOS format
 */
OFC_CORE_LIB OFC_VOID
ofc_time_elements_to_dos_date_time(OFC_UINT16 month, OFC_UINT16 day,
                                   OFC_UINT16 year, OFC_UINT16 hour,
                                   OFC_UINT16 min, OFC_UINT16 sec,
                                   OFC_WORD *lpFatDate, OFC_WORD *lpFatTime);
/**
 * ofc_dos_date_time_to_elements
 *
 * Convert a DOS Date and Time to various elements
 *
 * \param FatDate
 * The DOS Date
 *
 * \param FatTime
 * The DOS Time
 *
 * \param month
 * Pointer to where to return the month (1-12)
 *
 * \param day
 * Pointer to where to store the day of monthe (1-31)
 *
 * \param year
 * Pointer to where to store the year (regular year i.e. 2008)
 *
 * \param hour
 * Pointer to where to store the hour (0-23)
 *
 * \param min
 * Pointer to where to store the minute (0-59)
 *
 * \param sec
 * Pointer to where to store the sec (0-59)
 */
OFC_CORE_LIB OFC_VOID
ofc_dos_date_time_to_elements(OFC_WORD FatDate, OFC_WORD FatTime,
                              OFC_UINT16 *month, OFC_UINT16 *day,
                              OFC_UINT16 *year, OFC_UINT16 *hour,
                              OFC_UINT16 *min, OFC_UINT16 *sec);
/**
 * Get Process Runtime
 *
 * \returns
 * Returns the number of us of runtime
 */
OFC_CORE_LIB OFC_MSTIME
ofc_get_runtime(OFC_VOID);

#if defined(OFC_PERF_STATS)
OFC_CORE_LIB OFC_MSTIME
ofc_perf_start(OFC_TIME_PERF_ID id);

OFC_CORE_LIB OFC_VOID
ofc_perf_stop(OFC_TIME_PERF_ID id, OFC_MSTIME stamp, OFC_LONG bytes);

OFC_CORE_LIB OFC_VOID
ofc_perf_init(OFC_VOID);

OFC_CORE_LIB OFC_VOID
ofc_perf_destroy(OFC_VOID);

OFC_CORE_LIB OFC_VOID
ofc_perf_reset(OFC_VOID);

OFC_CORE_LIB OFC_VOID
ofc_perf_dump(OFC_VOID);

#endif

#if defined(__cplusplus)
}
#endif
/** \} */
#endif

