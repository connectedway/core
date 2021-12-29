/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_ENV_H__)
#define __OFC_ENV_H__

#include "ofc/core.h"
#include "ofc/types.h"

typedef enum {
    OFC_ENV_HOME = 0,
    OFC_ENV_INSTALL,
    OFC_ENV_ROOT,
    OFC_ENV_NUM
} OFC_ENV_VALUE;

#if defined(__cplusplus)
extern "C"
{
#endif
OFC_CORE_LIB OFC_BOOL
ofc_env_get(OFC_ENV_VALUE value, OFC_TCHAR *ptr, OFC_SIZET len);

#if defined(__cplusplus)
}
#endif
#endif

