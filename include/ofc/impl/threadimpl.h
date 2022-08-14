/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_THREAD_IMPL_H__)
#define __OFC_THREAD_IMPL_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/thread.h"

/**
 * \defgroup thread_impl Thread Management Implementation
 * \ingroup port
 *
 * This facility implements the Platform Thread abstractions
 */
#undef __cyg_profile
#if defined(__GNUC__) && (defined(__arm__) || defined(__aarch64__) || defined(ANDROID) || defined(__ANDROID__))
//#define __cyg_profile
#endif

/** \{ */

#if defined(__cplusplus)
extern "C"
{
#endif
#if defined(__cyg_profile)
void __cyg_profile_func_enter (void *this_fn, void *call_site)
  __attribute__((no_instrument_function));
void __cyg_profile_func_exit (void *this_fn, void *call_site)
  __attribute__((no_instrument_function));
void *__cyg_profile_return_address(int level)
  __attribute__((no_instrument_function)) ;
const char *__cyg_profile_addr2sym(void *address)
  __attribute__((no_instrument_function)) ;

#define RETURN_ADDRESS() __cyg_profile_return_address(0)
#elif defined(OFC_STACK_TRACE)
#define RETURN_ADDRESS() __builtin_return_address(0)
#else
#define RETURN_ADDRESS() OFC_NULL
#endif

/**
 * Create a Thread on the target platform
 *
 * The routine will create a thread on the target platform.  Typically,
 * there are many parameters that can be specified when creating
 * target platform threads but, for the most part, Open Files cares
 * only about a few.  For simplicity, we have limited our support only to
 * those parameters required.
 *
 * \param scheduler The Entry point to the thread.  The entry point will
 * be passed the thread handle and a pointer to the context.
 *
 * \param context The context to pass to the thread
 *
 * \param detachstate
 * Whether anyone wants to join with the thread or the thread should
 * clean up on it's own
 *
 * \param hNotify
 * Who to notify when the thread exits.  If OFC_HANDLE_NULL, then no
 * event is set.
 *
 * \returns The thread handle
 */

OFC_HANDLE ofc_thread_create_impl(OFC_THREAD_FN scheduler,
                                  OFC_CCHAR *thread_name,
                                  OFC_INT thread_instance,
                                  OFC_VOID *context,
                                  OFC_THREAD_DETACHSTATE detachstate,
                                  OFC_HANDLE hNotify);

/**
 * Delete a thread
 *
 * This routine will cause the target thread to begin deletion.  It
 * doesn't actually forcibly kill the thread, but rather sets a flag that
 * the thread can check by calling ofc_thread_is_deleting.  If this flag
 * is set, the thread should clean up and exit.
 *
 * \param hThread Handle to the thread to delete
 */
OFC_VOID ofc_thread_delete_impl(OFC_HANDLE hThread);

/**
 * Wait for a thread to exit
 *
 * \param hThread
 * Handle to thread to wait for
 */
OFC_VOID ofc_thread_wait_impl(OFC_HANDLE hThread);

/**
 * Check whether thread deletion has been scheduled.
 *
 * This function will return whether someone has called ofc_thread_delete
 *
 * \param hThread Handle of the thread to check
 * \returns OFC_TRUE if deletion is scheduled, OFC_FALSE otherwise
 */
OFC_BOOL ofc_thread_is_deleting_impl(OFC_HANDLE hThread);

/**
 * Sleep for a specified number of milliseconds
 *
 * \param milliseconds Number of milliseconds to sleep.  OFC_INFINITE will
 * sleep forever.
 */
OFC_VOID ofc_sleep_impl(OFC_DWORD milliseconds);

/**
 * Create a Thread Specific Variable
 *
 * It is often useful to create multiple threads all running the same
 * program.  In this case, it is also useful to create variables that are
 * specific to the thread.  Therefore, two threads will have variables
 * that are specific only to that thread.  This is typically used for
 * C runtime functions involving errno or GetLastError on Win32.
 *
 * \returns Variable ID
 */
OFC_DWORD ofc_thread_create_variable_impl(OFC_VOID);

OFC_VOID ofc_thread_destroy_variable_impl(OFC_DWORD dkey);

/**
 * Get the value of a variable
 *
 * \param var Variable id
 * \returns Pointer to the variable value structure.  If the variable value
 * was a 32 bit value, this can be cast to a DWORD value.
 */
OFC_DWORD_PTR ofc_thread_get_variable_impl(OFC_DWORD var);

/**
 * Set the value of a thread specific variable
 *
 * \param var Variable ID to set
 * \param val Pointer to a structure that becomes the variables value
 * If setting a 32 bit value, the pointer can be cast to a DWORD.
 */
OFC_VOID
ofc_thread_set_variable_impl(OFC_DWORD var, OFC_DWORD_PTR val);

/**
 * Set the wait set of a thread
 *
 * \param hThread
 * Thread to set the wait set for
 *
 * \param wait_set
 * wait set to set
 */
OFC_VOID
ofc_thread_set_waitset_impl(OFC_HANDLE hThread, OFC_HANDLE wait_set);
/**
 * Create Local Storage for Local Variables
 *
 * This routine initializes local storage for a thread.  This routine is
 * only required on platforms that do not support the notion of
 * Thread Local Storage.  It is a noop on platforms that do support TLS.
 * The routine should be called before any call using the file API
 * (any call that manipulates last error).
 */
OFC_CORE_LIB OFC_VOID
ofc_thread_create_local_storage_impl(OFC_VOID);
/**
 * Destroys Local Storage for a thread
 */
OFC_CORE_LIB OFC_VOID
ofc_thread_destroy_local_storage_impl(OFC_VOID);
/**
 * Initialize the Thread Facility
 */
OFC_CORE_LIB OFC_VOID
ofc_thread_init_impl(OFC_VOID);

OFC_CORE_LIB OFC_VOID
ofc_thread_destroy_impl(OFC_VOID);

OFC_CORE_LIB OFC_VOID
ofc_thread_detach_impl(OFC_HANDLE hThread);

#if defined(__cplusplus)
}
#endif

#endif

/** \} */
