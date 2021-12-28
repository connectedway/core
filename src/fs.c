/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */

#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/config.h"
#include "ofc/fs.h"

static BLUE_FILE_FSINFO *BlueFSTable[BLUE_FS_NUM] ;

static BLUE_HANDLE BlueFSUnknownHandleInv (OFC_VOID)
{
  return (BLUE_INVALID_HANDLE_VALUE) ;
}

static BLUE_HANDLE BlueFSUnknownHandleNull (OFC_VOID)
{
  return (BLUE_HANDLE_NULL) ;
}

static OFC_BOOL BlueFSUnknownBool (OFC_VOID)
{
  return (OFC_FALSE) ;
}

static OFC_DWORD BlueFSUnknownDword (OFC_VOID)
{
  return (OFC_INVALID_SET_FILE_POINTER) ;
}

static OFC_VOID BlueFSUnknownVoid (OFC_VOID)
{
}

static BLUE_FILE_FSINFO BlueFSUnknown =
  {
    (BLUE_HANDLE (*)(OFC_LPCTSTR,
                     OFC_DWORD,
                     OFC_DWORD,
                     OFC_LPSECURITY_ATTRIBUTES,
                     OFC_DWORD,
                     OFC_DWORD,
                     BLUE_HANDLE)) &BlueFSUnknownHandleInv,
    (OFC_BOOL (*)(OFC_LPCTSTR)) &BlueFSUnknownBool,
    (BLUE_HANDLE (*)(OFC_LPCTSTR,
                     OFC_LPWIN32_FIND_DATAW,
                     OFC_BOOL *)) &BlueFSUnknownHandleInv,
    (OFC_BOOL (*)(BLUE_HANDLE,
                  OFC_LPWIN32_FIND_DATAW,
                  OFC_BOOL *)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(BLUE_HANDLE)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(BLUE_HANDLE)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_LPCTSTR,
                  OFC_GET_FILEEX_INFO_LEVELS,
                  OFC_LPVOID)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(BLUE_HANDLE,
                  OFC_FILE_INFO_BY_HANDLE_CLASS,
                  OFC_LPVOID,
                  OFC_DWORD)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_LPCTSTR,
                  OFC_LPCTSTR)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(BLUE_HANDLE,
                  BLUE_HANDLE,
                  OFC_LPDWORD,
                  OFC_BOOL)) &BlueFSUnknownBool,
    (BLUE_HANDLE (*)(OFC_VOID)) &BlueFSUnknownHandleNull,
    (OFC_VOID (*)(BLUE_HANDLE)) &BlueFSUnknownVoid,
    (OFC_VOID (*)(BLUE_HANDLE,
                  OFC_OFFT)) &BlueFSUnknownVoid,
    (OFC_BOOL (*)(BLUE_HANDLE)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_LPCTSTR,
                  OFC_DWORD)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(BLUE_HANDLE,
                  OFC_FILE_INFO_BY_HANDLE_CLASS,
                  OFC_LPVOID,
                  OFC_DWORD)) &BlueFSUnknownBool,
    (OFC_DWORD (*)(BLUE_HANDLE,
                   OFC_LONG,
                   OFC_PLONG,
                   OFC_DWORD)) &BlueFSUnknownDword,
    (OFC_BOOL (*)(BLUE_HANDLE,
                  OFC_LPCVOID,
                  OFC_DWORD,
                  OFC_LPDWORD,
                  BLUE_HANDLE)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(BLUE_HANDLE,
                  OFC_LPVOID,
                  OFC_DWORD,
                  OFC_LPDWORD,
                  BLUE_HANDLE)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(BLUE_HANDLE)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(BLUE_HANDLE,
                  OFC_LPVOID,
                  OFC_DWORD,
                  OFC_LPVOID,
                  OFC_DWORD,
                  OFC_LPDWORD,
                  BLUE_HANDLE)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_LPCTSTR,
                  OFC_LPDWORD,
                  OFC_LPDWORD,
                  OFC_LPDWORD,
                  OFC_LPDWORD)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_LPCTSTR,
                  OFC_LPTSTR,
                  OFC_DWORD,
                  OFC_LPDWORD,
                  OFC_LPDWORD,
                  OFC_LPDWORD,
                  OFC_LPTSTR,
                  OFC_DWORD)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_LPCTSTR,
                  OFC_LPSECURITY_ATTRIBUTES)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_LPCTSTR)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(BLUE_HANDLE,
                  OFC_UINT32,
                  OFC_UINT32,
                  BLUE_HANDLE)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(BLUE_HANDLE,
                  OFC_DWORD,
                  OFC_DWORD,
                  OFC_DWORD,
                  BLUE_HANDLE))&BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_LPCTSTR))&BlueFSUnknownBool
  } ;

OFC_VOID BlueFSCIFSStartup (OFC_VOID) ;
OFC_VOID BlueFSWin32Startup (OFC_VOID) ;
OFC_VOID BlueFSWinCEStartup (OFC_VOID) ;
OFC_VOID BlueFSDarwinStartup (OFC_VOID) ;
OFC_VOID BlueFSLinuxStartup (OFC_VOID) ;
OFC_VOID BlueFSFileXStartup (OFC_VOID) ;
OFC_VOID BlueFSNUFileStartup (OFC_VOID) ;
OFC_VOID BlueFSBrowseWorkgroupsStartup (OFC_VOID) ;
OFC_VOID BlueFSBrowseServersStartup (OFC_VOID) ;
OFC_VOID BlueFSBrowseSharesStartup (OFC_VOID) ;
OFC_VOID BlueFSMailslotStartup (OFC_VOID) ;
OFC_VOID BlueFSPipeStartup (OFC_VOID) ;
OFC_VOID BlueFSBookmarksStartup (OFC_VOID) ;
OFC_VOID BlueFSAndroidStartup (OFC_VOID) ;

OFC_VOID BlueFSWinCEShutdown(OFC_VOID) ;
OFC_VOID BlueFSDarwinShutdown(OFC_VOID) ;
OFC_VOID BlueFSLinuxShutdown(OFC_VOID) ;
OFC_VOID BlueFSFileXShutdown(OFC_VOID) ;
OFC_VOID BlueFSAndroidShutdown(OFC_VOID) ;
OFC_VOID BlueFSNUFileShutdown(OFC_VOID) ;
OFC_VOID BlueFSBookmarksShutdown(OFC_VOID) ;
OFC_VOID BlueFSMailslotShutdown (OFC_VOID) ;
OFC_VOID BlueFSPipeShutdown(OFC_VOID) ;

OFC_CORE_LIB OFC_VOID
BlueFSInit (OFC_VOID)
{
  BlueFSRegister (BLUE_FS_UNKNOWN, &BlueFSUnknown) ;
#if defined(OFC_FS_CIFS)
  BlueFSCIFSStartup () ;
#endif
#if defined (OFC_FS_WIN32)
  BlueFSWin32Startup() ;
#endif
#if defined (OFC_FS_WINCE)
  BlueFSWinCEStartup() ;
#endif
#if defined(OFC_FS_DARWIN)
  BlueFSDarwinStartup() ;
#endif
#if defined(OFC_FS_LINUX)
  BlueFSLinuxStartup() ;
#endif
#if defined(OFC_FS_FILEX)
  BlueFSFileXStartup() ;
#endif
#if defined(OFC_FS_ANDROID)
  BlueFSAndroidStartup() ;
#endif
#if defined(OFC_FS_NUFILE)
  BlueFSNUFileStartup() ;
#endif
#if defined(OFC_FS_BROWSER)
  BlueFSBrowseWorkgroupsStartup() ;
  BlueFSBrowseServersStartup() ;
  BlueFSBrowseSharesStartup() ;
#endif
#if defined(OFC_FS_BOOKMARKS)
  BlueFSBookmarksStartup() ;
#endif
#if defined(OFC_LANMAN)
  BlueFSMailslotStartup () ;
  BlueFSPipeStartup() ;
#endif
}

OFC_CORE_LIB OFC_VOID
BlueFSDestroy (OFC_VOID)
{
  BlueFSRegister (BLUE_FS_UNKNOWN, &BlueFSUnknown) ;
#if defined(OFC_FS_CIFS)
  BlueFSRegister (BLUE_FS_CIFS, &BlueFSUnknown);
#endif
#if defined (OFC_FS_WIN32)
  BlueFSRegister (BLUE_FS_WIN32, &BlueFSUnknown);
#endif
#if defined (OFC_FS_WINCE)
  BlueFSWinCEShutdown() ;
  BlueFSRegister (BLUE_FS_WINCE, &BlueFSUnknown);
#endif
#if defined(OFC_FS_DARWIN)
  BlueFSDarwinShutdown() ;
  BlueFSRegister (BLUE_FS_DARWIN, &BlueFSUnknown);
#endif
#if defined(OFC_FS_LINUX)
  BlueFSLinuxShutdown() ;
  BlueFSRegister (BLUE_FS_LINUX, &BlueFSUnknown);
#endif
#if defined(OFC_FS_FILEX)
  BlueFSFileXShutdown() ;
  BlueFSRegister (BLUE_FS_FILEX, &BlueFSUnknown);
#endif
#if defined(OFC_FS_ANDROID)
  BlueFSAndroidShutdown() ;
  BlueFSRegister (BLUE_FS_ANDROID, &BlueFSUnknown);
#endif
#if defined(OFC_FS_NUFILE)
  BlueFSNUFileShutdown() ;
  BlueFSRegister (BLUE_FS_NUFILE, &BlueFSUnknown);
#endif
#if defined(OFC_FS_BROWSER)
  BlueFSRegister (BLUE_FS_BROWSE_WORKGROUPS, &BlueFSUnknown);
  BlueFSRegister (BLUE_FS_BROWSE_SERVERS, &BlueFSUnknown);
  BlueFSRegister (BLUE_FS_BROWSE_SHARES, &BlueFSUnknown);
#endif
#if defined(OFC_FS_BOOKMARKS)
  BlueFSBookmarksShutdown() ;
  BlueFSRegister (BLUE_FS_BOOKMARKS, &BlueFSUnknown);
#endif
#if defined(OFC_LANMAN)
  BlueFSMailslotShutdown () ;
  BlueFSRegister (BLUE_FS_MAILSLOT, &BlueFSUnknown);
  BlueFSPipeShutdown() ;
  BlueFSRegister (BLUE_FS_PIPE, &BlueFSUnknown);
#endif
}

OFC_CORE_LIB OFC_VOID
BlueFSRegister (BLUE_FS_TYPE fsType, BLUE_FILE_FSINFO *fsInfo)
{
  BlueFSTable[fsType] = fsInfo ;
}

BLUE_HANDLE BlueFSCreateFile (BLUE_FS_TYPE fsType,
                              OFC_LPCTSTR lpFileName,
                              OFC_DWORD dwDesiredAccess,
                              OFC_DWORD dwShareMode,
                              OFC_LPSECURITY_ATTRIBUTES lpSecAttributes,
                              OFC_DWORD dwCreationDisposition,
                              OFC_DWORD dwFlagsAndAttributes,
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

OFC_BOOL BlueFSCreateDirectory (BLUE_FS_TYPE fsType,
                                OFC_LPCTSTR lpPathName,
                                OFC_LPSECURITY_ATTRIBUTES lpSecurityAttr)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->CreateDirectory (lpPathName, lpSecurityAttr) ;
  return (ret) ;
}

OFC_BOOL BlueFSWriteFile (BLUE_FS_TYPE fsType,
                          BLUE_HANDLE hFile,
                          OFC_LPCVOID lpBuffer,
                          OFC_DWORD nNumberOfBytesToWrite,
                          OFC_LPDWORD lpNumberOfBytesWritten,
                          BLUE_HANDLE hOverlapped)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->WriteFile (hFile,
					lpBuffer,
					nNumberOfBytesToWrite,
					lpNumberOfBytesWritten,
					hOverlapped) ;
  return (ret) ;
}

OFC_BOOL BlueFSReadFile (BLUE_FS_TYPE fsType,
                         BLUE_HANDLE hFile,
                         OFC_LPVOID lpBuffer,
                         OFC_DWORD nNumberOfBytesToRead,
                         OFC_LPDWORD lpNumberOfBytesRead,
                         BLUE_HANDLE hOverlapped)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->ReadFile (hFile,
				       lpBuffer,
				       nNumberOfBytesToRead,
				       lpNumberOfBytesRead,
				       hOverlapped) ;
  return (ret) ;
}

OFC_BOOL BlueFSCloseHandle (BLUE_FS_TYPE fsType,
                            BLUE_HANDLE hFile)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->CloseHandle (hFile) ;

  return (ret) ;
}

OFC_BOOL BlueFSDeleteFile (BLUE_FS_TYPE fsType, OFC_LPCTSTR lpFileName)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->DeleteFile (lpFileName) ;

  return (ret) ;
}

OFC_BOOL BlueFSRemoveDirectory (BLUE_FS_TYPE fsType, OFC_LPCTSTR lpPathName)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->RemoveDirectory (lpPathName) ;

  return (ret) ;
}

BLUE_HANDLE BlueFSFindFirstFile (BLUE_FS_TYPE fsType,
                                 OFC_LPCTSTR lpFileName,
                                 OFC_LPWIN32_FIND_DATAW lpFindFileData,
                                 OFC_BOOL *more)
{
  BLUE_HANDLE ret ;
  ret = BlueFSTable[fsType]->FindFirstFile (lpFileName, lpFindFileData, more) ;

  return (ret) ;
}

OFC_BOOL BlueFSFindNextFile (BLUE_FS_TYPE fsType,
                             BLUE_HANDLE hFindFile,
                             OFC_LPWIN32_FIND_DATAW lpFindFileData,
                             OFC_BOOL *more)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->FindNextFile (hFindFile, lpFindFileData, more) ;

  return (ret) ;
}

OFC_BOOL BlueFSFindClose (BLUE_FS_TYPE fsType, BLUE_HANDLE hFindFile)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->FindClose (hFindFile) ;

  return (ret) ;
}

OFC_BOOL BlueFSFlushFileBuffers (BLUE_FS_TYPE fsType, BLUE_HANDLE hFile)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->FlushFileBuffers (hFile) ;

  return (ret) ;
}

OFC_BOOL BlueFSGetFileAttributesEx (BLUE_FS_TYPE fsType,
                                    OFC_LPCTSTR lpFileName,
                                    OFC_GET_FILEEX_INFO_LEVELS fInfoLevelId,
                                    OFC_LPVOID lpFileInformation)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->GetFileAttributesEx (lpFileName,
						  fInfoLevelId,
						  lpFileInformation) ;

  return (ret) ;
}

OFC_BOOL BlueFSGetFileInformationByHandleEx (BLUE_FS_TYPE fsType,
                                             BLUE_HANDLE hFile,
                                             OFC_FILE_INFO_BY_HANDLE_CLASS
					      FileInformationClass,
                                             OFC_LPVOID lpFileInformation,
                                             OFC_DWORD dwBufferSize)
{
  OFC_BOOL ret ;
  ret = 
    BlueFSTable[fsType]->GetFileInformationByHandleEx (hFile,
						       FileInformationClass,
						       lpFileInformation,
						       dwBufferSize) ;

  return (ret) ;
}

OFC_BOOL BlueFSMoveFile (BLUE_FS_TYPE fsType,
                         OFC_LPCTSTR lpExistingFileName,
                         OFC_LPCTSTR lpNewFileName)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->MoveFile (lpExistingFileName, lpNewFileName) ;

  return (ret) ;
}

BLUE_HANDLE BlueFSCreateOverlapped (BLUE_FS_TYPE fsType)
{
  BLUE_HANDLE ret ;

  ret = BlueFSTable[fsType]->CreateOverlapped () ;
  return (ret) ;
}

OFC_VOID BlueFSDestroyOverlapped (BLUE_FS_TYPE fsType,
                                  BLUE_HANDLE hOverlapped)
{
  BlueFSTable[fsType]->DestroyOverlapped (hOverlapped) ;
}

OFC_VOID BlueFSSetOverlappedOffset (BLUE_FS_TYPE fsType,
                                    BLUE_HANDLE hOverlapped,
                                    OFC_OFFT offset)
{
  BlueFSTable[fsType]->SetOverlappedOffset (hOverlapped, offset) ;
}

OFC_BOOL BlueFSGetOverlappedResult (BLUE_FS_TYPE fsType,
                                    BLUE_HANDLE hFile,
                                    BLUE_HANDLE hOverlapped,
                                    OFC_LPDWORD lpNumberOfBytesTransferred,
                                    OFC_BOOL bWait)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->GetOverlappedResult (hFile,
						  hOverlapped,
						  lpNumberOfBytesTransferred,
						  bWait) ;

  return (ret) ;
}

OFC_BOOL BlueFSSetEndOfFile (BLUE_FS_TYPE fsType, BLUE_HANDLE hFile)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->SetEndOfFile (hFile) ;

  return (ret) ;
}

OFC_BOOL BlueFSSetFileAttributes (BLUE_FS_TYPE fsType,
                                  OFC_LPCTSTR lpFileName,
                                  OFC_DWORD dwFileAttributes)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->SetFileAttributes (lpFileName,
						dwFileAttributes) ;

  return (ret) ;
}

OFC_BOOL BlueFSSetFileInformationByHandle (BLUE_FS_TYPE fsType,
                                           BLUE_HANDLE hFile,
                                           OFC_FILE_INFO_BY_HANDLE_CLASS
					    FileInformationClass,
                                           OFC_LPVOID lpFileInformation,
                                           OFC_DWORD dwBufferSize)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->SetFileInformationByHandle (hFile,
							 FileInformationClass,
							 lpFileInformation,
							 dwBufferSize) ;

  return (ret) ;
}

OFC_DWORD BlueFSSetFilePointer (BLUE_FS_TYPE fsType,
                                BLUE_HANDLE hFile,
                                OFC_LONG lDistanceToMove,
                                OFC_PLONG lpDistanceToMoveHigh,
                                OFC_DWORD dwMoveMethod)
{
  OFC_DWORD ret ;
  ret = BlueFSTable[fsType]->SetFilePointer (hFile, lDistanceToMove,
					     lpDistanceToMoveHigh,
					     dwMoveMethod) ;

  return (ret) ;
}

OFC_BOOL BlueFSTransactNamedPipe (BLUE_FS_TYPE fsType,
                                  BLUE_HANDLE hFile,
                                  OFC_LPVOID lpInBuffer,
                                  OFC_DWORD nInBufferSize,
                                  OFC_LPVOID lpOutBuffer,
                                  OFC_DWORD nOutBufferSize,
                                  OFC_LPDWORD lpBytesRead,
                                  BLUE_HANDLE hOverlapped)
{
  OFC_BOOL ret ;

  ret = BlueFSTable[fsType]->TransactNamedPipe (hFile,
						lpInBuffer,
						nInBufferSize,
						lpOutBuffer,
						nOutBufferSize,
						lpBytesRead,
						hOverlapped) ;
  return (ret) ;
}

OFC_BOOL BlueFSDeviceIoControl (BLUE_FS_TYPE fsType,
                                BLUE_HANDLE hFile,
                                OFC_DWORD dwIoControlCode,
                                OFC_LPVOID lpInBuffer,
                                OFC_DWORD nInBufferSize,
                                OFC_LPVOID lpOutBuffer,
                                OFC_DWORD nOutBufferSize,
                                OFC_LPDWORD lpBytesRead,
                                BLUE_HANDLE hOverlapped)
{
  OFC_BOOL ret ;

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

OFC_BOOL BlueFSGetDiskFreeSpace (BLUE_FS_TYPE fsType,
                                 OFC_LPCTSTR lpRootPathName,
                                 OFC_LPDWORD lpSectorsPerCluster,
                                 OFC_LPDWORD lpBytesPerSector,
                                 OFC_LPDWORD lpNumberOfFreeClusters,
                                 OFC_LPDWORD lpTotalNumberOfClusters)
{
  OFC_BOOL ret ;

  ret = BlueFSTable[fsType]->GetDiskFreeSpace (lpRootPathName,
					       lpSectorsPerCluster,
					       lpBytesPerSector,
					       lpNumberOfFreeClusters,
					       lpTotalNumberOfClusters) ;
  return (ret) ;
}

OFC_BOOL BlueFSGetVolumeInformation (BLUE_FS_TYPE fsType,
                                     OFC_LPCTSTR lpRootPathName,
                                     OFC_LPTSTR lpVolumeNameBuffer,
                                     OFC_DWORD nVolumeNameSize,
                                     OFC_LPDWORD lpVolumeSerialNumber,
                                     OFC_LPDWORD lpMaximumComponentLength,
                                     OFC_LPDWORD lpFileSystemFlags,
                                     OFC_LPTSTR lpFileSystemName,
                                     OFC_DWORD nFileSystemName)
{
  OFC_BOOL ret ;

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
 * OFC_TRUE if successful, OFC_FALSE otherwise
 */
OFC_BOOL BlueFSUnlockFileEx (BLUE_FS_TYPE fsType,
                             BLUE_HANDLE hFile,
                             OFC_UINT32 length_low,
                             OFC_UINT32 length_high,
                             BLUE_HANDLE hOverlapped)
{
  OFC_BOOL ret ;
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
 * OFC_TRUE if successful, OFC_FALSE otherwise
 */
OFC_BOOL BlueFSLockFileEx (BLUE_FS_TYPE fsType,
                           BLUE_HANDLE hFile, OFC_DWORD flags,
                           OFC_DWORD length_low, OFC_DWORD length_high,
                           BLUE_HANDLE hOverlapped)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->LockFileEx (hFile, flags,
					 length_low, length_high,
					 hOverlapped) ;
  return (ret) ;
}

OFC_BOOL BlueFSDismount (BLUE_FS_TYPE fsType,
                         OFC_LPCTSTR lpFileName)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->Dismount (lpFileName) ;

  return (ret) ;
}

