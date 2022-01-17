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

static OFC_FILE_FSINFO *ofc_fs_table[OFC_FST_NUM];

static OFC_HANDLE ofc_fs_unknown_handle_inv(OFC_VOID) {
    return (OFC_INVALID_HANDLE_VALUE);
}

static OFC_HANDLE ofc_fs_unknown_handle_null(OFC_VOID) {
    return (OFC_HANDLE_NULL);
}

static OFC_BOOL ofc_fs_unknown_bool(OFC_VOID) {
    return (OFC_FALSE);
}

static OFC_DWORD ofc_fs_unknown_dword(OFC_VOID) {
    return (OFC_INVALID_SET_FILE_POINTER);
}

static OFC_VOID ofc_fs_unknown_void(OFC_VOID) {
}

static OFC_FILE_FSINFO ofc_fs_unknown =
        {
                (OFC_HANDLE (*)(OFC_LPCTSTR,
                                OFC_DWORD,
                                OFC_DWORD,
                                OFC_LPSECURITY_ATTRIBUTES,
                                OFC_DWORD,
                                OFC_DWORD,
                                OFC_HANDLE)) &ofc_fs_unknown_handle_inv,
                (OFC_BOOL (*)(OFC_LPCTSTR)) &ofc_fs_unknown_bool,
                (OFC_HANDLE (*)(OFC_LPCTSTR,
                                OFC_LPWIN32_FIND_DATAW,
                                OFC_BOOL *)) &ofc_fs_unknown_handle_inv,
                (OFC_BOOL (*)(OFC_HANDLE,
                              OFC_LPWIN32_FIND_DATAW,
                              OFC_BOOL *)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_HANDLE)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_HANDLE)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_LPCTSTR,
                              OFC_GET_FILEEX_INFO_LEVELS,
                              OFC_LPVOID)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_HANDLE,
                              OFC_FILE_INFO_BY_HANDLE_CLASS,
                              OFC_LPVOID,
                              OFC_DWORD)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_LPCTSTR,
                              OFC_LPCTSTR)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_HANDLE,
                              OFC_HANDLE,
                              OFC_LPDWORD,
                              OFC_BOOL)) &ofc_fs_unknown_bool,
                (OFC_HANDLE (*)(OFC_VOID)) &ofc_fs_unknown_handle_null,
                (OFC_VOID (*)(OFC_HANDLE)) &ofc_fs_unknown_void,
                (OFC_VOID (*)(OFC_HANDLE,
                              OFC_OFFT)) &ofc_fs_unknown_void,
                (OFC_BOOL (*)(OFC_HANDLE)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_LPCTSTR,
                              OFC_DWORD)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_HANDLE,
                              OFC_FILE_INFO_BY_HANDLE_CLASS,
                              OFC_LPVOID,
                              OFC_DWORD)) &ofc_fs_unknown_bool,
                (OFC_DWORD (*)(OFC_HANDLE,
                               OFC_LONG,
                               OFC_PLONG,
                               OFC_DWORD)) &ofc_fs_unknown_dword,
                (OFC_BOOL (*)(OFC_HANDLE,
                              OFC_LPCVOID,
                              OFC_DWORD,
                              OFC_LPDWORD,
                              OFC_HANDLE)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_HANDLE,
                              OFC_LPVOID,
                              OFC_DWORD,
                              OFC_LPDWORD,
                              OFC_HANDLE)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_HANDLE)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_HANDLE,
                              OFC_LPVOID,
                              OFC_DWORD,
                              OFC_LPVOID,
                              OFC_DWORD,
                              OFC_LPDWORD,
                              OFC_HANDLE)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_LPCTSTR,
                              OFC_LPDWORD,
                              OFC_LPDWORD,
                              OFC_LPDWORD,
                              OFC_LPDWORD)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_LPCTSTR,
                              OFC_LPTSTR,
                              OFC_DWORD,
                              OFC_LPDWORD,
                              OFC_LPDWORD,
                              OFC_LPDWORD,
                              OFC_LPTSTR,
                              OFC_DWORD)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_LPCTSTR,
                              OFC_LPSECURITY_ATTRIBUTES)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_LPCTSTR)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_HANDLE,
                              OFC_UINT32,
                              OFC_UINT32,
                              OFC_HANDLE)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_HANDLE,
                              OFC_DWORD,
                              OFC_DWORD,
                              OFC_DWORD,
                              OFC_HANDLE)) &ofc_fs_unknown_bool,
                (OFC_BOOL (*)(OFC_LPCTSTR)) &ofc_fs_unknown_bool
        };

OFC_VOID OfcFSCIFSStartup(OFC_VOID);

OFC_VOID OfcFSWin32Startup(OFC_VOID);

OFC_VOID OfcFSWinCEStartup(OFC_VOID);

OFC_VOID OfcFSDarwinStartup(OFC_VOID);

OFC_VOID OfcFSLinuxStartup(OFC_VOID);

OFC_VOID OfcFSFileXStartup(OFC_VOID);

OFC_VOID OfcFSNUFileStartup(OFC_VOID);

OFC_VOID OfcFSBrowseWorkgroupsStartup(OFC_VOID);

OFC_VOID OfcFSBrowseServersStartup(OFC_VOID);

OFC_VOID OfcFSBrowseSharesStartup(OFC_VOID);

OFC_VOID OfcFSMailslotStartup(OFC_VOID);

OFC_VOID OfcFSPipeStartup(OFC_VOID);

OFC_VOID OfcFSBookmarksStartup(OFC_VOID);

OFC_VOID OfcFSAndroidStartup(OFC_VOID);

OFC_VOID OfcFSWinCEShutdown(OFC_VOID);

OFC_VOID OfcFSDarwinShutdown(OFC_VOID);

OFC_VOID OfcFSLinuxShutdown(OFC_VOID);

OFC_VOID OfcFSFileXShutdown(OFC_VOID);

OFC_VOID OfcFSAndroidShutdown(OFC_VOID);

OFC_VOID OfcFSNUFileShutdown(OFC_VOID);

OFC_VOID OfcFSBookmarksShutdown(OFC_VOID);

OFC_VOID OfcFSMailslotShutdown(OFC_VOID);

OFC_VOID OfcFSPipeShutdown(OFC_VOID);

OFC_CORE_LIB OFC_VOID
ofc_fs_init(OFC_VOID) {
    ofc_fs_register(OFC_FST_UNKNOWN, &ofc_fs_unknown);
#if defined (OFC_FS_WIN32)
    OfcFSWin32Startup() ;
#endif
#if defined (OFC_FS_WINCE)
    OfcFSWinCEStartup() ;
#endif
#if defined(OFC_FS_DARWIN)
    OfcFSDarwinStartup();
#endif
#if defined(OFC_FS_LINUX)
    OfcFSLinuxStartup() ;
#endif
#if defined(OFC_FS_FILEX)
    OfcFSFileXStartup() ;
#endif
#if defined(OFC_FS_ANDROID)
    OfcFSAndroidStartup() ;
#endif
#if defined(OFC_FS_NUFILE)
    OfcFSNUFileStartup() ;
#endif
#if defined(OFC_FS_BROWSER)
    OfcFSBrowseWorkgroupsStartup() ;
    OfcFSBrowseServersStartup() ;
    OfcFSBrowseSharesStartup() ;
#endif
#if defined(OFC_FS_BOOKMARKS)
    OfcFSBookmarksStartup();
#endif
#if defined(OFC_LANMAN)
    OfcFSMailslotStartup () ;
    OfcFSPipeStartup() ;
#endif
}

OFC_CORE_LIB OFC_VOID
ofc_fs_destroy(OFC_VOID) {
    ofc_fs_register(OFC_FST_UNKNOWN, &ofc_fs_unknown);
#if defined (OFC_FS_WIN32)
    ofc_fs_register (OFC_FST_WIN32, &ofc_fs_unknown);
#endif
#if defined (OFC_FS_WINCE)
    OfcFSWinCEShutdown() ;
    ofc_fs_register (OFC_FST_WINCE, &ofc_fs_unknown);
#endif
#if defined(OFC_FS_DARWIN)
    OfcFSDarwinShutdown();
    ofc_fs_register(OFC_FST_DARWIN, &ofc_fs_unknown);
#endif
#if defined(OFC_FS_LINUX)
    OfcFSLinuxShutdown() ;
    ofc_fs_register (OFC_FST_LINUX, &ofc_fs_unknown);
#endif
#if defined(OFC_FS_FILEX)
    OfcFSFileXShutdown() ;
    ofc_fs_register (OFC_FST_FILEX, &ofc_fs_unknown);
#endif
#if defined(OFC_FS_ANDROID)
    OfcFSAndroidShutdown() ;
    ofc_fs_register (OFC_FST_ANDROID, &ofc_fs_unknown);
#endif
#if defined(OFC_FS_NUFILE)
    OfcFSNUFileShutdown() ;
    ofc_fs_register (OFC_FST_NUFILE, &ofc_fs_unknown);
#endif
#if defined(OFC_FS_BROWSER)
    ofc_fs_register (OFC_FST_BROWSE_WORKGROUPS, &ofc_fs_unknown);
    ofc_fs_register (OFC_FST_BROWSE_SERVERS, &ofc_fs_unknown);
    ofc_fs_register (OFC_FST_BROWSE_SHARES, &ofc_fs_unknown);
#endif
#if defined(OFC_FS_BOOKMARKS)
    OfcFSBookmarksShutdown();
    ofc_fs_register(OFC_FST_BOOKMARKS, &ofc_fs_unknown);
#endif
#if defined(OFC_LANMAN)
    OfcFSMailslotShutdown () ;
    ofc_fs_register (OFC_FST_MAILSLOT, &ofc_fs_unknown);
    OfcFSPipeShutdown() ;
    ofc_fs_register (OFC_FST_PIPE, &ofc_fs_unknown);
#endif
}

OFC_CORE_LIB OFC_VOID
ofc_fs_register(OFC_FST_TYPE fsType, OFC_FILE_FSINFO *fsInfo) {
    ofc_fs_table[fsType] = fsInfo;
}

OFC_CORE_LIB OFC_VOID
ofc_fs_deregister(OFC_FST_TYPE fsType) {
    ofc_fs_table[fsType] = &ofc_fs_unknown;
}

OFC_HANDLE OfcFSCreateFile(OFC_FST_TYPE fsType,
                           OFC_LPCTSTR lpFileName,
                           OFC_DWORD dwDesiredAccess,
                           OFC_DWORD dwShareMode,
                           OFC_LPSECURITY_ATTRIBUTES lpSecAttributes,
                           OFC_DWORD dwCreationDisposition,
                           OFC_DWORD dwFlagsAndAttributes,
                           OFC_HANDLE hTemplateFile) {
    OFC_HANDLE ret;
    ret = ofc_fs_table[fsType]->CreateFile(lpFileName,
                                           dwDesiredAccess,
                                           dwShareMode,
                                           lpSecAttributes,
                                           dwCreationDisposition,
                                           dwFlagsAndAttributes,
                                           hTemplateFile);
    return (ret);
}

OFC_BOOL OfcFSCreateDirectory(OFC_FST_TYPE fsType,
                              OFC_LPCTSTR lpPathName,
                              OFC_LPSECURITY_ATTRIBUTES lpSecurityAttr) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->CreateDirectory(lpPathName, lpSecurityAttr);
    return (ret);
}

OFC_BOOL OfcFSWriteFile(OFC_FST_TYPE fsType,
                        OFC_HANDLE hFile,
                        OFC_LPCVOID lpBuffer,
                        OFC_DWORD nNumberOfBytesToWrite,
                        OFC_LPDWORD lpNumberOfBytesWritten,
                        OFC_HANDLE hOverlapped) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->WriteFile(hFile,
                                          lpBuffer,
                                          nNumberOfBytesToWrite,
                                          lpNumberOfBytesWritten,
                                          hOverlapped);
    return (ret);
}

OFC_BOOL OfcFSReadFile(OFC_FST_TYPE fsType,
                       OFC_HANDLE hFile,
                       OFC_LPVOID lpBuffer,
                       OFC_DWORD nNumberOfBytesToRead,
                       OFC_LPDWORD lpNumberOfBytesRead,
                       OFC_HANDLE hOverlapped) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->ReadFile(hFile,
                                         lpBuffer,
                                         nNumberOfBytesToRead,
                                         lpNumberOfBytesRead,
                                         hOverlapped);
    return (ret);
}

OFC_BOOL OfcFSCloseHandle(OFC_FST_TYPE fsType,
                          OFC_HANDLE hFile) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->CloseHandle(hFile);

    return (ret);
}

OFC_BOOL OfcFSDeleteFile(OFC_FST_TYPE fsType, OFC_LPCTSTR lpFileName) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->DeleteFile(lpFileName);

    return (ret);
}

OFC_BOOL OfcFSRemoveDirectory(OFC_FST_TYPE fsType, OFC_LPCTSTR lpPathName) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->RemoveDirectory(lpPathName);

    return (ret);
}

OFC_HANDLE OfcFSFindFirstFile(OFC_FST_TYPE fsType,
                              OFC_LPCTSTR lpFileName,
                              OFC_LPWIN32_FIND_DATAW lpFindFileData,
                              OFC_BOOL *more) {
    OFC_HANDLE ret;
    ret = ofc_fs_table[fsType]->FindFirstFile(lpFileName, lpFindFileData, more);

    return (ret);
}

OFC_BOOL OfcFSFindNextFile(OFC_FST_TYPE fsType,
                           OFC_HANDLE hFindFile,
                           OFC_LPWIN32_FIND_DATAW lpFindFileData,
                           OFC_BOOL *more) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->FindNextFile(hFindFile, lpFindFileData, more);

    return (ret);
}

OFC_BOOL OfcFSFindClose(OFC_FST_TYPE fsType, OFC_HANDLE hFindFile) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->FindClose(hFindFile);

    return (ret);
}

OFC_BOOL OfcFSFlushFileBuffers(OFC_FST_TYPE fsType, OFC_HANDLE hFile) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->FlushFileBuffers(hFile);

    return (ret);
}

OFC_BOOL OfcFSGetFileAttributesEx(OFC_FST_TYPE fsType,
                                  OFC_LPCTSTR lpFileName,
                                  OFC_GET_FILEEX_INFO_LEVELS fInfoLevelId,
                                  OFC_LPVOID lpFileInformation) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->GetFileAttributesEx(lpFileName,
                                                    fInfoLevelId,
                                                    lpFileInformation);

    return (ret);
}

OFC_BOOL OfcFSGetFileInformationByHandleEx(OFC_FST_TYPE fsType,
                                           OFC_HANDLE hFile,
                                           OFC_FILE_INFO_BY_HANDLE_CLASS
                                           FileInformationClass,
                                           OFC_LPVOID lpFileInformation,
                                           OFC_DWORD dwBufferSize) {
    OFC_BOOL ret;
    ret =
            ofc_fs_table[fsType]->GetFileInformationByHandleEx(hFile,
                                                               FileInformationClass,
                                                               lpFileInformation,
                                                               dwBufferSize);

    return (ret);
}

OFC_BOOL OfcFSMoveFile(OFC_FST_TYPE fsType,
                       OFC_LPCTSTR lpExistingFileName,
                       OFC_LPCTSTR lpNewFileName) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->MoveFile(lpExistingFileName, lpNewFileName);

    return (ret);
}

OFC_HANDLE OfcFSCreateOverlapped(OFC_FST_TYPE fsType) {
    OFC_HANDLE ret;

    ret = ofc_fs_table[fsType]->CreateOverlapped();
    return (ret);
}

OFC_VOID OfcFSDestroyOverlapped(OFC_FST_TYPE fsType,
                                OFC_HANDLE hOverlapped) {
    ofc_fs_table[fsType]->DestroyOverlapped(hOverlapped);
}

OFC_VOID OfcFSSetOverlappedOffset(OFC_FST_TYPE fsType,
                                  OFC_HANDLE hOverlapped,
                                  OFC_OFFT offset) {
    ofc_fs_table[fsType]->SetOverlappedOffset(hOverlapped, offset);
}

OFC_BOOL OfcFSGetOverlappedResult(OFC_FST_TYPE fsType,
                                  OFC_HANDLE hFile,
                                  OFC_HANDLE hOverlapped,
                                  OFC_LPDWORD lpNumberOfBytesTransferred,
                                  OFC_BOOL bWait) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->GetOverlappedResult(hFile,
                                                    hOverlapped,
                                                    lpNumberOfBytesTransferred,
                                                    bWait);

    return (ret);
}

OFC_BOOL OfcFSSetEndOfFile(OFC_FST_TYPE fsType, OFC_HANDLE hFile) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->SetEndOfFile(hFile);

    return (ret);
}

OFC_BOOL OfcFSSetFileAttributes(OFC_FST_TYPE fsType,
                                OFC_LPCTSTR lpFileName,
                                OFC_DWORD dwFileAttributes) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->SetFileAttributes(lpFileName,
                                                  dwFileAttributes);

    return (ret);
}

OFC_BOOL OfcFSSetFileInformationByHandle(OFC_FST_TYPE fsType,
                                         OFC_HANDLE hFile,
                                         OFC_FILE_INFO_BY_HANDLE_CLASS
                                         FileInformationClass,
                                         OFC_LPVOID lpFileInformation,
                                         OFC_DWORD dwBufferSize) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->SetFileInformationByHandle(hFile,
                                                           FileInformationClass,
                                                           lpFileInformation,
                                                           dwBufferSize);

    return (ret);
}

OFC_DWORD OfcFSSetFilePointer(OFC_FST_TYPE fsType,
                              OFC_HANDLE hFile,
                              OFC_LONG lDistanceToMove,
                              OFC_PLONG lpDistanceToMoveHigh,
                              OFC_DWORD dwMoveMethod) {
    OFC_DWORD ret;
    ret = ofc_fs_table[fsType]->SetFilePointer(hFile, lDistanceToMove,
                                               lpDistanceToMoveHigh,
                                               dwMoveMethod);

    return (ret);
}

OFC_BOOL OfcFSTransactNamedPipe(OFC_FST_TYPE fsType,
                                OFC_HANDLE hFile,
                                OFC_LPVOID lpInBuffer,
                                OFC_DWORD nInBufferSize,
                                OFC_LPVOID lpOutBuffer,
                                OFC_DWORD nOutBufferSize,
                                OFC_LPDWORD lpBytesRead,
                                OFC_HANDLE hOverlapped) {
    OFC_BOOL ret;

    ret = ofc_fs_table[fsType]->TransactNamedPipe(hFile,
                                                  lpInBuffer,
                                                  nInBufferSize,
                                                  lpOutBuffer,
                                                  nOutBufferSize,
                                                  lpBytesRead,
                                                  hOverlapped);
    return (ret);
}

OFC_BOOL OfcFSDeviceIoControl(OFC_FST_TYPE fsType,
                              OFC_HANDLE hFile,
                              OFC_DWORD dwIoControlCode,
                              OFC_LPVOID lpInBuffer,
                              OFC_DWORD nInBufferSize,
                              OFC_LPVOID lpOutBuffer,
                              OFC_DWORD nOutBufferSize,
                              OFC_LPDWORD lpBytesRead,
                              OFC_HANDLE hOverlapped) {
    OFC_BOOL ret;

    ret = ofc_fs_table[fsType]->DeviceIoControl(hFile,
                                                dwIoControlCode,
                                                lpInBuffer,
                                                nInBufferSize,
                                                lpOutBuffer,
                                                nOutBufferSize,
                                                lpBytesRead,
                                                hOverlapped);
    return (ret);
}

OFC_BOOL OfcFSGetDiskFreeSpace(OFC_FST_TYPE fsType,
                               OFC_LPCTSTR lpRootPathName,
                               OFC_LPDWORD lpSectorsPerCluster,
                               OFC_LPDWORD lpBytesPerSector,
                               OFC_LPDWORD lpNumberOfFreeClusters,
                               OFC_LPDWORD lpTotalNumberOfClusters) {
    OFC_BOOL ret;

    ret = ofc_fs_table[fsType]->GetDiskFreeSpace(lpRootPathName,
                                                 lpSectorsPerCluster,
                                                 lpBytesPerSector,
                                                 lpNumberOfFreeClusters,
                                                 lpTotalNumberOfClusters);
    return (ret);
}

OFC_BOOL OfcFSGetVolumeInformation(OFC_FST_TYPE fsType,
                                   OFC_LPCTSTR lpRootPathName,
                                   OFC_LPTSTR lpVolumeNameBuffer,
                                   OFC_DWORD nVolumeNameSize,
                                   OFC_LPDWORD lpVolumeSerialNumber,
                                   OFC_LPDWORD lpMaximumComponentLength,
                                   OFC_LPDWORD lpFileSystemFlags,
                                   OFC_LPTSTR lpFileSystemName,
                                   OFC_DWORD nFileSystemName) {
    OFC_BOOL ret;

    ret = ofc_fs_table[fsType]->GetVolumeInformation(lpRootPathName,
                                                     lpVolumeNameBuffer,
                                                     nVolumeNameSize,
                                                     lpVolumeSerialNumber,
                                                     lpMaximumComponentLength,
                                                     lpFileSystemFlags,
                                                     lpFileSystemName,
                                                     nFileSystemName);
    return (ret);
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
OFC_BOOL OfcFSUnlockFileEx(OFC_FST_TYPE fsType,
                           OFC_HANDLE hFile,
                           OFC_UINT32 length_low,
                           OFC_UINT32 length_high,
                           OFC_HANDLE hOverlapped) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->UnlockFileEx(hFile,
                                             length_low, length_high,
                                             hOverlapped);
    return (ret);
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
OFC_BOOL OfcFSLockFileEx(OFC_FST_TYPE fsType,
                         OFC_HANDLE hFile, OFC_DWORD flags,
                         OFC_DWORD length_low, OFC_DWORD length_high,
                         OFC_HANDLE hOverlapped) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->LockFileEx(hFile, flags,
                                           length_low, length_high,
                                           hOverlapped);
    return (ret);
}

OFC_BOOL OfcFSDismount(OFC_FST_TYPE fsType,
                       OFC_LPCTSTR lpFileName) {
    OFC_BOOL ret;
    ret = ofc_fs_table[fsType]->Dismount(lpFileName);

    return (ret);
}

