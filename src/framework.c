/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __BLUE_CORE_DLL__

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

#if defined(BLUE_PARAM_PERSIST)
static BLUE_LPTSTR config_filename = BLUE_NULL ;
#endif

/**
 * \defgroup BlueInit Initialization
 * \ingroup Applications
 */

/** \{ */

BLUE_CORE_LIB BLUE_VOID
BlueFrameworkInit (BLUE_VOID)
{
  /*
   * Load Blue Share if not done as part of library load
   */
#if !defined(BLUE_PARAM_INIT_ON_LOAD)
  BlueUtilLoad() ;
#endif

  /*
   * Print out the banner
   */
  BlueCprintf ("OpenFiles (%s) %d.%d %s\n", 
	       BLUE_PARAM_SHARE_VARIANT,
	       BLUE_PARAM_SHARE_MAJOR, BLUE_PARAM_SHARE_MINOR,
	       BLUE_PARAM_SHARE_TAG) ;
}

BLUE_CORE_LIB BLUE_VOID
BlueFrameworkDestroy (BLUE_VOID)
{
  /*
   * Load Blue Share if not done as part of library load
   */
#if defined(BLUE_PARAM_PERSIST)
  if (config_filename != BLUE_NULL)
    {
      BlueHeapFree(config_filename);
      config_filename = BLUE_NULL;
    }
#endif
#if !defined(BLUE_PARAM_INIT_ON_LOAD)
  BlueUtilUnload() ;
#endif
}

BLUE_CORE_LIB BLUE_VOID
BlueFrameworkStartup (BLUE_VOID)
{
  BLUE_HANDLE hScheduler ;

  hScheduler = BlueSchedCreate() ;
  BlueFrameworkStartupEv(hScheduler, BLUE_HANDLE_NULL);
}

BLUE_CORE_LIB BLUE_VOID
BlueFrameworkStartupEv (BLUE_HANDLE hScheduler, BLUE_HANDLE hEvent)
{
  /*
   * Configuration is complete.  Start up the stack
   */
#if defined(BLUE_PARAM_NETMON)
  BlueNetMonStartup (hScheduler, BLUE_HANDLE_NULL) ;
#endif

}

BLUE_CORE_LIB BLUE_VOID
BlueFrameworkShutdown (BLUE_VOID)
{
#if defined(BLUE_PARAM_NETMON)
#if 0
  BlueNetMonShutdown (hScheduler, BLUE_HANDLE_NULL) ;
#endif
#endif
}

BLUE_CORE_LIB BLUE_VOID BlueFrameworkLoad(BLUE_LPCTSTR filename)
{
#if defined(BLUE_PARAM_PERSIST)
  BlueCprintf ("Loading %S\n", filename) ;
  if (config_filename == BLUE_NULL)
    config_filename = BlueCtstrdup (filename) ;
  BlueConfigLoad (filename) ;
#else
  BlueCprintf ("Configuration Files Disabled\n") ;
#endif
}

BLUE_CORE_LIB BLUE_VOID BlueFrameworkSave (BLUE_LPCTSTR filename)
{
#if defined(BLUE_PARAM_PERSIST)
  if (filename == BLUE_NULL)
    filename = config_filename ;
  if (filename != BLUE_NULL)
    {
      BlueCprintf ("Saving %S\n", filename) ;
      BlueConfigSave (filename) ;
    }
#else
  BlueCprintf ("Configuration Files Disabled\n") ;
#endif
}

BLUE_CORE_LIB BLUE_VOID 
BlueFrameworkSetHostName (BLUE_LPCTSTR name, BLUE_LPCTSTR workgroup,
			  BLUE_LPCTSTR desc)
{
  BlueConfigSetNodeName (name, workgroup, desc) ;
}

BLUE_CORE_LIB BLUE_LPTSTR BlueFrameworkGetHostName (BLUE_VOID)
{
  BLUE_LPCTSTR name ;
  BLUE_LPCTSTR workgroup ;
  BLUE_LPCTSTR desc ;
  BLUE_LPTSTR tstrName ;

  BlueConfigNodeName (&name, &workgroup, &desc) ;

  tstrName = BlueCtstrdup (name) ;

  return (tstrName) ;
}

BLUE_CORE_LIB BLUE_VOID BlueFrameworkFreeHostName (BLUE_LPTSTR str)
{
  BlueHeapFree (str) ;
}

BLUE_CORE_LIB BLUE_LPTSTR BlueFrameworkGetWorkgroup (BLUE_VOID)
{
  BLUE_LPCTSTR name ;
  BLUE_LPCTSTR workgroup ;
  BLUE_LPCTSTR desc ;
  BLUE_LPTSTR tstrWorkgroup ;

  BlueConfigNodeName (&name, &workgroup, &desc) ;

  tstrWorkgroup = BlueCtstrdup (workgroup) ;

  return (tstrWorkgroup) ;
}

BLUE_CORE_LIB BLUE_VOID BlueFrameworkFreeWorkgroup (BLUE_LPTSTR str)
{
  BlueHeapFree (str) ;
}

BLUE_CORE_LIB BLUE_LPTSTR BlueFrameworkGetDescription (BLUE_VOID)
{
  BLUE_LPCTSTR name ;
  BLUE_LPCTSTR workgroup ;
  BLUE_LPCTSTR desc ;
  BLUE_LPTSTR tstrDesc ;

  BlueConfigNodeName (&name, &workgroup, &desc) ;

  tstrDesc = BlueCtstrdup (desc) ;

  return (tstrDesc) ;
}

BLUE_CORE_LIB BLUE_VOID BlueFrameworkFreeDescription (BLUE_LPTSTR str)
{
  BlueHeapFree (str) ;
}

BLUE_VOID BlueFrameworkSetUUID (const BLUE_CHAR *cuuid)
{
  BLUE_UUID uuid ;

  BlueCatouuid (cuuid, uuid) ;
  BlueConfigSetUUID (&uuid) ;
}

BLUE_CHAR *BlueFrameworkGetUUID (BLUE_VOID)
{
  BLUE_UUID uuid ;
  BLUE_CHAR *cuuid ;

  BlueConfigUUID (&uuid) ;

  cuuid = BlueHeapMalloc (UUID_STR_LEN+1) ;
  BlueCuuidtoa (uuid, cuuid) ;
  return (cuuid) ;
}

BLUE_CORE_LIB BLUE_VOID BlueFrameworkFreeUUID (BLUE_LPSTR str)
{
  BlueHeapFree (str) ;
}

BLUE_CORE_LIB BLUE_LPTSTR BlueFrameworkGetRootDir (BLUE_VOID)
{
  BLUE_LPTSTR tstrRootDir ;

  tstrRootDir = BlueHeapMalloc (BLUE_MAX_PATH * sizeof (BLUE_TCHAR)) ;
  BlueEnvGet (BLUE_ENV_ROOT, tstrRootDir, BLUE_MAX_PATH) ;
  return (tstrRootDir) ;
}

BLUE_CORE_LIB BLUE_VOID BlueFrameworkFreeRootDir (BLUE_LPTSTR str)
{
  BlueHeapFree (str) ;
}

BLUE_VOID BlueFrameworkSetInterfaceDiscovery (BLUE_BOOL on)
{
  BLUE_CONFIG_ICONFIG_TYPE itype ;

  itype = BLUE_CONFIG_ICONFIG_MANUAL ;
  if (on)
    itype = BLUE_CONFIG_ICONFIG_AUTO ;

  BlueConfigSetInterfaceType (itype) ;
}

BLUE_BOOL BlueFrameworkGetInterfaceDiscovery (BLUE_VOID)
{
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  if (BlueConfigInterfaceConfig() == BLUE_CONFIG_ICONFIG_AUTO)
    ret = BLUE_TRUE ;
  return (ret) ;
}

BLUE_VOID BlueFrameworkAddInterface (BLUE_FRAMEWORK_INTERFACE *iface)
{
  BLUE_INT old_count ;
  BLUE_INT new_count ;
  BLUE_CHAR *cstr ;

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

BLUE_VOID BlueFrameworkRemoveInterface (BLUE_IPADDR *ip)
{
  BlueConfigRemoveInterfaceConfig (ip) ;
}

BLUE_FRAMEWORK_INTERFACES *BlueFrameworkGetInterfaces (BLUE_VOID)
{
  BLUE_FRAMEWORK_INTERFACES *ifaces ;
  BLUE_INT i ;
  

  ifaces = BlueHeapMalloc (sizeof (BLUE_FRAMEWORK_INTERFACES)) ;
  if (ifaces != BLUE_NULL)
    {
      ifaces->num_interfaces = BlueConfigInterfaceCount() ;
      ifaces->iface = BlueHeapMalloc (sizeof (BLUE_FRAMEWORK_INTERFACE) *
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
				 
BLUE_VOID BlueFrameworkFreeInterfaces (BLUE_FRAMEWORK_INTERFACES *ifaces)
{
  BLUE_INT i ;

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

BLUE_FRAMEWORK_MAPS *BlueFrameworkGetMaps (BLUE_VOID)
{
  BLUE_FRAMEWORK_MAPS *maps ;
  BLUE_LPCTSTR prefix ;
  BLUE_LPCTSTR desc ;
  BLUE_BOOL thumbnail ;
  BLUE_PATH *path ;
  BLUE_INT i ;
  BLUE_SIZET len ;
  BLUE_SIZET rem ;
  BLUE_LPTSTR ptr ;
  
  maps = BlueHeapMalloc (sizeof (BLUE_FRAMEWORK_MAPS)) ;
  if (maps != BLUE_NULL)
    {
      maps->numMaps = 0 ;
      maps->map = BlueHeapMalloc (sizeof (BLUE_FRAMEWORK_MAP) *
				  BLUE_PARAM_MAX_MAPS) ;
      for (i = 0 ; i < BLUE_PARAM_MAX_MAPS ; i++)
	{
	  BluePathGetMapW (i, &prefix, &desc, &path, &thumbnail) ;
	  if (prefix != BLUE_NULL)
	    {
	      maps->map[maps->numMaps].prefix = BlueCtstrdup (prefix) ;
	      maps->map[maps->numMaps].desc = BlueCtstrdup (desc) ;
	      maps->map[maps->numMaps].type = BluePathType(path) ;
	      maps->map[maps->numMaps].thumbnail = thumbnail ;
	      rem = 0 ;
	      len = BluePathPrintW (path, BLUE_NULL, &rem) ;

	      maps->map[maps->numMaps].path  = 
		BlueHeapMalloc (sizeof (BLUE_TCHAR) * (len+1)) ;
	      
	      rem = len+1 ;
	      ptr = maps->map[maps->numMaps].path ;
	      BluePathPrintW (path, &ptr, &rem) ;
	      maps->numMaps++ ;
	    }
	}
    }
  return (maps) ;
}
				 
BLUE_VOID BlueFrameworkFreeMaps (BLUE_FRAMEWORK_MAPS *maps)
{
  BLUE_INT i ;

  for (i = 0 ; i < maps->numMaps ; i++)
    {
      BlueHeapFree (maps->map[i].prefix) ;
      BlueHeapFree (maps->map[i].path) ;
      BlueHeapFree (maps->map[i].desc) ;
    }
  BlueHeapFree (maps->map) ;
  BlueHeapFree (maps) ;
}

BLUE_VOID BlueFrameworkAddMap (BLUE_FRAMEWORK_MAP *map)
{
  BLUE_PATH *path ;

  path = BluePathCreateW (map->path) ;
  BluePathAddMapW (map->prefix, map->desc, path, map->type, map->thumbnail) ;
}

BLUE_VOID BlueFrameworkRemoveMap (BLUE_LPCTSTR tszPrefix)
{
  BluePathDeleteMapW (tszPrefix) ;
}

BLUE_VOID BlueFrameworkUpdate (BLUE_VOID)
{
  BlueConfigUpdate() ;
}

BLUE_VOID BlueFrameworkDumpHeap (BLUE_VOID)
{
  BlueHeapDump() ;
}

static BLUE_INT wifi_ip = 0 ;

BLUE_VOID BlueFrameworkSetWifiIP (BLUE_INT ip)
{
  wifi_ip = BLUE_NET_NTOL (&ip, 0) ;
}

BLUE_INT BlueFrameworkGetWifiIP(BLUE_VOID)
{
  return (wifi_ip) ;
}
