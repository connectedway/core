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
#if !defined(__BLUE_FS_H__)
#define __BLUE_FS_H__

/**
 * \defgroup BlueFS File System Abstraction
 * \ingroup BluePort
 *
 * The File System Redirector is a key component of the Blue Share Product
 * and is leveraged heavily by both the CIFS Client and Server.  The redirector
 * defines the file system to file system API mapping.  
 *
 * Platform dependent File System handlers are implemented as a plug in to
 * the redirector.  The CIFS Client is implemented as one such plug in.
 * The CIFS Server utilizes the redirector to dispatch to the supported
 * target file system server.
 *
 * Each file system handler should register an FSINFO structure detailing
 * the supported functions of that handler by calling BlueFSRegister.  
 * Currently suported file system handlers are Posix, Win32, 
 * CIFS Client, Browser, Pipe, and Mailslots
 *
 * Ports can extend the file system support by leveraging the BLUE_FS_OTHER
 * file handler type.
 *
 * A File Handler must call BlueFSRegister in order for it to be available
 * to the redirector.  The call to BlueFSRegister must be done sometime
 * after Blue Share configuration.
 * The architecture of how platform specific File System Handlers are 
 * started is platform specific.  Blue Share provided file system handlers
 * are started within the BlueStartup call.  Target ports can modify 
 * BlueStartup to add their own startup call to the handler, or it can
 * be done in the platform specific code immediately before or after
 * calls to BlueStartup.
 *
 * After registering a file system handler, it may be desired to 
 * map an arbitrary path to the file system.  See \ref BluePath in the BlueUtils
 * documentation.  With a mapped path, access to a file name will be mapped
 * to an appropriate file system handler and the file access function
 * will be dispatched through the handlers info dispatch table.
 *
 * It may also be desired to add an export to the Blue Share Configuration
 * with this mapped path so that access to the file system handler
 * can be exported through the Cifs Server.
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
   * This parameter is ignored and should be BLUE_HANDLE_NULL
   *
   * \returns
   * If the function fails, the return valid is BLUE_INVALID_HANDLE_VALUE
   * If it succeeds, it will return a file handle.
   */
  BLUE_HANDLE (*CreateFile)(BLUE_LPCTSTR lpFileName, 
			    BLUE_DWORD dwDesiredAccess, 
			    BLUE_DWORD dwShareMode,
			    BLUE_LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
			    BLUE_DWORD dwCreationDisposition,
			    BLUE_DWORD dwFlagsAndAttributes, 
			    BLUE_HANDLE hTemplateFile) ;
  /**
   * Deletes an existing file
   *
   * \param lpFileName
   * The name of the file in UNC or SMB URL format
   *
   * \returns
   * If success, returns BLUE_TRUE, if failure returns BLUE_FALSE
   */
  BLUE_BOOL (*DeleteFile)(BLUE_LPCTSTR lpFileName) ;
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
   * A pointer to the BLUE_WIN32_FIND_DATA structure.  
   *
   * \returns
   * If the function succeeds, it will return a handle that can be used
   * in subsequent BlueFindNextFile or BlueFindClose call.
   * If the function failed, it will return BLUE_INVALID_HANDLE_VALUE.
   */
  BLUE_HANDLE (*FindFirstFile)(BLUE_LPCTSTR lpFileName,
			       BLUE_LPWIN32_FIND_DATAW lpFindFileData,
			       BLUE_BOOL *more) ;
  /**
   * Continues a search from a previous call to BlueFindFirstFile
   *
   * \param hFindFile
   * The search handle returned from BlueFindFirstFile
   *
   * \param lpFindFileData
   * A pointer to the BLUE_WIN32_FIND_DATA structure.  
   *
   * \returns
   * BLUE_TRUE if the call succeeded, BLUE_FALSE otherwise
   */
  BLUE_BOOL (*FindNextFile)(BLUE_HANDLE hFindFile,
			    BLUE_LPWIN32_FIND_DATAW lpFindFileData,
			    BLUE_BOOL *more) ;
  /**
   * Closes a file search handle opened by a call to BlueFindFirstFile
   *
   * \param hFindFile
   * The file search handle
   *
   * \returns
   * BLUE_TRUE if the call succeeded, BLUE_FALSE otherwise
   */
  BLUE_BOOL (*FindClose)(BLUE_HANDLE hFindFile) ;
  /**
   * Write all buffered data to the file and clear any buffer cache
   *
   * \param hFile
   * A handle to the open file.
   *
   * \returns
   * BLUE_TRUE if the call succeeded, BLUE_FALSE otherwise
   */
  BLUE_BOOL (*FlushFileBuffers)(BLUE_HANDLE hFile) ;
  /**
   * Retrieves Attributes for a specified file or directory
   *
   * \param lpFileName
   * The name of the file or directory
   *
   * \param fInfoLevelId
   * The information class to retrieve.  Must be BlueGetFileExInfoStandard
   *
   * \param lpFileInformation
   * A pointer to the buffer that received the attribute info.  Must be
   * a pointer to a BLUE_WIN32_FILE_ATTRIBUTE_DATA structure
   *
   * \returns
   * BLUE_TRUE if the function succeeded, BLUE_FALSE otherwise
   */
  BLUE_BOOL (*GetFileAttributesEx)(BLUE_LPCTSTR lpFileName,
				   BLUE_GET_FILEEX_INFO_LEVELS fInfoLevelId,
				   BLUE_LPVOID lpFileInformation) ;
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
   * BLUE_FILE_BASIC_INFO 
   * BLUE_FILE_STANDARD_INFO 
   * BLUE_FILE_NAME_INFO
   * BLUE_FILE_STREAM_INFO
   * BLUE_FILE_COMPRESSION_INFO
   * BLUE_FILE_ATTRIBUTE_TAG_INFO
   * BLUE_FILE_ID_BOTH_DIR_INFO
   *
   * \param dwBufferSize
   * Size of the lpFileInformation buffer
   *
   * \returns
   * BLUE_TRUE if the call succeeded, BLUE_FALSE otherwise
   */
  BLUE_BOOL (*GetFileInformationByHandleEx)(BLUE_HANDLE hFile,
					    BLUE_FILE_INFO_BY_HANDLE_CLASS
					    FileInformatClass,
					    BLUE_LPVOID lpFileInformation,
					    BLUE_DWORD dwBuferSize) ;
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
   * BLUE_TRUE if success, BLUE_FAIL otherwise
   */
  BLUE_BOOL (*MoveFile)(BLUE_LPCTSTR lpExistingFileName,
			 BLUE_LPCTSTR lpNewFileName) ;
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
   * BLUE_TRUE if the function succeeds, BLUE_FALSE otherwise
   */
  BLUE_BOOL (*GetOverlappedResult)(BLUE_HANDLE hFile,
				   BLUE_HANDLE hOverlapped,
				   BLUE_LPDWORD lpNumberOfBytesTransferred,
				   BLUE_BOOL bWait) ;
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
  BLUE_HANDLE (*CreateOverlapped)(BLUE_VOID) ;
  /**
   * Destroy an Overlapped I/O context
   *
   * \param hOverlapped
   * The overlapped context to destroy
   */
  BLUE_VOID (*DestroyOverlapped)(BLUE_HANDLE hOverlapped) ;
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
  BLUE_VOID (*SetOverlappedOffset)(BLUE_HANDLE hOverlapped,
				   BLUE_OFFT offset) ;
  /**
   * Sets the physical file size for the specified file to the current position
   *
   * \param hFile
   * Handle to the file
   *
   * \returns 
   * BLUE_TRUE if success, BLUE_FALSE otherwise
   */
  BLUE_BOOL (*SetEndOfFile)(BLUE_HANDLE hFile) ;
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
   * BLUE_TRUE if success, BLUE_FALSE otherwise
   */
  BLUE_BOOL (*SetFileAttributes)(BLUE_LPCTSTR lpFileName,
				 BLUE_DWORD dwFileAttributes) ;
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
   * BLUE_FILE_BASIC_INFO, BLUE_FILE_RENAME_INFO, BLUE_FILE_DISPOSITION_INFO,
   * BLUE_FILE_ALLOCATION_INFO, BLUE_FILE_END_OF_FILE_INFO,
   * BLUE_FILE_IO_PRIORITY_HINT_INFO
   *
   * \param dwBufferSize
   * The size of the lpFileInformation
   *
   * \returns
   * BLUE_TRUE if success, BLUE_FALSE otherwise
   */
  BLUE_BOOL (*SetFileInformationByHandle)(BLUE_HANDLE hFile,
					  BLUE_FILE_INFO_BY_HANDLE_CLASS
					  FileInformationClass,
					  BLUE_LPVOID lpFileInformation,
					  BLUE_DWORD dwBufferSize) ;
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
   * BLUE_INVALID_SET_FILE_POINTER if Failed, low order file pointer 
   * if success.  Since BLUE_INVALID_SET_FILE_POINTER
   * may also be interpreted as a valid low order file pointer, care should
   * be taken not to get false negatives.
   */
  BLUE_DWORD (*SetFilePointer)(BLUE_HANDLE hFile,
			       BLUE_LONG lDistanceToMove,
			       BLUE_PLONG lpDistanceToMoveHigh,
			       BLUE_DWORD dwMoveMethod) ;
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
   * BLUE_TRUE if success, BLUE_FALSE if failed
   */
  BLUE_BOOL (*WriteFile)(BLUE_HANDLE, BLUE_LPCVOID, BLUE_DWORD,
			 BLUE_LPDWORD, BLUE_HANDLE) ;
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
   * BLUE_FALSE if the function fails, BLUE_TRUE otherwise
   */
  BLUE_BOOL (*ReadFile)(BLUE_HANDLE, BLUE_LPVOID, BLUE_DWORD,
			BLUE_LPDWORD, BLUE_HANDLE) ;
  /**
   * Close A File Handle
   *
   * Use this call to release a file handle obtained from the Create call
   *
   * \param hObject
   * The handle to close
   *
   * \returns
   * BLUE_TRUE if the function succeeded, BLUE_FALSE if failure
   */
  BLUE_BOOL (*CloseHandle)(BLUE_HANDLE) ;
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
   * BLUE_TRUE if success, BLUE_FALSE if failure.  On failure, GetLastError
   * will return the error code
   */
  BLUE_BOOL (*TransactNamedPipe)(BLUE_HANDLE hFile, 
				 BLUE_LPVOID lpInBuffer, 
				 BLUE_DWORD nInBufferSize, 
				 BLUE_LPVOID lpOutBuffer, 
				 BLUE_DWORD nOutBufferSize,
				 BLUE_LPDWORD lpBytesRead, 
				 BLUE_HANDLE hOverlapped) ;
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
   * BLUE_TRUE if success, BLUE_FALSE otherwise
   */
  BLUE_BOOL (*GetDiskFreeSpace)(BLUE_LPCTSTR lpRootPathName,
				BLUE_LPDWORD lpSectorsPerCluster,
				BLUE_LPDWORD lpBytesPerSector,
				BLUE_LPDWORD lpNumberOfFreeClusters,
				BLUE_LPDWORD lpTotalNumberOfClusters) ;
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
   * BLUE_TRUE if successful, BLUE_FALSE otherwise
   */
  BLUE_BOOL (*GetVolumeInformation)(BLUE_LPCTSTR lpRootPathName,
				    BLUE_LPTSTR lpVolumeNameBuffer,
				    BLUE_DWORD nVolumeNameSize,
				    BLUE_LPDWORD lpVolumeSerialNumber,
				    BLUE_LPDWORD lpMaximumComponentLength,
				    BLUE_LPDWORD lpFileSystemFlags,
				    BLUE_LPTSTR lpFileSystemName,
				    BLUE_DWORD nFileSystemName) ;
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
   * BLUE_TRUE if success, BLUE_FALSE otherwise
   */
  BLUE_BOOL (*CreateDirectory)(BLUE_LPCTSTR lpPathName,
			       BLUE_LPSECURITY_ATTRIBUTES lpSecurityAttr) ;
  /**
   * Deletes a directory
   *
   * \param lpPathName
   * The name of the file in UNC or SMB URL format
   *
   * \returns
   * If success, returns BLUE_TRUE, if failure returns BLUE_FALSE
   */
  BLUE_BOOL (*RemoveDirectory)(BLUE_LPCTSTR lpPathName) ;
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
   * BLUE_TRUE if successful, BLUE_FALSE otherwise
   */
  BLUE_BOOL (*UnlockFileEx)(BLUE_HANDLE hFile, 
			    BLUE_UINT32 length_low, BLUE_UINT32 length_high,
			    BLUE_HANDLE overlapped) ;
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
   * BLUE_TRUE if successful, BLUE_FALSE otherwise
   */
  BLUE_BOOL (*LockFileEx)(BLUE_HANDLE hFile, BLUE_DWORD flags,
			  BLUE_DWORD length_low, BLUE_DWORD length_high,
			  BLUE_HANDLE overlapped) ;
  BLUE_BOOL (*Dismount)(BLUE_LPCTSTR lpFileName) ;

  BLUE_BOOL (*DeviceIoControl)(BLUE_HANDLE hFile, 
			       BLUE_DWORD dwIoControlCode,
			       BLUE_LPVOID lpInBuffer, 
			       BLUE_DWORD nInBufferSize, 
			       BLUE_LPVOID lpOutBuffer, 
			       BLUE_DWORD nOutBufferSize,
			       BLUE_LPDWORD lpBytesReturned,
			       BLUE_HANDLE hOverlapped) ;
} BLUE_FILE_FSINFO ;

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Initialize the File System Redirector
   *
   * This should only be called by BlueInit
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueFSInit (BLUE_VOID) ;
  BLUE_CORE_LIB BLUE_VOID 
  BlueFSDestroy (BLUE_VOID);
  /**
   * Register a File System Handler
   *
   * \param fsType
   * Type of file system handler to register
   *
   * \param fsInfo
   * File Systems Information and Dispatch Table
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueFSRegister (BLUE_FS_TYPE fsType, BLUE_FILE_FSINFO *fsInfo) ;
  /**
   * \latexonly
   */
  BLUE_HANDLE BlueFSCreateFile (BLUE_FS_TYPE fsType,
				BLUE_LPCTSTR lpFileName,
				BLUE_DWORD dwDesiredAccess,
				BLUE_DWORD dwShareMode,
				BLUE_LPSECURITY_ATTRIBUTES lpSecAttributes,
				BLUE_DWORD dwCreationDisposition,
				BLUE_DWORD dwFlagsAndAttributes,
				BLUE_HANDLE hTemplateFile) ;
  BLUE_BOOL BlueFSCreateDirectory (BLUE_FS_TYPE fsType,
				   BLUE_LPCTSTR lpPathName,
				   BLUE_LPSECURITY_ATTRIBUTES lpSecurityAttr) ;
  BLUE_BOOL BlueFSDeleteFile (BLUE_FS_TYPE fsType, BLUE_LPCTSTR lpFileName) ;
  BLUE_BOOL BlueFSRemoveDirectory (BLUE_FS_TYPE fsType, 
				   BLUE_LPCTSTR lpPathName) ;
  BLUE_HANDLE BlueFSFindFirstFile (BLUE_FS_TYPE fsType,
				   BLUE_LPCTSTR lpFileName,
				   BLUE_LPWIN32_FIND_DATAW lpFindFileData,
				   BLUE_BOOL *more) ;
  BLUE_BOOL BlueFSFindNextFile (BLUE_FS_TYPE fsType,
				BLUE_HANDLE hFindFile,
				BLUE_LPWIN32_FIND_DATAW lpFindFileData,
				BLUE_BOOL *more) ;
  BLUE_BOOL BlueFSFindClose (BLUE_FS_TYPE fsType, BLUE_HANDLE hFindFile) ;
  BLUE_BOOL BlueFSFlushFileBuffers (BLUE_FS_TYPE fsType, BLUE_HANDLE hFile) ;
  BLUE_BOOL BlueFSGetFileAttributesEx (BLUE_FS_TYPE fsType, 
				       BLUE_LPCTSTR lpFileName,
				       BLUE_GET_FILEEX_INFO_LEVELS 
				       fInfoLevelId,
				       BLUE_LPVOID lpFileInformation) ;
  BLUE_BOOL BlueFSGetFileInformationByHandleEx (BLUE_FS_TYPE fsType,
						BLUE_HANDLE hFile,
						BLUE_FILE_INFO_BY_HANDLE_CLASS
						FileInformatClass,
						BLUE_LPVOID lpFileInformation,
						BLUE_DWORD dwBuferSize) ;
  BLUE_BOOL BlueFSMoveFile (BLUE_FS_TYPE fsType,
			    BLUE_LPCTSTR lpExistingFileName,
			    BLUE_LPCTSTR lpNewFileName) ;
  BLUE_HANDLE BlueFSCreateOverlapped (BLUE_FS_TYPE fsType) ;
  BLUE_VOID BlueFSDestroyOverlapped (BLUE_FS_TYPE fsType, 
				     BLUE_HANDLE hOverlapped) ;
  BLUE_VOID BlueFSSetOverlappedOffset (BLUE_FS_TYPE fsType, 
				       BLUE_HANDLE hOverlapped,
				       BLUE_OFFT offset) ;
  BLUE_BOOL BlueFSGetOverlappedResult (BLUE_FS_TYPE fsType,
				       BLUE_HANDLE hFile,
				       BLUE_HANDLE hOverlapped,
				       BLUE_LPDWORD lpNumberOfBytesTransferred,
				       BLUE_BOOL bWait) ;
  BLUE_BOOL BlueFSSetEndOfFile (BLUE_FS_TYPE fsType, BLUE_HANDLE hFile) ;
  BLUE_BOOL BlueFSSetFileAttributes (BLUE_FS_TYPE fsTYpe, 
				     BLUE_LPCTSTR lpFileName,
				     BLUE_DWORD dwFileAttributes) ;
  BLUE_BOOL BlueFSSetFileInformationByHandle (BLUE_FS_TYPE fsTYpe, 
					      BLUE_HANDLE hFile,
					      BLUE_FILE_INFO_BY_HANDLE_CLASS
					      FileInformationClass,
					      BLUE_LPVOID lpFileInformation,
					      BLUE_DWORD dwBufferSize) ;
  BLUE_DWORD BlueFSSetFilePointer (BLUE_FS_TYPE fsTYpe,
				   BLUE_HANDLE hFile,
				   BLUE_LONG lDistanceToMove,
				   BLUE_PLONG lpDistanceToMoveHigh,
				   BLUE_DWORD dwMoveMethod) ;
  BLUE_BOOL BlueFSWriteFile (BLUE_FS_TYPE fsType,
			     BLUE_HANDLE hFile,
			     BLUE_LPCVOID lpBuffer,
			     BLUE_DWORD nNumberOfBytesToWrite,
			     BLUE_LPDWORD lpNumberOfBytesWritten,
			     BLUE_HANDLE hOverlapped) ;
  BLUE_BOOL BlueFSReadFile (BLUE_FS_TYPE fsType,
			    BLUE_HANDLE hFile,
			    BLUE_LPVOID lpBuffer,
			    BLUE_DWORD nNumberOfBytesToRead,
			    BLUE_LPDWORD lpNumberOfBytesRead,
			    BLUE_HANDLE hOverlapped) ;
  BLUE_BOOL BlueFSTransactNamedPipe (BLUE_FS_TYPE fsType,
				     BLUE_HANDLE hFile,
				     BLUE_LPVOID lpInBuffer,
				     BLUE_DWORD nInBufferSize,
				     BLUE_LPVOID lpOutBuffer,
				     BLUE_DWORD nOutBufferSize,
				     BLUE_LPDWORD lpBytesRead,
				     BLUE_HANDLE hOverlapped) ;
  BLUE_BOOL BlueFSGetDiskFreeSpace (BLUE_FS_TYPE fsType,
				    BLUE_LPCTSTR lpRootPathName,
				    BLUE_LPDWORD lpSectorsPerCluster,
				    BLUE_LPDWORD lpBytesPerSector,
				    BLUE_LPDWORD lpNumberOfFreeClusters,
				    BLUE_LPDWORD lpTotalNumberOfClusters) ;
  BLUE_BOOL BlueFSGetVolumeInformation (BLUE_FS_TYPE fsType,
					BLUE_LPCTSTR lpRootPathName,
					BLUE_LPTSTR lpVolumeNameBuffer,
					BLUE_DWORD nVolumeNameSize,
					BLUE_LPDWORD lpVolumeSerialNumber,
					BLUE_LPDWORD lpMaximumComponentLength,
					BLUE_LPDWORD lpFileSystemFlags,
					BLUE_LPTSTR lpFileSystemName,
					BLUE_DWORD nFileSystemName) ;
  BLUE_BOOL BlueFSUnlockFileEx (BLUE_FS_TYPE fsType,
				BLUE_HANDLE hFile, 
				BLUE_UINT32 length_low, 
				BLUE_UINT32 length_high,
				BLUE_HANDLE hOverlapped) ;
  BLUE_BOOL BlueFSLockFileEx (BLUE_FS_TYPE fsType,
			      BLUE_HANDLE hFile, BLUE_DWORD flags,
			      BLUE_DWORD length_low, BLUE_DWORD length_high,
			      BLUE_HANDLE hOverlapped) ;

  BLUE_BOOL BlueFSCloseHandle (BLUE_FS_TYPE fsType,
			       BLUE_HANDLE hFile) ;
  BLUE_BOOL BlueFSDismount (BLUE_FS_TYPE fsType,
			    BLUE_LPCTSTR lpFileName) ;
  BLUE_BOOL BlueFSDeviceIoControl (BLUE_FS_TYPE fsType,
				   BLUE_HANDLE hFile, 
				   BLUE_DWORD dwIoControlCode,
				   BLUE_LPVOID lpInBuffer, 
				   BLUE_DWORD nInBufferSize, 
				   BLUE_LPVOID lpOutBuffer, 
				   BLUE_DWORD nOutBufferSize,
				   BLUE_LPDWORD lpBytesReturned,
				   BLUE_HANDLE hOverlapped) ;
/**
 * \endlatexonly
 */
#if defined(__cplusplus)
}
#endif
/** \} */
#endif

