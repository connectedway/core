/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_CORE_H__)
#define __OFC_CORE_H__

#include "ofc/types.h"
#include "ofc/config.h"

/**
 * \defgroup core Initialization Routines
 *
 * These APIs are used to initialize the Open Files framework.  They can
 * be called explicitly in some application startup code or, if the
 * build configuration INIT_ON_LOAD is defined, they will be called when
 * the of_core library is loaded or unloaded.
 *
 * These routines are protected and discouraged for applications to call
 * directly.  Instead, we recommend using framework routines (\ref framework).
 */

/** \{ */

/**
 * Windows DLL Function Attribute
 *
 * \protected
 * If building for windows, this function attribute will declare a routine
 * as part of a dynamic link library.  If used in the implementation of
 * the routine (__OFC_CORE_DLL__ is defined), then the function will be
 * exported.  If used in the function prototype and the prototype is 
 * referenced by an external library or application, then the function will
 * be imported.  
 *
 * Application developers will have no need to use this directly.
 *
 * On non Windows platforms, this attribute is empty.
 */
#if defined(_WIN32) && defined(WIN_DLL)
#if defined(__OFC_CORE_DLL__)
#define OFC_CORE_LIB __declspec(dllexport)
#else
#define OFC_CORE_LIB __declspec(dllimport)
#endif
#else
#define OFC_CORE_LIB
#endif

#if defined(__cplusplus)
extern "C"
{
#endif
/**
 * Load and Initialie the Open Files framework
 *
 * \protected
 * If explicit initialization of the Open Files framework is desired, 
 * use \ref ofc_framework_init instead.
 */
OFC_CORE_LIB OFC_VOID
ofc_core_load(OFC_VOID);

/**
 * Destroy the Open Files stack
 *
 * \protected
 * If explicit destruction of the Open Files framework is desired, 
 * use \ref ofc_framework_destroy instead.
 */
OFC_CORE_LIB OFC_VOID
ofc_core_unload(OFC_VOID);

#if defined(__cplusplus)
}
#endif

#endif
