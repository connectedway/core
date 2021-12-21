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
#define __BLUE_CORE_DLL__

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
#if defined(BLUE_PARAM_FILE_DEBUG)
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

BLUE_DWORD BlueLastError ;

#if defined(BLUE_PARAM_FILE_DEBUG)
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
  BLUE_LPCTSTR server ;
  BLUE_LPCTSTR share ;
  BLUE_BOOL server_wild ;
  BLUE_BOOL share_wild ;
  BLUE_CTCHAR *p ;

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
      server_wild = BLUE_FALSE ;
      share_wild = BLUE_FALSE ;

      server = BluePathServer(path) ;
      if (server == BLUE_NULL)
	server_wild = BLUE_TRUE ;
      else
	{
	  p = BlueCtstrtok (server, TSTR("*?")) ;
	  if (p != BLUE_NULL && *p != TCHAR_EOS)
	    {
	      server_wild = BLUE_TRUE ;
	    }
	}

      share = BluePathShare (path) ;
      if (share == BLUE_NULL)
	share_wild = BLUE_TRUE ;
      else
	{
	  p = BlueCtstrtok (share, TSTR("*?")) ;
	  if (p != BLUE_NULL && *p != TCHAR_EOS)
	    {
	      share_wild = BLUE_TRUE ;
	    }
	  else if (BluePathDir (path, 0) == BLUE_NULL &&
		   BluePathFilename (path) == BLUE_NULL)
	    {
	      share_wild = BLUE_TRUE ;
	    }
	}

      if (server_wild)
	{
#if defined(BLUE_PARAM_FS_BROWSER)
	  fstype = BLUE_FS_BROWSE_WORKGROUPS ;
#endif
	}
      else if (share_wild && LookupWorkgroup (server) == BLUE_TRUE)
	{
#if defined(BLUE_PARAM_FS_BROWSER)
	  fstype = BLUE_FS_BROWSE_SERVERS ;
#endif
	}
      else if (share_wild)
	{
#if defined(BLUE_PARAM_FS_BROWSER)
	  fstype = BLUE_FS_BROWSE_SHARES ;
#endif
	}
      else
	{
#if defined(BLUE_PARAM_FS_CIFS)
	  fstype = BLUE_FS_CIFS ;
#endif
	}
    }
  return (fstype) ;
}

BLUE_CORE_LIB BLUE_HANDLE 
BlueCreateFileW (BLUE_LPCTSTR lpFileName,
		 BLUE_DWORD dwDesiredAccess,
		 BLUE_DWORD dwShareMode,
		 BLUE_LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		 BLUE_DWORD dwCreationDisposition,
		 BLUE_DWORD dwFlagsAndAttributes,
		 BLUE_HANDLE hTemplateFile) 
{
  BLUE_LPTSTR lpMappedFileName ;
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_HANDLE retHandle ;
  BLUE_HANDLE hMappedTemplateHandle ;

  fileContext = BlueHeapMalloc (sizeof (BLUE_FILE_CONTEXT)) ;

  fileContext->overlappedList = BlueQcreate () ;

#if defined(BLUE_PARAM_FILE_DEBUG)
  BlueFileDebugAlloc (fileContext, RETURN_ADDRESS()) ;
#endif
  BluePathMapW (lpFileName, &lpMappedFileName, &fileContext->fsType) ;

  hMappedTemplateHandle = BLUE_HANDLE_NULL ;
  if (hTemplateFile != BLUE_HANDLE_NULL)
    {
      BLUE_FILE_CONTEXT *templateContext ;
      templateContext = BlueHandleLock (hTemplateFile) ;
      if (templateContext != BLUE_NULL)
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
#if defined(BLUE_PARAM_FILE_DEBUG)
      BlueFileDebugFree (fileContext) ;
#endif
      BlueHeapFree (fileContext) ;
    }
  else
    retHandle = BlueHandleCreate (BLUE_HANDLE_FILE, fileContext) ;

  BlueHeapFree (lpMappedFileName) ;

  return (retHandle) ;
} 

BLUE_CORE_LIB BLUE_HANDLE 
BlueCreateFileA (BLUE_LPCSTR lpFileName,
		 BLUE_DWORD dwDesiredAccess,
		 BLUE_DWORD dwShareMode,
		 BLUE_LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		 BLUE_DWORD dwCreationDisposition,
		 BLUE_DWORD dwFlagsAndAttributes,
		 BLUE_HANDLE hTemplateFile) 
{
  BLUE_TCHAR *lptFileName ;
  BLUE_HANDLE ret ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  ret = BlueCreateFileW (lptFileName, dwDesiredAccess, dwShareMode,
			 lpSecurityAttributes, dwCreationDisposition,
			 dwFlagsAndAttributes, hTemplateFile) ;
  BlueHeapFree (lptFileName) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueCreateDirectoryW (BLUE_LPCTSTR lpPathName,
		      BLUE_LPSECURITY_ATTRIBUTES lpSecurityAttr)
{
  BLUE_LPTSTR lpMappedPathName ;	
  BLUE_FS_TYPE type ;	
  BLUE_BOOL ret ;	

  BluePathMapW (lpPathName, &lpMappedPathName, &type) ;

  ret = BlueFSCreateDirectory (type, lpMappedPathName, lpSecurityAttr) ;

  BlueHeapFree (lpMappedPathName) ;

  return (ret) ;
}  

BLUE_CORE_LIB BLUE_BOOL 
BlueCreateDirectoryA (BLUE_LPCSTR lpPathName,
		      BLUE_LPSECURITY_ATTRIBUTES lpSecurityAttr)
{
  BLUE_BOOL ret ;
  BLUE_TCHAR *lptPathName ;

  lptPathName = BlueCcstr2tstr (lpPathName) ;
  ret = BlueCreateDirectoryW (lptPathName, lpSecurityAttr) ;
  BlueHeapFree (lptPathName) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueWriteFile (BLUE_HANDLE hFile,
	       BLUE_LPCVOID lpBuffer,
	       BLUE_DWORD nNumberOfBytesToWrite,
	       BLUE_LPDWORD lpNumberOfBytesWritten,
	       BLUE_HANDLE hOverlapped) 
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_BOOL ret ;

  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != BLUE_NULL)
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
    ret = BLUE_FALSE ;

  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueTransactNamedPipe (BLUE_HANDLE hFile,
		       BLUE_LPVOID lpInBuffer,
		       BLUE_DWORD nInBufferSize,
		       BLUE_LPVOID lpOutBuffer,
		       BLUE_DWORD nOutBufferSize,
		       BLUE_LPDWORD lpBytesRead,
		       BLUE_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_BOOL ret ;

  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != BLUE_NULL)
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
    ret = BLUE_FALSE ;

  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueCloseHandle (BLUE_HANDLE hObject) 
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_BOOL ret ;
  BLUE_HANDLE hOverlapped ;

  fileContext = BlueHandleLock (hObject) ;
  if (fileContext != BLUE_NULL)
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
	  BlueDestroyOverlapped (hObject, hOverlapped) ;
	}
      BlueQdestroy (fileContext->overlappedList) ;
      BlueHandleUnlock (hObject) ;
      BlueHandleDestroy (hObject) ;
#if defined(BLUE_PARAM_FILE_DEBUG)
      BlueFileDebugFree (fileContext) ;
#endif
      BlueHeapFree (fileContext) ;
    }
  else
    ret = BLUE_FALSE ;

  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueReadFile (BLUE_HANDLE hFile,
	      BLUE_LPVOID lpBuffer,
	      BLUE_DWORD nNumberOfBytesToRead,
	      BLUE_LPDWORD lpNumberOfBytesRead,
	      BLUE_HANDLE hOverlapped) 
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_BOOL ret ;

  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != BLUE_NULL)
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
    ret = BLUE_FALSE ;

  return (ret) ;
}

BLUE_CORE_LIB BLUE_HANDLE 
BlueCreateOverlapped (BLUE_HANDLE hFile)
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_HANDLE ret ;

  ret = BLUE_HANDLE_NULL ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != BLUE_NULL)
    {
      ret = BlueFSCreateOverlapped (fileContext->fsType) ;
      if (ret != BLUE_HANDLE_NULL)
	BlueQenqueue (fileContext->overlappedList, (BLUE_VOID *) ret) ;
      BlueHandleUnlock (hFile) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueDestroyOverlapped (BLUE_HANDLE hFile, BLUE_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;

  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != BLUE_NULL)
    {
      BlueQunlink (fileContext->overlappedList, (BLUE_VOID *) hOverlapped) ;
      BlueFSDestroyOverlapped (fileContext->fsType, hOverlapped) ;
      BlueHandleUnlock (hFile) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueSetOverlappedOffset (BLUE_HANDLE hFile,
			 BLUE_HANDLE hOverlapped, 
			 BLUE_OFFT offset)
{
  BLUE_FILE_CONTEXT *fileContext ;

  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != BLUE_NULL)
    {
      BlueFSSetOverlappedOffset (fileContext->fsType, hOverlapped, offset) ;
      BlueHandleUnlock (hFile) ;
    }
}

BLUE_CORE_LIB BLUE_BOOL 
BlueGetOverlappedResult (BLUE_HANDLE hFile,
			 BLUE_HANDLE hOverlapped,
			 BLUE_LPDWORD lpNumberOfBytesTransferred,
			 BLUE_BOOL bWait)
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_BOOL ret ;

  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != BLUE_NULL)
    {
      ret = BlueFSGetOverlappedResult (fileContext->fsType,
				       fileContext->fsHandle,
				       hOverlapped,
				       lpNumberOfBytesTransferred,
				       bWait) ;
      BlueHandleUnlock (hFile) ;
    }
  else
    ret = BLUE_FALSE ;

  return (ret) ;
}


BLUE_CORE_LIB BLUE_BOOL 
BlueDeleteFileW (BLUE_LPCTSTR lpFileName) 
{
  BLUE_LPTSTR lpMappedFileName ;	
  BLUE_FS_TYPE type ;	
  BLUE_BOOL ret ;	

  BluePathMapW (lpFileName, &lpMappedFileName, &type) ;

  ret = BlueFSDeleteFile (type, lpMappedFileName) ;

  BlueHeapFree (lpMappedFileName) ;

  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueDeleteFileA (BLUE_LPCSTR lpFileName) 
{
  BLUE_BOOL ret ;
  BLUE_TCHAR *lptFileName ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  ret = BlueDeleteFileW (lptFileName) ;
  BlueHeapFree (lptFileName) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueRemoveDirectoryW (BLUE_LPCTSTR lpPathName) 
{
  BLUE_LPTSTR lpMappedPathName ;	
  BLUE_FS_TYPE type ;	
  BLUE_BOOL ret ;	

  BluePathMapW (lpPathName, &lpMappedPathName, &type) ;

  ret = BlueFSRemoveDirectory (type, lpMappedPathName) ;

  BlueHeapFree (lpMappedPathName) ;

  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueRemoveDirectoryA (BLUE_LPCSTR lpPathName) 
{
  BLUE_BOOL ret ;
  BLUE_TCHAR *lptPathName ;

  lptPathName = BlueCcstr2tstr (lpPathName) ;
  ret = BlueRemoveDirectoryW(lptPathName) ;
  BlueHeapFree (lptPathName) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_HANDLE 
BlueFindFirstFileW (BLUE_LPCTSTR lpFileName,
		    BLUE_LPWIN32_FIND_DATAW lpFindFileData,
		    BLUE_BOOL *more) 
{
  BLUE_LPTSTR lpMappedFileName ;	
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_HANDLE retHandle ;
  BLUE_PATH *path ;


  retHandle = BLUE_INVALID_HANDLE_VALUE ;
  fileContext = BlueHeapMalloc (sizeof (BLUE_FILE_CONTEXT)) ;
  if (fileContext != BLUE_NULL)
    {
#if defined(BLUE_PARAM_FILE_DEBUG)
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
	      BluePathServer(path) != BLUE_NULL)
	    RemoveWorkgroup (BluePathServer(path)) ;

          retHandle = fileContext->fsHandle ;
#if defined(BLUE_PARAM_FILE_DEBUG)
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

BLUE_CORE_LIB BLUE_VOID 
BlueFileFindDataW2FindDataA (BLUE_LPWIN32_FIND_DATAA lpaFindFileData,
			     BLUE_LPWIN32_FIND_DATAW lpwFindFileData)
{
  BLUE_INT i ;

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
  for (i = 0 ; i < BLUE_MAX_PATH ; i++)
    lpaFindFileData->cFileName[i] = 
      (BLUE_CHAR) lpwFindFileData->cFileName[i] ;
  for (i = 0 ; i < 14 ; i++)
    lpaFindFileData->cAlternateFileName[i] = 
      (BLUE_CHAR) lpwFindFileData->cAlternateFileName[i] ;
}

BLUE_CORE_LIB BLUE_HANDLE 
BlueFindFirstFileA (BLUE_LPCSTR lpFileName,
		    BLUE_LPWIN32_FIND_DATAA lpFindFileData,
		    BLUE_BOOL *more) 
{
  BLUE_HANDLE ret ;
  BLUE_TCHAR *lptFileName ;
  BLUE_WIN32_FIND_DATAW tFindFileData ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  ret = BlueFindFirstFileW (lptFileName, &tFindFileData, more) ;
  BlueHeapFree (lptFileName) ;
  if (ret != BLUE_INVALID_HANDLE_VALUE)
    BlueFileFindDataW2FindDataA (lpFindFileData, &tFindFileData) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueFindNextFileW (BLUE_HANDLE hFindFile,
		   BLUE_LPWIN32_FIND_DATAW lpFindFileData,
		   BLUE_BOOL *more) 
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  fileContext = BlueHandleLock (hFindFile) ;
  if (fileContext != BLUE_NULL)
    {
      ret = BlueFSFindNextFile (fileContext->fsType,
			        fileContext->fsHandle,
                                lpFindFileData,
				more) ;

      if (ret == BLUE_TRUE && fileContext->fsType == BLUE_FS_BROWSE_WORKGROUPS)
	UpdateWorkgroup (lpFindFileData->cFileName) ;

      BlueHandleUnlock (hFindFile) ;
    }

  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueFindNextFileA (BLUE_HANDLE hFindFile,
		   BLUE_LPWIN32_FIND_DATAA lpFindFileData,
		   BLUE_BOOL *more) 
{
  BLUE_BOOL ret ;
  BLUE_WIN32_FIND_DATAW tFindFileData ;

  ret = BlueFindNextFileW (hFindFile, &tFindFileData, more) ;
  if (ret == BLUE_TRUE)
    BlueFileFindDataW2FindDataA (lpFindFileData, &tFindFileData) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueFindClose (BLUE_HANDLE hFindFile) 
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  fileContext = BlueHandleLock (hFindFile) ;
  if (fileContext != BLUE_NULL)
    {
      ret = BlueFSFindClose (fileContext->fsType,
			     fileContext->fsHandle) ;
      BlueHandleUnlock (hFindFile) ;
      BlueHandleDestroy (hFindFile) ;
#if defined(BLUE_PARAM_FILE_DEBUG)
      BlueFileDebugFree (fileContext) ;
#endif
      BlueHeapFree (fileContext) ;
    }

  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueFlushFileBuffers (BLUE_HANDLE hFile) 
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != BLUE_NULL)
    {
      ret = BlueFSFlushFileBuffers (fileContext->fsType,
			            fileContext->fsHandle) ;
      BlueHandleUnlock (hFile) ;
    }

  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueGetFileAttributesExW (BLUE_LPCTSTR lpFileName,
			  BLUE_GET_FILEEX_INFO_LEVELS fInfoLevelId,
			  BLUE_LPVOID lpFileInformation) 
{
  BLUE_LPTSTR lpMappedFileName ;	
  BLUE_FS_TYPE type ;	
  BLUE_BOOL ret ;	
  BLUE_PATH *path ;

  if (lpFileInformation == BLUE_NULL)
    {
      ret = BLUE_FALSE ;
      BlueThreadSetVariable (BlueLastError, 
			     (BLUE_DWORD_PTR) BLUE_ERROR_BAD_ARGUMENTS) ;
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

BLUE_CORE_LIB BLUE_BOOL 
BlueGetFileAttributesExA (BLUE_LPCSTR lpFileName,
			  BLUE_GET_FILEEX_INFO_LEVELS fInfoLevelId,
			  BLUE_LPVOID lpFileInformation) 
{
  BLUE_BOOL ret ;
  BLUE_TCHAR *lptFileName ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  ret = BlueGetFileAttributesExW (lptFileName, fInfoLevelId, 
				  lpFileInformation) ;
  BlueHeapFree (lptFileName) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueGetFileInformationByHandleEx (BLUE_HANDLE hFile,
				  BLUE_FILE_INFO_BY_HANDLE_CLASS 
				  FileInformationClass,
				  BLUE_LPVOID lpFileInformation,
				  BLUE_DWORD dwBufferSize) 
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != BLUE_NULL)
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

BLUE_CORE_LIB BLUE_BOOL 
BlueMoveFileW (BLUE_LPCTSTR lpExistingFileName,
	       BLUE_LPCTSTR lpNewFileName) 
{
  BLUE_LPTSTR lpExistingMappedFileName ;	
  BLUE_LPTSTR lpNewMappedFileName ;	
  BLUE_FS_TYPE existingType ;	
  BLUE_FS_TYPE newType ;
  BLUE_BOOL ret ;	

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

BLUE_CORE_LIB BLUE_BOOL 
BlueMoveFileA (BLUE_LPCSTR lpExistingFileName,
	       BLUE_LPCSTR lpNewFileName) 
{
  BLUE_BOOL ret ;
  BLUE_TCHAR *lptExistingFileName ;
  BLUE_TCHAR *lptNewFileName ;

  lptExistingFileName = BlueCcstr2tstr (lpExistingFileName) ;
  lptNewFileName = BlueCcstr2tstr (lpNewFileName) ;
  ret = BlueMoveFileW (lptExistingFileName, lptNewFileName) ;
  BlueHeapFree (lptExistingFileName) ;
  BlueHeapFree (lptNewFileName) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueSetEndOfFile (BLUE_HANDLE hFile) 
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != BLUE_NULL)
    {
      ret = BlueFSSetEndOfFile (fileContext->fsType,
			        fileContext->fsHandle) ;
      BlueHandleUnlock (hFile) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueSetFileAttributesW (BLUE_LPCTSTR lpFileName,
			BLUE_DWORD dwFileAttributes)
{
  BLUE_LPTSTR lpMappedFileName ;	
  BLUE_FS_TYPE type ;	
  BLUE_BOOL ret ;	

  BluePathMapW (lpFileName, &lpMappedFileName, &type) ;

  ret = BlueFSSetFileAttributes (type, lpMappedFileName, dwFileAttributes) ;

  BlueHeapFree (lpMappedFileName) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueSetFileAttributesA (BLUE_LPCSTR lpFileName,
			BLUE_DWORD dwFileAttributes)
{
  BLUE_BOOL ret ;
  BLUE_TCHAR *lptFileName ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  ret = BlueSetFileAttributesW (lptFileName, dwFileAttributes) ;
  BlueHeapFree (lptFileName) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueSetFileInformationByHandle (BLUE_HANDLE hFile,
				BLUE_FILE_INFO_BY_HANDLE_CLASS
				FileInformationClass,
				BLUE_LPVOID lpFileInformation,
				BLUE_DWORD dwBufferSize) 
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != BLUE_NULL)
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

BLUE_CORE_LIB BLUE_DWORD 
BlueSetFilePointer (BLUE_HANDLE hFile,
		    BLUE_LONG lDistanceToMove,
		    BLUE_PLONG lpDistanceToMoveHigh,
		    BLUE_DWORD dwMoveMethod) 
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_DWORD ret ;

  ret = BLUE_INVALID_SET_FILE_POINTER ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != BLUE_NULL)
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

BLUE_CORE_LIB BLUE_UINT32 
BlueGetLastFileError (BLUE_HANDLE hHandle)
{
  BLUE_UINT32 last_error ;

  last_error = BlueGetLastError () ;
  return (last_error) ;
}  

BLUE_CORE_LIB BLUE_UINT32 
BlueGetLastFileNameErrorW (BLUE_LPCTSTR lpFileName) 
{
  BLUE_UINT32 last_error ;

  last_error = BlueGetLastError () ;
  return (last_error) ;
}

BLUE_CORE_LIB BLUE_UINT32 
BlueGetLastFileNameErrorA (BLUE_LPCSTR lpFileName) 
{
  BLUE_UINT32 last_error ;

  last_error = BlueGetLastError () ;
  return (last_error) ;
}

BLUE_CORE_LIB BLUE_DWORD 
BlueGetLastError (BLUE_VOID)
{
  BLUE_DWORD ret ;

  /* this truncates the upper 32 bits when 64 bit pointers are used */
  ret = (BLUE_DWORD) BlueThreadGetVariable (BlueLastError) ;
  return (ret) ;
}

struct _err2str 
{
  BLUE_DWORD err ;
  const BLUE_CHAR *str ;
} ;

struct _err2str err2str[] =
  {
    { BLUE_ERROR_SUCCESS, "Success"},
    { BLUE_ERROR_INVALID_FUNCTION, "Invalid Function"},
    { BLUE_ERROR_FILE_NOT_FOUND, "File Not Found"},
    { BLUE_ERROR_PATH_NOT_FOUND, "Path Not Found"},
    { BLUE_ERROR_TOO_MANY_OPEN_FILES, "Too Many Open Files"},
    { BLUE_ERROR_ACCESS_DENIED, "Access Denied"},
    { BLUE_ERROR_INVALID_HANDLE, "Invalid Handle"},
    { BLUE_ERROR_NOT_ENOUGH_MEMORY, "Not Enough Memory"},
    { BLUE_ERROR_INVALID_ACCESS, "Invalid Access"},
    { BLUE_ERROR_OUTOFMEMORY, "Out of Memory"},
    { BLUE_ERROR_INVALID_DRIVE, "Invalid Drive"},
    { BLUE_ERROR_CURRENT_DIRECTORY, "Current Directory"},
    { BLUE_ERROR_NOT_SAME_DEVICE, "Not Same Device"},
    { BLUE_ERROR_NO_MORE_FILES, "No More Files"},
    { BLUE_ERROR_WRITE_PROTECT, "Write Protected"},
    { BLUE_ERROR_NOT_READY, "Not Ready"},
    { BLUE_ERROR_CRC, "Bad CRC"},
    { BLUE_ERROR_BAD_LENGTH, "Bad Length"},
    { BLUE_ERROR_SEEK, "Bad Seek"},
    { BLUE_ERROR_WRITE_FAULT, "Write Fault"},
    { BLUE_ERROR_READ_FAULT, "Read Fault"},
    { BLUE_ERROR_GEN_FAILURE, "General Failure"},
    { BLUE_ERROR_SHARING_VIOLATION, "Sharing Violation"},
    { BLUE_ERROR_LOCK_VIOLATION, "Lock Violation"},
    { BLUE_ERROR_WRONG_DISK, "Wrong Disk"},
    { BLUE_ERROR_SHARING_BUFFER_EXCEEDED, "Sharing Buffer Exceeded"},
    { BLUE_ERROR_HANDLE_EOF, "End of File"}, 
    { BLUE_ERROR_HANDLE_DISK_FULL, "Disk Full"},
    { BLUE_ERROR_BAD_NET_NAME, "File Server Not Found"},
    { BLUE_ERROR_NOT_SUPPORTED, "Not Supported"},
    { BLUE_ERROR_REM_NOT_LIST, "Remote Not Listed"},
    { BLUE_ERROR_DUP_NAME, "Duplicate Name"},
    { BLUE_ERROR_BAD_NETPATH, "Bad Network Path"},
    { BLUE_ERROR_NETWORK_BUSY, "Network Busy"},
    { BLUE_ERROR_DEV_NOT_EXIST, "Device Does Not Exist"},
    { BLUE_ERROR_BAD_NET_RESP, "Bad Network Response"},
    { BLUE_ERROR_UNEXP_NET_ERR, "Unexpected Network Error"},
    { BLUE_ERROR_BAD_DEV_TYPE, "Bad Device Type"},
    { BLUE_ERROR_FILE_EXISTS, "File Already Exists"},
    { BLUE_ERROR_CANNOT_MAKE, "Cannot Make"},
    { BLUE_ERROR_INVALID_PASSWORD, "Invalid Password"},
    { BLUE_ERROR_INVALID_PARAMETER, "Invalid Parameter"},
    { BLUE_ERROR_NET_WRITE_FAULT, "Network Wrote Fault"},
    { BLUE_ERROR_MORE_ENTRIES, "More Entries"},
    { BLUE_ERROR_BROKEN_PIPE, "Broken Pipe"},
    { BLUE_ERROR_OPEN_FAILED, "Open Failed"},
    { BLUE_ERROR_BUFFER_OVERFLOW, "Buffer Overflow"},
    { BLUE_ERROR_DISK_FULL, "Disk Full"},
    { BLUE_ERROR_CALL_NOT_IMPLEMENTED, "Call Not Implemented"},
    { BLUE_ERROR_INSUFFICIENT_BUFFER, "Insufficient Buffer"},
    { BLUE_ERROR_INVALID_NAME, "Invalid Name"},
    { BLUE_ERROR_INVALID_LEVEL, "Invalid Level"}, 
    { BLUE_ERROR_NO_VOLUME_LABEL, "No Volume Label"}, 
    { BLUE_ERROR_NEGATIVE_SEEK, "Negative Seek"},
    { BLUE_ERROR_SEEK_ON_DEVICE, "Bad Seek on Device"},
    { BLUE_ERROR_DIR_NOT_EMPTY, "Directory Not Empty"},
    { BLUE_ERROR_PATH_BUSY, "Path Busy"},
    { BLUE_ERROR_BAD_ARGUMENTS, "Bad Arguments"},
    { BLUE_ERROR_BAD_PATHNAME, "Bad Pathname"},
    { BLUE_ERROR_BUSY, "Busy"},
    { BLUE_ERROR_ALREADY_EXISTS, "Already Exists"},
    { BLUE_ERROR_INVALID_FLAG_NUMBER, "Invalid Flag Number"},
    { BLUE_ERROR_BAD_PIPE, "Bad Pipe"},
    { BLUE_ERROR_PIPE_BUSY, "Pipe Busy"},
    { BLUE_ERROR_NO_DATA, "No Data"},
    { BLUE_ERROR_PIPE_NOT_CONNECTED, "Pipe Not Connected"},
    { BLUE_ERROR_MORE_DATA, "More Data"},
    { BLUE_ERROR_INVALID_EA_NAME, "Invalid Attribute Name"},
    { BLUE_ERROR_EA_LIST_INCONSISTENT, "Inconsistent Attribute List"},
    { BLUE_ERROR_DIRECTORY, "Is Directory"},
    { BLUE_ERROR_EAS_DIDNT_FIT, "Attribute Didn't Fit"},
    { BLUE_ERROR_EA_FILE_CORRUPT, "Attributes Corrupt"},
    { BLUE_ERROR_EA_TABLE_FULL, "Attribute Table Full"},
    { BLUE_ERROR_INVALID_EA_HANDLE, "Invalid Attribute Handle"},
    { BLUE_ERROR_EAS_NOT_SUPPORTED, "Attribute Not Supported"},
    { BLUE_ERROR_OPLOCK_NOT_GRANTED, "Lock Not Granted"},
    { BLUE_ERROR_DISK_TOO_FRAGMENTED, "Disk Too Fragmented"},
    { BLUE_ERROR_DELETE_PENDING, "Delete Pending"},
    { BLUE_ERROR_PIPE_CONNECTED, "Pipe Connected"},
    { BLUE_ERROR_PIPE_LISTENING, "Pipe Listening"},
    { BLUE_ERROR_EA_ACCESS_DENIED, "Attribute Access Denied"},
    { BLUE_ERROR_OPERATION_ABORTED, "Operation Aborted"},
    { BLUE_ERROR_IO_INCOMPLETE, "I/O Incomplete"},
    { BLUE_ERROR_IO_PENDING, "I/O Pending"},
    { BLUE_ERROR_NOACCESS, "No Access"},
    { BLUE_ERROR_INVALID_FLAGS, "Invalid Flags"},
    { BLUE_ERROR_UNRECOGNIZED_VOLUME, "Unrecognized Volume"},
    { BLUE_ERROR_FILE_INVALID, "File Invalid"},
    { BLUE_ERROR_NOTIFY_ENUM_DIR, "Notify Enum Dir"},
    { BLUE_ERROR_BUS_RESET, "Bus Reset"},
    { BLUE_ERROR_IO_DEVICE, "IO Device Error"},
    { BLUE_ERROR_DISK_OPERATION_FAILED, "Disk Operation Failed"},
    { BLUE_ERROR_BAD_DEVICE, "Bad Device"},
    { BLUE_ERROR_INVALID_PASSWORDNAME, "Invalid Password Name"},
    { BLUE_ERROR_LOGON_FAILURE, "Logon Failure"},
    { BLUE_ERROR_NOT_ENOUGH_QUOTA, "Not Enough Quota"}
  };

BLUE_CORE_LIB const BLUE_CHAR *BlueGetErrorString(BLUE_DWORD dwerr)
{
  const BLUE_CHAR *errstr ;
  BLUE_INT i ;

  errstr = BLUE_NULL ;

  for (i = 0 ; 
       (i < sizeof(err2str) / sizeof (struct _err2str)) &&
	 (errstr == BLUE_NULL) ; i++)
    {
      if (err2str[i].err == dwerr)
	errstr = err2str[i].str ;
    }

  if (errstr == BLUE_NULL)
    errstr = "Unknown Error" ;

  return (errstr) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueGetDiskFreeSpaceW (BLUE_LPCTSTR lpRootPathName,
		       BLUE_LPDWORD lpSectorsPerCluster,
		       BLUE_LPDWORD lpBytesPerSector,
		       BLUE_LPDWORD lpNumberOfFreeClusters,
		       BLUE_LPDWORD lpTotalNumberOfClusters) 
{
  BLUE_LPTSTR lpMappedPathName ;	
  BLUE_FS_TYPE type ;	
  BLUE_BOOL ret ;	
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

BLUE_CORE_LIB BLUE_BOOL 
BlueGetDiskFreeSpaceA (BLUE_LPCSTR lpRootPathName,
		       BLUE_LPDWORD lpSectorsPerCluster,
		       BLUE_LPDWORD lpBytesPerSector,
		       BLUE_LPDWORD lpNumberOfFreeClusters,
		       BLUE_LPDWORD lpTotalNumberOfClusters) 
{
  BLUE_BOOL ret ;
  BLUE_TCHAR *lptRootPathName ;

  lptRootPathName = BlueCcstr2tstr (lpRootPathName) ;
  ret = BlueGetDiskFreeSpaceW (lptRootPathName, lpSectorsPerCluster,
			       lpBytesPerSector, lpNumberOfFreeClusters,
			       lpTotalNumberOfClusters) ;
  BlueHeapFree (lptRootPathName) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueGetVolumeInformationW (BLUE_LPCTSTR lpRootPathName,
			   BLUE_LPTSTR lpVolumeNameBuffer,
			   BLUE_DWORD nVolumeNameSize,
			   BLUE_LPDWORD lpVolumeSerialNumber,
			   BLUE_LPDWORD lpMaximumComponentLength,
			   BLUE_LPDWORD lpFileSystemFlags,
			   BLUE_LPTSTR lpFileSystemName,
			   BLUE_DWORD nFileSystemName) 
{
  BLUE_LPTSTR lpMappedPathName ;	
  BLUE_FS_TYPE type ;	
  BLUE_BOOL ret ;	

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

BLUE_CORE_LIB BLUE_BOOL 
BlueGetVolumeInformationA (BLUE_LPCSTR lpRootPathName,
			   BLUE_LPSTR lpVolumeNameBuffer,
			   BLUE_DWORD nVolumeNameSize,
			   BLUE_LPDWORD lpVolumeSerialNumber,
			   BLUE_LPDWORD lpMaximumComponentLength,
			   BLUE_LPDWORD lpFileSystemFlags,
			   BLUE_LPSTR lpFileSystemName,
			   BLUE_DWORD nFileSystemName) 
{
  BLUE_BOOL ret ;
  BLUE_TCHAR *lptRootPathName ;
  BLUE_TCHAR *lptVolumeNameBuffer ;
  BLUE_TCHAR *lptFileSystemName ;
  BLUE_DWORD i ;

  lptRootPathName = BlueCcstr2tstr (lpRootPathName) ;
  lptVolumeNameBuffer = BLUE_NULL ;
  lptFileSystemName = BLUE_NULL ;

  if (lpVolumeNameBuffer != BLUE_NULL)
    lptVolumeNameBuffer = 
      BlueHeapMalloc (nVolumeNameSize * sizeof (BLUE_TCHAR)) ;
  if (lpFileSystemName != BLUE_NULL)
    lptFileSystemName = 
      BlueHeapMalloc (nFileSystemName * sizeof (BLUE_TCHAR)) ;

  ret = BlueGetVolumeInformationW (lptRootPathName, 
				   lptVolumeNameBuffer, nVolumeNameSize, 
				   lpVolumeSerialNumber,
				   lpMaximumComponentLength,
				   lpFileSystemFlags,
				   lptFileSystemName, nFileSystemName) ;
  if (ret == BLUE_TRUE)
    {
      if (lpVolumeNameBuffer != BLUE_NULL)
	for (i = 0 ; i < nVolumeNameSize ; i++)
	  lpVolumeNameBuffer[i] = (BLUE_CHAR) lptVolumeNameBuffer[i] ;
      if (lpFileSystemName != BLUE_NULL)
	for (i = 0 ; i < nFileSystemName ; i++)
	  lpFileSystemName[i] = (BLUE_CHAR) lptFileSystemName[i] ;
    }

  if (lptVolumeNameBuffer != BLUE_NULL)
    BlueHeapFree (lptVolumeNameBuffer) ;
  if (lptFileSystemName != BLUE_NULL)
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
 * BLUE_TRUE if successful, BLUE_FALSE otherwise
 */
BLUE_CORE_LIB BLUE_BOOL 
BlueUnlockFileEx (BLUE_HANDLE hFile, 
		  BLUE_UINT32 length_low, BLUE_UINT32 length_high,
		  BLUE_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != BLUE_NULL)
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
 * BLUE_TRUE if successful, BLUE_FALSE otherwise
 */
BLUE_CORE_LIB BLUE_BOOL 
BlueLockFileEx (BLUE_HANDLE hFile, BLUE_DWORD flags,
		BLUE_DWORD length_low, BLUE_DWORD length_high,
		BLUE_HANDLE hOverlapped)
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != BLUE_NULL)
    {
      ret = BlueFSLockFileEx (fileContext->fsType, 
			      fileContext->fsHandle, flags,
			      length_low, length_high, hOverlapped) ;

      BlueHandleUnlock (hFile) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueDismountW (BLUE_LPCTSTR lpFileName)
{
  BLUE_LPTSTR lpMappedFileName ;
  BLUE_FS_TYPE fsType ;
  BLUE_BOOL ret ;

  BluePathMapW (lpFileName, &lpMappedFileName, &fsType) ;

  ret = BlueFSDismount (fsType, lpMappedFileName) ;

  BlueHeapFree (lpMappedFileName) ;

  return (ret) ;
} 

BLUE_CORE_LIB BLUE_BOOL 
BlueDismountA (BLUE_LPCSTR lpFileName)
{
  BLUE_TCHAR *lptFileName ;
  BLUE_BOOL ret ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  ret = BlueDismountW (lptFileName) ;

  BlueHeapFree (lptFileName) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL BlueDeviceIoControl (BLUE_HANDLE hFile, 
					     BLUE_DWORD dwIoControlCode,
					     BLUE_LPVOID lpInBuffer, 
					     BLUE_DWORD nInBufferSize, 
					     BLUE_LPVOID lpOutBuffer, 
					     BLUE_DWORD nOutBufferSize,
					     BLUE_LPDWORD lpBytesReturned,
					     BLUE_HANDLE hOverlapped) 
{
  BLUE_FILE_CONTEXT *fileContext ;
  BLUE_BOOL ret ;

  fileContext = BlueHandleLock (hFile) ;
  if (fileContext != BLUE_NULL)
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
    ret = BLUE_FALSE ;

  return (ret) ;
}

BLUE_CORE_LIB BLUE_FS_TYPE 
BlueFileGetFSType (BLUE_HANDLE hHandle)
{
  BLUE_FILE_CONTEXT *pFileContext ;
  BLUE_FS_TYPE ret ;

  ret = BLUE_FS_UNKNOWN ;
  pFileContext = BlueHandleLock (hHandle) ;
  if (pFileContext != BLUE_NULL)
    {
      ret = pFileContext->fsType ;
      BlueHandleUnlock (hHandle) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_HANDLE 
BlueFileGetFSHandle (BLUE_HANDLE hHandle)
{
  BLUE_FILE_CONTEXT *pFileContext ;
  BLUE_HANDLE ret ;

  ret = BLUE_HANDLE_NULL ;
  pFileContext = BlueHandleLock (hHandle) ;
  if (pFileContext != BLUE_NULL)
    {
      ret = pFileContext->fsHandle ;
      BlueHandleUnlock (hHandle) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_HANDLE 
BlueFileGetOverlappedEvent (BLUE_HANDLE hOverlapped)
{
  BLUE_OVERLAPPED *Overlapped ;
  BLUE_HANDLE ret ;

  ret = BLUE_HANDLE_NULL ;

  Overlapped = BlueHandleLock (hOverlapped) ;
  if (Overlapped != BLUE_NULL)
    {
      ret = BlueWaitQGetEventHandle (Overlapped->hUser) ;
      BlueHandleUnlock (hOverlapped) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_HANDLE 
BlueFileGetOverlappedWaitQ (BLUE_HANDLE hOverlapped)
{
  BLUE_OVERLAPPED *Overlapped ;
  BLUE_HANDLE ret ;

  ret = BLUE_HANDLE_NULL ;

  Overlapped = BlueHandleLock (hOverlapped) ;
  if (Overlapped != BLUE_NULL)
    {
      ret = Overlapped->hUser ;
      BlueHandleUnlock (hOverlapped) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueFileInit (BLUE_VOID)
{
  BlueLastError = BlueThreadCreateVariable () ;
  BlueThreadSetVariable (BlueLastError, (BLUE_DWORD_PTR) BLUE_ERROR_SUCCESS) ;
  InitWorkgroups () ;
#if defined(BLUE_PARAM_FILE_DEBUG)
  BlueFileDebug.Max = 0 ;
  BlueFileDebug.Total = 0 ;
  BlueFileDebug.Allocated = BLUE_NULL ;
  BlueFileLock = BlueLockInit () ;
#endif
}

BLUE_CORE_LIB BLUE_VOID 
BlueFileDestroy (BLUE_VOID)
{
#if defined(BLUE_PARAM_FILE_DEBUG)
  BlueLockDestroy (BlueFileLock) ;
#endif
  DestroyWorkgroups() ;
  BlueThreadDestroyVariable(BlueLastError);
}
