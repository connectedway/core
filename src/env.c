/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/env.h"
#include "ofc/impl/envimpl.h"


OFC_CORE_LIB OFC_BOOL
ofc_env_get (OFC_ENV_VALUE value, OFC_TCHAR *ptr, OFC_SIZET len)
{
  return (ofc_env_get_impl (value, ptr, len)) ;
}
