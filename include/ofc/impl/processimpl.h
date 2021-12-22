/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
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

