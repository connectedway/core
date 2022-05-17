/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#include "unity.h"
#include "unity_fixture.h"

#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/libc.h"
#include "ofc/event.h"
#include "ofc/thread.h"
#include "ofc/file.h"

#include "ofc/heap.h"

/**
 * \defgroup PipeTest Sample Pipe Client Test
 * \{ */
#define OFC_FSPIPE_TEST_COUNT 5
extern OFC_CHAR config_path[OFC_MAX_PATH+1];
OFC_VOID test_shutdown(OFC_VOID);
OFC_INT test_startup(OFC_VOID);
OFC_HANDLE hNotify;

static OFC_DWORD OfsFSPipeServerApp (OFC_HANDLE hThread, 
				       OFC_VOID *context) 
{
  OFC_HANDLE test_file ;
  OFC_DWORD dwLen ;
  OFC_CHAR *buf = "This is a test message\n" ;
  OFC_INT count ;

  /*
   * Let's first just see if we can open write and close a file
   */
  test_file = OfcCreateFile (TASTR("IPC:/test.test"),
			     OFC_GENERIC_READ | OFC_GENERIC_WRITE,
			     OFC_FILE_SHARE_READ,
			     OFC_NULL,
			     OFC_CREATE_ALWAYS,
			     OFC_FILE_ATTRIBUTE_NORMAL,
			     OFC_HANDLE_NULL) ;

  if (test_file == OFC_HANDLE_NULL)
    ofc_printf ("Couldn't open pipe server file\n") ;
  else
    {
      for (count = 0 ; count < OFC_FSPIPE_TEST_COUNT ; count++) 
	{
	  dwLen = (OFC_DWORD) ofc_strlen (buf) ;
	  OfcWriteFile (test_file, buf, dwLen, &dwLen, OFC_HANDLE_NULL) ;
	  ofc_sleep (2000) ;
	}
      OfcWriteFile (test_file, "\0", 1, &dwLen, OFC_HANDLE_NULL) ;
      ofc_printf ("Closing Pipe Write File\n") ;
      OfcCloseHandle (test_file) ;
    }
  return (0) ;
}

static OFC_DWORD OfsFSPipeClientApp (OFC_HANDLE hThread, 
				     OFC_VOID *context) 
{
  OFC_HANDLE test_file ;
  OFC_DWORD dwLen ;
  OFC_CHAR buf[11] ;
  OFC_BOOL quit ;
  OFC_HANDLE hServerApp ;
  OFC_BOOL ret ;
  OFC_INT count ;

  count = 0 ;
  while (!ofc_thread_is_deleting (hThread) && 
	 (count < 5))
    {
      /*
       * Create the server
       */
      hServerApp = ofc_thread_create (&OfsFSPipeServerApp, 
				     OFC_THREAD_PIPE_TEST, count,
				     OFC_NULL,
				     OFC_THREAD_DETACH, OFC_HANDLE_NULL) ;
      if (hServerApp == OFC_HANDLE_NULL)
	ofc_printf ("Could not create OfsFSPipeServerApp\n") ;
      else
	{
	  /*
	   * Let's first just see if we can open write and close a file
	   */
	  test_file = OFC_HANDLE_NULL ;
	  while (test_file == OFC_HANDLE_NULL)
	    {
	      test_file = 
		OfcCreateFile (TASTR("IPC:/test.test"),
			       OFC_GENERIC_READ | OFC_GENERIC_WRITE,
			       OFC_FILE_SHARE_READ,
			       OFC_NULL,
			       OFC_OPEN_ALWAYS,
			       OFC_FILE_ATTRIBUTE_NORMAL,
			       OFC_HANDLE_NULL) ;
	      if (test_file == OFC_HANDLE_NULL)
		{
		  ofc_printf ("Server Side not opened, waiting\n") ;
		  ofc_sleep (1000) ;
		}
	    }

	  quit = OFC_FALSE ;
	  while (!quit)
	    {
	      dwLen = 10 ;
	      ret = OfcReadFile (test_file, buf, dwLen, &dwLen, 
				  OFC_HANDLE_NULL) ;
	      if (buf[0] == '\0' || ret == OFC_FALSE)
		quit = OFC_TRUE ;
	      else
		{
		  buf[dwLen] = '\0' ;
		  ofc_printf ("%s", buf) ;
		}
	    }
	  
	  ofc_printf ("Closing Pipe Read File\n") ;
	  OfcCloseHandle (test_file) ;

	  ofc_thread_delete (hServerApp) ;
	}
      count++ ;
    }
  ofc_event_set(hNotify);
  return (0) ;
}

TEST_GROUP(pipe);

TEST_SETUP(pipe) {
    TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(pipe) {
    test_shutdown();
}

TEST(pipe, test_pipe) {
  hNotify = ofc_event_create(OFC_EVENT_AUTO);
  /*
   * Create the syncrhonous pipe Test thread
   */
  if (ofc_thread_create (&OfsFSPipeClientApp, 
			 OFC_THREAD_PIPE_TEST, OFC_THREAD_SINGLETON,
			 OFC_NULL,
			 OFC_THREAD_DETACH, OFC_HANDLE_NULL) == 
      OFC_HANDLE_NULL)
    ofc_printf ("Could not create OfsFSPipeClientApp\n") ;
  ofc_event_wait(hNotify);
  ofc_event_destroy(hNotify);
}	  

TEST_GROUP_RUNNER(pipe) {
    RUN_TEST_CASE(pipe, test_pipe);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(pipe);
}

int main(int argc, const char *argv[])
{
  if (argc >= 2) {
    if (ofc_strcmp(argv[1], "--config") == 0) {
      ofc_strncpy(config_path, argv[2], OFC_MAX_PATH);
    }
  }

  return UnityMain(argc, argv, runAllTests);
}
#endif

/* \} */
