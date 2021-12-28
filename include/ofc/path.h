/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__BLUE_PATH_H__)
#define __BLUE_PATH_H__

#define AUTHENTICATE

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/file.h"
#include "ofc/fs.h"

/**
 * \defgroup BluePath Path Handling Facility
 * \ingroup BlueUtil
 *
 * The Blue Path facility provides parsing of local and network paths as well
 * as providing maps of arbitrary paths to a specified destination.
 *
 * This is a key component of Blue Share.  The CIFS client uses the mapping so
 * that applications don't always have to use either UNC formatted names or
 * SMB URLs.  Administrators can map drive letters to particular network paths
 * as is typical in a Win32 environment, or they can map local filesystem
 * paths to a network path as is typpical in a Unix Mount.
 *
 * Each destination is specified with some target path as well as a file system
 * type.  This is used by the redirector to understand the file system handler
 * to use to service a particular file request.  This is used by the CIFS
 * server to translate a share path to a particular file system API and target
 * path.
 *
 * The Blue Path facility will parse file names in UNC or SMB URL format.
 * The follwing syntax is accepted:
 *
 * Universal Naming Convention:
 * 
\verbatim
\\[username[:password[:domain]]@]server[:port]\share\path\file 
\endverbatim
 * 
 * To specify credentials, modify the server portion as follows:
 *
 * username:password:domain@server
 *
 * The SMB URL format is:
 *
\verbatim
smb://[username[:passowrd[:domain]]@]/server/share/path/file
\endverbatim
 *
 * The IPC device allows file I/O through the Blue Share Pipe file handlers.
 *
\verbatim
IPC:\pipename
\endverbatim
 *
 * The WORKGROUPS device allows browsing of workgroups
 *
\verbatim
WORKGROUPS:[\\lmb]
\endverbatim
 *
 * where lmb is the node name of the local master browser.  If not specified
 * Blue Share will query for local master browser on each interface.
 *
 * The SERVERS device allows browsing of servers in a workgroup
 *
\verbatim
SERVERS:[\\lmb]\workgroup
\endverbatim
 *
 * where lmb is as defined for WORKGROUPS and workgroup is the workgroup
 * to browser for servers
 *
 * The SHARES device allows browsing of shares on a server
 *
\verbatim
SHARES:\\server
\endverbatim
 *
 * where server is the server to browse for shares on.
 */

/** \{ */

/**
 * The Internal Represenation of a Path
 *
 * This is exposed to users of the Blue Path API.  It may be preferable to
 * hide this structure behind additional APIs
 */
typedef OFC_VOID BLUE_PATH ;


#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Initialize the Blue Path Mapping
   *
   * This should only be called by BlueInit
   */
  OFC_CORE_LIB OFC_VOID
  BluePathInit (OFC_VOID) ;
  OFC_CORE_LIB OFC_VOID
  BluePathDestroy (OFC_VOID);
  /**
   * Add a map for a virtual path
   *
   * \param lpDevice
   * The Device Name to use for the path
   *
   * \param map
   * path to use for translating virtual path
   *
   * \param fsType
   * File System Handler to use for the map
   *
   * \returns 
   * OFC_TRUE if success, OFC_FALSE otherwise
   */
  OFC_CORE_LIB OFC_BOOL
  BluePathAddMapW (OFC_LPCTSTR lpDevice, OFC_LPCTSTR lpDesc,
                   BLUE_PATH *map, OFC_FST_TYPE fsType, OFC_BOOL thumbnail) ;
  OFC_CORE_LIB OFC_BOOL
  BluePathAddMapA (OFC_LPCSTR lpDevice, OFC_LPCSTR lpDesc,
                   BLUE_PATH *map, OFC_FST_TYPE fsType, OFC_BOOL thumbnail) ;
  /**
   * Create a path structure for a path string
   *
   * \param lpFileName
   * path string
   *
   * \returns
   * path structure
   */
  OFC_CORE_LIB BLUE_PATH *
  BluePathCreateW (OFC_LPCTSTR lpFileName) ;
  OFC_CORE_LIB BLUE_PATH *
  BluePathCreateA (OFC_LPCSTR lpFileName) ;

  OFC_CORE_LIB BLUE_PATH *
  BluePathInitPath (OFC_VOID) ;
  /**
   * Apply a map to a path
   *
   * The routine will assentially update the path pointed to by the first
   * argument with info from the map supplied in the second argument.  This
   * means that the device, server, credentials, share, and parent directories
   * of the map are simply prepended to the path
   *
   * \param path
   * Path that needs to be updated
   *
   * \param map
   * map to update the path with
   */
  OFC_CORE_LIB OFC_VOID
  BluePathUpdate (BLUE_PATH *path, BLUE_PATH *map) ;
  /**
   * Print a path structure to a string
   *
   * \param path
   * path to print
   *
   * \param filename
   * string to print path to
   *
   * \param rem
   * pointer to number of bytes available in destination filename
   * This is updated to account for any characters output
   *
   * \returns
   * Number of bytes printed
   */
  OFC_CORE_LIB OFC_SIZET
  BluePathPrintW (BLUE_PATH *path, OFC_LPTSTR *filename, OFC_SIZET *rem) ;
  OFC_CORE_LIB OFC_SIZET
  BluePathPrintA (BLUE_PATH *path, OFC_LPSTR *filename, OFC_SIZET *rem) ;

  /**
   * Construct a URL suitable for passing into Blue File APIs as the 
   * remote escaped file name.
   *
   * This routine will fill in a buffer provided by the application with the
   * URL.  Since it is difficult to predict the size of the buffer required
   * to hold the URL, this routine will typically be called twice in a row.
   * The first time it is called, it is called with a "rem" value of 0 and
   * a NULL as the pointer to the buffer.  The routine will return the number
   * of characters that would have been put into the string assuming the
   * buffer was big enough (len).  The application then then allocate a 
   * buffer large enough to hold the URL using the returned number of 
   * characters in the URL (len).
   * On the second call to the routine, If the Unicode version of the 
   * routine is called, the buffer size should be big enough to hold len + 1 
   * times the sizeof (OFC_TCHAR).  If the Ascii version of the routine is
   * called, the buffer size should be big enough to hold len+1 times the
   * size of a character, sizeof (OFC_CHAR).  The extra one in this
   * calculation is required if you want the string to be null terminated.
   *
   * \param filename
   * Pointer to a pointer to the buffer to place URL in.  The API specifies
   * a pointer to a pointer so that the routine can update the pointer to the
   * end of the string.
   *
   * \param rem
   * pointer to the number of remaining characters in the buffer.  The API
   * will update this with the remaining characters that can fit in the 
   * buffer.
   *
   * \param username
   * Pointer to the username to put in the URL.  A NULL pointer will leave
   * the username field empty.
   *
   * \param password
   * Pointer to the password to put in the URL.
   *
   * \param domain
   * Pointer to the domain to put in the URL
   *
   * \param server
   * Pointer to the server to put in the URL
   *
   * \param share
   * Pointer to the share to put in the URL
   *
   * \param path
   * Pointer to the directory path to put in the URL.  Subdirectories should
   * be encoded with the directory terminator ("\\" or "/") between each
   * directory level
   *
   * \param file
   * The file portion of the URL
   *
   * \returns
   * Number of bytes in the URL whether they fit in the output buffer or not.
   * The routine also updates the filename pointer to point to the next
   * location in the buffer to append additional text, and the number of
   * characters remaining in the buffer is also returned.
   */
  OFC_CORE_LIB OFC_SIZET
  BluePathMakeURLW (OFC_LPTSTR *filename,
                    OFC_SIZET *rem,
                    OFC_LPCTSTR username,
                    OFC_LPCTSTR password,
                    OFC_LPCTSTR domain,
                    OFC_LPCTSTR server,
                    OFC_LPCTSTR share,
                    OFC_LPCTSTR path,
                    OFC_LPCTSTR file) ;
  
  OFC_CORE_LIB OFC_SIZET
  BluePathMakeURLA (OFC_LPSTR *filename,
                    OFC_SIZET *rem,
                    OFC_LPCSTR username,
                    OFC_LPCSTR password,
                    OFC_LPCSTR domain,
                    OFC_LPCSTR server,
                    OFC_LPCSTR share,
                    OFC_LPCSTR path,
                    OFC_LPCSTR file) ;

  /**
   * Delete a path structure
   *
   * \param path
   * The path structure to delete
   */
  OFC_CORE_LIB OFC_VOID
  BluePathDelete (BLUE_PATH *path) ;
  /**
   * Map a local path to a target path
   *
   * \param lpFileName
   * The local path (virtual path used by applications or CIFS Server)
   *
   * \param lppMappedName
   * The target path (relative to the target file system type)
   *
   * \param filesystem
   * file system type
   */
  OFC_CORE_LIB OFC_VOID
  BluePathMapW (OFC_LPCTSTR lpFileName, OFC_LPTSTR *lppMappedName,
                OFC_FST_TYPE *filesystem) ;
  OFC_CORE_LIB OFC_VOID
  BluePathMapA (OFC_LPCSTR lpFileName, OFC_LPSTR *lppMappedName,
                OFC_FST_TYPE *filesystem) ;
#if 0
  /**
   * Map a local root path to a target root path
   * 
   * A Root Path is slightly different then a regular path in that no
   * all directory and file information is suppressed
   *
   * \param lpRootPath
   * The local path (virtual path used by applications or CIFS Server)
   *
   * \param lppMappedPath
   * The target path (relative to the target file system type)
   *
   * \param filesystem
   * file system type
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueRootPathMapW (BLUE_LPCTSTR lpRootPath, BLUE_LPTSTR *lppMappedPath,
		    BLUE_FS_TYPE *filesystem) ;
  BLUE_CORE_LIB BLUE_VOID 
  BlueRootPathMapA (BLUE_LPCSTR lpRootPath, OFC_LPSTR *lppMappedPath,
		    BLUE_FS_TYPE *filesystem) ;
#endif
  /**
   * Delete a map
   *
   * \param lpVirtual
   * Virtual path of map to delete
   */
  OFC_CORE_LIB OFC_VOID
  BluePathDeleteMapW (OFC_LPCTSTR lpVirtual) ;
  OFC_CORE_LIB OFC_VOID
  BluePathDeleteMapA (OFC_LPCSTR lpVirtual) ;
  /**
   * Find a map for a path
   *
   * \param lpDevice
   * The device string
   *
   * \returns
   * The map for the path
   */
  OFC_CORE_LIB BLUE_PATH *
  BluePathMapDeviceW (OFC_LPCTSTR lpDevice) ;
  OFC_CORE_LIB BLUE_PATH *
  BluePathMapDeviceA (OFC_LPCSTR lpDevice) ;


#if defined(AUTHENTICATE)
  OFC_CORE_LIB OFC_VOID
  BluePathUpdateCredentialsW (OFC_LPCTSTR filename, OFC_LPCTSTR username,
                              OFC_LPCTSTR password, OFC_LPCTSTR domain) ;
  OFC_CORE_LIB OFC_VOID
  BluePathUpdateCredentialsA (OFC_LPCSTR filename, OFC_LPCSTR username,
                              OFC_LPCSTR password, OFC_LPCSTR domain) ;
#else
  BLUE_CORE_LIB BLUE_VOID 
  BluePathUpdateCredentialsW (BLUE_PATH *path, BLUE_LPCTSTR username,
			      BLUE_LPCTSTR password, BLUE_LPCTSTR domain) ;
  BLUE_CORE_LIB BLUE_VOID 
  BluePathUpdateCredentialsA (BLUE_PATH *path, BLUE_LPCSTR username,
			      BLUE_LPCSTR password, BLUE_LPCSTR domain) ;
#endif

  OFC_BOOL BluePathIsWild (OFC_LPCTSTR dir) ;

  OFC_CORE_LIB OFC_VOID
  BluePathGetRootW (OFC_CTCHAR *lpFileName, OFC_TCHAR **lpRootName,
                    OFC_FST_TYPE *filesystem) ;
  OFC_CORE_LIB OFC_VOID
  BluePathGetRootA (OFC_CCHAR *lpFileName, OFC_CHAR **lpRootName,
                    OFC_FST_TYPE *filesystem) ;

  OFC_CORE_LIB OFC_VOID
  BluePathGetMapW (OFC_INT idx, OFC_LPCTSTR *lpDevice,
                   OFC_LPCTSTR *lpDesc, BLUE_PATH **map,
                   OFC_BOOL *thumbnail) ;

  OFC_CORE_LIB BLUE_PATH *BlueMapPath (OFC_LPCTSTR lpFileName,
                                       OFC_LPTSTR *lppMappedName) ;
  OFC_CORE_LIB OFC_BOOL BluePathRemote (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_VOID BluePathFreeServer (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_LPCTSTR BluePathServer (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_INT BluePathPort (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_VOID BluePathSetPort (BLUE_PATH *_path, OFC_INT port) ;
  OFC_CORE_LIB OFC_VOID
  BluePathSetServer (BLUE_PATH *path, OFC_LPTSTR server) ;
  OFC_CORE_LIB OFC_VOID
  BluePathSetShare (BLUE_PATH *_path, OFC_LPCTSTR share) ;
  OFC_CORE_LIB OFC_VOID
  BluePathSetFilename (BLUE_PATH *_path, OFC_LPCTSTR filename) ;
  OFC_CORE_LIB OFC_BOOL
  BluePathServerCmp (BLUE_PATH *path, OFC_LPCTSTR server) ;
  OFC_CORE_LIB OFC_BOOL
  BluePathPortCmp (BLUE_PATH *_path, OFC_UINT16 port) ;
  OFC_CORE_LIB OFC_LPCTSTR BluePathShare (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_BOOL
  BluePathShareCmp (BLUE_PATH *path, OFC_LPCTSTR share) ;
  OFC_CORE_LIB OFC_LPCTSTR BluePathUsername (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_VOID BluePathSetUsername (BLUE_PATH *_path,
                                             OFC_LPCTSTR username) ;
  OFC_CORE_LIB OFC_BOOL
  BluePathUsernameCmp (BLUE_PATH *path, OFC_LPCTSTR username) ;
  OFC_CORE_LIB OFC_LPCTSTR BluePathPassword (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_VOID BluePathSetPassword (BLUE_PATH *_path,
                                             OFC_LPCTSTR password) ;
  OFC_CORE_LIB OFC_BOOL
  BluePathPasswordCmp (BLUE_PATH *path, OFC_LPCTSTR password) ;
  OFC_CORE_LIB OFC_LPCTSTR BluePathDomain (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_VOID BluePathSetDomain (BLUE_PATH *_path,
                                           OFC_LPCTSTR domain) ;
  OFC_CORE_LIB OFC_LPCTSTR BluePathDevice (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_VOID BluePathFreeDevice (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_VOID BluePathFreeUsername (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_VOID BluePathFreePassword (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_VOID BluePathFreeDomain (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_VOID
  BluePathPromoteDirs (BLUE_PATH *path, OFC_UINT num_dirs) ;
  OFC_CORE_LIB OFC_VOID BluePathSetLocal (BLUE_PATH *_path) ;
  OFC_CORE_LIB OFC_VOID BluePathSetRemote (BLUE_PATH *_path) ;
  OFC_CORE_LIB OFC_VOID BluePathSetRelative (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_VOID BluePathSetAbsolute (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_BOOL BluePathAbsolute (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_FST_TYPE BluePathType (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_VOID
  BluePathSetType (BLUE_PATH *path, OFC_FST_TYPE fstype) ;
  OFC_CORE_LIB OFC_LPCTSTR BluePathFilename (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_VOID BluePathFreeFilename (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_INT BluePathNumDirs (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_LPCTSTR BluePathDir (BLUE_PATH *path, OFC_UINT ix) ;
  OFC_CORE_LIB OFC_VOID BluePathFreeDirs (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_VOID BluePathDebug (BLUE_PATH *path) ;
  OFC_CORE_LIB OFC_VOID InitWorkgroups (OFC_VOID) ;
  OFC_CORE_LIB OFC_VOID DestroyWorkgroups (OFC_VOID);
  OFC_CORE_LIB OFC_VOID UpdateWorkgroup (OFC_LPCTSTR workgroup) ;
  OFC_CORE_LIB OFC_VOID RemoveWorkgroup (OFC_LPCTSTR workgroup) ;
  OFC_CORE_LIB OFC_BOOL LookupWorkgroup (OFC_LPCTSTR workgroup) ;

#if defined(__cplusplus)
}
#endif

#if defined(OFC_UNICODE_API)
#define BluePathAddMap BluePathAddMapW
#define BluePathCreate BluePathCreateW
#define BluePathPrint BluePathPrintW
#define BluePathMakeURL BluePathMakeURLW
#define BluePathMap BluePathMapW
#define BluePathDeleteMap BluePathDeleteMapW
#define BluePathMapDevice BluePathMapDeviceW
#define BluePathUpdateCredentials BluePathUpdateCredentialsW
#define BluePathGetRoot BluePathGetRootW
#else
#define BluePathAddMap BluePathAddMapA
#define BluePathCreate BluePathCreateA
#define BluePathPrint BluePathPrintA
#define BluePathMakeURL BluePathMakeURLA
#define BluePathMap BluePathMapA
#define BluePathDeleteMap BluePathDeleteMapA
#define BluePathMapDevice BluePathMapDeviceA
#define BluePathUpdateCredentials BluePathUpdateCredentialsA
#define BluePathGetRoot BluePathGetRootA
#endif

/** \} */
#endif
