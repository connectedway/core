/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_FS_TYPE_H__)
#define __OFC_FS_TYPE_H__

/**
 * \addtogroup fs
 *
 */

/** \{ */

/**
 * Type types of File System Plugins Supported
 */
typedef enum {
    OFC_FST_WIN32,        /**< A win32 type filesystem  */
    OFC_FST_DARWIN,        /**< A Darwin/Posix type filesystem */
    OFC_FST_LINUX,        /**< A Linux/Posix type filesystem */
    OFC_FST_FILEX,        /**< A ThreadX type filesystem */
    OFC_FST_NUFILE,        /**< A Nucleus NUFile type filesystem */
    OFC_FST_ANDROID,        /**< An Android Filesystem */
    OFC_FST_OTHER,        /**< A platform specific type filesystem  */
    OFC_FST_SMB,        /**< A CIFS/SMB type filesystem  */
    OFC_FST_FILE,        /**< The redirectory filesystem (loopback) */
    OFC_FST_PIPE,        /**< A Pipe File System  */
    OFC_FST_MAILSLOT,        /**< A Mailslot File System  */
    OFC_FST_BROWSE_WORKGROUPS,    /**< A Filesystem for Browsing Workgroups  */
    OFC_FST_BROWSE_SERVERS,    /**< A Filesystem for Browsing Servers  */
    OFC_FST_BROWSE_SHARES,    /**< A Filesystem for Browsing Shares  */
    OFC_FST_BOOKMARKS,        /**< The Bookmarks browser */
    OFC_FST_WINCE,        /**< A wince type filesystem  */
    OFC_FST_UNKNOWN,        /**< An unknown Filesystem  */
    OFC_FST_NUM            /**< The number of filesystems supported  */
} OFC_FST_TYPE;

/** \} */

#endif
