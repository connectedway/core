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
