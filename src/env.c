/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __BLUE_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/env.h"
#include "ofc/impl/envimpl.h"


BLUE_CORE_LIB BLUE_BOOL
BlueEnvGet (BLUE_ENV_VALUE value, BLUE_TCHAR *ptr, BLUE_SIZET len) 
{
  return (BlueEnvGetImpl (value, ptr, len)) ;
}
