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

OFC_CORE_LIB BLUE_PROCESS_ID
BlueProcessGet (OFC_VOID)
{
  BLUE_PROCESS_ID pid ;

  pid = BlueProcessGetImpl () ;
  return (pid) ;
}

OFC_CORE_LIB OFC_HANDLE
BlueProcessExec (OFC_CTCHAR *name, OFC_TCHAR *uname,
                 OFC_INT argc, OFC_CHAR **argv)
{
  OFC_HANDLE hProcess ;

  hProcess = BlueProcessExecImpl (name, uname, argc, argv) ;
  return (hProcess) ;
}


OFC_CORE_LIB BLUE_PROCESS_ID
BlueProcessGetId (OFC_HANDLE hProcess)
{
  BLUE_PROCESS_ID ret ;

  ret = BlueProcessGetIdImpl (hProcess) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueProcessTerm (OFC_HANDLE hProcess)
{
  BlueProcessTermImpl (hProcess) ;
}

OFC_CORE_LIB OFC_VOID
BlueProcessKill (BLUE_PROCESS_ID id)
{
  BlueProcessKillImpl (id) ;
}

OFC_CORE_LIB OFC_BOOL
BlueProcessTermTrap (BLUE_PROCESS_TRAP_HANDLER trap)
{
  OFC_BOOL ret ;

  ret = BlueProcessTermTrapImpl (trap) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueProcessCrash (OFC_CCHAR *obuf)
{
  BlueProcessCrashImpl (obuf) ;
}
