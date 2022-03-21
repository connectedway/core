/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_PROCESS_IMPL_H__)
#define __OFC_PROCESS_IMPL_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

#if defined(__cplusplus)
extern "C"
{
#endif

OFC_PROCESS_ID ofc_process_get_impl(OFC_VOID);

OFC_HANDLE ofc_process_exec_impl(OFC_CTCHAR *name,
                                 OFC_TCHAR *uname,
                                 OFC_INT argc,
                                 OFC_CHAR **argv);

OFC_VOID ofc_process_term_impl(OFC_HANDLE hProcess);

OFC_BOOL ofc_process_term_trap_impl(OFC_PROCESS_TRAP_HANDLER trap);

OFC_VOID ofc_process_kill_impl(OFC_PROCESS_ID id);

OFC_PROCESS_ID ofc_process_get_id_impl(OFC_HANDLE hProcess);

OFC_VOID ofc_process_crash_impl(OFC_CCHAR *obuf);

OFC_VOID ofc_process_dump_libs_impl(OFC_VOID);

OFC_VOID *ofc_process_relative_addr_impl(OFC_VOID *addr);

#if defined(__cplusplus)
}
#endif

#endif

