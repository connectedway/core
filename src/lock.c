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
  BlueLockImpl (pLock) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueUnlock (BLUE_LOCK pLock) 
{
  BlueUnlockImpl (pLock) ;
}

BLUE_CORE_LIB BLUE_LOCK
BlueLockInit (BLUE_VOID) 
{
  BLUE_LOCK plock;
  plock = BlueLockInitImpl () ;
  return (plock);
}

