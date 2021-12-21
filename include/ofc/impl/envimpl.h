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
#if !defined(__BLUE_ENV_IMPL_H__)
#define __BLUE_ENV_IMPL_H__

#include "ofc/types.h"
#include "ofc/core.h"
#include "ofc/env.h"

/**
 * \defgroup BlueEnvImpl Environment Variable Implementation
 * \ingroup BluePort
 *
 * This facility implements the platform specific environment handling
 *
 * \{
 */

#if defined(__cplusplus)
extern "C"
{
#endif
  BLUE_BOOL
  BlueEnvGetImpl (BLUE_ENV_VALUE value, BLUE_TCHAR *ptr, BLUE_SIZET len) ;
#if defined(__cplusplus)
}
#endif

/* \} */
#endif

