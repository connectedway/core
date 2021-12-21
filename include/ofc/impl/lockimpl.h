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
  BLUE_VOID BlueLockDestroyImpl (BLUE_LOCK lock) ;
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
  BLUE_BOOL BlueLockTryImpl (BLUE_LOCK lock) ;
  /**
   * Wait for a lock
   *
   * This function will wait for the lock to be available
   *
   * \param pLock
   * Pointer to lock to obtain
   */
  BLUE_VOID BlueLockImpl (BLUE_LOCK pLock) ;
  /**
   * Release a Lock
   *
   * \param pLock
   * Pointer to lock to release
   */
  BLUE_VOID BlueUnlockImpl (BLUE_LOCK pLock) ;
  /**
   * Initialize the Locking subsystem
   *
   * This routine will be called during system initialization
   */
  BLUE_LOCK BlueLockInitImpl (BLUE_VOID) ;
#if defined(__cplusplus)
}
#endif

/* \} */
#endif

