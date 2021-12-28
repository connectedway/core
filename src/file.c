/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/config.h"
#include "ofc/handle.h"
#include "ofc/queue.h"
#include "ofc/waitq.h"
#include "ofc/libc.h"
#include "ofc/path.h"
#include "ofc/lock.h"
#include "ofc/thread.h"

#include "ofc/file.h"
#include "ofc/fs.h"
#include "ofc/heap.h"

typedef struct _BLUE_FILE_CONTEXT
{
  OFC_HANDLE fsHandle ;
  OFC_FST_TYPE fsType ;
#if defined(OFC_FILE_DEBUG)
  struct _BLUE_FILE_CONTEXT * dbgnext ;
  struct _BLUE_FILE_CONTEXT * dbgprev ;
#if defined(__GHU__)
  BLUE_VOID *caller1 ;
  BLUE_VOID *caller2 ;
  BLUE_VOID *caller3 ;
  BLUE_VOID *caller4 ;
#else
  BLUE_VOID *caller ;
#endif
#endif
  OFC_HANDLE overlappedList ;
} BLUE_FILE_CONTEXT ;

OFC_DWORD OfcLastError ;

#if defined(OFC_FILE_DEBUG)
typedef struct
{
  BLUE_UINT32 Max ;
  BLUE_UINT32 Total ;
  struct _BLUE_FILE_CONTEXT *Allocated ;
} BLUE_FILE_DEBUG ;

static BLUE_FILE_DEBUG BlueFileDebug ;
BLUE_LOCK BlueFileLock ;

static BLUE_VOID 
BlueFileDebugAlloc (struct _BLUE_FILE_CONTEXT *chunk, BLUE_VOID *ret)
{
  /*
   * Put on the allocation queue
   */
  BlueLock (BlueFileLock) ;
  chunk->dbgnext = BlueFileDebug.Allocated ;
  if (BlueFileDebug.Allocated != BLUE_NULL)
    BlueFileDebug.Allocated->dbgprev = chunk ;
  BlueFileDebug.Allocated = chunk ;
  chunk->dbgprev = 0 ;

  BlueFileDebug.Total++ ;
  if (BlueFileDebug.Total >= BlueFileDebug.Max)
    BlueFileDebug.Max = BlueFileDebug.Total ;
  BlueUnlock (BlueFileLock) ;
#if defined(__GNUC__) && defined(BLUE_STACK_TRACE)
  chunk->caller1 = __builtin_return_address(1) ;
  chunk->caller2 = __builtin_return_address(2) ;
  chunk->caller3 = __builtin_return_address(3) ;
  chunk->caller4 = __builtin_return_address(4) ;
#else
  chunk->caller = ret ;
#endif
}

static BLUE_VOID 
BlueFileDebugFree (struct _BLUE_FILE_CONTEXT *chunk)
{
  /*
   * Pull off the allocation queue
   */
  BlueLock (BlueFileLock) ;
  if (chunk->dbgprev != BLUE_NULL)
    chunk->dbgprev->dbgnext = chunk->dbgnext ;
  else
    BlueFileDebug.Allocated = chunk->dbgnext ;
  if (chunk->dbgnext != BLUE_NULL)
    chunk->dbgnext->dbgprev = chunk->dbgprev ;
  BlueFileDebug.Total-- ;
  BlueUnlock (BlueFileLock) ;
}

BLUE_VOID 
BlueFileDebugDump (BLUE_VOID)
{
  struct _BLUE_FILE_CONTEXT *chunk ;

  BlueCprintf ("Total Allocated Files %d, Max Allocated Files %d\n\n",
	       BlueFileDebug.Total, BlueFileDebug.Max) ;

#if defined(__GNUC__)
  BlueCprintf ("%-10s %-10s %-10s %-10s %-10s\n", "Address", 
	       "Caller1", "Caller2", "Caller3", "Caller4") ;

  for (chunk = BlueFileDebug.Allocated ;
       chunk != BLUE_NULL ;
       chunk = chunk->dbgnext)
    {
      BlueCprintf ("%-10p %-10p %-10p %-10p %-10p\n", chunk+1, 
		   chunk->caller1, chunk->caller2, chunk->caller3,
		   chunk->caller4) ;
    }
#else
  BlueCprintf ("%-20s %-20s\n", "Address", "Caller") ;

  for (chunk = BlueFileDebug.Allocated ;
       chunk != BLUE_NULL ;
       chunk = chunk->dbgnext)
    {
      BlueCprintf ("%-20p %-20p\n", chunk+1, chunk->caller) ;
    }
#endif
  BlueCprintf ("\n") ;
}
#endif

static OFC_FST_TYPE MapType (OFC_PATH *path)
{
  OFC_FST_TYPE fstype ;
  OFC_LPCTSTR server ;
  OFC_LPCTSTR share ;
  OFC_BOOL server_wild ;
  OFC_BOOL share_wild ;
  OFC_CTCHAR *p ;

  fstype = ofc_path_type(path) ;

  if (ofc_path_remote(path))
    {
      /*
       * It's a remote path.  What we want to know is if it's for
       * browsing workgroups, servers, or shares.  
       * It'll be browsing workgroups if there is the node is filtered
       * It'll be for servers if the node matches a previously discovered
       * workgroup and the share is filtered.
       * It'll be for shares if the node does not match a previously
       * discovered workgroup and the share is filtered.
       */
      server_wild = OFC_FALSE ;
      share_wild = OFC_FALSE ;

      server = ofc_path_server(path) ;
      if (server == OFC_NULL)
	server_wild = OFC_TRUE ;
      else
	{
	  p = ofc_tstrtok (server, TSTR("*?")) ;
	  if (p != OFC_NULL && *p != TCHAR_EOS)
	    {
	      server_wild = OFC_TRUE ;
	    }
	}

      share = ofc_path_share (path) ;
      if (share == OFC_NULL)
	share_wild = OFC_TRUE ;
      else
	{
	  p = ofc_tstrtok (share, TSTR("*?")) ;
	  if (p != OFC_NULL && *p != TCHAR_EOS)
	    {
	      share_wild = OFC_TRUE ;
	    }
	  else if (ofc_path_dir (path, 0) == OFC_NULL &&
               ofc_path_filename (path) == OFC_NULL)
	    {
	      share_wild = OFC_TRUE ;
	    }
	}

      if (server_wild)
	{
#if defined(OFC_FS_BROWSER)
	  fstype = BLUE_FS_BROWSE_WORKGROUPS ;
#endif
	}
      else if (share_wild && lookup_workgroup (server) == OFC_TRUE)
	{
#if defined(OFC_FS_BROWSER)
	  fstype = BLUE_FS_BROWSE_SERVERS ;
#endif
	}
      else if (share_wild)
	{
#if defined(OFC_FS_BROWSER)
	  fstype = BLUE_FS_BROWSE_SHARES ;
#endif
	}
      else
	{
#if defined(OFC_FS_CIFS)
	  fstype = BLUE_FS_CIFS ;
#endif
	}
    }
  return (fstype) ;
}

OFC_CORE_LIB OFC_HANDLE
OfcCreateFileW (OFC_LPCTSTR lpFileName,
                OFC_DWORD dwDesiredAccess,
                OFC_DWORD dwShareMode,
                OFC_LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                OFC_DWORD dwCreationDisposition,
                OFC_DWORD dwFlagsAndAttributes,
                OFC_HANDLE hTemplateFile)
{
  OFC_LPTSTR lpMappedFileName ;
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_HANDLE retHandle ;
  OFC_HANDLE hMappedTemplateHandle ;

  fileContext = ofc_malloc (sizeof (BLUE_FILE_CONTEXT)) ;

  fileContext->overlappedList = BlueQcreate () ;

#if defined(OFC_FILE_DEBUG)
  BlueFileDebugAlloc (fileContext, RETURN_ADDRESS()) ;
#endif
  ofc_path_mapW (lpFileName, &lpMappedFileName, &fileContext->fsType) ;

  hMappedTemplateHandle = OFC_HANDLE_NULL ;
  if (hTemplateFile != OFC_HANDLE_NULL)
    {
      BLUE_FILE_CONTEXT *templateContext ;
      templateContext = ofc_handle_lock (hTemplateFile) ;
      if (templateContext != OFC_NULL)
	{
	  hMappedTemplateHandle = templateContext->fsHandle ;
	  ofc_handle_unlock (hTemplateFile) ;
	}
    }

  fileContext->fsHandle = OfcFSCreateFile (fileContext->fsType,
                                           lpMappedFileName,
                                           dwDesiredAccess,
                                           dwShareMode,
                                           lpSecurityAttributes,
                                           dwCreationDisposition,
                                           dwFlagsAndAttributes,
                                           hMappedTemplateHandle) ;


  if (fileContext->fsHandle == OFC_HANDLE_NULL ||
      fileContext->fsHandle == OFC_INVALID_HANDLE_VALUE)
    {
      retHandle = fileContext->fsHandle ;
      BlueQdestroy (fileContext->overlappedList) ;
#if defined(OFC_FILE_DEBUG)
      BlueFileDebugFree (fileContext) ;
#endif
      ofc_free (fileContext) ;
    }
  else
    retHandle = ofc_handle_create (OFC_HANDLE_FILE, fileContext) ;

  ofc_free (lpMappedFileName) ;

  return (retHandle) ;
} 

OFC_CORE_LIB OFC_HANDLE
OfcCreateFileA (OFC_LPCSTR lpFileName,
                OFC_DWORD dwDesiredAccess,
                OFC_DWORD dwShareMode,
                OFC_LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                OFC_DWORD dwCreationDisposition,
                OFC_DWORD dwFlagsAndAttributes,
                OFC_HANDLE hTemplateFile)
{
  OFC_TCHAR *lptFileName ;
  OFC_HANDLE ret ;

  lptFileName = ofc_cstr2tstr (lpFileName) ;
  ret = OfcCreateFileW (lptFileName, dwDesiredAccess, dwShareMode,
                        lpSecurityAttributes, dwCreationDisposition,
                        dwFlagsAndAttributes, hTemplateFile) ;
  ofc_free (lptFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcCreateDirectoryW (OFC_LPCTSTR lpPathName,
                     OFC_LPSECURITY_ATTRIBUTES lpSecurityAttr)
{
  OFC_LPTSTR lpMappedPathName ;
  OFC_FST_TYPE type ;
  OFC_BOOL ret ;

  ofc_path_mapW (lpPathName, &lpMappedPathName, &type) ;

  ret = OfcFSCreateDirectory (type, lpMappedPathName, lpSecurityAttr) ;

  ofc_free (lpMappedPathName) ;

  return (ret) ;
}  

OFC_CORE_LIB OFC_BOOL
OfcCreateDirectoryA (OFC_LPCSTR lpPathName,
                     OFC_LPSECURITY_ATTRIBUTES lpSecurityAttr)
{
  OFC_BOOL ret ;
  OFC_TCHAR *lptPathName ;

  lptPathName = ofc_cstr2tstr (lpPathName) ;
  ret = OfcCreateDirectoryW (lptPathName, lpSecurityAttr) ;
  ofc_free (lptPathName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcWriteFile (OFC_HANDLE hFile,
              OFC_LPCVOID lpBuffer,
              OFC_DWORD nNumberOfBytesToWrite,
              OFC_LPDWORD lpNumberOfBytesWritten,
              OFC_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  fileContext = ofc_handle_lock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSWriteFile (fileContext->fsType,
                            fileContext->fsHandle,
                            lpBuffer,
                            nNumberOfBytesToWrite,
                            lpNumberOfBytesWritten,
                            hOverlapped) ;
      ofc_handle_unlock (hFile) ;
    }
  else
    ret = OFC_FALSE ;

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcTransactNamedPipe (OFC_HANDLE hFile,
                      OFC_LPVOID lpInBuffer,
                      OFC_DWORD nInBufferSize,
                      OFC_LPVOID lpOutBuffer,
                      OFC_DWORD nOutBufferSize,
                      OFC_LPDWORD lpBytesRead,
                      OFC_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  fileContext = ofc_handle_lock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSTransactNamedPipe (fileContext->fsType,
                                    fileContext->fsHandle,
                                    lpInBuffer,
                                    nInBufferSize,
                                    lpOutBuffer,
                                    nOutBufferSize,
                                    lpBytesRead,
                                    hOverlapped) ;
      ofc_handle_unlock (hFile) ;
    }
  else
    ret = OFC_FALSE ;

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcCloseHandle (OFC_HANDLE hObject)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;
  OFC_HANDLE hOverlapped ;

  fileContext = ofc_handle_lock (hObject) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSCloseHandle (fileContext->fsType,
                              fileContext->fsHandle) ;
      for (hOverlapped = 
	     (OFC_HANDLE) BlueQfirst (fileContext->overlappedList) ;
           hOverlapped != OFC_HANDLE_NULL ;
	   hOverlapped = 
	     (OFC_HANDLE) BlueQfirst (fileContext->overlappedList))
	{
	  /*
	   * This will also remove it from the overlapped list so first 
	   * also is returning the next one after this has been removed
	   */
	  OfcDestroyOverlapped (hObject, hOverlapped) ;
	}
      BlueQdestroy (fileContext->overlappedList) ;
      ofc_handle_unlock (hObject) ;
      ofc_handle_destroy (hObject) ;
#if defined(OFC_FILE_DEBUG)
      BlueFileDebugFree (fileContext) ;
#endif
      ofc_free (fileContext) ;
    }
  else
    ret = OFC_FALSE ;

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcReadFile (OFC_HANDLE hFile,
             OFC_LPVOID lpBuffer,
             OFC_DWORD nNumberOfBytesToRead,
             OFC_LPDWORD lpNumberOfBytesRead,
             OFC_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  fileContext = ofc_handle_lock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSReadFile (fileContext->fsType,
                           fileContext->fsHandle,
                           lpBuffer,
                           nNumberOfBytesToRead,
                           lpNumberOfBytesRead,
                           hOverlapped) ;
      ofc_handle_unlock (hFile) ;
    }
  else
    ret = OFC_FALSE ;

  return (ret) ;
}

OFC_CORE_LIB OFC_HANDLE
OfcCreateOverlapped (OFC_HANDLE hFile)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_HANDLE ret ;

  ret = OFC_HANDLE_NULL ;
  fileContext = ofc_handle_lock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSCreateOverlapped (fileContext->fsType) ;
      if (ret != OFC_HANDLE_NULL)
	BlueQenqueue (fileContext->overlappedList, (OFC_VOID *) ret) ;
      ofc_handle_unlock (hFile) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
OfcDestroyOverlapped (OFC_HANDLE hFile, OFC_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;

  fileContext = ofc_handle_lock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      BlueQunlink (fileContext->overlappedList, (OFC_VOID *) hOverlapped) ;
      OfcFSDestroyOverlapped (fileContext->fsType, hOverlapped) ;
      ofc_handle_unlock (hFile) ;
    }
}

OFC_CORE_LIB OFC_VOID
OfcSetOverlappedOffset (OFC_HANDLE hFile,
                        OFC_HANDLE hOverlapped,
                        OFC_OFFT offset)
{
  BLUE_FILE_CONTEXT *fileContext ;

  fileContext = ofc_handle_lock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      OfcFSSetOverlappedOffset (fileContext->fsType, hOverlapped, offset) ;
      ofc_handle_unlock (hFile) ;
    }
}

OFC_CORE_LIB OFC_BOOL
OfcGetOverlappedResult (OFC_HANDLE hFile,
                        OFC_HANDLE hOverlapped,
                        OFC_LPDWORD lpNumberOfBytesTransferred,
                        OFC_BOOL bWait)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  fileContext = ofc_handle_lock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSGetOverlappedResult (fileContext->fsType,
                                      fileContext->fsHandle,
                                      hOverlapped,
                                      lpNumberOfBytesTransferred,
                                      bWait) ;
      ofc_handle_unlock (hFile) ;
    }
  else
    ret = OFC_FALSE ;

  return (ret) ;
}


OFC_CORE_LIB OFC_BOOL
OfcDeleteFileW (OFC_LPCTSTR lpFileName)
{
  OFC_LPTSTR lpMappedFileName ;
  OFC_FST_TYPE type ;
  OFC_BOOL ret ;

  ofc_path_mapW (lpFileName, &lpMappedFileName, &type) ;

  ret = OfcFSDeleteFile (type, lpMappedFileName) ;

  ofc_free (lpMappedFileName) ;

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcDeleteFileA (OFC_LPCSTR lpFileName)
{
  OFC_BOOL ret ;
  OFC_TCHAR *lptFileName ;

  lptFileName = ofc_cstr2tstr (lpFileName) ;
  ret = OfcDeleteFileW (lptFileName) ;
  ofc_free (lptFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcRemoveDirectoryW (OFC_LPCTSTR lpPathName)
{
  OFC_LPTSTR lpMappedPathName ;
  OFC_FST_TYPE type ;
  OFC_BOOL ret ;

  ofc_path_mapW (lpPathName, &lpMappedPathName, &type) ;

  ret = OfcFSRemoveDirectory (type, lpMappedPathName) ;

  ofc_free (lpMappedPathName) ;

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcRemoveDirectoryA (OFC_LPCSTR lpPathName)
{
  OFC_BOOL ret ;
  OFC_TCHAR *lptPathName ;

  lptPathName = ofc_cstr2tstr (lpPathName) ;
  ret = OfcRemoveDirectoryW(lptPathName) ;
  ofc_free (lptPathName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_HANDLE
OfcFindFirstFileW (OFC_LPCTSTR lpFileName,
                   OFC_LPWIN32_FIND_DATAW lpFindFileData,
                   OFC_BOOL *more)
{
  OFC_LPTSTR lpMappedFileName ;
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_HANDLE retHandle ;
  OFC_PATH *path ;


  retHandle = OFC_INVALID_HANDLE_VALUE ;
  fileContext = ofc_malloc (sizeof (BLUE_FILE_CONTEXT)) ;
  if (fileContext != OFC_NULL)
    {
#if defined(OFC_FILE_DEBUG)
      BlueFileDebugAlloc (fileContext, RETURN_ADDRESS()) ;
#endif
      path = ofc_map_path (lpFileName, &lpMappedFileName) ;

      fileContext->fsType = MapType (path) ;

      fileContext->fsHandle = OfcFSFindFirstFile (fileContext->fsType,
                                                  lpMappedFileName,
                                                  lpFindFileData,
                                                  more) ;
      if (fileContext->fsHandle == OFC_HANDLE_NULL ||
          fileContext->fsHandle == OFC_INVALID_HANDLE_VALUE)
        {
	  if (fileContext->fsType == OFC_FST_BROWSE_SERVERS &&
          ofc_path_server(path) != OFC_NULL)
	    remove_workgroup (ofc_path_server(path)) ;

          retHandle = fileContext->fsHandle ;
#if defined(OFC_FILE_DEBUG)
	  BlueFileDebugFree (fileContext) ;
#endif
          ofc_free (fileContext) ;
        }
      else
	{
	  retHandle = ofc_handle_create (OFC_HANDLE_FILE, fileContext) ;
	  if (fileContext->fsType == OFC_FST_BROWSE_WORKGROUPS)
	    update_workgroup (lpFindFileData->cFileName) ;
	}
      ofc_free (lpMappedFileName) ;
      ofc_path_delete (path) ;
    }
  return (retHandle) ;
}

OFC_CORE_LIB OFC_VOID
BlueFileFindDataW2FindDataA (OFC_LPWIN32_FIND_DATAA lpaFindFileData,
                             OFC_LPWIN32_FIND_DATAW lpwFindFileData)
{
  OFC_INT i ;

  lpaFindFileData->dwFileAttributes = lpwFindFileData->dwFileAttributes ;
  lpaFindFileData->ftCreateTime.dwLowDateTime = 
    lpwFindFileData->ftCreateTime.dwLowDateTime ;
  lpaFindFileData->ftCreateTime.dwHighDateTime =
    lpwFindFileData->ftCreateTime.dwHighDateTime ;
  lpaFindFileData->ftLastAccessTime.dwLowDateTime = 
    lpwFindFileData->ftLastAccessTime.dwLowDateTime ;
  lpaFindFileData->ftLastAccessTime.dwHighDateTime =
    lpwFindFileData->ftLastAccessTime.dwHighDateTime ;
  lpaFindFileData->ftLastWriteTime.dwLowDateTime = 
    lpwFindFileData->ftLastWriteTime.dwLowDateTime ;
  lpaFindFileData->ftLastWriteTime.dwHighDateTime =
    lpwFindFileData->ftLastWriteTime.dwHighDateTime ;
  lpaFindFileData->nFileSizeHigh = lpwFindFileData->nFileSizeHigh ;
  lpaFindFileData->nFileSizeLow = lpwFindFileData->nFileSizeLow ;
  lpaFindFileData->dwReserved0 = lpwFindFileData->dwReserved0 ;
  lpaFindFileData->dwReserved1 = lpwFindFileData->dwReserved1 ;
  for (i = 0 ; i < OFC_MAX_PATH ; i++)
    lpaFindFileData->cFileName[i] = 
      (OFC_CHAR) lpwFindFileData->cFileName[i] ;
  for (i = 0 ; i < 14 ; i++)
    lpaFindFileData->cAlternateFileName[i] = 
      (OFC_CHAR) lpwFindFileData->cAlternateFileName[i] ;
}

OFC_CORE_LIB OFC_HANDLE
OfcFindFirstFileA (OFC_LPCSTR lpFileName,
                   OFC_LPWIN32_FIND_DATAA lpFindFileData,
                   OFC_BOOL *more)
{
  OFC_HANDLE ret ;
  OFC_TCHAR *lptFileName ;
  OFC_WIN32_FIND_DATAW tFindFileData ;

  lptFileName = ofc_cstr2tstr (lpFileName) ;
  ret = OfcFindFirstFileW (lptFileName, &tFindFileData, more) ;
  ofc_free (lptFileName) ;
  if (ret != OFC_INVALID_HANDLE_VALUE)
    BlueFileFindDataW2FindDataA (lpFindFileData, &tFindFileData) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcFindNextFileW (OFC_HANDLE hFindFile,
                  OFC_LPWIN32_FIND_DATAW lpFindFileData,
                  OFC_BOOL *more)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = ofc_handle_lock (hFindFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSFindNextFile (fileContext->fsType,
                               fileContext->fsHandle,
                               lpFindFileData,
                               more) ;

      if (ret == OFC_TRUE && fileContext->fsType == OFC_FST_BROWSE_WORKGROUPS)
	update_workgroup (lpFindFileData->cFileName) ;

      ofc_handle_unlock (hFindFile) ;
    }

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcFindNextFileA (OFC_HANDLE hFindFile,
                  OFC_LPWIN32_FIND_DATAA lpFindFileData,
                  OFC_BOOL *more)
{
  OFC_BOOL ret ;
  OFC_WIN32_FIND_DATAW tFindFileData ;

  ret = OfcFindNextFileW (hFindFile, &tFindFileData, more) ;
  if (ret == OFC_TRUE)
    BlueFileFindDataW2FindDataA (lpFindFileData, &tFindFileData) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcFindClose (OFC_HANDLE hFindFile)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = ofc_handle_lock (hFindFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSFindClose (fileContext->fsType,
                            fileContext->fsHandle) ;
      ofc_handle_unlock (hFindFile) ;
      ofc_handle_destroy (hFindFile) ;
#if defined(OFC_FILE_DEBUG)
      BlueFileDebugFree (fileContext) ;
#endif
      ofc_free (fileContext) ;
    }

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcFlushFileBuffers (OFC_HANDLE hFile)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = ofc_handle_lock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSFlushFileBuffers (fileContext->fsType,
                                   fileContext->fsHandle) ;
      ofc_handle_unlock (hFile) ;
    }

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcGetFileAttributesExW (OFC_LPCTSTR lpFileName,
                         OFC_GET_FILEEX_INFO_LEVELS fInfoLevelId,
                         OFC_LPVOID lpFileInformation)
{
  OFC_LPTSTR lpMappedFileName ;
  OFC_FST_TYPE type ;
  OFC_BOOL ret ;
  OFC_PATH *path ;

  if (lpFileInformation == OFC_NULL)
    {
      ret = OFC_FALSE ;
      BlueThreadSetVariable (OfcLastError,
                             (OFC_DWORD_PTR) OFC_ERROR_BAD_ARGUMENTS) ;
    }
  else
    {
      path = ofc_map_path (lpFileName, &lpMappedFileName) ;
      type = MapType (path) ;

      ret = OfcFSGetFileAttributesEx (type, lpMappedFileName,
                                      fInfoLevelId, lpFileInformation) ;
      ofc_path_delete (path) ;
      ofc_free (lpMappedFileName) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcGetFileAttributesExA (OFC_LPCSTR lpFileName,
                         OFC_GET_FILEEX_INFO_LEVELS fInfoLevelId,
                         OFC_LPVOID lpFileInformation)
{
  OFC_BOOL ret ;
  OFC_TCHAR *lptFileName ;

  lptFileName = ofc_cstr2tstr (lpFileName) ;
  ret = OfcGetFileAttributesExW (lptFileName, fInfoLevelId,
                                 lpFileInformation) ;
  ofc_free (lptFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcGetFileInformationByHandleEx (OFC_HANDLE hFile,
                                 OFC_FILE_INFO_BY_HANDLE_CLASS
				  FileInformationClass,
                                 OFC_LPVOID lpFileInformation,
                                 OFC_DWORD dwBufferSize)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = ofc_handle_lock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSGetFileInformationByHandleEx (fileContext->fsType,
                                               fileContext->fsHandle,
                                               FileInformationClass,
                                               lpFileInformation,
                                               dwBufferSize) ;
      ofc_handle_unlock (hFile) ;
    }

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcMoveFileW (OFC_LPCTSTR lpExistingFileName,
              OFC_LPCTSTR lpNewFileName)
{
  OFC_LPTSTR lpExistingMappedFileName ;
  OFC_LPTSTR lpNewMappedFileName ;
  OFC_FST_TYPE existingType ;
  OFC_FST_TYPE newType ;
  OFC_BOOL ret ;

  ofc_path_mapW (lpExistingFileName, &lpExistingMappedFileName, &existingType) ;
  /*
   * Map the new path.  We'll through away the type.  
   * We'll leave it to the target filesystem to determine if it can move
   * to the new name
   */
  ofc_path_mapW (lpNewFileName, &lpNewMappedFileName, &newType) ;
  
  ret = OfcFSMoveFile (existingType,
                       lpExistingMappedFileName,
                       lpNewMappedFileName) ;

  ofc_free (lpExistingMappedFileName) ;
  ofc_free (lpNewMappedFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcMoveFileA (OFC_LPCSTR lpExistingFileName,
              OFC_LPCSTR lpNewFileName)
{
  OFC_BOOL ret ;
  OFC_TCHAR *lptExistingFileName ;
  OFC_TCHAR *lptNewFileName ;

  lptExistingFileName = ofc_cstr2tstr (lpExistingFileName) ;
  lptNewFileName = ofc_cstr2tstr (lpNewFileName) ;
  ret = OfcMoveFileW (lptExistingFileName, lptNewFileName) ;
  ofc_free (lptExistingFileName) ;
  ofc_free (lptNewFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcSetEndOfFile (OFC_HANDLE hFile)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = ofc_handle_lock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSSetEndOfFile (fileContext->fsType,
                               fileContext->fsHandle) ;
      ofc_handle_unlock (hFile) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcSetFileAttributesW (OFC_LPCTSTR lpFileName,
                       OFC_DWORD dwFileAttributes)
{
  OFC_LPTSTR lpMappedFileName ;
  OFC_FST_TYPE type ;
  OFC_BOOL ret ;

  ofc_path_mapW (lpFileName, &lpMappedFileName, &type) ;

  ret = OfcFSSetFileAttributes (type, lpMappedFileName, dwFileAttributes) ;

  ofc_free (lpMappedFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcSetFileAttributesA (OFC_LPCSTR lpFileName,
                       OFC_DWORD dwFileAttributes)
{
  OFC_BOOL ret ;
  OFC_TCHAR *lptFileName ;

  lptFileName = ofc_cstr2tstr (lpFileName) ;
  ret = OfcSetFileAttributesW (lptFileName, dwFileAttributes) ;
  ofc_free (lptFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcSetFileInformationByHandle (OFC_HANDLE hFile,
                               OFC_FILE_INFO_BY_HANDLE_CLASS
				FileInformationClass,
                               OFC_LPVOID lpFileInformation,
                               OFC_DWORD dwBufferSize)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = ofc_handle_lock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSSetFileInformationByHandle (fileContext->fsType,
                                             fileContext->fsHandle,
                                             FileInformationClass,
                                             lpFileInformation,
                                             dwBufferSize) ;
      ofc_handle_unlock (hFile) ;
    }

  return (ret) ;
}

OFC_CORE_LIB OFC_DWORD
OfcSetFilePointer (OFC_HANDLE hFile,
                   OFC_LONG lDistanceToMove,
                   OFC_PLONG lpDistanceToMoveHigh,
                   OFC_DWORD dwMoveMethod)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_DWORD ret ;

  ret = OFC_INVALID_SET_FILE_POINTER ;
  fileContext = ofc_handle_lock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSSetFilePointer (fileContext->fsType,
                                 fileContext->fsHandle,
                                 lDistanceToMove,
                                 lpDistanceToMoveHigh,
                                 dwMoveMethod) ;
      ofc_handle_unlock (hFile) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_UINT32
OfcGetLastFileError (OFC_HANDLE hHandle)
{
  OFC_UINT32 last_error ;

  last_error = OfcGetLastError () ;
  return (last_error) ;
}  

OFC_CORE_LIB OFC_UINT32
OfcGetLastFileNameErrorW (OFC_LPCTSTR lpFileName)
{
  OFC_UINT32 last_error ;

  last_error = OfcGetLastError () ;
  return (last_error) ;
}

OFC_CORE_LIB OFC_UINT32
OfcGetLastFileNameErrorA (OFC_LPCSTR lpFileName)
{
  OFC_UINT32 last_error ;

  last_error = OfcGetLastError () ;
  return (last_error) ;
}

OFC_CORE_LIB OFC_DWORD
OfcGetLastError (OFC_VOID)
{
  OFC_DWORD ret ;

  /* this truncates the upper 32 bits when 64 bit pointers are used */
  ret = (OFC_DWORD) BlueThreadGetVariable (OfcLastError) ;
  return (ret) ;
}

struct _err2str 
{
  OFC_DWORD err ;
  const OFC_CHAR *str ;
} ;

struct _err2str err2str[] =
  {
    { OFC_ERROR_SUCCESS, "Success"},
    { OFC_ERROR_INVALID_FUNCTION, "Invalid Function"},
    { OFC_ERROR_FILE_NOT_FOUND, "File Not Found"},
    { OFC_ERROR_PATH_NOT_FOUND, "Path Not Found"},
    { OFC_ERROR_TOO_MANY_OPEN_FILES, "Too Many Open Files"},
    { OFC_ERROR_ACCESS_DENIED, "Access Denied"},
    { OFC_ERROR_INVALID_HANDLE, "Invalid Handle"},
    { OFC_ERROR_NOT_ENOUGH_MEMORY, "Not Enough Memory"},
    { OFC_ERROR_INVALID_ACCESS, "Invalid Access"},
    { OFC_ERROR_OUTOFMEMORY, "Out of Memory"},
    { OFC_ERROR_INVALID_DRIVE, "Invalid Drive"},
    { OFC_ERROR_CURRENT_DIRECTORY, "Current Directory"},
    { OFC_ERROR_NOT_SAME_DEVICE, "Not Same Device"},
    { OFC_ERROR_NO_MORE_FILES, "No More Files"},
    { OFC_ERROR_WRITE_PROTECT, "Write Protected"},
    { OFC_ERROR_NOT_READY, "Not Ready"},
    { OFC_ERROR_CRC, "Bad CRC"},
    { OFC_ERROR_BAD_LENGTH, "Bad Length"},
    { OFC_ERROR_SEEK, "Bad Seek"},
    { OFC_ERROR_WRITE_FAULT, "Write Fault"},
    { OFC_ERROR_READ_FAULT, "Read Fault"},
    { OFC_ERROR_GEN_FAILURE, "General Failure"},
    { OFC_ERROR_SHARING_VIOLATION, "Sharing Violation"},
    { OFC_ERROR_LOCK_VIOLATION, "Lock Violation"},
    { OFC_ERROR_WRONG_DISK, "Wrong Disk"},
    { OFC_ERROR_SHARING_BUFFER_EXCEEDED, "Sharing Buffer Exceeded"},
    { OFC_ERROR_HANDLE_EOF, "End of File"}, 
    { OFC_ERROR_HANDLE_DISK_FULL, "Disk Full"},
    { OFC_ERROR_BAD_NET_NAME, "File Server Not Found"},
    { OFC_ERROR_NOT_SUPPORTED, "Not Supported"},
    { OFC_ERROR_REM_NOT_LIST, "Remote Not Listed"},
    { OFC_ERROR_DUP_NAME, "Duplicate Name"},
    { OFC_ERROR_BAD_NETPATH, "Bad Network Path"},
    { OFC_ERROR_NETWORK_BUSY, "Network Busy"},
    { OFC_ERROR_DEV_NOT_EXIST, "Device Does Not Exist"},
    { OFC_ERROR_BAD_NET_RESP, "Bad Network Response"},
    { OFC_ERROR_UNEXP_NET_ERR, "Unexpected Network Error"},
    { OFC_ERROR_BAD_DEV_TYPE, "Bad Device Type"},
    { OFC_ERROR_FILE_EXISTS, "File Already Exists"},
    { OFC_ERROR_CANNOT_MAKE, "Cannot Make"},
    { OFC_ERROR_INVALID_PASSWORD, "Invalid Password"},
    { OFC_ERROR_INVALID_PARAMETER, "Invalid Parameter"},
    { OFC_ERROR_NET_WRITE_FAULT, "Network Wrote Fault"},
    { OFC_ERROR_MORE_ENTRIES, "More Entries"},
    { OFC_ERROR_BROKEN_PIPE, "Broken Pipe"},
    { OFC_ERROR_OPEN_FAILED, "Open Failed"},
    { OFC_ERROR_BUFFER_OVERFLOW, "Buffer Overflow"},
    { OFC_ERROR_DISK_FULL, "Disk Full"},
    { OFC_ERROR_CALL_NOT_IMPLEMENTED, "Call Not Implemented"},
    { OFC_ERROR_INSUFFICIENT_BUFFER, "Insufficient Buffer"},
    { OFC_ERROR_INVALID_NAME, "Invalid Name"},
    { OFC_ERROR_INVALID_LEVEL, "Invalid Level"}, 
    { OFC_ERROR_NO_VOLUME_LABEL, "No Volume Label"}, 
    { OFC_ERROR_NEGATIVE_SEEK, "Negative Seek"},
    { OFC_ERROR_SEEK_ON_DEVICE, "Bad Seek on Device"},
    { OFC_ERROR_DIR_NOT_EMPTY, "Directory Not Empty"},
    { OFC_ERROR_PATH_BUSY, "Path Busy"},
    { OFC_ERROR_BAD_ARGUMENTS, "Bad Arguments"},
    { OFC_ERROR_BAD_PATHNAME, "Bad Pathname"},
    { OFC_ERROR_BUSY, "Busy"},
    { OFC_ERROR_ALREADY_EXISTS, "Already Exists"},
    { OFC_ERROR_INVALID_FLAG_NUMBER, "Invalid Flag Number"},
    { OFC_ERROR_BAD_PIPE, "Bad Pipe"},
    { OFC_ERROR_PIPE_BUSY, "Pipe Busy"},
    { OFC_ERROR_NO_DATA, "No Data"},
    { OFC_ERROR_PIPE_NOT_CONNECTED, "Pipe Not Connected"},
    { OFC_ERROR_MORE_DATA, "More Data"},
    { OFC_ERROR_INVALID_EA_NAME, "Invalid Attribute Name"},
    { OFC_ERROR_EA_LIST_INCONSISTENT, "Inconsistent Attribute List"},
    { OFC_ERROR_DIRECTORY, "Is Directory"},
    { OFC_ERROR_EAS_DIDNT_FIT, "Attribute Didn't Fit"},
    { OFC_ERROR_EA_FILE_CORRUPT, "Attributes Corrupt"},
    { OFC_ERROR_EA_TABLE_FULL, "Attribute Table Full"},
    { OFC_ERROR_INVALID_EA_HANDLE, "Invalid Attribute Handle"},
    { OFC_ERROR_EAS_NOT_SUPPORTED, "Attribute Not Supported"},
    { OFC_ERROR_OPLOCK_NOT_GRANTED, "Lock Not Granted"},
    { OFC_ERROR_DISK_TOO_FRAGMENTED, "Disk Too Fragmented"},
    { OFC_ERROR_DELETE_PENDING, "Delete Pending"},
    { OFC_ERROR_PIPE_CONNECTED, "Pipe Connected"},
    { OFC_ERROR_PIPE_LISTENING, "Pipe Listening"},
    { OFC_ERROR_EA_ACCESS_DENIED, "Attribute Access Denied"},
    { OFC_ERROR_OPERATION_ABORTED, "Operation Aborted"},
    { OFC_ERROR_IO_INCOMPLETE, "I/O Incomplete"},
    { OFC_ERROR_IO_PENDING, "I/O Pending"},
    { OFC_ERROR_NOACCESS, "No Access"},
    { OFC_ERROR_INVALID_FLAGS, "Invalid Flags"},
    { OFC_ERROR_UNRECOGNIZED_VOLUME, "Unrecognized Volume"},
    { OFC_ERROR_FILE_INVALID, "File Invalid"},
    { OFC_ERROR_NOTIFY_ENUM_DIR, "Notify Enum Dir"},
    { OFC_ERROR_BUS_RESET, "Bus Reset"},
    { OFC_ERROR_IO_DEVICE, "IO Device Error"},
    { OFC_ERROR_DISK_OPERATION_FAILED, "Disk Operation Failed"},
    { OFC_ERROR_BAD_DEVICE, "Bad Device"},
    { OFC_ERROR_INVALID_PASSWORDNAME, "Invalid Password Name"},
    { OFC_ERROR_LOGON_FAILURE, "Logon Failure"},
    { OFC_ERROR_NOT_ENOUGH_QUOTA, "Not Enough Quota"}
  };

OFC_CORE_LIB const OFC_CHAR *BlueGetErrorString(OFC_DWORD dwerr)
{
  const OFC_CHAR *errstr ;
  OFC_INT i ;

  errstr = OFC_NULL ;

  for (i = 0 ; 
       (i < sizeof(err2str) / sizeof (struct _err2str)) &&
	 (errstr == OFC_NULL) ; i++)
    {
      if (err2str[i].err == dwerr)
	errstr = err2str[i].str ;
    }

  if (errstr == OFC_NULL)
    errstr = "Unknown Error" ;

  return (errstr) ;
}

OFC_CORE_LIB OFC_BOOL
OfcGetDiskFreeSpaceW (OFC_LPCTSTR lpRootPathName,
                      OFC_LPDWORD lpSectorsPerCluster,
                      OFC_LPDWORD lpBytesPerSector,
                      OFC_LPDWORD lpNumberOfFreeClusters,
                      OFC_LPDWORD lpTotalNumberOfClusters)
{
  OFC_LPTSTR lpMappedPathName ;
  OFC_FST_TYPE type ;
  OFC_BOOL ret ;
  OFC_PATH *path ;

#if 0
  BluePathGetRootW (lpRootPathName, &lpMappedPathName, &type) ;
#else
  path = ofc_map_path (lpRootPathName, &lpMappedPathName) ;
  type = MapType (path) ;
#endif

  ret = OfcFSGetDiskFreeSpace (type,
                               lpMappedPathName,
                               lpSectorsPerCluster,
                               lpBytesPerSector,
                               lpNumberOfFreeClusters,
                               lpTotalNumberOfClusters) ;

  ofc_free (lpMappedPathName) ;
#if 1
  ofc_path_delete (path) ;
#endif

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcGetDiskFreeSpaceA (OFC_LPCSTR lpRootPathName,
                      OFC_LPDWORD lpSectorsPerCluster,
                      OFC_LPDWORD lpBytesPerSector,
                      OFC_LPDWORD lpNumberOfFreeClusters,
                      OFC_LPDWORD lpTotalNumberOfClusters)
{
  OFC_BOOL ret ;
  OFC_TCHAR *lptRootPathName ;

  lptRootPathName = ofc_cstr2tstr (lpRootPathName) ;
  ret = OfcGetDiskFreeSpaceW (lptRootPathName, lpSectorsPerCluster,
                              lpBytesPerSector, lpNumberOfFreeClusters,
                              lpTotalNumberOfClusters) ;
  ofc_free (lptRootPathName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcGetVolumeInformationW (OFC_LPCTSTR lpRootPathName,
                          OFC_LPTSTR lpVolumeNameBuffer,
                          OFC_DWORD nVolumeNameSize,
                          OFC_LPDWORD lpVolumeSerialNumber,
                          OFC_LPDWORD lpMaximumComponentLength,
                          OFC_LPDWORD lpFileSystemFlags,
                          OFC_LPTSTR lpFileSystemName,
                          OFC_DWORD nFileSystemName)
{
  OFC_LPTSTR lpMappedPathName ;
  OFC_FST_TYPE type ;
  OFC_BOOL ret ;

  ofc_path_get_rootW (lpRootPathName, &lpMappedPathName, &type) ;

  ret = OfcFSGetVolumeInformation (type,
                                   lpMappedPathName,
                                   lpVolumeNameBuffer,
                                   nVolumeNameSize,
                                   lpVolumeSerialNumber,
                                   lpMaximumComponentLength,
                                   lpFileSystemFlags,
                                   lpFileSystemName,
                                   nFileSystemName)  ;

  ofc_free (lpMappedPathName) ;

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcGetVolumeInformationA (OFC_LPCSTR lpRootPathName,
                          OFC_LPSTR lpVolumeNameBuffer,
                          OFC_DWORD nVolumeNameSize,
                          OFC_LPDWORD lpVolumeSerialNumber,
                          OFC_LPDWORD lpMaximumComponentLength,
                          OFC_LPDWORD lpFileSystemFlags,
                          OFC_LPSTR lpFileSystemName,
                          OFC_DWORD nFileSystemName)
{
  OFC_BOOL ret ;
  OFC_TCHAR *lptRootPathName ;
  OFC_TCHAR *lptVolumeNameBuffer ;
  OFC_TCHAR *lptFileSystemName ;
  OFC_DWORD i ;

  lptRootPathName = ofc_cstr2tstr (lpRootPathName) ;
  lptVolumeNameBuffer = OFC_NULL ;
  lptFileSystemName = OFC_NULL ;

  if (lpVolumeNameBuffer != OFC_NULL)
    lptVolumeNameBuffer = 
      ofc_malloc (nVolumeNameSize * sizeof (OFC_TCHAR)) ;
  if (lpFileSystemName != OFC_NULL)
    lptFileSystemName = 
      ofc_malloc (nFileSystemName * sizeof (OFC_TCHAR)) ;

  ret = OfcGetVolumeInformationW (lptRootPathName,
                                  lptVolumeNameBuffer, nVolumeNameSize,
                                  lpVolumeSerialNumber,
                                  lpMaximumComponentLength,
                                  lpFileSystemFlags,
                                  lptFileSystemName, nFileSystemName) ;
  if (ret == OFC_TRUE)
    {
      if (lpVolumeNameBuffer != OFC_NULL)
	for (i = 0 ; i < nVolumeNameSize ; i++)
	  lpVolumeNameBuffer[i] = (OFC_CHAR) lptVolumeNameBuffer[i] ;
      if (lpFileSystemName != OFC_NULL)
	for (i = 0 ; i < nFileSystemName ; i++)
	  lpFileSystemName[i] = (OFC_CHAR) lptFileSystemName[i] ;
    }

  if (lptVolumeNameBuffer != OFC_NULL)
    ofc_free (lptVolumeNameBuffer) ;
  if (lptFileSystemName != OFC_NULL)
    ofc_free (lptFileSystemName) ;
  ofc_free (lptRootPathName) ;
  return (ret) ;
}

/**
 * Unlock a region in a file
 * 
 * \param hFile
 * File Handle to unlock 
 *
 * \param length_low
 * the low order 32 bits of the length of the region
 *
 * \param length_high
 * the high order 32 bits of the length of the region
 *
 * \param hOverlapped
 * The overlapped structure which specifies the offset
 *
 * \returns
 * OFC_TRUE if successful, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcUnlockFileEx (OFC_HANDLE hFile,
                 OFC_UINT32 length_low, OFC_UINT32 length_high,
                 OFC_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = ofc_handle_lock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSUnlockFileEx (fileContext->fsType,
                               fileContext->fsHandle,
                               length_low, length_high, hOverlapped) ;

      ofc_handle_unlock (hFile) ;
    }
  return (ret) ;
}

/**
 * Lock a region of a file
 * 
 * \param hFile
 * Handle to file to unlock region in 
 *
 * \param flags
 * Flags for lock
 *
 * \param length_low
 * Low order 32 bits of length of region
 *
 * \param length_high
 * High order 32 bits of length of region
 *
 * \param hOverlapped
 * Pointer to overlapped structure containing offset of region
 *
 * \returns
 * OFC_TRUE if successful, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcLockFileEx (OFC_HANDLE hFile, OFC_DWORD flags,
               OFC_DWORD length_low, OFC_DWORD length_high,
               OFC_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = ofc_handle_lock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSLockFileEx (fileContext->fsType,
                             fileContext->fsHandle, flags,
                             length_low, length_high, hOverlapped) ;

      ofc_handle_unlock (hFile) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcDismountW (OFC_LPCTSTR lpFileName)
{
  OFC_LPTSTR lpMappedFileName ;
  OFC_FST_TYPE fsType ;
  OFC_BOOL ret ;

  ofc_path_mapW (lpFileName, &lpMappedFileName, &fsType) ;

  ret = OfcFSDismount (fsType, lpMappedFileName) ;

  ofc_free (lpMappedFileName) ;

  return (ret) ;
} 

OFC_CORE_LIB OFC_BOOL
OfcDismountA (OFC_LPCSTR lpFileName)
{
  OFC_TCHAR *lptFileName ;
  OFC_BOOL ret ;

  lptFileName = ofc_cstr2tstr (lpFileName) ;
  ret = OfcDismountW (lptFileName) ;

  ofc_free (lptFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL OfcDeviceIoControl (OFC_HANDLE hFile,
                                          OFC_DWORD dwIoControlCode,
                                          OFC_LPVOID lpInBuffer,
                                          OFC_DWORD nInBufferSize,
                                          OFC_LPVOID lpOutBuffer,
                                          OFC_DWORD nOutBufferSize,
                                          OFC_LPDWORD lpBytesReturned,
                                          OFC_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  fileContext = ofc_handle_lock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = OfcFSDeviceIoControl (fileContext->fsType,
                                  fileContext->fsHandle,
                                  dwIoControlCode,
                                  lpInBuffer,
                                  nInBufferSize,
                                  lpOutBuffer,
                                  nOutBufferSize,
                                  lpBytesReturned,
                                  hOverlapped) ;
      ofc_handle_unlock (hFile) ;
    }
  else
    ret = OFC_FALSE ;

  return (ret) ;
}

OFC_CORE_LIB OFC_FST_TYPE
OfcFileGetFSType (OFC_HANDLE hHandle)
{
  BLUE_FILE_CONTEXT *pFileContext ;
  OFC_FST_TYPE ret ;

  ret = OFC_FST_UNKNOWN ;
  pFileContext = ofc_handle_lock (hHandle) ;
  if (pFileContext != OFC_NULL)
    {
      ret = pFileContext->fsType ;
      ofc_handle_unlock (hHandle) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_HANDLE
OfcFileGetFSHandle (OFC_HANDLE hHandle)
{
  BLUE_FILE_CONTEXT *pFileContext ;
  OFC_HANDLE ret ;

  ret = OFC_HANDLE_NULL ;
  pFileContext = ofc_handle_lock (hHandle) ;
  if (pFileContext != OFC_NULL)
    {
      ret = pFileContext->fsHandle ;
      ofc_handle_unlock (hHandle) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_HANDLE
OfcFileGetOverlappedEvent (OFC_HANDLE hOverlapped)
{
  OFC_OVERLAPPED *Overlapped ;
  OFC_HANDLE ret ;

  ret = OFC_HANDLE_NULL ;

  Overlapped = ofc_handle_lock (hOverlapped) ;
  if (Overlapped != OFC_NULL)
    {
      ret = BlueWaitQGetEventHandle (Overlapped->hUser) ;
      ofc_handle_unlock (hOverlapped) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_HANDLE
OfcFileGetOverlappedWaitQ (OFC_HANDLE hOverlapped)
{
  OFC_OVERLAPPED *Overlapped ;
  OFC_HANDLE ret ;

  ret = OFC_HANDLE_NULL ;

  Overlapped = ofc_handle_lock (hOverlapped) ;
  if (Overlapped != OFC_NULL)
    {
      ret = Overlapped->hUser ;
      ofc_handle_unlock (hOverlapped) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
OfcFileInit (OFC_VOID)
{
    OfcLastError = BlueThreadCreateVariable () ;
  BlueThreadSetVariable (OfcLastError, (OFC_DWORD_PTR) OFC_ERROR_SUCCESS) ;
  init_workgroups () ;
#if defined(OFC_FILE_DEBUG)
  BlueFileDebug.Max = 0 ;
  BlueFileDebug.Total = 0 ;
  BlueFileDebug.Allocated = BLUE_NULL ;
  BlueFileLock = BlueLockInit () ;
#endif
}

OFC_CORE_LIB OFC_VOID
OfcFileDestroy (OFC_VOID)
{
#if defined(OFC_FILE_DEBUG)
  BlueLockDestroy (BlueFileLock) ;
#endif
  destroy_workgroups() ;
  BlueThreadDestroyVariable(OfcLastError);
}
