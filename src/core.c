/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/libc.h"
#include "ofc/path.h"
#include "ofc/net.h"
#include "ofc/time.h"
#include "ofc/thread.h"
#if defined(OFC_MESSAGE_DEBUG)
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
OFC_LOAD OFC_CORE_LIB OFC_VOID
ofc_core_load (OFC_VOID)
{
  ofc_heap_load() ;
  ofc_handle16_init() ;
  BlueThreadInit() ;
  ofc_trace_init() ;
  ofc_path_init() ;
  ofc_net_init() ;
  ofc_fs_init() ;
  OfcFileInit() ;
  BlueConfigInit() ;
#if defined(OFC_PERF_STATS)
  BlueTimePerfInit() ;
#endif
}

OFC_UNLOAD OFC_CORE_LIB OFC_VOID
ofc_core_unload (OFC_VOID)
{
#if defined(OFC_PERF_STATS)
  BlueTimePerfDestroy() ;
#endif

  BlueConfigUnload() ;
  OfcFileDestroy();

  ofc_fs_destroy();

  ofc_net_destroy() ;

  ofc_path_destroy() ;

  ofc_trace_destroy() ;

  BlueThreadDestroy() ;

  ofc_handle16_free() ;

  ofc_heap_unload() ;
}

/** \} */
