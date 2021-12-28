/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__BLUE_CONSOLE_IMPL_H__)
#define __BLUE_CONSOLE_IMPL_H__

#include "ofc/types.h"
#include "ofc/core.h"

/**
 * \defgroup BlueConsoleImpl Console I/O Implementation
 * \ingroup BluePort
 *
 * This facility implements the platform specific console I/O routines
 */

/** \{ */

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Output a buffer to the console
   *
   * This routine should implement a write to the console.  Whether the
   * console is STDOUT or a physical console or a log file is platform
   * dependent
   *
   * \param obuf
   * Pointer to output buffer
   *
   * \param len
   * Number of characters to output
   */
  OFC_VOID
  BlueWriteStdOutImpl (OFC_CCHAR *obuf, OFC_SIZET len) ;
  OFC_CORE_LIB OFC_VOID
  BlueWriteConsoleImpl (OFC_CCHAR *obuf) ;
  OFC_VOID
  BlueReadStdInImpl (OFC_CHAR *inbuf, OFC_SIZET len) ;
  OFC_VOID
  BlueReadPasswordImpl (OFC_CHAR *inbuf, OFC_SIZET len) ;
#if defined(__cplusplus)
}
#endif
/** \} */
#endif

