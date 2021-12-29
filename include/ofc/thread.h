/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_THREAD_H__)
#define __OFC_THREAD_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/**
 * \defgroup util Open File APIs
 *
 * The Open File APIs are a collection of APIs that are used both
 * by the internal Open Files stack implementation and are used by 
 * the Applications.
 * 
 * The main facilities of Open Files are the SMB Client, SMB Server, and
 * NetBIOS stack.  The Server and NetBIOS stack interact with users
 * through the configuration facility only.  The Client faility 
 * interacts with the user through the File Redirector and the Path
 * handling faclity.  
 */

/**
 * \defgroup internal APIS for Internal Facilities
 */

/**
 * \defgroup thread Threading Utility Functions
 * \ingroup internal
 * 
 * The thread facility allows Open Files to manage platform threads in
 * a generic fashion.
 *
 * For examples on using the threads see test_thread.c
 *
 * \see thread_test.c
 *
 * \{
 */

/**
 * A constant to indicate infinite sleep
 */
#define OFC_INFINITE -1
/**
 * A constant to specify that a thread has no instance
 */
#define OFC_THREAD_SINGLETON -1
/**
 * Thread Names
 */
#define OFC_THREAD_SESSION       "BLSESS"
#define OFC_THREAD_CONFIG_UPDATE "BLCFGU"
#define OFC_THREAD_AIO           "BLAIOT"
#define OFC_THREAD_SRVSVC        "BLSVRV"
#define OFC_THREAD_WKSSVC        "BLWKSV"
#define OFC_THREAD_BROWSER_TEST  "BLBRWT"
#define OFC_THREAD_BROWSE_SHARES_TEST  "BLBRSH"
#define OFC_THREAD_PIPE_TEST     "BLPIPT"
#define OFC_THREAD_FILE_TEST     "BLFILT"
#define OFC_THREAD_MAIL_TEST     "BLMALT"
#define OFC_THREAD_NAME_TEST     "BLNAMT"
#define OFC_THREAD_PROCESS_TEST  "BLPRCT"
#define OFC_THREAD_THREAD_TEST   "BLTHDT"
#define OFC_THREAD_XACTION_TEST  "BLXCTT"
#define OFC_THREAD_SCHED         "BLSKED"
#define OFC_THREAD_SOCKET        "BLSOCK"

/**
 * The detach states
 */
typedef enum {
    OFC_THREAD_DETACH,        /**< The thread should clean up itself */
    OFC_THREAD_JOIN        /**< The thread will be joined */
} OFC_THREAD_DETACHSTATE;


/**
 * The prototype for the thread function
 */
typedef OFC_DWORD(OFC_THREAD_FN)(OFC_HANDLE hThread, OFC_VOID *context);

#if defined(__cplusplus)
extern "C"
{
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
 * Whether the thread should clean up on it's own or whether someone will
 * join with it
 *
 * \returns The thread handle
 */
OFC_CORE_LIB OFC_HANDLE
ofc_thread_create(OFC_THREAD_FN scheduler,
                  OFC_CCHAR *thread_name, OFC_INT thread_instance,
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
OFC_CORE_LIB OFC_VOID
ofc_thread_delete(OFC_HANDLE hThread);
/**
 * Wait for a thread to exit
 *
 * \param hThread
 * Handle to the thread to wait for
 */
OFC_CORE_LIB OFC_VOID
ofc_thread_wait(OFC_HANDLE hThread);
/**
 * Check whether thread deletion has been scheduled.
 *
 * This function will return whether someone has called ofc_thread_delete
 *
 * \param hThread Handle of the thread to check
 * \returns OFC_TRUE if deletion is scheduled, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_thread_is_deleting(OFC_HANDLE hThread);
/**
 * Sleep for a specified number of milliseconds
 *
 * \param milliseconds Number of milliseconds to sleep.  OFC_INFINITE will
 * sleep forever.
 */
OFC_CORE_LIB OFC_VOID
ofc_sleep(OFC_DWORD milliseconds);
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
OFC_CORE_LIB OFC_DWORD
ofc_thread_create_variable(OFC_VOID);

OFC_VOID ofc_thread_destroy_variable(OFC_DWORD dkey);
/**
 * Set the value of a thread specific variable
 *
 * \param var Variable ID to set
 * \param val Pointer to a structure that becomes the variables value
 * If setting a 32 bit value, the pointer can be cast to a DWORD.
 */
OFC_CORE_LIB OFC_VOID
ofc_thread_set_variable(OFC_DWORD var, OFC_DWORD_PTR val);
/**
 * Get the value of a variable
 *
 * \param var Variable id
 * \returns Pointer to the variable value structure.  If the variable value
 * was a 32 bit value, this can be cast to a DWORD value.
 */
OFC_CORE_LIB OFC_DWORD_PTR
ofc_thread_get_variable(OFC_DWORD var);
/**
 * Create Local Storage for Local Variables
 *
 * This routine initializes local storage for a thread.  This routine is
 * only required on platforms that do not support the notion of
 * Thread Local Storage.  It is a noop on platforms that do support TLS.
 * The routine should be called before any call using the Open Files API
 * (any call that manipulates last error).
 */
OFC_CORE_LIB OFC_VOID
ofc_thread_create_local_storage(OFC_VOID);
/**
 * Destroys Local Storage for a thread
 */
OFC_CORE_LIB OFC_VOID
ofc_thread_destroy_local_storage(OFC_VOID);
/**
 * Initialize the Thread Facility
 */
OFC_CORE_LIB OFC_VOID
ofc_thread_init(OFC_VOID);
/**
 * Destructor for thread facility
 */
OFC_CORE_LIB OFC_VOID
ofc_thread_destroy(OFC_VOID);
/**
 * Set the wait set of a thread
 *
 * Used to wake the thread up
 *
 * \param hThread
 * Thread to set the wait set to
 *
 * \param wait_set
 * Handle to the wait set
 */
OFC_CORE_LIB OFC_VOID
ofc_thread_set_wait(OFC_HANDLE hThread, OFC_HANDLE wait_set);

#if defined(__cplusplus)
}
#endif
/** \} */
#include "impl/threadimpl.h"

#endif

