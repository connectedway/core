/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
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
