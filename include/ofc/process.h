/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__BLUE_PROCESS_H__)
#define __BLUE_PROCESS_H__

typedef OFC_VOID (BLUE_PROCESS_TRAP_HANDLER) (OFC_INT signal) ;

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

typedef enum
  {
    BLUE_PROCESS_PRIORITY_APP = 0,
    BLUE_PROCESS_PRIORITY_CLIENT,
    BLUE_PROCESS_PRIORITY_SERVER,
    BLUE_PROCESS_PRIORITY_SESSION,
    BLUE_PROCESS_PRIORITY_NUM
  } BLUE_PROCESS_PRIORITY ;

#if defined(__cplusplus)
extern "C"
{
#endif
  OFC_CORE_LIB BLUE_PROCESS_ID
  BlueProcessGet (OFC_VOID) ;

  OFC_CORE_LIB OFC_VOID
  BlueProcessBlockSignal (OFC_INT signal) ;

  OFC_CORE_LIB OFC_VOID
  BlueProcessUnblockSignal (OFC_INT signal) ;

  OFC_CORE_LIB BLUE_HANDLE
  BlueProcessExec (OFC_CTCHAR *name, OFC_TCHAR *uname,
                   OFC_INT arg_count, OFC_CHAR **argv) ;

  OFC_CORE_LIB OFC_VOID
  BlueProcessTerm (BLUE_HANDLE hProcess) ;

  OFC_CORE_LIB OFC_BOOL
  BlueProcessTermTrap (BLUE_PROCESS_TRAP_HANDLER trap) ;

  OFC_CORE_LIB OFC_VOID
  BlueProcessSetPriority (BLUE_PROCESS_PRIORITY prio) ;
  /**
   * Kill a process.
   *
   * Essentially the same as BlueProcessTerm, but accepts a process id rather
   * than a handle.  This is good for interprocess control
   * 
   * \param id
   * Process id of process to kill
   */
  OFC_CORE_LIB OFC_VOID
  BlueProcessKill (BLUE_PROCESS_ID id) ;
  /**
   * Get the process id of a process given it's process handle
   *
   * \param hProcess
   * Process Handle
   *
   * \returns
   * Process Id
   */
  OFC_CORE_LIB BLUE_PROCESS_ID
  BlueProcessGetId (BLUE_HANDLE hProcess) ;
  OFC_CORE_LIB OFC_VOID
  BlueProcessCrash (OFC_CCHAR *obuf) ;

#if defined(__cplusplus)
}
#endif
#endif

