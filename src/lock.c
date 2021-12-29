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
ofc_lock_destroy (OFC_LOCK lock)
{
  ofc_lock_destroy_impl (lock) ;
}

OFC_CORE_LIB OFC_BOOL
ofc_lock_try (OFC_LOCK lock)
{
  return (ofc_lock_try_impl (lock)) ;
}

OFC_CORE_LIB OFC_VOID
ofc_lock (OFC_LOCK pLock)
{
  if (pLock != OFC_NULL)
    ofc_lock_impl (pLock) ;
}

OFC_CORE_LIB OFC_VOID
ofc_unlock (OFC_LOCK pLock)
{
  if (pLock != OFC_NULL)
    ofc_unlock_impl (pLock) ;
}

OFC_CORE_LIB OFC_LOCK
ofc_lock_init (OFC_VOID)
{
  OFC_LOCK plock;
  plock = ofc_lock_init_impl () ;
  return (plock);
}

