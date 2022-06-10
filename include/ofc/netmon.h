/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_NETMON_H__)
#define __OFC_NETMON_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/** 
 * \defgroup netmon Network Monitor
 *
 */

/** \{ */

#if defined(__cplusplus)
extern "C"
{
#endif
/**
 * Initialize the Network Facility
 *
 * This should only be called by the net_init routine
 */
OFC_CORE_LIB OFC_VOID
ofc_netmon_startup (OFC_HANDLE hScheduler, OFC_HANDLE hNotify);
#if defined(__cplusplus)
}
#endif
/** \} */
#endif

