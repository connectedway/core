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
  BLUE_PROCESS_ID BlueProcessGetImpl (OFC_VOID) ;
  OFC_HANDLE BlueProcessExecImpl (OFC_CTCHAR *name,
                                  OFC_TCHAR *uname,
                                  OFC_INT argc,
                                  OFC_CHAR **argv) ;
  OFC_VOID BlueProcessTermImpl (OFC_HANDLE hProcess) ;
  OFC_BOOL BlueProcessTermTrapImpl (BLUE_PROCESS_TRAP_HANDLER trap) ;
  OFC_VOID BlueProcessKillImpl (BLUE_PROCESS_ID id) ;
  BLUE_PROCESS_ID BlueProcessGetIdImpl (OFC_HANDLE hProcess) ;
  OFC_VOID BlueProcessCrashImpl (OFC_CCHAR *obuf) ;
#if defined(__cplusplus)
}
#endif

#endif

