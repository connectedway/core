/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
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

