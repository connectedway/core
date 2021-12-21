/* Copyright (c) 2009 Blue Peach Solutions, Inc.
 * All rights reserved.
 *
 * This software is protected by copyright and intellectual 
 * property laws as well as international treaties.  It is to be 
 * used and copied only by authorized licensees under the 
 * conditions described in their licenses.  
 *
 * Title to and ownership of the software shall at all times 
 * remain with Blue Peach Solutions.
 */
#if !defined(__BLUE_PROCESS_H__)
#define __BLUE_PROCESS_H__

typedef BLUE_VOID (BLUE_PROCESS_TRAP_HANDLER) (BLUE_INT signal) ;

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
  BLUE_CORE_LIB BLUE_PROCESS_ID 
  BlueProcessGet (BLUE_VOID) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueProcessBlockSignal (BLUE_INT signal) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueProcessUnblockSignal (BLUE_INT signal) ;

  BLUE_CORE_LIB BLUE_HANDLE 
  BlueProcessExec (BLUE_CTCHAR *name, BLUE_TCHAR *uname,
		   BLUE_INT arg_count, BLUE_CHAR **argv) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueProcessTerm (BLUE_HANDLE hProcess) ;

  BLUE_CORE_LIB BLUE_BOOL 
  BlueProcessTermTrap (BLUE_PROCESS_TRAP_HANDLER trap) ;

  BLUE_CORE_LIB BLUE_VOID 
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
  BLUE_CORE_LIB BLUE_VOID 
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
  BLUE_CORE_LIB BLUE_PROCESS_ID 
  BlueProcessGetId (BLUE_HANDLE hProcess) ;
  BLUE_CORE_LIB BLUE_VOID
  BlueProcessCrash (BLUE_CCHAR *obuf) ;

#if defined(__cplusplus)
}
#endif
#endif

