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
#if !defined(__BLUE_CONSOLE_H__)
#define __BLUE_CONSOLE_H__

#include "ofc/core.h"
#include "ofc/types.h"

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Write a buffer to standard output
   *
   * This routine is called by the BlueCprintf function.
   *
   * \param obuf
   * Pointer to output buffer.  For compatibility with some platfom specific
   * code, this buffer must be NULL terminated.  The BlueCprintf function
   * does NULL terminate the buffer.
   *
   * \param len
   * Number of characters to output
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueWriteStdOut (BLUE_CCHAR *obuf, BLUE_SIZET len) ;

  /**
   * Writes a null terminated string
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueWriteConsole (BLUE_CCHAR *obuf) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueReadLine (BLUE_CHAR *inbuf, BLUE_SIZET len) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueReadPassword (BLUE_CHAR *inbuf, BLUE_SIZET len) ;
#if defined(__cplusplus)
}
#endif
#endif

