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
#include "ofc/thread.h"
#include "test_file.h"

static OFC_INT test_startup(OFC_VOID) {
#if defined(INIT_ON_LOAD)
  volatile OFC_VOID *init = ofc_framework_init;
#else
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

TEST_GROUP(fs_linux);

TEST_SETUP(fs_linux) {
    TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(fs_linux) {
    test_shutdown();
}

TEST(fs_linux, test_fs_linux) {
    OFC_INT ret;
    ofc_thread_create_local_storage();
    ret = test_file(OFC_TEST_FS_LINUX_PATH);
    ofc_thread_destroy_local_storage();
    TEST_ASSERT_FALSE_MESSAGE(ret, "File Test Failed");
}

TEST_GROUP_RUNNER(fs_linux) {
    RUN_TEST_CASE(fs_linux, test_fs_linux);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(fs_linux);
}

int main(int argc, const char *argv[])
{
  return UnityMain(argc, argv, runAllTests);
}
#endif
