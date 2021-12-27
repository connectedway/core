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
#include "test_file.h"

static BLUE_INT test_startup(BLUE_VOID)
{
  BlueFrameworkInit();
  return(0);
}

static BLUE_VOID test_shutdown(BLUE_VOID)
{
  BlueFrameworkShutdown();
  BlueFrameworkDestroy();
}

TEST_GROUP(fs_darwin);

TEST_SETUP(fs_darwin)
{
  TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(fs_darwin)
{
  test_shutdown();
}  

TEST(fs_darwin, test_fs_darwin)
{
  BLUE_INT ret ;
  ret = test_file(OFC_TEST_FS_DARWIN_PATH);
  TEST_ASSERT_FALSE_MESSAGE(ret, "File Test Failed");
}	  

TEST_GROUP_RUNNER(fs_darwin)
{
  RUN_TEST_CASE(fs_darwin, test_fs_darwin);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(fs_darwin);
}

int main(int argc, const char *argv[])
{
  return UnityMain(argc, argv, runAllTests);
}
#endif
