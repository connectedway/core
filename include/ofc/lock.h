/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__BLUE_LOCK_H__)
#define __BLUE_LOCK_H__

#include "ofc/core.h"
#include "ofc/types.h"

/**
 * The Platform Abstracted Lock Structure
 */
typedef OFC_VOID *BLUE_LOCK ;

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Initialize a lock
   *
   * Pointer to lock to initialize
   */
  OFC_CORE_LIB BLUE_LOCK
  BlueLockInit (OFC_VOID) ;
  /**
   * Destroy a Lock
   *
   * \param lock
   * Pointer to the lock to destroy
   */
  OFC_CORE_LIB OFC_VOID
  BlueLockDestroy (BLUE_LOCK lock) ;
  /**
   * Test if a lock is available and lock it if it is.
   *
   * Returns immediately whether the lock has been obtained or not.
   *
   * \param lock
   * Pointer to the lock to try
   *
   * \returns
   * OFC_TRUE if the lock has been obtained, OFC_FALSE if the lock was
   * not available
   */
  OFC_CORE_LIB OFC_BOOL
  BlueLockTry (BLUE_LOCK lock) ;
  /**
   * Wait for a lock
   *
   * This function will wait for the lock to be available
   *
   * \param pLock
   * Pointer to lock to obtain
   */
  OFC_CORE_LIB OFC_VOID
  BlueLock (BLUE_LOCK pLock) ;
  /**
   * Release a Lock
   *
   * \param pLock
   * Pointer to lock to release
   */
  OFC_CORE_LIB OFC_VOID
  BlueUnlock (BLUE_LOCK pLock) ;
#if defined(__cplusplus)
}
#endif
#endif

