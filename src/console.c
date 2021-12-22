/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __BLUE_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/impl/consoleimpl.h"

BLUE_CORE_LIB BLUE_VOID 
BlueWriteStdOut (BLUE_CCHAR *obuf, BLUE_SIZET len)
{
  BlueWriteStdOutImpl (obuf, len) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueWriteConsole (BLUE_CCHAR *obuf)
{
  BlueWriteConsoleImpl (obuf) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueReadLine (BLUE_CHAR *inbuf, BLUE_SIZET len)
{
  BlueReadStdInImpl (inbuf, len) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueReadPassword (BLUE_CHAR *inbuf, BLUE_SIZET len)
{
  BlueReadPasswordImpl (inbuf, len) ;
}

