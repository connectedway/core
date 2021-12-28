/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_CONSOLE_H__)
#define __OFC_CONSOLE_H__

#include "ofc/core.h"
#include "ofc/types.h"

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Write a buffer to standard output
   *
   * This routine is called by the ofc_printf function.
   *
   * \param obuf
   * Pointer to output buffer.  For compatibility with some platfom specific
   * code, this buffer must be NULL terminated.  The ofc_printf function
   * does NULL terminate the buffer.
   *
   * \param len
   * Number of characters to output
   */
  OFC_CORE_LIB OFC_VOID
  ofc_write_stdout (OFC_CCHAR *obuf, OFC_SIZET len) ;

  /**
   * Writes a null terminated string
   */
  OFC_CORE_LIB OFC_VOID
  ofc_write_console (OFC_CCHAR *obuf) ;

  OFC_CORE_LIB OFC_VOID
  ofc_read_line (OFC_CHAR *inbuf, OFC_SIZET len) ;

  OFC_CORE_LIB OFC_VOID
  ofc_read_password (OFC_CHAR *inbuf, OFC_SIZET len) ;
#if defined(__cplusplus)
}
#endif
#endif

