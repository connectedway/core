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

static OFC_FILE_FSINFO *BlueFSTable[OFC_FST_NUM] ;

static OFC_HANDLE BlueFSUnknownHandleInv (OFC_VOID)
{
  return (OFC_INVALID_HANDLE_VALUE) ;
}

static OFC_HANDLE BlueFSUnknownHandleNull (OFC_VOID)
{
  return (OFC_HANDLE_NULL) ;
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

static OFC_FILE_FSINFO BlueFSUnknown =
  {
    (OFC_HANDLE (*)(OFC_LPCTSTR,
                    OFC_DWORD,
                    OFC_DWORD,
                    OFC_LPSECURITY_ATTRIBUTES,
                    OFC_DWORD,
                    OFC_DWORD,
                    OFC_HANDLE)) &BlueFSUnknownHandleInv,
    (OFC_BOOL (*)(OFC_LPCTSTR)) &BlueFSUnknownBool,
    (OFC_HANDLE (*)(OFC_LPCTSTR,
                    OFC_LPWIN32_FIND_DATAW,
                    OFC_BOOL *)) &BlueFSUnknownHandleInv,
    (OFC_BOOL (*)(OFC_HANDLE,
                  OFC_LPWIN32_FIND_DATAW,
                  OFC_BOOL *)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_HANDLE)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_HANDLE)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_LPCTSTR,
                  OFC_GET_FILEEX_INFO_LEVELS,
                  OFC_LPVOID)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_HANDLE,
                  OFC_FILE_INFO_BY_HANDLE_CLASS,
                  OFC_LPVOID,
                  OFC_DWORD)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_LPCTSTR,
                  OFC_LPCTSTR)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_HANDLE,
                  OFC_HANDLE,
                  OFC_LPDWORD,
                  OFC_BOOL)) &BlueFSUnknownBool,
    (OFC_HANDLE (*)(OFC_VOID)) &BlueFSUnknownHandleNull,
    (OFC_VOID (*)(OFC_HANDLE)) &BlueFSUnknownVoid,
    (OFC_VOID (*)(OFC_HANDLE,
                  OFC_OFFT)) &BlueFSUnknownVoid,
    (OFC_BOOL (*)(OFC_HANDLE)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_LPCTSTR,
                  OFC_DWORD)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_HANDLE,
                  OFC_FILE_INFO_BY_HANDLE_CLASS,
                  OFC_LPVOID,
                  OFC_DWORD)) &BlueFSUnknownBool,
    (OFC_DWORD (*)(OFC_HANDLE,
                   OFC_LONG,
                   OFC_PLONG,
                   OFC_DWORD)) &BlueFSUnknownDword,
    (OFC_BOOL (*)(OFC_HANDLE,
                  OFC_LPCVOID,
                  OFC_DWORD,
                  OFC_LPDWORD,
                  OFC_HANDLE)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_HANDLE,
                  OFC_LPVOID,
                  OFC_DWORD,
                  OFC_LPDWORD,
                  OFC_HANDLE)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_HANDLE)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_HANDLE,
                  OFC_LPVOID,
                  OFC_DWORD,
                  OFC_LPVOID,
                  OFC_DWORD,
                  OFC_LPDWORD,
                  OFC_HANDLE)) &BlueFSUnknownBool,
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
    (OFC_BOOL (*)(OFC_HANDLE,
                  OFC_UINT32,
                  OFC_UINT32,
                  OFC_HANDLE)) &BlueFSUnknownBool,
    (OFC_BOOL (*)(OFC_HANDLE,
                  OFC_DWORD,
                  OFC_DWORD,
                  OFC_DWORD,
                  OFC_HANDLE))&BlueFSUnknownBool,
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
ofc_fs_init (OFC_VOID)
{
  ofc_fs_register (OFC_FST_UNKNOWN, &BlueFSUnknown) ;
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
ofc_fs_destroy (OFC_VOID)
{
  ofc_fs_register (OFC_FST_UNKNOWN, &BlueFSUnknown) ;
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
  ofc_fs_register (OFC_FST_DARWIN, &BlueFSUnknown);
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
  ofc_fs_register (OFC_FST_BOOKMARKS, &BlueFSUnknown);
#endif
#if defined(OFC_LANMAN)
  BlueFSMailslotShutdown () ;
  BlueFSRegister (BLUE_FS_MAILSLOT, &BlueFSUnknown);
  BlueFSPipeShutdown() ;
  BlueFSRegister (BLUE_FS_PIPE, &BlueFSUnknown);
#endif
}

OFC_CORE_LIB OFC_VOID
ofc_fs_register (OFC_FST_TYPE fsType, OFC_FILE_FSINFO *fsInfo)
{
  BlueFSTable[fsType] = fsInfo ;
}

OFC_HANDLE OfcFSCreateFile (OFC_FST_TYPE fsType,
                            OFC_LPCTSTR lpFileName,
                            OFC_DWORD dwDesiredAccess,
                            OFC_DWORD dwShareMode,
                            OFC_LPSECURITY_ATTRIBUTES lpSecAttributes,
                            OFC_DWORD dwCreationDisposition,
                            OFC_DWORD dwFlagsAndAttributes,
                            OFC_HANDLE hTemplateFile)
{
  OFC_HANDLE ret ;
  ret = BlueFSTable[fsType]->CreateFile (lpFileName,
					 dwDesiredAccess,
					 dwShareMode,
					 lpSecAttributes,
					 dwCreationDisposition,
					 dwFlagsAndAttributes,
					 hTemplateFile) ;
  return (ret) ;
}

OFC_BOOL OfcFSCreateDirectory (OFC_FST_TYPE fsType,
                               OFC_LPCTSTR lpPathName,
                               OFC_LPSECURITY_ATTRIBUTES lpSecurityAttr)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->CreateDirectory (lpPathName, lpSecurityAttr) ;
  return (ret) ;
}

OFC_BOOL OfcFSWriteFile (OFC_FST_TYPE fsType,
                         OFC_HANDLE hFile,
                         OFC_LPCVOID lpBuffer,
                         OFC_DWORD nNumberOfBytesToWrite,
                         OFC_LPDWORD lpNumberOfBytesWritten,
                         OFC_HANDLE hOverlapped)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->WriteFile (hFile,
					lpBuffer,
					nNumberOfBytesToWrite,
					lpNumberOfBytesWritten,
					hOverlapped) ;
  return (ret) ;
}

OFC_BOOL OfcFSReadFile (OFC_FST_TYPE fsType,
                        OFC_HANDLE hFile,
                        OFC_LPVOID lpBuffer,
                        OFC_DWORD nNumberOfBytesToRead,
                        OFC_LPDWORD lpNumberOfBytesRead,
                        OFC_HANDLE hOverlapped)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->ReadFile (hFile,
				       lpBuffer,
				       nNumberOfBytesToRead,
				       lpNumberOfBytesRead,
				       hOverlapped) ;
  return (ret) ;
}

OFC_BOOL OfcFSCloseHandle (OFC_FST_TYPE fsType,
                           OFC_HANDLE hFile)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->CloseHandle (hFile) ;

  return (ret) ;
}

OFC_BOOL OfcFSDeleteFile (OFC_FST_TYPE fsType, OFC_LPCTSTR lpFileName)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->DeleteFile (lpFileName) ;

  return (ret) ;
}

OFC_BOOL OfcFSRemoveDirectory (OFC_FST_TYPE fsType, OFC_LPCTSTR lpPathName)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->RemoveDirectory (lpPathName) ;

  return (ret) ;
}

OFC_HANDLE OfcFSFindFirstFile (OFC_FST_TYPE fsType,
                               OFC_LPCTSTR lpFileName,
                               OFC_LPWIN32_FIND_DATAW lpFindFileData,
                               OFC_BOOL *more)
{
  OFC_HANDLE ret ;
  ret = BlueFSTable[fsType]->FindFirstFile (lpFileName, lpFindFileData, more) ;

  return (ret) ;
}

OFC_BOOL OfcFSFindNextFile (OFC_FST_TYPE fsType,
                            OFC_HANDLE hFindFile,
                            OFC_LPWIN32_FIND_DATAW lpFindFileData,
                            OFC_BOOL *more)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->FindNextFile (hFindFile, lpFindFileData, more) ;

  return (ret) ;
}

OFC_BOOL OfcFSFindClose (OFC_FST_TYPE fsType, OFC_HANDLE hFindFile)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->FindClose (hFindFile) ;

  return (ret) ;
}

OFC_BOOL OfcFSFlushFileBuffers (OFC_FST_TYPE fsType, OFC_HANDLE hFile)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->FlushFileBuffers (hFile) ;

  return (ret) ;
}

OFC_BOOL OfcFSGetFileAttributesEx (OFC_FST_TYPE fsType,
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

OFC_BOOL OfcFSGetFileInformationByHandleEx (OFC_FST_TYPE fsType,
                                            OFC_HANDLE hFile,
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

OFC_BOOL OfcFSMoveFile (OFC_FST_TYPE fsType,
                        OFC_LPCTSTR lpExistingFileName,
                        OFC_LPCTSTR lpNewFileName)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->MoveFile (lpExistingFileName, lpNewFileName) ;

  return (ret) ;
}

OFC_HANDLE OfcFSCreateOverlapped (OFC_FST_TYPE fsType)
{
  OFC_HANDLE ret ;

  ret = BlueFSTable[fsType]->CreateOverlapped () ;
  return (ret) ;
}

OFC_VOID OfcFSDestroyOverlapped (OFC_FST_TYPE fsType,
                                 OFC_HANDLE hOverlapped)
{
  BlueFSTable[fsType]->DestroyOverlapped (hOverlapped) ;
}

OFC_VOID OfcFSSetOverlappedOffset (OFC_FST_TYPE fsType,
                                   OFC_HANDLE hOverlapped,
                                   OFC_OFFT offset)
{
  BlueFSTable[fsType]->SetOverlappedOffset (hOverlapped, offset) ;
}

OFC_BOOL OfcFSGetOverlappedResult (OFC_FST_TYPE fsType,
                                   OFC_HANDLE hFile,
                                   OFC_HANDLE hOverlapped,
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

OFC_BOOL OfcFSSetEndOfFile (OFC_FST_TYPE fsType, OFC_HANDLE hFile)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->SetEndOfFile (hFile) ;

  return (ret) ;
}

OFC_BOOL OfcFSSetFileAttributes (OFC_FST_TYPE fsType,
                                 OFC_LPCTSTR lpFileName,
                                 OFC_DWORD dwFileAttributes)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->SetFileAttributes (lpFileName,
						dwFileAttributes) ;

  return (ret) ;
}

OFC_BOOL OfcFSSetFileInformationByHandle (OFC_FST_TYPE fsType,
                                          OFC_HANDLE hFile,
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

OFC_DWORD OfcFSSetFilePointer (OFC_FST_TYPE fsType,
                               OFC_HANDLE hFile,
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

OFC_BOOL OfcFSTransactNamedPipe (OFC_FST_TYPE fsType,
                                 OFC_HANDLE hFile,
                                 OFC_LPVOID lpInBuffer,
                                 OFC_DWORD nInBufferSize,
                                 OFC_LPVOID lpOutBuffer,
                                 OFC_DWORD nOutBufferSize,
                                 OFC_LPDWORD lpBytesRead,
                                 OFC_HANDLE hOverlapped)
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

OFC_BOOL OfcFSDeviceIoControl (OFC_FST_TYPE fsType,
                               OFC_HANDLE hFile,
                               OFC_DWORD dwIoControlCode,
                               OFC_LPVOID lpInBuffer,
                               OFC_DWORD nInBufferSize,
                               OFC_LPVOID lpOutBuffer,
                               OFC_DWORD nOutBufferSize,
                               OFC_LPDWORD lpBytesRead,
                               OFC_HANDLE hOverlapped)
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

OFC_BOOL OfcFSGetDiskFreeSpace (OFC_FST_TYPE fsType,
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

OFC_BOOL OfcFSGetVolumeInformation (OFC_FST_TYPE fsType,
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
OFC_BOOL OfcFSUnlockFileEx (OFC_FST_TYPE fsType,
                            OFC_HANDLE hFile,
                            OFC_UINT32 length_low,
                            OFC_UINT32 length_high,
                            OFC_HANDLE hOverlapped)
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
OFC_BOOL OfcFSLockFileEx (OFC_FST_TYPE fsType,
                          OFC_HANDLE hFile, OFC_DWORD flags,
                          OFC_DWORD length_low, OFC_DWORD length_high,
                          OFC_HANDLE hOverlapped)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->LockFileEx (hFile, flags,
					 length_low, length_high,
					 hOverlapped) ;
  return (ret) ;
}

OFC_BOOL OfcFSDismount (OFC_FST_TYPE fsType,
                        OFC_LPCTSTR lpFileName)
{
  OFC_BOOL ret ;
  ret = BlueFSTable[fsType]->Dismount (lpFileName) ;

  return (ret) ;
}

