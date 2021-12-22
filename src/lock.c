/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __BLUE_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/lock.h"
#include "ofc/impl/lockimpl.h"

BLUE_CORE_LIB BLUE_VOID 
BlueLockDestroy (BLUE_LOCK lock) 
{
  BlueLockDestroyImpl (lock) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueLockTry (BLUE_LOCK lock) 
{
  return (BlueLockTryImpl (lock)) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueLock (BLUE_LOCK pLock) 
{
  if (pLock != BLUE_NULL)
    BlueLockImpl (pLock) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueUnlock (BLUE_LOCK pLock) 
{
  if (pLock != BLUE_NULL)
    BlueUnlockImpl (pLock) ;
}

BLUE_CORE_LIB BLUE_LOCK
BlueLockInit (BLUE_VOID) 
{
  BLUE_LOCK plock;
  plock = BlueLockInitImpl () ;
  return (plock);
}

