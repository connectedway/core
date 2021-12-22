/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
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

