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
  BLUE_VOID 
  BlueWriteStdOutImpl (BLUE_CCHAR *obuf, BLUE_SIZET len) ;
  BLUE_CORE_LIB BLUE_VOID 
  BlueWriteConsoleImpl (BLUE_CCHAR *obuf) ;
  BLUE_VOID 
  BlueReadStdInImpl (BLUE_CHAR *inbuf, BLUE_SIZET len) ;
  BLUE_VOID 
  BlueReadPasswordImpl (BLUE_CHAR *inbuf, BLUE_SIZET len) ;
#if defined(__cplusplus)
}
#endif
/** \} */
#endif

