/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
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

#define OFC_FS_TEST_INTERVAL 1000
#define OFC_FILE_TEST_COUNT 2

/*
 * This application demonstrates the API to the Open File I/O Facility
 *
 * The Open File I/O Facility is the primary interface fo the
 * Open Files SMB Client.  Once the SMB client is configured, network
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
static OFC_DWORD OfcFSTestApp(OFC_PATH *path);

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
#define NUM_FILE_BUFFERS 4
/*
 * Define buffer states.
 */
typedef enum {
    BUFFER_STATE_IDLE,        /* There is no I/O active */
    BUFFER_STATE_READ,        /* Data is being read into the buffer */
    BUFFER_STATE_WRITE        /* Data is being written from the buffer */
} BUFFER_STATE;
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
typedef struct {
    OFC_HANDLE readOverlapped;    /* The handle to the buffer when reading */
    OFC_HANDLE writeOverlapped;    /* The handle to the buffer when writing */
    OFC_CHAR *data;        /* Pointer to the buffer */
    BUFFER_STATE state;        /* Buffer state */
    OFC_LARGE_INTEGER offset;    /* Offset in file for I/O */
} OFC_FILE_BUFFER;
/*
 * Result of Async I/O
 *
 * This essentially is a OFC_BOOL with the addition of a PENDING flag
 */
typedef enum {
    ASYNC_RESULT_DONE,        /* I/O is successful */
    ASYNC_RESULT_ERROR,        /* I/O was in error */
    ASYNC_RESULT_EOF,        /* I/O hit EOF */
    ASYNC_RESULT_PENDING    /* I/O is still pending */
} ASYNC_RESULT;

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
 * OFC_TRUE if success, OFC_FALSE otherwise
 */
static ASYNC_RESULT
AsyncRead(OFC_HANDLE wait_set, OFC_HANDLE read_file,
          OFC_FILE_BUFFER *buffer, OFC_DWORD dwLen) {
    ASYNC_RESULT result;
    OFC_BOOL status;

    /*
   * initialize the read buffer using the read file, the read overlapped
   * handle and the current read offset
   */
    ofc_trace ("Reading 0x%08x\n", OFC_LARGE_INTEGER_LOW(buffer->offset));
    OfcSetOverlappedOffset(read_file, buffer->readOverlapped, buffer->offset);
    /*
   * Set the state to reading
   */
    buffer->state = BUFFER_STATE_READ;
    /*
   * Add the buffer to the wait set
   */
    ofc_waitset_add(wait_set, (OFC_HANDLE) buffer, buffer->readOverlapped);
    /*
   * Issue the read (this will be non blocking)
   */
    status = OfcReadFile(read_file, buffer->data, dwLen,
                         OFC_NULL, buffer->readOverlapped);
    /*
   * If it completed, the status will be OFC_TRUE.  We actually expect
   * the status to fail and the last error to be OFC_ERROR_IO_PENDING
   */
    if (status == OFC_TRUE)
        result = ASYNC_RESULT_DONE;
    else {
        OFC_DWORD dwLastError;
        /*
       * Let's check the last error
       */
        dwLastError = OfcGetLastError();
        if (dwLastError == OFC_ERROR_IO_PENDING) {
            /*
	   * This is what we expect, so say the I/O submission succeeded
	   */
            result = ASYNC_RESULT_PENDING;
        } else {
            if (dwLastError == OFC_ERROR_HANDLE_EOF)
                result = ASYNC_RESULT_EOF;
            else
                result = ASYNC_RESULT_ERROR;
            /*
	   * It's not pending
	   */
            buffer->state = BUFFER_STATE_IDLE;
            ofc_waitset_remove(wait_set, buffer->readOverlapped);
        }
    }

    return (result);
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
static ASYNC_RESULT AsyncReadResult(OFC_HANDLE wait_set,
                                    OFC_HANDLE read_file,
                                    OFC_FILE_BUFFER *buffer,
                                    OFC_DWORD *dwLen) {
    ASYNC_RESULT result;
    OFC_BOOL status;

    /*
   * Get the overlapped result
   */
    status = OfcGetOverlappedResult(read_file, buffer->readOverlapped,
                                    dwLen, OFC_FALSE);
    /*
   * If the I/O is complete, status will be true and length will be non zero
   */
    if (status == OFC_TRUE) {
        if (*dwLen == 0) {
            result = ASYNC_RESULT_EOF;
        } else {
            result = ASYNC_RESULT_DONE;
        }
    } else {
        OFC_DWORD dwLastError;
        /*
       * I/O may still be pending
       */
        dwLastError = OfcGetLastError();
        if (dwLastError == OFC_ERROR_IO_PENDING)
            result = ASYNC_RESULT_PENDING;
        else {
            /*
	   * I/O may also be EOF
	   */
            if (dwLastError != OFC_ERROR_HANDLE_EOF) {
                ofc_printf("Read Error %d\n", dwLastError);
                result = ASYNC_RESULT_ERROR;
            } else
                result = ASYNC_RESULT_EOF;
        }
    }

    if (result != ASYNC_RESULT_PENDING) {
        /*
       * Finish up the buffer if the I/O is no longer pending
       */
        buffer->state = BUFFER_STATE_IDLE;
        ofc_waitset_remove(wait_set, buffer->readOverlapped);
    }

    return (result);
}

/*
 * Submit an asynchronous Write
 */
static OFC_BOOL AsyncWrite(OFC_HANDLE wait_set, OFC_HANDLE write_file,
                           OFC_FILE_BUFFER *buffer, OFC_DWORD dwLen) {
    OFC_BOOL status;

    ofc_trace ("Writing 0x%08x\n", OFC_LARGE_INTEGER_LOW(buffer->offset));
    OfcSetOverlappedOffset(write_file, buffer->writeOverlapped,
                           buffer->offset);

    buffer->state = BUFFER_STATE_WRITE;
    ofc_waitset_add(wait_set, (OFC_HANDLE) buffer, buffer->writeOverlapped);

    status = OfcWriteFile(write_file, buffer->data, dwLen, OFC_NULL,
                          buffer->writeOverlapped);

    if (status != OFC_TRUE) {
        OFC_DWORD dwLastError;

        dwLastError = OfcGetLastError();
        if (dwLastError == OFC_ERROR_IO_PENDING)
            status = OFC_TRUE;
        else {
            buffer->state = BUFFER_STATE_IDLE;
            ofc_waitset_remove(wait_set, buffer->writeOverlapped);
        }
    }
    return (status);
}

static ASYNC_RESULT AsyncWriteResult(OFC_HANDLE wait_set,
                                     OFC_HANDLE write_file,
                                     OFC_FILE_BUFFER *buffer,
                                     OFC_DWORD *dwLen) {
    ASYNC_RESULT result;
    OFC_BOOL status;

    status = OfcGetOverlappedResult(write_file, buffer->writeOverlapped,
                                    dwLen, OFC_FALSE);
    if (status == OFC_TRUE)
        result = ASYNC_RESULT_DONE;
    else {
        OFC_DWORD dwLastError;

        dwLastError = OfcGetLastError();
        if (dwLastError == OFC_ERROR_IO_PENDING)
            result = ASYNC_RESULT_PENDING;
        else {
            ofc_printf("Write Error %d\n", dwLastError);
            result = ASYNC_RESULT_ERROR;
        }
    }

    if (result != ASYNC_RESULT_PENDING) {
        buffer->state = BUFFER_STATE_IDLE;
        ofc_waitset_remove(wait_set, buffer->writeOverlapped);
    }

    return (result);
}

static OFC_TCHAR *MakeFilename(OFC_CTCHAR *device, OFC_CTCHAR *name) {
    OFC_SIZET devlen;
    OFC_SIZET namelen;
    OFC_TCHAR *filename;

    devlen = ofc_tastrlen (device);
    namelen = ofc_tastrlen (name);
    filename =
            ofc_malloc((devlen + namelen + 2) * sizeof(OFC_TCHAR));
    ofc_tastrcpy (filename, device);
    filename[devlen] = TCHAR('/');
    ofc_tastrcpy (&filename[devlen+1], name);
    filename[devlen + 1 + namelen] = TCHAR_EOS;
    return (filename);
}

/*
 * Create File Test.  This routine will create a file using a blocking
 * (sequential) algorithm
 */

/* 2 MB */
//#define CREATE_SIZE (2*1024*1024)
#define CREATE_SIZE (2*1024)

static OFC_BOOL OfcCreateFileTest(OFC_CTCHAR *device) {
    OFC_HANDLE write_file;
    OFC_MSTIME start_time;
    OFC_CHAR *buffer;
    OFC_INT i;
    OFC_BOOL status;
    OFC_TCHAR *wfilename;
    OFC_BOOL ret;
    OFC_INT size;
    OFC_ULONG *p;
    OFC_DWORD dwBytesWritten;

    ret = OFC_TRUE;

    /*
   * Create the file used for the copy file test.
   * This is the read file
   */
    wfilename = MakeFilename(device, FS_TEST_READ);
    /*
   * Get the time for simple performance analysis
   */
    status = OFC_TRUE;
    start_time = ofc_time_get_now();
#if defined(OFC_PERF_STATS)
    ofc_perf_reset();
#endif
    /*
   * Open up our write file.  If it exists, it will be deleted
   */
    write_file = OfcCreateFile(wfilename,
                               OFC_GENERIC_WRITE,
                               OFC_FILE_SHARE_DELETE |
                               OFC_FILE_SHARE_WRITE |
                               OFC_FILE_SHARE_READ,
                               OFC_NULL,
                               OFC_CREATE_ALWAYS,
                               OFC_FILE_ATTRIBUTE_NORMAL,
                               OFC_HANDLE_NULL);

    if (write_file == OFC_INVALID_HANDLE_VALUE) {
        ofc_printf("Failed to open Copy Destination %A, Error Code %d\n",
                   wfilename, OfcGetLastError());
        ret = OFC_FALSE;
    } else {
        buffer = ofc_malloc(BUFFER_SIZE);
        if (buffer == OFC_NULL) {
            ofc_printf("OfcFSTest: Failed to allocate Shared "
                       "memory buffer\n");
            ret = OFC_FALSE;
        } else {
            for (size = 0; size < CREATE_SIZE && ret == OFC_TRUE;
                 size += BUFFER_SIZE) {
                p = (OFC_ULONG *) buffer;
                /*
	       * Fill in the buffer with an random data
	       */
                for (i = 0; i < (BUFFER_SIZE / sizeof(OFC_ULONG)); i++) {
                    *p++ = i;
                }

                /*
	       * Write the buffer
	       */
                status = OfcWriteFile(write_file, buffer, BUFFER_SIZE,
                                      &dwBytesWritten, OFC_HANDLE_NULL);

                if (status == OFC_FALSE) {
                    ofc_printf("Write to File Failed with %d\n",
                               OfcGetLastError());
                    ret = OFC_FALSE;
                } else {
                    if (size % (1024 * 1024) == 0)
                        ofc_printf("Wrote %d MB\n", (size / (1024 * 1024)) + 1);
                }
            }
            ofc_free(buffer);
        }
        OfcCloseHandle(write_file);
    }

    ofc_free(wfilename);

#if defined(OFC_PERF_STATS)
    ofc_perf_dump();
#endif
    if (ret == OFC_TRUE)
        ofc_printf("Create File Done, Elapsed Time %dms\n",
                   ofc_time_get_now() - start_time);
    else
        ofc_printf("Create File Test Failed\n");
    return (ret);
}

static OFC_BOOL OfcDismountTest(OFC_CTCHAR *device) {
    OFC_BOOL status;
    OFC_TCHAR *wfilename;
    OFC_BOOL ret;

    ret = OFC_TRUE;

    /*
   * Create the file used for the dismount file test.
   */
    wfilename = MakeFilename(device, FS_TEST_READ);

    status = OFC_TRUE;
    /*
   * Dismount the file
   */
    OfcDismount(wfilename);

    ofc_free(wfilename);

    return (ret);
}

static OFC_BOOL OfcCopyFileTest(OFC_CTCHAR *device) {
    OFC_HANDLE read_file;
    OFC_HANDLE write_file;
    OFC_MSTIME start_time;
    OFC_LARGE_INTEGER offset;
    OFC_BOOL eof;
    OFC_INT pending;
    OFC_HANDLE buffer_list;
    OFC_FILE_BUFFER *buffer;
    OFC_INT i;
    OFC_HANDLE wait_set;
    OFC_DWORD dwLen;
    OFC_BOOL status;
    ASYNC_RESULT result;
    OFC_HANDLE hEvent;
    OFC_TCHAR *rfilename;
    OFC_TCHAR *wfilename;
    OFC_BOOL ret;

    ret = OFC_TRUE;

    rfilename = MakeFilename(device, FS_TEST_READ);
    wfilename = MakeFilename(device, FS_TEST_WRITE);
    /*
   * Get the time for simple performance analysis
   */
    status = OFC_TRUE;
    start_time = ofc_time_get_now();
#if defined(OFC_PERF_STATS)
    ofc_perf_reset();
#endif
    /*
   * Open up our read file.  This file should
   * exist
   */
    read_file = OfcCreateFile(rfilename,
                              OFC_GENERIC_READ,
                              OFC_FILE_SHARE_READ,
                              OFC_NULL,
                              OFC_OPEN_EXISTING,
                              OFC_FILE_ATTRIBUTE_NORMAL |
                              OFC_FILE_FLAG_OVERLAPPED,
                              OFC_HANDLE_NULL);

    if (read_file == OFC_INVALID_HANDLE_VALUE) {
        ofc_printf("Failed to open Copy Source %A, Error Code %d\n",
                   rfilename, OfcGetLastError());
        ret = OFC_FALSE;
    } else {
        /*
       * Open up our write file.  If it exists, it will be deleted
       */
        write_file = OfcCreateFile(wfilename,
                                   OFC_GENERIC_WRITE,
                                   0,
                                   OFC_NULL,
                                   OFC_CREATE_ALWAYS,
                                   OFC_FILE_ATTRIBUTE_NORMAL |
                                   OFC_FILE_FLAG_OVERLAPPED,
                                   OFC_HANDLE_NULL);

        if (write_file == OFC_INVALID_HANDLE_VALUE) {
            ofc_printf("Failed to open Copy Destination %A, Error Code %d\n",
                       wfilename, OfcGetLastError());
            ret = OFC_FALSE;
        } else {
            /*
	   * Now, create a wait set that we will wait for
	   */
            wait_set = ofc_waitset_create();
            /*
	   * And create our own buffer list that we will manage
	   */
            buffer_list = ofc_queue_create();
            /*
	   * Set some flags
	   */
            eof = OFC_FALSE;
            OFC_LARGE_INTEGER_SET(offset, 0, 0);
            pending = 0;
            /*
	   * Prime the engine.  Priming involves obtaining a buffer
	   * for each overlapped I/O and initilizing them
	   */
            for (i = 0; i < NUM_FILE_BUFFERS && !eof; i++) {
                /*
	       * Get the buffer descriptor and the data buffer
	       */
                buffer = ofc_malloc(sizeof(OFC_FILE_BUFFER));
                if (buffer == OFC_NULL) {
                    ofc_printf("test_file: Failed to alloc buffer context\n");
                    ret = OFC_FALSE;
                } else {
                    buffer->data = ofc_malloc(BUFFER_SIZE);
                    if (buffer->data == OFC_NULL) {
                        ofc_printf("test_file: Failed to allocate "
                                   "memory buffer\n");
                        ofc_free(buffer);
                        buffer = OFC_NULL;
                        ret = OFC_FALSE;
                    } else {
                        /*
		       * Initialize the offset to the file
		       */
                        buffer->offset = offset;
                        /*
		       * And initialize the overlapped handles
		       */
                        buffer->readOverlapped = OfcCreateOverlapped(read_file);
                        buffer->writeOverlapped =
                                OfcCreateOverlapped(write_file);
                        if (buffer->readOverlapped == OFC_HANDLE_NULL ||
                            buffer->writeOverlapped == OFC_HANDLE_NULL)
                            ofc_process_crash("An Overlapped Handle is NULL");
                        /*
		       * Add it to our buffer list
		       */
                        ofc_enqueue(buffer_list, buffer);
                        dwLen = BUFFER_SIZE;
                        /*
		       * Issue the read (pre increment the pending to
		       * avoid races
		       */
                        pending++;
                        result = AsyncRead(wait_set, read_file, buffer, dwLen);
                        if (result != ASYNC_RESULT_PENDING) {
                            /*
			   * discount pending and set eof
			   */
                            pending--;
                            if (result == ASYNC_RESULT_ERROR) {
                                ret = OFC_FALSE;
                            }
                            /*
			   * Set eof either because it really is eof, or we
			   * want to clean up.
			   */
                            eof = OFC_TRUE;
                        }
                        /*
		       * Prepare for the next buffer
		       */
#if defined(OFC_64BIT_INTEGER)
                        offset += BUFFER_SIZE;
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
            while (pending > 0) {
                /*
	       * Wait for some buffer to finish (may be a read if we've
	       * just finished priming, but it may be a write also if
	       * we've been in this loop a bit
	       */
                hEvent = ofc_waitset_wait(wait_set);
                if (hEvent != OFC_HANDLE_NULL) {
                    /*
		   * We use the app of the event as a pointer to the
		   * buffer descriptor.  Yeah, this isn't really nice but
		   * the alternative is to add a context to each handle.
		   * That may be cleaner, but basically unnecessary.  If
		   * we did this kind of thing a lot, I'm all for a
		   * new property of a handle
		   */
                    buffer = (OFC_FILE_BUFFER *) ofc_handle_get_app(hEvent);
                    /*
		   * Now we have both read and write overlapped descriptors
		   * See what state we're in
		   */
                    if (buffer->state == BUFFER_STATE_READ) {
                        /*
		       * Read, so let's see the result of the read
		       */
                        result = AsyncReadResult(wait_set, read_file,
                                                 buffer, &dwLen);
                        if (result == ASYNC_RESULT_ERROR) {
                            ret = OFC_FALSE;
                        } else if (result == ASYNC_RESULT_DONE) {
                            /*
			   * When the read is done, let's start up the write
			   */
                            status = AsyncWrite(wait_set, write_file, buffer,
                                                dwLen);
                            if (status == OFC_FALSE) {
                                ret = OFC_FALSE;
                            }
                        }
                        /*
		       * If the write failed, or we had a read result error
		       */
                        if (result == ASYNC_RESULT_ERROR ||
                            result == ASYNC_RESULT_EOF ||
                            (result == ASYNC_RESULT_DONE && status == OFC_FALSE)) {
                            /*
			   * The I/O is no longer pending.
			   */
                            pending--;
                            eof = OFC_TRUE;
                        }
                    } else {
                        /*
		       * The buffer state was write.  Let's look at our
		       * status
		       */
                        result = AsyncWriteResult(wait_set, write_file,
                                                  buffer, &dwLen);
                        /*
		       * Did it finish
		       */
                        if (result == ASYNC_RESULT_DONE) {
                            /*
			   * The write is finished.
			   * Let's step the buffer
			   */
                            /*
			   * Only report on MB boundaries
			   */
                            if (buffer->offset % (1024 * 1024) == 0)
                                ofc_printf("Copied %d MB\n",
                                           (buffer->offset / (1024 * 1024)) + 1);

                            OFC_LARGE_INTEGER_ASSIGN (buffer->offset, offset);
#if defined(OFC_64BIT_INTEGER)
                            offset += BUFFER_SIZE;
#else
                            offset.low += BUFFER_SIZE ;
#endif
                            /*
			   * And start a read on the next chunk
			   */
                            result = AsyncRead(wait_set, read_file,
                                               buffer, BUFFER_SIZE);
                        }

                        if (result != ASYNC_RESULT_PENDING) {
                            if (result == ASYNC_RESULT_ERROR)
                                ret = OFC_FALSE;

                            eof = OFC_TRUE;
                            pending--;
                        }
                    }
                }
            }

            /*
	   * The pending count is zero so we've gotten completions
	   * either due to errors or eof on all of our outstanding
	   * reads and writes.
	   */
            for (buffer = ofc_dequeue(buffer_list);
                 buffer != OFC_NULL;
                 buffer = ofc_dequeue(buffer_list)) {
                /*
	       * Destroy the overlapped I/O handle for each buffer
	       */
                OfcDestroyOverlapped(read_file, buffer->readOverlapped);
                OfcDestroyOverlapped(write_file, buffer->writeOverlapped);
                /*
	       * Free the data buffer and the buffer descriptor
	       */
                ofc_free(buffer->data);
                ofc_free(buffer);
            }
            /*
	   * Destroy the buffer list
	   */
            ofc_queue_destroy(buffer_list);
            /*
	   * And destroy the wait list
	   */
            ofc_waitset_destroy(wait_set);
            /*
	   * Now close the write file
	   */
            OfcCloseHandle(write_file);
        }
        /*
       * And close the read file
       */
        OfcCloseHandle(read_file);
    }

    ofc_free(rfilename);
    ofc_free(wfilename);

#if defined(OFC_PERF_STATS)
    ofc_perf_dump();
#endif
    if (ret == OFC_TRUE)
        ofc_printf("Copy Done, Elapsed Time %dms\n",
                   ofc_time_get_now() - start_time);
    else
        ofc_printf("Copy Test Failed\n");
    return (ret);
}

/*
 * Delete File Test
 *
 * \param delete test type
 */
static OFC_BOOL
OfcDeleteTest(OFC_CTCHAR *device) {
    OFC_HANDLE write_file;
    OFC_DWORD dwLastError;
    OFC_CHAR *buffer;
    OFC_SIZET len;
    OFC_BOOL status;
    OFC_TCHAR *filename;
    OFC_DWORD dwBytesWritten;
    OFC_BOOL ret;

    ret = OFC_TRUE;
    filename = MakeFilename(device, FS_TEST_DELETE);
    /*
   * First we need to create a file to delete
   * This also tests us creating and writing to a file without overlap
   * i/o
   */
    write_file = OfcCreateFile(filename,
                               OFC_GENERIC_WRITE,
                               OFC_FILE_SHARE_READ,
                               OFC_NULL,
                               OFC_CREATE_ALWAYS,
                               OFC_FILE_ATTRIBUTE_NORMAL,
                               OFC_HANDLE_NULL);

    if (write_file == OFC_INVALID_HANDLE_VALUE) {
        ofc_printf("Failed to create delete file %A, Error Code %d\n",
                   filename, OfcGetLastError());
        ret = OFC_FALSE;
    } else {
        /*
       * Let's write some stuff to the file
       */
        buffer = ofc_malloc(BUFFER_SIZE);
        len = BUFFER_SIZE;

        while (len > 0) {
            len -= ofc_snprintf(buffer + (BUFFER_SIZE - len), len,
                                "This Text begins at offset %d\n",
                                BUFFER_SIZE - len);
        }
        /*
       * And write it to the file
       */
        status = OfcWriteFile(write_file, buffer, BUFFER_SIZE, &dwBytesWritten,
                              OFC_HANDLE_NULL);
        /*
       * Free the buffer
       */
        ofc_free(buffer);
        if (status != OFC_TRUE) {
            /*
	   * We failed writing, complain
	   */
            dwLastError = OfcGetLastError();
            ofc_printf("Write to Delete File Failed with Error %d\n",
                       dwLastError);
            ret = OFC_FALSE;
        } else {
            /* Close file
	   */
            status = OfcCloseHandle(write_file);
            if (status != OFC_TRUE) {
                dwLastError = OfcGetLastError();
                ofc_printf("Close of Delete File Failed with Error %d\n",
                           dwLastError);
                ret = OFC_FALSE;
            } else {
                /*
	       * We've written the file.  Now depending on the type
	       * of delete, delete the file
	       */
                write_file =
                        OfcCreateFile(filename,
                                      OFC_FILE_DELETE,
                                      OFC_FILE_SHARE_DELETE,
                                      OFC_NULL,
                                      OFC_OPEN_EXISTING,
                                      OFC_FILE_FLAG_DELETE_ON_CLOSE,
                                      OFC_HANDLE_NULL);

                if (write_file == OFC_INVALID_HANDLE_VALUE) {
                    ofc_printf("Failed to create delete on close file %A, "
                               "Error Code %d\n",
                               filename,
                               OfcGetLastError());
                    ret = OFC_FALSE;
                } else {
                    /* Close file
		   */
                    status = OfcCloseHandle(write_file);
                    if (status != OFC_TRUE) {
                        dwLastError = OfcGetLastError();
                        ofc_printf("Close of Delete on close File "
                                   "Failed with Error %d\n",
                                   dwLastError);
                        ret = OFC_FALSE;
                    } else
                        ofc_printf("Delete on Close Test Succeeded\n");
                }
            }
        }
    }
    ofc_free(filename);
    return (ret);
}

/*
 * Rename file test
 */
static OFC_BOOL OfcMoveTest(OFC_CTCHAR *device) {
    OFC_HANDLE rename_file;
    OFC_DWORD dwLastError;
    OFC_CHAR *buffer;
    OFC_SIZET len;
    OFC_BOOL status;
    OFC_TCHAR *filename;
    OFC_TCHAR *tofilename;
    OFC_DWORD dwBytesWritten;
    OFC_BOOL ret;
    OFC_TCHAR *dirname;
    OFC_HANDLE dirHandle;

    /*
   * Rename within a directory
   */
    ret = OFC_TRUE;
    dirname = MakeFilename(device, FS_TEST_DIRECTORY);
    status = OfcCreateDirectory(dirname, OFC_NULL);
    if (status == OFC_TRUE || OfcGetLastError() == OFC_ERROR_FILE_EXISTS) {
        ret = OFC_TRUE;
        filename = MakeFilename(device, FS_TEST_RENAME);
        tofilename = MakeFilename(device, FS_TEST_RENAMETO);
        /*
       * First, make sure the destination file is not there
       * We don't care about the error.  It is probable, file does not exist
       */
        OfcDeleteFile(tofilename);
        /*
       * Now create a file to rename
       * Clean up any debris before starting
       */
        status = OfcDeleteFile(tofilename);
        /*
       * First we need to create a file to rename
       * This also tests us creating and writing to a file without overlap
       * i/o
       */
        rename_file = OfcCreateFile(filename,
                                    OFC_GENERIC_WRITE,
                                    OFC_FILE_SHARE_READ,
                                    OFC_NULL,
                                    OFC_CREATE_ALWAYS,
                                    OFC_FILE_ATTRIBUTE_NORMAL,
                                    OFC_HANDLE_NULL);

        if (rename_file == OFC_INVALID_HANDLE_VALUE) {
            ofc_printf("Failed to create rename file %A, Error Code %d\n",
                       filename, OfcGetLastError());
            ret = OFC_FALSE;
        } else {
            /*
	   * Let's write some stuff to the file
	   */
            buffer = ofc_malloc(BUFFER_SIZE);
            len = BUFFER_SIZE;

            while (len > 0) {
                len -= ofc_snprintf(buffer + (BUFFER_SIZE - len), len,
                                    "This Text begins at offset %d\n",
                                    BUFFER_SIZE - len);
            }

            status = OfcWriteFile(rename_file, buffer, BUFFER_SIZE,
                                  &dwBytesWritten, OFC_HANDLE_NULL);

            ofc_free(buffer);
            if (status != OFC_TRUE) {
                dwLastError = OfcGetLastError();
                ofc_printf("Write to Rename File Failed with Error %d\n",
                           dwLastError);
                ret = OFC_FALSE;
            } else {
                /* Close file
	       */
                status = OfcCloseHandle(rename_file);
                if (status != OFC_TRUE) {
                    dwLastError = OfcGetLastError();
                    ofc_printf("Close of Rename File Failed with Error %d\n",
                               dwLastError);
                    ret = OFC_FALSE;
                } else {
                    /*
		   * Now we can rename the file
		   */
                    status = OfcMoveFile(filename, tofilename);
                    if (status == OFC_TRUE) {
                        /*
		       * Now we can delete the file
		       */
                        status = OfcDeleteFile(tofilename);
                        if (status == OFC_TRUE)
                            ofc_printf("Rename Test Succeeded\n");
                        else {
                            dwLastError = OfcGetLastError();
                            ofc_printf("Delete Rename File Failed with Error %d\n",
                                       dwLastError);
                            ret = OFC_FALSE;
                        }
                    } else {
                        dwLastError = OfcGetLastError();
                        ofc_printf("Rename File Failed with Error %d\n",
                                   dwLastError);
                        ret = OFC_FALSE;
                    }
                }
            }
        }
        ofc_free(filename);
        ofc_free(tofilename);

	status = OfcRemoveDirectory(dirname);
	if (status != OFC_TRUE) {
	  dwLastError = OfcGetLastError();
	  ofc_printf("Couldn't delete directory, failed with error %d\n",
		     dwLastError);
	}
    } else {

    dwLastError = OfcGetLastError();
        ofc_printf("Create of Directory Failed with Error %d\n",
                   dwLastError);
        ret = OFC_FALSE;
    }
    ofc_free(dirname);
    return (ret);
}

/*
 * Rename file test
 */
static OFC_BOOL OfcRenameTest(OFC_CTCHAR *device) {
    OFC_HANDLE rename_file;
    OFC_DWORD dwLastError;
    OFC_CHAR *buffer;
    OFC_SIZET len;
    OFC_BOOL status;
    OFC_TCHAR *dirname;
    OFC_HANDLE dirHandle;
    OFC_TCHAR *filename;
    OFC_TCHAR *tofilename;
    OFC_TCHAR *fulltofilename;
    OFC_DWORD dwBytesWritten;
    OFC_BOOL ret;
    OFC_FILE_RENAME_INFO *rename_info;
    OFC_SIZET newlen;
    OFC_SIZET rename_info_len;

    /*
   * Rename within a directory
   */
    ret = OFC_TRUE;
    dirname = MakeFilename(device, FS_TEST_DIRECTORY);
    status = OfcCreateDirectory(dirname, OFC_NULL);
    if (status == OFC_TRUE || OfcGetLastError() == OFC_ERROR_FILE_EXISTS) {
        filename = MakeFilename(device, FS_TEST_RENAME);
        fulltofilename = MakeFilename(device, FS_TEST_RENAMETO);
        tofilename = ofc_tastrdup (FS_TEST_RENAMETO_ROOT);
        /*
       * First we need to create a file to rename
       * This also tests us creating and writing to a file without overlap
       * i/o
       */
        rename_file = OfcCreateFile(filename,
                                    OFC_GENERIC_WRITE,
                                    OFC_FILE_SHARE_READ,
                                    OFC_NULL,
                                    OFC_CREATE_ALWAYS,
                                    OFC_FILE_ATTRIBUTE_NORMAL,
                                    OFC_HANDLE_NULL);

        if (rename_file == OFC_INVALID_HANDLE_VALUE) {
            ofc_printf("Failed to create rename file %A, Error Code %d\n",
                       filename, OfcGetLastError());
            ret = OFC_FALSE;
        } else {
            /*
	   * Let's write some stuff to the file
	   */
            buffer = ofc_malloc(BUFFER_SIZE);
            len = BUFFER_SIZE;

            while (len > 0) {
                len -= ofc_snprintf(buffer + (BUFFER_SIZE - len), len,
                                    "This Text begins at offset %d\n",
                                    BUFFER_SIZE - len);
            }

            status = OfcWriteFile(rename_file, buffer, BUFFER_SIZE, &dwBytesWritten,
                                  OFC_HANDLE_NULL);

            ofc_free(buffer);
            if (status != OFC_TRUE) {
                dwLastError = OfcGetLastError();
                ofc_printf("Write to Rename File Failed with Error %d\n",
                           dwLastError);
                ret = OFC_FALSE;
            } else {
                /* Close file
	       */
                status = OfcCloseHandle(rename_file);
                if (status != OFC_TRUE) {
                    dwLastError = OfcGetLastError();
                    ofc_printf("Close of Rename File Failed with Error %d\n",
                               dwLastError);
                    ret = OFC_FALSE;
                } else {
                    rename_file = OfcCreateFile(filename,
                                                OFC_FILE_DELETE,
                                                OFC_FILE_SHARE_DELETE,
                                                OFC_NULL,
                                                OFC_OPEN_EXISTING,
                                                0,
                                                OFC_HANDLE_NULL);

                    if (rename_file == OFC_INVALID_HANDLE_VALUE) {
                        ofc_printf("Failed to create rename file %S, "
                                   "Error Code %d\n",
                                   filename,
                                   OfcGetLastError());
                        ret = OFC_FALSE;
                    } else {
                        /*
		       * First delete the to file if it exists
		       */
                        OfcDeleteFile(fulltofilename);

                        newlen = ofc_tstrlen(tofilename);
                        rename_info_len = sizeof(OFC_FILE_RENAME_INFO) +
                                          (newlen * sizeof(OFC_WCHAR));

                        rename_info = ofc_malloc(rename_info_len);

                        rename_info->ReplaceIfExists = OFC_FALSE;
                        rename_info->RootDirectory = OFC_HANDLE_NULL;
                        rename_info->FileNameLength =
                                (OFC_DWORD) newlen * sizeof(OFC_WCHAR);
                        ofc_tstrncpy(rename_info->FileName,
                                     tofilename, newlen);
                        status =
                                OfcSetFileInformationByHandle(rename_file,
                                                              OfcFileRenameInfo,
                                                              rename_info,
                                                              (OFC_DWORD) rename_info_len);
                        ofc_free(rename_info);
                        if (status != OFC_TRUE) {
                            dwLastError = OfcGetLastError();
                            ofc_printf("Set File Info on rename File "
                                       "Failed with Error %d\n",
                                       dwLastError);
                            ret = OFC_FALSE;
                        }

                        status = OfcCloseHandle(rename_file);
                        if (status != OFC_TRUE) {
                            dwLastError = OfcGetLastError();
                            ofc_printf("Close of rename File after Rename"
                                       "Failed with Error %d\n",
                                       dwLastError);
                            ret = OFC_FALSE;
                        }
                        /*
		       * Now we can delete the file
		       */
                        status = OfcDeleteFile(fulltofilename);
                        if (status == OFC_TRUE)
                            ofc_printf("Rename Test Succeeded\n");
                        else {
                            dwLastError = OfcGetLastError();
                            ofc_printf("Delete Rename File Failed with Error %d\n",
                                       dwLastError);
                            ret = OFC_FALSE;
                        }
                    }
                }
            }
        }
        ofc_free(filename);
        ofc_free(tofilename);
        ofc_free(fulltofilename);

	status = OfcRemoveDirectory(dirname);
	if (status != OFC_TRUE) {
	  dwLastError = OfcGetLastError();
	  ofc_printf("Couldn't delete directory, failed with error %d\n",
		     dwLastError);
	}
    } else {
        dwLastError = OfcGetLastError();
        ofc_printf("Create of Directory Failed with Error %d\n",
                   dwLastError);
        ret = OFC_FALSE;
    }
    ofc_free(dirname);
    return (ret);
}

/*
 * Flush file test
 */
static OFC_BOOL OfcFlushTest(OFC_CTCHAR *device) {
    OFC_HANDLE flush_file;
    OFC_DWORD dwLastError;
    OFC_CHAR *buffer;
    OFC_SIZET len;
    OFC_BOOL status;
    OFC_INT i;
    OFC_TCHAR *filename;
    OFC_DWORD dwBytesWritten;
    OFC_BOOL ret;

    ret = OFC_TRUE;
    filename = MakeFilename(device, FS_TEST_FLUSH);
    /*
   * First we need to create a file to flush
   * This also tests us creating and writing to a file without overlap
   * i/o
   */
    flush_file = OfcCreateFile(filename,
                               OFC_GENERIC_WRITE,
                               OFC_FILE_SHARE_READ,
                               OFC_NULL,
                               OFC_CREATE_ALWAYS,
                               OFC_FILE_ATTRIBUTE_NORMAL,
                               OFC_HANDLE_NULL);

    if (flush_file == OFC_INVALID_HANDLE_VALUE) {
        ofc_printf("Failed to create flush file %A, Error Code %d\n",
                   filename, OfcGetLastError());
        ret = OFC_FALSE;
    } else {
        /*
       * Let's write some stuff to the file
       */
        buffer = ofc_malloc(BUFFER_SIZE);

        status = OFC_TRUE;
        for (i = 0; i < 10 && status == OFC_TRUE; i++) {
            len = ofc_snprintf(buffer, BUFFER_SIZE,
                               "This is the Text for line %d\n", i);

            status = OfcWriteFile(flush_file, buffer, (OFC_DWORD) len, &dwBytesWritten,
                                  OFC_HANDLE_NULL);

            if (status != OFC_TRUE) {
                dwLastError = OfcGetLastError();
                ofc_printf("Write to Flush File Failed with Error %d\n",
                           dwLastError);
                ret = OFC_FALSE;
            } else {
                /*
	       * Flush the file buffers
	       */
                status = OfcFlushFileBuffers(flush_file);
                if (status != OFC_TRUE) {
                    dwLastError = OfcGetLastError();
                    ofc_printf("Flush to Flush File Failed with Error %d\n",
                               dwLastError);
                    ret = OFC_FALSE;
                }
            }

        }

        ofc_free(buffer);
        /* Close file
       */
        status = OfcCloseHandle(flush_file);
        if (status != OFC_TRUE) {
            dwLastError = OfcGetLastError();
            ofc_printf("Close of Flush File Failed with Error %d\n",
                       dwLastError);
            ret = OFC_FALSE;
        } else {
            /*
	   * Now we can delete the file
	   */
            status = OfcDeleteFile(filename);
            if (status == OFC_TRUE)
                ofc_printf("Flush Test Succeeded\n");
            else {
                dwLastError = OfcGetLastError();
                ofc_printf("Delete of Flush File Failed with Error %d\n",
                           dwLastError);
                ret = OFC_FALSE;
            }
        }
    }
    ofc_free(filename);
    return (ret);
}

/*
 * Create directory test
 */
static OFC_BOOL OfcCreateDirectoryTest(OFC_CTCHAR *device) {
    OFC_BOOL status;
    OFC_DWORD dwLastError;
    OFC_TCHAR *filename;
    OFC_BOOL ret;

    ret = OFC_TRUE;
    filename = MakeFilename(device, FS_TEST_DIRECTORY);
    status = OfcCreateDirectory(filename, OFC_NULL);
    if (status == OFC_TRUE)
        ofc_printf("Create Directory Test Succeeded\n");
    else {
        dwLastError = OfcGetLastError();
        ofc_printf("Create of Directory Failed with Error %d\n",
                   dwLastError);
        ret = OFC_FALSE;
    }
    ofc_free(filename);
    return (ret);
}

/*
 * Delete Directory Test
 */
static OFC_BOOL OfcDeleteDirectoryTest(OFC_CTCHAR *device) {
    OFC_BOOL status;
    OFC_DWORD dwLastError;
    OFC_TCHAR *filename;
    OFC_BOOL ret;
    OFC_HANDLE dirhandle;

    ret = OFC_TRUE;
    filename = MakeFilename(device, FS_TEST_DIRECTORY);

    status = OfcRemoveDirectory(filename);
    if (status != OFC_TRUE) {
      dwLastError = OfcGetLastError();
      ofc_printf("Couldn't delete directory, failed with error %d\n",
		 dwLastError);
    }
    ofc_free(filename);
    return (ret);
}

static OFC_BOOL OfcSetEOFTest(OFC_CTCHAR *device) {
    OFC_HANDLE seteof_file;
    OFC_DWORD dwLastError;
    OFC_CHAR *buffer;
    OFC_SIZET len;
    OFC_BOOL status;
    OFC_DWORD pos;
    OFC_TCHAR *filename;
    OFC_DWORD dwBytesWritten;
    OFC_BOOL ret;

    ret = OFC_TRUE;
    filename = MakeFilename(device, FS_TEST_SETEOF);
    /*
   * First we need to create a file to play with
   * This also tests us creating and writing to a file without overlap
   * i/o
   */
    seteof_file = OfcCreateFile(filename,
                                OFC_GENERIC_WRITE,
                                OFC_FILE_SHARE_READ,
                                OFC_NULL,
                                OFC_CREATE_ALWAYS,
                                OFC_FILE_ATTRIBUTE_NORMAL,
                                OFC_HANDLE_NULL);

    if (seteof_file == OFC_INVALID_HANDLE_VALUE) {
        ofc_printf("Failed to create seteof file %A, Error Code %d\n",
                   filename, OfcGetLastError());
        ret = OFC_FALSE;
    } else {
        /*
       * Let's write some stuff to the file
       */
        buffer = ofc_malloc(BUFFER_SIZE);
        len = BUFFER_SIZE;

        while (len > 0) {
            len -= ofc_snprintf(buffer + (BUFFER_SIZE - len), len,
                                "This Text begins at offset %d\n",
                                BUFFER_SIZE - len);
        }

        status = OfcWriteFile(seteof_file, buffer, BUFFER_SIZE, &dwBytesWritten,
                              OFC_HANDLE_NULL);

        ofc_free(buffer);
        if (status != OFC_TRUE) {
            dwLastError = OfcGetLastError();
            ofc_printf("Write to SetEOF File Failed with Error %d\n",
                       dwLastError);
            ret = OFC_FALSE;
        } else {
            /* Close file
	   */
            status = OfcCloseHandle(seteof_file);
            if (status != OFC_TRUE) {
                dwLastError = OfcGetLastError();
                ofc_printf("Close of SetEOF File Failed with Error %d\n",
                           dwLastError);
                ret = OFC_FALSE;
            } else {
                /*
	       * Now we want to open the file for read/write
	       */
                seteof_file = OfcCreateFile(filename,
                                            OFC_GENERIC_WRITE |
                                            OFC_GENERIC_READ,
                                            OFC_FILE_SHARE_READ,
                                            OFC_NULL,
                                            OFC_OPEN_EXISTING,
                                            OFC_FILE_ATTRIBUTE_NORMAL,
                                            OFC_HANDLE_NULL);

                if (seteof_file == OFC_INVALID_HANDLE_VALUE) {
                    ofc_printf("Failed to open seteof file %A, "
                               "Error Code %d\n",
                               filename, OfcGetLastError());
                    ret = OFC_FALSE;
                } else {
                    /*
		   * So now we want to set the file pointer to say, 2048
		   */
                    pos = OfcSetFilePointer(seteof_file,
                                            BUFFER_SIZE / 2,
                                            OFC_NULL,
                                            OFC_FILE_BEGIN);
                    dwLastError = OfcGetLastError();
                    if (pos == OFC_INVALID_SET_FILE_POINTER &&
                        dwLastError != OFC_ERROR_SUCCESS) {
                        ofc_printf("SetFilePosition Failed with Error %d\n",
                                   dwLastError);
                        ret = OFC_FALSE;
                    } else {
                        /*
		       * Now we want to set eof
		       */
                        status = OfcSetEndOfFile(seteof_file);
                        if (status != OFC_TRUE) {
                            dwLastError = OfcGetLastError();
                            ofc_printf("Set End Of File Failed with %d\n",
                                       dwLastError);
                            ret = OFC_FALSE;
                        }
#if 0
                                                                                                                                                else
			{
			  OFC_FILE_STANDARD_INFO standard_info ;

			  status = OfcGetFileInformationByHandleEx
			    (seteof_file,
			     OfcFileStandardInfo,
			     &standard_info,
			     sizeof (OFC_FILE_STANDARD_INFO)) ;

			  if (status != OFC_TRUE)
			    {
			      dwLastError = OfcGetLastError () ;
			      ofc_printf ("Cannot get end of file position %d\n",
					   dwLastError) ;
			      ret = OFC_FALSE ;
			    }
			  else
			    {
			      if (standard_info.EndOfFile != BUFFER_SIZE / 2)
				{
				  ofc_printf
				    ("End Of File Doesn't Match.  Expected %d, Got %d\n",
				     BUFFER_SIZE / 2,
				     (OFC_INT) standard_info.EndOfFile) ;
				  ret = OFC_FALSE ;
				}
			    }
			}
#endif
                    }
                    status = OfcCloseHandle(seteof_file);
                    if (status != OFC_TRUE) {
                        dwLastError = OfcGetLastError();
                        ofc_printf("Close of SetEOF File "
                                   "after pos Failed with Error %d\n",
                                   dwLastError);
                        ret = OFC_FALSE;
                    }
                }
                /*
	       * Now we can delete the file
	       */
                status = OfcDeleteFile(filename);
                if (status == OFC_TRUE)
                    ofc_printf("SetEOF Test Succeeded\n");
                else {
                    dwLastError = OfcGetLastError();
                    ofc_printf("SetEOF Delete File Failed with Error %d\n",
                               dwLastError);
                    ret = OFC_FALSE;
                }
            }
        }
    }
    ofc_free(filename);
    return (ret);
}

/*
 * Atrtribute test
 */
static OFC_CHAR *Attr2Str[17] =
        {
                "RO",            /* Read Only */
                "HID",            /* Hidden */
                "SYS",            /* System */
                "",
                "DIR",            /* Directory */
                "ARV",            /* Archive */
                "",
                "NRM",            /* Normal */
                "TMP",            /* Temporary */
                "SPR",            /* Sparse */
                "",
                "CMP",            /* Compressed */
                "OFF",            /* Offline */
                "",
                "ENC",            /* Encrypted */
                ""
                "VIRT",            /* Virtual */
        };

static OFC_VOID OfcFSPrintFindData(OFC_WIN32_FIND_DATA *find_data) {
    OFC_INT i;
    OFC_UINT16 mask;
    OFC_CHAR str[100];        /* Guaranteed big enough for all attributes */
    OFC_CHAR *p;
    OFC_BOOL first;
    OFC_WORD fat_date;
    OFC_WORD fat_time;
    OFC_UINT16 month;
    OFC_UINT16 day;
    OFC_UINT16 year;
    OFC_UINT16 hour;
    OFC_UINT16 min;
    OFC_UINT16 sec;

    ofc_printf("File: %A\n", find_data->cFileName);
    ofc_printf("Alternate Name: %.14A\n", find_data->cAlternateFileName);
    ofc_printf("Attributes: 0x%08x\n", find_data->dwFileAttributes);

    mask = 0x0001;
    str[0] = '\0';
    p = str;
    first = OFC_TRUE;
    for (i = 0; i < 16; i++, mask <<= 1) {
        if (find_data->dwFileAttributes & mask) {
            if (first)
                first = OFC_FALSE;
            else {
                ofc_strcpy(p, ", ");
                p += 2;
            }
            ofc_strcpy(p, Attr2Str[i]);
            p += ofc_strlen(Attr2Str[i]);
        }
    }
    *p = '\0';

    ofc_printf("    %s\n", str);

    ofc_file_time_to_dos_date_time(&find_data->ftCreateTime,
                                   &fat_date, &fat_time);

    ofc_dos_date_time_to_elements(fat_date, fat_time,
                                  &month, &day, &year, &hour, &min, &sec);
    ofc_printf("Create Time: %02d/%02d/%04d %02d:%02d:%02d GMT\n",
               month, day, year, hour, min, sec);
    ofc_file_time_to_dos_date_time(&find_data->ftLastAccessTime,
                                   &fat_date, &fat_time);

    ofc_dos_date_time_to_elements(fat_date, fat_time,
                                  &month, &day, &year, &hour, &min, &sec);
    ofc_printf("Last Access Time: %02d/%02d/%04d %02d:%02d:%02d GMT\n",
               month, day, year, hour, min, sec);
    ofc_file_time_to_dos_date_time(&find_data->ftLastWriteTime,
                                   &fat_date, &fat_time);
    ofc_dos_date_time_to_elements(fat_date, fat_time,
                                  &month, &day, &year, &hour, &min, &sec);
    ofc_printf("Last Write Time: %02d/%02d/%04d %02d:%02d:%02d GMT\n",
               month, day, year, hour, min, sec);

    ofc_printf("File Size High: 0x%08x, Low: 0x%08x\n",
               find_data->nFileSizeHigh, find_data->nFileSizeLow);
    ofc_printf("\n");
}

static OFC_VOID
OfcFSPrintFileAttributeData(OFC_WIN32_FILE_ATTRIBUTE_DATA *file_data) {
    OFC_INT i;
    OFC_UINT16 mask;
    OFC_CHAR str[100];        /* Guaranteed big enough for all attributes */
    OFC_CHAR *p;
    OFC_BOOL first;
    OFC_WORD fat_date;
    OFC_WORD fat_time;
    OFC_UINT16 month;
    OFC_UINT16 day;
    OFC_UINT16 year;
    OFC_UINT16 hour;
    OFC_UINT16 min;
    OFC_UINT16 sec;

    ofc_printf("    Attributes: 0x%08x\n", file_data->dwFileAttributes);

    mask = 0x0001;
    str[0] = '\0';
    p = str;
    first = OFC_TRUE;
    for (i = 0; i < 16; i++, mask <<= 1) {
        if (file_data->dwFileAttributes & mask) {
            if (first)
                first = OFC_FALSE;
            else {
                ofc_strcpy(p, ", ");
                p += 2;
            }
            ofc_strcpy(p, Attr2Str[i]);
            p += ofc_strlen(Attr2Str[i]);
        }
    }
    *p = '\0';

    ofc_printf("    %s\n", str);

    ofc_file_time_to_dos_date_time(&file_data->ftCreateTime,
                                   &fat_date, &fat_time);

    ofc_dos_date_time_to_elements(fat_date, fat_time,
                                  &month, &day, &year, &hour, &min, &sec);
    ofc_printf("Create Time: %02d/%02d/%04d %02d:%02d:%02d GMT\n",
               month, day, year, hour, min, sec);
    ofc_file_time_to_dos_date_time(&file_data->ftLastAccessTime,
                                   &fat_date, &fat_time);

    ofc_dos_date_time_to_elements(fat_date, fat_time,
                                  &month, &day, &year, &hour, &min, &sec);
    ofc_printf("Last Access Time: %02d/%02d/%04d %02d:%02d:%02d GMT\n",
               month, day, year, hour, min, sec);
    ofc_file_time_to_dos_date_time(&file_data->ftLastWriteTime,
                                   &fat_date, &fat_time);
    ofc_dos_date_time_to_elements(fat_date, fat_time,
                                  &month, &day, &year, &hour, &min, &sec);
    ofc_printf("Last Write Time: %02d/%02d/%04d %02d:%02d:%02d GMT\n",
               month, day, year, hour, min, sec);

    ofc_printf("File Size High: 0x%08x, Low: 0x%08x\n",
               file_data->nFileSizeHigh, file_data->nFileSizeLow);
}

static OFC_BOOL OfcListDirTest(OFC_CTCHAR *device) {
    OFC_HANDLE list_handle;
    OFC_WIN32_FIND_DATA find_data;
    OFC_BOOL more = OFC_FALSE;
    OFC_BOOL status;
    OFC_TCHAR *filename;
    OFC_DWORD last_error;
    OFC_BOOL retry;
    OFC_CHAR username[80];
    OFC_CHAR password[80];
    OFC_CHAR domain[80];
    OFC_TCHAR *tusername;
    OFC_TCHAR *tpassword;
    OFC_TCHAR *tdomain;
    OFC_BOOL ret;
    OFC_INT count;

    ret = OFC_TRUE;
    retry = OFC_TRUE;
    list_handle = OFC_INVALID_HANDLE_VALUE;
    /*
   * The initial credentials in device are anonymous.  If that fails,
   * prompt for credentials and try again.
   */

    count = 0;
    while (retry == OFC_TRUE) {
        filename = MakeFilename(device, TSTR("*"));

        list_handle = OfcFindFirstFile(filename, &find_data, &more);

        if (list_handle != OFC_INVALID_HANDLE_VALUE)
            retry = OFC_FALSE;
        else {
            last_error = OfcGetLastError();
            ofc_printf("Failed to list dir %A, Error Code %d\n",
                       filename, last_error);
            /*
	   * LOGON_FAILURE can be returned if authentication failed
	   * ACCESS_DENIED if authentication succeeded but no access to share
	   * INVALID_PASSWORD if password different then connection
	   */
            if (last_error == OFC_ERROR_LOGON_FAILURE ||
                last_error == OFC_ERROR_ACCESS_DENIED ||
                last_error == OFC_ERROR_INVALID_PASSWORD) {
                ofc_printf("Please Authenticate\n");
                ofc_printf("Username: ");
                ofc_read_line(username, 80);
                if (ofc_strlen(username) == 0)
                    retry = OFC_FALSE;
                else {
                    tusername = ofc_cstr2tastr (username);
                    ofc_printf("Domain: ");
                    ofc_read_line(domain, 80);
                    tdomain = ofc_cstr2tastr (domain);
                    ofc_printf("Password: ");
                    ofc_read_password(password, 80);
                    tpassword = ofc_cstr2tastr (password);
                    /*
		   * Now remap the device
		   */
                    ofc_path_update_credentials(filename, tusername,
                                                tpassword, tdomain);
                    ofc_free(tusername);
                    ofc_free(tpassword);
                    ofc_free(tdomain);
                }
            } else {
                retry = OFC_FALSE;
                ret = OFC_FALSE;
            }
        }
        ofc_free(filename);
    }

    if (list_handle != OFC_INVALID_HANDLE_VALUE) {
        count++;
        OfcFSPrintFindData(&find_data);

        status = OFC_TRUE;
        while (more && status == OFC_TRUE) {
            status = OfcFindNextFile(list_handle,
                                     &find_data,
                                     &more);
            if (status == OFC_TRUE) {
                count++;
                OfcFSPrintFindData(&find_data);
            } else {
                last_error = OfcGetLastError();
                if (last_error != OFC_ERROR_NO_MORE_FILES) {
                    ofc_printf("Failed to Find Next, Error Code %d\n",
                               last_error);
                    ret = OFC_FALSE;
                }
            }
        }
        OfcFindClose(list_handle);
    }
    ofc_printf("Total Number of Files in Directory %d\n", count);
    return (ret);
}

static OFC_BOOL OfcGetFileAttributesTest(OFC_CTCHAR *device) {
    OFC_HANDLE getex_file;
    OFC_DWORD dwLastError;
    OFC_CHAR *buffer;
    OFC_SIZET len;
    OFC_BOOL status;
    OFC_WIN32_FILE_ATTRIBUTE_DATA fInfo;
    OFC_TCHAR *filename;
    OFC_DWORD dwBytesWritten;
    OFC_BOOL ret;

    ret = OFC_TRUE;
    filename = MakeFilename(device, FS_TEST_GETEX);
    /*
   * First we need to create a file to play with
   */
    getex_file = OfcCreateFile(filename,
                               OFC_GENERIC_WRITE,
                               OFC_FILE_SHARE_READ,
                               OFC_NULL,
                               OFC_CREATE_ALWAYS,
                               OFC_FILE_ATTRIBUTE_NORMAL,
                               OFC_HANDLE_NULL);

    if (getex_file == OFC_INVALID_HANDLE_VALUE) {
        ofc_printf("Failed to create getex file %A, Error Code %d\n",
                   filename, OfcGetLastError());
        ret = OFC_FALSE;
    } else {
        /*
       * Let's write some stuff to the file
       */
        buffer = ofc_malloc(BUFFER_SIZE);
        len = BUFFER_SIZE;

        while (len > 0) {
            len -= ofc_snprintf(buffer + (BUFFER_SIZE - len), len,
                                "This Text begins at offset %d\n",
                                BUFFER_SIZE - len);
        }

        status = OfcWriteFile(getex_file, buffer, BUFFER_SIZE, &dwBytesWritten,
                              OFC_HANDLE_NULL);

        ofc_free(buffer);
        if (status != OFC_TRUE) {
            dwLastError = OfcGetLastError();
            ofc_printf("Write to GetEX File Failed with Error %d\n",
                       dwLastError);
            ret = OFC_FALSE;
        } else {
            /* Close file
	   */
            status = OfcCloseHandle(getex_file);
            if (status != OFC_TRUE) {
                dwLastError = OfcGetLastError();
                ofc_printf("Close of GetEX File Failed with Error %d\n",
                           dwLastError);
                ret = OFC_FALSE;
            } else {
                status = OfcGetFileAttributesEx(filename,
                                                OfcGetFileExInfoStandard,
                                                &fInfo);
                if (status != OFC_TRUE) {
                    dwLastError = OfcGetLastError();
                    ofc_printf("GetEx Get File Info Failed with Error %d\n",
                               dwLastError);
                    ret = OFC_FALSE;
                } else {
                    ofc_printf("Get File Info for %A\n", filename);
                    OfcFSPrintFileAttributeData(&fInfo);
                }
                /*
	       * Now we can delete the file
	       */
                status = OfcDeleteFile(filename);
                if (status != OFC_TRUE) {
                    dwLastError = OfcGetLastError();
                    ofc_printf("GetEx Delete File Failed with Error %d\n",
                               dwLastError);
                    ret = OFC_FALSE;
                }
            }
        }
    }
    ofc_free(filename);
    return (ret);
}

static OFC_BOOL OfcGetDiskFreeSpaceTest(OFC_CTCHAR *device) {
    OFC_DWORD SectorsPerCluster;
    OFC_DWORD BytesPerSector;
    OFC_DWORD NumberOfFreeClusters;
    OFC_DWORD TotalNumberOfClusters;
    OFC_BOOL result;
    OFC_TCHAR *filename;
    OFC_BOOL ret;

    ret = OFC_TRUE;
    filename = MakeFilename(device, TSTR(":"));

    result = OfcGetDiskFreeSpace(filename, &SectorsPerCluster,
                                 &BytesPerSector, &NumberOfFreeClusters,
                                 &TotalNumberOfClusters);
    if (result == OFC_FALSE) {
        ofc_printf("Failed to get disk free space on %A, Error Code %d\n",
                   filename, OfcGetLastError());
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
        ofc_printf("Not supported on reverse connections, "
                   "see code comment\n");
        ret = OFC_TRUE;
    } else {
        ofc_printf("Free Space On %A\n", filename);
        ofc_printf("  Sectors Per Cluster: %d\n", SectorsPerCluster);
        ofc_printf("  Bytes Per Sector: %d\n", BytesPerSector);
        ofc_printf("  Number of Free Clusters: %d\n", NumberOfFreeClusters);
        ofc_printf("  Total Number of Clusters: %d\n", TotalNumberOfClusters);
    }
    ofc_free(filename);
    return (ret);
}

static OFC_BOOL OfcGetVolumeInformationTest(OFC_CTCHAR *device) {
    OFC_TCHAR VolumeNameBuffer[OFC_MAX_PATH + 1];
    OFC_DWORD VolumeSerialNumber;
    OFC_DWORD MaximumComponentLength;
    OFC_DWORD FileSystemFlags;
    OFC_TCHAR FileSystemName[OFC_MAX_PATH + 1];
    OFC_BOOL result;
    OFC_TCHAR *filename;
    OFC_BOOL ret;
    OFC_TCHAR *root;

    ret = OFC_TRUE;
    filename = MakeFilename(device, TSTR(":"));
    ofc_path_get_root(filename, &root, OFC_NULL);

    result = OfcGetVolumeInformation(root,
                                     VolumeNameBuffer,
                                     OFC_MAX_PATH + 1,
                                     &VolumeSerialNumber,
                                     &MaximumComponentLength,
                                     &FileSystemFlags,
                                     FileSystemName,
                                     OFC_MAX_PATH + 1);
    if (result == OFC_FALSE) {
        ofc_printf("Failed to get volume info on %A, Error Code %d\n",
                   filename, OfcGetLastError());
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
        ofc_printf("Not supported on reverse connections, "
                   "see code comment\n");
        ret = OFC_TRUE;
    } else {
        ofc_printf("Volume Info for %A\n", filename);
        ofc_printf("  Volume Name: %A\n", VolumeNameBuffer);
        ofc_printf("  Volume Serial Number: 0x%08x\n", VolumeSerialNumber);
        ofc_printf("  Max Component Length: %d\n", MaximumComponentLength);
        ofc_printf("  File System Flags: 0x%08x\n", FileSystemFlags);
        ofc_printf("  File System Name: %A\n", FileSystemName);
    }
    ofc_free(filename);
    ofc_free(root);
    return (ret);
}

struct _test_path {
    OFC_CHAR *description;
    OFC_CHAR *username;
    OFC_CHAR *password;
    OFC_CHAR *domain;
    OFC_CHAR *smbserver;
    OFC_CHAR *sharefolder;
    OFC_CHAR *subdirectory;
    OFC_UINT32 expected_error1;
    OFC_UINT32 expected_error2;
};

static struct _test_path test_paths[] =
        {
                /* Test 0, Good case */
                {
                        "Valid Path",
                        "user", "password", "workgroup",
                        "tester", "openfiles", "mydir",
                        OFC_ERROR_SUCCESS,        OFC_ERROR_SUCCESS
                },
                /* Test 1, SMB server is wrong */
                {
                        "Non-existent SMB Server",
                        "user", "password", "workgroup",
                        "bogus",  "openfiles", "mydir",
                        OFC_ERROR_BAD_NET_NAME,   OFC_ERROR_BAD_NET_NAME
                },
                /* Test 2, Share folder is wrong */
                {
                        "Non-existent SMB Share",
                        "user", "password", "workgroup",
                        "tester", "bogus",     "mydir",
                        OFC_ERROR_BAD_NETPATH,    OFC_ERROR_BAD_NETPATH
                },
                /* Test 3, Password is wrong */
                {
                        "Bad Passwowrd",
                        "user", "bogus",    "workgroup",
                        "tester", "openfiles", "mydir",
                        OFC_ERROR_LOGON_FAILURE,  OFC_ERROR_INVALID_PASSWORD
                },
                /* Test 4, Subdirectory is wrong */
                {
                        "Non-existent Subdirectory",
                        "user", "password", "workgroup",
                        "tester", "openfiles", "bogus",
                        OFC_ERROR_FILE_NOT_FOUND, OFC_ERROR_PATH_NOT_FOUND
                },
                /* Test 5, Share folder doesn't have write permission */
                {
                        "Read-Only Share",
                        "user", "password", "workgroup",
                        "tester", "roshare",   "mydir",
                        OFC_ERROR_ACCESS_DENIED,  OFC_ERROR_PATH_NOT_FOUND
                },
                /* Test 6, Subdirectory doesn't have write permission */
                {
                        "Read-Only Subdirectory",
                        "user", "password", "workgroup",
                        "tester", "openfiles", "rodir",
                        OFC_ERROR_ACCESS_DENIED,  OFC_ERROR_ACCESS_DENIED
                },
                /* end of list */
                {
                        OFC_NULL,
                        OFC_NULL, OFC_NULL, OFC_NULL,
                        OFC_NULL, OFC_NULL, OFC_NULL,
                        OFC_ERROR_SUCCESS,        OFC_ERROR_SUCCESS
                }
        };

static OFC_BOOL OfcBadFileNamesTest(OFC_CTCHAR *device) {
    OFC_BOOL ret;
    OFC_BOOL test_status;
    OFC_HANDLE hFile;

    char uncfilename[OFC_MAX_PATH];
    OFC_SIZET filelen;
    char *tmpfilename;
    OFC_UINT32 lasterror;

    OFC_INT i;

    ret = OFC_TRUE;

    for (i = 0; test_paths[i].description != OFC_NULL; i++) {
        ofc_printf("\n");
        ofc_printf("Test %d, %s\n", i, test_paths[i].description);
        /*
       * method A, Opening Directory
       */
        filelen = OFC_MAX_PATH;
        tmpfilename = &uncfilename[0];
        ofc_path_make_urlA(&tmpfilename, &filelen,
                           test_paths[i].username,
                           test_paths[i].password,
                           test_paths[i].domain,
                           test_paths[i].smbserver,
                           test_paths[i].sharefolder,
                           test_paths[i].subdirectory,
                           OFC_NULL);

        ofc_printf("Writeable Directory Mode\n");
        ofc_printf("Testing Write Access to %s\n", uncfilename);

        hFile = OfcCreateFileA(uncfilename,
                               OFC_GENERIC_WRITE,
                               OFC_FILE_SHARE_WRITE,
                               OFC_NULL,
                               OFC_OPEN_EXISTING,
                               OFC_FILE_ATTRIBUTE_DIRECTORY,
                               OFC_HANDLE_NULL);

        test_status = OFC_TRUE;
        if (hFile == OFC_INVALID_HANDLE_VALUE) {
            lasterror = OfcGetLastError();
        } else {
            OfcCloseHandle(hFile);
            lasterror = OFC_ERROR_SUCCESS;
        }

        if (test_paths[i].expected_error1 != lasterror &&
            test_paths[i].expected_error2 != lasterror) {
            test_status = OFC_FALSE;
            ret = OFC_FALSE;
        }

        ofc_printf("%s: last error %d, expected error %d or %d\n",
                   test_status == OFC_TRUE ? "SUCCESS" : "FAILURE",
                   lasterror, test_paths[i].expected_error1,
                   test_paths[i].expected_error2);
        OfcDismountA(uncfilename);
        /*
       * method B, Temporary File
       */
        filelen = OFC_MAX_PATH;
        tmpfilename = &uncfilename[0];
        ofc_path_make_urlA(&tmpfilename, &filelen,
                           test_paths[i].username,
                           test_paths[i].password,
                           test_paths[i].domain,
                           test_paths[i].smbserver,
                           test_paths[i].sharefolder,
                           test_paths[i].subdirectory,
                           "uniquefile");
        ofc_printf("Writeable Temporary File Mode\n");
        ofc_printf("Testing Write Access to %s\n", uncfilename);

        hFile = OfcCreateFileA(uncfilename,
                               OFC_GENERIC_READ | OFC_GENERIC_WRITE |
                               OFC_FILE_DELETE,
                               OFC_FILE_SHARE_NONE,
                               OFC_NULL,
                               OFC_CREATE_NEW,
                               OFC_FILE_ATTRIBUTE_TEMPORARY |
                               OFC_FILE_FLAG_DELETE_ON_CLOSE,
                               OFC_HANDLE_NULL);
        test_status = OFC_TRUE;
        if (hFile == OFC_INVALID_HANDLE_VALUE) {
            lasterror = OfcGetLastError();
        } else {
            OfcCloseHandle(hFile);
            lasterror = OFC_ERROR_SUCCESS;
        }

        if (test_paths[i].expected_error1 != lasterror &&
            test_paths[i].expected_error2 != lasterror) {
            test_status = OFC_FALSE;
            ret = OFC_FALSE;
        }

        ofc_printf("%s: last error %d, expected error %d or %d\n",
                   test_status == OFC_TRUE ? "SUCCESS" : "FAILURE",
                   lasterror, test_paths[i].expected_error1,
                   test_paths[i].expected_error2);
        OfcDismountA(uncfilename);
    }
    return (ret);
}

/*
 * Startup Entry point to the test
 *
 * \param hScheduler
 * The scheduler that this test should create apps within (if any)
 * In our case, the file system test does not create any apps.
 */
OFC_INT test_file(OFC_LPCSTR test_root) {
    OFC_DWORD ret;
    OFC_TCHAR *device;
    OFC_BOOL test_result;
    OFC_INT count;

    /*
   * Map the test device to the test path.  Determine what
   * File System Handler we should test.
   */
    ofc_printf("Starting File Test with %s\n", test_root);

    ofc_sleep(3000);
    ofc_thread_create_local_storage();

    count = 0;

    device = ofc_cstr2tstr(test_root);

    while (count != OFC_FILE_TEST_COUNT) {
        test_result = OFC_TRUE;

	/*
	 * OfcGetFileAttributesExW on "/" (initially failed with java)
	 */
	{
	  OFC_WIN32_FILE_ATTRIBUTE_DATA fadFile;
	  OFC_BOOL bret;
	  ofc_printf("Trying an OfcGetFileAttributesExW on /");
	  bret = OfcGetFileAttributesExW(TSTR("/"),
					 OfcGetFileExInfoStandard,
					 &fadFile);
	  if (bret != OFC_TRUE)
	    {
	      ofc_printf ("%s: OfcGetFileAttributesExW failed for %S, Last Error %d\n",
			  __func__, TSTR("/"), OfcGetLastError()) ;
	    }
	  else
	    ofc_printf("Succeeded\n");
	}

        /*
       * First, create a random file
       */
        ofc_printf("  Create File Test\n");
        if (OfcCreateFileTest(device) == OFC_FALSE) {
            ofc_printf("  *** Create File Test Failed *** \n");
            test_result = OFC_FALSE;
        }
        ofc_printf("  Dismount Test\n");
        if (OfcDismountTest(device) == OFC_FALSE) {
            ofc_printf("  *** Dismount Test Failed ***\n");
            test_result = OFC_FALSE;
        }
        /* This may or may not fail.  It is posible for the previous test
       * to complete (dismount test) before the connection has been
       * cleaned up.  So, if the connection is up, the API will succeed.
       * if the connection has already gone down, then the API will
       * fail.
       * Either case is good
       */
        ofc_printf("  Dismount Test with No connection\n");
        OfcDismountTest(device);
        /*
       * Then see if we can copy files
       */
        ofc_printf("  Copy File Test\n");
        if (OfcCopyFileTest(device) == OFC_FALSE) {
            ofc_printf("  *** Copy File Test Failed *** \n");
            test_result = OFC_FALSE;
        }
        ofc_printf("  List Directory Test\n");
        if (OfcListDirTest(device) == OFC_FALSE) {
            ofc_printf("  *** List Directory Test Failed *** \n");
            test_result = OFC_FALSE;
        }
        ofc_printf("  Delete File Test\n");
        if (OfcDeleteTest(device) == OFC_FALSE) {
            ofc_printf("  *** Delete File Test Failed ***\n");
            test_result = OFC_FALSE;
        }
        /* This one only works in SMBv2, we can update SMBv1 to support
       *  this style
       * Since the API is only supported on SMBv2, we should use the
       * move API instead.  So, don't enable this approach yet.
       */
#if 0
                                                                                                                                ofc_printf ("  Rename File Test\n") ;
      if (OfcRenameTest (device) == OFC_FALSE)
	{
	  ofc_printf ("  *** Rename File Test Failed ***\n") ;
	  ofc_printf ("  Failures expected on SMBv1 Systems\n") ;
	  test_result = OFC_FALSE ;
	}
#endif
        ofc_printf("  Move File Test\n");
        if (OfcMoveTest(device) == OFC_FALSE) {
            ofc_printf("  *** Move File Test Failed ***\n");
            test_result = OFC_FALSE;
        }
        ofc_printf("  Flush File Test\n");
        if (OfcFlushTest(device) == OFC_FALSE) {
            ofc_printf("  *** Flush File Test Failed ***\n");
            test_result = OFC_FALSE;
        }
        ofc_printf("  Create Directory Test\n");
        if (OfcCreateDirectoryTest(device) == OFC_FALSE) {
            ofc_printf("  *** Create Directory Test Failed ***\n");
            test_result = OFC_FALSE;
        }
        ofc_printf("  Delete Directory Test\n");
        if (OfcDeleteDirectoryTest(device) == OFC_FALSE) {
            ofc_printf("  *** Delete Directory Test Failed ***\n");
            test_result = OFC_FALSE;
        }
        ofc_printf("  Set File Pointer and Set EOF Test\n");
        if (OfcSetEOFTest(device) == OFC_FALSE) {
            ofc_printf("  *** Set EOF Test Failed ***\n");
            test_result = OFC_FALSE;
        }
        ofc_printf("  Get File Attributes Test\n");
        if (OfcGetFileAttributesTest(device) == OFC_FALSE) {
            ofc_printf(" *** Get File Attributes Test Failed ***\n");
            test_result = OFC_FALSE;
        }
        ofc_printf("  Get Disk Free Space Test\n");
        if (OfcGetDiskFreeSpaceTest(device) == OFC_FALSE) {
            ofc_printf("  *** Get Disk Free Space Failed ***\n");
            test_result = OFC_FALSE;
        }
        ofc_printf("  Get Volume Information Test\n");
        if (OfcGetVolumeInformationTest(device) == OFC_FALSE) {
            ofc_printf("  *** Get Volume Information Failed ***\n");
            test_result = OFC_FALSE;
        }

        if (test_result == OFC_FALSE)
            ofc_printf("*** File Test Failed ***\n");
        else
            ofc_printf("File Test Succeeded\n");
        count++;

        if (count != OFC_FILE_TEST_COUNT)
            ofc_sleep(OFC_FS_TEST_INTERVAL);
    }

    ofc_free(device);
    return (0);
}

