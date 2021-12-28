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
  BLUE_HANDLE fsHandle ;
  BLUE_FS_TYPE fsType ;
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
  BLUE_HANDLE overlappedList ;
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

static BLUE_FS_TYPE MapType (BLUE_PATH *path)
{
  BLUE_FS_TYPE fstype ;
  OFC_LPCTSTR server ;
  OFC_LPCTSTR share ;
  OFC_BOOL server_wild ;
  OFC_BOOL share_wild ;
  OFC_CTCHAR *p ;

  fstype = BluePathType(path) ;

  if (BluePathRemote(path))
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

      server = BluePathServer(path) ;
      if (server == OFC_NULL)
	server_wild = OFC_TRUE ;
      else
	{
	  p = BlueCtstrtok (server, TSTR("*?")) ;
	  if (p != OFC_NULL && *p != TCHAR_EOS)
	    {
	      server_wild = OFC_TRUE ;
	    }
	}

      share = BluePathShare (path) ;
      if (share == OFC_NULL)
	share_wild = OFC_TRUE ;
      else
	{
	  p = BlueCtstrtok (share, TSTR("*?")) ;
	  if (p != OFC_NULL && *p != TCHAR_EOS)
	    {
	      share_wild = OFC_TRUE ;
	    }
	  else if (BluePathDir (path, 0) == OFC_NULL &&
               BluePathFilename (path) == OFC_NULL)
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
      else if (share_wild && LookupWorkgroup (server) == OFC_TRUE)
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

OFC_CORE_LIB BLUE_HANDLE
OfcCreateFileW (OFC_LPCTSTR lpFileName,
                OFC_DWORD dwDesiredAccess,
                OFC_DWORD dwShareMode,
                OFC_LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                OFC_DWORD dwCreationDisposition,
                OFC_DWORD dwFlagsAndAttributes,
                BLUE_HANDLE hTemplateFile)
{
  OFC_LPTSTR lpMappedFileName ;
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_HANDLE retHandle ;
  BLUE_HANDLE hMappedTemplateHandle ;

  fileContext = BlueHeapMalloc (sizeof (BLUE_FILE_CONTEXT)) ;

  fileContext->overlappedList = BlueQcreate () ;

#if defined(OFC_FILE_DEBUG)
  BlueFileDebugAlloc (fileContext, RETURN_ADDRESS()) ;
#endif
  BluePathMapW (lpFileName, &lpMappedFileName, &fileContext->fsType) ;

  hMappedTemplateHandle = BLUE_HANDLE_NULL ;
  if (hTemplateFile != BLUE_HANDLE_NULL)
    {
      BLUE_FILE_CONTEXT *templateContext ;
      templateContext = BlueHandleLock (hTemplateFile) ;
      if (templateContext != OFC_NULL)
	{
	  hMappedTemplateHandle = templateContext->fsHandle ;
	  BlueHandleUnlock (hTemplateFile) ;
	}
    }

  fileContext->fsHandle = BlueFSCreateFile (fileContext->fsType, 
					    lpMappedFileName,
					    dwDesiredAccess,
					    dwShareMode,
					    lpSecurityAttributes,
					    dwCreationDisposition,
					    dwFlagsAndAttributes,
					    hMappedTemplateHandle) ;


  if (fileContext->fsHandle == BLUE_HANDLE_NULL ||
      fileContext->fsHandle == BLUE_INVALID_HANDLE_VALUE)
    {
      retHandle = fileContext->fsHandle ;
      BlueQdestroy (fileContext->overlappedList) ;
#if defined(OFC_FILE_DEBUG)
      BlueFileDebugFree (fileContext) ;
#endif
      BlueHeapFree (fileContext) ;
    }
  else
    retHandle = BlueHandleCreate (BLUE_HANDLE_FILE, fileContext) ;

  BlueHeapFree (lpMappedFileName) ;

  return (retHandle) ;
} 

OFC_CORE_LIB BLUE_HANDLE
OfcCreateFileA (OFC_LPCSTR lpFileName,
                OFC_DWORD dwDesiredAccess,
                OFC_DWORD dwShareMode,
                OFC_LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                OFC_DWORD dwCreationDisposition,
                OFC_DWORD dwFlagsAndAttributes,
                BLUE_HANDLE hTemplateFile)
{
  OFC_TCHAR *lptFileName ;
  BLUE_HANDLE ret ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  ret = OfcCreateFileW (lptFileName, dwDesiredAccess, dwShareMode,
                        lpSecurityAttributes, dwCreationDisposition,
                        dwFlagsAndAttributes, hTemplateFile) ;
  BlueHeapFree (lptFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcCreateDirectoryW (OFC_LPCTSTR lpPathName,
                     OFC_LPSECURITY_ATTRIBUTES lpSecurityAttr)
{
  OFC_LPTSTR lpMappedPathName ;
  BLUE_FS_TYPE type ;	
  OFC_BOOL ret ;

  BluePathMapW (lpPathName, &lpMappedPathName, &type) ;

  ret = BlueFSCreateDirectory (type, lpMappedPathName, lpSecurityAttr) ;

  BlueHeapFree (lpMappedPathName) ;

  return (ret) ;
}  

OFC_CORE_LIB OFC_BOOL
OfcCreateDirectoryA (OFC_LPCSTR lpPathName,
                     OFC_LPSECURITY_ATTRIBUTES lpSecurityAttr)
{
  OFC_BOOL ret ;
  OFC_TCHAR *lptPathName ;

  lptPathName = BlueCcstr2tstr (lpPathName) ;
  ret = OfcCreateDirectoryW (lptPathName, lpSecurityAttr) ;
  BlueHeapFree (lptPathName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcWriteFile (BLUE_HANDLE hFile,
              OFC_LPCVOID lpBuffer,
              OFC_DWORD nNumberOfBytesToWrite,
              OFC_LPDWORD lpNumberOfBytesWritten,
              BLUE_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSWriteFile (fileContext->fsType,
			     fileContext->fsHandle,
			     lpBuffer,
			     nNumberOfBytesToWrite,
			     lpNumberOfBytesWritten,
			     hOverlapped) ;
      BlueHandleUnlock (hFile) ;
    }
  else
    ret = OFC_FALSE ;

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcTransactNamedPipe (BLUE_HANDLE hFile,
                      OFC_LPVOID lpInBuffer,
                      OFC_DWORD nInBufferSize,
                      OFC_LPVOID lpOutBuffer,
                      OFC_DWORD nOutBufferSize,
                      OFC_LPDWORD lpBytesRead,
                      BLUE_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSTransactNamedPipe (fileContext->fsType,
				     fileContext->fsHandle,
				     lpInBuffer,
				     nInBufferSize,
				     lpOutBuffer,
				     nOutBufferSize,
				     lpBytesRead,
				     hOverlapped) ;
      BlueHandleUnlock (hFile) ;
    }
  else
    ret = OFC_FALSE ;

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcCloseHandle (BLUE_HANDLE hObject)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;
  BLUE_HANDLE hOverlapped ;

  fileContext = BlueHandleLock (hObject) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSCloseHandle (fileContext->fsType,
			       fileContext->fsHandle) ;
      for (hOverlapped = 
	     (BLUE_HANDLE) BlueQfirst (fileContext->overlappedList) ;
	   hOverlapped != BLUE_HANDLE_NULL ;
	   hOverlapped = 
	     (BLUE_HANDLE) BlueQfirst (fileContext->overlappedList))
	{
	  /*
	   * This will also remove it from the overlapped list so first 
	   * also is returning the next one after this has been removed
	   */
	  OfcDestroyOverlapped (hObject, hOverlapped) ;
	}
      BlueQdestroy (fileContext->overlappedList) ;
      BlueHandleUnlock (hObject) ;
      BlueHandleDestroy (hObject) ;
#if defined(OFC_FILE_DEBUG)
      BlueFileDebugFree (fileContext) ;
#endif
      BlueHeapFree (fileContext) ;
    }
  else
    ret = OFC_FALSE ;

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcReadFile (BLUE_HANDLE hFile,
             OFC_LPVOID lpBuffer,
             OFC_DWORD nNumberOfBytesToRead,
             OFC_LPDWORD lpNumberOfBytesRead,
             BLUE_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSReadFile (fileContext->fsType,
			    fileContext->fsHandle,
			    lpBuffer,
			    nNumberOfBytesToRead,
			    lpNumberOfBytesRead,
			    hOverlapped) ;
      BlueHandleUnlock (hFile) ;
    }
  else
    ret = OFC_FALSE ;

  return (ret) ;
}

OFC_CORE_LIB BLUE_HANDLE
OfcCreateOverlapped (BLUE_HANDLE hFile)
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_HANDLE ret ;

  ret = BLUE_HANDLE_NULL ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSCreateOverlapped (fileContext->fsType) ;
      if (ret != BLUE_HANDLE_NULL)
	BlueQenqueue (fileContext->overlappedList, (OFC_VOID *) ret) ;
      BlueHandleUnlock (hFile) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
OfcDestroyOverlapped (BLUE_HANDLE hFile, BLUE_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;

  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      BlueQunlink (fileContext->overlappedList, (OFC_VOID *) hOverlapped) ;
      BlueFSDestroyOverlapped (fileContext->fsType, hOverlapped) ;
      BlueHandleUnlock (hFile) ;
    }
}

OFC_CORE_LIB OFC_VOID
OfcSetOverlappedOffset (BLUE_HANDLE hFile,
                        BLUE_HANDLE hOverlapped,
                        OFC_OFFT offset)
{
  BLUE_FILE_CONTEXT *fileContext ;

  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      BlueFSSetOverlappedOffset (fileContext->fsType, hOverlapped, offset) ;
      BlueHandleUnlock (hFile) ;
    }
}

OFC_CORE_LIB OFC_BOOL
OfcGetOverlappedResult (BLUE_HANDLE hFile,
                        BLUE_HANDLE hOverlapped,
                        OFC_LPDWORD lpNumberOfBytesTransferred,
                        OFC_BOOL bWait)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSGetOverlappedResult (fileContext->fsType,
				       fileContext->fsHandle,
				       hOverlapped,
				       lpNumberOfBytesTransferred,
				       bWait) ;
      BlueHandleUnlock (hFile) ;
    }
  else
    ret = OFC_FALSE ;

  return (ret) ;
}


OFC_CORE_LIB OFC_BOOL
OfcDeleteFileW (OFC_LPCTSTR lpFileName)
{
  OFC_LPTSTR lpMappedFileName ;
  BLUE_FS_TYPE type ;	
  OFC_BOOL ret ;

  BluePathMapW (lpFileName, &lpMappedFileName, &type) ;

  ret = BlueFSDeleteFile (type, lpMappedFileName) ;

  BlueHeapFree (lpMappedFileName) ;

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcDeleteFileA (OFC_LPCSTR lpFileName)
{
  OFC_BOOL ret ;
  OFC_TCHAR *lptFileName ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  ret = OfcDeleteFileW (lptFileName) ;
  BlueHeapFree (lptFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcRemoveDirectoryW (OFC_LPCTSTR lpPathName)
{
  OFC_LPTSTR lpMappedPathName ;
  BLUE_FS_TYPE type ;	
  OFC_BOOL ret ;

  BluePathMapW (lpPathName, &lpMappedPathName, &type) ;

  ret = BlueFSRemoveDirectory (type, lpMappedPathName) ;

  BlueHeapFree (lpMappedPathName) ;

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcRemoveDirectoryA (OFC_LPCSTR lpPathName)
{
  OFC_BOOL ret ;
  OFC_TCHAR *lptPathName ;

  lptPathName = BlueCcstr2tstr (lpPathName) ;
  ret = OfcRemoveDirectoryW(lptPathName) ;
  BlueHeapFree (lptPathName) ;
  return (ret) ;
}

OFC_CORE_LIB BLUE_HANDLE
OfcFindFirstFileW (OFC_LPCTSTR lpFileName,
                   OFC_LPWIN32_FIND_DATAW lpFindFileData,
                   OFC_BOOL *more)
{
  OFC_LPTSTR lpMappedFileName ;
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_HANDLE retHandle ;
  BLUE_PATH *path ;


  retHandle = BLUE_INVALID_HANDLE_VALUE ;
  fileContext = BlueHeapMalloc (sizeof (BLUE_FILE_CONTEXT)) ;
  if (fileContext != OFC_NULL)
    {
#if defined(OFC_FILE_DEBUG)
      BlueFileDebugAlloc (fileContext, RETURN_ADDRESS()) ;
#endif
      path = BlueMapPath (lpFileName, &lpMappedFileName) ;

      fileContext->fsType = MapType (path) ;

      fileContext->fsHandle = BlueFSFindFirstFile (fileContext->fsType, 
	                                           lpMappedFileName, 
                                                   lpFindFileData,
						   more) ;
      if (fileContext->fsHandle == BLUE_HANDLE_NULL ||
          fileContext->fsHandle == BLUE_INVALID_HANDLE_VALUE)
        {
	  if (fileContext->fsType == BLUE_FS_BROWSE_SERVERS &&
          BluePathServer(path) != OFC_NULL)
	    RemoveWorkgroup (BluePathServer(path)) ;

          retHandle = fileContext->fsHandle ;
#if defined(OFC_FILE_DEBUG)
	  BlueFileDebugFree (fileContext) ;
#endif
          BlueHeapFree (fileContext) ;
        }
      else
	{
	  retHandle = BlueHandleCreate (BLUE_HANDLE_FILE, fileContext) ;
	  if (fileContext->fsType == BLUE_FS_BROWSE_WORKGROUPS)
	    UpdateWorkgroup (lpFindFileData->cFileName) ;
	}
      BlueHeapFree (lpMappedFileName) ;
      BluePathDelete (path) ;
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

OFC_CORE_LIB BLUE_HANDLE
OfcFindFirstFileA (OFC_LPCSTR lpFileName,
                   OFC_LPWIN32_FIND_DATAA lpFindFileData,
                   OFC_BOOL *more)
{
  BLUE_HANDLE ret ;
  OFC_TCHAR *lptFileName ;
  OFC_WIN32_FIND_DATAW tFindFileData ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  ret = OfcFindFirstFileW (lptFileName, &tFindFileData, more) ;
  BlueHeapFree (lptFileName) ;
  if (ret != BLUE_INVALID_HANDLE_VALUE)
    BlueFileFindDataW2FindDataA (lpFindFileData, &tFindFileData) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcFindNextFileW (BLUE_HANDLE hFindFile,
                  OFC_LPWIN32_FIND_DATAW lpFindFileData,
                  OFC_BOOL *more)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = BlueHandleLock (hFindFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSFindNextFile (fileContext->fsType,
			        fileContext->fsHandle,
                                lpFindFileData,
				more) ;

      if (ret == OFC_TRUE && fileContext->fsType == BLUE_FS_BROWSE_WORKGROUPS)
	UpdateWorkgroup (lpFindFileData->cFileName) ;

      BlueHandleUnlock (hFindFile) ;
    }

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcFindNextFileA (BLUE_HANDLE hFindFile,
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
OfcFindClose (BLUE_HANDLE hFindFile)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = BlueHandleLock (hFindFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSFindClose (fileContext->fsType,
			     fileContext->fsHandle) ;
      BlueHandleUnlock (hFindFile) ;
      BlueHandleDestroy (hFindFile) ;
#if defined(OFC_FILE_DEBUG)
      BlueFileDebugFree (fileContext) ;
#endif
      BlueHeapFree (fileContext) ;
    }

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcFlushFileBuffers (BLUE_HANDLE hFile)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSFlushFileBuffers (fileContext->fsType,
			            fileContext->fsHandle) ;
      BlueHandleUnlock (hFile) ;
    }

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcGetFileAttributesExW (OFC_LPCTSTR lpFileName,
                         OFC_GET_FILEEX_INFO_LEVELS fInfoLevelId,
                         OFC_LPVOID lpFileInformation)
{
  OFC_LPTSTR lpMappedFileName ;
  BLUE_FS_TYPE type ;	
  OFC_BOOL ret ;
  BLUE_PATH *path ;

  if (lpFileInformation == OFC_NULL)
    {
      ret = OFC_FALSE ;
      BlueThreadSetVariable (OfcLastError,
                             (OFC_DWORD_PTR) OFC_ERROR_BAD_ARGUMENTS) ;
    }
  else
    {
      path = BlueMapPath (lpFileName, &lpMappedFileName) ;
      type = MapType (path) ;

      ret = BlueFSGetFileAttributesEx (type, lpMappedFileName, 
				       fInfoLevelId, lpFileInformation) ;
      BluePathDelete (path) ;
      BlueHeapFree (lpMappedFileName) ;
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

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  ret = OfcGetFileAttributesExW (lptFileName, fInfoLevelId,
                                 lpFileInformation) ;
  BlueHeapFree (lptFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcGetFileInformationByHandleEx (BLUE_HANDLE hFile,
                                 OFC_FILE_INFO_BY_HANDLE_CLASS
				  FileInformationClass,
                                 OFC_LPVOID lpFileInformation,
                                 OFC_DWORD dwBufferSize)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSGetFileInformationByHandleEx (fileContext->fsType,
			                        fileContext->fsHandle,
                                                FileInformationClass,
                                                lpFileInformation,
                                                dwBufferSize) ;
      BlueHandleUnlock (hFile) ;
    }

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcMoveFileW (OFC_LPCTSTR lpExistingFileName,
              OFC_LPCTSTR lpNewFileName)
{
  OFC_LPTSTR lpExistingMappedFileName ;
  OFC_LPTSTR lpNewMappedFileName ;
  BLUE_FS_TYPE existingType ;	
  BLUE_FS_TYPE newType ;
  OFC_BOOL ret ;

  BluePathMapW (lpExistingFileName, &lpExistingMappedFileName, &existingType) ;
  /*
   * Map the new path.  We'll through away the type.  
   * We'll leave it to the target filesystem to determine if it can move
   * to the new name
   */
  BluePathMapW (lpNewFileName, &lpNewMappedFileName, &newType) ;
  
  ret = BlueFSMoveFile (existingType,
			lpExistingMappedFileName,
			lpNewMappedFileName) ;

  BlueHeapFree (lpExistingMappedFileName) ;
  BlueHeapFree (lpNewMappedFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcMoveFileA (OFC_LPCSTR lpExistingFileName,
              OFC_LPCSTR lpNewFileName)
{
  OFC_BOOL ret ;
  OFC_TCHAR *lptExistingFileName ;
  OFC_TCHAR *lptNewFileName ;

  lptExistingFileName = BlueCcstr2tstr (lpExistingFileName) ;
  lptNewFileName = BlueCcstr2tstr (lpNewFileName) ;
  ret = OfcMoveFileW (lptExistingFileName, lptNewFileName) ;
  BlueHeapFree (lptExistingFileName) ;
  BlueHeapFree (lptNewFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcSetEndOfFile (BLUE_HANDLE hFile)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSSetEndOfFile (fileContext->fsType,
			        fileContext->fsHandle) ;
      BlueHandleUnlock (hFile) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcSetFileAttributesW (OFC_LPCTSTR lpFileName,
                       OFC_DWORD dwFileAttributes)
{
  OFC_LPTSTR lpMappedFileName ;
  BLUE_FS_TYPE type ;	
  OFC_BOOL ret ;

  BluePathMapW (lpFileName, &lpMappedFileName, &type) ;

  ret = BlueFSSetFileAttributes (type, lpMappedFileName, dwFileAttributes) ;

  BlueHeapFree (lpMappedFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcSetFileAttributesA (OFC_LPCSTR lpFileName,
                       OFC_DWORD dwFileAttributes)
{
  OFC_BOOL ret ;
  OFC_TCHAR *lptFileName ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  ret = OfcSetFileAttributesW (lptFileName, dwFileAttributes) ;
  BlueHeapFree (lptFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcSetFileInformationByHandle (BLUE_HANDLE hFile,
                               OFC_FILE_INFO_BY_HANDLE_CLASS
				FileInformationClass,
                               OFC_LPVOID lpFileInformation,
                               OFC_DWORD dwBufferSize)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSSetFileInformationByHandle (fileContext->fsType,
					      fileContext->fsHandle,
					      FileInformationClass,
					      lpFileInformation,
					      dwBufferSize) ;
      BlueHandleUnlock (hFile) ;
    }

  return (ret) ;
}

OFC_CORE_LIB OFC_DWORD
OfcSetFilePointer (BLUE_HANDLE hFile,
                   OFC_LONG lDistanceToMove,
                   OFC_PLONG lpDistanceToMoveHigh,
                   OFC_DWORD dwMoveMethod)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_DWORD ret ;

  ret = OFC_INVALID_SET_FILE_POINTER ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSSetFilePointer (fileContext->fsType,
			          fileContext->fsHandle,
                                  lDistanceToMove,
                                  lpDistanceToMoveHigh,
                                  dwMoveMethod) ;
      BlueHandleUnlock (hFile) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_UINT32
OfcGetLastFileError (BLUE_HANDLE hHandle)
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
  BLUE_FS_TYPE type ;	
  OFC_BOOL ret ;
  BLUE_PATH *path ;

#if 0
  BluePathGetRootW (lpRootPathName, &lpMappedPathName, &type) ;
#else
  path = BlueMapPath (lpRootPathName, &lpMappedPathName) ;
  type = MapType (path) ;
#endif

  ret = BlueFSGetDiskFreeSpace (type,
				lpMappedPathName,
				lpSectorsPerCluster,
				lpBytesPerSector,
				lpNumberOfFreeClusters,
				lpTotalNumberOfClusters) ;

  BlueHeapFree (lpMappedPathName) ;
#if 1
  BluePathDelete (path) ;
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

  lptRootPathName = BlueCcstr2tstr (lpRootPathName) ;
  ret = OfcGetDiskFreeSpaceW (lptRootPathName, lpSectorsPerCluster,
                              lpBytesPerSector, lpNumberOfFreeClusters,
                              lpTotalNumberOfClusters) ;
  BlueHeapFree (lptRootPathName) ;
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
  BLUE_FS_TYPE type ;	
  OFC_BOOL ret ;

  BluePathGetRootW (lpRootPathName, &lpMappedPathName, &type) ;

  ret = BlueFSGetVolumeInformation (type,
				    lpMappedPathName,
				    lpVolumeNameBuffer,
				    nVolumeNameSize,
				    lpVolumeSerialNumber,
				    lpMaximumComponentLength,
				    lpFileSystemFlags,
				    lpFileSystemName,
				    nFileSystemName)  ;

  BlueHeapFree (lpMappedPathName) ;

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

  lptRootPathName = BlueCcstr2tstr (lpRootPathName) ;
  lptVolumeNameBuffer = OFC_NULL ;
  lptFileSystemName = OFC_NULL ;

  if (lpVolumeNameBuffer != OFC_NULL)
    lptVolumeNameBuffer = 
      BlueHeapMalloc (nVolumeNameSize * sizeof (OFC_TCHAR)) ;
  if (lpFileSystemName != OFC_NULL)
    lptFileSystemName = 
      BlueHeapMalloc (nFileSystemName * sizeof (OFC_TCHAR)) ;

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
    BlueHeapFree (lptVolumeNameBuffer) ;
  if (lptFileSystemName != OFC_NULL)
    BlueHeapFree (lptFileSystemName) ;
  BlueHeapFree (lptRootPathName) ;
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
OfcUnlockFileEx (BLUE_HANDLE hFile,
                 OFC_UINT32 length_low, OFC_UINT32 length_high,
                 BLUE_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSUnlockFileEx (fileContext->fsType,
				fileContext->fsHandle,
				length_low, length_high, hOverlapped) ;

      BlueHandleUnlock (hFile) ;
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
OfcLockFileEx (BLUE_HANDLE hFile, OFC_DWORD flags,
               OFC_DWORD length_low, OFC_DWORD length_high,
               BLUE_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSLockFileEx (fileContext->fsType, 
			      fileContext->fsHandle, flags,
			      length_low, length_high, hOverlapped) ;

      BlueHandleUnlock (hFile) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
OfcDismountW (OFC_LPCTSTR lpFileName)
{
  OFC_LPTSTR lpMappedFileName ;
  BLUE_FS_TYPE fsType ;
  OFC_BOOL ret ;

  BluePathMapW (lpFileName, &lpMappedFileName, &fsType) ;

  ret = BlueFSDismount (fsType, lpMappedFileName) ;

  BlueHeapFree (lpMappedFileName) ;

  return (ret) ;
} 

OFC_CORE_LIB OFC_BOOL
OfcDismountA (OFC_LPCSTR lpFileName)
{
  OFC_TCHAR *lptFileName ;
  OFC_BOOL ret ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  ret = OfcDismountW (lptFileName) ;

  BlueHeapFree (lptFileName) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL OfcDeviceIoControl (BLUE_HANDLE hFile,
                                          OFC_DWORD dwIoControlCode,
                                          OFC_LPVOID lpInBuffer,
                                          OFC_DWORD nInBufferSize,
                                          OFC_LPVOID lpOutBuffer,
                                          OFC_DWORD nOutBufferSize,
                                          OFC_LPDWORD lpBytesReturned,
                                          BLUE_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;
  OFC_BOOL ret ;

  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != OFC_NULL)
    {
      ret = BlueFSDeviceIoControl (fileContext->fsType,
				   fileContext->fsHandle,
				   dwIoControlCode,
				   lpInBuffer,
				   nInBufferSize,
				   lpOutBuffer,
				   nOutBufferSize,
				   lpBytesReturned,
				   hOverlapped) ;
      BlueHandleUnlock (hFile) ;
    }
  else
    ret = OFC_FALSE ;

  return (ret) ;
}

OFC_CORE_LIB BLUE_FS_TYPE
OfcFileGetFSType (BLUE_HANDLE hHandle)
{
  BLUE_FILE_CONTEXT *pFileContext ;
  BLUE_FS_TYPE ret ;

  ret = BLUE_FS_UNKNOWN ;
  pFileContext = BlueHandleLock (hHandle) ;
  if (pFileContext != OFC_NULL)
    {
      ret = pFileContext->fsType ;
      BlueHandleUnlock (hHandle) ;
    }
  return (ret) ;
}

OFC_CORE_LIB BLUE_HANDLE
OfcFileGetFSHandle (BLUE_HANDLE hHandle)
{
  BLUE_FILE_CONTEXT *pFileContext ;
  BLUE_HANDLE ret ;

  ret = BLUE_HANDLE_NULL ;
  pFileContext = BlueHandleLock (hHandle) ;
  if (pFileContext != OFC_NULL)
    {
      ret = pFileContext->fsHandle ;
      BlueHandleUnlock (hHandle) ;
    }
  return (ret) ;
}

OFC_CORE_LIB BLUE_HANDLE
OfcFileGetOverlappedEvent (BLUE_HANDLE hOverlapped)
{
  OFC_OVERLAPPED *Overlapped ;
  BLUE_HANDLE ret ;

  ret = BLUE_HANDLE_NULL ;

  Overlapped = BlueHandleLock (hOverlapped) ;
  if (Overlapped != OFC_NULL)
    {
      ret = BlueWaitQGetEventHandle (Overlapped->hUser) ;
      BlueHandleUnlock (hOverlapped) ;
    }
  return (ret) ;
}

OFC_CORE_LIB BLUE_HANDLE
OfcFileGetOverlappedWaitQ (BLUE_HANDLE hOverlapped)
{
  OFC_OVERLAPPED *Overlapped ;
  BLUE_HANDLE ret ;

  ret = BLUE_HANDLE_NULL ;

  Overlapped = BlueHandleLock (hOverlapped) ;
  if (Overlapped != OFC_NULL)
    {
      ret = Overlapped->hUser ;
      BlueHandleUnlock (hOverlapped) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
OfcFileInit (OFC_VOID)
{
    OfcLastError = BlueThreadCreateVariable () ;
  BlueThreadSetVariable (OfcLastError, (OFC_DWORD_PTR) OFC_ERROR_SUCCESS) ;
  InitWorkgroups () ;
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
  DestroyWorkgroups() ;
  BlueThreadDestroyVariable(OfcLastError);
}
