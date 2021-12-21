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
#if !defined(__BLUE_THREAD_H__)
#define __BLUE_THREAD_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/**
 * \defgroup BlueUtil Blue Share Facility APIs
 *
 * The Blue Share Facility APIs are a collection of APIs that are used both
 * by the internal Blue Share stack implementation and are used by 
 * the Blue Share Customer Applications.
 * 
 * The main facilities of Blue Share are the Cifs Client, Cifs Server, and
 * NetBIOS stack.  The Cifs Server and NetBIOS stack interact with users
 * through the configuration facility only.  The Cifs Client faility 
 * interacts with the user through the Blue File Redirectory and the Path
 * handling faclity.  
 */

/**
 * \defgroup BlueInternal APIS for Internal Facilities
 */

/**
 * \defgroup BlueThread Threading Utility Functions
 * \ingroup BlueInternal
 * 
 * The BlueThread facility allows Blue Share to manage platform threads in
 * a generic fashion.
 *
 * For examples on using the BlueThread see BlueThreadTest.c
 *
 * \see BlueThreadTest.c
 *
 * \{
 */

/**
 * A constant to indicate infinite sleep
 */
#define BLUE_INFINITE -1
/**
 * A constant to specify that a thread has no instance
 */
#define BLUE_THREAD_SINGLETON -1
/**
 * Thread Names
 */
#define BLUE_THREAD_SESSION       "BLSESS"
#define BLUE_THREAD_CONFIG_UPDATE "BLCFGU"
#define BLUE_THREAD_AIO           "BLAIOT"
#define BLUE_THREAD_SRVSVC        "BLSVRV"
#define BLUE_THREAD_WKSSVC        "BLWKSV"
#define BLUE_THREAD_BROWSER_TEST  "BLBRWT"
#define BLUE_THREAD_BROWSE_SHARES_TEST  "BLBRSH"
#define BLUE_THREAD_PIPE_TEST     "BLPIPT"
#define BLUE_THREAD_FILE_TEST     "BLFILT"
#define BLUE_THREAD_MAIL_TEST     "BLMALT"
#define BLUE_THREAD_NAME_TEST     "BLNAMT"
#define BLUE_THREAD_PROCESS_TEST  "BLPRCT"
#define BLUE_THREAD_THREAD_TEST   "BLTHDT"
#define BLUE_THREAD_XACTION_TEST  "BLXCTT"
#define BLUE_THREAD_SCHED         "BLSKED"
#define BLUE_THREAD_SOCKET        "BLSOCK"

/**
 * The detach states
 */
typedef enum
  {
    BLUE_THREAD_DETACH,		/**< The thread should clean up itself */
    BLUE_THREAD_JOIN		/**< The thread will be joined */
  } BLUE_THREAD_DETACHSTATE ;


/**
 * The prototype for the thread function
 */
typedef BLUE_DWORD(BLUE_THREAD_FN)(BLUE_HANDLE hThread, BLUE_VOID *context) ;

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Create a Thread on the target platform
   *
   * The routine will create a thread on the target platform.  Typically,
   * there are many parameters that can be specified when creating 
   * target platform threads but, for the most part, Blue Share cares
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
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueThreadCreate (BLUE_THREAD_FN scheduler, 
		    BLUE_CCHAR *thread_name, BLUE_INT thread_instance,
		    BLUE_VOID *context,
		    BLUE_THREAD_DETACHSTATE detachstate,
		    BLUE_HANDLE hNotify) ;
  /**
   * Delete a thread
   *
   * This routine will cause the target thread to begin deletion.  It 
   * doesn't actually forcibly kill the thread, but rather sets a flag that
   * the thread can check by calling BlueThreadIsDeleting.  If this flag
   * is set, the thread should clean up and exit.
   *
   * \param hThread Handle to the thread to delete
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueThreadDelete (BLUE_HANDLE hThread) ;
  /**
   * Wait for a thread to exit
   *
   * \param hThread
   * Handle to the thread to wait for
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueThreadWait (BLUE_HANDLE hThread) ;
  /**
   * Check whether thread deletion has been scheduled.
   *
   * This function will return whether someone has called BlueThreadDelete
   *
   * \param hThread Handle of the thread to check
   * \returns BLUE_TRUE if deletion is scheduled, BLUE_FALSE otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueThreadIsDeleting (BLUE_HANDLE hThread) ;
  /**
   * Sleep for a specified number of milliseconds
   *
   * \param milliseconds Number of milliseconds to sleep.  BLUE_INFINITE will
   * sleep forever.
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSleep (BLUE_DWORD milliseconds) ;
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
  BLUE_CORE_LIB BLUE_DWORD 
  BlueThreadCreateVariable (BLUE_VOID) ;
  BLUE_VOID BlueThreadDestroyVariable (BLUE_DWORD dkey);
  /**
   * Set the value of a thread specific variable
   *
   * \param var Variable ID to set
   * \param val Pointer to a structure that becomes the variables value
   * If setting a 32 bit value, the pointer can be cast to a DWORD.
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueThreadSetVariable (BLUE_DWORD var, BLUE_DWORD_PTR val) ;
  /**
   * Get the value of a variable
   *
   * \param var Variable id
   * \returns Pointer to the variable value structure.  If the variable value
   * was a 32 bit value, this can be cast to a DWORD value.
   */
  BLUE_CORE_LIB BLUE_DWORD_PTR
  BlueThreadGetVariable (BLUE_DWORD var) ;
  /**
   * Create Local Storage for Local Variables
   *
   * This routine initializes local storage for a thread.  This routine is 
   * only required on platforms that do not support the notion of 
   * Thread Local Storage.  It is a noop on platforms that do support TLS.
   * The routine should be called before any call using the BlueFile API 
   * (any call that manipulates last error).
   */
  BLUE_CORE_LIB BLUE_VOID
  BlueThreadCreateLocalStorage (BLUE_VOID) ;
  /**
   * Destroys Local Storage for a thread
   */
  BLUE_CORE_LIB BLUE_VOID
  BlueThreadDestroyLocalStorage (BLUE_VOID) ;
  /**
   * Initialize the Blue Thread Facility
   */
  BLUE_CORE_LIB BLUE_VOID
  BlueThreadInit (BLUE_VOID) ;
  /**
   * Destructor for thread facility
   */
   BLUE_CORE_LIB BLUE_VOID
   BlueThreadDestroy (BLUE_VOID) ;
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
  BLUE_CORE_LIB BLUE_VOID 
  BlueThreadSetWaitSet (BLUE_HANDLE hThread, BLUE_HANDLE wait_set) ;

#if defined(__cplusplus)
}
#endif
/** \} */
#include "impl/threadimpl.h"

#endif

