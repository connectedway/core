/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_BACKTRACE_IMPL_H__)
#define __OFC_BACKTRACE_IMPL_H__

#include "ofc/types.h"

#if defined(__cplusplus)
extern "C"
{
#endif
  OFC_VOID ofc_backtrace_impl(OFC_VOID **trace, OFC_SIZET len);
#if defined(__cplusplus)
}
#endif
#endif
  
