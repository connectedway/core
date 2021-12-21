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
#if !defined(__BLUE_ENV_H__)
#define __BLUE_ENV_H__

#include "ofc/core.h"
#include "ofc/types.h"

typedef enum 
{
  BLUE_ENV_HOME = 0,
  BLUE_ENV_INSTALL,
  BLUE_ENV_ROOT,
  BLUE_ENV_NUM
} BLUE_ENV_VALUE ;

#if defined(__cplusplus)
extern "C"
{
#endif
  BLUE_CORE_LIB BLUE_BOOL
  BlueEnvGet (BLUE_ENV_VALUE value, BLUE_TCHAR *ptr, BLUE_SIZET len) ;
#if defined(__cplusplus)
}
#endif
#endif

