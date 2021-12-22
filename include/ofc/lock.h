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
typedef BLUE_VOID *BLUE_LOCK ;

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Initialize a lock
   *
   * Pointer to lock to initialize
   */
  BLUE_CORE_LIB BLUE_LOCK
  BlueLockInit (BLUE_VOID) ;
  /**
   * Destroy a Lock
   *
   * \param lock
   * Pointer to the lock to destroy
   */
  BLUE_CORE_LIB BLUE_VOID 
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
   * BLUE_TRUE if the lock has been obtained, BLUE_FALSE if the lock was
   * not available
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueLockTry (BLUE_LOCK lock) ;
  /**
   * Wait for a lock
   *
   * This function will wait for the lock to be available
   *
   * \param pLock
   * Pointer to lock to obtain
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueLock (BLUE_LOCK pLock) ;
  /**
   * Release a Lock
   *
   * \param pLock
   * Pointer to lock to release
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueUnlock (BLUE_LOCK pLock) ;
#if defined(__cplusplus)
}
#endif
#endif

