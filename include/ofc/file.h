/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__BLUE_FILE_H__)
#define __BLUE_FILE_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/config.h"
#include "ofc/handle.h"
#include "ofc/waitq.h"
#include "ofc/fstype.h"

/**
 * \defgroup BlueFile Blue File Redirector
 * \ingroup BlueUtil
 *
 * The Blue Share File Facility provides Generic File Handling APIs to 
 * applications.
 *
 * Blue File also provides a redirector to platform specific File System APIs. 
 * Lastly, the Blue File Module provides abstractions for device
 * mapping and URL specifications.  
 *
 * Consider the Blue File Module to be a many to many mux.  Various APIs 
 * are provided for applications that redirect control to multiple file system 
 * backends.  In order to function as a many to many mux, there must be a 
 * common denominator to which APIs are translated to and from which 
 * backends are dispatched.  This common denominator is a Win32 like file 
 * system layer.  Win32 has been chosen, not only because Win32 has a more 
 * robust API set then other standard File APIs, but because the main 
 * product that it will be used for is the Blue Share CIFS product which 
 * leverages the Win32 APIs.
 * 
 * \{ 
 */

/**
 * The maximum size of a path
 */
#define BLUE_MAX_PATH 260

/**
 * The 64 bit representation of a file time
 */
typedef struct BLUE_FILETIME
{
  BLUE_DWORD dwLowDateTime ;	/**< The Low 32 bits  */
  BLUE_DWORD dwHighDateTime ;	/**< The High 32 bits  */
} BLUE_FILETIME ;

/**
 * Valid Access Rights for Files and Directories
 */
enum 
  {
    /**
     * The right to create a file in the directory
     */
    BLUE_FILE_ADD_FILE = 0x0002,
    /**
     * The right to create a subdirectory
     */
    BLUE_FILE_ADD_SUBDIRECTORY = 0x0004,
    /**
     * All possible access rights for a file
     */
    BLUE_FILE_ALL_ACCESS = 0xFFFF,
    /**
     * The right to append data to a file
     */
    BLUE_FILE_APPEND_DATA = 0x0004,
    /**
     * The right to delete a directory
     */
    BLUE_FILE_DELETE_CHILD = 0x0040,
    /**
     * The right to execute the file
     */
    BLUE_FILE_EXECUTE = 0x0020,
    /**
     * The right to list contents of a directory
     */
    BLUE_FILE_LIST_DIRECTORY = 0x0001,
    /**
     * The right to read file attributes
     */
    BLUE_FILE_READ_ATTRIBUTES = 0x0080,
    /**
     * The right to read file data
     */
    BLUE_FILE_READ_DATA = 0x0001,
    /**
     * The right to read extended file attributes
     */
    BLUE_FILE_READ_EA = 0x0008,
    /**
     * The right to traverse a directory
     */
    BLUE_FILE_TRAVERSE = 0x0020,
    /**
     * The right to write file attributes
     */
    BLUE_FILE_WRITE_ATTRIBUTES = 0x0100,
    /**
     * The right to write data to a file
     */
    BLUE_FILE_WRITE_DATA = 0x0002,
    /**
     * The right to write extended file attributes
     */
    BLUE_FILE_WRITE_EA = 0x0010,
    /**
     * Delete Access
     */
    BLUE_FILE_DELETE = 0x00010000L,
    /**
     * Generic Execute Access
     */
    BLUE_GENERIC_EXECUTE = 
    (BLUE_FILE_READ_ATTRIBUTES | 
     BLUE_FILE_EXECUTE),
    /**
     * Generic Read Access
     */
    BLUE_GENERIC_READ =
    (BLUE_FILE_READ_ATTRIBUTES |
     BLUE_FILE_READ_DATA |
     BLUE_FILE_READ_EA),
    /**
     * Generic Write Access
     */
    BLUE_GENERIC_WRITE = 
    (BLUE_FILE_APPEND_DATA |
     BLUE_FILE_WRITE_ATTRIBUTES |
     BLUE_FILE_WRITE_DATA |
     BLUE_FILE_WRITE_EA),
  } ;

/**
 * The Sharing Mode of an object
 */
enum 
  {
    /**
     * Open file for exclusive access
     */
    BLUE_FILE_SHARE_NONE = 0,
    /**
     * Allow subsequent opens to request delete access and allow this access if
     * file previously opened with delete access
     */
    BLUE_FILE_SHARE_DELETE = 0x04,
    /**
     * Allow subsequent opens to request write access and allow this access if
     * file previously opened with write access
     */
    BLUE_FILE_SHARE_WRITE = 0x02,
    /**
     * Allow subsequent opens to request read access and allow this access if 
     * file previously opened with read access
     */
    BLUE_FILE_SHARE_READ = 0x01
  } ;

/**
 * A action to take on files whether they exist or not.
 */
enum
  {
    /**
     * Create a New File.  Fail if file already exists
     */
    BLUE_CREATE_NEW = 1,
    /**
     * Create a file always.  If it exists overwrite.
     */
    BLUE_CREATE_ALWAYS = 2,
    /**
     * Open an Existing File.  If the file doesn't exist, fail
     */
    BLUE_OPEN_EXISTING = 3,
    /**
     * Open a file always.  If the file doesn't exist, create
     */
    BLUE_OPEN_ALWAYS = 4,
    /**
     * Open a file and truncate.  If the file doesn't exist, fail.
     */
    BLUE_TRUNCATE_EXISTING = 5
  } ;


/**
 * File Attributes and Flags
 */
enum
  {
    /**
     * The file is read only
     */
    BLUE_FILE_ATTRIBUTE_READONLY = 0x0001,
    /**
     * The file is hidden
     */
    BLUE_FILE_ATTRIBUTE_HIDDEN = 0x0002,
    /**
     * The file is a system file
     */
    BLUE_FILE_ATTRIBUTE_SYSTEM = 0x0004,
    BLUE_FILE_ATTRIBUTE_BOOKMARK = 0x0008,
    /**
     * The file is a directory
     */
    BLUE_FILE_ATTRIBUTE_DIRECTORY = 0x0010,
    /**
     * The file should be archived
     */
    BLUE_FILE_ATTRIBUTE_ARCHIVE = 0x0020,
    /**
     * The file is a normal file
     */
    BLUE_FILE_ATTRIBUTE_NORMAL = 0x0080,
    /**
     * The file is a temporary file
     */
    BLUE_FILE_ATTRIBUTE_TEMPORARY = 0x0100,
    /**
     * The file is a sparse file
     */
    BLUE_FILE_ATTRIBUTE_SPARSE_FILE = 0x0200,

    BLUE_FILE_ATTRIBUTE_REPARSE_POINT = 0x0400,

    /**
     * The file is compressed
     */
    BLUE_FILE_ATTRIBUTE_COMPRESSED = 0x0800,
    /**
     * The file data is not immediate available
     */
    BLUE_FILE_ATTRIBUTE_OFFLINE = 0x1000,

    BLUE_FILE_ATTRIBUTE_NOT_CONTENT_INDEXED = 0x00002000,

    /**
     * The file is encrypted
     */
    BLUE_FILE_ATTRIBUTE_ENCRYPTED = 0x4000,

    BLUE_FILE_ATTRIBUTE_INTEGRITY_STREAM = 0x8000,

    BLUE_FILE_ATTRIBUTE_NO_SCRUB_DATA = 0x00020000,

    /**
     * The file is virtual
     */
    BLUE_FILE_ATTRIBUTE_VIRTUAL = 0x00010000,
    /**
     * If the file is remote, don't move back to local storage
     */
    BLUE_FILE_FLAG_OPEN_NO_RECALL = 0x00100000,
    /*
     * These are Blue Peach Specific to denote remote files
     */
    /**
     * File is a workgroup directory
     */
    BLUE_FILE_FLAG_WORKGROUP = 0x00200000,
    /**
     * File is a server directory
     */
    BLUE_FILE_FLAG_SERVER = 0x00400000,
    /*
     * File is a Share directory
     */
    BLUE_FILE_FLAG_SHARE = 0x00800000,
    /**
     * The file is being opened with Posix Semantics (i.e. case sensitive)
     */
    BLUE_FILE_FLAG_POSIX_SEMANTICS = 0x01000000,
    /**
     * The file is opened for a backup or restore operation
     */
    BLUE_FILE_FLAG_BACKUP_SEMANTICS = 0x02000000,
    /**
     * The file is to be deleted when closed
     */
    BLUE_FILE_FLAG_DELETE_ON_CLOSE = 0x04000000,
    /**
     * The file will be accessed sequentially from beggining to end
     */
    BLUE_FILE_FLAG_SEQUENTIAL_SCAN = 0x08000000,
    /**
     * The file is to be accessed randomly
     */
    BLUE_FILE_FLAG_RANDOM_ACCESS = 0x10000000,
    /**
     * The file data should not be cached
     */
    BLUE_FILE_FLAG_NO_BUFFERING = 0x20000000,
    /**
     * The file is being opened for asynchronous I/O
     */
    BLUE_FILE_FLAG_OVERLAPPED = 0x40000000,
    /**
     * Write Caching should not occur
     */
    BLUE_FILE_FLAG_WRITE_THROUGH = 0x80000000
  } ;

/*
 * Create Options
 */
enum 
  {
    /**
     * Create a directory
     */
    BLUE_FILE_CREATE_DIRECTORY = 0x0001,
    /**
     * Do not cache writes
     */
    BLUE_FILE_CREATE_WRITETHROUGH = 0x0002,
    /**
     * I/O is performed sequentially
     */
    BLUE_FILE_CREATE_SEQUENTIAL = 0x0004,
    /**
     * Enable Alerts on the file
     */
    BLUE_FILE_CREATE_SIOALERT = 0x0010,
    /**
     * Do not enable alerts on the file
     */
    BLUE_FILE_CREATE_SIONONALERT = 0x0020,
    /**
     * This is not a directory
     */
    BLUE_FILE_CREATE_NONDIRECTORY = 0x0040,
    /**
     * Do not create extended attributes
     */
    BLUE_FILE_CREATE_NOEA = 0x0200,
    /**
     * Enforce 8x3 file naming
     */
    BLUE_FILE_CREATE_8x3 = 0x0400,
    /**
     * Create a random file
     */
    BLUE_FILE_CREATE_RANDOM = 0x0800,
    /**
     * Delete this file when closed
     */
    BLUE_FILE_CREATE_DELONCLOSE = 0x1000
  } ;

/*
 * Open Actions
 */
enum 
  {
    /**
     * The file was opened
     */
    BLUE_FILE_OPEN_ACTION_OPENED,
    /**
     * The file was created
     */
    BLUE_FILE_OPEN_ACTION_CREATED,
    /**
     * The file was truncated
     */
    BLUE_FILE_OPEN_ACTION_TRUNCATED
  } ;

/**
 * Identifies the type of file information for the 
 * GetFileInformationByHandleEx or SetFileInformationByHandleEx call
 */
typedef enum _BLUE_FILE_INFO_BY_HANDLE_CLASS
  {
    /**
     * Minimal Information
     */
    BlueFileBasicInfo = 0,
    /**
     * Extended Information
     */
    BlueFileStandardInfo,
    /**
     * All Information
     */
    BlueFileAllInfo,
    /**
     * The File Name
     */
    BlueFileNameInfo,
    /**
     * The File Name should be changed
     */
    BlueFileRenameInfo,
    /**
     * The File Should be deleted
     */
    BlueFileDispositionInfo,
    /**
     * File Allocation Info should be changed
     */
    BlueFileAllocationInfo,
    /**
     * The end of the file should be set
     */
    BlueFileEndOfFileInfo,
    /**
     * File Stream Information should be retrieved
     */
    BlueFileStreamInfo,
    /**
     * File Compression Info should be retrieved
     */
    BlueFileCompressionInfo,
    /**
     * File Attribute Information Should be Retrieved
     */
    BlueFileAttributeTagInfo,
    /**
     * Files in the directory should be retrieved
     */
    BlueFileIdBothDirectoryInfo,
    /**
     * Identical fo FileIdBothDirectoryInfo but the enumeration should begin
     * from the beginning
     */
    BlueFileIdBothDirectoryRestartInfo,
    /*
     * We'll include this one too.  It's the one you can get without a file
     * handle 
     */
    BlueFileInfoStandard,
    /*
     * All
     */
    BlueFileInfoAll,
    /*
     */
    BlueFileNetworkOpenInfo,
    /*
     * Query Directory Values
     */
    BlueFileDirectoryInformation,
    BlueFileFullDirectoryInformation,
    BlueFileIdFullDirectoryInformation,
    BlueFileBothDirectoryInformation,
    BlueFileIdBothDirectoryInformation,
    BlueFileNamesInformation,
    BlueFileInternalInformation,
    /**
     * Maximum Value
     */
    BlueMaximumFileInfoByHandlesClass
  } BLUE_FILE_INFO_BY_HANDLE_CLASS ;

typedef enum _BLUE_FILEFS_INFO_BY_HANDLE_CLASS
  {
    BlueFileFSAttributeInformation,
    BlueFileFSSizeInformation,
    BlueFileFSVolumeInformation,
    BlueFileFSFullSizeInformation,
    BlueFileFSDeviceInformation
  } BLUE_FILEFS_INFO_BY_HANDLE_CLASS ;

typedef enum _BLUE_FILE_INFO_BY_HANDLE_TYPE
  {
    BlueFileInfoTypeFile,
    BlueFileInfoTypeFS,
    BlueFileInfoTypeSecurity,
    BlueFileInfoTypeQuota
  } BLUE_FILE_INFO_BY_HANDLE_TYPE ;

/**
 * Security Attributes for files
 *
 * This is not used but is here for completeness
 */
typedef struct _BLUE_SECURITY_ATTRIBUTES 
{
  /**
   * The length of the security descriptor
   */
  BLUE_DWORD nLength;
  /**
   * A pointer to the security descriptor
   */
  BLUE_LPVOID lpSecurityDescriptor;
  /**
   * Whether this security descriptor can be inherited
   */
  BLUE_BOOL bInheritHandle;
} BLUE_SECURITY_ATTRIBUTES ;

/**
 * A pointer to a security descriptor
 */
typedef BLUE_VOID *BLUE_PSECURITY_DESCRIPTOR ;
/**
 * A pointer to the security attributes
 */
typedef BLUE_SECURITY_ATTRIBUTES *BLUE_LPSECURITY_ATTRIBUTES ;

/**
 * Contains information about the file that is found by BlueFindFirstFile, 
 * BlueFindFirstFileEx or BlueFindNextFile
 */
typedef struct _BLUE_WIN32_FIND_DATAW
{
  /**
   * The attributes of the file
   */
  BLUE_DWORD dwFileAttributes ;
  /**
   * The create time of the file
   */
  BLUE_FILETIME ftCreateTime ;
  /**
   * The time the file was last accessed
   */
  BLUE_FILETIME ftLastAccessTime ;
  /**
   * The time the file was last written
   */
  BLUE_FILETIME ftLastWriteTime ;
  /**
   * The upper Dword value of the file size
   */
  BLUE_DWORD nFileSizeHigh ;
  /**
   * The low order value of the file size
   */
  BLUE_DWORD nFileSizeLow ;
  /**
   * Unused
   */
  BLUE_DWORD dwReserved0 ;
  /**
   * Unused
   */
  BLUE_DWORD dwReserved1 ;
  /**
   * The name of the file
   */
  BLUE_TCHAR cFileName[BLUE_MAX_PATH] ;
  /**
   * The 8.3 name of the file
   */
  BLUE_TCHAR cAlternateFileName[14] ;
} BLUE_WIN32_FIND_DATAW ;
/**
 * A pointer to the Win32 Find data
 */
typedef BLUE_WIN32_FIND_DATAW *BLUE_LPWIN32_FIND_DATAW ;

typedef struct _BLUE_WIN32_FIND_DATAA
{
  /**
   * The attributes of the file
   */
  BLUE_DWORD dwFileAttributes ;
  /**
   * The create time of the file
   */
  BLUE_FILETIME ftCreateTime ;
  /**
   * The time the file was last accessed
   */
  BLUE_FILETIME ftLastAccessTime ;
  /**
   * The time the file was last written
   */
  BLUE_FILETIME ftLastWriteTime ;
  /**
   * The upper Dword value of the file size
   */
  BLUE_DWORD nFileSizeHigh ;
  /**
   * The low order value of the file size
   */
  BLUE_DWORD nFileSizeLow ;
  /**
   * Unused
   */
  BLUE_DWORD dwReserved0 ;
  /**
   * Unused
   */
  BLUE_DWORD dwReserved1 ;
  /**
   * The name of the file
   */
  BLUE_CHAR cFileName[BLUE_MAX_PATH] ;
  /**
   * The 8.3 name of the file
   */
  BLUE_CHAR cAlternateFileName[14] ;
} BLUE_WIN32_FIND_DATAA ;
/**
 * A pointer to the Win32 Find data
 */
typedef BLUE_WIN32_FIND_DATAA *BLUE_LPWIN32_FIND_DATAA ;

/**
 * Contains information about the file that is found by BlueFindFirstFile, 
 * BlueFindFirstFileEx or BlueFindNextFile
 */
typedef struct _BLUE_WIN32_FILE_ATTRIBUTE_DATA
{
  /**
   * The attributes of the file
   */
  BLUE_DWORD dwFileAttributes ;
  /**
   * The create time of the file
   */
  BLUE_FILETIME ftCreateTime ;
  /**
   * The time the file was last accessed
   */
  BLUE_FILETIME ftLastAccessTime ;
  /**
   * The time the file was last written
   */
  BLUE_FILETIME ftLastWriteTime ;
  /**
   * The upper Dword value of the file size
   */
  BLUE_DWORD nFileSizeHigh ;
  /**
   * The low order value of the file size
   */
  BLUE_DWORD nFileSizeLow ;
} BLUE_WIN32_FILE_ATTRIBUTE_DATA ;

/**
 * A pointer to the Win32 File Attribute Data
 */
typedef BLUE_WIN32_FILE_ATTRIBUTE_DATA *BLUE_LPWIN32_FILE_ATTRIBUTE_DATA ;

/**
 * Values for the BlueGetFileAttributesEx Call
 */
typedef enum _BLUE_GET_FILEEX_INFO_LEVELS 
  {
    /**
     * Standard Information
     */
    BlueGetFileExInfoStandard,
    /**
     * Max Value
     */
    BlueGetFileExMaxInfoLevel
} BLUE_GET_FILEEX_INFO_LEVELS;

/** \latexonly */
/**
 * Contains information used in asynchrous I/O
 */
typedef struct _BLUE_OVERLAPPED 
{
  BLUE_BOOL status ;
  BLUE_HANDLE hFile ;
  BLUE_DWORD dwLen ;
  BLUE_OFFT offset ;
  BLUE_HANDLE hContext ;
  BLUE_HANDLE hUser ;
#if defined(BLUE_PARAM_PERF_STATS)
  BLUE_INT perf_id ;
  BLUE_MSTIME stamp ;
#endif
} BLUE_OVERLAPPED ;

typedef BLUE_OVERLAPPED *BLUE_LPOVERLAPPED ;
/** \endlatexonly */

/**
 * Basic Information for a file
 */
typedef struct _BLUE_FILE_BASIC_INFO
{
  /**
   * The time fthe file was created
   */
  BLUE_LARGE_INTEGER CreationTime ;
  /**
   * The time the file was last accessed
   */
  BLUE_LARGE_INTEGER LastAccessTime ;
  /**
   * The time the file was last written to
   */
  BLUE_LARGE_INTEGER LastWriteTime ;
  /**
   * The time the file was changed
   */
  BLUE_LARGE_INTEGER ChangeTime ;
  /**
   * The File Attributes
   */
  BLUE_DWORD FileAttributes ;
} BLUE_FILE_BASIC_INFO ;

#define BLUE_FILE_BASIC_CREATION_TIME 0
#define BLUE_FILE_BASIC_LAST_ACCESS_TIME 8
#define BLUE_FILE_BASIC_LAST_WRITE_TIME 16
#define BLUE_FILE_BASIC_CHANGE_TIME 24
#define BLUE_FILE_BASIC_ATTRIBUTES 32
#define BLUE_FILE_BASIC_SIZE 36

typedef struct _BLUE_FILE_INTERNAL_INFO
{
  BLUE_LARGE_INTEGER IndexNumber ;
} BLUE_FILE_INTERNAL_INFO ;

typedef struct _BLUE_FILE_EA_INFO
{
  BLUE_DWORD EaSize ;
} BLUE_FILE_EA_INFO ;

typedef struct _BLUE_FILE_ACCESS_INFO
{
  BLUE_DWORD AccessFlags ;
} BLUE_FILE_ACCESS_INFO ;

typedef struct _BLUE_FILE_POSITION_INFO
{
  BLUE_LARGE_INTEGER CurrentByteOffset ;
} BLUE_FILE_POSITION_INFO ;

typedef struct _BLUE_FILE_MODE_INFO
{
  BLUE_DWORD Mode ;
} BLUE_FILE_MODE_INFO ;

typedef struct _BLUE_FILE_ALIGNMENT_INFO
{
  BLUE_DWORD AlignmentRequirement ;
} BLUE_FILE_ALIGNMENT_INFO ;

/**
 * Network Open Information for a file
 */
typedef struct _BLUE_FILE_NETWORK_OPEN_INFO
{
  /**
   * The time fthe file was created
   */
  BLUE_LARGE_INTEGER CreationTime ;
  /**
   * The time the file was last accessed
   */
  BLUE_LARGE_INTEGER LastAccessTime ;
  /**
   * The time the file was last written to
   */
  BLUE_LARGE_INTEGER LastWriteTime ;
  /**
   * The time the file was changed
   */
  BLUE_LARGE_INTEGER ChangeTime ;
  /**
   * The Alloction Size
   */
  BLUE_LARGE_INTEGER AllocationSize ;
  /**
   * The End of File
   */
  BLUE_LARGE_INTEGER EndOfFile ;
  /**
   * The File Attributes
   */
  BLUE_DWORD FileAttributes ;
} BLUE_FILE_NETWORK_OPEN_INFO ;

/**
 * Extended Information for a file
 */
typedef struct _BLUE_FILE_STANDARD_INFO
{
  /**
   * The amount of space that is allocated for the file
   */
  BLUE_LARGE_INTEGER AllocationSize ;
  /**
   * The end of the file
   */
  BLUE_LARGE_INTEGER EndOfFile ;
  /**
   * The number of links to the file
   */
  BLUE_DWORD NumberOfLinks ;
  /**
   * BLUE_TRUE if the file is being deleted
   */
  BLUE_BOOL DeletePending ;
  /**
   * BLUE_TRUE if the file is a directory
   */
  BLUE_BOOL Directory ;
} BLUE_FILE_STANDARD_INFO ;

#define BLUE_FILE_STANDARD_ALLOCATION_SIZE 0
#define BLUE_FILE_STANDARD_END_OF_FILE 8
#define BLUE_FILE_STANDARD_NUMBER_OF_LINKS 16
#define BLUE_FILE_STANDARD_DELETE_PENDING 20
#define BLUE_FILE_STANDARD_DIRECTORY 21
#define BLUE_FILE_STANDARD_SIZE 22

/**
 * Receives the File Name
 */
typedef struct _BLUE_FILE_NAME_INFO
{
  /**
   * The size of the FileName string
   */
  BLUE_DWORD FileNameLength ;
  /**
   * The file name
   */
  BLUE_WCHAR FileName[1] ;
} BLUE_FILE_NAME_INFO ;

/**
 * Contains the name to which the file should be renamed
 */
typedef struct _BLUE_FILE_RENAME_INFO
{
  /**
   * BLUE_TRUE to replace an existing file if it exists
   */
  BLUE_BOOL ReplaceIfExists ;
  /**
   * A Handle to the root directory of the file to be renamed
   */
  BLUE_HANDLE RootDirectory ;
  /**
   * The size of the FileName
   */
  BLUE_DWORD FileNameLength ;
  /**
   * The New File Name
   */
  BLUE_WCHAR FileName[1] ;
} BLUE_FILE_RENAME_INFO ;

/** 
 * Indicates whether a file should be deleted
 */
typedef struct _BLUE_FILE_DISPOSITION_INFO
{
  /**
   * Indicates whether the file should be deleted.  BLUE_TRUE if it should
   */
  BLUE_BOOL DeleteFile ;
} BLUE_FILE_DISPOSITION_INFO ;

/**
 * Contains the total number of bytes that should be allocated for a file
 */
typedef struct _BLUE_FILE_ALLOCATION_INFO
{
  /**
   * The new file allocation size
   */
  BLUE_LARGE_INTEGER AllocationSize ;
} BLUE_FILE_ALLOCATION_INFO ;

/** 
 * Contains the specified value to which the end of file should be set
 */
typedef struct _BLUE_FILE_END_OF_FILE_INFO
{
  /**
   * The specified value for the new end of file
   */
  BLUE_LARGE_INTEGER EndOfFile ;
} BLUE_FILE_END_OF_FILE_INFO ;

/**
 * Receives file stream info for the file
 */
typedef struct _BLUE_FILE_STREAM_INFO
{
  /**
   * The offset for the next FILE_STREAM_INFO entry
   */
  BLUE_DWORD NextEntryOffset ;
  /**
   * The length in bytes of the Stream Name
   */
  BLUE_DWORD StreamNameLength ;
  /**
   * The size of the data stream
   */
  BLUE_LARGE_INTEGER StreamSize ;
  /**
   * The amount of space allocated for the stream
   */
  BLUE_LARGE_INTEGER StreamAllocationSize ;
  /**
   * The Stream Name
   */
  BLUE_WCHAR StreamName[1] ;
} BLUE_FILE_STREAM_INFO ;

/**
 * Receives file compression information
 */
typedef struct _BLUE_FILE_COMPRESSION_INFO
{
  /**
   * The file size of the compressed file
   */
  BLUE_LARGE_INTEGER CompressedFileSize ;
  /**
   * The compression format
   */
  BLUE_WORD CompressionFormat ;
  /**
   * The factor that the compression uses
   */
  BLUE_UCHAR CompressionUnitShift ;
  /**
   * The number of chuncks that are shifted by compression
   */
  BLUE_UCHAR ChunkShift ;
  /**
   * The number of clusters that are shifted
   */
  BLUE_UCHAR ClusterShift ;
  /**
   * Reserved
   */
  BLUE_UCHAR Reserved[3] ;
} BLUE_FILE_COMPRESSION_INFO ;

/**
 * Receives the requested file attribute information
 */
typedef struct _BLUE_FILE_ATTRIBUTE_TAG_INFO
{
  /**
   * The file attribute information
   */
  BLUE_DWORD FileAttributes ;
  /**
   * The reparse tag
   */
  BLUE_DWORD ReparseTag ;
} BLUE_FILE_ATTRIBUTE_TAG_INFO ;

/**
 * Contains Information about files in a specified directory
 */
typedef struct _BLUE_FILE_DIR_INFO
{
  /**
   * The offset for the next FILE_ID_BOTH_DIR_INFO structure in the buffer
   */
  BLUE_DWORD NextEntryOffset ;
  /**
   * The byte offset of the file within the parent directory
   */
  BLUE_DWORD FileIndex ;
  /**
   * The time that the file was created
   */
  BLUE_LARGE_INTEGER CreationTime ;
  /**
   * The time that the file was last accessed
   */
  BLUE_LARGE_INTEGER LastAccessTime ;
  /**
   * The time that the file was last written to
   */
  BLUE_LARGE_INTEGER LastWriteTime ;
  /**
   * The time that the file was last changed 
   */
  BLUE_LARGE_INTEGER ChangeTime ;
  /**
   * The absolute end of file position.
   */
  BLUE_LARGE_INTEGER EndOfFile ;
} BLUE_FILE_DIR_INFO ;

/**
 * Contains Information about files in a specified directory
 */
typedef struct _BLUE_FILE_FULL_DIR_INFO
{
  /**
   * The offset for the next FILE_ID_BOTH_DIR_INFO structure in the buffer
   */
  BLUE_DWORD NextEntryOffset ;
  /**
   * The byte offset of the file within the parent directory
   */
  BLUE_DWORD FileIndex ;
  /**
   * The time that the file was created
   */
  BLUE_LARGE_INTEGER CreationTime ;
  /**
   * The time that the file was last accessed
   */
  BLUE_LARGE_INTEGER LastAccessTime ;
  /**
   * The time that the file was last written to
   */
  BLUE_LARGE_INTEGER LastWriteTime ;
  /**
   * The time that the file was last changed 
   */
  BLUE_LARGE_INTEGER ChangeTime ;
  /**
   * The absolute end of file position.
   */
  BLUE_LARGE_INTEGER EndOfFile ;
  /**
   * The number of bytes allocated for the file
   */
  BLUE_LARGE_INTEGER AllocationSize ;
  /**
   * The file attributes
   */
  BLUE_DWORD FileAttributes ;
  /**
   * The length of the file name
   */
  BLUE_DWORD FileNameLength ;
  /**
   * The size of the extended attributes for the file
   */
  BLUE_DWORD EaSize ;
} BLUE_FILE_FULL_DIR_INFO ;

/**
 * Contains Information about files in a specified directory
 */
typedef struct _BLUE_FILE_ID_FULL_DIR_INFO
{
  /**
   * The offset for the next FILE_ID_BOTH_DIR_INFO structure in the buffer
   */
  BLUE_DWORD NextEntryOffset ;
  /**
   * The byte offset of the file within the parent directory
   */
  BLUE_DWORD FileIndex ;
  /**
   * The time that the file was created
   */
  BLUE_LARGE_INTEGER CreationTime ;
  /**
   * The time that the file was last accessed
   */
  BLUE_LARGE_INTEGER LastAccessTime ;
  /**
   * The time that the file was last written to
   */
  BLUE_LARGE_INTEGER LastWriteTime ;
  /**
   * The time that the file was last changed 
   */
  BLUE_LARGE_INTEGER ChangeTime ;
  /**
   * The absolute end of file position.
   */
  BLUE_LARGE_INTEGER EndOfFile ;
  /**
   * The number of bytes allocated for the file
   */
  BLUE_LARGE_INTEGER AllocationSize ;
  /**
   * The file attributes
   */
  BLUE_DWORD FileAttributes ;
  /**
   * The length of the file name
   */
  BLUE_DWORD FileNameLength ;
  /**
   * The size of the extended attributes for the file
   */
  BLUE_DWORD EaSize ;
  /**
   * Reserved
   */
  BLUE_DWORD Reserved ;
  /**
   * The File ID
   */
  BLUE_LARGE_INTEGER FileId ;
  /**
   * The File Name
   */
  BLUE_WCHAR FileName[1] ;
} BLUE_FILE_ID_FULL_DIR_INFO ;

/**
 * Contains Information about files in a specified directory
 */
typedef struct _BLUE_FILE_BOTH_DIR_INFO
{
  /**
   * The offset for the next FILE_ID_BOTH_DIR_INFO structure in the buffer
   */
  BLUE_DWORD NextEntryOffset ;
  /**
   * The byte offset of the file within the parent directory
   */
  BLUE_DWORD FileIndex ;
  /**
   * The time that the file was created
   */
  BLUE_LARGE_INTEGER CreationTime ;
  /**
   * The time that the file was last accessed
   */
  BLUE_LARGE_INTEGER LastAccessTime ;
  /**
   * The time that the file was last written to
   */
  BLUE_LARGE_INTEGER LastWriteTime ;
  /**
   * The time that the file was last changed 
   */
  BLUE_LARGE_INTEGER ChangeTime ;
  /**
   * The absolute end of file position.
   */
  BLUE_LARGE_INTEGER EndOfFile ;
  /**
   * The number of bytes allocated for the file
   */
  BLUE_LARGE_INTEGER AllocationSize ;
  /**
   * The file attributes
   */
  BLUE_DWORD FileAttributes ;
  /**
   * The length of the file name
   */
  BLUE_DWORD FileNameLength ;
  /**
   * The size of the extended attributes for the file
   */
  BLUE_DWORD EaSize ;
  /**
   * The length of the short name (8.3 name)
   */
  BLUE_CHAR ShortNameLength ;
  /**
   * Reserved
   */
  BLUE_CHAR Reserved ;
  /**
   * The short 8.3 file name
   */
  BLUE_WCHAR ShortName[12] ;
  /**
   * The File Name
   */
  BLUE_WCHAR FileName[1] ;
} BLUE_FILE_BOTH_DIR_INFO ;

/**
 * Contains Information about files in a specified directory
 */
typedef struct _BLUE_FILE_ID_BOTH_DIR_INFO
{
  /**
   * The offset for the next FILE_ID_BOTH_DIR_INFO structure in the buffer
   */
  BLUE_DWORD NextEntryOffset ;
  /**
   * The byte offset of the file within the parent directory
   */
  BLUE_DWORD FileIndex ;
  /**
   * The time that the file was created
   */
  BLUE_LARGE_INTEGER CreationTime ;
  /**
   * The time that the file was last accessed
   */
  BLUE_LARGE_INTEGER LastAccessTime ;
  /**
   * The time that the file was last written to
   */
  BLUE_LARGE_INTEGER LastWriteTime ;
  /**
   * The time that the file was last changed 
   */
  BLUE_LARGE_INTEGER ChangeTime ;
  /**
   * The absolute end of file position.
   */
  BLUE_LARGE_INTEGER EndOfFile ;
  /**
   * The number of bytes allocated for the file
   */
  BLUE_LARGE_INTEGER AllocationSize ;
  /**
   * The file attributes
   */
  BLUE_DWORD FileAttributes ;
  /**
   * The length of the file name
   */
  BLUE_DWORD FileNameLength ;
  /**
   * The size of the extended attributes for the file
   */
  BLUE_DWORD EaSize ;
  /**
   * The length of the short name (8.3 name)
   */
  BLUE_CHAR ShortNameLength ;
  /**
   * The short 8.3 file name
   */
  BLUE_WCHAR ShortName[12] ;
  /**
   * The File ID
   */
  BLUE_LARGE_INTEGER FileId ;
  /**
   * The File Name
   */
  BLUE_WCHAR FileName[1] ;
} BLUE_FILE_ID_BOTH_DIR_INFO ;

typedef struct _BLUE_FILE_ALL_INFO
{
  BLUE_FILE_BASIC_INFO BasicInfo ;
  BLUE_FILE_STANDARD_INFO StandardInfo ;
  BLUE_FILE_INTERNAL_INFO InternalInfo ;
  BLUE_FILE_EA_INFO EAInfo ;
  BLUE_FILE_ACCESS_INFO AccessInfo ;
  BLUE_FILE_POSITION_INFO PositionInfo ;
  BLUE_FILE_MODE_INFO ModeInfo ;
  BLUE_FILE_ALIGNMENT_INFO AlignmentInfo ;
  BLUE_FILE_NAME_INFO NameInfo ;
} BLUE_FILE_ALL_INFO ;

/*
 * Access Mask Encodings
 */
#define BLUE_FILE_READ_DATA 0x00000001
#define BLUE_FILE_WRITE_DATA 0x00000002
#define BLUE_FILE_APPEND_DATA 0x00000004
#define BLUE_FILE_READ_EA 0x00000008
#define BLUE_FILE_WRITE_EA 0x00000010
#define BLUE_FILE_DELETE_CHILD 0x00000040
#define BLUE_FILE_EXECUTE 0x00000020
#define BLUE_FILE_READ_ATTRIBUTES 0x00000080
#define BLUE_FILE_WRITE_ATTRIBUTES 0x00000100
#define BLUE_DELETE 0x00010000
#define BLUE_READ_CONTROL 0x00020000
#define BLUE_WRITE_DAC 0x00040000
#define BLUE_WRITE_OWNER 0x00080000
#define BLUE_SYNCHRONIZE 0x00100000
#define BLUE_ACCESS_SYSTEM_SECURITY 0x01000000
#define BLUE_MAXIMUM_ALLOWED 0x02000000
#define BLUE_GENERIC_ALL 0x10000000
#define BLUE_GENERIC_EXECUTE 0x20000000
#define BLUE_GENERIC_WRITE 0x40000000
#define BLUE_GENERIC_READ 0x80000000

#define BLUE_FILE_LIST_DIRECTORY 0x00000001
#define BLUE_FILE_ADD_FILE 0x00000002
#define BLUE_FILE_ADD_SUBDIRECTORY 0x00000004
#define BLUE_FILE_DELETE_CHILD 0x00000040
#define BLUE_MAXIMUM_ALLOWED 0x02000000

/**
 * Contains Information about files in a specified directory
 */
typedef struct _BLUE_FILE_NAMES_INFO
{
  /**
   * The offset for the next FILE_ID_BOTH_DIR_INFO structure in the buffer
   */
  BLUE_DWORD NextEntryOffset ;
  /**
   * The byte offset of the file within the parent directory
   */
  BLUE_DWORD FileIndex ;
  /**
   * The length of the file name
   */
  BLUE_DWORD FileNameLength ;
  /**
   * The File Name
   */
  BLUE_WCHAR FileName[1] ;
} BLUE_FILE_NAMES_INFO ;

/**
 * Defines values for File I/O Priority
 */
typedef enum _BLUE_PRIORITY_HINT
  {
    BlueIoPriorityHintVeryLow = 0,
    BlueIoPriorityHintLow,
    BlueIoPriorityHintNormal,
    BlueMaximumIoPriorityHintType
  } BLUE_PRIORITY_HINT ;

/**
 * Specifies the priority hint for a file I/O operation
 */
typedef struct _BLUE_FILE_IO_PRIORITY_HINT_INFO
{
  /**
   * The pirority hint
   */
  BLUE_PRIORITY_HINT PriorityHint ;
} BLUE_FILE_IO_PRIORITY_HINT_INFO ;

/* The MS-FSCC spec defines the following.  For some reason, I get
 * something different back
 */
typedef struct _BLUE_FILEFS_SIZE_INFO
{
  BLUE_LARGE_INTEGER TotalAllocationUnits ;
  BLUE_LARGE_INTEGER AvailableAllocationUnits ;
  BLUE_UINT32 SectorsPerAllocationUnit ;
  BLUE_UINT32 BytesPerSector ;
} BLUE_FILEFS_SIZE_INFO ;

#define FILEFS_SIZE_TOTAL_ALLOCATION_UNITS 0
#define FILEFS_SIZE_AVAILABLE_ALLOCATION_UNITS 8
#define FILEFS_SIZE_SECTORS_PER_ALLOCATION_UNIT 16
#define FILEFS_SIZE_BYTES_PER_SECTOR 20
#define FILEFS_SIZE_SIZE 24

typedef struct _BLUE_FILEFS_FULL_SIZE_INFO
{
  BLUE_LARGE_INTEGER TotalAllocationUnits ;
  BLUE_LARGE_INTEGER CallerAvailableAllocationUnits ;
  BLUE_LARGE_INTEGER ActualAvailableAllocationUnits ;
  BLUE_UINT32 SectorsPerAllocationUnit ;
  BLUE_UINT32 BytesPerSector ;
} BLUE_FILEFS_FULL_SIZE_INFO ;

typedef struct _BLUE_FILEFS_ATTRIBUTE_INFO
{
  BLUE_UINT32 FileSystemAttributes ;
  BLUE_UINT32 MaximumComponentNameLength ;
  BLUE_UINT32 FileSystemNameLength ;
} BLUE_FILEFS_ATTRIBUTE_INFO ;

#define BLUE_FILEFS_FILE_SYSTEM_ATTRIBUTES 0
#define BLUE_FILEFS_MAX_COMPONENT_NAME_LENGTH 4
#define BLUE_FILEFS_FILE_SYSTEM_NAME_LENGTH 8
#define BLUE_FILEFS_FILE_SYSTEM_NAME 12

typedef struct _BLUE_FILEFS_VOLUME_INFO
{
  BLUE_LARGE_INTEGER VolumeCreationTime ;
  BLUE_UINT32 VolumeSerialNumber ;
  BLUE_UINT32 VolumeLabelLength ;
  BLUE_UINT8 SupportsObjects ;
  BLUE_UINT8 Reserved ;
} BLUE_FILEFS_VOLUME_INFO ;

#define BLUE_FILEFS_VOLUME_CREATION_TIME 0
#define BLUE_FILEFS_VOLUME_SERIAL_NUMBER 8
#define BLUE_FILEFS_VOLUME_LABEL_LENGTH 12
#define BLUE_FILEFS_SUPPORTS_OBJECTS 16
#define BLUE_FILEFS_RESERVED 17
#define BLUE_FILEFS_VOLUME_LABEL 18

typedef struct _BLUE_FILEFS_DEVICE_INFO
{
  BLUE_UINT32 DeviceType ;
  BLUE_UINT32 Characteristics ;
} BLUE_FILEFS_DEVICE_INFO ;

#define BLUE_FILE_DEVICE_CD_ROM 2
#define BLUE_FILE_DEVICE_DISK 7

#define BLUE_FILE_REMOVABLE_MEDIA 0x01
#define BLUE_FILE_READ_ONLY_DEVICE 0x02
#define BLUE_FILE_FLOPPY_DISKETTE 0x04
#define BLUE_FILE_WRITE_ONCE_MEDIA 0x08
#define BLUE_FILE_REMOTE_DEVICE 0x10
#define BLUE_FILE_DEVICE_IS_MOUNTED 0x20
#define BLUE_FILE_VIRTUAL_VOLUME 0x40
#define BLUE_FILE_DEVICE_SECURE_OPEN 0x100
#define BLUE_FILE_CHARACTERISTIC_TS_DEVICE 0x1000
#define BLUE_FILE_CHARACTERISTIC_WEBDEV_DEVICE 0x2000
#define BLUE_FILE_DEVICE_ALLOW_APPCONTAINER_TRAVERSAL 0x20000
#define BLUE_FILE_PORTABLE_DEVICE 0x4000

/**
 * Enumerator for Move Method
 */
enum
  {
    /**
     * Move relative from the beginning of the file
     */
    BLUE_FILE_BEGIN = 0,
    /**
     * Move relative to the current file pointer
     */
    BLUE_FILE_CURRENT = 1,
    /**
     * Move relative to EOF
     */
    BLUE_FILE_END = 2
  } ;

/**
 * Flags for BlueLockFileEx
 */
enum 
  {
    BLUE_LOCKFILE_FAIL_IMMEDIATELY = 0x01,
    BLUE_LOCKFILE_EXCLUSIVE_LOCK = 0x02
  } ;

/**
 * Value returned to BlueSetFilePointer function when the pointer could not
 * be set
 */
#define BLUE_INVALID_SET_FILE_POINTER ((BLUE_DWORD)-1)

/**
 * Error Codes that can be returned by File APIs
 */
typedef enum
  {
    BLUE_ERROR_SUCCESS = 0,
    BLUE_ERROR_INVALID_FUNCTION = 1,
    BLUE_ERROR_FILE_NOT_FOUND = 2,
    BLUE_ERROR_PATH_NOT_FOUND = 3,
    BLUE_ERROR_TOO_MANY_OPEN_FILES = 4,
    BLUE_ERROR_ACCESS_DENIED = 5,
    BLUE_ERROR_INVALID_HANDLE = 6,
    BLUE_ERROR_NOT_ENOUGH_MEMORY = 8,
    BLUE_ERROR_INVALID_ACCESS = 12,
    BLUE_ERROR_OUTOFMEMORY = 14,
    BLUE_ERROR_INVALID_DRIVE = 15,
    BLUE_ERROR_CURRENT_DIRECTORY = 16,
    BLUE_ERROR_NOT_SAME_DEVICE = 17,
    BLUE_ERROR_NO_MORE_FILES = 18,
    BLUE_ERROR_WRITE_PROTECT = 19,
    BLUE_ERROR_NOT_READY = 21,
    BLUE_ERROR_CRC = 23, 
    BLUE_ERROR_BAD_LENGTH = 24,
    BLUE_ERROR_SEEK = 25,
    BLUE_ERROR_WRITE_FAULT = 29,
    BLUE_ERROR_READ_FAULT = 30,
    BLUE_ERROR_GEN_FAILURE = 31,
    BLUE_ERROR_SHARING_VIOLATION = 32,
    BLUE_ERROR_LOCK_VIOLATION = 33,
    BLUE_ERROR_WRONG_DISK = 34,
    BLUE_ERROR_SHARING_BUFFER_EXCEEDED = 35,
    BLUE_ERROR_HANDLE_EOF = 38,
    BLUE_ERROR_HANDLE_DISK_FULL = 39,
    BLUE_ERROR_BAD_NET_NAME = 43,
    BLUE_ERROR_NOT_SUPPORTED = 50,
    BLUE_ERROR_REM_NOT_LIST = 51,
    BLUE_ERROR_DUP_NAME = 52,
    BLUE_ERROR_BAD_NETPATH = 53,
    BLUE_ERROR_NETWORK_BUSY = 54,
    BLUE_ERROR_DEV_NOT_EXIST = 55,
    BLUE_ERROR_BAD_NET_RESP = 58,
    BLUE_ERROR_UNEXP_NET_ERR = 59,
    BLUE_ERROR_BAD_DEV_TYPE = 66,
    BLUE_ERROR_FILE_EXISTS = 80,
    BLUE_ERROR_CANNOT_MAKE = 82,
    BLUE_ERROR_INVALID_PASSWORD = 86,
    BLUE_ERROR_INVALID_PARAMETER = 87,
    BLUE_ERROR_NET_WRITE_FAULT = 88,
    BLUE_ERROR_MORE_ENTRIES = 105,
    BLUE_ERROR_BROKEN_PIPE = 109,
    BLUE_ERROR_OPEN_FAILED = 110,
    BLUE_ERROR_BUFFER_OVERFLOW = 111,
    BLUE_ERROR_DISK_FULL = 112,
    BLUE_ERROR_CALL_NOT_IMPLEMENTED = 120,
    BLUE_ERROR_INSUFFICIENT_BUFFER = 122,
    BLUE_ERROR_INVALID_NAME = 123,
    BLUE_ERROR_INVALID_LEVEL = 124,
    BLUE_ERROR_NO_VOLUME_LABEL = 125,
    BLUE_ERROR_NEGATIVE_SEEK = 131,
    BLUE_ERROR_SEEK_ON_DEVICE = 132,
    BLUE_ERROR_DIR_NOT_EMPTY = 145,
    BLUE_ERROR_PATH_BUSY = 148,
    BLUE_ERROR_BAD_ARGUMENTS = 160,
    BLUE_ERROR_BAD_PATHNAME = 161,
    BLUE_ERROR_BUSY = 170,
    BLUE_ERROR_ALREADY_EXISTS = 183,
    BLUE_ERROR_INVALID_FLAG_NUMBER = 186,
    BLUE_ERROR_BAD_PIPE = 230,
    BLUE_ERROR_PIPE_BUSY = 231,
    BLUE_ERROR_NO_DATA = 232,
    BLUE_ERROR_PIPE_NOT_CONNECTED = 233,
    BLUE_ERROR_MORE_DATA = 234,
    BLUE_ERROR_INVALID_EA_NAME = 254,
    BLUE_ERROR_EA_LIST_INCONSISTENT = 255,
    BLUE_ERROR_DIRECTORY = 267,
    BLUE_ERROR_EAS_DIDNT_FIT = 275,
    BLUE_ERROR_EA_FILE_CORRUPT = 276,
    BLUE_ERROR_EA_TABLE_FULL = 277,
    BLUE_ERROR_INVALID_EA_HANDLE = 278,
    BLUE_ERROR_EAS_NOT_SUPPORTED = 282,
    BLUE_ERROR_OPLOCK_NOT_GRANTED = 300,
    BLUE_ERROR_DISK_TOO_FRAGMENTED = 302,
    BLUE_ERROR_DELETE_PENDING = 303,
    BLUE_ERROR_PIPE_CONNECTED = 535,
    BLUE_ERROR_PIPE_LISTENING = 536,
    BLUE_ERROR_EA_ACCESS_DENIED = 994,
    BLUE_ERROR_OPERATION_ABORTED = 995,
    BLUE_ERROR_IO_INCOMPLETE = 996,
    BLUE_ERROR_IO_PENDING = 997,
    BLUE_ERROR_NOACCESS = 998,
    BLUE_ERROR_INVALID_FLAGS = 1004,
    BLUE_ERROR_UNRECOGNIZED_VOLUME = 1005,
    BLUE_ERROR_FILE_INVALID = 1006,
    BLUE_ERROR_NOTIFY_ENUM_DIR = 1022,
    BLUE_ERROR_BUS_RESET = 1111,
    BLUE_ERROR_IO_DEVICE = 1117,
    BLUE_ERROR_DISK_OPERATION_FAILED = 1127,
    BLUE_ERROR_BAD_DEVICE = 1200,
    BLUE_ERROR_INVALID_PASSWORDNAME = 1215,
    BLUE_ERROR_LOGON_FAILURE = 1326,
    BLUE_ERROR_NOT_ENOUGH_QUOTA = 1816
  } BLUE_FILE_ERRORS ;

/**
 * \defgroup BlueFileCall A Blue Call application for the Blue File API
 * \ingroup BlueFile
 * \{ 
 */

enum
  {
    FILE_CALL_TAG_CREATE,
    FILE_CALL_TAG_CLOSE,
    FILE_CALL_TAG_DELETE,
    FILE_CALL_TAG_FIND_FIRST,
    FILE_CALL_TAG_FIND_NEXT,
    FILE_CALL_TAG_FIND_CLOSE,
    FILE_CALL_TAG_FLUSH_BUFFERS,
    FILE_CALL_TAG_GET_FILEEX,
    FILE_CALL_TAG_QUERY_INFO,
    FILE_CALL_TAG_MOVE_FILE,
    FILE_CALL_TAG_READ,
    FILE_CALL_TAG_WRITE,
    FILE_CALL_TAG_SET_EOF,
    FILE_CALL_TAG_SET_FILEEX,
    FILE_CALL_TAG_SET_HFILEEX,
    FILE_CALL_TAG_SET_FILE_POINTER,
    FILE_CALL_TAG_TRANSACT_LANMAN,
    FILE_CALL_TAG_TRANSACT2_NAMED_PIPE,
    FILE_CALL_TAG_GET_VOLUME_INFO,
    FILE_CALL_TAG_REMOVE_DIR,
    FILE_CALL_TAG_LOCK,
    FILE_CALL_TAG_NOTIFY_COMPLETE,
    FILE_CALL_TAG_DISMOUNT,
    FILE_CALL_TAG_DEVICE_IO_CONTROL,
    FILE_CALL_TAG_QUERY_DIRECTORY
  } ;

/*
 * Overlapped I/O Complete Notification
 */
typedef struct
{
  BLUE_DWORD nNumberOfBytesTransferred ; /**< The number of bytes xferred */
  BLUE_UINT32 offset ;
} FILE_NOTIFY_COMPLETE ;

/*
 * Security attributes descriptor is pushed
 */
typedef struct 
{
  BLUE_DWORD dwDesiredAccess ;
  BLUE_DWORD dwShareMode ;
  BLUE_DWORD secLength ;
  BLUE_BOOL secInherit ;
  BLUE_DWORD dwCreationDisposition ;
  BLUE_DWORD dwFlagsAndAttributes ;
  BLUE_DWORD dwCreateOptions ;
  BLUE_UINT32 createAction ;
  BLUE_UINT64 CreationTime ;
  BLUE_UINT64 LastAccessTime ;
  BLUE_UINT64 LastWriteTime ;
  BLUE_UINT64 LastChangeTime ;
  BLUE_UINT32 dwExtAttributes ;
  BLUE_UINT64 AllocationSize ;
  BLUE_UINT64 EndOfFile ;
  BLUE_UINT16 fileType ;
  BLUE_UINT16 deviceState ;
  BLUE_UINT16 Directory ;
} FILE_CALL_CREATE ;

typedef struct
{
  BLUE_HANDLE find_file ;
} FILE_CALL_CLOSE ;

typedef struct
{
  BLUE_INT empty ;
} FILE_CALL_DELETE ;

typedef struct
{
  BLUE_INT empty ;
} FILE_CALL_REMOVE_DIR ;

/* Find data is pushed */
typedef struct
{
  BLUE_BOOL more ;
  BLUE_BOOL close_after ;
  BLUE_BOOL close_eos ;
  BLUE_UINT16 search_count ;
  BLUE_SIZET nFindFileDataSize ;
} FILE_CALL_FIND ;

/* Query Directory data is pushed */
typedef struct
{
  BLUE_BOOL more ;
  BLUE_UINT32 name_length ;
  BLUE_UINT32 buffer_length ;
  BLUE_HANDLE find_file ;
  BLUE_BOOL reopen ;
} FILE_CALL_QUERY_DIRECTORY ;

typedef struct
{
  BLUE_INT empty ;
} FILE_CALL_FIND_CLOSE ;


typedef struct
{
  BLUE_INT empty ;
} FILE_CALL_FLUSH_BUFFERS ;

/* lpFileInformation is pushed */
typedef struct
{
  BLUE_FILE_INFO_BY_HANDLE_CLASS FileInformationClass ;
  BLUE_DWORD dwBufferSize ;
} FILE_CALL_GET_FILEEX ;

/* lpFileInformation is pushed */
typedef struct
{
  BLUE_FILE_INFO_BY_HANDLE_TYPE FileInformationType ;
  BLUE_DWORD FileInformationClass ;
  BLUE_DWORD dwBufferSize ;
} FILE_CALL_QUERY_INFO ;

/* new file name is pushed for move */

typedef struct
{
  BLUE_OFFT offBuffer ;
  BLUE_DWORD nNumberOfBytesToRead ;
  BLUE_DWORD nNumberOfBytesRead ;
  BLUE_HANDLE msgqOverlapped ;
  /*
   * These are added to help the CIFS write call.  Others that call write
   * should set distance to move to 0 and move method to FILE_CURRENT.
   */
  BLUE_LONG lDistanceToMove ;
  BLUE_LONG lDistanceToMoveHigh ;
  BLUE_DWORD dwMoveMethod ;
  BLUE_DWORD nLowOffset ;
  BLUE_DWORD nHighOffset ;
} FILE_CALL_READ ;

typedef struct
{
  BLUE_OFFT offBuffer ;
  BLUE_DWORD nNumberOfBytesToWrite ;
  BLUE_DWORD nNumberOfBytesWritten ;
  BLUE_HANDLE msgqOverlapped ;
  /*
   * These are added to help the CIFS write call.  Others that call write
   * should set distance to move to 0 and move method to FILE_CURRENT.
   */
  BLUE_LONG lDistanceToMove ;
  BLUE_LONG lDistanceToMoveHigh ;
  BLUE_DWORD dwMoveMethod ;
  BLUE_DWORD nLowOffset ;
  BLUE_DWORD nHighOffset ;
} FILE_CALL_WRITE ;

typedef struct
{
  BLUE_INT empty ;
} FILE_CALL_SET_EOF ;

/* lpFileInformation is pushed */
typedef struct
{
  BLUE_FILE_INFO_BY_HANDLE_CLASS FileInformationClass ;
  BLUE_DWORD dwBufferSize ;
} FILE_CALL_SET_FILEEX ;

/* lpFileInformation is pushed */
typedef struct
{
  BLUE_FILE_INFO_BY_HANDLE_CLASS FileInformationClass ;
  BLUE_DWORD dwBufferSize ;
} FILE_CALL_SET_HFILEEX ;

typedef struct
{
  BLUE_LONG lDistanceToMove ;
  BLUE_LONG lDistanceToMoveHigh ;
  BLUE_DWORD dwMoveMethod ;
  BLUE_DWORD dwPosition ;
} FILE_CALL_SET_FILE_POINTER ;

typedef struct
{
  BLUE_DWORD nInBufferSize ;
  BLUE_DWORD nOutBufferSize ;
  BLUE_DWORD nBytesRead ;
  BLUE_HANDLE msgqOverlapped ;
} FILE_CALL_TRANSACT2_NAMED_PIPE ;

typedef struct
{
  BLUE_DWORD dwIoControlCode ;
  BLUE_DWORD nInBufferSize ;
  BLUE_DWORD nOutBufferSize ;
  BLUE_DWORD nBytesReturned ;
  BLUE_HANDLE msgqOverlapped ;
} FILE_CALL_DEVICE_IO_CONTROL ;

typedef struct
{
  BLUE_DWORD nInParamSize ;
  BLUE_DWORD nInDataSize ;
  BLUE_DWORD nOutParamSize ;
  BLUE_DWORD nOutParamRead ;
  BLUE_DWORD nOutDataSize ;
  BLUE_DWORD nOutDataRead ;
} FILE_CALL_TRANSACT_LANMAN ;

/* volume name buffer is pushed */
typedef struct
{
  BLUE_DWORD dwSectorsPerCluster ;
  BLUE_DWORD dwBytesPerSector ;
  BLUE_DWORD dwNumberOfFreeClusters ;
  BLUE_DWORD dwTotalNumberOfClusters ;
  BLUE_DWORD nVolumeNameSize ;
  BLUE_DWORD dwVolumeSerialNumber ;
  BLUE_DWORD dwMaximumComponentLength ;
  BLUE_DWORD dwFileSystemFlags ;
  BLUE_DWORD nFileSystemNameSize ;
} FILE_CALL_GET_VOLUME_INFO ;

typedef struct
{
  BLUE_UINT32 offset_high ;
  BLUE_UINT32 offset_low ;
  BLUE_UINT32 length_high ;
  BLUE_UINT32 length_low ;
} FILE_CALL_LOCK_RANGE ;

typedef enum
  {
    FileCallLockLevelShared,
    FileCallLockLevelExclusive
  } FILE_CALL_LOCK_LEVEL ;

/* ranges are pushed */
typedef struct
{
  FILE_CALL_LOCK_LEVEL lock_level ;
  BLUE_BOOL change_level ;
  BLUE_UINT16 num_unlocks ;
  BLUE_UINT16 num_locks ;
} FILE_CALL_LOCK ;

typedef struct
{
  BLUE_INT empty ;
} FILE_CALL_DISMOUNT ;

/* file name is pushed */
typedef struct
{
  BLUE_BOOL status ;
  BLUE_DWORD dwLastError ;
  BLUE_HANDLE hFile ;
  BLUE_MSTIME stamp ;
  union 
  {
    FILE_CALL_CREATE create ;
    FILE_CALL_CLOSE close ;
    FILE_CALL_DELETE del ;
    FILE_CALL_FIND find ;
    FILE_CALL_FIND_CLOSE find_close ;
    FILE_CALL_FLUSH_BUFFERS flush_buffers ;
    FILE_CALL_GET_FILEEX get_fileex ;
    FILE_CALL_QUERY_INFO query_info ;
    FILE_CALL_READ read ;
    FILE_CALL_WRITE write ;
    FILE_CALL_SET_EOF set_eof ;
    FILE_CALL_SET_FILEEX set_fileex ;
    FILE_CALL_SET_HFILEEX set_hfileex ;
    FILE_CALL_SET_FILE_POINTER set_file_pointer ;
    FILE_CALL_TRANSACT2_NAMED_PIPE transact2_named_pipe ;
    FILE_CALL_TRANSACT_LANMAN transact_lanman ;
    FILE_CALL_GET_VOLUME_INFO get_volume_info ;
    FILE_CALL_REMOVE_DIR remove_dir ;
    FILE_CALL_LOCK lock ;
    FILE_NOTIFY_COMPLETE complete ;
    FILE_CALL_DISMOUNT dismount ;
    FILE_CALL_DEVICE_IO_CONTROL device_io_control ;
    FILE_CALL_QUERY_DIRECTORY query_directory ;
  } u ;
} FILE_CALL ;

/**
 * File System Global Last Error
 */
extern BLUE_DWORD BlueLastError ;

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Initialize the Blue File Redirector
   *
   * This should only be called by the BlueInit Layer
   */  
  BLUE_CORE_LIB BLUE_VOID 
  BlueFileInit (BLUE_VOID) ;
  BLUE_CORE_LIB BLUE_VOID 
  BlueFileDestroy (BLUE_VOID);
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
  BLUE_CORE_LIB BLUE_BOOL 
  BlueCloseHandle (BLUE_HANDLE hObject) ;
  /**
   * Create or Open a File
   *
   * Use this call to obtain a file handle for a new or existing file
   *
   * \param lpFileName
   * The name of the object to be crated or opened.  
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
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueCreateFileW (BLUE_LPCTSTR lpFileName,
		   BLUE_DWORD dwDesiredAccess,
		   BLUE_DWORD dwShareMode,
		   BLUE_LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		   BLUE_DWORD dwCreationDisposition,
		   BLUE_DWORD dwFlagsAndAttributes,
		   BLUE_HANDLE hTemplateFile) ;
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueCreateFileA (BLUE_LPCSTR lpFileName,
		   BLUE_DWORD dwDesiredAccess,
		   BLUE_DWORD dwShareMode,
		   BLUE_LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		   BLUE_DWORD dwCreationDisposition,
		   BLUE_DWORD dwFlagsAndAttributes,
		   BLUE_HANDLE hTemplateFile) ;
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
  BLUE_CORE_LIB BLUE_BOOL 
  BlueCreateDirectoryW (BLUE_LPCTSTR lpPathName,
			BLUE_LPSECURITY_ATTRIBUTES lpSecurityAttr) ;
  BLUE_CORE_LIB BLUE_BOOL 
  BlueCreateDirectoryA (BLUE_LPCSTR lpPathName,
			BLUE_LPSECURITY_ATTRIBUTES lpSecurityAttr) ;
  /**
   * Deletes an existing file
   *
   * \param lpFileName
   * The name of the file in UNC or SMB URL format
   *
   * \returns
   * If success, returns BLUE_TRUE, if failure returns BLUE_FALSE
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueDeleteFileW(BLUE_LPCTSTR lpFileName) ;
  BLUE_CORE_LIB BLUE_BOOL 
  BlueDeleteFileA(BLUE_LPCSTR lpFileName) ;
  /**
   * Deletes a directory
   *
   * \param lpPathName
   * The name of the file in UNC or SMB URL format
   *
   * \returns
   * If success, returns BLUE_TRUE, if failure returns BLUE_FALSE
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueRemoveDirectoryW(BLUE_LPCTSTR lpPathName) ;
  BLUE_CORE_LIB BLUE_BOOL 
  BlueRemoveDirectoryA(BLUE_LPCSTR lpPathName) ;
  /**
   * Searches a director for a file or subdirectory that matches the name
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
   * \param more
   * Pointer to where to return more indication.  BLUE_TRUE says more files are
   * available.  BLUE_FALSE says this is the last one
   *
   * \returns
   * If the function succeeds, it will return a handle that can be used
   * in subsequent BlueFindNextFile or BlueFindClose call.
   * If the function failed, it will return BLUE_INVALID_HANDLE_VALUE.
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueFindFirstFileW (BLUE_LPCTSTR lpFileName,
		      BLUE_LPWIN32_FIND_DATAW lpFindFileData,
		      BLUE_BOOL *more) ;
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueFindFirstFileA (BLUE_LPCSTR lpFileName,
		      BLUE_LPWIN32_FIND_DATAA lpFindFileData,
		      BLUE_BOOL *more) ;
  /**
   * Continues a search from a previous call to BlueFindFirstFile
   *
   * \param hFindFile
   * The search handle returned from BlueFindFirstFile
   *
   * \param lpFindFileData
   * A pointer to the BLUE_WIN32_FIND_DATA structure.  See
   * http://msdn2.microsoft.com/en-us/library/aa365247.aspx
   *
   * \param more
   * Pointer to where to return more indication.  BLUE_TRUE says more files are
   * available.  BLUE_FALSE says this is the last one
   *
   * \returns
   * BLUE_TRUE if the call succeeded, BLUE_FALSE otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueFindNextFileW (BLUE_HANDLE hFindFile,
		     BLUE_LPWIN32_FIND_DATAW lpFindFileData,
		     BLUE_BOOL *more) ;
  BLUE_CORE_LIB BLUE_BOOL 
  BlueFindNextFileA (BLUE_HANDLE hFindFile,
		     BLUE_LPWIN32_FIND_DATAA lpFindFileData,
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
  BLUE_CORE_LIB BLUE_BOOL 
  BlueFindClose (BLUE_HANDLE hFindFile) ;
  /**
   * Write all buffered data to the file and clear any buffer cache
   *
   * \param hFile
   * A handle to the open file.
   *
   * \returns
   * BLUE_TRUE if the call succeeded, BLUE_FALSE otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueFlushFileBuffers (BLUE_HANDLE hFile) ;
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
   * see http://msdn2.microsoft.com/en-us/library/aa365739.aspx
   *
   * \returns
   * BLUE_TRUE if the function succeeded, BLUE_FALSE otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueGetFileAttributesExW (BLUE_LPCTSTR lpFileName,
			    BLUE_GET_FILEEX_INFO_LEVELS fInfoLevelId,
			    BLUE_LPVOID lpFileInformation) ;
  BLUE_CORE_LIB BLUE_BOOL 
  BlueGetFileAttributesExA (BLUE_LPCSTR lpFileName,
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
  BLUE_CORE_LIB BLUE_BOOL 
  BlueGetFileInformationByHandleEx (BLUE_HANDLE hFile,
				    BLUE_FILE_INFO_BY_HANDLE_CLASS
				    FileInformationClass,
				    BLUE_LPVOID lpFileInformation,
				    BLUE_DWORD dwBufferSize) ;

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
  BLUE_CORE_LIB BLUE_BOOL 
  BlueMoveFileW (BLUE_LPCTSTR lpExistingFileName,
		 BLUE_LPCTSTR lpNewFileName) ;
  BLUE_CORE_LIB BLUE_BOOL 
  BlueMoveFileA (BLUE_LPCSTR lpExistingFileName,
		 BLUE_LPCSTR lpNewFileName) ;
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
   * \param hOverlapped
   * Handle to the overlapped structure
   *
   * \returns
   * BLUE_FALSE if the function fails, BLUE_TRUE otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueReadFile (BLUE_HANDLE hFile,
		BLUE_LPVOID lpBuffer,
		BLUE_DWORD nNumberOfBytesToRead,
		BLUE_LPDWORD lpNumberOfBytesRead,
		BLUE_HANDLE hOverlapped) ;
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
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueCreateOverlapped (BLUE_HANDLE hFile) ;
  /**
   * Destroy an Overlapped I/O context
   *
   * \param hFile
   * File handle that overlapped context handle is for
   * 
   * \param hOverlapped
   * The overlapped context to destroy
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueDestroyOverlapped (BLUE_HANDLE hFile, BLUE_HANDLE hOverlapped) ;
  /**
   * Set the offset at which an overlapped I/O should occur
   *
   * Since Overlapped I/O is platform specific, this abstraction
   * is provided to communicate to the File handler the file offset
   * to perform the I/O at
   *
   * \param hFile
   * File to set the offset for
   *
   * \param hOverlapped
   * The handle to the overlapped context
   *
   * \param offset
   * The file offset to set
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueSetOverlappedOffset (BLUE_HANDLE hFile, BLUE_HANDLE hOverlapped, 
			   BLUE_OFFT offset) ;
  /**
   * Retrieves the results of an overlapped operation on the specified file
   *
   * \param hFile
   * A handle to the file
   *
   * \param hOverlapped
   * Handle to the overlapped structure
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
  BLUE_CORE_LIB BLUE_BOOL 
  BlueGetOverlappedResult (BLUE_HANDLE hFile,
			   BLUE_HANDLE hOverlapped,
			   BLUE_LPDWORD lpNumberOfBytesTransferred,
			   BLUE_BOOL bWait) ;
  /**
   * Sets the physical file size for the specified file to the current position
   *
   * \param hFile
   * Handle to the file
   *
   * \returns 
   * BLUE_TRUE if success, BLUE_FALSE otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueSetEndOfFile (BLUE_HANDLE hFile) ;
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
  BLUE_CORE_LIB BLUE_BOOL 
  BlueSetFileAttributesW (BLUE_LPCTSTR lpFileName, 
			  BLUE_DWORD dwFileAttributes) ;
  BLUE_CORE_LIB BLUE_BOOL 
  BlueSetFileAttributesA (BLUE_LPCSTR lpFileName,
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
  BLUE_CORE_LIB BLUE_BOOL 
  BlueSetFileInformationByHandle (BLUE_HANDLE hFile,
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
  BLUE_CORE_LIB BLUE_DWORD 
  BlueSetFilePointer (BLUE_HANDLE hFile, BLUE_LONG lDistanceToMove,
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
   * \param hOverlapped
   * Handle to the overlapped structure for asynchronous I/O
   *
   * \returns
   * BLUE_TRUE if success, BLUE_FALSE if failed
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueWriteFile (BLUE_HANDLE hFile,
		 BLUE_LPCVOID lpBuffer,
		 BLUE_DWORD nNumberOfBytesToWrite,
		 BLUE_LPDWORD lpNumberOfBytesWritten,
		 BLUE_HANDLE hOverlapped) ;
  /**
   * Perform a transaction on a named pipe
   *
   * \param hFile
   * File handle of named pipe
   *
   * \param lpInBuffer
   * Pointer to the buffer to send to the pipe
   *
   * \param nInBufferSize
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
  BLUE_CORE_LIB BLUE_BOOL 
  BlueTransactNamedPipe (BLUE_HANDLE hFile,
			 BLUE_LPVOID lpInBuffer,
			 BLUE_DWORD nInBufferSize,
			 BLUE_LPVOID lpOutBuffer,
			 BLUE_DWORD nOutBufferSize,
			 BLUE_LPDWORD lpBytesRead,
			 BLUE_HANDLE hOverlapped) ;
  /**
   * Get the last file error that occured in this file handler for
   * this thread
   *
   * \returns
   * Last File Error
   *
   * NOTE: Use BlueGetLastError instead.
   */
  BLUE_CORE_LIB BLUE_UINT32 
  BlueGetLastFileError (BLUE_HANDLE hHandle) ;
  /**
   * Get the last error encountered by the Blue File Component of Blue Share
   *
   * \returns
   * Error Code
   */
  BLUE_CORE_LIB BLUE_DWORD 
  BlueGetLastError (BLUE_VOID) ;

  BLUE_CORE_LIB const BLUE_CHAR *BlueGetErrorString(BLUE_DWORD dwerr) ;
  /**
   * Get the last file error that occured in the file handler for a 
   * specified file in this thread's context
   *
   * \param lpFileName
   * Name of the file to map to a file handler
   *
   * \returns
   * Last File Error
   *
   * NOTE: Use BlueGetLastError instead.
   */
  BLUE_CORE_LIB BLUE_UINT32 
  BlueGetLastFileNameErrorW (BLUE_LPCTSTR lpFileName) ;
  BLUE_CORE_LIB BLUE_UINT32 
  BlueGetLastFileNameErrorA (BLUE_LPCSTR lpFileName) ;
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
  BLUE_CORE_LIB BLUE_BOOL 
  BlueGetDiskFreeSpaceW (BLUE_LPCTSTR lpRootPathName,
			 BLUE_LPDWORD lpSectorsPerCluster,
			 BLUE_LPDWORD lpBytesPerSector,
			 BLUE_LPDWORD lpNumberOfFreeClusters,
			 BLUE_LPDWORD lpTotalNumberOfClusters) ;
  BLUE_CORE_LIB BLUE_BOOL 
  BlueGetDiskFreeSpaceA (BLUE_LPCSTR lpRootPathName,
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
  BLUE_CORE_LIB BLUE_BOOL 
  BlueGetVolumeInformationW (BLUE_LPCTSTR lpRootPathName,
			     BLUE_LPTSTR lpVolumeNameBuffer,
			     BLUE_DWORD nVolumeNameSize,
			     BLUE_LPDWORD lpVolumeSerialNumber,
			     BLUE_LPDWORD lpMaximumComponentLength,
			     BLUE_LPDWORD lpFileSystemFlags,
			     BLUE_LPTSTR lpFileSystemName,
			     BLUE_DWORD nFileSystemName) ;

  BLUE_CORE_LIB BLUE_BOOL 
  BlueGetVolumeInformationA (BLUE_LPCSTR lpRootPathName,
			     BLUE_LPSTR lpVolumeNameBuffer,
			     BLUE_DWORD nVolumeNameSize,
			     BLUE_LPDWORD lpVolumeSerialNumber,
			     BLUE_LPDWORD lpMaximumComponentLength,
			     BLUE_LPDWORD lpFileSystemFlags,
			     BLUE_LPSTR lpFileSystemName,
			     BLUE_DWORD nFileSystemName) ;
  /**
   * Return the type of file system that a file is on
   *
   * \param hHandle
   * Handle to the file
   *
   * \returns
   * The type of file system
   */  
  BLUE_CORE_LIB BLUE_FS_TYPE 
  BlueFileGetFSType (BLUE_HANDLE hHandle) ;
  /**
   * An internal call to return the native file system handle
   *
   * \param hHandle
   * Handle to the file
   *
   * \returns
   * A handle to the lower file system specific file
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueFileGetFSHandle (BLUE_HANDLE hHandle) ;
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
		    BLUE_UINT32 length_low, 
		    BLUE_UINT32 length_high,
		    BLUE_HANDLE hOverlapped) ;
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
   * Handle to the overlapped structure
   *
   * \returns
   * BLUE_TRUE if successful, BLUE_FALSE otherwise
   */
  BLUE_CORE_LIB BLUE_BOOL 
  BlueLockFileEx (BLUE_HANDLE hFile, BLUE_DWORD flags,
		  BLUE_DWORD length_low, BLUE_DWORD length_high,
		  BLUE_HANDLE hOverlapped) ;
  /**
   * Get the event handle of the overlapped event (deprecated)
   *
   * \param hOverlapped
   * The overlapped event
   *
   * \returns
   * The event handle
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueFileGetOverlappedEvent (BLUE_HANDLE hOverlapped) ;
  /**
   * Get the Message Queue of the overlapped event
   *
   * \param hOverlapped
   * The overlapped event
   *
   * \returns
   * The msgq handle
   */
  BLUE_CORE_LIB BLUE_HANDLE 
  BlueFileGetOverlappedWaitQ (BLUE_HANDLE hOverlapped) ;
  /**
   * Dismount an SMB client connection
   *
   * This is a non-standard and optional API for those that wish to
   * ensure that a client connection to a server is disconnected.
   * Under normal operation, client connections are disconnected after
   * idle for a period of time.  
   *
   * \param lpFileName
   * A path that includes the server URL
   *
   * \returns
   * Status
   */
  BLUE_CORE_LIB BLUE_BOOL
  BlueDismountW (BLUE_LPCTSTR lpFileName) ;
  BLUE_CORE_LIB BLUE_BOOL
  BlueDismountA (BLUE_LPCSTR lpFileName) ;
  /**
   * Perform a Device IOCTL
   *
   * This is an internal API that is used for DFS resolution
   */
  BLUE_CORE_LIB BLUE_BOOL BlueDeviceIoControl (BLUE_HANDLE hFile, 
					       BLUE_DWORD dwIoControlCode,
					       BLUE_LPVOID lpInBuffer, 
					       BLUE_DWORD nInBufferSize, 
					       BLUE_LPVOID lpOutBuffer, 
					       BLUE_DWORD nOutBufferSize,
					       BLUE_LPDWORD lpBytesReturned,
					       BLUE_HANDLE hOverlapped) ;
#if defined(__cplusplus)
}
#endif

#if defined(BLUE_PARAM_UNICODE_API)
#define BlueCreateFile BlueCreateFileW
#define BlueCreateDirectory BlueCreateDirectoryW
#define BlueDeleteFile BlueDeleteFileW
#define BlueRemoveDirectory BlueRemoveDirectoryW
#define BlueFindFirstFile BlueFindFirstFileW
#define BlueFindNextFile BlueFindNextFileW
#define BlueGetFileAttributesEx BlueGetFileAttributesExW
#define BlueMoveFile BlueMoveFileW
#define BlueSetFileAttributes BlueSetFileAttributesW
#define BlueGetLastFileNameError BlueGetLastFileNameErrorW
#define BlueGetDiskFreeSpace BlueGetDiskFreeSpaceW
#define BlueGetVolumeInformation BlueGetVolumeInformationW
#define BlueDismount BlueDismountW

#define BLUE_WIN32_FIND_DATA BLUE_WIN32_FIND_DATAW
#define BLUE_LPWIN32_FIND_DATA BLUE_LPWIN32_FIND_DATAW

#else
#define BlueCreateFile BlueCreateFileA
#define BlueCreateDirectory BlueCreateDirectoryA
#define BlueDeleteFile BlueDeleteFileA
#define BlueRemoveDirectory BlueRemoveDirectoryA
#define BlueFindFirstFile BlueFindFirstFileA
#define BlueFindNextFile BlueFindNextFileA
#define BlueGetFileAttributesEx BlueGetFileAttributesExA
#define BlueMoveFile BlueMoveFileA
#define BlueSetFileAttributes BlueSetFileAttributesA
#define BlueGetLastFileNameError BlueGetLastFileNameErrorA
#define BlueGetDiskFreeSpace BlueGetDiskFreeSpaceA
#define BlueGetVolumeInformation BlueGetVolumeInformationA
#define BlueDismount BlueDismountA

#define BLUE_WIN32_FIND_DATA BLUE_WIN32_FIND_DATAA
#define BLUE_LPWIN32_FIND_DATA BLUE_LPWIN32_FIND_DATAA

#endif

/** \} */
#endif

