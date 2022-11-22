/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#include "unity_fixture.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/config.h"
#include "ofc/file.h"

extern OFC_CHAR config_path[OFC_MAX_PATH+1];

static void runAllTests(void) {
    RUN_TEST_GROUP(timer);
    RUN_TEST_GROUP(event);
    RUN_TEST_GROUP(waitq);
    RUN_TEST_GROUP(thread);
    RUN_TEST_GROUP(dg);
    RUN_TEST_GROUP(stream);
    RUN_TEST_GROUP(path);
    RUN_TEST_GROUP(iovec);
#if defined(OFC_FS_DARWIN)
    RUN_TEST_GROUP(fs_darwin);
#endif
#if defined(OFC_FS_WIN32)
    RUN_TEST_GROUP(fs_windows);
#endif
#if defined(OFC_FS_LINUX)
    RUN_TEST_GROUP(fs_linux);
#endif
#if defined(OFC_FS_ANDROID)
    RUN_TEST_GROUP(fs_android);
#endif
#if defined(OFC_PERSIST)
    RUN_TEST_GROUP(subpersist);
#endif
}

int main(int argc, const char *argv[]) {
  if (argc >= 2) {
    if (ofc_strcmp(argv[1], "--config") == 0) {
      ofc_strncpy(config_path, argv[2], OFC_MAX_PATH);
    }
  }

    return UnityMain(argc, argv, runAllTests);
}
