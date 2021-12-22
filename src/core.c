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
#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/libc.h"
#include "ofc/path.h"
#include "ofc/net.h"
#include "ofc/time.h"
#include "ofc/thread.h"
#if defined(BLUE_PARAM_MESSAGE_DEBUG)
#include "ofc/message.h"
#endif

#include "ofc/heap.h"
#include "ofc/persist.h"
/**
 * \defgroup BlueInit Initialization
 * \ingroup Applications
 */

/** \{ */

/*
 * BlueHeap must be loaded first
 */
BLUE_LOAD BLUE_CORE_LIB BLUE_VOID 
BlueUtilLoad (BLUE_VOID)
{
  BlueHeapLoad() ;
  BlueHandle16Init() ;
  BlueThreadInit() ;
  BlueTraceInit() ;
  BluePathInit() ;
  BlueNetInit() ;
  BlueFSInit() ;
  BlueFileInit() ;
  BlueConfigInit() ;
#if defined(BLUE_PARAM_PERF_STATS)
  BlueTimePerfInit() ;
#endif
}

BLUE_UNLOAD BLUE_CORE_LIB BLUE_VOID 
BlueUtilUnload (BLUE_VOID)
{
#if defined(BLUE_PARAM_PERF_STATS)
  BlueTimePerfDestroy() ;
#endif

  BlueConfigUnload() ;
  BlueFileDestroy();

  BlueFSDestroy();

  BlueNetDestroy() ;

  BluePathDestroy() ;

  BlueTraceDestroy() ;

  BlueThreadDestroy() ;

  BlueHandle16Free() ;

  BlueHeapUnload() ;
}

/** \} */
