/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __BLUE_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/process.h"
#include "ofc/impl/processimpl.h"

BLUE_CORE_LIB BLUE_PROCESS_ID 
BlueProcessGet (BLUE_VOID)
{
  BLUE_PROCESS_ID pid ;

  pid = BlueProcessGetImpl () ;
  return (pid) ;
}

BLUE_CORE_LIB BLUE_HANDLE 
BlueProcessExec (BLUE_CTCHAR *name, BLUE_TCHAR *uname,
		 BLUE_INT argc, BLUE_CHAR **argv) 
{
  BLUE_HANDLE hProcess ;

  hProcess = BlueProcessExecImpl (name, uname, argc, argv) ;
  return (hProcess) ;
}


BLUE_CORE_LIB BLUE_PROCESS_ID 
BlueProcessGetId (BLUE_HANDLE hProcess)
{
  BLUE_PROCESS_ID ret ;

  ret = BlueProcessGetIdImpl (hProcess) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueProcessTerm (BLUE_HANDLE hProcess) 
{
  BlueProcessTermImpl (hProcess) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueProcessKill (BLUE_PROCESS_ID id)
{
  BlueProcessKillImpl (id) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueProcessTermTrap (BLUE_PROCESS_TRAP_HANDLER trap)
{
  BLUE_BOOL ret ;

  ret = BlueProcessTermTrapImpl (trap) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID
BlueProcessCrash (BLUE_CCHAR *obuf)
{
  BlueProcessCrashImpl (obuf) ;
}
