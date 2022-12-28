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

static OFC_INT test_startup(OFC_VOID) {
#if !defined(INIT_ON_LOAD)
    ofc_framework_init();
#endif
    return (0);
}

static OFC_VOID test_shutdown(OFC_VOID) {
#if !defined(INIT_ON_LOAD)
    ofc_framework_shutdown();
    ofc_framework_destroy();
#endif
}

TEST_GROUP(fs_windows);

TEST_SETUP(fs_windows) {
    TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(fs_windows) {
    test_shutdown();
}

TEST(fs_windows, test_fs_windows) {
    OFC_INT ret;
    ret = test_file(OFC_TEST_FS_WINDOWS_PATH);
    TEST_ASSERT_FALSE_MESSAGE(ret, "File Test Failed");
}

TEST_GROUP_RUNNER(fs_windows) {
    RUN_TEST_CASE(fs_windows, test_fs_windows);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(fs_windows);
}

int main(int argc, const char *argv[])
{
  return UnityMain(argc, argv, runAllTests);
}
#endif
