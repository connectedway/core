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

