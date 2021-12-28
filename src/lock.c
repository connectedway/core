/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/lock.h"
#include "ofc/impl/lockimpl.h"

OFC_CORE_LIB OFC_VOID
BlueLockDestroy (BLUE_LOCK lock) 
{
  BlueLockDestroyImpl (lock) ;
}

OFC_CORE_LIB OFC_BOOL
BlueLockTry (BLUE_LOCK lock) 
{
  return (BlueLockTryImpl (lock)) ;
}

OFC_CORE_LIB OFC_VOID
BlueLock (BLUE_LOCK pLock) 
{
  if (pLock != OFC_NULL)
    BlueLockImpl (pLock) ;
}

OFC_CORE_LIB OFC_VOID
BlueUnlock (BLUE_LOCK pLock) 
{
  if (pLock != OFC_NULL)
    BlueUnlockImpl (pLock) ;
}

OFC_CORE_LIB BLUE_LOCK
BlueLockInit (OFC_VOID)
{
  BLUE_LOCK plock;
  plock = BlueLockInitImpl () ;
  return (plock);
}

