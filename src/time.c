/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __BLUE_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/time.h"
#include "ofc/impl/timeimpl.h"

#if defined(BLUE_PARAM_64BIT_INTEGER)

BLUE_VOID EpochTimeToFileTime (const BLUE_ULONG tv_sec,
			       const BLUE_ULONG tv_nsec,
			       BLUE_FILETIME *filetime)
{
  BLUE_UINT64 secs ;
  BLUE_UINT64 ns ;

  secs = (BLUE_UINT64) tv_sec + BLUE_TIME_S_EPOCH_OFFSET_1900 ;

  /*
   * Convert to number of 100 ns
   */
  ns = secs * 1000 * 1000 * 10 + (tv_nsec / 100) ;
  /*
   * Name into 100ns chunks
   */
  filetime->dwHighDateTime = (BLUE_DWORD) (ns >> 32) ;
  filetime->dwLowDateTime = (BLUE_DWORD) (ns & 0xFFFFFFFF) ;
}

BLUE_VOID FileTimeToEpochTime (const BLUE_FILETIME *filetime,
			       BLUE_ULONG *tv_sec,
			       BLUE_ULONG *tv_nsec)
{
  BLUE_UINT64 ns ;

  /*
   * Name into 100ns chunks
   */
  ns = (BLUE_UINT64)filetime->dwHighDateTime << 32 | filetime->dwLowDateTime ;
  /*
   * Convert to number of secs
   */
  *tv_sec = (BLUE_ULONG) ((ns / (1000 * 1000 * 10)) - 
	                  BLUE_TIME_S_EPOCH_OFFSET_1900) ;
  *tv_nsec = (ns % (1000 * 1000 * 10)) * 100 ;
}
#else
#include "BlueUtil/BlueInt64.h"

#if 1
/*
 * This brings us to 1900
 */
static BLUE_UINT64 Blue64EpochOffset =
  {
    0xB6109100,
    0x00000002
  } ;
#else
/*
 * This brings us to 1970
 */
static BLUE_UINT64 Blue64EpochOffset =
  {
    0x32661280
    0x00000002
  } ;
#endif

BLUE_VOID EpochTimeToFileTime (const BLUE_ULONG tv_sec,
			       const BLUE_ULONG tv_nsec,
			       BLUE_FILETIME *filetime)
{
  BLUE_UINT64 ns ;
  BLUE_UINT64 temp ;

  Blue64UAssign32 (&ns, tv_sec) ;
  Blue64UAdd (&ns, &Blue64EpochOffset) ;
  /*
   * Convert to number of 100 ns
   */
  Blue64UMult32 (&ns, 1000 * 1000 * 10) ;

  Blue64UAssign32 (&temp, tv_nsec / 100) ;
  Blue64UAdd (&ns, &temp) ;
  /*
   * Name into 100ns chunks
   */
  filetime->dwHighDateTime = Blue64URetrieve32h (&ns) ;
  filetime->dwLowDateTime = Blue64URetrieve32 (&ns) ;
}

BLUE_VOID FileTimeToEpochTime (const BLUE_FILETIME *filetime,
			       BLUE_ULONG *tv_sec,
			       BLUE_ULONG *tv_nsec)
{
  BLUE_UINT64 secs ;
  BLUE_UINT64 ns ;
  BLUE_UINT64 temp ;
  BLUE_UINT64 quot ;
  BLUE_UINT64 rem ;

  /*
   * Name into 100ns chunks
   */
  Blue64UAssign64 (&ns,
		  filetime->dwLowDateTime, filetime->dwHighDateTime) ;
  /*
   * Convert to number of secs
   */
  Blue64UAssign (&secs, &ns) ;
  Blue64UAssign32 (&temp, 1000 * 1000 * 10) ;
  Blue64UDivide (&secs, &temp, &quot, &rem) ;
  Blue64USub (&quot, &Blue64EpochOffset) ;
  *tv_sec = Blue64URetrieve32 (&quot) ;
  *tv_nsec = Blue64URetrieve32 (&rem) * 100 ;
}
#endif

BLUE_CORE_LIB BLUE_MSTIME 
BlueTimeGetNow(BLUE_VOID) 
{
  return (BlueTimeGetNowImpl ()) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueTimeGetFileTime(BLUE_FILETIME *filetime)
{
  BlueTimeGetFileTimeImpl (filetime) ;
}

BLUE_CORE_LIB BLUE_UINT16 
BlueTimeGetTimeZone (BLUE_VOID)
{
  return (BlueTimeGetTimeZoneImpl ()) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueFileTimeToDosDateTime (const BLUE_FILETIME *lpFileTime,
			   BLUE_WORD *lpFatDate, BLUE_WORD *lpFatTime)
{
  return (BlueFileTimeToDosDateTimeImpl (lpFileTime, lpFatDate, lpFatTime)) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueDosDateTimeToFileTime (BLUE_WORD FatDate, BLUE_WORD FatTime,
			   BLUE_FILETIME *lpFileTime) 
{
  return (BlueDosDateTimeToFileTimeImpl (FatDate, FatTime, lpFileTime)) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueTimeElementsToDosDateTime (BLUE_UINT16 month,
			       BLUE_UINT16 day,
			       BLUE_UINT16 year,
			       BLUE_UINT16 hour,
			       BLUE_UINT16 min,
			       BLUE_UINT16 sec,
			       BLUE_WORD *lpFatDate,
			       BLUE_WORD *lpFatTime)
{					 
  *lpFatDate = 
    month << BLUE_DOS_MONTH_SHIFT |
    day << BLUE_DOS_DAY_SHIFT |
    (year - BLUE_DOS_YEAR_BASE) << BLUE_DOS_YEAR_SHIFT ;
  *lpFatTime =
    hour << BLUE_DOS_HRS_SHIFT |
    min << BLUE_DOS_MINS_SHIFT |
    sec << BLUE_DOS_SECS_SHIFT ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueTimeDosDateTimeToElements (BLUE_WORD FatDate,
			       BLUE_WORD FatTime,
			       BLUE_UINT16 *month,
			       BLUE_UINT16 *day,
			       BLUE_UINT16 *year,
			       BLUE_UINT16 *hour,
			       BLUE_UINT16 *min,
			       BLUE_UINT16 *sec) 
{
  *hour = (FatTime & BLUE_DOS_HRS) >> BLUE_DOS_HRS_SHIFT ;
  *min = (FatTime & BLUE_DOS_MINS) >> BLUE_DOS_MINS_SHIFT ;
  *sec = (FatTime & BLUE_DOS_SECS) >> BLUE_DOS_SECS_SHIFT ;

  *month = ((FatDate & BLUE_DOS_MONTH) >> BLUE_DOS_MONTH_SHIFT) ;
  *day = (FatDate & BLUE_DOS_DAY) >> BLUE_DOS_DAY_SHIFT ;
  *year = ((FatDate & BLUE_DOS_YEAR) >> BLUE_DOS_YEAR_SHIFT) + 
    BLUE_DOS_YEAR_BASE ; 
}

#if defined(BLUE_PARAM_PERF_STATS)

static BLUE_CHAR *BlueTimePerfNames[BLUE_TIME_PERF_NUM] =
  {
    "App Read",
    "App Write",
    "Client Read",
    "Client Write",
    "Server Read",
    "Server Write",
    "Session Read",
    "Session Write"
} ;


BLUE_CORE_LIB BLUE_MSTIME 
BlueTimeGetRuntime (BLUE_VOID)
{
  return (BlueTimeGetRuntimeImpl()) ;
}

BLUE_CORE_LIB BLUE_MSTIME 
BlueTimePerfStart (BLUE_TIME_PERF_ID id)
{
  BLUE_MSTIME stamp ;
  BLUE_TIME_PERF_STAT *BlueTimePerfStats ;

  BlueTimePerfStats = BlueGetPerfStats() ;
  stamp = BlueTimeGetNow() ;

  if (BlueTimePerfStats != BLUE_NULL)
    BlueTimePerfStats[id].depth++ ;

  if (BlueTimePerfStats[id].runtime_start == 0)
    BlueTimePerfStats[id].runtime_start = BlueTimeGetRuntime() ;
  return (stamp) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueTimePerfStop (BLUE_TIME_PERF_ID id, BLUE_MSTIME stamp,
		  BLUE_LONG bytes_transferred)
{
  BLUE_TIME_PERF_STAT *BlueTimePerfStats ;

  BlueTimePerfStats = BlueGetPerfStats() ;
  if (BlueTimePerfStats != BLUE_NULL)
    {
      BlueTimePerfStats[id].elapsed += BlueTimeGetNow() - stamp ;
      BlueTimePerfStats[id].depthsum += BlueTimePerfStats[id].depth ;
      BlueTimePerfStats[id].totalbytes += bytes_transferred ;
      BlueTimePerfStats[id].count++ ;
      BlueTimePerfStats[id].depth-- ;
      BlueTimePerfStats[id].runtime_end = BlueTimeGetRuntime() ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueTimePerfReset(BLUE_VOID)
{
  BLUE_INT id ;
  BLUE_TIME_PERF_STAT *BlueTimePerfStats ;

  BlueTimePerfStats = BlueGetPerfStats() ;
  if (BlueTimePerfStats != BLUE_NULL)
    {
      for (id = 0 ; id < BLUE_TIME_PERF_NUM ; id++)
	{
	  BlueTimePerfStats[id].id = id ;
	  BlueTimePerfStats[id].elapsed = 0 ;
	  BlueTimePerfStats[id].totalbytes = 0 ;
	  BlueTimePerfStats[id].count = 0 ;
	  BlueTimePerfStats[id].depth = 0 ;
	  BlueTimePerfStats[id].depthsum = 0 ;
	  BlueTimePerfStats[id].runtime_start = 0 ;
	  BlueTimePerfStats[id].runtime_end = 0 ;
	}
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueTimePerfInit (BLUE_VOID)
{
  BLUE_TIME_PERF_STAT *BlueTimePerfStats ;

  BlueTimePerfStats = BlueGetPerfStats() ;
  if (BlueTimePerfStats == BLUE_NULL)
    {
      BlueTimePerfStats = BlueHeapMalloc (sizeof (BLUE_TIME_PERF_STAT) * 
					  BLUE_TIME_PERF_NUM) ;

      BlueSetPerfStats(BlueTimePerfStats) ;

      BlueTimePerfReset () ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueTimePerfDestroy (BLUE_VOID)
{
  BLUE_TIME_PERF_STAT *BlueTimePerfStats ;

  BlueTimePerfStats = BlueGetPerfStats() ;
  if (BlueTimePerfStats != BLUE_NULL)
    {
      BlueSetPerfStats(BLUE_NULL) ;
      BlueHeapFree(BlueTimePerfStats);
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueTimePerfDump (BLUE_VOID)
{
  BLUE_INT i ;
  BLUE_MSTIME uspio ;
  BLUE_MSTIME bpsec ;
  BLUE_MSTIME uspq ;
  BLUE_MSTIME count ;
  BLUE_MSTIME depth ;
  BLUE_MSTIME runtime ;
  BLUE_LONG bpio ;

  BLUE_TIME_PERF_STAT *BlueTimePerfStats ;

  BlueTimePerfStats = BlueGetPerfStats() ;
  if (BlueTimePerfStats != BLUE_NULL)
    {
      BlueCprintf ("ID      Name      us/io  byte/sec  us/q   count  b/io  "
		   "depth runtime(us) \n") ;

      for (i = 0 ; i < BLUE_TIME_PERF_NUM ; i++)
	{
	  count = BlueTimePerfStats[i].count ;
	  if (count > 0)
	    {
	      depth = BlueTimePerfStats[i].depthsum / count ;
	      uspq = (BlueTimePerfStats[i].elapsed * 1000) / count ;
	      if (BlueTimePerfStats[i].depthsum == 0)
		uspio = 0 ;
	      else
		uspio = (BlueTimePerfStats[i].elapsed * 1000 ) / 
		  BlueTimePerfStats[i].depthsum ;
	      bpio = BlueTimePerfStats[i].totalbytes / count ;
	      if (uspio == 0)
		bpsec = 0 ;
	      else
		bpsec = (bpio * 1000 * 1000) / uspio ;
	      runtime = BlueTimePerfStats[i].runtime_end -
		BlueTimePerfStats[i].runtime_start ;
	      BlueCprintf ("%2d %-14s %6d %8d %6d %6d %5d %5d %7d\n",
			   i, BlueTimePerfNames[i], 
			   uspio, bpsec, uspq, count, bpio, depth, runtime) ;
	    }
	}
    }
}

#endif
