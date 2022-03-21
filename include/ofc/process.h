/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_PROCESS_H__)
#define __OFC_PROCESS_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/**
 * \defgroup process Open Files Process Abstraction
 */

/** \{ */
typedef OFC_VOID (OFC_PROCESS_TRAP_HANDLER)(OFC_INT signal);

typedef enum {
    OFC_PROCESS_PRIORITY_APP = 0,
    OFC_PROCESS_PRIORITY_CLIENT,
    OFC_PROCESS_PRIORITY_SERVER,
    OFC_PROCESS_PRIORITY_SESSION,
    OFC_PROCESS_PRIORITY_NUM
} OFC_PROCESS_PRIORITY;

#if defined(__cplusplus)
extern "C"
{
#endif
OFC_CORE_LIB OFC_PROCESS_ID
ofc_process_get(OFC_VOID);

OFC_CORE_LIB OFC_VOID
ofc_process_block_signal(OFC_INT signal);

OFC_CORE_LIB OFC_VOID
ofc_process_unblock_signal(OFC_INT signal);

OFC_CORE_LIB OFC_HANDLE
ofc_process_exec(OFC_CTCHAR *name, OFC_TCHAR *uname,
                 OFC_INT arg_count, OFC_CHAR **argv);

OFC_CORE_LIB OFC_VOID
ofc_process_term(OFC_HANDLE hProcess);

OFC_CORE_LIB OFC_BOOL
ofc_process_term_trap(OFC_PROCESS_TRAP_HANDLER trap);

OFC_CORE_LIB OFC_VOID
ofc_process_set_priority(OFC_PROCESS_PRIORITY prio);
/**
 * Kill a process.
 *
 * Essentially the same as ofc_process_term, but accepts a process id rather
 * than a handle.  This is good for interprocess control
 *
 * \param id
 * Process id of process to kill
 */
OFC_CORE_LIB OFC_VOID
ofc_process_kill(OFC_PROCESS_ID id);
/**
 * Get the process id of a process given it's process handle
 *
 * \param hProcess
 * Process Handle
 *
 * \returns
 * Process Id
 */
OFC_CORE_LIB OFC_PROCESS_ID
ofc_process_get_id(OFC_HANDLE hProcess);

OFC_CORE_LIB OFC_VOID
ofc_process_crash(OFC_CCHAR *obuf);

OFC_CORE_LIB OFC_VOID
ofc_process_dump_libs(OFC_VOID);

OFC_CORE_LIB OFC_VOID *
ofc_process_relative_addr(OFC_VOID *addr);

#if defined(__cplusplus)
}
#endif

static inline OFC_CORE_LIB OFC_VOID
ofc_assert(OFC_BOOL condition, OFC_CCHAR *error)
{
  if (!condition)
    ofc_process_crash(error);
}

/** \} */
#endif

