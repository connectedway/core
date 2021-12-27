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

#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/queue.h"
#include "ofc/path.h"
#include "ofc/libc.h"
#include "ofc/time.h"
#include "ofc/thread.h"
#include "ofc/event.h"
#include "ofc/waitset.h"
#include "ofc/console.h"
#include "ofc/process.h"

#include "ofc/heap.h"
#include "ofc/file.h"

#define BLUE_FS_TEST_INTERVAL 1000
#define OFC_FILE_TEST_COUNT 2

/*
 * This application demonstrates the API to the Blue Share File I/O Facility
 *
 * The Blue Share File I/O Facility is the primary interface fo the
 * Blue Share CIFS Client.  Once the CIFS client is configured, network
 * files are abstracted through this API.  Whether files are access over
 * the network, or local the the system that the application is running on
 * is transparent to the running application.  The location of the files
 * (whether local or remote) is encoded in the file path.  The file name
 * can explicitly specify a remote file using a UNC or SMB URL.  The file
 * name can also specify a local path that is mapped to a UNC or SMB URL.
 */

/*
 * Forward declaration of the File System Test Thread
 */
static BLUE_DWORD BlueFSTestApp (BLUE_PATH *path) ;

/*
 * This test utility will map the destination file location to the device
 * 'FS_TEST'.  The typical situation would likely have the application
 * specifying the path directly and not using a map.  In our case, the
 * advantage of the map is that we direct the target of the test simply
 * by changing the map.  The rest of the application remains unchanged.
 *
 * This test can be used to test the local file system abstraction
 * or can be used to test the CIFS file system abstraction.  Also,
 * by using a loopback address to the CIFS url, we can exercise the
 * CIFS Server as well.
 */

/*
 * These are various file and directory names.  There is nothing special
 * about any of these names.
 */
#define FS_TEST_READ TSTR("copy.from")
#define FS_TEST_WRITE TSTR("copy.to")
#define FS_TEST_DELETE TSTR("delete.me")
#define FS_TEST_RENAME TSTR("directory\\rename.me")
#define FS_TEST_RENAMETO TSTR("directory\\new.me")
#define FS_TEST_RENAMETO_ROOT TSTR("new.me")
#define FS_TEST_FLUSH TSTR("flush.me")
#define FS_TEST_DIRECTORY TSTR("directory")
#define FS_TEST_SETEOF TSTR("seteof.txt")
#define FS_TEST_GETEX TSTR("getex.txt")
/*
 * Buffering definitions.  We test using overlapped asynchronous I/O.  This
 * implies multi-buffering
 *
 * The Buffer Size
 */
#define BUFFER_SIZE 4096
/*
 * And the number of buffers
 */
#define NUM_FILE_BUFFERS 1
/*
 * Define buffer states.  
 */
typedef enum
  {
    BUFFER_STATE_IDLE,		/* There is no I/O active */
    BUFFER_STATE_READ,		/* Data is being read into the buffer */
    BUFFER_STATE_WRITE		/* Data is being written from the buffer */
  } BUFFER_STATE ;
/*
 * The buffer context
 *
 * Currently, handles to the overlapped i/o context may be platform 
 * dependent.  Because of this, an overlapped i/o may not be shared
 * between files unless it is guaranteed that the files are on the
 * same device (using the same type of overlapped context).
 *
 * Ideally, overlapped I/Os should be platform independent. This will
 * require changes to the way overlapped handles are managed.
 */
typedef struct
{
  BLUE_HANDLE readOverlapped ;	/* The handle to the buffer when reading */
  BLUE_HANDLE writeOverlapped ;	/* The handle to the buffer when writing */
  BLUE_CHAR *data ;		/* Pointer to the buffer */
  BUFFER_STATE state ;		/* Buffer state */
  BLUE_LARGE_INTEGER offset ;	/* Offset in file for I/O */
} BLUE_FILE_BUFFER ;
/*
 * Result of Async I/O
 *
 * This essentially is a BLUE_BOOL with the addition of a PENDING flag
 */
typedef enum
  {
    ASYNC_RESULT_DONE,		/* I/O is successful */
    ASYNC_RESULT_ERROR,		/* I/O was in error */
    ASYNC_RESULT_EOF,		/* I/O hit EOF */
    ASYNC_RESULT_PENDING	/* I/O is still pending */
  } ASYNC_RESULT ;

/*
 * Perform an I/O Read
 *
 * \param wait_set
 * The wait set that this I/O and it's overlapped handles will be part of
 *
 * \param read_file
 * Handle of read file
 * 
 * \param buffer
 * Pointer to buffer to read into
 *
 * \param dwLen
 * Length of buffer to read
 *
 * \returns
 * BLUE_TRUE if success, BLUE_FALSE otherwise
 */
static ASYNC_RESULT 
AsyncRead (BLUE_HANDLE wait_set, BLUE_HANDLE read_file, 
	   BLUE_FILE_BUFFER *buffer, BLUE_DWORD dwLen)
{
  ASYNC_RESULT result ;
  BLUE_BOOL status ;
    
  /*
   * initialize the read buffer using the read file, the read overlapped
   * handle and the current read offset
   */
  BlueCTrace ("Reading 0x%08x\n", BLUE_LARGE_INTEGER_LOW(buffer->offset)) ;
  BlueSetOverlappedOffset (read_file, buffer->readOverlapped, buffer->offset) ;
  /*
   * Set the state to reading
   */
  buffer->state = BUFFER_STATE_READ ;
  /*
   * Add the buffer to the wait set
   */
  BlueWaitSetAdd (wait_set, (BLUE_HANDLE) buffer, buffer->readOverlapped) ;
  /*
   * Issue the read (this will be non blocking)
   */
  status = BlueReadFile (read_file, buffer->data, dwLen,
			 BLUE_NULL, buffer->readOverlapped) ;
  /*
   * If it completed, the status will be BLUE_TRUE.  We actually expect
   * the status to fail and the last error to be BLUE_ERROR_IO_PENDING
   */
  if (status == BLUE_TRUE)
    result = ASYNC_RESULT_DONE ;
  else
    {
      BLUE_DWORD dwLastError ;
      /*
       * Let's check the last error
       */
      dwLastError = BlueGetLastError() ;
      if (dwLastError == BLUE_ERROR_IO_PENDING)
	{
	  /*
	   * This is what we expect, so say the I/O submission succeeded
	   */
	  result = ASYNC_RESULT_PENDING ;
	}
      else
	{
	  if (dwLastError == BLUE_ERROR_HANDLE_EOF)
	    result = ASYNC_RESULT_EOF ;
	  else
	    result = ASYNC_RESULT_ERROR ;
	  /*
	   * It's not pending
	   */
	  buffer->state = BUFFER_STATE_IDLE ;
	  BlueWaitSetRemove (wait_set, buffer->readOverlapped) ;
	}
    }
      
  return (result) ;
}

/*
 * Return the state of the read
 *
 * \param wait_set
 * Wait set that the I/O should be part of
 *
 * \param read_file
 * Handle to the read file
 *
 * \param buffer
 * Pointer to the buffer
 *
 * \param dwLen
 * Number of bytes to read / number of bytes read
 *
 * \returns
 * state of the read
 */
static ASYNC_RESULT AsyncReadResult (BLUE_HANDLE wait_set, 
				     BLUE_HANDLE read_file,
				     BLUE_FILE_BUFFER *buffer, 
				     BLUE_DWORD *dwLen)
{
  ASYNC_RESULT result ;
  BLUE_BOOL status ;
    
  /*
   * Get the overlapped result
   */
  status = BlueGetOverlappedResult (read_file, buffer->readOverlapped,
				    dwLen, BLUE_FALSE) ;
  /*
   * If the I/O is complete, status will be true and length will be non zero
   */
  if (status == BLUE_TRUE )
    {
      if (*dwLen == 0)
	{
	  result = ASYNC_RESULT_EOF ;
	}
      else
	{
	  result = ASYNC_RESULT_DONE ;
	}
    }
  else
    {
      BLUE_DWORD dwLastError ;
      /*
       * I/O may still be pending
       */
      dwLastError = BlueGetLastError () ;
      if (dwLastError == BLUE_ERROR_IO_PENDING)
	result = ASYNC_RESULT_PENDING ;
      else
	{
	  /*
	   * I/O may also be EOF
	   */
	  if (dwLastError != BLUE_ERROR_HANDLE_EOF)
	    {
	      BlueCprintf ("Read Error %d\n", dwLastError) ;
	      result = ASYNC_RESULT_ERROR ;
	    }
	  else
	    result = ASYNC_RESULT_EOF ;
	}
    }
    
  if (result != ASYNC_RESULT_PENDING)
    {
      /*
       * Finish up the buffer if the I/O is no longer pending
       */
      buffer->state = BUFFER_STATE_IDLE ;
      BlueWaitSetRemove (wait_set, buffer->readOverlapped) ;
    }
    
  return (result) ;
}

/*
 * Submit an asynchronous Write
 */
static BLUE_BOOL AsyncWrite (BLUE_HANDLE wait_set, BLUE_HANDLE write_file, 
			     BLUE_FILE_BUFFER *buffer, BLUE_DWORD dwLen)
{
  BLUE_BOOL status ;
    
  BlueCTrace ("Writing 0x%08x\n", BLUE_LARGE_INTEGER_LOW(buffer->offset)) ;
  BlueSetOverlappedOffset (write_file, buffer->writeOverlapped, 
			   buffer->offset) ;
    
  buffer->state = BUFFER_STATE_WRITE ;
  BlueWaitSetAdd (wait_set, (BLUE_HANDLE) buffer, buffer->writeOverlapped) ;
    
  status = BlueWriteFile (write_file, buffer->data, dwLen, BLUE_NULL,
			  buffer->writeOverlapped) ;
    
  if (status != BLUE_TRUE)
    {
      BLUE_DWORD dwLastError ;
        
      dwLastError = BlueGetLastError() ;
      if (dwLastError == BLUE_ERROR_IO_PENDING)
	status = BLUE_TRUE ;
      else
	{
	  buffer->state = BUFFER_STATE_IDLE ;
	  BlueWaitSetRemove (wait_set, buffer->writeOverlapped) ;
	}
    }
  return (status) ;
}

static ASYNC_RESULT AsyncWriteResult (BLUE_HANDLE wait_set, 
				      BLUE_HANDLE write_file,
				      BLUE_FILE_BUFFER *buffer, 
				      BLUE_DWORD *dwLen)
{
  ASYNC_RESULT result ;
  BLUE_BOOL status ;
    
  status = BlueGetOverlappedResult (write_file, buffer->writeOverlapped,
				    dwLen, BLUE_FALSE) ;
  if (status == BLUE_TRUE )
    result = ASYNC_RESULT_DONE ;
  else
    {
      BLUE_DWORD dwLastError ;
        
      dwLastError = BlueGetLastError() ;
      if (dwLastError == BLUE_ERROR_IO_PENDING)
	result = ASYNC_RESULT_PENDING ;
      else
	{
	  BlueCprintf ("Write Error %d\n", dwLastError) ;
	  result = ASYNC_RESULT_ERROR ;
	}
    }
    
  if (result != ASYNC_RESULT_PENDING)
    {
      buffer->state = BUFFER_STATE_IDLE ;
      BlueWaitSetRemove (wait_set, buffer->writeOverlapped) ;
    }
    
  return (result) ;
}

static BLUE_TCHAR *MakeFilename (BLUE_CTCHAR *device, BLUE_CTCHAR *name)
{
  BLUE_SIZET devlen ;
  BLUE_SIZET namelen ;
  BLUE_TCHAR *filename ;

  devlen = BlueCtastrlen (device) ;
  namelen = BlueCtastrlen (name) ;
  filename = 
    BlueHeapMalloc ((devlen + namelen + 1) * sizeof (BLUE_TCHAR)) ;
  BlueCtastrcpy (filename, device) ;
  BlueCtastrcpy (&filename[devlen], name) ;
  filename[devlen+namelen] = TCHAR_EOS ;
  return (filename) ;
}

/*
 * Create File Test.  This routine will create a file using a blocking
 * (sequential) algorithm
 */

/* 2 MB */
//#define CREATE_SIZE (2*1024*1024)
#define CREATE_SIZE (2*1024)

static BLUE_BOOL BlueCreateFileTest (BLUE_CTCHAR *device)
{
  BLUE_HANDLE write_file ;
  BLUE_MSTIME start_time ;
  BLUE_CHAR *buffer ;
  BLUE_INT i ;
  BLUE_BOOL status ;
  BLUE_TCHAR *wfilename ;
  BLUE_BOOL ret ;
  BLUE_INT size ;
  BLUE_ULONG *p ;
  BLUE_DWORD dwBytesWritten ;
                
  ret = BLUE_TRUE ;

  /*
   * Create the file used for the copy file test.
   * This is the read file
   */
  wfilename = MakeFilename (device, FS_TEST_READ) ;
  /*
   * Get the time for simple performance analysis
   */
  status = BLUE_TRUE ;
  start_time = BlueTimeGetNow() ;
#if defined(BLUE_PARAM_PERF_STATS) 
  BlueTimePerfReset() ;
#endif
  /*
   * Open up our write file.  If it exists, it will be deleted
   */
  write_file = BlueCreateFile (wfilename,
			       BLUE_GENERIC_WRITE,
			       BLUE_FILE_SHARE_DELETE |
			       BLUE_FILE_SHARE_WRITE |
			       BLUE_FILE_SHARE_READ,
			       BLUE_NULL,
			       BLUE_CREATE_ALWAYS,
			       BLUE_FILE_ATTRIBUTE_NORMAL,
			       BLUE_HANDLE_NULL) ;
        
  if (write_file == BLUE_INVALID_HANDLE_VALUE)
    {
      BlueCprintf ("Failed to open Copy Destination %A, Error Code %d\n",
		   wfilename, BlueGetLastError ()) ;
      ret = BLUE_FALSE ;
    }
  else
    {
      buffer = BlueHeapMalloc (BUFFER_SIZE) ;
      if (buffer == BLUE_NULL)
	{
	  BlueCprintf ("BlueFSTest: Failed to allocate Shared "
		       "memory buffer\n") ;
	  ret = BLUE_FALSE ;
	}
      else
	{
	  for (size = 0 ; size < CREATE_SIZE && ret == BLUE_TRUE; 
	       size += BUFFER_SIZE)
	    {
	      p = (BLUE_ULONG *) buffer ;
	      /*
	       * Fill in the buffer with an random data
	       */
	      for (i = 0 ; i < (BUFFER_SIZE / sizeof(BLUE_ULONG)) ; i++)
		{
		  *p++ = i ;
		}
		 
	      /*
	       * Write the buffer
	       */
	      status = BlueWriteFile (write_file, buffer, BUFFER_SIZE, 
				      &dwBytesWritten, BLUE_HANDLE_NULL) ;

	      if (status == BLUE_FALSE)
		{
		  BlueCprintf ("Write to File Failed with %d\n",
			       BlueGetLastError()) ;
		  ret = BLUE_FALSE ;
		}
	      else
		{
		  if (size % (1024*1024) == 0)
		    BlueCprintf ("Wrote %d MB\n", (size / (1024*1024))+1) ;
		}
	    }
	  BlueHeapFree (buffer) ;
	}
      BlueCloseHandle (write_file) ;
    }
    
  BlueHeapFree (wfilename) ;

#if defined(BLUE_PARAM_PERF_STATS)
  BlueTimePerfDump() ;
#endif
  if (ret == BLUE_TRUE)
    BlueCprintf ("Create File Done, Elapsed Time %dms\n", 
		 BlueTimeGetNow() - start_time) ;
  else
    BlueCprintf ("Create File Test Failed\n") ;
  return (ret) ;
}

static BLUE_BOOL BlueDismountTest (BLUE_CTCHAR *device)
{
  BLUE_BOOL status ;
  BLUE_TCHAR *wfilename ;
  BLUE_BOOL ret ;
                
  ret = BLUE_TRUE ;

  /*
   * Create the file used for the dismount file test.
   */
  wfilename = MakeFilename (device, FS_TEST_READ) ;

  status = BLUE_TRUE ;
  /*
   * Dismount the file
   */
  BlueDismount (wfilename) ;

  BlueHeapFree (wfilename) ;

  return (ret) ;
}

static BLUE_BOOL BlueCopyFileTest (BLUE_CTCHAR *device)
{
  BLUE_HANDLE read_file ;
  BLUE_HANDLE write_file ;
  BLUE_MSTIME start_time ;
  BLUE_LARGE_INTEGER offset ;
  BLUE_BOOL eof ;
  BLUE_INT pending ;
  BLUE_HANDLE buffer_list ;
  BLUE_FILE_BUFFER *buffer ;
  BLUE_INT i ;
  BLUE_HANDLE wait_set ;
  BLUE_DWORD dwLen ;
  BLUE_BOOL status ;
  ASYNC_RESULT result ;
  BLUE_HANDLE hEvent ;
  BLUE_TCHAR *rfilename ;
  BLUE_TCHAR *wfilename ;
  BLUE_BOOL ret ;
                
  ret = BLUE_TRUE ;

  rfilename = MakeFilename (device, FS_TEST_READ) ;
  wfilename = MakeFilename (device, FS_TEST_WRITE) ;
  /*
   * Get the time for simple performance analysis
   */
  status = BLUE_TRUE ;
  start_time = BlueTimeGetNow() ;
#if defined(BLUE_PARAM_PERF_STATS) 
  BlueTimePerfReset() ;
#endif
  /*
   * Open up our read file.  This file should
   * exist
   */
  read_file = BlueCreateFile (rfilename,
			      BLUE_GENERIC_READ,
			      BLUE_FILE_SHARE_READ,
			      BLUE_NULL,
			      BLUE_OPEN_EXISTING,
			      BLUE_FILE_ATTRIBUTE_NORMAL | 
			      BLUE_FILE_FLAG_OVERLAPPED,
			      BLUE_HANDLE_NULL) ;
    
  if (read_file == BLUE_INVALID_HANDLE_VALUE)
    {
      BlueCprintf ("Failed to open Copy Source %A, Error Code %d\n",
		   rfilename, BlueGetLastError ()) ;
      ret = BLUE_FALSE ;
    }
  else
    {
      /*
       * Open up our write file.  If it exists, it will be deleted
       */
      write_file = BlueCreateFile (wfilename,
				   BLUE_GENERIC_WRITE,
				   0,
				   BLUE_NULL,
				   BLUE_CREATE_ALWAYS,
				   BLUE_FILE_ATTRIBUTE_NORMAL | 
				   BLUE_FILE_FLAG_OVERLAPPED,
				   BLUE_HANDLE_NULL) ;
        
      if (write_file == BLUE_INVALID_HANDLE_VALUE)
	{
	  BlueCprintf ("Failed to open Copy Destination %A, Error Code %d\n",
		       wfilename, BlueGetLastError ()) ;
	  ret = BLUE_FALSE ;
	}
      else
	{
	  /*
	   * Now, create a wait set that we will wait for
	   */
	  wait_set = BlueWaitSetCreate() ;
	  /*
	   * And create our own buffer list that we will manage
	   */
	  buffer_list = BlueQcreate () ;
	  /*
	   * Set some flags
	   */
	  eof = BLUE_FALSE ;
	  BLUE_LARGE_INTEGER_SET(offset, 0, 0) ;
	  pending = 0 ;
	  /*
	   * Prime the engine.  Priming involves obtaining a buffer
	   * for each overlapped I/O and initilizing them
	   */
	  for (i = 0 ; i < NUM_FILE_BUFFERS && !eof ; i++)
	    {
	      /*
	       * Get the buffer descriptor and the data buffer
	       */
	      buffer = BlueHeapMalloc (sizeof (BLUE_FILE_BUFFER)) ;
	      if (buffer == BLUE_NULL)
		{
		  BlueCprintf ("BlueFSTest: Failed to alloc buffer context\n") ;
		  ret = BLUE_FALSE ;
		}
	      else
		{
		  buffer->data = BlueHeapMalloc (BUFFER_SIZE) ;
		  if (buffer->data == BLUE_NULL)
		    {
		      BlueCprintf ("BlueFSTest: Failed to allocate "
				   "memory buffer\n") ;
		      BlueHeapFree (buffer) ;
		      buffer = BLUE_NULL ;
		      ret = BLUE_FALSE ;
		    }
		  else
		    {
		      /*
		       * Initialize the offset to the file
		       */
		      buffer->offset = offset ;
		      /*
		       * And initialize the overlapped handles
		       */
		      buffer->readOverlapped = BlueCreateOverlapped(read_file) ;
		      buffer->writeOverlapped = 
			BlueCreateOverlapped(write_file) ;
		      if (buffer->readOverlapped == BLUE_HANDLE_NULL ||
			  buffer->writeOverlapped == BLUE_HANDLE_NULL)
			BlueProcessCrash("An Overlapped Handle is NULL");
		      /*
		       * Add it to our buffer list
		       */
		      BlueQenqueue (buffer_list, buffer) ;
		      dwLen = BUFFER_SIZE ;
		      /*
		       * Issue the read (pre increment the pending to 
		       * avoid races
		       */
		      pending++ ;
		      result = AsyncRead (wait_set, read_file, buffer, dwLen) ;
		      if (result != ASYNC_RESULT_PENDING)
			{
			  /*
			   * discount pending and set eof
			   */
			  pending-- ;
			  if (result == ASYNC_RESULT_ERROR)
			    {
			      ret = BLUE_FALSE ;
			    }
			  /*
			   * Set eof either because it really is eof, or we
			   * want to clean up.
			   */
			  eof = BLUE_TRUE ;
			}
		      /*
		       * Prepare for the next buffer
		       */
#if defined(BLUE_PARAM_64BIT_INTEGER)
		      offset += BUFFER_SIZE ;
#else
		      offset.low += BUFFER_SIZE ;
#endif
		    }
		}
	    }
	  /*
	   * Now all our buffers should be busy doing reads.  Keep pumping 
	   * more data to read and service writes
	   */
	  while (pending > 0)
	    {
	      /*
	       * Wait for some buffer to finish (may be a read if we've
	       * just finished priming, but it may be a write also if
	       * we've been in this loop a bit
	       */
	      hEvent = BlueWaitSetWait (wait_set) ;
	      if (hEvent != BLUE_HANDLE_NULL)
		{
		  /*
		   * We use the app of the event as a pointer to the
		   * buffer descriptor.  Yeah, this isn't really nice but
		   * the alternative is to add a context to each handle.
		   * That may be cleaner, but basically unnecessary.  If 
		   * we did this kind of thing a lot, I'm all for a
		   * new property of a handle
		   */
		  buffer = (BLUE_FILE_BUFFER *) BlueHandleGetApp (hEvent) ;
		  /*
		   * Now we have both read and write overlapped descriptors
		   * See what state we're in
		   */
		  if (buffer->state == BUFFER_STATE_READ)
		    {
		      /*
		       * Read, so let's see the result of the read
		       */
		      result = AsyncReadResult (wait_set, read_file, 
						buffer, &dwLen) ;
		      if (result == ASYNC_RESULT_ERROR)
			{
			  ret = BLUE_FALSE ;
			}
		      else if (result == ASYNC_RESULT_DONE)
			{
			  /*
			   * When the read is done, let's start up the write
			   */
			  status = AsyncWrite (wait_set, write_file, buffer, 
					       dwLen) ;
			  if (status == BLUE_FALSE)
			    {
			      ret = BLUE_FALSE ;
			    }
			}
		      /*
		       * If the write failed, or we had a read result error
		       */
		      if (result == ASYNC_RESULT_ERROR || 
			  result == ASYNC_RESULT_EOF ||
			  (result == ASYNC_RESULT_DONE && status == BLUE_FALSE))
			{
			  /*
			   * The I/O is no longer pending.
			   */
			  pending-- ;
			  eof = BLUE_TRUE ;
			}
		    }
		  else
		    {
		      /*
		       * The buffer state was write.  Let's look at our
		       * status
		       */
		      result = AsyncWriteResult (wait_set, write_file, 
						 buffer, &dwLen) ;
		      /*
		       * Did it finish
		       */
		      if (result == ASYNC_RESULT_DONE)
			{
			  /*
			   * The write is finished.
			   * Let's step the buffer 
			   */
			  /*
			   * Only report on MB boundaries 
			   */
			  if (buffer->offset % (1024*1024) == 0)
			    BlueCprintf ("Copied %d MB\n", 
					 (buffer->offset / (1024 * 1024))+1) ;

			  BLUE_LARGE_INTEGER_ASSIGN (buffer->offset, offset) ;
#if defined(BLUE_PARAM_64BIT_INTEGER)
			  offset += BUFFER_SIZE ;
#else
			  offset.low += BUFFER_SIZE ;
#endif
			  /*
			   * And start a read on the next chunk
			   */
			  result = AsyncRead (wait_set, read_file, 
					      buffer, BUFFER_SIZE) ;
			}

		      if (result != ASYNC_RESULT_PENDING)
			{
			  if (result == ASYNC_RESULT_ERROR)
			    ret = BLUE_FALSE ;

			  eof = BLUE_TRUE ;
			  pending-- ;
			}
		    }
		}
	    }
            
	  /*
	   * The pending count is zero so we've gotten completions
	   * either due to errors or eof on all of our outstanding
	   * reads and writes.
	   */
	  for (buffer = BlueQdequeue (buffer_list) ;
	       buffer != BLUE_NULL ;
	       buffer = BlueQdequeue (buffer_list))
	    {
	      /*
	       * Destroy the overlapped I/O handle for each buffer
	       */
	      BlueDestroyOverlapped (read_file, buffer->readOverlapped) ;
	      BlueDestroyOverlapped (write_file, buffer->writeOverlapped) ;
	      /*
	       * Free the data buffer and the buffer descriptor
	       */
	      BlueHeapFree (buffer->data) ;
	      BlueHeapFree (buffer) ;
	    }
	  /*
	   * Destroy the buffer list
	   */
	  BlueQdestroy (buffer_list) ;
	  /*
	   * And destroy the wait list
	   */
	  BlueWaitSetDestroy (wait_set) ;
	  /*
	   * Now close the write file
	   */
	  BlueCloseHandle (write_file) ;
	}
      /*
       * And close the read file
       */
      BlueCloseHandle (read_file) ;
    }
    
  BlueHeapFree (rfilename) ;
  BlueHeapFree (wfilename) ;

#if defined(BLUE_PARAM_PERF_STATS)
  BlueTimePerfDump() ;
#endif
  if (ret == BLUE_TRUE)
    BlueCprintf ("Copy Done, Elapsed Time %dms\n", 
		 BlueTimeGetNow() - start_time) ;
  else
    BlueCprintf ("Copy Test Failed\n") ;
  return (ret) ;
}

/*
 * Delete File Test
 *
 * \param delete test type
 */
static BLUE_BOOL 
BlueDeleteTest (BLUE_CTCHAR *device)
{
  BLUE_HANDLE write_file ;
  BLUE_DWORD dwLastError ;
  BLUE_CHAR *buffer ;
  BLUE_SIZET len ;
  BLUE_BOOL status ;
  BLUE_TCHAR *filename ;
  BLUE_DWORD dwBytesWritten ;
  BLUE_BOOL ret ;

  ret = BLUE_TRUE ;
  filename = MakeFilename (device, FS_TEST_DELETE) ;
  /*
   * First we need to create a file to delete
   * This also tests us creating and writing to a file without overlap
   * i/o
   */
  write_file = BlueCreateFile (filename,
			       BLUE_GENERIC_WRITE,
			       BLUE_FILE_SHARE_READ,
			       BLUE_NULL,
			       BLUE_CREATE_ALWAYS,
			       BLUE_FILE_ATTRIBUTE_NORMAL,
			       BLUE_HANDLE_NULL) ;
        
  if (write_file == BLUE_INVALID_HANDLE_VALUE)
    {
      BlueCprintf ("Failed to create delete file %A, Error Code %d\n",
		   filename, BlueGetLastError ()) ;
      ret = BLUE_FALSE ;
    }
  else
    {
      /*
       * Let's write some stuff to the file
       */
      buffer = BlueHeapMalloc (BUFFER_SIZE) ;
      len = BUFFER_SIZE ;

      while (len > 0)
	{
	  len -= BlueCsnprintf (buffer + (BUFFER_SIZE-len), len,
				"This Text begins at offset %d\n", 
				BUFFER_SIZE - len) ;
	}
      /*
       * And write it to the file
       */
      status = BlueWriteFile (write_file, buffer, BUFFER_SIZE, &dwBytesWritten,
			      BLUE_HANDLE_NULL) ;
      /*
       * Free the buffer
       */
      BlueHeapFree (buffer) ;
      if (status != BLUE_TRUE)
	{
	  /*
	   * We failed writing, complain
	   */
	  dwLastError = BlueGetLastError () ;
	  BlueCprintf ("Write to Delete File Failed with Error %d\n",
		       dwLastError) ;
	  ret = BLUE_FALSE ;
	}
      else
	{
	  /* Close file
	   */
	  status = BlueCloseHandle (write_file) ;
	  if (status != BLUE_TRUE)
	    {
	      dwLastError = BlueGetLastError () ;
	      BlueCprintf ("Close of Delete File Failed with Error %d\n",
			   dwLastError) ;
	      ret = BLUE_FALSE ;
	    }
	  else
	    {
	      /*
	       * We've written the file.  Now depending on the type
	       * of delete, delete the file
	       */
	      write_file = 
		BlueCreateFile (filename,
				BLUE_FILE_DELETE,
				BLUE_FILE_SHARE_DELETE,
				BLUE_NULL,
				BLUE_OPEN_EXISTING,
				BLUE_FILE_FLAG_DELETE_ON_CLOSE,
				BLUE_HANDLE_NULL) ;
        
	      if (write_file == BLUE_INVALID_HANDLE_VALUE)
		{
		  BlueCprintf ("Failed to create delete on close file %A, "
			       "Error Code %d\n",
			       filename,
			       BlueGetLastError ()) ;
		  ret = BLUE_FALSE ;
		}
	      else
		{
		  /* Close file
		   */
		  status = BlueCloseHandle (write_file) ;
		  if (status != BLUE_TRUE)
		    {
		      dwLastError = BlueGetLastError () ;
		      BlueCprintf ("Close of Delete on close File "
				   "Failed with Error %d\n",
				   dwLastError) ;
		      ret = BLUE_FALSE ;
		    }
		  else
		    BlueCprintf ("Delete on Close Test Succeeded\n") ;
		}
	    }
	}
    }
  BlueHeapFree (filename) ;
  return (ret) ;
}

/*
 * Rename file test
 */
static BLUE_BOOL BlueMoveTest (BLUE_CTCHAR *device)
{
  BLUE_HANDLE rename_file ;
  BLUE_DWORD dwLastError ;
  BLUE_CHAR *buffer ;
  BLUE_SIZET len ;
  BLUE_BOOL status ;
  BLUE_TCHAR *filename ;
  BLUE_TCHAR *tofilename ;
  BLUE_DWORD dwBytesWritten ;
  BLUE_BOOL ret ;
  BLUE_TCHAR *dirname ; 
  BLUE_HANDLE dirHandle ;

  /*
   * Rename within a directory
   */
  ret = BLUE_TRUE ;
  dirname = MakeFilename (device, FS_TEST_DIRECTORY) ;
  status = BlueCreateDirectory (dirname, BLUE_NULL) ;
  if (status == BLUE_TRUE || BlueGetLastError () == BLUE_ERROR_FILE_EXISTS)
    {
      ret = BLUE_TRUE ;
      filename = MakeFilename (device, FS_TEST_RENAME) ;
      tofilename = MakeFilename (device, FS_TEST_RENAMETO) ;
      /*
       * First, make sure the destination file is not there
       * We don't care about the error.  It is probable, file does not exist
       */
      BlueDeleteFile (tofilename) ;
      /*
       * Now create a file to rename
       * Clean up any debris before starting
       */
      status = BlueDeleteFile (tofilename) ;
      /*
       * First we need to create a file to rename
       * This also tests us creating and writing to a file without overlap
       * i/o
       */
      rename_file = BlueCreateFile (filename,
				    BLUE_GENERIC_WRITE,
				    BLUE_FILE_SHARE_READ,
				    BLUE_NULL,
				    BLUE_CREATE_ALWAYS,
				    BLUE_FILE_ATTRIBUTE_NORMAL,
				    BLUE_HANDLE_NULL) ;
        
      if (rename_file == BLUE_INVALID_HANDLE_VALUE)
	{
	  BlueCprintf ("Failed to create rename file %A, Error Code %d\n",
		       filename, BlueGetLastError ()) ;
	  ret = BLUE_FALSE ;
	}
      else
	{
	  /*
	   * Let's write some stuff to the file
	   */
	  buffer = BlueHeapMalloc (BUFFER_SIZE) ;
	  len = BUFFER_SIZE ;

	  while (len > 0)
	    {
	      len -= BlueCsnprintf (buffer + (BUFFER_SIZE-len), len,
				    "This Text begins at offset %d\n", 
				    BUFFER_SIZE - len) ;
	    }

	  status = BlueWriteFile (rename_file, buffer, BUFFER_SIZE,
				  &dwBytesWritten, BLUE_HANDLE_NULL) ;
    
	  BlueHeapFree (buffer) ;
	  if (status != BLUE_TRUE)
	    {
	      dwLastError = BlueGetLastError () ;
	      BlueCprintf ("Write to Rename File Failed with Error %d\n",
			   dwLastError) ;
	      ret = BLUE_FALSE ;
	    }
	  else
	    {
	      /* Close file
	       */
	      status = BlueCloseHandle (rename_file) ;
	      if (status != BLUE_TRUE)
		{
		  dwLastError = BlueGetLastError () ;
		  BlueCprintf ("Close of Rename File Failed with Error %d\n",
			       dwLastError) ;
		  ret = BLUE_FALSE ;
		}
	      else
		{
		  /*
		   * Now we can rename the file
		   */
		  status = BlueMoveFile (filename, tofilename) ;
		  if (status == BLUE_TRUE)
		    {
		      /*
		       * Now we can delete the file
		       */
		      status = BlueDeleteFile (tofilename) ;
		      if (status == BLUE_TRUE)
			BlueCprintf ("Rename Test Succeeded\n") ;
		      else
			{
			  dwLastError = BlueGetLastError () ;
			  BlueCprintf ("Delete Rename File Failed with Error %d\n",
				       dwLastError) ;
			  ret = BLUE_FALSE ;
			}
		    }
		  else
		    {
		      dwLastError = BlueGetLastError () ;
		      BlueCprintf ("Rename File Failed with Error %d\n",
				   dwLastError) ;
		      ret = BLUE_FALSE ;
		    }
		}
	    }
	}
      BlueHeapFree (filename) ;
      BlueHeapFree (tofilename) ;

      dirHandle = BlueCreateFile (dirname,
				  BLUE_FILE_DELETE,
				  BLUE_FILE_SHARE_DELETE,
				  BLUE_NULL,
				  BLUE_OPEN_EXISTING,
				  BLUE_FILE_FLAG_DELETE_ON_CLOSE |
				  BLUE_FILE_ATTRIBUTE_DIRECTORY,
				  BLUE_HANDLE_NULL) ;
        
      if (dirHandle == BLUE_INVALID_HANDLE_VALUE)
	{
	  BlueCprintf ("Failed to create delete on close dir %A, "
		       "Error Code %d\n",
		       dirname,
		       BlueGetLastError ()) ;
	  ret = BLUE_FALSE ;
	}
      else
	{
	  /* Close file
	   */
	  status = BlueCloseHandle (dirHandle) ;
	  if (status != BLUE_TRUE)
	    {
	      dwLastError = BlueGetLastError () ;
	      BlueCprintf ("Close of Delete on close dir "
			   "Failed with Error %d\n",
			   dwLastError) ;
	      ret = BLUE_FALSE ;
	    }
	}
    }
  else
    {
      dwLastError = BlueGetLastError() ;
      BlueCprintf ("Create of Directory Failed with Error %d\n",
		   dwLastError) ;
      ret = BLUE_FALSE ;
    }
  BlueHeapFree (dirname) ;
  return (ret) ;
}

/*
 * Rename file test
 */
static BLUE_BOOL BlueRenameTest (BLUE_CTCHAR *device)
{
  BLUE_HANDLE rename_file ;
  BLUE_DWORD dwLastError ;
  BLUE_CHAR *buffer ;
  BLUE_SIZET len ;
  BLUE_BOOL status ;
  BLUE_TCHAR *dirname ; 
  BLUE_HANDLE dirHandle ;
  BLUE_TCHAR *filename ; 
  BLUE_TCHAR *tofilename ;
  BLUE_TCHAR *fulltofilename ;
  BLUE_DWORD dwBytesWritten ;
  BLUE_BOOL ret ;
  BLUE_FILE_RENAME_INFO *rename_info ;
  BLUE_SIZET newlen ;
  BLUE_SIZET rename_info_len ;

  /*
   * Rename within a directory
   */
  ret = BLUE_TRUE ;
  dirname = MakeFilename (device, FS_TEST_DIRECTORY) ;
  status = BlueCreateDirectory (dirname, BLUE_NULL) ;
  if (status == BLUE_TRUE || BlueGetLastError () == BLUE_ERROR_FILE_EXISTS)
    {
      filename = MakeFilename (device, FS_TEST_RENAME) ;
      fulltofilename = MakeFilename (device, FS_TEST_RENAMETO) ;
      tofilename = BlueCtastrdup (FS_TEST_RENAMETO_ROOT) ;
      /*
       * First we need to create a file to rename
       * This also tests us creating and writing to a file without overlap
       * i/o
       */
      rename_file = BlueCreateFile (filename,
				    BLUE_GENERIC_WRITE,
				    BLUE_FILE_SHARE_READ,
				    BLUE_NULL,
				    BLUE_CREATE_ALWAYS,
				    BLUE_FILE_ATTRIBUTE_NORMAL,
				    BLUE_HANDLE_NULL) ;
        
      if (rename_file == BLUE_INVALID_HANDLE_VALUE)
	{
	  BlueCprintf ("Failed to create rename file %A, Error Code %d\n",
		       filename, BlueGetLastError ()) ;
	  ret = BLUE_FALSE ;
	}
      else
	{
	  /*
	   * Let's write some stuff to the file
	   */
	  buffer = BlueHeapMalloc (BUFFER_SIZE) ;
	  len = BUFFER_SIZE ;

	  while (len > 0)
	    {
	      len -= BlueCsnprintf (buffer + (BUFFER_SIZE-len), len,
				    "This Text begins at offset %d\n", 
				    BUFFER_SIZE - len) ;
	    }

	  status = BlueWriteFile (rename_file, buffer, BUFFER_SIZE, &dwBytesWritten,
				  BLUE_HANDLE_NULL) ;
    
	  BlueHeapFree (buffer) ;
	  if (status != BLUE_TRUE)
	    {
	      dwLastError = BlueGetLastError () ;
	      BlueCprintf ("Write to Rename File Failed with Error %d\n",
			   dwLastError) ;
	      ret = BLUE_FALSE ;
	    }
	  else
	    {
	      /* Close file
	       */
	      status = BlueCloseHandle (rename_file) ;
	      if (status != BLUE_TRUE)
		{
		  dwLastError = BlueGetLastError () ;
		  BlueCprintf ("Close of Rename File Failed with Error %d\n",
			       dwLastError) ;
		  ret = BLUE_FALSE ;
		}
	      else
		{
		  rename_file = BlueCreateFile (filename,
						BLUE_FILE_DELETE,
						BLUE_FILE_SHARE_DELETE,
						BLUE_NULL,
						BLUE_OPEN_EXISTING,
						0,
						BLUE_HANDLE_NULL) ;
        
		  if (rename_file == BLUE_INVALID_HANDLE_VALUE)
		    {
		      BlueCprintf ("Failed to create rename file %S, "
				   "Error Code %d\n",
				   filename,
				   BlueGetLastError ()) ;
		      ret = BLUE_FALSE ;
		    }
		  else
		    {
		      /*
		       * First delete the to file if it exists
		       */
		      BlueDeleteFile (fulltofilename) ;

		      newlen = BlueCtstrlen (tofilename) ;
		      rename_info_len = sizeof (BLUE_FILE_RENAME_INFO) +
			(newlen * sizeof (BLUE_WCHAR)) ;

		      rename_info = BlueHeapMalloc (rename_info_len) ;
		  
		      rename_info->ReplaceIfExists = BLUE_FALSE ;
		      rename_info->RootDirectory = BLUE_HANDLE_NULL ;
		      rename_info->FileNameLength = 
			(BLUE_DWORD) newlen * sizeof (BLUE_WCHAR) ;
		      BlueCtstrncpy (rename_info->FileName,
				     tofilename, newlen) ;
		      status = 
			BlueSetFileInformationByHandle(rename_file,
						       BlueFileRenameInfo,
						       rename_info,
						       (BLUE_DWORD) rename_info_len) ;
		      BlueHeapFree (rename_info) ;
		      if (status != BLUE_TRUE)
			{
			  dwLastError = BlueGetLastError () ;
			  BlueCprintf ("Set File Info on rename File "
				       "Failed with Error %d\n",
				       dwLastError) ;
			  ret = BLUE_FALSE ;
			}

		      status = BlueCloseHandle (rename_file) ;
		      if (status != BLUE_TRUE)
			{
			  dwLastError = BlueGetLastError () ;
			  BlueCprintf ("Close of rename File after Rename"
				       "Failed with Error %d\n",
				       dwLastError) ;
			  ret = BLUE_FALSE ;
			}
		      /*
		       * Now we can delete the file
		       */
		      status = BlueDeleteFile (fulltofilename) ;
		      if (status == BLUE_TRUE)
			BlueCprintf ("Rename Test Succeeded\n") ;
		      else
			{
			  dwLastError = BlueGetLastError () ;
			  BlueCprintf ("Delete Rename File Failed with Error %d\n",
				       dwLastError) ;
			  ret = BLUE_FALSE ;
			}
		    }
		}
	    }
	}
      BlueHeapFree (filename) ;
      BlueHeapFree (tofilename) ;
      BlueHeapFree (fulltofilename) ;

      dirHandle = BlueCreateFile (dirname,
				  BLUE_FILE_DELETE,
				  BLUE_FILE_SHARE_DELETE,
				  BLUE_NULL,
				  BLUE_OPEN_EXISTING,
				  BLUE_FILE_FLAG_DELETE_ON_CLOSE |
				  BLUE_FILE_ATTRIBUTE_DIRECTORY,
				  BLUE_HANDLE_NULL) ;
        
      if (dirHandle == BLUE_INVALID_HANDLE_VALUE)
	{
	  BlueCprintf ("Failed to create delete on close dir %A, "
		       "Error Code %d\n",
		       dirname,
		       BlueGetLastError ()) ;
	  ret = BLUE_FALSE ;
	}
      else
	{
	  /* Close file
	   */
	  status = BlueCloseHandle (dirHandle) ;
	  if (status != BLUE_TRUE)
	    {
	      dwLastError = BlueGetLastError () ;
	      BlueCprintf ("Close of Delete on close dir "
			   "Failed with Error %d\n",
			   dwLastError) ;
	      ret = BLUE_FALSE ;
	    }
	}
    }
  else
    {
      dwLastError = BlueGetLastError() ;
      BlueCprintf ("Create of Directory Failed with Error %d\n",
		   dwLastError) ;
      ret = BLUE_FALSE ;
    }
  BlueHeapFree (dirname) ;
  return (ret) ;
}

/*
 * Flush file test
 */
static BLUE_BOOL BlueFlushTest (BLUE_CTCHAR *device)
{
  BLUE_HANDLE flush_file ;
  BLUE_DWORD dwLastError ;
  BLUE_CHAR *buffer ;
  BLUE_SIZET len ;
  BLUE_BOOL status ;
  BLUE_INT i ;
  BLUE_TCHAR *filename ;
  BLUE_DWORD dwBytesWritten ;
  BLUE_BOOL ret ;

  ret = BLUE_TRUE ;
  filename = MakeFilename (device, FS_TEST_FLUSH) ;
  /*
   * First we need to create a file to flush
   * This also tests us creating and writing to a file without overlap
   * i/o
   */
  flush_file = BlueCreateFile (filename, 
			       BLUE_GENERIC_WRITE,
			       BLUE_FILE_SHARE_READ,
			       BLUE_NULL,
			       BLUE_CREATE_ALWAYS,
			       BLUE_FILE_ATTRIBUTE_NORMAL,
			       BLUE_HANDLE_NULL) ;
        
  if (flush_file == BLUE_INVALID_HANDLE_VALUE)
    {
      BlueCprintf ("Failed to create flush file %A, Error Code %d\n",
		   filename, BlueGetLastError ()) ;
      ret = BLUE_FALSE ;
    }
  else
    {
      /*
       * Let's write some stuff to the file
       */
      buffer = BlueHeapMalloc (BUFFER_SIZE) ;

      status = BLUE_TRUE ;
      for (i = 0 ; i < 10 && status == BLUE_TRUE ; i++)
	{
	  len = BlueCsnprintf (buffer, BUFFER_SIZE,
			       "This is the Text for line %d\n", i) ;

	  status = BlueWriteFile (flush_file, buffer, (BLUE_DWORD) len, &dwBytesWritten,
				  BLUE_HANDLE_NULL) ;
    
	  if (status != BLUE_TRUE)
	    {
	      dwLastError = BlueGetLastError () ;
	      BlueCprintf ("Write to Flush File Failed with Error %d\n",
			   dwLastError) ;
	      ret = BLUE_FALSE ;
	    }
	  else
	    {
	      /*
	       * Flush the file buffers
	       */
	      status = BlueFlushFileBuffers (flush_file) ;
	      if (status != BLUE_TRUE)
		{
		  dwLastError = BlueGetLastError() ;
		  BlueCprintf ("Flush to Flush File Failed with Error %d\n",
			       dwLastError) ;
		  ret = BLUE_FALSE ;
		}
	    }

	}

      BlueHeapFree (buffer) ;
      /* Close file
       */
      status = BlueCloseHandle (flush_file) ;
      if (status != BLUE_TRUE)
	{
	  dwLastError = BlueGetLastError () ;
	  BlueCprintf ("Close of Flush File Failed with Error %d\n",
		       dwLastError) ;
	  ret = BLUE_FALSE ;
	}
      else
	{
	  /*
	   * Now we can delete the file
	   */
	  status = BlueDeleteFile (filename) ;
	  if (status == BLUE_TRUE)
	    BlueCprintf ("Flush Test Succeeded\n") ;
	  else
	    {
	      dwLastError = BlueGetLastError () ;
	      BlueCprintf ("Delete of Flush File Failed with Error %d\n",
			   dwLastError) ;
	      ret = BLUE_FALSE ;
	    }
	}
    }
  BlueHeapFree (filename) ;
  return (ret) ;
}

/*
 * Create directory test
 */
static BLUE_BOOL BlueCreateDirectoryTest (BLUE_CTCHAR *device)
{
  BLUE_BOOL status ;
  BLUE_DWORD dwLastError ;
  BLUE_TCHAR *filename ;
  BLUE_BOOL ret ;

  ret = BLUE_TRUE ;
  filename = MakeFilename (device, FS_TEST_DIRECTORY) ;
  status = BlueCreateDirectory (filename, BLUE_NULL) ;
  if (status == BLUE_TRUE)
    BlueCprintf ("Create Directory Test Succeeded\n") ;
  else
    {
      dwLastError = BlueGetLastError() ;
      BlueCprintf ("Create of Directory Failed with Error %d\n",
		   dwLastError) ;
      ret = BLUE_FALSE ;
    }
  BlueHeapFree (filename) ;
  return (ret) ;
}

/*
 * Delete Directory Test
 */
static BLUE_BOOL BlueDeleteDirectoryTest (BLUE_CTCHAR *device)
{
  BLUE_BOOL status ;
  BLUE_DWORD dwLastError ;
  BLUE_TCHAR *filename ;
  BLUE_BOOL ret ;
  BLUE_HANDLE dirhandle ;

  ret = BLUE_TRUE ;
  filename = MakeFilename (device, FS_TEST_DIRECTORY) ;

  dirhandle = BlueCreateFile (filename,
			      BLUE_FILE_DELETE,
			      BLUE_FILE_SHARE_DELETE,
			      BLUE_NULL,
			      BLUE_OPEN_EXISTING,
			      BLUE_FILE_FLAG_DELETE_ON_CLOSE |
			      BLUE_FILE_ATTRIBUTE_DIRECTORY,
			      BLUE_HANDLE_NULL) ;
        
  if (dirhandle == BLUE_INVALID_HANDLE_VALUE)
    {
      BlueCprintf ("Failed to create delete on close dir %A, "
		   "Error Code %d\n",
		   filename,
		   BlueGetLastError ()) ;
      ret = BLUE_FALSE ;
    }
  else
    {
      /* Close file
       */
      status = BlueCloseHandle (dirhandle) ;
      if (status != BLUE_TRUE)
	{
	  dwLastError = BlueGetLastError () ;
	  BlueCprintf ("Close of Delete on close dir "
		       "Failed with Error %d\n",
		       dwLastError) ;
	  ret = BLUE_FALSE ;
	}
      else
	BlueCprintf ("Delete Directory Test Succeeded\n") ;
    }
  BlueHeapFree (filename) ;
  return (ret) ;
}

static BLUE_BOOL BlueSetEOFTest (BLUE_CTCHAR *device)
{
  BLUE_HANDLE seteof_file ;
  BLUE_DWORD dwLastError ;
  BLUE_CHAR *buffer ;
  BLUE_SIZET len ;
  BLUE_BOOL status ;
  BLUE_DWORD pos ;
  BLUE_TCHAR *filename ;
  BLUE_DWORD dwBytesWritten ;
  BLUE_BOOL ret ;

  ret = BLUE_TRUE ;
  filename = MakeFilename (device, FS_TEST_SETEOF) ;
  /*
   * First we need to create a file to play with
   * This also tests us creating and writing to a file without overlap
   * i/o
   */
  seteof_file = BlueCreateFile (filename,
				BLUE_GENERIC_WRITE,
				BLUE_FILE_SHARE_READ,
				BLUE_NULL,
				BLUE_CREATE_ALWAYS,
				BLUE_FILE_ATTRIBUTE_NORMAL,
				BLUE_HANDLE_NULL) ;
        
  if (seteof_file == BLUE_INVALID_HANDLE_VALUE)
    {
      BlueCprintf ("Failed to create seteof file %A, Error Code %d\n",
		   filename, BlueGetLastError ()) ;
      ret = BLUE_FALSE ;
    }
  else
    {
      /*
       * Let's write some stuff to the file
       */
      buffer = BlueHeapMalloc (BUFFER_SIZE) ;
      len = BUFFER_SIZE ;

      while (len > 0)
 	{
	  len -= BlueCsnprintf (buffer + (BUFFER_SIZE-len), len,
				"This Text begins at offset %d\n", 
				BUFFER_SIZE - len) ;
	}

      status = BlueWriteFile (seteof_file, buffer, BUFFER_SIZE, &dwBytesWritten,
			      BLUE_HANDLE_NULL) ;
    
      BlueHeapFree (buffer) ;
      if (status != BLUE_TRUE)
	{
	  dwLastError = BlueGetLastError () ;
	  BlueCprintf ("Write to SetEOF File Failed with Error %d\n",
		       dwLastError) ;
	  ret = BLUE_FALSE ;
	}
      else
	{
	  /* Close file
	   */
	  status = BlueCloseHandle (seteof_file) ;
	  if (status != BLUE_TRUE)
	    {
	      dwLastError = BlueGetLastError () ;
	      BlueCprintf ("Close of SetEOF File Failed with Error %d\n",
			   dwLastError) ;
	      ret = BLUE_FALSE ;
	    }
	  else
	    {
	      /*
	       * Now we want to open the file for read/write
	       */
	      seteof_file = BlueCreateFile (filename,
					    BLUE_GENERIC_WRITE |
					    BLUE_GENERIC_READ,
					    BLUE_FILE_SHARE_READ,
					    BLUE_NULL,
					    BLUE_OPEN_EXISTING,
					    BLUE_FILE_ATTRIBUTE_NORMAL,
					    BLUE_HANDLE_NULL) ;
        
	      if (seteof_file == BLUE_INVALID_HANDLE_VALUE)
		{
		  BlueCprintf ("Failed to open seteof file %A, "
			       "Error Code %d\n",
			       filename, BlueGetLastError ()) ;
		  ret = BLUE_FALSE ;
		}
	      else
		{
		  /*
		   * So now we want to set the file pointer to say, 2048
		   */
		  pos = BlueSetFilePointer (seteof_file,
					    BUFFER_SIZE / 2,
					    BLUE_NULL,
					    BLUE_FILE_BEGIN) ;
		  dwLastError = BlueGetLastError () ;
		  if (pos == BLUE_INVALID_SET_FILE_POINTER &&
		      dwLastError != BLUE_ERROR_SUCCESS)
		    {
		      BlueCprintf ("SetFilePosition Failed with Error %d\n",
				   dwLastError) ;
		      ret = BLUE_FALSE ;
		    }
		  else
		    {
		      /*
		       * Now we want to set eof
		       */
		      status = BlueSetEndOfFile (seteof_file) ;
		      if (status != BLUE_TRUE)
			{
			  dwLastError = BlueGetLastError () ;
			  BlueCprintf ("Set End Of File Failed with %d\n",
				       dwLastError) ;
			  ret = BLUE_FALSE ;
			}
#if 0
		      else
			{
			  BLUE_FILE_STANDARD_INFO standard_info ;

			  status = BlueGetFileInformationByHandleEx
			    (seteof_file,
			     BlueFileStandardInfo,
			     &standard_info,
			     sizeof (BLUE_FILE_STANDARD_INFO)) ;

			  if (status != BLUE_TRUE)
			    {
			      dwLastError = BlueGetLastError () ;
			      BlueCprintf ("Cannot get end of file position %d\n",
					   dwLastError) ;
			      ret = BLUE_FALSE ;
			    }
			  else
			    {
			      if (standard_info.EndOfFile != BUFFER_SIZE / 2)
				{
				  BlueCprintf 
				    ("End Of File Doesn't Match.  Expected %d, Got %d\n",
				     BUFFER_SIZE / 2,
				     (BLUE_INT) standard_info.EndOfFile) ;
				  ret = BLUE_FALSE ;
				}
			    }
			}
#endif
		    }
		  status = BlueCloseHandle (seteof_file) ;
		  if (status != BLUE_TRUE)
		    {
		      dwLastError = BlueGetLastError () ;
		      BlueCprintf ("Close of SetEOF File "
				   "after pos Failed with Error %d\n",
				   dwLastError) ;
		      ret = BLUE_FALSE ;
		    }
		}
	      /*
	       * Now we can delete the file
	       */
	      status = BlueDeleteFile (filename) ;
	      if (status == BLUE_TRUE)
		BlueCprintf ("SetEOF Test Succeeded\n") ;
	      else
		{
		  dwLastError = BlueGetLastError () ;
		  BlueCprintf ("SetEOF Delete File Failed with Error %d\n",
			       dwLastError) ;
		  ret = BLUE_FALSE ;
		}
	    }
	}
    }
  BlueHeapFree (filename) ;
  return (ret) ;
}

/*
 * Atrtribute test
 */
static BLUE_CHAR *Attr2Str[17] =
  {
    "RO",			/* Read Only */
    "HID",			/* Hidden */
    "SYS",			/* System */
    "",
    "DIR",			/* Directory */
    "ARV",			/* Archive */
    "",
    "NRM",			/* Normal */
    "TMP",			/* Temporary */
    "SPR",			/* Sparse */
    "",
    "CMP",			/* Compressed */
    "OFF",			/* Offline */
    "",
    "ENC",			/* Encrypted */
    ""
    "VIRT",			/* Virtual */
  } ;

static BLUE_VOID BlueFSPrintFindData (BLUE_WIN32_FIND_DATA *find_data)
{
  BLUE_INT i ;
  BLUE_UINT16 mask ;
  BLUE_CHAR str[100] ; 		/* Guaranteed big enough for all attributes */
  BLUE_CHAR *p ;
  BLUE_BOOL first ;
  BLUE_WORD fat_date ;
  BLUE_WORD fat_time ;
  BLUE_UINT16 month ;
  BLUE_UINT16 day ;
  BLUE_UINT16 year ;
  BLUE_UINT16 hour ;
  BLUE_UINT16 min ;
  BLUE_UINT16 sec ;

  BlueCprintf ("File: %A\n", find_data->cFileName) ;
  BlueCprintf ("Alternate Name: %.14A\n", find_data->cAlternateFileName) ;
  BlueCprintf ("Attributes: 0x%08x\n", find_data->dwFileAttributes) ;

  mask = 0x0001 ;
  str[0] = '\0' ;
  p = str ;
  first = BLUE_TRUE ;
  for (i = 0 ; i < 16 ; i++, mask <<= 1)
    {
      if (find_data->dwFileAttributes & mask)
	{
	  if (first)
	    first = BLUE_FALSE ;
	  else
	    {
	      BlueCstrcpy (p, ", ") ;
	      p += 2 ;
	    }
	  BlueCstrcpy (p, Attr2Str[i]) ;
	  p += BlueCstrlen (Attr2Str[i]) ;
	}
    }
  *p = '\0' ;
  
  BlueCprintf ("    %s\n", str) ;

  BlueFileTimeToDosDateTime (&find_data->ftCreateTime,
			     &fat_date, &fat_time) ;

  BlueTimeDosDateTimeToElements (fat_date, fat_time,
				 &month, &day, &year, &hour, &min, &sec) ;
  BlueCprintf ("Create Time: %02d/%02d/%04d %02d:%02d:%02d GMT\n",
	       month, day, year, hour, min, sec) ;
  BlueFileTimeToDosDateTime (&find_data->ftLastAccessTime,
			     &fat_date, &fat_time) ;

  BlueTimeDosDateTimeToElements (fat_date, fat_time,
				 &month, &day, &year, &hour, &min, &sec) ;
  BlueCprintf ("Last Access Time: %02d/%02d/%04d %02d:%02d:%02d GMT\n",
	       month, day, year, hour, min, sec) ;
  BlueFileTimeToDosDateTime (&find_data->ftLastWriteTime,
			     &fat_date, &fat_time) ;
  BlueTimeDosDateTimeToElements (fat_date, fat_time,
				 &month, &day, &year, &hour, &min, &sec) ;
  BlueCprintf ("Last Write Time: %02d/%02d/%04d %02d:%02d:%02d GMT\n",
	       month, day, year, hour, min, sec) ;

  BlueCprintf ("File Size High: 0x%08x, Low: 0x%08x\n",
	       find_data->nFileSizeHigh, find_data->nFileSizeLow) ;
  BlueCprintf ("\n") ;
}

static BLUE_VOID 
BlueFSPrintFileAttributeData (BLUE_WIN32_FILE_ATTRIBUTE_DATA *file_data)
{
  BLUE_INT i ;
  BLUE_UINT16 mask ;
  BLUE_CHAR str[100] ; 		/* Guaranteed big enough for all attributes */
  BLUE_CHAR *p ;
  BLUE_BOOL first ;
  BLUE_WORD fat_date ;
  BLUE_WORD fat_time ;
  BLUE_UINT16 month ;
  BLUE_UINT16 day ;
  BLUE_UINT16 year ;
  BLUE_UINT16 hour ;
  BLUE_UINT16 min ;
  BLUE_UINT16 sec ;

  BlueCprintf ("    Attributes: 0x%08x\n", file_data->dwFileAttributes) ;

  mask = 0x0001 ;
  str[0] = '\0' ;
  p = str ;
  first = BLUE_TRUE ;
  for (i = 0 ; i < 16 ; i++, mask <<= 1)
    {
      if (file_data->dwFileAttributes & mask)
	{
	  if (first)
	    first = BLUE_FALSE ;
	  else
	    {
	      BlueCstrcpy (p, ", ") ;
	      p += 2 ;
	    }
	  BlueCstrcpy (p, Attr2Str[i]) ;
	  p += BlueCstrlen (Attr2Str[i]) ;
	}
    }
  *p = '\0' ;
  
  BlueCprintf ("    %s\n", str) ;

  BlueFileTimeToDosDateTime (&file_data->ftCreateTime,
			     &fat_date, &fat_time) ;

  BlueTimeDosDateTimeToElements (fat_date, fat_time,
				 &month, &day, &year, &hour, &min, &sec) ;
  BlueCprintf ("Create Time: %02d/%02d/%04d %02d:%02d:%02d GMT\n",
	       month, day, year, hour, min, sec) ;
  BlueFileTimeToDosDateTime (&file_data->ftLastAccessTime,
			     &fat_date, &fat_time) ;

  BlueTimeDosDateTimeToElements (fat_date, fat_time,
				 &month, &day, &year, &hour, &min, &sec) ;
  BlueCprintf ("Last Access Time: %02d/%02d/%04d %02d:%02d:%02d GMT\n",
	       month, day, year, hour, min, sec) ;
  BlueFileTimeToDosDateTime (&file_data->ftLastWriteTime,
			     &fat_date, &fat_time) ;
  BlueTimeDosDateTimeToElements (fat_date, fat_time,
				 &month, &day, &year, &hour, &min, &sec) ;
  BlueCprintf ("Last Write Time: %02d/%02d/%04d %02d:%02d:%02d GMT\n",
	       month, day, year, hour, min, sec) ;

  BlueCprintf ("File Size High: 0x%08x, Low: 0x%08x\n",
	       file_data->nFileSizeHigh, file_data->nFileSizeLow) ;
}

static BLUE_BOOL BlueListDirTest (BLUE_CTCHAR *device)
{
  BLUE_HANDLE list_handle ;
  BLUE_WIN32_FIND_DATA find_data ;
  BLUE_BOOL more = BLUE_FALSE ;
  BLUE_BOOL status ;
  BLUE_TCHAR *filename ;
  BLUE_DWORD last_error ;
  BLUE_BOOL retry ;
  BLUE_CHAR username[80] ;
  BLUE_CHAR password[80] ;
  BLUE_CHAR domain[80] ;
  BLUE_TCHAR *tusername ;
  BLUE_TCHAR *tpassword ;
  BLUE_TCHAR *tdomain ;
  BLUE_BOOL ret ;
  BLUE_INT count ;

  ret = BLUE_TRUE ;
  retry = BLUE_TRUE ;
  list_handle = BLUE_INVALID_HANDLE_VALUE ;
  /*
   * The initial credentials in device are anonymous.  If that fails,
   * prompt for credentials and try again.
   */
  
  count = 0 ;
  while (retry == BLUE_TRUE)
    {
      filename = MakeFilename (device, TSTR("*")) ;

      list_handle = BlueFindFirstFile (filename, &find_data, &more) ;
        
      if (list_handle != BLUE_INVALID_HANDLE_VALUE)
	retry = BLUE_FALSE ;
      else
	{
	  last_error = BlueGetLastError () ;
	  BlueCprintf ("Failed to list dir %A, Error Code %d\n",
		       filename, last_error) ;
	  /*
	   * LOGON_FAILURE can be returned if authentication failed
	   * ACCESS_DENIED if authentication succeeded but no access to share
	   * INVALID_PASSWORD if password different then connection
	   */
	  if (last_error == BLUE_ERROR_LOGON_FAILURE ||
	      last_error == BLUE_ERROR_ACCESS_DENIED ||
	      last_error == BLUE_ERROR_INVALID_PASSWORD)
	    {
	      BlueCprintf ("Please Authenticate\n") ;
	      BlueCprintf ("Username: ") ;
	      BlueReadLine (username, 80) ;
	      if (BlueCstrlen (username) == 0)
		retry = BLUE_FALSE ;
	      else
		{
		  tusername = BlueCcstr2tastr (username) ;
		  BlueCprintf ("Domain: ") ;
		  BlueReadLine (domain, 80) ;
		  tdomain = BlueCcstr2tastr (domain) ;
		  BlueCprintf ("Password: ") ;
		  BlueReadPassword (password, 80) ;
		  tpassword = BlueCcstr2tastr (password) ;
		  /*
		   * Now remap the device
		   */
		  BluePathUpdateCredentials (filename, tusername,
					     tpassword, tdomain) ;
		  BlueHeapFree (tusername) ;
		  BlueHeapFree (tpassword) ;
		  BlueHeapFree (tdomain) ;
		}
	    }
	  else
	    {
	      retry = BLUE_FALSE ;
	      ret = BLUE_FALSE ;
	    }
	}
      BlueHeapFree (filename) ;
    }

  if (list_handle != BLUE_INVALID_HANDLE_VALUE)
    {
      count++ ;
      BlueFSPrintFindData (&find_data) ;

      status = BLUE_TRUE ;
      while (more && status == BLUE_TRUE )
	{
	  status = BlueFindNextFile (list_handle,
				     &find_data,
				     &more) ;
	  if (status == BLUE_TRUE)
	    {
	      count++ ;
	      BlueFSPrintFindData (&find_data) ;
	    }
	  else
	    {
	      last_error = BlueGetLastError () ;
	      if (last_error != BLUE_ERROR_NO_MORE_FILES)
		{
		  BlueCprintf ("Failed to Find Next, Error Code %d\n",
			       last_error) ;
		  ret = BLUE_FALSE ;
		}
	    }
	}
      BlueFindClose (list_handle) ;
    }
  BlueCprintf ("Total Number of Files in Directory %d\n", count) ;
  return (ret) ;
}

static BLUE_BOOL BlueGetFileAttributesTest (BLUE_CTCHAR *device)
{
  BLUE_HANDLE getex_file ;
  BLUE_DWORD dwLastError ;
  BLUE_CHAR *buffer ;
  BLUE_SIZET len ;
  BLUE_BOOL status ;
  BLUE_WIN32_FILE_ATTRIBUTE_DATA fInfo ;
  BLUE_TCHAR *filename ;
  BLUE_DWORD dwBytesWritten ;
  BLUE_BOOL ret ;

  ret = BLUE_TRUE ;
  filename = MakeFilename (device, FS_TEST_GETEX) ;
  /*
   * First we need to create a file to play with
   */
  getex_file = BlueCreateFile (filename,
			       BLUE_GENERIC_WRITE,
			       BLUE_FILE_SHARE_READ,
			       BLUE_NULL,
			       BLUE_CREATE_ALWAYS,
			       BLUE_FILE_ATTRIBUTE_NORMAL,
			       BLUE_HANDLE_NULL) ;
        
  if (getex_file == BLUE_INVALID_HANDLE_VALUE)
    {
      BlueCprintf ("Failed to create getex file %A, Error Code %d\n",
		   filename, BlueGetLastError ()) ;
      ret = BLUE_FALSE ;
    }
  else
    {
      /*
       * Let's write some stuff to the file
       */
      buffer = BlueHeapMalloc (BUFFER_SIZE) ;
      len = BUFFER_SIZE ;

      while (len > 0)
 	{
	  len -= BlueCsnprintf (buffer + (BUFFER_SIZE-len), len,
				"This Text begins at offset %d\n", 
				BUFFER_SIZE - len) ;
	}

      status = BlueWriteFile (getex_file, buffer, BUFFER_SIZE, &dwBytesWritten,
			      BLUE_HANDLE_NULL) ;
    
      BlueHeapFree (buffer) ;
      if (status != BLUE_TRUE)
	{
	  dwLastError = BlueGetLastError () ;
	  BlueCprintf ("Write to GetEX File Failed with Error %d\n",
		       dwLastError) ;
	  ret = BLUE_FALSE ;
	}
      else
	{
	  /* Close file
	   */
	  status = BlueCloseHandle (getex_file) ;
	  if (status != BLUE_TRUE)
	    {
	      dwLastError = BlueGetLastError () ;
	      BlueCprintf ("Close of GetEX File Failed with Error %d\n",
			   dwLastError) ;
	      ret = BLUE_FALSE ;
	    }
	  else
	    {
	      status = BlueGetFileAttributesEx (filename,
						BlueGetFileExInfoStandard,
						&fInfo) ;
	      if (status != BLUE_TRUE)
		{
		  dwLastError = BlueGetLastError () ;
		  BlueCprintf ("GetEx Get File Info Failed with Error %d\n",
			       dwLastError) ;
		  ret = BLUE_FALSE ;
		}
	      else
		{
		  BlueCprintf ("Get File Info for %A\n", filename) ;
		  BlueFSPrintFileAttributeData (&fInfo) ;
		}
	      /*
	       * Now we can delete the file
	       */
	      status = BlueDeleteFile (filename) ;
	      if (status != BLUE_TRUE)
		{
		  dwLastError = BlueGetLastError () ;
		  BlueCprintf ("GetEx Delete File Failed with Error %d\n",
			       dwLastError) ;
		  ret = BLUE_FALSE ;
		}
	    }
	}
    }
  BlueHeapFree (filename) ;
  return (ret) ;
}

static BLUE_BOOL BlueGetDiskFreeSpaceTest (BLUE_CTCHAR *device)
{
  BLUE_DWORD SectorsPerCluster ;
  BLUE_DWORD BytesPerSector ;
  BLUE_DWORD NumberOfFreeClusters ;
  BLUE_DWORD TotalNumberOfClusters ;
  BLUE_BOOL result ;
  BLUE_TCHAR *filename ;
  BLUE_BOOL ret ;

  ret = BLUE_TRUE ;
  filename = MakeFilename (device, TSTR(":")) ;

  result = BlueGetDiskFreeSpace (filename, &SectorsPerCluster,
				 &BytesPerSector, &NumberOfFreeClusters,
				 &TotalNumberOfClusters) ;
  if (result == BLUE_FALSE)
    {
      BlueCprintf ("Failed to get disk free space on %A, Error Code %d\n",
		   filename, BlueGetLastError ()) ;
      /*
       * Disk Free Space and Volume Information only works on Shares.
       * (ie. there is no file name).  No, reverse connections are tricky.
       * The way we build names for a reverse connection uses the server
       * as the gateway and the share as the server.  It's not until the
       * first directory that we actually specify the share.  Unfortunately,
       * the SMB protocol only accepts the share id in the request.  That
       * would imply the server.  So getting free space or volume info on
       * a server just doesn't make sense.  In order to support this through
       * a proxy, there needs to be a one to one correspondence between
       * client share, proxy share, and server share.  This makes proxying
       * pretty useless since we could not proxy multiple servers through the
       * same proxy.  I think this is the only limitation.  
       */
      BlueCprintf ("Not supported on reverse connections, "
		   "see code comment\n") ;
      ret = BLUE_TRUE ;
    }
  else
    {
      BlueCprintf ("Free Space On %A\n", filename) ; 
      BlueCprintf ("  Sectors Per Cluster: %d\n", SectorsPerCluster) ;
      BlueCprintf ("  Bytes Per Sector: %d\n", BytesPerSector) ;
      BlueCprintf ("  Number of Free Clusters: %d\n", NumberOfFreeClusters) ;
      BlueCprintf ("  Total Number of Clusters: %d\n", TotalNumberOfClusters) ;
    }
  BlueHeapFree (filename) ;
  return (ret) ;
}

static BLUE_BOOL BlueGetVolumeInformationTest (BLUE_CTCHAR *device)
{
  BLUE_TCHAR VolumeNameBuffer[BLUE_MAX_PATH+1] ;
  BLUE_DWORD VolumeSerialNumber ;
  BLUE_DWORD MaximumComponentLength ;
  BLUE_DWORD FileSystemFlags ;
  BLUE_TCHAR FileSystemName[BLUE_MAX_PATH+1] ;
  BLUE_BOOL result ;
  BLUE_TCHAR *filename ;
  BLUE_BOOL ret ;
  BLUE_TCHAR *root ;

  ret = BLUE_TRUE ;
  filename = MakeFilename (device, TSTR(":")) ;
  BluePathGetRoot (filename, &root, BLUE_NULL) ;

  result = BlueGetVolumeInformation (root,
				     VolumeNameBuffer,
				     BLUE_MAX_PATH+1,
				     &VolumeSerialNumber,
				     &MaximumComponentLength,
				     &FileSystemFlags,
				     FileSystemName,
				     BLUE_MAX_PATH+1) ;
  if (result == BLUE_FALSE)
    {
      BlueCprintf ("Failed to get volume info on %A, Error Code %d\n",
		   filename, BlueGetLastError ()) ;
      /*
       * Disk Free Space and Volume Information only works on Shares.
       * (ie. there is no file name).  No, reverse connections are tricky.
       * The way we build names for a reverse connection uses the server
       * as the gateway and the share as the server.  It's not until the
       * first directory that we actually specify the share.  Unfortunately,
       * the SMB protocol only accepts the share id in the request.  That
       * would imply the server.  So getting free space or volume info on
       * a server just doesn't make sense.  In order to support this through
       * a proxy, there needs to be a one to one correspondence between
       * client share, proxy share, and server share.  This makes proxying
       * pretty useless since we could not proxy multiple servers through the
       * same proxy.  I think this is the only limitation.  
       */
      BlueCprintf ("Not supported on reverse connections, "
		   "see code comment\n") ;
      ret = BLUE_TRUE ;
    }
  else
    {
      BlueCprintf ("Volume Info for %A\n", filename) ;
      BlueCprintf ("  Volume Name: %A\n", VolumeNameBuffer) ;
      BlueCprintf ("  Volume Serial Number: 0x%08x\n", VolumeSerialNumber) ;
      BlueCprintf ("  Max Component Length: %d\n", MaximumComponentLength) ;
      BlueCprintf ("  File System Flags: 0x%08x\n", FileSystemFlags) ;
      BlueCprintf ("  File System Name: %A\n", FileSystemName) ;
    }
  BlueHeapFree (filename) ;
  BlueHeapFree (root) ;
  return (ret) ;
}

struct _test_path
{
  BLUE_CHAR *description ;
  BLUE_CHAR *username ;
  BLUE_CHAR *password ;
  BLUE_CHAR *domain ;
  BLUE_CHAR *smbserver ;
  BLUE_CHAR *sharefolder ;
  BLUE_CHAR *subdirectory ;
  BLUE_UINT32 expected_error1 ;
  BLUE_UINT32 expected_error2 ;
} ;

static struct _test_path test_paths[] =
  { 
    /* Test 0, Good case */
    {
      "Valid Path",
      "rschmitt", "holy0grail", "workgroup", 
      "tester", "blueshare", "mydir",
      BLUE_ERROR_SUCCESS, BLUE_ERROR_SUCCESS
    }, 
    /* Test 1, SMB server is wrong */
    {
      "Non-existent SMB Server",
      "rschmitt", "holy0grail", "workgroup", 
      "bogus", "blueshare", "mydir",
      BLUE_ERROR_BAD_NET_NAME, BLUE_ERROR_BAD_NET_NAME
    }, 
    /* Test 2, Share folder is wrong */
    {
      "Non-existent SMB Share",
      "rschmitt", "holy0grail", "workgroup", 
      "tester", "bogus", "mydir",
      BLUE_ERROR_BAD_NETPATH, BLUE_ERROR_BAD_NETPATH
    }, 
    /* Test 3, Password is wrong */
    {
      "Bad Passwowrd",
      "rschmitt", "bogus", "workgroup", 
      "tester", "blueshare", "mydir",
      BLUE_ERROR_LOGON_FAILURE, BLUE_ERROR_INVALID_PASSWORD
    }, 
    /* Test 4, Subdirectory is wrong */
    {
      "Non-existent Subdirectory",
      "rschmitt", "holy0grail", "workgroup", 
      "tester", "blueshare", "bogus",
      BLUE_ERROR_FILE_NOT_FOUND, BLUE_ERROR_PATH_NOT_FOUND
    }, 
    /* Test 5, Share folder doesn't have write permission */
    {
      "Read-Only Share",
      "rschmitt", "holy0grail", "workgroup", 
      "tester", "roshare", "mydir",
      BLUE_ERROR_ACCESS_DENIED, BLUE_ERROR_PATH_NOT_FOUND
    }, 
    /* Test 6, Subdirectory doesn't have write permission */
    {
      "Read-Only Subdirectory",
      "rschmitt", "holy0grail", "workgroup", 
      "tester", "blueshare", "rodir",
      BLUE_ERROR_ACCESS_DENIED, BLUE_ERROR_ACCESS_DENIED
    },
    /* end of list */
    {
      BLUE_NULL,
      BLUE_NULL, BLUE_NULL, BLUE_NULL,
      BLUE_NULL, BLUE_NULL, BLUE_NULL,
      BLUE_ERROR_SUCCESS, BLUE_ERROR_SUCCESS
    }
  } ;

static BLUE_BOOL BlueBadFileNamesTest (BLUE_CTCHAR *device)
{
  BLUE_BOOL ret ;
  BLUE_BOOL test_status ;
  BLUE_HANDLE hFile ;

  char uncfilename[BLUE_MAX_PATH] ;
  BLUE_SIZET filelen ;
  char *tmpfilename ;
  BLUE_UINT32 lasterror ;
 
  BLUE_INT i ;

  ret = BLUE_TRUE ;

  for (i = 0 ; test_paths[i].description != BLUE_NULL ; i++)
    {
      BlueCprintf ("\n") ;
      BlueCprintf ("Test %d, %s\n", i, test_paths[i].description) ;
      /*
       * method A, Opening Directory 
       */
      filelen = BLUE_MAX_PATH ;
      tmpfilename = &uncfilename[0];
      BluePathMakeURLA(&tmpfilename, &filelen, 
		       test_paths[i].username, 
		       test_paths[i].password, 
		       test_paths[i].domain,
		       test_paths[i].smbserver,
		       test_paths[i].sharefolder,
		       test_paths[i].subdirectory,
		       BLUE_NULL) ;

      BlueCprintf ("Writeable Directory Mode\n") ;
      BlueCprintf ("Testing Write Access to %s\n", uncfilename) ;

      hFile = BlueCreateFileA (uncfilename,
			       BLUE_GENERIC_WRITE,
			       BLUE_FILE_SHARE_WRITE,
			       BLUE_NULL,
			       BLUE_OPEN_EXISTING,
			       BLUE_FILE_ATTRIBUTE_DIRECTORY,
			       BLUE_HANDLE_NULL) ;

      test_status = BLUE_TRUE ;
      if (hFile == BLUE_INVALID_HANDLE_VALUE)
	{
	  lasterror = BlueGetLastError() ;
	}
      else
	{
	  BlueCloseHandle (hFile) ;
	  lasterror = BLUE_ERROR_SUCCESS ;
	}

      if (test_paths[i].expected_error1 != lasterror &&
	  test_paths[i].expected_error2 != lasterror)
	{
	  test_status = BLUE_FALSE ;
	  ret = BLUE_FALSE ;
	}
	  
      BlueCprintf ("%s: last error %d, expected error %d or %d\n",
		   test_status == BLUE_TRUE ? "SUCCESS" : "FAILURE",
		   lasterror, test_paths[i].expected_error1,
		   test_paths[i].expected_error2) ;
      BlueDismountA (uncfilename);
      /*
       * method B, Temporary File
       */
      filelen = BLUE_MAX_PATH ;
      tmpfilename = &uncfilename[0];
      BluePathMakeURLA(&tmpfilename, &filelen, 
		       test_paths[i].username, 
		       test_paths[i].password, 
		       test_paths[i].domain,
		       test_paths[i].smbserver,
		       test_paths[i].sharefolder,
		       test_paths[i].subdirectory,
		       "uniquefile") ;
      BlueCprintf ("Writeable Temporary File Mode\n") ;
      BlueCprintf ("Testing Write Access to %s\n", uncfilename) ;

      hFile = BlueCreateFileA (uncfilename,
			       BLUE_GENERIC_READ | BLUE_GENERIC_WRITE |
			       BLUE_FILE_DELETE,
			       BLUE_FILE_SHARE_NONE,
			       BLUE_NULL,
			       BLUE_CREATE_NEW,
			       BLUE_FILE_ATTRIBUTE_TEMPORARY |
			       BLUE_FILE_FLAG_DELETE_ON_CLOSE,
			       BLUE_HANDLE_NULL) ;
      test_status = BLUE_TRUE ;
      if (hFile == BLUE_INVALID_HANDLE_VALUE)
	{
	  lasterror = BlueGetLastError() ;
	}
      else
	{
	  BlueCloseHandle (hFile) ;
	  lasterror = BLUE_ERROR_SUCCESS ;
	}

      if (test_paths[i].expected_error1 != lasterror &&
	  test_paths[i].expected_error2 != lasterror)
	{
	  test_status = BLUE_FALSE ;
	  ret = BLUE_FALSE ;
	}
	  
      BlueCprintf ("%s: last error %d, expected error %d or %d\n",
		   test_status == BLUE_TRUE ? "SUCCESS" : "FAILURE",
		   lasterror, test_paths[i].expected_error1,
		   test_paths[i].expected_error2) ;
      BlueDismountA (uncfilename);
    }
  return (ret) ;
}

/*
 * Startup Entry point to the test
 * 
 * \param hScheduler
 * The scheduler that this test should create apps within (if any)
 * In our case, the file system test does not create any apps.
 */
BLUE_INT test_file (BLUE_LPCSTR test_root)
{
  BLUE_DWORD ret;
  BLUE_TCHAR *device ;
  BLUE_BOOL test_result ;
  BLUE_INT count ;

  /*
   * Map the test device to the test path.  Determine what
   * File System Handler we should test.
   */
  BlueCprintf ("Starting File Test with %s\n", test_root);

  BlueSleep (3000) ;
  BlueThreadCreateLocalStorage() ;

  count = 0;

  device = BlueCcstr2tstr(test_root);

  while (count != OFC_FILE_TEST_COUNT)
    {
      test_result = BLUE_TRUE ;

      /*
       * First, create a random file
       */
      BlueCprintf ("  Create File Test\n") ;
      if (BlueCreateFileTest (device) == BLUE_FALSE)
	{
	  BlueCprintf ("  *** Create File Test Failed *** \n") ;
	  test_result = BLUE_FALSE ;
	}
      BlueCprintf ("  Dismount Test\n") ;
      if (BlueDismountTest (device) == BLUE_FALSE)
	{
	  BlueCprintf ("  *** Dismount Test Failed ***\n") ;
	  test_result = BLUE_FALSE ;
	}
      /* This may or may not fail.  It is posible for the previous test
       * to complete (dismount test) before the connection has been
       * cleaned up.  So, if the connection is up, the API will succeed.
       * if the connection has already gone down, then the API will
       * fail.
       * Either case is good 
       */
      BlueCprintf ("  Dismount Test with No connection\n") ;
      BlueDismountTest (device) ;
      /*
       * Then see if we can copy files
       */
      BlueCprintf ("  Copy File Test\n") ;
      if (BlueCopyFileTest (device) == BLUE_FALSE)
	{
	  BlueCprintf ("  *** Copy File Test Failed *** \n") ;
	  test_result = BLUE_FALSE ;
	}
      BlueCprintf ("  List Directory Test\n") ;
      if (BlueListDirTest (device) == BLUE_FALSE)
	{
	  BlueCprintf ("  *** List Directory Test Failed *** \n") ;
	  test_result = BLUE_FALSE ;
	}
      BlueCprintf ("  Delete File Test\n") ;
      if (BlueDeleteTest (device) == BLUE_FALSE)
	{
	  BlueCprintf ("  *** Delete File Test Failed ***\n") ;
	  test_result = BLUE_FALSE ;
	}
      /* This one only works in SMBv2, we can update SMBv1 to support
       *  this style 
       * Since the API is only supported on SMBv2, we should use the
       * move API instead.  So, don't enable this approach yet.
       */
#if 0
      BlueCprintf ("  Rename File Test\n") ;
      if (BlueRenameTest (device) == BLUE_FALSE)
	{
	  BlueCprintf ("  *** Rename File Test Failed ***\n") ;
	  BlueCprintf ("  Failures expected on SMBv1 Systems\n") ;
	  test_result = BLUE_FALSE ;
	}
#endif
      BlueCprintf ("  Move File Test\n") ;
      if (BlueMoveTest (device) == BLUE_FALSE)
	{
	  BlueCprintf ("  *** Move File Test Failed ***\n") ;
	  test_result = BLUE_FALSE ;
	}
      BlueCprintf ("  Flush File Test\n") ;
      if (BlueFlushTest (device) == BLUE_FALSE)
	{
	  BlueCprintf ("  *** Flush File Test Failed ***\n") ;
	  test_result = BLUE_FALSE ;
	}
      BlueCprintf ("  Create Directory Test\n") ;
      if (BlueCreateDirectoryTest(device) == BLUE_FALSE)
	{
	  BlueCprintf ("  *** Create Directory Test Failed ***\n") ;
	  test_result = BLUE_FALSE ;
	}
      BlueCprintf ("  Delete Directory Test\n") ;
      if (BlueDeleteDirectoryTest(device) == BLUE_FALSE)
	{
	  BlueCprintf ("  *** Delete Directory Test Failed ***\n") ;
	  test_result = BLUE_FALSE ;
	}
      BlueCprintf ("  Set File Pointer and Set EOF Test\n") ;
      if (BlueSetEOFTest (device) == BLUE_FALSE)
	{
	  BlueCprintf ("  *** Set EOF Test Failed ***\n") ;
	  test_result = BLUE_FALSE ;
	}
      BlueCprintf ("  Get File Attributes Test\n") ;
      if (BlueGetFileAttributesTest(device) == BLUE_FALSE)
	{
	  BlueCprintf (" *** Get File Attributes Test Failed ***\n") ;
	  test_result = BLUE_FALSE ;
	}
      BlueCprintf ("  Get Disk Free Space Test\n") ;
      if (BlueGetDiskFreeSpaceTest (device) == BLUE_FALSE)
	{
	  BlueCprintf ("  *** Get Disk Free Space Failed ***\n") ;
	  test_result = BLUE_FALSE ;
	}
      BlueCprintf ("  Get Volume Information Test\n") ;
      if (BlueGetVolumeInformationTest (device) == BLUE_FALSE)
	{
	  BlueCprintf ("  *** Get Volume Information Failed ***\n") ;
	  test_result = BLUE_FALSE ;
	}

      if (test_result == BLUE_FALSE)
	BlueCprintf ("*** File Test Failed ***\n") ;
      else
	BlueCprintf ("File Test Succeeded\n") ;
      count++ ;

      if (count != OFC_FILE_TEST_COUNT)
	BlueSleep (BLUE_FS_TEST_INTERVAL) ;
    }

  BlueHeapFree (device);
  return (0) ;
}

