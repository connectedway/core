/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__BLUE_FS_TYPE_H__)
#define __BLUE_FS_TYPE_H__

/**
 * \addtogroup BlueFS
 *
 */

/** \{ */

/**
 * Type types of File System Plugins Supported
 */
typedef enum
  {
    BLUE_FS_WIN32,		/**< A win32 type filesystem  */
    BLUE_FS_DARWIN,		/**< A Darwin/Posix type filesystem */
    BLUE_FS_LINUX,		/**< A Linux/Posix type filesystem */
    BLUE_FS_FILEX,		/**< A ThreadX type filesystem */
    BLUE_FS_NUFILE,		/**< A Nucleus NUFile type filesystem */
    BLUE_FS_ANDROID,		/**< An Android Filesystem */
    BLUE_FS_OTHER,		/**< A platform specific type filesystem  */
    BLUE_FS_CIFS,		/**< A CIFS/SMB type filesystem  */
    BLUE_FS_FILE,		/**< The redirectory filesystem (loopback) */
    BLUE_FS_PIPE,		/**< A Pipe File System  */
    BLUE_FS_MAILSLOT,		/**< A Mailslot File System  */
    BLUE_FS_BROWSE_WORKGROUPS,	/**< A Filesystem for Browsing Workgroups  */
    BLUE_FS_BROWSE_SERVERS,	/**< A Filesystem for Browsing Servers  */
    BLUE_FS_BROWSE_SHARES,	/**< A Filesystem for Browsing Shares  */
    BLUE_FS_BOOKMARKS,		/**< The Bookmarks browser */
    BLUE_FS_WINCE,		/**< A wince type filesystem  */
    BLUE_FS_UNKNOWN,		/**< An unknown Filesystem  */
    BLUE_FS_NUM			/**< The number of filesystems supported  */
  } BLUE_FS_TYPE ;

/** \} */

#endif
