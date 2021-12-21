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
#if !defined(__BLUE_PROCESS_IMPL_H__)
#define __BLUE_PROCESS_IMPL_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

#if defined(__cplusplus)
extern "C"
{
#endif
  BLUE_PROCESS_ID BlueProcessGetImpl (BLUE_VOID) ;
  BLUE_HANDLE BlueProcessExecImpl (BLUE_CTCHAR *name,
				   BLUE_TCHAR *uname,
				   BLUE_INT argc,
				   BLUE_CHAR **argv) ;
  BLUE_VOID BlueProcessTermImpl (BLUE_HANDLE hProcess) ;
  BLUE_BOOL BlueProcessTermTrapImpl (BLUE_PROCESS_TRAP_HANDLER trap) ;
  BLUE_VOID BlueProcessKillImpl (BLUE_PROCESS_ID id) ;
  BLUE_PROCESS_ID BlueProcessGetIdImpl (BLUE_HANDLE hProcess) ;
  BLUE_VOID BlueProcessCrashImpl (BLUE_CCHAR *obuf) ;
#if defined(__cplusplus)
}
#endif

#endif

