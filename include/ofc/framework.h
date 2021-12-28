/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_FRAMEWORK_H__)
#define __OFC_FRAMEWORK_H__

#include "ofc/types.h"
#include "ofc/config.h"
#include "ofc/core.h"
#include "ofc/net.h"
#include "ofc/path.h"
#include "ofc/persist.h"
#include "ofc/file.h"

typedef struct
{
  OFC_INT num_wins ;
  BLUE_IPADDR *winsaddr ;
} BLUE_FRAMEWORK_WINSLIST ;

typedef struct 
{
  BLUE_CONFIG_MODE netBiosMode ;
  BLUE_IPADDR ip ;
  BLUE_IPADDR bcast ;
  BLUE_IPADDR mask ;
  OFC_LPCSTR lmb ;
  BLUE_FRAMEWORK_WINSLIST wins ;
} BLUE_FRAMEWORK_INTERFACE ;

typedef struct
{
  OFC_UINT16 num_interfaces ;
  BLUE_FRAMEWORK_INTERFACE *iface ;
} BLUE_FRAMEWORK_INTERFACES ;

typedef struct
{
  OFC_LPTSTR prefix ;
  OFC_LPTSTR desc ;
  OFC_LPTSTR path ;
  BLUE_FS_TYPE type ;
  OFC_BOOL thumbnail ;
} BLUE_FRAMEWORK_MAP ;

typedef struct
{
  OFC_UINT16 numMaps ;
  BLUE_FRAMEWORK_MAP *map ;
} BLUE_FRAMEWORK_MAPS ;

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Initialize the Connected SMB Stack
   *
   * This routine should be called before any other ConnectedSMB 
   * function.  It initializes heap and other variables
   */
  OFC_CORE_LIB OFC_VOID
    BlueFrameworkInit (OFC_VOID) ;
  OFC_CORE_LIB OFC_VOID
    BlueFrameworkDestroy (OFC_VOID) ;

  /**
   * Start up the Connected SMB Stack
   * 
   * This routine should be called after the stack has been 
   * initialized and after all configuration has been performed.
   * It will start the various components running
   */
  OFC_CORE_LIB OFC_VOID
    BlueFrameworkStartup (OFC_VOID) ;
  OFC_CORE_LIB OFC_VOID
  BlueFrameworkStartupEv (BLUE_HANDLE hScheduler, BLUE_HANDLE hEvent);
  OFC_CORE_LIB OFC_VOID
  BlueFrameworkShutdown (OFC_VOID) ;
  /**
   * Load the ConnectedSMB configuration from a file
   *
   * This is an optional call mainly used for SMB Server configuration.
   * It is not needed by the client unless you have stored drive
   * maps in the configuration file
   */
  OFC_CORE_LIB OFC_VOID BlueFrameworkLoad(OFC_LPCTSTR filename) ;
  /**
   * Save the current configuration to a file
   *
   * Useful if you've configured the stack using the various API calls
   * and wish to capture them so they can be later loaded.
   */
  OFC_CORE_LIB OFC_VOID BlueFrameworkSave (OFC_LPCTSTR filename) ;
  /**
   * Set the host name of the running instance
   *
   * Mostly used to set the server name but this will also set the
   * client host name as well.
   */
  OFC_CORE_LIB OFC_VOID
    BlueFrameworkSetHostName (OFC_LPCTSTR name, OFC_LPCTSTR workgroup,
                              OFC_LPCTSTR desc) ;
  /**
   * Return the ConnectedSMB Host Name
   *
   * This returns a hostname allocated from the heap.  You must call
   * BlueFrameworkFreeHostName to free the returned name
   */
  OFC_CORE_LIB OFC_LPTSTR BlueFrameworkGetHostName (OFC_VOID) ;
  /**
   * Free the hostname returned from BlueFrameworkGetHostName
   */
  OFC_CORE_LIB OFC_VOID BlueFrameworkFreeHostName (OFC_LPTSTR str) ;
  /**
   * Get the workgroup that this instance is part of
   *
   * The returned workgroup name must be freed with BlueFrameworkFreeWorkgroup
   */
  OFC_CORE_LIB OFC_LPTSTR BlueFrameworkGetWorkgroup (OFC_VOID) ;
  /**
   * Free the workgroup name returned by BlueFrameworkGetWorkgroup
   */
  OFC_CORE_LIB OFC_VOID BlueFrameworkFreeWorkgroup (OFC_LPTSTR str) ;
  /**
   * Get the description of the host
   */
  OFC_CORE_LIB OFC_LPTSTR BlueFrameworkGetDescription (OFC_VOID) ;
  /**
   * Free the returned description of the host
   */
  OFC_CORE_LIB OFC_VOID BlueFrameworkFreeDescription (OFC_LPTSTR str) ;
  /**
   * Set the UUID of the host
   *
   * This should be called to set the UUID of the host used in
   * SMB authentication.  
   */
  OFC_VOID BlueFrameworkSetUUID (const OFC_CHAR *cuuid) ;
  /**
   * Get the stacks UUID
   */
  OFC_CHAR *BlueFrameworkGetUUID (OFC_VOID) ;
  /**
   * Free the UUID returned from GetUUID
   */
  OFC_CORE_LIB OFC_VOID BlueFrameworkFreeUUID (OFC_LPSTR str) ;
  /**
   * Get the Home/Root directory of the stack
   *
   * Used by the Android app only
   */
  OFC_CORE_LIB OFC_LPTSTR BlueFrameworkGetRootDir (OFC_VOID) ;
  /**
   * Free the string returned from getrootdir
   */
  OFC_CORE_LIB OFC_VOID BlueFrameworkFreeRootDir (OFC_LPTSTR str) ;
  /**
   * Set whether the stack should query the underlying platform for
   * available interfaces and IP addresses or whether the network
   * configuration is done manually or not
   */
  OFC_VOID BlueFrameworkSetInterfaceDiscovery (OFC_BOOL on) ;
  /**
   * Return the setting of interface discovery
   */
  OFC_BOOL BlueFrameworkGetInterfaceDiscovery (OFC_VOID) ;
  /**
   * Add an interface.
   *
   * Only useful if interface discovery is off
   */
  OFC_VOID BlueFrameworkAddInterface (BLUE_FRAMEWORK_INTERFACE *iface) ;
  /**
   * Remove an interface
   *
   * Only useful if interface discovery is off
   */
  OFC_VOID BlueFrameworkRemoveInterface (BLUE_IPADDR *ip) ;
  /**
   * Get configured interfaces
   */
  BLUE_FRAMEWORK_INTERFACES* BlueFrameworkGetInterfaces (OFC_VOID) ;
  /**
   * Free interfaces returned from getinterfaces
   */
  OFC_VOID BlueFrameworkFreeInterfaces (BLUE_FRAMEWORK_INTERFACES *ifaces) ;
  /**
   * Add an alias
   *
   * This allows a shortened name for a path
   *
   * Optional
   */
  OFC_VOID BlueFrameworkAddMap (BLUE_FRAMEWORK_MAP *map) ;
  /**
   * Return the aliases
   */
  BLUE_FRAMEWORK_MAPS *BlueFrameworkGetMaps (OFC_VOID) ;
  /**
   * Free the returned aliases
   */
  OFC_VOID BlueFrameworkFreeMaps (BLUE_FRAMEWORK_MAPS *maps) ;
  /**
   * Reconfigure the stack 
   *
   * Used to propogate configuration changes to all components
   */
  OFC_VOID BlueFrameworkUpdate (OFC_VOID) ;
  /**
   * Dump the heap
   *
   * Used in debug mode only
   */
  OFC_VOID BlueFrameworkDumpHeap (OFC_VOID) ;
  OFC_VOID BlueFrameworkSetWifiIP(OFC_INT) ;
  OFC_INT BlueFrameworkGetWifiIP(OFC_VOID);

#if defined(__cplusplus)
}
#endif

#endif
