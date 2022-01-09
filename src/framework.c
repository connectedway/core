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

#include "ofc/heap.h"

#if defined(OFC_PERSIST)
static OFC_LPTSTR config_filename = OFC_NULL;
#endif

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
#if defined(OFC_PERSIST)
    if (config_filename != OFC_NULL) {
        ofc_free(config_filename);
        config_filename = OFC_NULL;
    }
#endif
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
#if defined(OFC_PERSIST)
    ofc_printf("Loading %S\n", filename);
    if (config_filename == OFC_NULL)
        config_filename = ofc_tstrdup(filename);
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

#if defined(SMB)
BLUE_UTIL_LIB BLUE_LPTSTR BlueFrameworkGetRootDir (BLUE_VOID)
{
  BLUE_LPTSTR tstrRootDir ;

  tstrRootDir = BlueHeapMalloc (BLUE_MAX_PATH * sizeof (BLUE_TCHAR)) ;
  BlueEnvGet (BLUE_ENV_ROOT, tstrRootDir, BLUE_MAX_PATH) ;
  return (tstrRootDir) ;
}

BLUE_UTIL_LIB BLUE_VOID BlueFrameworkFreeRootDir (BLUE_LPTSTR str)
{
  BlueHeapFree (str) ;
}

BLUE_VOID 
BlueFrameworkSetAuthenticationMode (BLUE_CONFIG_AUTH_MODE mode)
{
  BlueConfigSetAuthMode (mode) ;
}

BLUE_FRAMEWORK_EXPORTS *BlueFrameworkGetExports (BLUE_VOID)
{
  BLUE_FRAMEWORK_EXPORTS *exports ;
  BLUE_UINT16 export_count ;
  BLUE_INT i ;
  CONFIG_EXPORT configExport ;
  
  exports = BlueHeapMalloc (sizeof (BLUE_FRAMEWORK_EXPORTS)) ;
  if (exports != BLUE_NULL)
    {
      BlueConfigExportCount (&export_count) ;
      exports->numExports = export_count ;
      exports->exp = BlueHeapMalloc (sizeof (BLUE_FRAMEWORK_EXPORT) *
					exports->numExports) ;
      for (i = 0 ; i < exports->numExports ; i++)
	{
	  BlueConfigExport (i, &configExport) ;
	  exports->exp[i].exportType = configExport.type ;
	  exports->exp[i].share = BlueCcstr2tstr (configExport.share) ;
	  exports->exp[i].path = BlueCtstrdup (configExport.path) ;
	  exports->exp[i].description = 
	    BlueCcstr2tstr (configExport.comment) ;
	  exports->exp[i].native_fs =
	    BlueCcstr2tstr (configExport.native_fs) ;
	}
    }
  return (exports) ;
}
				 
BLUE_VOID BlueFrameworkFreeExports (BLUE_FRAMEWORK_EXPORTS *exports)
{
  BLUE_INT i ;

  for (i = 0 ; i < exports->numExports ; i++)
    {
      BlueHeapFree (exports->exp[i].share) ;
      BlueHeapFree (exports->exp[i].path) ;
      BlueHeapFree (exports->exp[i].description) ;
      BlueHeapFree (exports->exp[i].native_fs) ;
    }
  BlueHeapFree (exports->exp) ;
  BlueHeapFree (exports) ;
}

BLUE_VOID BlueFrameworkRemoveExport (BLUE_LPCTSTR tszExport)
{
  BlueConfigRemoveExport (tszExport) ;
}

BLUE_VOID BlueFrameworkAddExport (BLUE_FRAMEWORK_EXPORT *export) 
{
  CONFIG_EXPORT_PARAM configExport ;

  configExport.type = export->exportType ;
  configExport.share = BlueCtstr2cstr (export->share) ;
  configExport.path = export->path ;
  configExport.comment = BlueCtstr2cstr (export->description) ;
  configExport.native_fs = BlueCtstr2cstr (export->native_fs) ;

  BlueConfigAddExport (&configExport) ;

  BlueHeapFree (configExport.share) ;
  BlueHeapFree (configExport.comment) ;
  BlueHeapFree (configExport.native_fs) ;
}

BLUE_VOID BlueFrameworkAddProxyGateway (BLUE_FRAMEWORK_PROXY *proxy) 
{
  CONFIG_PROXYSERVER configProxy ;

  configProxy.path = proxy->path ;

  BlueConfigAddProxyServer (&configProxy) ;
}

BLUE_FRAMEWORK_REMOTES *BlueFrameworkGetRemotes (BLUE_VOID)
{
  BLUE_FRAMEWORK_REMOTES *remotes ;
#if defined(BLUE_PARAM_SMB_TOPOLOGY)
  BLUE_CTCHAR *name ;
  BLUE_CTCHAR *ip ;
  BLUE_UINT16 port ;
  BLUE_INT i ;
#endif
  
  remotes = BLUE_NULL ;
  remotes = BlueHeapMalloc (sizeof (BLUE_FRAMEWORK_REMOTES)) ;

#if defined(BLUE_PARAM_SMB_TOPOLOGY)
  if (remotes != BLUE_NULL)
    {
      remotes->numRemotes = 0 ;
      remotes->remote = BlueHeapMalloc (sizeof (BLUE_FRAMEWORK_REMOTE) *
					BLUE_PARAM_MAX_REMOTES) ;
      for (i = 0 ; i < BLUE_PARAM_MAX_REMOTES ; i++)
	{
	  SMBTopologyGetRemote (i, &name, &ip, &port) ;
	  if (name != BLUE_NULL)
	    {
	      remotes->remote[remotes->numRemotes].name = 
		BlueCtstrdup (name) ;
	      remotes->remote[remotes->numRemotes].ip = 
		BlueCtstrdup (ip) ;
	      remotes->remote[remotes->numRemotes].port = port ;
	      remotes->numRemotes++ ;
	    }
	}
    }
#endif
  return (remotes) ;
}
				 
BLUE_VOID BlueFrameworkFreeRemotes (BLUE_FRAMEWORK_REMOTES *remotes)
{
  BLUE_INT i ;

  if (remotes != BLUE_NULL)
    {
      for (i = 0 ; i < remotes->numRemotes ; i++)
	{
	  BlueHeapFree (remotes->remote[i].name) ;
	  BlueHeapFree (remotes->remote[i].ip) ;
	}
      BlueHeapFree (remotes->remote) ;
      BlueHeapFree (remotes) ;
    }
}

BLUE_VOID BlueFrameworkAddRemote (BLUE_FRAMEWORK_REMOTE *remote)
{
#if defined(BLUE_PARAM_SMB_TOPOLOGY)
  SMBTopologyUpdateServer (remote->name, BlueCtstrlen(remote->name),
			   remote->ip, remote->port) ;
#endif
}

BLUE_VOID BlueFrameworkRemoveRemote (BLUE_LPCTSTR name)
{
#if defined(BLUE_PARAM_SMB_TOPOLOGY)
  SMBTopologyRemoveRemote (name) ;
#endif
}

BLUE_BOOL BlueFrameworkGetServerEnabled (BLUE_VOID) 
{
  BLUE_UINT16 num_servers ;
  BLUE_INT ix ;
  BLUE_BOOL enabled ;
  BLUE_UINT16 port ;

  BlueConfigServerCount (&num_servers) ;

  enabled = BLUE_FALSE ;
  for (ix = 0 ; ix < num_servers && !enabled ; ix++)
    {
      BlueConfigServer (ix, &port) ;
      if ((port == 4445) || (port == 445))
	enabled = BLUE_TRUE ;
    }
  return (enabled) ;
}

BLUE_TCHAR *BlueFrameworkGetServerUsername (BLUE_VOID) 
{
  BLUE_LPCTSTR name ;
  BLUE_LPTSTR tstrName ;

  BlueConfigAuthUsername (&name) ;

  tstrName = BlueCtstrdup (name) ;

  return (tstrName) ;
}

BLUE_TCHAR *BlueFrameworkGetServerPassword (BLUE_VOID) 
{
  BLUE_LPCTSTR pass ;
  BLUE_LPTSTR tstrPass ;

  BlueConfigAuthServer (&pass) ;

  tstrPass = BlueCtstrdup (pass) ;

  return (tstrPass) ;
}

BLUE_VOID BlueFrameworkSetServerEnabled (BLUE_BOOL on) 
{
  BLUE_UINT16 num_servers ;
  BLUE_INT ix ;
  BLUE_INT jx ;
  BLUE_BOOL enabled_phone ;
  BLUE_BOOL enabled_computer ;
  BLUE_UINT16 port ;

  BlueConfigServerCount (&num_servers) ;

  enabled_phone = BLUE_FALSE ;
  enabled_computer = BLUE_FALSE ;

  for (ix = 0 ; ix < num_servers ; ix++)
    {
      BlueConfigServer (ix, &port) ;
      if (port == 4445)
	enabled_phone = BLUE_TRUE ;
      if (port == 445)
	enabled_computer = BLUE_TRUE ;
    }

  if (!enabled_phone && on)
    {
      /* enable it */
      /* easy, increase count and add to the end */
      BlueConfigSetServerCount (num_servers + 1) ;
      BlueConfigSetServer (num_servers, 4445) ;
      num_servers++ ;
      BlueConfigUpdate() ;
    }

  /* 
   * this is a problem if we can't open port 445 for some reason.
   * it will break the other server ports
   */
  if (!enabled_computer && on)
    {
      /* enable it */
      /* easy, increase count and add to the end */
      BlueConfigSetServerCount (num_servers + 1) ;
      BlueConfigSetServer (num_servers, 445) ;
      num_servers++ ;
      BlueConfigUpdate() ;
    }

  if ((enabled_phone || enabled_computer) && !on)
    {
      /* disable it */
      /* a little more difficult, we need to remove it from the list */
      for (ix = 0, jx = 0 ; ix < num_servers; )
	{
	  BlueConfigServer (ix, &port) ;
	  if (!(enabled_phone && port == 4445) &&
	      !(enabled_computer && port == 445))
	    {
	      BlueConfigSetServer (jx, port) ;
	      jx++ ;
	    }
	  ix++ ;
	}
      BlueConfigSetServerCount (jx) ;
      BlueConfigUpdate() ;
    }
}

BLUE_VOID BlueFrameworkSetServerUsername (BLUE_LPTSTR tstrName) 
{
  BlueConfigSetAuthUsername(tstrName) ;
}

BLUE_VOID BlueFrameworkSetServerPassword (BLUE_LPTSTR tstrPass) 
{
  BlueConfigSetAuthServer (tstrPass) ;
}

BLUE_INT BlueFrameworkGetMaxEvents(BLUE_VOID) 
{
  return (BlueConfigGetMaxEvents()) ;
}

BLUE_LONG BlueFrameworkGetTimePeriod(BLUE_VOID)
{
  return (BlueConfigGetTimePeriod()) ;
}

BLUE_VOID BlueFrameworkSetMaxEvents(BLUE_INT maxEvents)
{
  BlueConfigSetMaxEvents(maxEvents) ;
}

BLUE_VOID BlueFrameworkSetTimePeriod(BLUE_LONG timePeriod)
{
  BlueConfigSetTimePeriod(timePeriod) ;
}

#endif  





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

OFC_VOID ofc_framework_add_map(OFC_FRAMEWORK_MAP *map) {
    OFC_PATH *path;

    path = ofc_path_createW(map->path);
    ofc_path_add_mapW(map->prefix, map->desc, path, map->type, map->thumbnail);
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

static OFC_INT wifi_ip = 0;

OFC_VOID ofc_framework_set_wifi_ip(OFC_INT ip) {
    wifi_ip = OFC_NET_NTOL (&ip, 0);
}

OFC_INT ofc_framework_get_wifi_ip(OFC_VOID) {
    return (wifi_ip);
}
