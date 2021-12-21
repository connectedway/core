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
typedef BLUE_VOID BLUE_PATH ;


#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Initialize the Blue Path Mapping
   *
   * This should only be called by BlueInit
   */
  BLUE_CORE_LIB BLUE_VOID 
  BluePathInit (BLUE_VOID) ;
  BLUE_CORE_LIB BLUE_VOID 
  BluePathDestroy (BLUE_VOID);
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
   * BLUE_TRUE if success, BLUE_FALSE otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BluePathAddMapW (BLUE_LPCTSTR lpDevice, BLUE_LPCTSTR lpDesc, 
		   BLUE_PATH *map, BLUE_FS_TYPE fsType, BLUE_BOOL thumbnail) ;
  BLUE_CORE_LIB BLUE_BOOL 
  BluePathAddMapA (BLUE_LPCSTR lpDevice, BLUE_LPCSTR lpDesc,
		   BLUE_PATH *map, BLUE_FS_TYPE fsType, BLUE_BOOL thumbnail) ;
  /**
   * Create a path structure for a path string
   *
   * \param lpFileName
   * path string
   *
   * \returns
   * path structure
   */
  BLUE_CORE_LIB BLUE_PATH *
  BluePathCreateW (BLUE_LPCTSTR lpFileName) ;
  BLUE_CORE_LIB BLUE_PATH *
  BluePathCreateA (BLUE_LPCSTR lpFileName) ;

  BLUE_CORE_LIB BLUE_PATH * 
  BluePathInitPath (BLUE_VOID) ;
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
  BLUE_CORE_LIB BLUE_VOID 
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
  BLUE_CORE_LIB BLUE_SIZET 
  BluePathPrintW (BLUE_PATH *path, BLUE_LPTSTR *filename, BLUE_SIZET *rem) ;
  BLUE_CORE_LIB BLUE_SIZET 
  BluePathPrintA (BLUE_PATH *path, BLUE_LPSTR *filename, BLUE_SIZET *rem) ;

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
   * times the sizeof (BLUE_TCHAR).  If the Ascii version of the routine is 
   * called, the buffer size should be big enough to hold len+1 times the
   * size of a character, sizeof (BLUE_CHAR).  The extra one in this 
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
  BLUE_CORE_LIB BLUE_SIZET 
  BluePathMakeURLW (BLUE_LPTSTR *filename,
		    BLUE_SIZET *rem,
		    BLUE_LPCTSTR username,
		    BLUE_LPCTSTR password,
		    BLUE_LPCTSTR domain,
		    BLUE_LPCTSTR server,
		    BLUE_LPCTSTR share,
		    BLUE_LPCTSTR path,
		    BLUE_LPCTSTR file) ;
  
  BLUE_CORE_LIB BLUE_SIZET 
  BluePathMakeURLA (BLUE_LPSTR *filename,
		    BLUE_SIZET *rem,
		    BLUE_LPCSTR username,
		    BLUE_LPCSTR password,
		    BLUE_LPCSTR domain,
		    BLUE_LPCSTR server,
		    BLUE_LPCSTR share,
		    BLUE_LPCSTR path,
		    BLUE_LPCSTR file) ;

  /**
   * Delete a path structure
   *
   * \param path
   * The path structure to delete
   */
  BLUE_CORE_LIB BLUE_VOID 
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
  BLUE_CORE_LIB BLUE_VOID 
  BluePathMapW (BLUE_LPCTSTR lpFileName, BLUE_LPTSTR *lppMappedName,
		BLUE_FS_TYPE *filesystem) ;
  BLUE_CORE_LIB BLUE_VOID 
  BluePathMapA (BLUE_LPCSTR lpFileName, BLUE_LPSTR *lppMappedName,
		BLUE_FS_TYPE *filesystem) ;
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
  BlueRootPathMapA (BLUE_LPCSTR lpRootPath, BLUE_LPSTR *lppMappedPath,
		    BLUE_FS_TYPE *filesystem) ;
#endif
  /**
   * Delete a map
   *
   * \param lpVirtual
   * Virtual path of map to delete
   */
  BLUE_CORE_LIB BLUE_VOID 
  BluePathDeleteMapW (BLUE_LPCTSTR lpVirtual) ;
  BLUE_CORE_LIB BLUE_VOID 
  BluePathDeleteMapA (BLUE_LPCSTR lpVirtual) ;
  /**
   * Find a map for a path
   *
   * \param lpDevice
   * The device string
   *
   * \returns
   * The map for the path
   */
  BLUE_CORE_LIB BLUE_PATH *
  BluePathMapDeviceW (BLUE_LPCTSTR lpDevice) ;
  BLUE_CORE_LIB BLUE_PATH *
  BluePathMapDeviceA (BLUE_LPCSTR lpDevice) ;


#if defined(AUTHENTICATE)
  BLUE_CORE_LIB BLUE_VOID 
  BluePathUpdateCredentialsW (BLUE_LPCTSTR filename, BLUE_LPCTSTR username,
			      BLUE_LPCTSTR password, BLUE_LPCTSTR domain) ;
  BLUE_CORE_LIB BLUE_VOID 
  BluePathUpdateCredentialsA (BLUE_LPCSTR filename, BLUE_LPCSTR username,
			      BLUE_LPCSTR password, BLUE_LPCSTR domain) ;
#else
  BLUE_CORE_LIB BLUE_VOID 
  BluePathUpdateCredentialsW (BLUE_PATH *path, BLUE_LPCTSTR username,
			      BLUE_LPCTSTR password, BLUE_LPCTSTR domain) ;
  BLUE_CORE_LIB BLUE_VOID 
  BluePathUpdateCredentialsA (BLUE_PATH *path, BLUE_LPCSTR username,
			      BLUE_LPCSTR password, BLUE_LPCSTR domain) ;
#endif

  BLUE_BOOL BluePathIsWild (BLUE_LPCTSTR dir) ;

  BLUE_CORE_LIB BLUE_VOID
  BluePathGetRootW (BLUE_CTCHAR *lpFileName, BLUE_TCHAR **lpRootName,
		    BLUE_FS_TYPE *filesystem) ;
  BLUE_CORE_LIB BLUE_VOID
  BluePathGetRootA (BLUE_CCHAR *lpFileName, BLUE_CHAR **lpRootName,
    		    BLUE_FS_TYPE *filesystem) ;

  BLUE_CORE_LIB BLUE_VOID
  BluePathGetMapW (BLUE_INT idx, BLUE_LPCTSTR *lpDevice, 
		   BLUE_LPCTSTR *lpDesc, BLUE_PATH **map,
		   BLUE_BOOL *thumbnail) ;

  BLUE_CORE_LIB BLUE_PATH *BlueMapPath (BLUE_LPCTSTR lpFileName,
					BLUE_LPTSTR *lppMappedName) ;
  BLUE_CORE_LIB BLUE_BOOL BluePathRemote (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_VOID BluePathFreeServer (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_LPCTSTR BluePathServer (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_INT BluePathPort (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_VOID BluePathSetPort (BLUE_PATH *_path, BLUE_INT port) ;
  BLUE_CORE_LIB BLUE_VOID 
  BluePathSetServer (BLUE_PATH *path, BLUE_LPTSTR server) ;
  BLUE_CORE_LIB BLUE_VOID
  BluePathSetShare (BLUE_PATH *_path, BLUE_LPCTSTR share) ;
  BLUE_CORE_LIB BLUE_VOID
  BluePathSetFilename (BLUE_PATH *_path, BLUE_LPCTSTR filename) ;
  BLUE_CORE_LIB BLUE_BOOL 
  BluePathServerCmp (BLUE_PATH *path, BLUE_LPCTSTR server) ;
  BLUE_CORE_LIB BLUE_BOOL 
  BluePathPortCmp (BLUE_PATH *_path, BLUE_UINT16 port) ;
  BLUE_CORE_LIB BLUE_LPCTSTR BluePathShare (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_BOOL
  BluePathShareCmp (BLUE_PATH *path, BLUE_LPCTSTR share) ;
  BLUE_CORE_LIB BLUE_LPCTSTR BluePathUsername (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_VOID BluePathSetUsername (BLUE_PATH *_path, 
					       BLUE_LPCTSTR username) ;
  BLUE_CORE_LIB BLUE_BOOL 
  BluePathUsernameCmp (BLUE_PATH *path, BLUE_LPCTSTR username) ;
  BLUE_CORE_LIB BLUE_LPCTSTR BluePathPassword (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_VOID BluePathSetPassword (BLUE_PATH *_path, 
					       BLUE_LPCTSTR password) ;
  BLUE_CORE_LIB BLUE_BOOL 
  BluePathPasswordCmp (BLUE_PATH *path, BLUE_LPCTSTR password) ;
  BLUE_CORE_LIB BLUE_LPCTSTR BluePathDomain (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_VOID BluePathSetDomain (BLUE_PATH *_path, 
					     BLUE_LPCTSTR domain) ;
  BLUE_CORE_LIB BLUE_LPCTSTR BluePathDevice (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_VOID BluePathFreeDevice (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_VOID BluePathFreeUsername (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_VOID BluePathFreePassword (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_VOID BluePathFreeDomain (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_VOID 
  BluePathPromoteDirs (BLUE_PATH *path, BLUE_UINT num_dirs) ;
  BLUE_CORE_LIB BLUE_VOID BluePathSetLocal (BLUE_PATH *_path) ;
  BLUE_CORE_LIB BLUE_VOID BluePathSetRemote (BLUE_PATH *_path) ;
  BLUE_CORE_LIB BLUE_VOID BluePathSetRelative (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_VOID BluePathSetAbsolute (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_BOOL BluePathAbsolute (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_FS_TYPE BluePathType (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_VOID 
  BluePathSetType (BLUE_PATH *path, BLUE_FS_TYPE fstype) ;
  BLUE_CORE_LIB BLUE_LPCTSTR BluePathFilename (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_VOID BluePathFreeFilename (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_INT BluePathNumDirs (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_LPCTSTR BluePathDir (BLUE_PATH *path, BLUE_UINT ix) ;
  BLUE_CORE_LIB BLUE_VOID BluePathFreeDirs (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_VOID BluePathDebug (BLUE_PATH *path) ;
  BLUE_CORE_LIB BLUE_VOID InitWorkgroups (BLUE_VOID) ;
  BLUE_CORE_LIB BLUE_VOID DestroyWorkgroups (BLUE_VOID);
  BLUE_CORE_LIB BLUE_VOID UpdateWorkgroup (BLUE_LPCTSTR workgroup) ;
  BLUE_CORE_LIB BLUE_VOID RemoveWorkgroup (BLUE_LPCTSTR workgroup) ;
  BLUE_CORE_LIB BLUE_BOOL LookupWorkgroup (BLUE_LPCTSTR workgroup) ;

#if defined(__cplusplus)
}
#endif

#if defined(BLUE_PARAM_UNICODE_API)
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
