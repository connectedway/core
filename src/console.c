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

