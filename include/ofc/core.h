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
#if !defined(__BLUE_CORE_H__)
#define __BLUE_CORE_H__

#include "ofc/types.h"
#include "ofc/config.h"

#undef WIN_DLL
#if defined(_WIN32) && defined(WIN_DLL)
#if defined(__BLUE_CORE_DLL__)
#define BLUE_CORE_LIB __declspec(dllexport)
#else
#define BLUE_CORE_LIB __declspec(dllimport)
#endif
#else
#define BLUE_CORE_LIB
#endif

#if defined(__cplusplus)
extern "C"
{
#endif
  BLUE_CORE_LIB BLUE_VOID 
    BlueUtilLoad (BLUE_VOID) ;

  BLUE_CORE_LIB BLUE_VOID 
    BlueUtilUnload (BLUE_VOID) ;
#if defined(__cplusplus)
}
#endif

#endif
