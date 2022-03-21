/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/process.h"
#include "ofc/impl/processimpl.h"

OFC_CORE_LIB OFC_PROCESS_ID
ofc_process_get(OFC_VOID) {
    OFC_PROCESS_ID pid;

    pid = ofc_process_get_impl();
    return (pid);
}

OFC_CORE_LIB OFC_HANDLE
ofc_process_exec(OFC_CTCHAR *name, OFC_TCHAR *uname,
                 OFC_INT argc, OFC_CHAR **argv) {
    OFC_HANDLE hProcess;

    hProcess = ofc_process_exec_impl(name, uname, argc, argv);
    return (hProcess);
}


OFC_CORE_LIB OFC_PROCESS_ID
ofc_process_get_id(OFC_HANDLE hProcess) {
    OFC_PROCESS_ID ret;

    ret = ofc_process_get_id_impl(hProcess);
    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_process_term(OFC_HANDLE hProcess) {
    ofc_process_term_impl(hProcess);
}

OFC_CORE_LIB OFC_VOID
ofc_process_kill(OFC_PROCESS_ID id) {
    ofc_process_kill_impl(id);
}

OFC_CORE_LIB OFC_BOOL
ofc_process_term_trap(OFC_PROCESS_TRAP_HANDLER trap) {
    OFC_BOOL ret;

    ret = ofc_process_term_trap_impl(trap);
    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_process_crash(OFC_CCHAR *obuf) {
    ofc_process_crash_impl(obuf);
}

OFC_CORE_LIB OFC_VOID
ofc_process_dump_libs(OFC_VOID)
{
  ofc_process_dump_libs_impl();
}

OFC_CORE_LIB OFC_VOID *ofc_process_relative_addr(OFC_VOID *addr)
{
  return (ofc_process_relative_addr_impl(addr));
}

