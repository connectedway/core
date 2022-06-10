/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/framework.h"
#include "ofc/core.h"
#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/libc.h"
#include "ofc/sched.h"
#include "ofc/thread.h"
#include "ofc/persist.h"
#include "ofc/net.h"
#include "ofc/net_internal.h"
#include "ofc/env.h"
#include "ofc/version.h"
#if defined(OFC_NETMON)
#include "ofc/netmon.h"
#endif

#include "ofc/heap.h"

static OFC_LPTSTR config_filename = OFC_NULL;

/**
 * \defgroup init Initialization
 * \ingroup Applications
 */

/** \{ */

OFC_CORE_LIB OFC_VOID
ofc_framework_init(OFC_VOID) {
    /*
     * Load Open Files if not done as part of library load
     */
#if !defined(INIT_ON_LOAD)
    ofc_core_load();
#endif

    /*
     * Print out the banner
     */
    ofc_printf("OpenFiles (%s) %d.%d %s\n",
               OFC_SHARE_VARIANT,
               OFC_SHARE_MAJOR, OFC_SHARE_MINOR,
               OFC_SHARE_TAG);
}

OFC_CORE_LIB OFC_VOID
ofc_framework_destroy(OFC_VOID) {
    if (config_filename != OFC_NULL) {
        ofc_free(config_filename);
        config_filename = OFC_NULL;
    }
#if !defined(INIT_ON_LOAD)
    ofc_core_unload() ;
#endif
}

OFC_CORE_LIB OFC_VOID
ofc_framework_startup(OFC_VOID) {
    OFC_HANDLE hScheduler;

    hScheduler = ofc_sched_create();
    ofc_framework_startup_ev(hScheduler, OFC_HANDLE_NULL);
}

OFC_CORE_LIB OFC_VOID
ofc_framework_startup_ev(OFC_HANDLE hScheduler, OFC_HANDLE hEvent) {
    /*
     * Configuration is complete.  Start up the stack
     */
#if defined(OFC_NETMON)
    ofc_netmon_startup (hScheduler, OFC_HANDLE_NULL) ;
#endif

}

OFC_CORE_LIB OFC_VOID
ofc_framework_shutdown(OFC_VOID) {
#if defined(OFC_NETMON)
#if 0
    ofc_netmon_shutdown (hScheduler, OFC_HANDLE_NULL) ;
#endif
#endif
}

OFC_CORE_LIB OFC_VOID ofc_framework_load(OFC_LPCTSTR filename) {
    if (config_filename == OFC_NULL)
        config_filename = ofc_tstrdup(filename);

#if defined(OFC_PERSIST)
    ofc_printf("Loading %S\n", filename);
    ofc_persist_load(filename);
#else
    ofc_printf("Configuration Files Disabled\n") ;
#endif
}

OFC_CORE_LIB OFC_VOID ofc_framework_save(OFC_LPCTSTR filename) {
#if defined(OFC_PERSIST)
    if (filename == OFC_NULL)
        filename = config_filename;
    if (filename != OFC_NULL) {
        ofc_printf("Saving %S\n", filename);
        ofc_persist_save(filename);
    }
#else
    ofc_printf("Configuration Files Disabled\n") ;
#endif
}

OFC_CORE_LIB OFC_BOOL ofc_get_config_dir(OFC_TCHAR *config_dir, OFC_SIZET len)
{
  OFC_BOOL ret;
  ret = OFC_FALSE;
  if (config_filename != OFC_NULL)
    {
      OFC_PATH *path;
      OFC_SIZET rem;
      OFC_SIZET size;
      OFC_TCHAR *dir;
      
      path = ofc_path_createW(config_filename);
      ofc_path_free_filename(path);
      rem = 128;
      dir = config_dir;
      ofc_path_printW(path, &dir, &rem);
      ofc_path_delete(path);
      ret = OFC_TRUE;
    }
  return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_framework_set_host_name(OFC_LPCTSTR name, OFC_LPCTSTR workgroup,
                            OFC_LPCTSTR desc) {
    ofc_persist_set_node_name(name, workgroup, desc);
}

OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_host_name(OFC_VOID) {
    OFC_LPCTSTR name;
    OFC_LPCTSTR workgroup;
    OFC_LPCTSTR desc;
    OFC_LPTSTR tstrName;

    ofc_persist_node_name(&name, &workgroup, &desc);

    tstrName = ofc_tstrdup(name);

    return (tstrName);
}

OFC_CORE_LIB OFC_VOID ofc_framework_free_host_name(OFC_LPTSTR str) {
    ofc_free(str);
}

OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_workgroup(OFC_VOID) {
    OFC_LPCTSTR name;
    OFC_LPCTSTR workgroup;
    OFC_LPCTSTR desc;
    OFC_LPTSTR tstrWorkgroup;

    ofc_persist_node_name(&name, &workgroup, &desc);

    tstrWorkgroup = ofc_tstrdup(workgroup);

    return (tstrWorkgroup);
}

OFC_CORE_LIB OFC_VOID ofc_framework_free_workgroup(OFC_LPTSTR str) {
    ofc_free(str);
}

OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_description(OFC_VOID) {
    OFC_LPCTSTR name;
    OFC_LPCTSTR workgroup;
    OFC_LPCTSTR desc;
    OFC_LPTSTR tstrDesc;

    ofc_persist_node_name(&name, &workgroup, &desc);

    tstrDesc = ofc_tstrdup(desc);

    return (tstrDesc);
}

OFC_CORE_LIB OFC_VOID ofc_framework_free_description(OFC_LPTSTR str) {
    ofc_free(str);
}

OFC_VOID ofc_framework_set_uuid(const OFC_CHAR *cuuid) {
    OFC_UUID uuid;

    ofc_atouuid(cuuid, uuid);
    ofc_persist_set_uuid(&uuid);
}

OFC_CHAR *ofc_framework_get_uuid(OFC_VOID) {
    OFC_UUID uuid;
    OFC_CHAR *cuuid;

    ofc_persist_uuid(&uuid);

    cuuid = ofc_malloc(UUID_STR_LEN + 1);
    ofc_uuidtoa(uuid, cuuid);
    return (cuuid);
}

OFC_CORE_LIB OFC_VOID ofc_framework_free_uuid(OFC_LPSTR str) {
    ofc_free(str);
}

OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_root_dir(OFC_VOID) {
    OFC_LPTSTR tstrRootDir;

    tstrRootDir = ofc_malloc(OFC_MAX_PATH * sizeof(OFC_TCHAR));
    ofc_env_get(OFC_ENV_ROOT, tstrRootDir, OFC_MAX_PATH);
    return (tstrRootDir);
}

OFC_CORE_LIB OFC_VOID ofc_framework_free_root_dir(OFC_LPTSTR str) {
    ofc_free(str);
}

OFC_VOID ofc_framework_set_interface_discovery(OFC_BOOL on) {
    OFC_CONFIG_ICONFIG_TYPE itype;

    itype = OFC_CONFIG_ICONFIG_MANUAL;
    if (on)
        itype = OFC_CONFIG_ICONFIG_AUTO;

    ofc_persist_set_interface_type(itype);
}

OFC_BOOL ofc_framework_get_interface_discovery(OFC_VOID) {
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (ofc_persist_interface_config() == OFC_CONFIG_ICONFIG_AUTO)
        ret = OFC_TRUE;
    return (ret);
}

OFC_VOID ofc_framework_add_interface(OFC_FRAMEWORK_INTERFACE *iface) {
    OFC_INT old_count;
    OFC_INT new_count;
    OFC_CHAR *cstr;

    cstr = ofc_strdup(iface->lmb);

    old_count = ofc_persist_interface_count();
    new_count = old_count + 1;
    ofc_persist_set_interface_count(new_count);

    ofc_persist_set_interface_config(old_count,
                                     iface->netBiosMode,
                                     &iface->ip,
                                     &iface->bcast,
                                     &iface->mask,
                                     cstr,
                                     iface->wins.num_wins,
                                     iface->wins.winsaddr);
    ofc_free(cstr);
}

OFC_VOID ofc_framework_remove_interface(OFC_IPADDR *ip) {
    ofc_persist_remove_interface_config(ip);
}

OFC_FRAMEWORK_INTERFACES *ofc_framework_get_interfaces(OFC_VOID) {
    OFC_FRAMEWORK_INTERFACES *ifaces;
    OFC_INT i;


    ifaces = ofc_malloc(sizeof(OFC_FRAMEWORK_INTERFACES));
    if (ifaces != OFC_NULL) {
        ifaces->num_interfaces = ofc_persist_interface_count();
        ifaces->iface = ofc_malloc(sizeof(OFC_FRAMEWORK_INTERFACE) *
                                   ifaces->num_interfaces);
        for (i = 0; i < ifaces->num_interfaces; i++) {
            ifaces->iface[i].netBiosMode =
                    ofc_persist_interface_mode(i, &ifaces->iface[i].wins.num_wins,
                                               &ifaces->iface[i].wins.winsaddr);
            ofc_persist_interface_addr(i, &ifaces->iface[i].ip,
                                       &ifaces->iface[i].bcast,
                                       &ifaces->iface[i].mask);
            ofc_persist_local_master(i, &ifaces->iface[i].lmb);
        }
    }
    return (ifaces);
}

OFC_VOID ofc_framework_free_interfaces(OFC_FRAMEWORK_INTERFACES *ifaces) {
    OFC_INT i;

    for (i = 0; i < ifaces->num_interfaces; i++) {
        /*
         * lmb is a pointer to config space.  Do not free
         */
        ofc_free(ifaces->iface[i].wins.winsaddr);
    }
    ofc_free(ifaces->iface);
    ofc_free(ifaces);
}

OFC_FRAMEWORK_MAPS *ofc_framework_get_maps(OFC_VOID) {
    OFC_FRAMEWORK_MAPS *maps;
    OFC_LPCTSTR prefix;
    OFC_LPCTSTR desc;
    OFC_BOOL thumbnail;
    OFC_PATH *path;
    OFC_INT i;
    OFC_SIZET len;
    OFC_SIZET rem;
    OFC_LPTSTR ptr;

    maps = ofc_malloc(sizeof(OFC_FRAMEWORK_MAPS));
    if (maps != OFC_NULL) {
        maps->numMaps = 0;
        maps->map = ofc_malloc(sizeof(OFC_FRAMEWORK_MAP) *
                               OFC_MAX_MAPS);
        for (i = 0; i < OFC_MAX_MAPS; i++) {
            ofc_path_get_mapW(i, &prefix, &desc, &path, &thumbnail);
            if (prefix != OFC_NULL) {
                maps->map[maps->numMaps].prefix = ofc_tstrdup(prefix);
                maps->map[maps->numMaps].desc = ofc_tstrdup(desc);
                maps->map[maps->numMaps].type = ofc_path_type(path);
                maps->map[maps->numMaps].thumbnail = thumbnail;
                rem = 0;
                len = ofc_path_printW(path, OFC_NULL, &rem);

                maps->map[maps->numMaps].path =
                        ofc_malloc(sizeof(OFC_TCHAR) * (len + 1));

                rem = len + 1;
                ptr = maps->map[maps->numMaps].path;
                ofc_path_printW(path, &ptr, &rem);
                maps->numMaps++;
            }
        }
    }
    return (maps);
}

OFC_VOID ofc_framework_free_maps(OFC_FRAMEWORK_MAPS *maps) {
    OFC_INT i;

    for (i = 0; i < maps->numMaps; i++) {
        ofc_free(maps->map[i].prefix);
        ofc_free(maps->map[i].path);
        ofc_free(maps->map[i].desc);
    }
    ofc_free(maps->map);
    ofc_free(maps);
}

OFC_BOOL ofc_framework_add_map(OFC_FRAMEWORK_MAP *map) {
    OFC_PATH *path;
    OFC_BOOL ret;

    path = ofc_path_createW(map->path);
    ret = ofc_path_add_mapW(map->prefix, map->desc, path, map->type, map->thumbnail);
    return (ret);
}

OFC_VOID ofc_framework_remove_map(OFC_LPCTSTR tszPrefix) {
    ofc_path_delete_mapW(tszPrefix);
}

OFC_VOID ofc_framework_update(OFC_VOID) {
    ofc_persist_update();
}

OFC_VOID ofc_framework_dump_heap(OFC_VOID) {
    ofc_heap_dump();
}

OFC_VOID ofc_framework_stats_heap(OFC_VOID) {
    ofc_heap_dump_stats();
}

static OFC_INT wifi_ip = 0;

OFC_VOID ofc_framework_set_wifi_ip(OFC_INT ip) {
    wifi_ip = OFC_NET_NTOL (&ip, 0);
}

OFC_INT ofc_framework_get_wifi_ip(OFC_VOID) {
    return (wifi_ip);
}
