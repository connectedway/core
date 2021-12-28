/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/thread.h"
#include "ofc/impl/threadimpl.h"
#include "ofc/file.h"
#include "ofc/libc.h"

OFC_CORE_LIB BLUE_HANDLE
BlueThreadCreate (OFC_DWORD(scheduler)(BLUE_HANDLE hThread,
                                       OFC_VOID *context),
                  OFC_CCHAR *thread_name, OFC_INT thread_instance,
                  OFC_VOID *context,
                  BLUE_THREAD_DETACHSTATE detachstate,
                  BLUE_HANDLE hNotify)
{
  return (BlueThreadCreateImpl (scheduler, thread_name, thread_instance,
				context, detachstate, hNotify)) ;
}

OFC_CORE_LIB OFC_VOID
BlueThreadSetWaitSet (BLUE_HANDLE hThread, BLUE_HANDLE wait_set)
{
  BlueThreadSetWaitSetImpl (hThread, wait_set) ;
}

OFC_CORE_LIB OFC_VOID
BlueThreadDelete (BLUE_HANDLE hThread)
{
  BlueThreadDeleteImpl (hThread) ;
}

OFC_CORE_LIB OFC_VOID
BlueThreadWait (BLUE_HANDLE hThread)
{
  BlueThreadWaitImpl (hThread) ;
}

OFC_CORE_LIB OFC_BOOL
BlueThreadIsDeleting (BLUE_HANDLE hThread)
{
  OFC_BOOL ret ;
  ret = BlueThreadIsDeletingImpl (hThread) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueSleep (OFC_DWORD milliseconds)
{
  BlueSleepImpl (milliseconds) ;
}

OFC_CORE_LIB OFC_DWORD
BlueThreadCreateVariable (OFC_VOID)
{
  return (BlueThreadCreateVariableImpl()) ;
}

OFC_VOID BlueThreadDestroyVariable (OFC_DWORD dkey)
{
  BlueThreadDestroyVariableImpl(dkey);
}

OFC_CORE_LIB OFC_DWORD_PTR
BlueThreadGetVariable (OFC_DWORD var)
{
  OFC_DWORD_PTR val ;

  val = BlueThreadGetVariableImpl (var) ;
  if (val > (OFC_DWORD_PTR) OFC_ERROR_NOT_ENOUGH_QUOTA)
    BlueCprintf ("Obtained Bad Status on variable %d, val 0x%08x\n",
		 var, (OFC_UINT) (OFC_DWORD_PTR) val) ;
  return (val) ;
}

OFC_CORE_LIB OFC_VOID
BlueThreadSetVariable (OFC_DWORD var, OFC_DWORD_PTR val)
{
  if (val > (OFC_DWORD_PTR) OFC_ERROR_NOT_ENOUGH_QUOTA)
    BlueCprintf ("Setting Bad Status on variable %d, val 0x%08x\n",
		 var, (OFC_UINT) val) ;
  if (var != OfcLastError || val != (OFC_DWORD_PTR) OFC_ERROR_SUCCESS)
    BlueThreadSetVariableImpl (var, val) ;
}  

OFC_CORE_LIB OFC_VOID
BlueThreadCreateLocalStorage (OFC_VOID)
{
  BlueThreadCreateLocalStorageImpl() ;
}

OFC_CORE_LIB OFC_VOID
BlueThreadDestroyLocalStorage (OFC_VOID)
{
  BlueThreadDestroyLocalStorageImpl() ;
}

OFC_CORE_LIB OFC_VOID
BlueThreadInit (OFC_VOID)
{
  BlueThreadInitImpl() ;
}

OFC_CORE_LIB OFC_VOID
BlueThreadDestroy (OFC_VOID)
{
  BlueThreadDestroyImpl() ;
}
