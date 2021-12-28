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
#include "ofc/env.h"
#include "ofc/version.h"

#include "ofc/heap.h"

#if defined(OFC_PERSIST)
static OFC_LPTSTR config_filename = OFC_NULL ;
#endif

/**
 * \defgroup BlueInit Initialization
 * \ingroup Applications
 */

/** \{ */

OFC_CORE_LIB OFC_VOID
ofc_framework_init (OFC_VOID)
{
  /*
   * Load Blue Share if not done as part of library load
   */
#if !defined(INIT_ON_LOAD)
  of_core_load();
#endif

  /*
   * Print out the banner
   */
  BlueCprintf ("OpenFiles (%s) %d.%d %s\n", 
	       OFC_SHARE_VARIANT,
	       OFC_SHARE_MAJOR, OFC_SHARE_MINOR,
	       OFC_SHARE_TAG) ;
}

OFC_CORE_LIB OFC_VOID
ofc_framework_destroy (OFC_VOID)
{
  /*
   * Load Blue Share if not done as part of library load
   */
#if defined(OFC_PERSIST)
  if (config_filename != OFC_NULL)
    {
      BlueHeapFree(config_filename);
      config_filename = OFC_NULL;
    }
#endif
#if !defined(INIT_ON_LOAD)
  of_core_unload() ;
#endif
}

OFC_CORE_LIB OFC_VOID
ofc_framework_startup (OFC_VOID)
{
  OFC_HANDLE hScheduler ;

  hScheduler = BlueSchedCreate() ;
    ofc_framework_startup_ev(hScheduler, OFC_HANDLE_NULL);
}

OFC_CORE_LIB OFC_VOID
ofc_framework_startup_ev (OFC_HANDLE hScheduler, OFC_HANDLE hEvent)
{
  /*
   * Configuration is complete.  Start up the stack
   */
#if defined(OFC_NETMON)
  BlueNetMonStartup (hScheduler, BLUE_HANDLE_NULL) ;
#endif

}

OFC_CORE_LIB OFC_VOID
ofc_framework_shutdown (OFC_VOID)
{
#if defined(OFC_NETMON)
#if 0
  BlueNetMonShutdown (hScheduler, BLUE_HANDLE_NULL) ;
#endif
#endif
}

OFC_CORE_LIB OFC_VOID ofc_framework_load(OFC_LPCTSTR filename)
{
#if defined(OFC_PERSIST)
  BlueCprintf ("Loading %S\n", filename) ;
  if (config_filename == OFC_NULL)
    config_filename = BlueCtstrdup (filename) ;
  BlueConfigLoad (filename) ;
#else
  BlueCprintf ("Configuration Files Disabled\n") ;
#endif
}

OFC_CORE_LIB OFC_VOID ofc_framework_save (OFC_LPCTSTR filename)
{
#if defined(OFC_PERSIST)
  if (filename == OFC_NULL)
    filename = config_filename ;
  if (filename != OFC_NULL)
    {
      BlueCprintf ("Saving %S\n", filename) ;
      BlueConfigSave (filename) ;
    }
#else
  BlueCprintf ("Configuration Files Disabled\n") ;
#endif
}

OFC_CORE_LIB OFC_VOID
ofc_framework_set_host_name (OFC_LPCTSTR name, OFC_LPCTSTR workgroup,
                             OFC_LPCTSTR desc)
{
  BlueConfigSetNodeName (name, workgroup, desc) ;
}

OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_host_name (OFC_VOID)
{
  OFC_LPCTSTR name ;
  OFC_LPCTSTR workgroup ;
  OFC_LPCTSTR desc ;
  OFC_LPTSTR tstrName ;

  BlueConfigNodeName (&name, &workgroup, &desc) ;

  tstrName = BlueCtstrdup (name) ;

  return (tstrName) ;
}

OFC_CORE_LIB OFC_VOID ofc_framework_free_host_name (OFC_LPTSTR str)
{
  BlueHeapFree (str) ;
}

OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_workgroup (OFC_VOID)
{
  OFC_LPCTSTR name ;
  OFC_LPCTSTR workgroup ;
  OFC_LPCTSTR desc ;
  OFC_LPTSTR tstrWorkgroup ;

  BlueConfigNodeName (&name, &workgroup, &desc) ;

  tstrWorkgroup = BlueCtstrdup (workgroup) ;

  return (tstrWorkgroup) ;
}

OFC_CORE_LIB OFC_VOID ofc_framework_free_workgroup (OFC_LPTSTR str)
{
  BlueHeapFree (str) ;
}

OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_description (OFC_VOID)
{
  OFC_LPCTSTR name ;
  OFC_LPCTSTR workgroup ;
  OFC_LPCTSTR desc ;
  OFC_LPTSTR tstrDesc ;

  BlueConfigNodeName (&name, &workgroup, &desc) ;

  tstrDesc = BlueCtstrdup (desc) ;

  return (tstrDesc) ;
}

OFC_CORE_LIB OFC_VOID ofc_framework_free_description (OFC_LPTSTR str)
{
  BlueHeapFree (str) ;
}

OFC_VOID ofc_framework_set_uuid (const OFC_CHAR *cuuid)
{
  OFC_UUID uuid ;

  BlueCatouuid (cuuid, uuid) ;
  BlueConfigSetUUID (&uuid) ;
}

OFC_CHAR *ofc_framework_get_uuid (OFC_VOID)
{
  OFC_UUID uuid ;
  OFC_CHAR *cuuid ;

  BlueConfigUUID (&uuid) ;

  cuuid = BlueHeapMalloc (UUID_STR_LEN+1) ;
  BlueCuuidtoa (uuid, cuuid) ;
  return (cuuid) ;
}

OFC_CORE_LIB OFC_VOID ofc_framework_free_uuid (OFC_LPSTR str)
{
  BlueHeapFree (str) ;
}

OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_root_dir (OFC_VOID)
{
  OFC_LPTSTR tstrRootDir ;

  tstrRootDir = BlueHeapMalloc (OFC_MAX_PATH * sizeof (OFC_TCHAR)) ;
  ofc_env_get (OFC_ENV_ROOT, tstrRootDir, OFC_MAX_PATH) ;
  return (tstrRootDir) ;
}

OFC_CORE_LIB OFC_VOID ofc_framework_free_root_dir (OFC_LPTSTR str)
{
  BlueHeapFree (str) ;
}

OFC_VOID ofc_framework_set_interface_discovery (OFC_BOOL on)
{
  BLUE_CONFIG_ICONFIG_TYPE itype ;

  itype = BLUE_CONFIG_ICONFIG_MANUAL ;
  if (on)
    itype = BLUE_CONFIG_ICONFIG_AUTO ;

  BlueConfigSetInterfaceType (itype) ;
}

OFC_BOOL ofc_framework_get_interface_discovery (OFC_VOID)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (BlueConfigInterfaceConfig() == BLUE_CONFIG_ICONFIG_AUTO)
    ret = OFC_TRUE ;
  return (ret) ;
}

OFC_VOID ofc_framework_add_interface (OFC_FRAMEWORK_INTERFACE *iface)
{
  OFC_INT old_count ;
  OFC_INT new_count ;
  OFC_CHAR *cstr ;

  cstr = BlueCstrdup (iface->lmb) ;

  old_count = BlueConfigInterfaceCount() ;
  new_count = old_count + 1 ;
  BlueConfigSetInterfaceCount(new_count) ;

  BlueConfigSetInterfaceConfig (old_count,
				iface->netBiosMode,
				&iface->ip,
				&iface->bcast,
				&iface->mask,
				cstr,
				iface->wins.num_wins,
				iface->wins.winsaddr) ;
  BlueHeapFree (cstr) ;
}

OFC_VOID ofc_framework_remove_interface (BLUE_IPADDR *ip)
{
  BlueConfigRemoveInterfaceConfig (ip) ;
}

OFC_FRAMEWORK_INTERFACES *ofc_framework_get_interfaces (OFC_VOID)
{
  OFC_FRAMEWORK_INTERFACES *ifaces ;
  OFC_INT i ;
  

  ifaces = BlueHeapMalloc (sizeof (OFC_FRAMEWORK_INTERFACES)) ;
  if (ifaces != OFC_NULL)
    {
      ifaces->num_interfaces = BlueConfigInterfaceCount() ;
      ifaces->iface = BlueHeapMalloc (sizeof (OFC_FRAMEWORK_INTERFACE) *
                                      ifaces->num_interfaces) ;
      for (i = 0 ; i < ifaces->num_interfaces ; i++)
	{
	  ifaces->iface[i].netBiosMode = 
	    BlueConfigInterfaceMode (i, &ifaces->iface[i].wins.num_wins,
				     &ifaces->iface[i].wins.winsaddr) ;
	  BlueConfigInterfaceAddr (i, &ifaces->iface[i].ip,
				   &ifaces->iface[i].bcast,
				   &ifaces->iface[i].mask) ;
	  BlueConfigLocalMaster (i, &ifaces->iface[i].lmb) ;
	}
    }
  return (ifaces) ;
}
				 
OFC_VOID ofc_framework_free_interfaces (OFC_FRAMEWORK_INTERFACES *ifaces)
{
  OFC_INT i ;

  for (i = 0 ; i < ifaces->num_interfaces ; i++)
    {
      /*
       * lmb is a pointer to config space.  Do not free
       */
      BlueHeapFree (ifaces->iface[i].wins.winsaddr) ;
    }
  BlueHeapFree (ifaces->iface) ;
  BlueHeapFree (ifaces) ;
}

OFC_FRAMEWORK_MAPS *ofc_framework_get_maps (OFC_VOID)
{
  OFC_FRAMEWORK_MAPS *maps ;
  OFC_LPCTSTR prefix ;
  OFC_LPCTSTR desc ;
  OFC_BOOL thumbnail ;
  BLUE_PATH *path ;
  OFC_INT i ;
  OFC_SIZET len ;
  OFC_SIZET rem ;
  OFC_LPTSTR ptr ;
  
  maps = BlueHeapMalloc (sizeof (OFC_FRAMEWORK_MAPS)) ;
  if (maps != OFC_NULL)
    {
      maps->numMaps = 0 ;
      maps->map = BlueHeapMalloc (sizeof (OFC_FRAMEWORK_MAP) *
                                  OFC_MAX_MAPS) ;
      for (i = 0 ; i < OFC_MAX_MAPS ; i++)
	{
	  BluePathGetMapW (i, &prefix, &desc, &path, &thumbnail) ;
	  if (prefix != OFC_NULL)
	    {
	      maps->map[maps->numMaps].prefix = BlueCtstrdup (prefix) ;
	      maps->map[maps->numMaps].desc = BlueCtstrdup (desc) ;
	      maps->map[maps->numMaps].type = BluePathType(path) ;
	      maps->map[maps->numMaps].thumbnail = thumbnail ;
	      rem = 0 ;
	      len = BluePathPrintW (path, OFC_NULL, &rem) ;

	      maps->map[maps->numMaps].path  = 
		BlueHeapMalloc (sizeof (OFC_TCHAR) * (len + 1)) ;
	      
	      rem = len+1 ;
	      ptr = maps->map[maps->numMaps].path ;
	      BluePathPrintW (path, &ptr, &rem) ;
	      maps->numMaps++ ;
	    }
	}
    }
  return (maps) ;
}
				 
OFC_VOID ofc_framework_free_maps (OFC_FRAMEWORK_MAPS *maps)
{
  OFC_INT i ;

  for (i = 0 ; i < maps->numMaps ; i++)
    {
      BlueHeapFree (maps->map[i].prefix) ;
      BlueHeapFree (maps->map[i].path) ;
      BlueHeapFree (maps->map[i].desc) ;
    }
  BlueHeapFree (maps->map) ;
  BlueHeapFree (maps) ;
}

OFC_VOID ofc_framework_add_map (OFC_FRAMEWORK_MAP *map)
{
  BLUE_PATH *path ;

  path = BluePathCreateW (map->path) ;
  BluePathAddMapW (map->prefix, map->desc, path, map->type, map->thumbnail) ;
}

OFC_VOID BlueFrameworkRemoveMap (OFC_LPCTSTR tszPrefix)
{
  BluePathDeleteMapW (tszPrefix) ;
}

OFC_VOID ofc_framework_update (OFC_VOID)
{
  BlueConfigUpdate() ;
}

OFC_VOID ofc_framework_dump_heap (OFC_VOID)
{
  BlueHeapDump() ;
}

static OFC_INT wifi_ip = 0 ;

OFC_VOID ofc_framework_set_wifi_ip (OFC_INT ip)
{
  wifi_ip = BLUE_NET_NTOL (&ip, 0) ;
}

OFC_INT ofc_framework_get_wifi_ip(OFC_VOID)
{
  return (wifi_ip) ;
}
