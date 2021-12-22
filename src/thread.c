/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __BLUE_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/thread.h"
#include "ofc/impl/threadimpl.h"
#include "ofc/file.h"
#include "ofc/libc.h"

BLUE_CORE_LIB BLUE_HANDLE 
BlueThreadCreate (BLUE_DWORD(scheduler)(BLUE_HANDLE hThread,
					BLUE_VOID *context),
		  BLUE_CCHAR *thread_name, BLUE_INT thread_instance,
		  BLUE_VOID *context,
		  BLUE_THREAD_DETACHSTATE detachstate,
		  BLUE_HANDLE hNotify) 
{
  return (BlueThreadCreateImpl (scheduler, thread_name, thread_instance,
				context, detachstate, hNotify)) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueThreadSetWaitSet (BLUE_HANDLE hThread, BLUE_HANDLE wait_set)
{
  BlueThreadSetWaitSetImpl (hThread, wait_set) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueThreadDelete (BLUE_HANDLE hThread)
{
  BlueThreadDeleteImpl (hThread) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueThreadWait (BLUE_HANDLE hThread)
{
  BlueThreadWaitImpl (hThread) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueThreadIsDeleting (BLUE_HANDLE hThread)
{
  BLUE_BOOL ret ;
  ret = BlueThreadIsDeletingImpl (hThread) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueSleep (BLUE_DWORD milliseconds) 
{
  BlueSleepImpl (milliseconds) ;
}

BLUE_CORE_LIB BLUE_DWORD 
BlueThreadCreateVariable (BLUE_VOID)
{
  return (BlueThreadCreateVariableImpl()) ;
}

BLUE_VOID BlueThreadDestroyVariable (BLUE_DWORD dkey)
{
  BlueThreadDestroyVariableImpl(dkey);
}

BLUE_CORE_LIB BLUE_DWORD_PTR 
BlueThreadGetVariable (BLUE_DWORD var) 
{
  BLUE_DWORD_PTR val ;

  val = BlueThreadGetVariableImpl (var) ;
  if (val > (BLUE_DWORD_PTR) BLUE_ERROR_NOT_ENOUGH_QUOTA)
    BlueCprintf ("Obtained Bad Status on variable %d, val 0x%08x\n",
		 var, (BLUE_UINT) (BLUE_DWORD_PTR) val) ;
  return (val) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueThreadSetVariable (BLUE_DWORD var, BLUE_DWORD_PTR val) 
{
  if (val > (BLUE_DWORD_PTR) BLUE_ERROR_NOT_ENOUGH_QUOTA)
    BlueCprintf ("Setting Bad Status on variable %d, val 0x%08x\n",
		 var, (BLUE_UINT) val) ;
  if (var != BlueLastError || val != (BLUE_DWORD_PTR) BLUE_ERROR_SUCCESS)
    BlueThreadSetVariableImpl (var, val) ;
}  

BLUE_CORE_LIB BLUE_VOID
BlueThreadCreateLocalStorage (BLUE_VOID)
{
  BlueThreadCreateLocalStorageImpl() ;
}

BLUE_CORE_LIB BLUE_VOID
BlueThreadDestroyLocalStorage (BLUE_VOID)
{
  BlueThreadDestroyLocalStorageImpl() ;
}

BLUE_CORE_LIB BLUE_VOID
BlueThreadInit (BLUE_VOID)
{
  BlueThreadInitImpl() ;
}

BLUE_CORE_LIB BLUE_VOID
BlueThreadDestroy (BLUE_VOID)
{
  BlueThreadDestroyImpl() ;
}
