/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */

#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/heap.h"
#include "ofc/env.h"
#include "ofc/framework.h"
#include "ofc/persist.h"
#include "ofc/sched.h"
#include "ofc/event.h"

OFC_HANDLE hScheduler;
OFC_HANDLE hDone;

OFC_CHAR config_path[OFC_MAX_PATH+1] = {0};

#if !defined(INIT_ON_LOAD)
#if defined(OFC_PERSIST)
static OFC_INT test_startup_persist(OFC_VOID) {
    OFC_INT ret = 0;
    OFC_TCHAR *tpath = OFC_NULL;

    if (config_path[0] != '\0')
      {
	tpath = ofc_cstr2tstr(config_path);
      }
    else
      {
	tpath = ofc_malloc((OFC_MAX_PATH+1)*sizeof(OFC_TCHAR));
	if (ofc_env_get(OFC_ENV_HOME, tpath, OFC_MAX_PATH) == OFC_FALSE)
	  {
#if defined(__linux__)
	    ofc_free(tpath);
	    tpath = OFC_NULL;
#else
	    ret = -1;
#endif
	  }
      }

    if (ret == 0)
      {
	ofc_framework_load(tpath);
      }

    if (tpath != OFC_NULL)
      ofc_free(tpath);

    return (ret);
}
#endif

#if !defined(OFC_PERSIST)
static OFC_INT test_startup_default(OFC_VOID) {
    static OFC_UUID uuid =
            {
                    0x04, 0x5d, 0x88, 0x8a, 0xeb, 0x1c, 0xc9, 0x11,
                    0x9f, 0xe8, 0x08, 0x00, 0x2b, 0x10, 0x48, 0x60
            };

    ofc_persist_default();
    ofc_persist_set_interface_type(OFC_CONFIG_ICONFIG_AUTO);
    ofc_persist_set_node_name(TSTR("localhost"), TSTR("WORKGROUP"),
                              TSTR("OpenFiles Unit Test"));
    ofc_persist_set_uuid(&uuid);
    return (0);
}
#endif
#endif

OFC_INT test_startup(OFC_VOID) {
    OFC_INT ret;
    ret = 0;

#if !defined(INIT_ON_LOAD)
    ofc_framework_init();
#if defined(OFC_PERSIST)
    ret = test_startup_persist();
#else
    ret = test_startup_default();
#endif
#endif
    hScheduler = ofc_sched_create();
    hDone = ofc_event_create(OFC_EVENT_AUTO);

    return (ret);
}

OFC_VOID test_shutdown(OFC_VOID) {
    ofc_event_destroy(hDone);
    ofc_sched_quit(hScheduler);
#if !defined(INIT_ON_LOAD)
    ofc_framework_shutdown();
    ofc_framework_destroy();
#endif
}

