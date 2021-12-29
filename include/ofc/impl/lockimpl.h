/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_LOCK_IMPL_H__)
#define __OFC_LOCK_IMPL_H__

#include "ofc/core.h"

/**
 * \defgroup ofc_lock_impl Lock Implementation
 * \ingroup port
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
  OFC_VOID ofc_lock_destroy_impl (OFC_LOCK lock) ;
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
  OFC_BOOL ofc_lock_try_impl (OFC_LOCK lock) ;
  /**
   * Wait for a lock
   *
   * This function will wait for the lock to be available
   *
   * \param pLock
   * Pointer to lock to obtain
   */
  OFC_VOID ofc_lock_impl (OFC_LOCK pLock) ;
  /**
   * Release a Lock
   *
   * \param pLock
   * Pointer to lock to release
   */
  OFC_VOID ofc_unlock_impl (OFC_LOCK pLock) ;
  /**
   * Initialize the Locking subsystem
   *
   * This routine will be called during system initialization
   */
  OFC_LOCK ofc_lock_init_impl (OFC_VOID) ;
#if defined(__cplusplus)
}
#endif

/* \} */
#endif

