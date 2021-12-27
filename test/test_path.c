/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#include "unity.h"
#include "unity_fixture.h"

#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/path.h"
#include "ofc/libc.h"
#include "ofc/heap.h"
#include "ofc/framework.h"

BLUE_INT test_startup(BLUE_VOID)
{
  BlueFrameworkInit();
  return(0);
}

BLUE_VOID test_shutdown(BLUE_VOID)
{
  BlueFrameworkShutdown();
  BlueFrameworkDestroy();
}

TEST_GROUP(path);

TEST_SETUP(path)
{
  TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(path)
{
  test_shutdown();
}  

TEST(path, test_path)
{
    BLUE_LPTSTR tfilename ;
    BLUE_LPSTR cfilename ;
    BLUE_LPTSTR tcursor ;
    BLUE_LPSTR ccursor ;
    BLUE_SIZET rem ;
    BLUE_SIZET trem ;
    BLUE_SIZET len ;

    rem = 20 ;
    trem = rem ;
    tfilename = BlueHeapMalloc ((trem+1) * sizeof (BLUE_TCHAR)) ;
    tcursor = tfilename ;

    len = BluePathMakeURLW (&tcursor, &trem,
			    TSTR("COYOTE"),
			    TSTR("ROAD@RUNNER"),
			    TSTR("ACME"),
			    TSTR("BOOM"),
			    TSTR("DESERT"),
			    TSTR("dir1\\dir2\\dir3"),
			    TSTR("PAIN")) ;
    tfilename[rem] = TCHAR_EOS ;

    BlueCprintf ("Truncated Path is %S\n", tfilename) ;
    BlueHeapFree (tfilename) ;

    rem = len ;
    trem = rem ;
    tfilename = BlueHeapMalloc ((trem+1) * sizeof (BLUE_TCHAR)) ;
    tcursor = tfilename ;

    len = BluePathMakeURLW (&tcursor, &trem,
			    TSTR("COYOTE"),
			    TSTR("ROAD@RUNNER"),
			    TSTR("ACME"),
			    TSTR("BOOM"),
			    TSTR("DESERT"),
			    TSTR("dir1\\dir2\\dir3"),
			    TSTR("PAIN")) ;
    tfilename[rem] = TCHAR_EOS ;

    BlueCprintf ("Expanded Path is %S\n", tfilename) ;
    BlueHeapFree (tfilename) ;

    rem = 20 ;
    trem = rem ;
    cfilename = BlueHeapMalloc ((trem+1) * sizeof (BLUE_CHAR)) ;
    ccursor = cfilename ;

    len = BluePathMakeURLA (&ccursor, &trem,
			    "COYOTE",
			    "ROAD@RUNNER",
			    "ACME",
			    "BOOM",
			    "DESERT",
			    "dir1\\dir2\\dir3",
			    "PAIN") ;
    cfilename[rem] = '\0' ;

    BlueCprintf ("Truncated Path is %s\n", cfilename) ;
    BlueHeapFree (cfilename) ;

    rem = len ;
    trem = rem ;
    cfilename = BlueHeapMalloc ((trem+1) * sizeof (BLUE_CHAR)) ;
    ccursor = cfilename ;

    len = BluePathMakeURLA (&ccursor, &trem,
			    "COYOTE",
			    "ROAD@RUNNER",
			    "ACME",
			    "BOOM",
			    "DESERT",
			    "dir1\\dir2\\dir3",
			    "PAIN") ;
    cfilename[rem] = '\0' ;

    BlueCprintf ("Expanded Path is %s\n", cfilename) ;

    BlueHeapFree (cfilename) ;
}	  

TEST_GROUP_RUNNER(path)
{
  RUN_TEST_CASE(path, test_path);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(path);
}

int main(int argc, const char *argv[])
{
  return UnityMain(argc, argv, runAllTests);
}
#endif
