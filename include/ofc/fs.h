/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_FS_H__)
#define __OFC_FS_H__

/**
 * \defgroup OfcFS File System Abstraction
 * \ingroup OFCPort
 *
 * The File System Redirector is a key component of the Open Files Product
 * and is leveraged heavily by both the SMB Client and Server.  The redirector
 * defines the file system to file system API mapping.  
 *
 * Platform dependent File System handlers are implemented as a plug in to
 * the redirector.  The SMB Client is implemented as one such plug in.
 * The SMB Server utilizes the redirector to dispatch to the supported
 * target file system server.
 *
 * Each file system handler should register an FSINFO structure detailing
 * the supported functions of that handler by calling ofc_fs_register.
 * Currently suported file system handlers are Posix, Win32, 
 * SMB Client, Browser, Pipe, and Mailslots
 *
 * Ports can extend the file system support by leveraging the OFC_FST_OTHER
 * file handler type.
 *
 * A File Handler must call ofc_fs_register in order for it to be available
 * to the redirector.  The call to ofc_fs_register must be done sometime
 * after Open Files configuration.
 * The architecture of how platform specific File System Handlers are 
 * started is platform specific.  Open Files provided file system handlers
 * are started within the ofc_startup call.  Target ports can modify 
 * ofc_startup to add their own startup call to the handler, or it can
 * be done in the platform specific code immediately before or after
 * calls to ofc_startup.
 *
 * After registering a file system handler, it may be desired to 
 * map an arbitrary path to the file system.  See \ref ofc_path in the utils
 * documentation.  With a mapped path, access to a file name will be mapped
 * to an appropriate file system handler and the file access function
 * will be dispatched through the handlers info dispatch table.
 *
 * It may also be desired to add an export to the Open Files Configuration
 * with this mapped path so that access to the file system handler
 * can be exported through the SMB Server.
 */

/** \{ */

#include "ofc/core.h"
#include "ofc/file.h"
#include "ofc/fstype.h"
#include "ofc/types.h"

/**
 * The File System Redirector Dispatch Table
 *
 * Each supported file system handler defines and registers one of these
 * tables that allows the redirector to dispatch into the file system specific
 * handling routines.
 */
typedef struct
{
  /**
   * Create or Open a File
   *
   * Use this call to obtain a file handle for a new or existing file
   *
   * \param lpFileName
   * The name of the object to be crated or opened.  The name is in the 
   * Universal Naming Convention format or an SMB URL.  
   *
   * \param dwDesiredAccess
   * The access to the object, which can be read, write, or both.
   *
   * \param dwShareMode
   * The sharing mode of an object which can be read, write, both or none.
   *
   * \param lpSecurityAttributes
   * This parameter is ignored and should be NULL.
   *
   * \param dwCreationDisposition
   *  An action to take on files that exist or do not exist.
   *
   * \param dwFlagsAndAttributes
   * The file attributes and flag.  The flags used by Win32 are accepted.
   * 
   * \param hTemplateFile
   * This parameter is ignored and should be OFC_HANDLE_NULL
   *
   * \returns
   * If the function fails, the return valid is OFC_INVALID_HANDLE_VALUE
   * If it succeeds, it will return a file handle.
   */
  OFC_HANDLE (*CreateFile)(OFC_LPCTSTR lpFileName,
                           OFC_DWORD dwDesiredAccess,
                           OFC_DWORD dwShareMode,
                           OFC_LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                           OFC_DWORD dwCreationDisposition,
                           OFC_DWORD dwFlagsAndAttributes,
                           OFC_HANDLE hTemplateFile) ;
  /**
   * Deletes an existing file
   *
   * \param lpFileName
   * The name of the file in UNC or SMB URL format
   *
   * \returns
   * If success, returns OFC_TRUE, if failure returns OFC_FALSE
   */
  OFC_BOOL (*DeleteFile)(OFC_LPCTSTR lpFileName) ;
  /**
   * Searches a directory for a file or subdirectory that matches the name
   * or pattern.
   *
   * \param lpFileName
   * The file name or pattern.  The file name can contain '?' characters which
   * will specified a wildcard for that particular character, or '*' which 
   * specifies a wildcard for all characters up to the next text pattern or
   * the next directory level.  For instance File*me would match FileName and
   * FileGnome.  Direct* would match Direct, Directory, DirectDeposit
   *
   * \param lpFindFileData
   * A pointer to the OFC_WIN32_FIND_DATA structure.
   *
   * \returns
   * If the function succeeds, it will return a handle that can be used
   * in subsequent OfcFindNextFile or OfcFindClose call.
   * If the function failed, it will return OFC_INVALID_HANDLE_VALUE.
   */
  OFC_HANDLE (*FindFirstFile)(OFC_LPCTSTR lpFileName,
                              OFC_LPWIN32_FIND_DATAW lpFindFileData,
                              OFC_BOOL *more) ;
  /**
   * Continues a search from a previous call to OfcFindFirstFile
   *
   * \param hFindFile
   * The search handle returned from OfcFindFirstFile
   *
   * \param lpFindFileData
   * A pointer to the OFC_WIN32_FIND_DATA structure.
   *
   * \returns
   * OFC_TRUE if the call succeeded, OFC_FALSE otherwise
   */
  OFC_BOOL (*FindNextFile)(OFC_HANDLE hFindFile,
                           OFC_LPWIN32_FIND_DATAW lpFindFileData,
                           OFC_BOOL *more) ;
  /**
   * Closes a file search handle opened by a call to OfcFindFirstFile
   *
   * \param hFindFile
   * The file search handle
   *
   * \returns
   * OFC_TRUE if the call succeeded, OFC_FALSE otherwise
   */
  OFC_BOOL (*FindClose)(OFC_HANDLE hFindFile) ;
  /**
   * Write all buffered data to the file and clear any buffer cache
   *
   * \param hFile
   * A handle to the open file.
   *
   * \returns
   * OFC_TRUE if the call succeeded, OFC_FALSE otherwise
   */
  OFC_BOOL (*FlushFileBuffers)(OFC_HANDLE hFile) ;
  /**
   * Retrieves Attributes for a specified file or directory
   *
   * \param lpFileName
   * The name of the file or directory
   *
   * \param fInfoLevelId
   * The information class to retrieve.  Must be OfcGetFileExInfoStandard
   *
   * \param lpFileInformation
   * A pointer to the buffer that received the attribute info.  Must be
   * a pointer to a OFC_WIN32_FILE_ATTRIBUTE_DATA structure
   *
   * \returns
   * OFC_TRUE if the function succeeded, OFC_FALSE otherwise
   */
  OFC_BOOL (*GetFileAttributesEx)(OFC_LPCTSTR lpFileName,
								  OFC_GET_FILEEX_INFO_LEVELS fInfoLevelId,
								  OFC_LPVOID lpFileInformation) ;
  /**
   * Retrieves Attribute Infor for a file by File Handle
   *
   * \param hFile
   * Handle to the file
   *
   * \param FileInformationClass
   * Value that specifies the type of info to return
   *
   * \param lpFileInformation
   * Pointer to buffer that receives the requested file informaiton
   * OFC_FILE_BASIC_INFO
   * OFC_FILE_STANDARD_INFO
   * OFC_FILE_NAME_INFO
   * OFC_FILE_STREAM_INFO
   * OFC_FILE_COMPRESSION_INFO
   * OFC_FILE_ATTRIBUTE_TAG_INFO
   * OFC_FILE_ID_BOTH_DIR_INFO
   *
   * \param dwBufferSize
   * Size of the lpFileInformation buffer
   *
   * \returns
   * OFC_TRUE if the call succeeded, OFC_FALSE otherwise
   */
  OFC_BOOL (*GetFileInformationByHandleEx)(OFC_HANDLE hFile,
                                           OFC_FILE_INFO_BY_HANDLE_CLASS
					    FileInformatClass,
                                           OFC_LPVOID lpFileInformation,
                                           OFC_DWORD dwBuferSize) ;
  /**
   * Moves an existing file or directory
   *
   * \param lpExistingFileName
   * The current name of the file or directory
   *
   * \param lpNewFileName
   * The new name for the file or directory
   *
   * \returns
   * OFC_TRUE if success, OFC_FALSE otherwise
   */
  OFC_BOOL (*MoveFile)(OFC_LPCTSTR lpExistingFileName,
					   OFC_LPCTSTR lpNewFileName) ;
  /**
   * Retrieves the results of an overlapped operation on the specified file
   *
   * \param hFile
   * A handle to the file
   *
   * \param lpOverlapped
   * A pointer to the overlapped structure used
   *
   * \param lpNumberOfBytesTransferred
   * Number of bytes read or written during the operation
   *
   * \param bWait
   * Whether this function should block until the operation completes.  If this
   * is false, and the operation is not complete, the call returns false.
   *
   * \returns
   * OFC_TRUE if the function succeeds, OFC_FALSE otherwise
   */
  OFC_BOOL (*GetOverlappedResult)(OFC_HANDLE hFile,
                                  OFC_HANDLE hOverlapped,
                                  OFC_LPDWORD lpNumberOfBytesTransferred,
                                  OFC_BOOL bWait) ;
  /**
   * Create an Overlapped I/O Structure for the desired platform
   *
   * This is used by Read and Write calls to schedule overlapped I/O.
   * The implementation of Overlapped I/O is platform specific so the
   * context for an overlapped I/O is also platform specific
   *
   * \returns
   * Handle to the Overlapped I/O Context
   */
  OFC_HANDLE (*CreateOverlapped)(OFC_VOID) ;
  /**
   * Destroy an Overlapped I/O context
   *
   * \param hOverlapped
   * The overlapped context to destroy
   */
  OFC_VOID (*DestroyOverlapped)(OFC_HANDLE hOverlapped) ;
  /**
   * Set the offset at which an overlapped I/O should occur
   *
   * Since Overlapped I/O is platform specific, this abstraction
   * is provided to communicate to the File handler the file offset
   * to perform the I/O at
   *
   * \param hOverlapped
   * The handle to the overlapped context
   *
   * \param offset
   * The file offset to set
   *
   * \returns Nothing
   */
  OFC_VOID (*SetOverlappedOffset)(OFC_HANDLE hOverlapped,
                                  OFC_OFFT offset) ;
  /**
   * Sets the physical file size for the specified file to the current position
   *
   * \param hFile
   * Handle to the file
   *
   * \returns 
   * OFC_TRUE if success, OFC_FALSE otherwise
   */
  OFC_BOOL (*SetEndOfFile)(OFC_HANDLE hFile) ;
  /**
   * Sets the attributes for a file or directory
   *
   * \param lpFileName 
   * Name of the file
   *
   * \param dwFileAttributes
   * The file attributes to set for the file
   *
   * \returns
   * OFC_TRUE if success, OFC_FALSE otherwise
   */
  OFC_BOOL (*SetFileAttributes)(OFC_LPCTSTR lpFileName,
								OFC_DWORD dwFileAttributes) ;
  /**
   * Set the file information for the specified file
   *
   * \param hFile
   * Handle to the file
   *
   * \param FileInformationClass
   * An enumeration that specifies the type of information to be changed
   *
   * \param lpFileInformation
   * A pointer to the buffer that contains the information to be set
   * OFC_FILE_BASIC_INFO, OFC_FILE_RENAME_INFO, OFC_FILE_DISPOSITION_INFO,
   * OFC_FILE_ALLOCATION_INFO, OFC_FILE_END_OF_FILE_INFO,
   * OFC_FILE_IO_PRIORITY_HINT_INFO
   *
   * \param dwBufferSize
   * The size of the lpFileInformation
   *
   * \returns
   * OFC_TRUE if success, OFC_FALSE otherwise
   */
  OFC_BOOL (*SetFileInformationByHandle)(OFC_HANDLE hFile,
                                         OFC_FILE_INFO_BY_HANDLE_CLASS
					  FileInformationClass,
                                         OFC_LPVOID lpFileInformation,
                                         OFC_DWORD dwBufferSize) ;
  /**
   * Moves the file pointer of the specified file
   *
   * \param hFile
   * File Handle
   * 
   * \param lDistanceToMove
   * The lower order 32 bits of a signed value that specifies the number
   * of bytes to move relative to the move method
   *
   * \param lpDistanceToMoveHigh
   *  pointer to the high order 32 bits.  This may be NULL.  If not NULL
   * it also returns the high order 32 bits of the resulting pointer.
   * (the function's return value returns the low order 32 bits)
   *
   * \param dwMoveMethod
   * The starting point for the file pointer move
   * (FILE_BEGIN, FILE_CURRENT, FILE_END)
   *
   * \returns
   * OFC_INVALID_SET_FILE_POINTER if Failed, low order file pointer
   * if success.  Since OFC_INVALID_SET_FILE_POINTER
   * may also be interpreted as a valid low order file pointer, care should
   * be taken not to get false negatives.
   */
  OFC_DWORD (*SetFilePointer)(OFC_HANDLE hFile,
                              OFC_LONG lDistanceToMove,
                              OFC_PLONG lpDistanceToMoveHigh,
                              OFC_DWORD dwMoveMethod) ;
  /**
   * Writes Data to the specified file
   *
   * \param hFile
   * Handle to the file
   *
   * \param lpBuffer
   * Pointer to buffer containing data to write
   *
   * \param nNumberOfBytesToWrite
   * The number of bytes to write
   *
   * \param lpNumberOfBytesWritten
   * A pointer to a long to contain number of bytes written
   *
   * \param lpOverlapped
   * Pointer to the overlapped structure for asynchronous I/O
   *
   * \returns
   * OFC_TRUE if success, OFC_FALSE if failed
   */
  OFC_BOOL (*WriteFile)(OFC_HANDLE, OFC_LPCVOID, OFC_DWORD,
                        OFC_LPDWORD, OFC_HANDLE) ;
  /**
   * Reads data from a file
   *
   * \param hFile
   * A Handle to the file to be read
   *
   * \param lpBuffer
   * A pointer to the buffer that receives the data read from a file
   *
   * \param nNumberOfBytesToRead
   * The Maximum numer of bytes to be read
   *
   * \param lpNumberOfBytesRead
   * A pointer to a variable to receive the number of bytes read
   *
   * \param lpOverlapped
   * A pointer to the overlapped structure if the file is opened with the flag
   * FILE_FLAG_OVERLAPPED.
   *
   * \returns
   * OFC_FALSE if the function fails, OFC_TRUE otherwise
   */
  OFC_BOOL (*ReadFile)(OFC_HANDLE, OFC_LPVOID, OFC_DWORD,
                       OFC_LPDWORD, OFC_HANDLE) ;
  /**
   * Close A File Handle
   *
   * Use this call to release a file handle obtained from the Create call
   *
   * \param hObject
   * The handle to close
   *
   * \returns
   * OFC_TRUE if the function succeeded, OFC_FALSE if failure
   */
  OFC_BOOL (*CloseHandle)(OFC_HANDLE) ;
  /**
   * Perform a transaction on a named pipe
   *
   * \param hFile
   * File handle of named pipe
   *
   * \param lpInBuffer
   * Pointer to the buffer to send to the pipe
   *
   * \param nBufferSize
   * Size of the input buffer
   * 
   * \param lpOutBuffer
   * Pointer to the buffer to receive the response
   *
   * \param nOutBufferSize
   * Size of the output buffer
   * 
   * \param lpBytesRead
   * Pointer to where to return the number of bytes read
   *
   * \param hOverlapped
   * Handle to the overlapped context if this is asynchronous
   *
   * \returns
   * OFC_TRUE if success, OFC_FALSE if failure.  On failure, GetLastError
   * will return the error code
   */
  OFC_BOOL (*TransactNamedPipe)(OFC_HANDLE hFile,
                                OFC_LPVOID lpInBuffer,
                                OFC_DWORD nInBufferSize,
                                OFC_LPVOID lpOutBuffer,
                                OFC_DWORD nOutBufferSize,
                                OFC_LPDWORD lpBytesRead,
                                OFC_HANDLE hOverlapped) ;
  /**
   * Get the amount of free space for the disk 
   *
   * \param lpRootPathName
   * The path name to the disk or share
   * 
   * \param lpSectorsPerCluster
   * Pointer to where to return the number of sectors per cluster for the
   * volume
   *
   * \param lpBytesPerSector
   * Pointer to where to return the number of bytes per sector for the
   * volume
   *
   * \param lpNumberOfFreeClusters
   * Pointer to where to return the number of free clusters
   *
   * \param lpTotalNumberOfClusters
   * Pointer to where to return the total number of clusters
   *
   * \returns
   * OFC_TRUE if success, OFC_FALSE otherwise
   */
  OFC_BOOL (*GetDiskFreeSpace)(OFC_LPCTSTR lpRootPathName,
							   OFC_LPDWORD lpSectorsPerCluster,
							   OFC_LPDWORD lpBytesPerSector,
							   OFC_LPDWORD lpNumberOfFreeClusters,
							   OFC_LPDWORD lpTotalNumberOfClusters) ;
  /**
   * Get Volume Information for the volume
   *
   * \param lpRootPathName
   * Path to the volume or share
   *
   * \param lpVolumeNameBuffer
   * Pointer to buffer where to store the Volume Name
   *
   * \param nVolumeNameSize
   * Size of the Volume Name Buffer
   *
   * \param lpVolumeSerialNumber
   * Pointer to where to store the volume serial number
   * 
   * \param lpMaximumComponentLength
   * Pointer to where to store the max size of a file name path
   *
   * \param lpFileSystemFlags
   * Pointer to where to store the file system flags
   *
   * \param lpFileSystemName
   * Pointer to where to store the file system name (i.e. NTFS)
   *
   * \param nFileSystemName
   * Size of the file system name buffer
   *
   * \returns
   * OFC_TRUE if successful, OFC_FALSE otherwise
   */
  OFC_BOOL (*GetVolumeInformation)(OFC_LPCTSTR lpRootPathName,
								   OFC_LPTSTR lpVolumeNameBuffer,
								   OFC_DWORD nVolumeNameSize,
								   OFC_LPDWORD lpVolumeSerialNumber,
								   OFC_LPDWORD lpMaximumComponentLength,
								   OFC_LPDWORD lpFileSystemFlags,
								   OFC_LPTSTR lpFileSystemName,
								   OFC_DWORD nFileSystemName) ;
  /**
   * Create a directory
   *
   * \param lpPathName
   * Path to the directory to create
   *
   * \param lpSecurityAttr
   * Security Attributes for the directory
   * 
   * \returns 
   * OFC_TRUE if success, OFC_FALSE otherwise
   */
  OFC_BOOL (*CreateDirectory)(OFC_LPCTSTR lpPathName,
							  OFC_LPSECURITY_ATTRIBUTES lpSecurityAttr) ;
  /**
   * Deletes a directory
   *
   * \param lpPathName
   * The name of the file in UNC or SMB URL format
   *
   * \returns
   * If success, returns OFC_TRUE, if failure returns OFC_FALSE
   */
  OFC_BOOL (*RemoveDirectory)(OFC_LPCTSTR lpPathName) ;
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
   * \param overlapped
   * The overlapped structure which specifies the offset
   *
   * \returns
   * OFC_TRUE if successful, OFC_FALSE otherwise
   */
  OFC_BOOL (*UnlockFileEx)(OFC_HANDLE hFile,
                           OFC_UINT32 length_low, OFC_UINT32 length_high,
                           OFC_HANDLE overlapped) ;
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
   * \param overlapped
   * Pointer to overlapped structure containing offset of region
   *
   * \returns
   * OFC_TRUE if successful, OFC_FALSE otherwise
   */
  OFC_BOOL (*LockFileEx)(OFC_HANDLE hFile, OFC_DWORD flags,
                         OFC_DWORD length_low, OFC_DWORD length_high,
                         OFC_HANDLE overlapped) ;
  OFC_BOOL (*Dismount)(OFC_LPCTSTR lpFileName) ;

  OFC_BOOL (*DeviceIoControl)(OFC_HANDLE hFile,
                              OFC_DWORD dwIoControlCode,
                              OFC_LPVOID lpInBuffer,
                              OFC_DWORD nInBufferSize,
                              OFC_LPVOID lpOutBuffer,
                              OFC_DWORD nOutBufferSize,
                              OFC_LPDWORD lpBytesReturned,
                              OFC_HANDLE hOverlapped) ;
} OFC_FILE_FSINFO ;

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Initialize the File System Redirector
   *
   * This should only be called by ofc_init
   */
  OFC_CORE_LIB OFC_VOID
  ofc_fs_init (OFC_VOID) ;
  OFC_CORE_LIB OFC_VOID
  ofc_fs_destroy (OFC_VOID);
  /**
   * Register a File System Handler
   *
   * \param fsType
   * Type of file system handler to register
   *
   * \param fsInfo
   * File Systems Information and Dispatch Table
   */
  OFC_CORE_LIB OFC_VOID
  ofc_fs_register (OFC_FST_TYPE fsType, OFC_FILE_FSINFO *fsInfo) ;
  /**
   * \latexonly
   */
  OFC_HANDLE OfcFSCreateFile (OFC_FST_TYPE fsType,
                              OFC_LPCTSTR lpFileName,
                              OFC_DWORD dwDesiredAccess,
                              OFC_DWORD dwShareMode,
                              OFC_LPSECURITY_ATTRIBUTES lpSecAttributes,
                              OFC_DWORD dwCreationDisposition,
                              OFC_DWORD dwFlagsAndAttributes,
                              OFC_HANDLE hTemplateFile) ;
  OFC_BOOL OfcFSCreateDirectory (OFC_FST_TYPE fsType,
                                 OFC_LPCTSTR lpPathName,
                                 OFC_LPSECURITY_ATTRIBUTES lpSecurityAttr) ;
  OFC_BOOL OfcFSDeleteFile (OFC_FST_TYPE fsType, OFC_LPCTSTR lpFileName) ;
  OFC_BOOL OfcFSRemoveDirectory (OFC_FST_TYPE fsType,
                                 OFC_LPCTSTR lpPathName) ;
  OFC_HANDLE OfcFSFindFirstFile (OFC_FST_TYPE fsType,
                                 OFC_LPCTSTR lpFileName,
                                 OFC_LPWIN32_FIND_DATAW lpFindFileData,
                                 OFC_BOOL *more) ;
  OFC_BOOL OfcFSFindNextFile (OFC_FST_TYPE fsType,
                              OFC_HANDLE hFindFile,
                              OFC_LPWIN32_FIND_DATAW lpFindFileData,
                              OFC_BOOL *more) ;
  OFC_BOOL OfcFSFindClose (OFC_FST_TYPE fsType, OFC_HANDLE hFindFile) ;
  OFC_BOOL OfcFSFlushFileBuffers (OFC_FST_TYPE fsType, OFC_HANDLE hFile) ;
  OFC_BOOL OfcFSGetFileAttributesEx (OFC_FST_TYPE fsType,
                                     OFC_LPCTSTR lpFileName,
                                     OFC_GET_FILEEX_INFO_LEVELS
				       fInfoLevelId,
                                     OFC_LPVOID lpFileInformation) ;
  OFC_BOOL OfcFSGetFileInformationByHandleEx (OFC_FST_TYPE fsType,
                                              OFC_HANDLE hFile,
                                              OFC_FILE_INFO_BY_HANDLE_CLASS
						FileInformatClass,
                                              OFC_LPVOID lpFileInformation,
                                              OFC_DWORD dwBuferSize) ;
  OFC_BOOL OfcFSMoveFile (OFC_FST_TYPE fsType,
                          OFC_LPCTSTR lpExistingFileName,
                          OFC_LPCTSTR lpNewFileName) ;
  OFC_HANDLE OfcFSCreateOverlapped (OFC_FST_TYPE fsType) ;
  OFC_VOID OfcFSDestroyOverlapped (OFC_FST_TYPE fsType,
                                   OFC_HANDLE hOverlapped) ;
  OFC_VOID OfcFSSetOverlappedOffset (OFC_FST_TYPE fsType,
                                     OFC_HANDLE hOverlapped,
                                     OFC_OFFT offset) ;
  OFC_BOOL OfcFSGetOverlappedResult (OFC_FST_TYPE fsType,
                                     OFC_HANDLE hFile,
                                     OFC_HANDLE hOverlapped,
                                     OFC_LPDWORD lpNumberOfBytesTransferred,
                                     OFC_BOOL bWait) ;
  OFC_BOOL OfcFSSetEndOfFile (OFC_FST_TYPE fsType, OFC_HANDLE hFile) ;
  OFC_BOOL OfcFSSetFileAttributes (OFC_FST_TYPE fsTYpe,
                                   OFC_LPCTSTR lpFileName,
                                   OFC_DWORD dwFileAttributes) ;
  OFC_BOOL OfcFSSetFileInformationByHandle (OFC_FST_TYPE fsTYpe,
                                            OFC_HANDLE hFile,
                                            OFC_FILE_INFO_BY_HANDLE_CLASS
					      FileInformationClass,
                                            OFC_LPVOID lpFileInformation,
                                            OFC_DWORD dwBufferSize) ;
  OFC_DWORD OfcFSSetFilePointer (OFC_FST_TYPE fsTYpe,
                                 OFC_HANDLE hFile,
                                 OFC_LONG lDistanceToMove,
                                 OFC_PLONG lpDistanceToMoveHigh,
                                 OFC_DWORD dwMoveMethod) ;
  OFC_BOOL OfcFSWriteFile (OFC_FST_TYPE fsType,
                           OFC_HANDLE hFile,
                           OFC_LPCVOID lpBuffer,
                           OFC_DWORD nNumberOfBytesToWrite,
                           OFC_LPDWORD lpNumberOfBytesWritten,
                           OFC_HANDLE hOverlapped) ;
  OFC_BOOL OfcFSReadFile (OFC_FST_TYPE fsType,
                          OFC_HANDLE hFile,
                          OFC_LPVOID lpBuffer,
                          OFC_DWORD nNumberOfBytesToRead,
                          OFC_LPDWORD lpNumberOfBytesRead,
                          OFC_HANDLE hOverlapped) ;
  OFC_BOOL OfcFSTransactNamedPipe (OFC_FST_TYPE fsType,
                                   OFC_HANDLE hFile,
                                   OFC_LPVOID lpInBuffer,
                                   OFC_DWORD nInBufferSize,
                                   OFC_LPVOID lpOutBuffer,
                                   OFC_DWORD nOutBufferSize,
                                   OFC_LPDWORD lpBytesRead,
                                   OFC_HANDLE hOverlapped) ;
  OFC_BOOL OfcFSGetDiskFreeSpace (OFC_FST_TYPE fsType,
                                  OFC_LPCTSTR lpRootPathName,
                                  OFC_LPDWORD lpSectorsPerCluster,
                                  OFC_LPDWORD lpBytesPerSector,
                                  OFC_LPDWORD lpNumberOfFreeClusters,
                                  OFC_LPDWORD lpTotalNumberOfClusters) ;
  OFC_BOOL OfcFSGetVolumeInformation (OFC_FST_TYPE fsType,
                                      OFC_LPCTSTR lpRootPathName,
                                      OFC_LPTSTR lpVolumeNameBuffer,
                                      OFC_DWORD nVolumeNameSize,
                                      OFC_LPDWORD lpVolumeSerialNumber,
                                      OFC_LPDWORD lpMaximumComponentLength,
                                      OFC_LPDWORD lpFileSystemFlags,
                                      OFC_LPTSTR lpFileSystemName,
                                      OFC_DWORD nFileSystemName) ;
  OFC_BOOL OfcFSUnlockFileEx (OFC_FST_TYPE fsType,
                              OFC_HANDLE hFile,
                              OFC_UINT32 length_low,
                              OFC_UINT32 length_high,
                              OFC_HANDLE hOverlapped) ;
  OFC_BOOL OfcFSLockFileEx (OFC_FST_TYPE fsType,
                            OFC_HANDLE hFile, OFC_DWORD flags,
                            OFC_DWORD length_low, OFC_DWORD length_high,
                            OFC_HANDLE hOverlapped) ;

  OFC_BOOL OfcFSCloseHandle (OFC_FST_TYPE fsType,
                             OFC_HANDLE hFile) ;
  OFC_BOOL OfcFSDismount (OFC_FST_TYPE fsType,
                          OFC_LPCTSTR lpFileName) ;
  OFC_BOOL OfcFSDeviceIoControl (OFC_FST_TYPE fsType,
                                 OFC_HANDLE hFile,
                                 OFC_DWORD dwIoControlCode,
                                 OFC_LPVOID lpInBuffer,
                                 OFC_DWORD nInBufferSize,
                                 OFC_LPVOID lpOutBuffer,
                                 OFC_DWORD nOutBufferSize,
                                 OFC_LPDWORD lpBytesReturned,
                                 OFC_HANDLE hOverlapped) ;
/**
 * \endlatexonly
 */
#if defined(__cplusplus)
}
#endif
/** \} */
#endif

