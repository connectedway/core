/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/types.h"
#include "ofc/impl/backtraceimpl.h"

OFC_VOID ofc_backtrace(OFC_VOID **trace, OFC_SIZET len)
{
  ofc_backtrace_impl(trace, len);
}

#if !defined(__ANDROID__) && (defined(__linux__)
OFC_VOID ofc_backtrace_sym(OFC_CHAR ***trace, OFC_SIZET len)
{
  return(ofc_backtrace_sym_impl(trace, len));
}

OFC_VOID ofc_backtrace_sym_free(OFC_CHAR **trace)
{
  ofc_backtrace_sym_free_impl(trace);
}
#endif
