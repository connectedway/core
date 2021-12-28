/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/impl/consoleimpl.h"

OFC_CORE_LIB OFC_VOID
ofc_write_stdout (OFC_CCHAR *obuf, OFC_SIZET len)
{
  BlueWriteStdOutImpl (obuf, len) ;
}

OFC_CORE_LIB OFC_VOID
ofc_write_console (OFC_CCHAR *obuf)
{
  BlueWriteConsoleImpl (obuf) ;
}

OFC_CORE_LIB OFC_VOID
ofc_read_line (OFC_CHAR *inbuf, OFC_SIZET len)
{
  BlueReadStdInImpl (inbuf, len) ;
}

OFC_CORE_LIB OFC_VOID
ofc_read_password (OFC_CHAR *inbuf, OFC_SIZET len)
{
  BlueReadPasswordImpl (inbuf, len) ;
}

