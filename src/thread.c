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

OFC_CORE_LIB OFC_HANDLE
ofc_thread_create(OFC_DWORD(scheduler)(OFC_HANDLE hThread,
                                       OFC_VOID *context),
                  OFC_CCHAR *thread_name, OFC_INT thread_instance,
                  OFC_VOID *context,
                  OFC_THREAD_DETACHSTATE detachstate,
                  OFC_HANDLE hNotify) {
    return (ofc_thread_create_impl(scheduler, thread_name, thread_instance,
                                   context, detachstate, hNotify));
}

OFC_CORE_LIB OFC_VOID
ofc_thread_set_wait(OFC_HANDLE hThread, OFC_HANDLE wait_set) {
    ofc_thread_set_waitset_impl(hThread, wait_set);
}

OFC_CORE_LIB OFC_VOID
ofc_thread_delete(OFC_HANDLE hThread) {
    ofc_thread_delete_impl(hThread);
}

OFC_CORE_LIB OFC_VOID
ofc_thread_wait(OFC_HANDLE hThread) {
    ofc_thread_wait_impl(hThread);
}

OFC_CORE_LIB OFC_BOOL
ofc_thread_is_deleting(OFC_HANDLE hThread) {
    OFC_BOOL ret;
    ret = ofc_thread_is_deleting_impl(hThread);
    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_sleep(OFC_DWORD milliseconds) {
    ofc_sleep_impl(milliseconds);
}

OFC_CORE_LIB OFC_DWORD
ofc_thread_create_variable(OFC_VOID) {
    return (ofc_thread_create_variable_impl());
}

OFC_VOID ofc_thread_destroy_variable(OFC_DWORD dkey) {
    ofc_thread_destroy_variable_impl(dkey);
}

OFC_CORE_LIB OFC_DWORD_PTR
ofc_thread_get_variable(OFC_DWORD var) {
    OFC_DWORD_PTR val;

    val = ofc_thread_get_variable_impl(var);
    return (val);
}

OFC_CORE_LIB OFC_VOID
ofc_thread_set_variable(OFC_DWORD var, OFC_DWORD_PTR val) {
    if (var != OfcLastError || val != (OFC_DWORD_PTR) OFC_ERROR_SUCCESS)
        ofc_thread_set_variable_impl(var, val);
}

OFC_CORE_LIB OFC_VOID
ofc_thread_create_local_storage(OFC_VOID) {
    ofc_thread_create_local_storage_impl();
}

OFC_CORE_LIB OFC_VOID
ofc_thread_destroy_local_storage(OFC_VOID) {
    ofc_thread_destroy_local_storage_impl();
}

OFC_CORE_LIB OFC_VOID
ofc_thread_init(OFC_VOID) {
    ofc_thread_init_impl();
}

OFC_CORE_LIB OFC_VOID
ofc_thread_destroy(OFC_VOID) {
    ofc_thred_destroy_impl();
}
