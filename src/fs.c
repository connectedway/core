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
#include "ofc/fs.h"

static BLUE_FILE_FSINFO *BlueFSTable[BLUE_FS_NUM] ;

static BLUE_HANDLE BlueFSUnknownHandleInv (BLUE_VOID)
{
  return (BLUE_INVALID_HANDLE_VALUE) ;
}

static BLUE_HANDLE BlueFSUnknownHandleNull (BLUE_VOID)
{
  return (BLUE_HANDLE_NULL) ;
}

static BLUE_BOOL BlueFSUnknownBool (BLUE_VOID)
{
  return (BLUE_FALSE) ;
}

static BLUE_DWORD BlueFSUnknownDword (BLUE_VOID)
{
  return (BLUE_INVALID_SET_FILE_POINTER) ;
}

static BLUE_VOID BlueFSUnknownVoid (BLUE_VOID)
{
}

static BLUE_FILE_FSINFO BlueFSUnknown =
  {
    (BLUE_HANDLE (*)(BLUE_LPCTSTR,
		     BLUE_DWORD,
		     BLUE_DWORD,
		     BLUE_LPSECURITY_ATTRIBUTES,
		     BLUE_DWORD,
		     BLUE_DWORD,
		     BLUE_HANDLE)) &BlueFSUnknownHandleInv,
    (BLUE_BOOL (*)(BLUE_LPCTSTR)) &BlueFSUnknownBool,
    (BLUE_HANDLE (*)(BLUE_LPCTSTR,
		     BLUE_LPWIN32_FIND_DATAW,
		     BLUE_BOOL *)) &BlueFSUnknownHandleInv,
    (BLUE_BOOL (*)(BLUE_HANDLE,
		   BLUE_LPWIN32_FIND_DATAW,
		   BLUE_BOOL *)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_HANDLE)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_HANDLE)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_LPCTSTR,
		   BLUE_GET_FILEEX_INFO_LEVELS,
		   BLUE_LPVOID)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_HANDLE,
		   BLUE_FILE_INFO_BY_HANDLE_CLASS,
		   BLUE_LPVOID,
		   BLUE_DWORD)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_LPCTSTR,
		   BLUE_LPCTSTR)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_HANDLE,
		   BLUE_HANDLE,
		   BLUE_LPDWORD,
		   BLUE_BOOL)) &BlueFSUnknownBool,
    (BLUE_HANDLE (*)(BLUE_VOID)) &BlueFSUnknownHandleNull,
    (BLUE_VOID (*)(BLUE_HANDLE)) &BlueFSUnknownVoid,
    (BLUE_VOID (*)(BLUE_HANDLE,
		   BLUE_OFFT)) &BlueFSUnknownVoid,
    (BLUE_BOOL (*)(BLUE_HANDLE)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_LPCTSTR, 
		   BLUE_DWORD)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_HANDLE,
		   BLUE_FILE_INFO_BY_HANDLE_CLASS,
		   BLUE_LPVOID,
		   BLUE_DWORD)) &BlueFSUnknownBool,
    (BLUE_DWORD (*)(BLUE_HANDLE,
		    BLUE_LONG,
		    BLUE_PLONG,
		    BLUE_DWORD)) &BlueFSUnknownDword,
    (BLUE_BOOL (*)(BLUE_HANDLE,
		   BLUE_LPCVOID,
		   BLUE_DWORD,
		   BLUE_LPDWORD,
		   BLUE_HANDLE)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_HANDLE,
		   BLUE_LPVOID,
		   BLUE_DWORD,
		   BLUE_LPDWORD,
		   BLUE_HANDLE)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_HANDLE)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_HANDLE,
		   BLUE_LPVOID,
		   BLUE_DWORD,
		   BLUE_LPVOID,
		   BLUE_DWORD,
		   BLUE_LPDWORD,
		   BLUE_HANDLE)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_LPCTSTR,
		   BLUE_LPDWORD,
		   BLUE_LPDWORD,
		   BLUE_LPDWORD,
		   BLUE_LPDWORD)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_LPCTSTR,
		   BLUE_LPTSTR,
		   BLUE_DWORD,
		   BLUE_LPDWORD,
		   BLUE_LPDWORD,
		   BLUE_LPDWORD,
		   BLUE_LPTSTR,
		   BLUE_DWORD)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_LPCTSTR,
		   BLUE_LPSECURITY_ATTRIBUTES)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_LPCTSTR)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_HANDLE,
		   BLUE_UINT32,
		   BLUE_UINT32,
		   BLUE_HANDLE)) &BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_HANDLE,
		   BLUE_DWORD,
		   BLUE_DWORD,
		   BLUE_DWORD,
		   BLUE_HANDLE))&BlueFSUnknownBool,
    (BLUE_BOOL (*)(BLUE_LPCTSTR))&BlueFSUnknownBool
  } ;

BLUE_VOID BlueFSCIFSStartup (BLUE_VOID) ;
BLUE_VOID BlueFSWin32Startup (BLUE_VOID) ;
BLUE_VOID BlueFSWinCEStartup (BLUE_VOID) ;
BLUE_VOID BlueFSDarwinStartup (BLUE_VOID) ;
BLUE_VOID BlueFSLinuxStartup (BLUE_VOID) ;
BLUE_VOID BlueFSFileXStartup (BLUE_VOID) ;
BLUE_VOID BlueFSNUFileStartup (BLUE_VOID) ;
BLUE_VOID BlueFSBrowseWorkgroupsStartup (BLUE_VOID) ;
BLUE_VOID BlueFSBrowseServersStartup (BLUE_VOID) ;
BLUE_VOID BlueFSBrowseSharesStartup (BLUE_VOID) ;
BLUE_VOID BlueFSMailslotStartup (BLUE_VOID) ;
BLUE_VOID BlueFSPipeStartup (BLUE_VOID) ;
BLUE_VOID BlueFSBookmarksStartup (BLUE_VOID) ;
BLUE_VOID BlueFSAndroidStartup (BLUE_VOID) ;

BLUE_VOID BlueFSWinCEShutdown(BLUE_VOID) ;
BLUE_VOID BlueFSDarwinShutdown(BLUE_VOID) ;
BLUE_VOID BlueFSLinuxShutdown(BLUE_VOID) ;
BLUE_VOID BlueFSFileXShutdown(BLUE_VOID) ;
BLUE_VOID BlueFSAndroidShutdown(BLUE_VOID) ;
BLUE_VOID BlueFSNUFileShutdown(BLUE_VOID) ;
BLUE_VOID BlueFSBookmarksShutdown(BLUE_VOID) ;
BLUE_VOID BlueFSMailslotShutdown (BLUE_VOID) ;
BLUE_VOID BlueFSPipeShutdown(BLUE_VOID) ;

BLUE_CORE_LIB BLUE_VOID 
BlueFSInit (BLUE_VOID)
{
  BlueFSRegister (BLUE_FS_UNKNOWN, &BlueFSUnknown) ;
#if defined(BLUE_PARAM_FS_CIFS)
  BlueFSCIFSStartup () ;
#endif
#if defined (BLUE_PARAM_FS_WIN32)
  BlueFSWin32Startup() ;
#endif
#if defined (BLUE_PARAM_FS_WINCE)
  BlueFSWinCEStartup() ;
#endif
#if defined(OFC_FS_DARWIN)
  BlueFSDarwinStartup() ;
#endif
#if defined(BLUE_PARAM_FS_LINUX)
  BlueFSLinuxStartup() ;
#endif
#if defined(BLUE_PARAM_FS_FILEX)
  BlueFSFileXStartup() ;
#endif
#if defined(BLUE_PARAM_FS_ANDROID)
  BlueFSAndroidStartup() ;
#endif
#if defined(BLUE_PARAM_FS_NUFILE)
  BlueFSNUFileStartup() ;
#endif
#if defined(BLUE_PARAM_FS_BROWSER)
  BlueFSBrowseWorkgroupsStartup() ;
  BlueFSBrowseServersStartup() ;
  BlueFSBrowseSharesStartup() ;
#endif
#if defined(BLUE_PARAM_FS_BOOKMARKS)
  BlueFSBookmarksStartup() ;
#endif
#if defined(BLUE_PARAM_LANMAN)
  BlueFSMailslotStartup () ;
  BlueFSPipeStartup() ;
#endif
}

BLUE_CORE_LIB BLUE_VOID 
BlueFSDestroy (BLUE_VOID)
{
  BlueFSRegister (BLUE_FS_UNKNOWN, &BlueFSUnknown) ;
#if defined(BLUE_PARAM_FS_CIFS)
  BlueFSRegister (BLUE_FS_CIFS, &BlueFSUnknown);
#endif
#if defined (BLUE_PARAM_FS_WIN32)
  BlueFSRegister (BLUE_FS_WIN32, &BlueFSUnknown);
#endif
#if defined (BLUE_PARAM_FS_WINCE)
  BlueFSWinCEShutdown() ;
  BlueFSRegister (BLUE_FS_WINCE, &BlueFSUnknown);
#endif
#if defined(OFC_FS_DARWIN)
  BlueFSDarwinShutdown() ;
  BlueFSRegister (BLUE_FS_DARWIN, &BlueFSUnknown);
#endif
#if defined(BLUE_PARAM_FS_LINUX)
  BlueFSLinuxShutdown() ;
  BlueFSRegister (BLUE_FS_LINUX, &BlueFSUnknown);
#endif
#if defined(BLUE_PARAM_FS_FILEX)
  BlueFSFileXShutdown() ;
  BlueFSRegister (BLUE_FS_FILEX, &BlueFSUnknown);
#endif
#if defined(BLUE_PARAM_FS_ANDROID)
  BlueFSAndroidShutdown() ;
  BlueFSRegister (BLUE_FS_ANDROID, &BlueFSUnknown);
#endif
#if defined(BLUE_PARAM_FS_NUFILE)
  BlueFSNUFileShutdown() ;
  BlueFSRegister (BLUE_FS_NUFILE, &BlueFSUnknown);
#endif
#if defined(BLUE_PARAM_FS_BROWSER)
  BlueFSRegister (BLUE_FS_BROWSE_WORKGROUPS, &BlueFSUnknown);
  BlueFSRegister (BLUE_FS_BROWSE_SERVERS, &BlueFSUnknown);
  BlueFSRegister (BLUE_FS_BROWSE_SHARES, &BlueFSUnknown);
#endif
#if defined(BLUE_PARAM_FS_BOOKMARKS)
  BlueFSBookmarksShutdown() ;
  BlueFSRegister (BLUE_FS_BOOKMARKS, &BlueFSUnknown);
#endif
#if defined(BLUE_PARAM_LANMAN)
  BlueFSMailslotShutdown () ;
  BlueFSRegister (BLUE_FS_MAILSLOT, &BlueFSUnknown);
  BlueFSPipeShutdown() ;
  BlueFSRegister (BLUE_FS_PIPE, &BlueFSUnknown);
#endif
}

BLUE_CORE_LIB BLUE_VOID 
BlueFSRegister (BLUE_FS_TYPE fsType, BLUE_FILE_FSINFO *fsInfo)
{
  BlueFSTable[fsType] = fsInfo ;
}

BLUE_HANDLE BlueFSCreateFile (BLUE_FS_TYPE fsType,
			      BLUE_LPCTSTR lpFileName,
			      BLUE_DWORD dwDesiredAccess,
			      BLUE_DWORD dwShareMode,
			      BLUE_LPSECURITY_ATTRIBUTES lpSecAttributes,
			      BLUE_DWORD dwCreationDisposition,
			      BLUE_DWORD dwFlagsAndAttributes,
			      BLUE_HANDLE hTemplateFile)
{
  BLUE_HANDLE ret ;
  ret = BlueFSTable[fsType]->CreateFile (lpFileName,
					 dwDesiredAccess,
					 dwShareMode,
					 lpSecAttributes,
					 dwCreationDisposition,
					 dwFlagsAndAttributes,
					 hTemplateFile) ;
  return (ret) ;
}

BLUE_BOOL BlueFSCreateDirectory (BLUE_FS_TYPE fsType,
				 BLUE_LPCTSTR lpPathName,
				 BLUE_LPSECURITY_ATTRIBUTES lpSecurityAttr) 
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->CreateDirectory (lpPathName, lpSecurityAttr) ;
  return (ret) ;
}

BLUE_BOOL BlueFSWriteFile (BLUE_FS_TYPE fsType,
			   BLUE_HANDLE hFile,
			   BLUE_LPCVOID lpBuffer,
			   BLUE_DWORD nNumberOfBytesToWrite,
			   BLUE_LPDWORD lpNumberOfBytesWritten,
			   BLUE_HANDLE hOverlapped)
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->WriteFile (hFile,
					lpBuffer,
					nNumberOfBytesToWrite,
					lpNumberOfBytesWritten,
					hOverlapped) ;
  return (ret) ;
}

BLUE_BOOL BlueFSReadFile (BLUE_FS_TYPE fsType,
			  BLUE_HANDLE hFile,
			  BLUE_LPVOID lpBuffer,
			  BLUE_DWORD nNumberOfBytesToRead,
			  BLUE_LPDWORD lpNumberOfBytesRead,
			  BLUE_HANDLE hOverlapped)
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->ReadFile (hFile,
				       lpBuffer,
				       nNumberOfBytesToRead,
				       lpNumberOfBytesRead,
				       hOverlapped) ;
  return (ret) ;
}

BLUE_BOOL BlueFSCloseHandle (BLUE_FS_TYPE fsType,
			     BLUE_HANDLE hFile)
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->CloseHandle (hFile) ;

  return (ret) ;
}

BLUE_BOOL BlueFSDeleteFile (BLUE_FS_TYPE fsType, BLUE_LPCTSTR lpFileName) 
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->DeleteFile (lpFileName) ;

  return (ret) ;
}

BLUE_BOOL BlueFSRemoveDirectory (BLUE_FS_TYPE fsType, BLUE_LPCTSTR lpPathName) 
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->RemoveDirectory (lpPathName) ;

  return (ret) ;
}

BLUE_HANDLE BlueFSFindFirstFile (BLUE_FS_TYPE fsType,
				 BLUE_LPCTSTR lpFileName,
				 BLUE_LPWIN32_FIND_DATAW lpFindFileData,
				 BLUE_BOOL *more)
{
  BLUE_HANDLE ret ;
  ret = BlueFSTable[fsType]->FindFirstFile (lpFileName, lpFindFileData, more) ;

  return (ret) ;
}

BLUE_BOOL BlueFSFindNextFile (BLUE_FS_TYPE fsType,
			      BLUE_HANDLE hFindFile,
			      BLUE_LPWIN32_FIND_DATAW lpFindFileData,
			      BLUE_BOOL *more) 
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->FindNextFile (hFindFile, lpFindFileData, more) ;

  return (ret) ;
}

BLUE_BOOL BlueFSFindClose (BLUE_FS_TYPE fsType, BLUE_HANDLE hFindFile) 
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->FindClose (hFindFile) ;

  return (ret) ;
}

BLUE_BOOL BlueFSFlushFileBuffers (BLUE_FS_TYPE fsType, BLUE_HANDLE hFile) 
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->FlushFileBuffers (hFile) ;

  return (ret) ;
}

BLUE_BOOL BlueFSGetFileAttributesEx (BLUE_FS_TYPE fsType, 
				     BLUE_LPCTSTR lpFileName,
				     BLUE_GET_FILEEX_INFO_LEVELS fInfoLevelId,
				     BLUE_LPVOID lpFileInformation)
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->GetFileAttributesEx (lpFileName,
						  fInfoLevelId,
						  lpFileInformation) ;

  return (ret) ;
}

BLUE_BOOL BlueFSGetFileInformationByHandleEx (BLUE_FS_TYPE fsType,
					      BLUE_HANDLE hFile,
					      BLUE_FILE_INFO_BY_HANDLE_CLASS
					      FileInformationClass,
					      BLUE_LPVOID lpFileInformation,
					      BLUE_DWORD dwBufferSize) 
{
  BLUE_BOOL ret ;
  ret = 
    BlueFSTable[fsType]->GetFileInformationByHandleEx (hFile,
						       FileInformationClass,
						       lpFileInformation,
						       dwBufferSize) ;

  return (ret) ;
}

BLUE_BOOL BlueFSMoveFile (BLUE_FS_TYPE fsType,
			  BLUE_LPCTSTR lpExistingFileName,
			  BLUE_LPCTSTR lpNewFileName) 
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->MoveFile (lpExistingFileName, lpNewFileName) ;

  return (ret) ;
}

BLUE_HANDLE BlueFSCreateOverlapped (BLUE_FS_TYPE fsType)
{
  BLUE_HANDLE ret ;

  ret = BlueFSTable[fsType]->CreateOverlapped () ;
  return (ret) ;
}

BLUE_VOID BlueFSDestroyOverlapped (BLUE_FS_TYPE fsType, 
				   BLUE_HANDLE hOverlapped)
{
  BlueFSTable[fsType]->DestroyOverlapped (hOverlapped) ;
}

BLUE_VOID BlueFSSetOverlappedOffset (BLUE_FS_TYPE fsType, 
				     BLUE_HANDLE hOverlapped,
				     BLUE_OFFT offset)
{
  BlueFSTable[fsType]->SetOverlappedOffset (hOverlapped, offset) ;
}

BLUE_BOOL BlueFSGetOverlappedResult (BLUE_FS_TYPE fsType,
				     BLUE_HANDLE hFile,
				     BLUE_HANDLE hOverlapped,
				     BLUE_LPDWORD lpNumberOfBytesTransferred,
				     BLUE_BOOL bWait)
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->GetOverlappedResult (hFile,
						  hOverlapped,
						  lpNumberOfBytesTransferred,
						  bWait) ;

  return (ret) ;
}

BLUE_BOOL BlueFSSetEndOfFile (BLUE_FS_TYPE fsType, BLUE_HANDLE hFile) 
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->SetEndOfFile (hFile) ;

  return (ret) ;
}

BLUE_BOOL BlueFSSetFileAttributes (BLUE_FS_TYPE fsType, 
				   BLUE_LPCTSTR lpFileName,
				   BLUE_DWORD dwFileAttributes)
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->SetFileAttributes (lpFileName,
						dwFileAttributes) ;

  return (ret) ;
}

BLUE_BOOL BlueFSSetFileInformationByHandle (BLUE_FS_TYPE fsType, 
					    BLUE_HANDLE hFile,
					    BLUE_FILE_INFO_BY_HANDLE_CLASS
					    FileInformationClass,
					    BLUE_LPVOID lpFileInformation,
					    BLUE_DWORD dwBufferSize)
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->SetFileInformationByHandle (hFile,
							 FileInformationClass,
							 lpFileInformation,
							 dwBufferSize) ;

  return (ret) ;
}

BLUE_DWORD BlueFSSetFilePointer (BLUE_FS_TYPE fsType,
				 BLUE_HANDLE hFile,
				 BLUE_LONG lDistanceToMove,
				 BLUE_PLONG lpDistanceToMoveHigh,
				 BLUE_DWORD dwMoveMethod)
{
  BLUE_DWORD ret ;
  ret = BlueFSTable[fsType]->SetFilePointer (hFile, lDistanceToMove,
					     lpDistanceToMoveHigh,
					     dwMoveMethod) ;

  return (ret) ;
}

BLUE_BOOL BlueFSTransactNamedPipe (BLUE_FS_TYPE fsType,
				   BLUE_HANDLE hFile,
				   BLUE_LPVOID lpInBuffer,
				   BLUE_DWORD nInBufferSize,
				   BLUE_LPVOID lpOutBuffer,
				   BLUE_DWORD nOutBufferSize,
				   BLUE_LPDWORD lpBytesRead,
				   BLUE_HANDLE hOverlapped)
{
  BLUE_BOOL ret ;

  ret = BlueFSTable[fsType]->TransactNamedPipe (hFile,
						lpInBuffer,
						nInBufferSize,
						lpOutBuffer,
						nOutBufferSize,
						lpBytesRead,
						hOverlapped) ;
  return (ret) ;
}

BLUE_BOOL BlueFSDeviceIoControl (BLUE_FS_TYPE fsType,
				 BLUE_HANDLE hFile,
				 BLUE_DWORD dwIoControlCode,
				 BLUE_LPVOID lpInBuffer,
				 BLUE_DWORD nInBufferSize,
				 BLUE_LPVOID lpOutBuffer,
				 BLUE_DWORD nOutBufferSize,
				 BLUE_LPDWORD lpBytesRead,
				 BLUE_HANDLE hOverlapped)
{
  BLUE_BOOL ret ;

  ret = BlueFSTable[fsType]->DeviceIoControl (hFile,
					      dwIoControlCode,
					      lpInBuffer,
					      nInBufferSize,
					      lpOutBuffer,
					      nOutBufferSize,
					      lpBytesRead,
					      hOverlapped) ;
  return (ret) ;
}

BLUE_BOOL BlueFSGetDiskFreeSpace (BLUE_FS_TYPE fsType,
				  BLUE_LPCTSTR lpRootPathName,
				  BLUE_LPDWORD lpSectorsPerCluster,
				  BLUE_LPDWORD lpBytesPerSector,
				  BLUE_LPDWORD lpNumberOfFreeClusters,
				  BLUE_LPDWORD lpTotalNumberOfClusters) 
{
  BLUE_BOOL ret ;

  ret = BlueFSTable[fsType]->GetDiskFreeSpace (lpRootPathName,
					       lpSectorsPerCluster,
					       lpBytesPerSector,
					       lpNumberOfFreeClusters,
					       lpTotalNumberOfClusters) ;
  return (ret) ;
}

BLUE_BOOL BlueFSGetVolumeInformation (BLUE_FS_TYPE fsType,
				      BLUE_LPCTSTR lpRootPathName,
				      BLUE_LPTSTR lpVolumeNameBuffer,
				      BLUE_DWORD nVolumeNameSize,
				      BLUE_LPDWORD lpVolumeSerialNumber,
				      BLUE_LPDWORD lpMaximumComponentLength,
				      BLUE_LPDWORD lpFileSystemFlags,
				      BLUE_LPTSTR lpFileSystemName,
				      BLUE_DWORD nFileSystemName) 
{
  BLUE_BOOL ret ;

  ret = BlueFSTable[fsType]->GetVolumeInformation (lpRootPathName,
						   lpVolumeNameBuffer,
						   nVolumeNameSize,
						   lpVolumeSerialNumber,
						   lpMaximumComponentLength,
						   lpFileSystemFlags,
						   lpFileSystemName,
						   nFileSystemName) ;
  return (ret) ;
}

/**
 * Unlock a region in a file
 * 
 * \param fsType
 * Index into type of file system API to translate to 
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
BLUE_BOOL BlueFSUnlockFileEx (BLUE_FS_TYPE fsType,
			      BLUE_HANDLE hFile, 
			      BLUE_UINT32 length_low, 
			      BLUE_UINT32 length_high,
			      BLUE_HANDLE hOverlapped)
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->UnlockFileEx (hFile,
					   length_low, length_high,
					   hOverlapped) ;
  return (ret) ;
}
/**
 * Lock a region of a file
 * 
 * \param fsType
 * Index into type of file system API to translate to 
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
BLUE_BOOL BlueFSLockFileEx (BLUE_FS_TYPE fsType,
			    BLUE_HANDLE hFile, BLUE_DWORD flags,
			    BLUE_DWORD length_low, BLUE_DWORD length_high,
			    BLUE_HANDLE hOverlapped) 
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->LockFileEx (hFile, flags,
					 length_low, length_high,
					 hOverlapped) ;
  return (ret) ;
}

BLUE_BOOL BlueFSDismount (BLUE_FS_TYPE fsType,
			  BLUE_LPCTSTR lpFileName)
{
  BLUE_BOOL ret ;
  ret = BlueFSTable[fsType]->Dismount (lpFileName) ;

  return (ret) ;
}

