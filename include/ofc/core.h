/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_CORE_H__)
#define __OFC_CORE_H__

#include "ofc/types.h"
#include "ofc/config.h"

#undef WIN_DLL
#if defined(_WIN32) && defined(WIN_DLL)
#if defined(__OFC_CORE_DLL__)
#define OFC_CORE_LIB __declspec(dllexport)
#else
#define OFC_CORE_LIB __declspec(dllimport)
#endif
#else
#define OFC_CORE_LIB
#endif

#if defined(__cplusplus)
extern "C"
{
#endif
  OFC_CORE_LIB OFC_VOID
    ofc_core_load (OFC_VOID) ;

  OFC_CORE_LIB OFC_VOID
    ofc_core_unload (OFC_VOID) ;
#if defined(__cplusplus)
}
#endif

#endif
