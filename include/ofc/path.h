/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_PATH_H__)
#define __OFC_PATH_H__

#define AUTHENTICATE

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/file.h"
#include "ofc/fs.h"

/**
 * \defgroup Open Files Path Handling Facility
 * \ingroup util
 *
 * The Open Files Path facility provides parsing of local and network paths as well
 * as providing maps of arbitrary paths to a specified destination.
 *
 * This is a key component of Open Files.  The SMB client uses the mapping so
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
 * The Open Files Path facility will parse file names in UNC or SMB URL format.
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
 * The IPC device allows file I/O through the Open Files Pipe file handlers.
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
 * Open Files will query for local master browser on each interface.
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
 * This is exposed to users of the Open Files Path API.  It may be preferable to
 * hide this structure behind additional APIs
 */
typedef OFC_VOID OFC_PATH;


#if defined(__cplusplus)
extern "C"
{
#endif
/**
 * Initialize the Open Files Path Mapping
 *
 * This should only be called by framework_init
 */
OFC_CORE_LIB OFC_VOID
ofc_path_init(OFC_VOID);

OFC_CORE_LIB OFC_VOID
ofc_path_destroy(OFC_VOID);
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
ofc_path_add_mapW(OFC_LPCTSTR lpDevice, OFC_LPCTSTR lpDesc,
                  OFC_PATH *map, OFC_FST_TYPE fsType, OFC_BOOL thumbnail);

OFC_CORE_LIB OFC_BOOL
ofc_path_add_mapA(OFC_LPCSTR lpDevice, OFC_LPCSTR lpDesc,
                  OFC_PATH *map, OFC_FST_TYPE fsType, OFC_BOOL thumbnail);
/**
 * Create a path structure for a path string
 *
 * \param lpFileName
 * path string
 *
 * \returns
 * path structure
 */
OFC_CORE_LIB OFC_PATH *
ofc_path_createW(OFC_LPCTSTR lpFileName);

OFC_CORE_LIB OFC_PATH *
ofc_path_createA(OFC_LPCSTR lpFileName);

OFC_CORE_LIB OFC_PATH *
ofc_path_init_path(OFC_VOID);
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
ofc_path_update(OFC_PATH *path, OFC_PATH *map);
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
ofc_path_printW(OFC_PATH *path, OFC_LPTSTR *filename, OFC_SIZET *rem);

OFC_CORE_LIB OFC_SIZET
ofc_path_printA(OFC_PATH *path, OFC_LPSTR *filename, OFC_SIZET *rem);

/**
 * Construct a URL suitable for passing into Open File APIs as the
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
ofc_path_make_urlW(OFC_LPTSTR *filename,
                   OFC_SIZET *rem,
                   OFC_LPCTSTR username,
                   OFC_LPCTSTR password,
                   OFC_LPCTSTR domain,
                   OFC_LPCTSTR server,
                   OFC_LPCTSTR share,
                   OFC_LPCTSTR path,
                   OFC_LPCTSTR file);

OFC_CORE_LIB OFC_SIZET
ofc_path_make_urlA(OFC_LPSTR *filename,
                   OFC_SIZET *rem,
                   OFC_LPCSTR username,
                   OFC_LPCSTR password,
                   OFC_LPCSTR domain,
                   OFC_LPCSTR server,
                   OFC_LPCSTR share,
                   OFC_LPCSTR path,
                   OFC_LPCSTR file);

/**
 * Delete a path structure
 *
 * \param path
 * The path structure to delete
 */
OFC_CORE_LIB OFC_VOID
ofc_path_delete(OFC_PATH *path);
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
ofc_path_mapW(OFC_LPCTSTR lpFileName, OFC_LPTSTR *lppMappedName,
              OFC_FST_TYPE *filesystem);

OFC_CORE_LIB OFC_VOID
ofc_path_mapA(OFC_LPCSTR lpFileName, OFC_LPSTR *lppMappedName,
              OFC_FST_TYPE *filesystem);

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
OFC_CORE_LIB OFC_VOID
ofc_root_path_mapW(OFC_LPCTSTR lpRootPath, OFC_LPTSTR *lppMappedPath,
          OFC_FS_TYPE *filesystem) ;
OFC_CORE_LIB OFC_VOID
ofc_root_path_mapA(OFC_LPCSTR lpRootPath, OFC_LPSTR *lppMappedPath,
          OFC_FS_TYPE *filesystem) ;
#endif
/**
 * Delete a map
 *
 * \param lpVirtual
 * Virtual path of map to delete
 */
OFC_CORE_LIB OFC_VOID
ofc_path_delete_mapW(OFC_LPCTSTR lpVirtual);

OFC_CORE_LIB OFC_VOID
ofc_path_delete_mapA(OFC_LPCSTR lpVirtual);
/**
 * Find a map for a path
 *
 * \param lpDevice
 * The device string
 *
 * \returns
 * The map for the path
 */
OFC_CORE_LIB OFC_PATH *
ofc_path_map_deviceW(OFC_LPCTSTR lpDevice);

OFC_CORE_LIB OFC_PATH *
ofc_path_map_deviceA(OFC_LPCSTR lpDevice);


#if defined(AUTHENTICATE)
OFC_CORE_LIB OFC_VOID
ofc_path_update_credentialsW(OFC_LPCTSTR filename, OFC_LPCTSTR username,
                             OFC_LPCTSTR password, OFC_LPCTSTR domain);

OFC_CORE_LIB OFC_VOID
ofc_path_update_credentialsA(OFC_LPCSTR filename, OFC_LPCSTR username,
                             OFC_LPCSTR password, OFC_LPCSTR domain);

#else
OFC_CORE_LIB OFC_VOID
ofc_path_update_credentialsW(OFC_PATH *path, OFC_LPCTSTR username,
                OFC_LPCTSTR password, OFC_LPCTSTR domain) ;
OFC_CORE_LIB OFC_VOID
ofc_path_update_credentialsA(OFC_PATH *path, OFC_LPCSTR username,
                OFC_LPCSTR password, OFC_LPCSTR domain) ;
#endif

OFC_BOOL ofc_path_is_wild(OFC_LPCTSTR dir);

OFC_CORE_LIB OFC_VOID
ofc_path_get_rootW(OFC_CTCHAR *lpFileName, OFC_TCHAR **lpRootName,
                   OFC_FST_TYPE *filesystem);

OFC_CORE_LIB OFC_VOID
ofc_path_get_rootA(OFC_CCHAR *lpFileName, OFC_CHAR **lpRootName,
                   OFC_FST_TYPE *filesystem);

OFC_CORE_LIB OFC_VOID
ofc_path_get_mapW(OFC_INT idx, OFC_LPCTSTR *lpDevice,
                  OFC_LPCTSTR *lpDesc, OFC_PATH **map,
                  OFC_BOOL *thumbnail);

OFC_CORE_LIB OFC_PATH *ofc_map_path(OFC_LPCTSTR lpFileName,
                                    OFC_LPTSTR *lppMappedName);

OFC_CORE_LIB OFC_BOOL ofc_path_remote(OFC_PATH *path);

OFC_CORE_LIB OFC_VOID ofc_path_free_server(OFC_PATH *path);

OFC_CORE_LIB OFC_LPCTSTR ofc_path_server(OFC_PATH *path);

OFC_CORE_LIB OFC_INT ofc_path_port(OFC_PATH *path);

OFC_CORE_LIB OFC_VOID ofc_path_set_port(OFC_PATH *_path, OFC_INT port);

OFC_CORE_LIB OFC_VOID
ofc_path_set_server(OFC_PATH *path, OFC_LPTSTR server);

OFC_CORE_LIB OFC_VOID
ofc_path_set_share(OFC_PATH *_path, OFC_LPCTSTR share);

OFC_CORE_LIB OFC_VOID
ofc_path_set_filename(OFC_PATH *_path, OFC_LPCTSTR filename);

OFC_CORE_LIB OFC_BOOL
ofc_path_server_cmp(OFC_PATH *path, OFC_LPCTSTR server);

OFC_CORE_LIB OFC_BOOL
ofc_path_port_cmp(OFC_PATH *_path, OFC_UINT16 port);

OFC_CORE_LIB OFC_LPCTSTR ofc_path_share(OFC_PATH *path);

OFC_CORE_LIB OFC_BOOL
ofc_path_share_cmp(OFC_PATH *path, OFC_LPCTSTR share);

OFC_CORE_LIB OFC_LPCTSTR ofc_path_username(OFC_PATH *path);

OFC_CORE_LIB OFC_VOID ofc_path_set_username(OFC_PATH *_path,
                                            OFC_LPCTSTR username);

OFC_CORE_LIB OFC_BOOL
ofc_path_username_cmp(OFC_PATH *path, OFC_LPCTSTR username);

OFC_CORE_LIB OFC_LPCTSTR ofc_path_password(OFC_PATH *path);

OFC_CORE_LIB OFC_VOID ofc_path_set_password(OFC_PATH *_path,
                                            OFC_LPCTSTR password);

OFC_CORE_LIB OFC_BOOL
ofc_path_password_cmp(OFC_PATH *path, OFC_LPCTSTR password);

OFC_CORE_LIB OFC_LPCTSTR ofc_path_domain(OFC_PATH *path);

OFC_CORE_LIB OFC_VOID ofc_path_set_domain(OFC_PATH *_path,
                                          OFC_LPCTSTR domain);

OFC_CORE_LIB OFC_LPCTSTR ofc_path_device(OFC_PATH *path);

OFC_CORE_LIB OFC_VOID ofc_path_free_device(OFC_PATH *path);

OFC_CORE_LIB OFC_VOID ofc_path_free_usernane(OFC_PATH *path);

OFC_CORE_LIB OFC_VOID ofc_path_free_password(OFC_PATH *path);

OFC_CORE_LIB OFC_VOID ofc_path_free_domain(OFC_PATH *path);

OFC_CORE_LIB OFC_VOID
ofc_path_promote_dirs(OFC_PATH *path, OFC_UINT num_dirs);

OFC_CORE_LIB OFC_VOID ofc_path_set_local(OFC_PATH *_path);

OFC_CORE_LIB OFC_VOID ofc_path_set_remote(OFC_PATH *_path);

OFC_CORE_LIB OFC_VOID ofc_path_set_relative(OFC_PATH *path);

OFC_CORE_LIB OFC_VOID ofc_path_set_absolute(OFC_PATH *path);

OFC_CORE_LIB OFC_BOOL ofc_path_absolute(OFC_PATH *path);

OFC_CORE_LIB OFC_FST_TYPE ofc_path_type(OFC_PATH *path);

OFC_CORE_LIB OFC_VOID
ofc_path_set_type(OFC_PATH *path, OFC_FST_TYPE fstype);

OFC_CORE_LIB OFC_LPCTSTR ofc_path_filename(OFC_PATH *path);

OFC_CORE_LIB OFC_VOID ofc_path_free_filename(OFC_PATH *path);

OFC_CORE_LIB OFC_INT ofc_path_num_dirs(OFC_PATH *path);

OFC_CORE_LIB OFC_LPCTSTR ofc_path_dir(OFC_PATH *path, OFC_UINT ix);

OFC_CORE_LIB OFC_VOID ofc_path_free_dirs(OFC_PATH *path);

OFC_CORE_LIB OFC_VOID ofc_path_debug(OFC_PATH *path);

OFC_CORE_LIB OFC_VOID init_workgroups(OFC_VOID);

OFC_CORE_LIB OFC_VOID destroy_workgroups(OFC_VOID);

OFC_CORE_LIB OFC_VOID update_workgroup(OFC_LPCTSTR workgroup);

OFC_CORE_LIB OFC_VOID remove_workgroup(OFC_LPCTSTR workgroup);

OFC_CORE_LIB OFC_BOOL lookup_workgroup(OFC_LPCTSTR workgroup);

#if defined(__cplusplus)
}
#endif

#if defined(OFC_UNICODE_API)
#define ofc_path_add_map ofc_path_add_mapW
#define ofc_path_create ofc_path_createW
#define ofc_path_print ofc_path_printW
#define ofc_path_make_url ofc_path_make_urlW
#define ofc_path_map ofc_path_mapW
#define ofc_path_delete_map ofc_path_delete_mapW
#define ofc_path_map_device ofc_path_map_deviceW
#define ofc_path_update_credentials ofc_path_update_credentialsW
#define ofc_path_get_root ofc_path_get_rootW
#else
#define ofc_path_add_map ofc_path_add_mapA
#define ofc_path_create ofc_path_createA
#define ofc_path_print ofc_path_printA
#define ofc_path_make_url ofc_path_make_urlA
#define ofc_path_map ofc_path_mapA
#define ofc_path_delete_map ofc_path_delete_mapA
#define ofc_path_map_device ofc_path_map_deviceA
#define ofc_path_update_credentials ofc_path_update_credentialsA
#define ofc_path_get_root ofc_path_get_rootA
#endif

/** \} */
#endif
