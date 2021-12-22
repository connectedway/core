/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__BLUE_HANDLE_H__)
#define __BLUE_HANDLE_H__

#include "ofc/core.h"
#include "ofc/types.h"

/**
 * The definition of the 32 bit handle
 */
typedef BLUE_DWORD_PTR BLUE_HANDLE ;
/**
 * The definition of the 16 bit handle
 */
typedef BLUE_UINT16 BLUE_HANDLE16 ;

/**
 * An Invalid 32 bit handle Handle
 */
#define BLUE_INVALID_HANDLE_VALUE ((BLUE_HANDLE)-1)
/**
 * An invalid 16 bit handle value
 */
#define BLUE_HANDLE16_INVALID (BLUE_HANDLE16)0xFFFF
/**
 * A NULL 16 bit Handle
 */
#define BLUE_HANDLE16_NULL ((BLUE_HANDLE16) 0)
/**
 * A NULL handle
 */
#define BLUE_HANDLE_NULL ((BLUE_HANDLE) 0)

/**
 * Available Handle Types for the 32 bit handles
 */
typedef enum
  {
    BLUE_HANDLE_UNKNOWN,	/**< The Handle type is Unknown (0x00) */
    BLUE_HANDLE_WAIT_SET,	/**< The Handle is for a Wait Set (a 
				   collection of events to wait on) */
    BLUE_HANDLE_QUEUE,		/**< The handle is for a linked list of 
				   items */
    BLUE_HANDLE_FILE,		/**< The handle is for an abstracted file f
				   (platform and fs independent) */
    BLUE_HANDLE_FSWIN32_FILE,	/**< The handle is for a Win32 file */
    BLUE_HANDLE_FSWIN32_OVERLAPPED, /**< The handle is for a Win32 Overlapped 
				       Buffer */
    BLUE_HANDLE_FSWINCE_OVERLAPPED, /**< The handle is for a WinCE Overlapped 
				       Buffer */
    BLUE_HANDLE_FSCIFS_OVERLAPPED, /**< An Overlapped Buffer for CIFS) */
    BLUE_HANDLE_FSDARWIN_OVERLAPPED, /**< An Overlapped Buffer for Darwin */
    BLUE_HANDLE_FSLINUX_OVERLAPPED, /**< The handle is for a linux overlapped
				       buffer */
    BLUE_HANDLE_FSFILEX_OVERLAPPED, /**< The handle is for a threadx filex
				       overlapped buffer */
    BLUE_HANDLE_FSNUFILE_OVERLAPPED, /**< The handle is for a Nucleus filex
					overlapped buffer */
    BLUE_HANDLE_FSANDROID_OVERLAPPED, /**< The handle is for a Nucleus filex
					 overlapped buffer */
    BLUE_HANDLE_FSDARWIN_FILE,	/**< The handle is for a posix file */
    BLUE_HANDLE_FSLINUX_FILE,	/**< The handle for a linux file  */
    BLUE_HANDLE_FSFILEX_FILE,	/**< The handle for a threadx filex file */
    BLUE_HANDLE_FSANDROID_FILE,	/**< The handle for a android file */
    BLUE_HANDLE_FSNUFILE_FILE,  /**< The handle for a Nucleus NUFile file */
    BLUE_HANDLE_FSOTHER_FILE,   /**< The handle for a port specific file */
    BLUE_HANDLE_FSBROWSER_FILE, /**< The handle on a browser file  */
    BLUE_HANDLE_FSBOOKMARK_FILE, /**< A bookmark browser handle */
    BLUE_HANDLE_SCHED, 		/**< The handle for a scheduler  */
    BLUE_HANDLE_APP,		/**< The handle for an application  */
    BLUE_HANDLE_THREAD,		/**< Handle for a platform thread */
    BLUE_HANDLE_SOCKET,         /**< handle is for an abstracted socket */
    BLUE_HANDLE_SOCKET_IMPL,	/**< Handle for a platform socket */
    BLUE_HANDLE_EVENT,		/**< Handle for an event  */
    BLUE_HANDLE_TIMER,		/**< Handle for a timer  */
    BLUE_HANDLE_PIPE,		/**< Handle for a pipe  */
    BLUE_HANDLE_WAIT_QUEUE,	/**< Handle for a wait queue  */
    BLUE_HANDLE_TRANSACTION,	/**< Handle for an ipc transaction  */
    BLUE_HANDLE_CIFS_FILE,	/**< Handle for a CIFS File system  */
    BLUE_HANDLE_MAILSLOT,	/**< Handle for a mailslot  */
    BLUE_HANDLE_PROCESS,	/**< Process */
    BLUE_HANDLE_NUM		/**< Number of handle types  */
  } BLUE_HANDLE_TYPE ;

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Create a handle from some context
   *
   * \param hType
   * The handle Type
   *
   * \param context
   * The context to associate with the handle
   *
   * \returns
   * The Handle
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueHandleCreate (BLUE_HANDLE_TYPE hType, BLUE_VOID *context) ;
  /**
   * Increase the reference count of a handle and return the context
   *
   * \param handle
   * The handle to reference
   *
   * \returns
   * The context for the handle
   */
  BLUE_CORE_LIB BLUE_VOID *
  BlueHandleLock (BLUE_HANDLE handle) ;
  BLUE_CORE_LIB BLUE_VOID *
  BlueHandleLockEx (BLUE_HANDLE handle, BLUE_HANDLE_TYPE type) ;
  /**
   * Decrement the reference count of a handle
   *
   * \param handle
   * The handle to unreference
   *
   * \remark
   * If the handle is marked to be destroyed and the decrement results in
   * the reference count reaching 0, the handle will be freed
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueHandleUnlock (BLUE_HANDLE handle) ;
  /**
   * Mark a handle to be destroyed
   *
   * \param handle
   * The handle to destroy
   *
   * \remark
   * If outstanding references are still held on the handle, the handle will
   * be marked for destruction.  If no references are outstanding, the 
   * handle will be destroyed immediately.
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueHandleDestroy (BLUE_HANDLE handle) ;
  /**
   * Associate a handle with an application
   *
   * \param hHandle
   * Handle to associate
   *
   * \param hApp
   * Application to associate
   *
   * \param hSet
   * The Wait Set that this handle is part of
   *
   * \remark
   * This is used when adding a handle to a wait set which the scheduler uses
   * to wait for an event.  By associating the handle with an application,
   * the scheduler knows which application to service when an event occurs
   * on a specific handle
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueHandleSetApp (BLUE_HANDLE hHandle, BLUE_HANDLE hApp, BLUE_HANDLE hSet) ;
  /**
   * Return the application associated with a handle
   *
   * \param hHandle
   * The handle to find the associated application for.
   *
   * \returns
   * The handle of the application
   *
   * \see BlueHandleSetApp
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueHandleGetApp (BLUE_HANDLE hHandle) ;
  /**
   * Return the Wait Set that this handle is part of
   *
   * \param hHandle
   * The handle to find the wait set for.
   *
   * \returns
   * The handle of the wait set
   *
   * \see BlueHandleSetApp
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueHandleGetWaitSet (BLUE_HANDLE hHandle) ;
  /**
   * Get the handle type
   *
   * \param hHandle
   * The handle to find the type of
   *
   * \returns
   * The type of the handle
   */
  BLUE_CORE_LIB BLUE_HANDLE_TYPE 
  BlueHandleGetType (BLUE_HANDLE hHandle) ;
  /**
   * Initialize the 16 bit handle support. 
   *
   * This should be called at system initialization/startup time to 
   * create the handle database and initialize the check digits.
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueHandle16Init (BLUE_VOID) ;
  /**
   * Destroy the 16 bit handle structures
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueHandle16Free (BLUE_VOID) ;
  /**
   * Create a 16 bit handle and associate with a context
   *
   * \param context
   * The context to associate with the handle
   *
   * \returns
   * The 16 bit handle
   */
  BLUE_CORE_LIB BLUE_HANDLE16 
  BlueHandle16Create (BLUE_VOID *context) ;
  /**
   * Destroy a 16 bit handle
   *
   * \param hHandle
   * The 16 bit handle to destroy
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueHandle16Destroy (BLUE_HANDLE16 hHandle) ;
  /**
   * Reference a 16 bit handle
   *
   * This will take a 16 bit handle, increment it's reference count
   * and return the context to the handle.
   *
   * \param hHandle
   * The 16 bit handle
   *
   * \returns
   * A pointer to the context associated with the handle.  If the handle is 
   * invalid, BLUE_NULL is returned.
   */
  BLUE_CORE_LIB BLUE_VOID *
  BlueHandle16Lock (BLUE_HANDLE16 hHandle) ;
  /**
   * Dereference a 16 bit handle
   *
   * \param hHandle
   * The 16 bit handle to dereference
   *
   * \remark
   * If the reference count reaches zero and the handle is marked for 
   * destruction the handle will be released.
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueHandle16Unlock (BLUE_HANDLE16 hHandle) ;

#if defined(BLUE_PARAM_HANDLE_DEBUG)
  BLUE_CORE_LIB BLUE_VOID BlueHandleMeasure (BLUE_HANDLE hHandle) ;
  BLUE_CORE_LIB BLUE_MSTIME BlueHandleGetAvgInterval (BLUE_HANDLE hHandle,
						      BLUE_UINT32 *count,
						      BLUE_HANDLE_TYPE *type) ;
  BLUE_CORE_LIB BLUE_VOID BlueHandlePrintIntervalHeader (BLUE_VOID) ;
  BLUE_CORE_LIB BLUE_VOID BlueHandlePrintInterval (BLUE_CHAR *prefix,
						   BLUE_HANDLE hHandle) ;
#endif
#if defined(__cplusplus)
}
#endif
#endif
