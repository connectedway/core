/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__BLUE_LOCK_IMPL_H__)
#define __BLUE_LOCK_IMPL_H__

#include "ofc/core.h"

/**
 * \defgroup BlueLockImpl Lock Implementation
 * \ingroup BluePort
 *
 * This facility implements the platform specific lock handling
 *
 * \{
 */
#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Destroy a Lock
   *
   * \param lock
   * Pointer to the lock to destroy
   */
  OFC_VOID BlueLockDestroyImpl (BLUE_LOCK lock) ;
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
  OFC_BOOL BlueLockTryImpl (BLUE_LOCK lock) ;
  /**
   * Wait for a lock
   *
   * This function will wait for the lock to be available
   *
   * \param pLock
   * Pointer to lock to obtain
   */
  OFC_VOID BlueLockImpl (BLUE_LOCK pLock) ;
  /**
   * Release a Lock
   *
   * \param pLock
   * Pointer to lock to release
   */
  OFC_VOID BlueUnlockImpl (BLUE_LOCK pLock) ;
  /**
   * Initialize the Locking subsystem
   *
   * This routine will be called during system initialization
   */
  BLUE_LOCK BlueLockInitImpl (OFC_VOID) ;
#if defined(__cplusplus)
}
#endif

/* \} */
#endif

