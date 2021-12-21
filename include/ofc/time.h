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
#if !defined(__BLUE_TIME_H__)
#define __BLUE_TIME_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/file.h"

/**
 * \defgroup BlueTime Time Management
 * \ingroup BlueInternal
 *
 * This facility provides Time conversion functions
 */

/** \{ */

#if defined(BLUE_PARAM_PERF_STATS)
/**
 * A structure for helping us measure queing delay
 */
typedef enum
  {
    BLUE_TIME_PERF_FSCIFS_READ = 0,
    BLUE_TIME_PERF_FSCIFS_WRITE = 1,
    BLUE_TIME_PERF_CLIENT_READ = 2,
    BLUE_TIME_PERF_CLIENT_WRITE = 3,
    BLUE_TIME_PERF_SERVER_READ = 4,
    BLUE_TIME_PERF_SERVER_WRITE = 5,
    BLUE_TIME_PERF_SESSION_READ = 6,
    BLUE_TIME_PERF_SESSION_WRITE = 7,
    BLUE_TIME_PERF_NUM = 8
  } BLUE_TIME_PERF_ID ;

typedef struct
{
  BLUE_TIME_PERF_ID id ;	/**< This perf id */
  BLUE_MSTIME elapsed ;		/**< The total wait time  */
  BLUE_LONG depthsum ;		/**< The cumulative queue depth  */
  BLUE_LONG depth ;		/**< The current queue depth  */
  BLUE_LONG count ;		/**< The number of I/Os */
  BLUE_LONG totalbytes ;	/**< Sum of bytes  */
  BLUE_MSTIME runtime_start ;	/**< Runtime at the last reset */
  BLUE_MSTIME runtime_end ;	/**< Runtime at the last measurement */
} BLUE_TIME_PERF_STAT ;
#endif

/**
 * Definition for DOS Day Field in DOS Day Word
 */
#define BLUE_DOS_DAY 0x001F
/**
 * Bit position for DOS Day Field
 */
#define BLUE_DOS_DAY_SHIFT 0
/**
 * Definition for DOS Month Field in DOS Day Word
 */
#define BLUE_DOS_MONTH 0x01E0
/**
 * Bit position for DOS Month Field
 */
#define BLUE_DOS_MONTH_SHIFT 5
/**
 * Definition for DOS Year Field in DOS Day Word
 */
#define BLUE_DOS_YEAR 0xFE00
/**
 * Bit Position for DOS Year Field
 */
#define BLUE_DOS_YEAR_SHIFT 9
/**
 * Relative Offset of DOS Year
 */
#define BLUE_DOS_YEAR_BASE 1980
/**
 * Defition of DOS Seconds Field in DOS Seconds Word
 */
#define BLUE_DOS_SECS 0x001F
/**
 * Bit position of DOS Seconds Field
 */
#define BLUE_DOS_SECS_SHIFT 0
/**
 * Definition of DOS Minute Field in DOS Seconds Word
 */
#define BLUE_DOS_MINS 0x07E0
/**
 * Bit Position of DOS Minute Field
 */
#define BLUE_DOS_MINS_SHIFT 5
/**
 * Definition of DOS Hours Field in DOS Seconds Word
 */
#define BLUE_DOS_HRS 0xF800
/**
 * Bit position of DOS Hours Field
 */
#define BLUE_DOS_HRS_SHIFT 11

/*
 * This brings us to 1900
 */
#define BLUE_TIME_S_EPOCH_OFFSET_1900 11644473600LL
/*
 * This brings us to 1970
 */
#define BLUE_TIME_S_EPOCH_OFFSET_1970 9435484800LL

#if defined(__cplusplus)
extern "C"
{
#endif
  BLUE_VOID EpochTimeToFileTime (const BLUE_ULONG tv_sec,
				 const BLUE_ULONG tv_nsec,
				 BLUE_FILETIME *filetime) ;
  BLUE_VOID FileTimeToEpochTime (const BLUE_FILETIME *filetime,
				 BLUE_ULONG *tv_sec,
				 BLUE_ULONG *tv_nsec) ;
  /**
   * Get a millisecond tick count
   *
   * \returns
   * A Millisecond Tick Count
   */
  BLUE_CORE_LIB BLUE_MSTIME 
  BlueTimeGetNow(BLUE_VOID) ;
  /**
   * Get Time of Day in BLUE_FILETIME format
   *
   * \param now
   * Pointer to where to store the current file time
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueTimeGetFileTime(BLUE_FILETIME *now) ;
  /**
   * Get The local time zone offset from UTC
   *
   * \returns
   * The number of minutes from UTC that this timezone is in
   */
  BLUE_CORE_LIB BLUE_UINT16 
  BlueTimeGetTimeZone(BLUE_VOID) ;
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
  BLUE_CORE_LIB BLUE_BOOL 
  BlueFileTimeToDosDateTime (const BLUE_FILETIME *lpFileTime,
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
  BLUE_CORE_LIB BLUE_BOOL 
  BlueDosDateTimeToFileTime (BLUE_WORD FatDate, BLUE_WORD FatTime,
			     BLUE_FILETIME *lpFileTime) ;
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
  BLUE_CORE_LIB BLUE_VOID 
  BlueTimeElementsToDosDateTime (BLUE_UINT16 month, BLUE_UINT16 day,
				 BLUE_UINT16 year, BLUE_UINT16 hour,
				 BLUE_UINT16 min, BLUE_UINT16 sec,
				 BLUE_WORD *lpFatDate, BLUE_WORD *lpFatTime) ;
  /**
   * BlueTimeDosDateTimeToElements
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
  BLUE_CORE_LIB BLUE_VOID 
  BlueTimeDosDateTimeToElements (BLUE_WORD FatDate, BLUE_WORD FatTime,
				 BLUE_UINT16 *month, BLUE_UINT16 *day,
				 BLUE_UINT16 *year, BLUE_UINT16 *hour,
				 BLUE_UINT16 *min, BLUE_UINT16 *sec) ;
  /**
   * Get Process Runtime
   *
   * \returns
   * Returns the number of us of runtime
   */
  BLUE_CORE_LIB BLUE_MSTIME 
  BlueTimeGetRuntime (BLUE_VOID) ;
#if defined(BLUE_PARAM_PERF_STATS)
  BLUE_CORE_LIB BLUE_MSTIME 
  BlueTimePerfStart (BLUE_TIME_PERF_ID id) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueTimePerfStop (BLUE_TIME_PERF_ID id, BLUE_MSTIME stamp, BLUE_LONG bytes) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueTimePerfInit (BLUE_VOID) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueTimePerfDestroy (BLUE_VOID);
  
  BLUE_CORE_LIB BLUE_VOID 
  BlueTimePerfReset(BLUE_VOID) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueTimePerfDump (BLUE_VOID) ;
#endif

#if defined(__cplusplus)
}
#endif
/** \} */
#endif

