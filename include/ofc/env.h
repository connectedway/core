/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_ENV_H__)
#define __OFC_ENV_H__

#include "ofc/core.h"
#include "ofc/types.h"

/**
 * \defgroup env APIs to Access Environment Variables
 *
 * These APIs allow applications to read select platform environment
 * variables.
 */

/** \{ */

/**
 * Supported Environment Variables
 */

typedef enum {
    OFC_ENV_HOME = 0, /**< Location of configuration XML file */
    OFC_ENV_INSTALL,  /**< Unused */
    OFC_ENV_ROOT,     /**< Unused */
    OFC_ENV_MODE,     /**< Linux Server/Client Mode [server|client]  */
    OFC_ENV_NUM	      /**< Number of Environment Variables  */
} OFC_ENV_VALUE;

#if defined(__cplusplus)
extern "C"
{
#endif
/**
 * Get the value of an environment variable
 *
 * \param value
 * The environment variable id
 *
 * \param ptr
 * Pointer to where to store the env variable value
 *
 * \param len
 * size of the buffer for the env variable
 *
 * \returns
 * Number of bytes placed into the buffer
 */
OFC_CORE_LIB OFC_BOOL
ofc_env_get(OFC_ENV_VALUE value, OFC_TCHAR *ptr, OFC_SIZET len);

#if defined(__cplusplus)
}
#endif
/** \} */
#endif

