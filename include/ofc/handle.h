/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_HANDLE_H__)
#define __OFC_HANDLE_H__

#include "ofc/core.h"
#include "ofc/types.h"

/**
 * \defgroup handle Open Files Abstract Handles
 *
 * The File System Redirector is a key component of the Open Files Product
 * and is leveraged heavily by both the SMB Client and Server.  The redirector
 * defines the file system to file system API mapping.  
 */

/** \{ */

/**
 * The definition of the 32 bit handle
 */
typedef OFC_DWORD_PTR OFC_HANDLE;
/**
 * The definition of the 16 bit handle
 */
typedef OFC_UINT16 OFC_HANDLE16;

/**
 * An Invalid 32 bit handle Handle
 */
#define OFC_INVALID_HANDLE_VALUE ((OFC_HANDLE)-1)
/**
 * An invalid 16 bit handle value
 */
#define OFC_HANDLE16_INVALID (OFC_HANDLE16)0xFFFF
/**
 * A NULL 16 bit Handle
 */
#define OFC_HANDLE16_NULL ((OFC_HANDLE16) 0)
/**
 * A NULL handle
 */
#define OFC_HANDLE_NULL ((OFC_HANDLE) 0)

/**
 * Available Handle Types for the 32 bit handles
 */
typedef enum {
    OFC_HANDLE_UNKNOWN,    /**< The Handle type is Unknown (0x00) */
    OFC_HANDLE_WAIT_SET,    /**< The Handle is for a Wait Set (a
				   collection of events to wait on) */
    OFC_HANDLE_QUEUE,        /**< The handle is for a linked list of
				   items */
    OFC_HANDLE_FILE,        /**< The handle is for an abstracted file f
				   (platform and fs independent) */
    OFC_HANDLE_FSWIN32_FILE,    /**< The handle is for a Win32 file */
    OFC_HANDLE_FSWIN32_OVERLAPPED, /**< The handle is for a Win32 Overlapped
				       Buffer */
    OFC_HANDLE_FSWINCE_OVERLAPPED, /**< The handle is for a WinCE Overlapped
				       Buffer */
    OFC_HANDLE_FSSMB_OVERLAPPED, /**< An Overlapped Buffer for CIFS) */
    OFC_HANDLE_FSDARWIN_OVERLAPPED, /**< An Overlapped Buffer for Darwin */
    OFC_HANDLE_FSLINUX_OVERLAPPED, /**< The handle is for a linux overlapped
				       buffer */
    OFC_HANDLE_FSFILEX_OVERLAPPED, /**< The handle is for a threadx filex
				       overlapped buffer */
    OFC_HANDLE_FSNUFILE_OVERLAPPED, /**< The handle is for a Nucleus filex
					overlapped buffer */
    OFC_HANDLE_FSANDROID_OVERLAPPED, /**< The handle is for a Nucleus filex
					 overlapped buffer */
    OFC_HANDLE_FSDARWIN_FILE,    /**< The handle is for a posix file */
    OFC_HANDLE_FSLINUX_FILE,    /**< The handle for a linux file  */
    OFC_HANDLE_FSFILEX_FILE,    /**< The handle for a threadx filex file */
    OFC_HANDLE_FSANDROID_FILE,    /**< The handle for a android file */
    OFC_HANDLE_FSNUFILE_FILE,  /**< The handle for a Nucleus NUFile file */
    OFC_HANDLE_FSOTHER_FILE,   /**< The handle for a port specific file */
    OFC_HANDLE_FSBROWSER_FILE, /**< The handle on a browser file  */
    OFC_HANDLE_FSBOOKMARK_FILE, /**< A bookmark browser handle */
    OFC_HANDLE_SCHED,        /**< The handle for a scheduler  */
    OFC_HANDLE_APP,        /**< The handle for an application  */
    OFC_HANDLE_THREAD,        /**< Handle for a platform thread */
    OFC_HANDLE_SOCKET,         /**< handle is for an abstracted socket */
    OFC_HANDLE_SOCKET_IMPL,    /**< Handle for a platform socket */
    OFC_HANDLE_EVENT,        /**< Handle for an event  */
    OFC_HANDLE_TIMER,        /**< Handle for a timer  */
    OFC_HANDLE_PIPE,        /**< Handle for a pipe  */
    OFC_HANDLE_WAIT_QUEUE,    /**< Handle for a wait queue  */
    OFC_HANDLE_TRANSACTION,    /**< Handle for an ipc transaction  */
    OFC_HANDLE_SMB_FILE,    /**< Handle for a CIFS File system  */
    OFC_HANDLE_MAILSLOT,    /**< Handle for a mailslot  */
    OFC_HANDLE_PROCESS,    /**< Process */
    OFC_HANDLE_NUM        /**< Number of handle types  */
} OFC_HANDLE_TYPE;

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
OFC_CORE_LIB OFC_HANDLE
ofc_handle_create(OFC_HANDLE_TYPE hType, OFC_VOID *context);
/**
 * Increase the reference count of a handle and return the context
 *
 * \param handle
 * The handle to reference
 *
 * \returns
 * The context for the handle
 */
OFC_CORE_LIB OFC_VOID *
ofc_handle_lock(OFC_HANDLE handle);

OFC_CORE_LIB OFC_VOID *
ofc_handle_lock_ex(OFC_HANDLE handle, OFC_HANDLE_TYPE type);
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
OFC_CORE_LIB OFC_VOID
ofc_handle_unlock(OFC_HANDLE handle);
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
OFC_CORE_LIB OFC_VOID
ofc_handle_destroy(OFC_HANDLE handle);
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
OFC_CORE_LIB OFC_VOID
ofc_handle_set_app(OFC_HANDLE hHandle, OFC_HANDLE hApp, OFC_HANDLE hSet);
/**
 * Return the application associated with a handle
 *
 * \param hHandle
 * The handle to find the associated application for.
 *
 * \returns
 * The handle of the application
 *
 * \see ofc_handle_set_app
 */
OFC_CORE_LIB OFC_HANDLE
ofc_handle_get_app(OFC_HANDLE hHandle);
/**
 * Return the Wait Set that this handle is part of
 *
 * \param hHandle
 * The handle to find the wait set for.
 *
 * \returns
 * The handle of the wait set
 *
 * \see ofc_handle_set_app
 */
OFC_CORE_LIB OFC_HANDLE
ofc_handle_get_wait_set(OFC_HANDLE hHandle);
/**
 * Get the handle type
 *
 * \param hHandle
 * The handle to find the type of
 *
 * \returns
 * The type of the handle
 */
OFC_CORE_LIB OFC_HANDLE_TYPE
ofc_handle_get_type(OFC_HANDLE hHandle);
/**
 * Initialize the 16 bit handle support.
 *
 * This should be called at system initialization/startup time to
 * create the handle database and initialize the check digits.
 */
OFC_CORE_LIB OFC_VOID
ofc_handle16_init(OFC_VOID);
/**
 * Destroy the 16 bit handle structures
 */
OFC_CORE_LIB OFC_VOID
ofc_handle16_free(OFC_VOID);
/**
 * Create a 16 bit handle and associate with a context
 *
 * \param context
 * The context to associate with the handle
 *
 * \returns
 * The 16 bit handle
 */
OFC_CORE_LIB OFC_HANDLE16
ofc_handle16_create(OFC_VOID *context);
/**
 * Destroy a 16 bit handle
 *
 * \param hHandle
 * The 16 bit handle to destroy
 */
OFC_CORE_LIB OFC_VOID
ofc_handle16_destroy(OFC_HANDLE16 hHandle);
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
 * invalid, OFC_NULL is returned.
 */
OFC_CORE_LIB OFC_VOID *
ofc_handle16_lock(OFC_HANDLE16 hHandle);
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
OFC_CORE_LIB OFC_VOID
ofc_handle16_unlock(OFC_HANDLE16 hHandle);

#if defined(OFC_HANDLE_DEBUG)
OFC_CORE_LIB OFC_VOID ofc_handle_measure (OFC_HANDLE hHandle) ;
OFC_CORE_LIB OFC_MSTIME ofc_handle_get_avg_interval (OFC_HANDLE hHandle,
                            OFC_UINT32 *count,
                            OFC_HANDLE_TYPE *type) ;
OFC_CORE_LIB OFC_VOID ofc_handle_print_interval_header (OFC_VOID) ;
OFC_CORE_LIB OFC_VOID ofc_handle_print_interval (OFC_CHAR *prefix,
                         OFC_HANDLE hHandle) ;
#endif
#if defined(__cplusplus)
}
#endif
/** \} */
#endif
