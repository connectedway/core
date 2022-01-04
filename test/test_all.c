/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#include "unity_fixture.h"

static void runAllTests(void) {
    RUN_TEST_GROUP(timer);
    RUN_TEST_GROUP(event);
    RUN_TEST_GROUP(waitq);
    RUN_TEST_GROUP(thread);
    RUN_TEST_GROUP(dg);
    RUN_TEST_GROUP(stream);
    RUN_TEST_GROUP(path);
#if defined(OFC_FS_DARWIN)
    RUN_TEST_GROUP(fs_darwin);
#endif
#if defined(OFC_FS_WINDOWS)
    RUN_TEST_GROUP(fs_windows);
#endif
#if defined(OFC_FS_LINUX)
    RUN_TEST_GROUP(fs_linux);
#endif
}

int main(int argc, const char *argv[]) {
    return UnityMain(argc, argv, runAllTests);
}
