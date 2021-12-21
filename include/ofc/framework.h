/* Copyright (c) 2009 Blue Peach Solutions, Inc.
 * All rights reserved.
 *
 * This software is protected by copyright and intellectual 
 * property laws as well as international treaties.  It is to be 
 * used and copied only by authorized licensees under the 
 * conditions described in their licenses.  
 *
 * Title to and ownership of the software shall at all times 
 * remain with Blue Peach Solutions.
 */
#if !defined(__BLUE_FRAMEWORK_H__)
#define __BLUE_FRAMEWORK_H__

#include "ofc/types.h"
#include "ofc/config.h"
#include "ofc/core.h"
#include "ofc/net.h"
#include "ofc/path.h"
#include "ofc/persist.h"
#include "ofc/file.h"

typedef struct
{
  BLUE_INT num_wins ;
  BLUE_IPADDR *winsaddr ;
} BLUE_FRAMEWORK_WINSLIST ;

typedef struct 
{
  BLUE_CONFIG_MODE netBiosMode ;
  BLUE_IPADDR ip ;
  BLUE_IPADDR bcast ;
  BLUE_IPADDR mask ;
  BLUE_LPCSTR lmb ;
  BLUE_FRAMEWORK_WINSLIST wins ;
} BLUE_FRAMEWORK_INTERFACE ;

typedef struct
{
  BLUE_UINT16 num_interfaces ;
  BLUE_FRAMEWORK_INTERFACE *iface ;
} BLUE_FRAMEWORK_INTERFACES ;

typedef struct
{
  BLUE_LPTSTR prefix ;
  BLUE_LPTSTR desc ;
  BLUE_LPTSTR path ;
  BLUE_FS_TYPE type ;
  BLUE_BOOL thumbnail ;
} BLUE_FRAMEWORK_MAP ;

typedef struct
{
  BLUE_UINT16 numMaps ;
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
  BLUE_CORE_LIB BLUE_VOID
    BlueFrameworkInit (BLUE_VOID) ;
  BLUE_CORE_LIB BLUE_VOID
    BlueFrameworkDestroy (BLUE_VOID) ;

  /**
   * Start up the Connected SMB Stack
   * 
   * This routine should be called after the stack has been 
   * initialized and after all configuration has been performed.
   * It will start the various components running
   */
  BLUE_CORE_LIB BLUE_VOID
    BlueFrameworkStartup (BLUE_VOID) ;
  BLUE_CORE_LIB BLUE_VOID
  BlueFrameworkStartupEv (BLUE_HANDLE hScheduler, BLUE_HANDLE hEvent);
  BLUE_CORE_LIB BLUE_VOID
  BlueFrameworkShutdown (BLUE_VOID) ;
  /**
   * Load the ConnectedSMB configuration from a file
   *
   * This is an optional call mainly used for SMB Server configuration.
   * It is not needed by the client unless you have stored drive
   * maps in the configuration file
   */
  BLUE_CORE_LIB BLUE_VOID BlueFrameworkLoad(BLUE_LPCTSTR filename) ;
  /**
   * Save the current configuration to a file
   *
   * Useful if you've configured the stack using the various API calls
   * and wish to capture them so they can be later loaded.
   */
  BLUE_CORE_LIB BLUE_VOID BlueFrameworkSave (BLUE_LPCTSTR filename) ;
  /**
   * Set the host name of the running instance
   *
   * Mostly used to set the server name but this will also set the
   * client host name as well.
   */
  BLUE_CORE_LIB BLUE_VOID 
    BlueFrameworkSetHostName (BLUE_LPCTSTR name, BLUE_LPCTSTR workgroup,
			      BLUE_LPCTSTR desc) ;
  /**
   * Return the ConnectedSMB Host Name
   *
   * This returns a hostname allocated from the heap.  You must call
   * BlueFrameworkFreeHostName to free the returned name
   */
  BLUE_CORE_LIB BLUE_LPTSTR BlueFrameworkGetHostName (BLUE_VOID) ;
  /**
   * Free the hostname returned from BlueFrameworkGetHostName
   */
  BLUE_CORE_LIB BLUE_VOID BlueFrameworkFreeHostName (BLUE_LPTSTR str) ;
  /**
   * Get the workgroup that this instance is part of
   *
   * The returned workgroup name must be freed with BlueFrameworkFreeWorkgroup
   */
  BLUE_CORE_LIB BLUE_LPTSTR BlueFrameworkGetWorkgroup (BLUE_VOID) ;
  /**
   * Free the workgroup name returned by BlueFrameworkGetWorkgroup
   */
  BLUE_CORE_LIB BLUE_VOID BlueFrameworkFreeWorkgroup (BLUE_LPTSTR str) ;
  /**
   * Get the description of the host
   */
  BLUE_CORE_LIB BLUE_LPTSTR BlueFrameworkGetDescription (BLUE_VOID) ;
  /**
   * Free the returned description of the host
   */
  BLUE_CORE_LIB BLUE_VOID BlueFrameworkFreeDescription (BLUE_LPTSTR str) ;
  /**
   * Set the UUID of the host
   *
   * This should be called to set the UUID of the host used in
   * SMB authentication.  
   */
  BLUE_VOID BlueFrameworkSetUUID (const BLUE_CHAR *cuuid) ;
  /**
   * Get the stacks UUID
   */
  BLUE_CHAR *BlueFrameworkGetUUID (BLUE_VOID) ;
  /**
   * Free the UUID returned from GetUUID
   */
  BLUE_CORE_LIB BLUE_VOID BlueFrameworkFreeUUID (BLUE_LPSTR str) ;
  /**
   * Get the Home/Root directory of the stack
   *
   * Used by the Android app only
   */
  BLUE_CORE_LIB BLUE_LPTSTR BlueFrameworkGetRootDir (BLUE_VOID) ;
  /**
   * Free the string returned from getrootdir
   */
  BLUE_CORE_LIB BLUE_VOID BlueFrameworkFreeRootDir (BLUE_LPTSTR str) ;
  /**
   * Set whether the stack should query the underlying platform for
   * available interfaces and IP addresses or whether the network
   * configuration is done manually or not
   */
  BLUE_VOID BlueFrameworkSetInterfaceDiscovery (BLUE_BOOL on) ;
  /**
   * Return the setting of interface discovery
   */
  BLUE_BOOL BlueFrameworkGetInterfaceDiscovery (BLUE_VOID) ;
  /**
   * Add an interface.
   *
   * Only useful if interface discovery is off
   */
  BLUE_VOID BlueFrameworkAddInterface (BLUE_FRAMEWORK_INTERFACE *iface) ;
  /**
   * Remove an interface
   *
   * Only useful if interface discovery is off
   */
  BLUE_VOID BlueFrameworkRemoveInterface (BLUE_IPADDR *ip) ;
  /**
   * Get configured interfaces
   */
  BLUE_FRAMEWORK_INTERFACES* BlueFrameworkGetInterfaces (BLUE_VOID) ;
  /**
   * Free interfaces returned from getinterfaces
   */
  BLUE_VOID BlueFrameworkFreeInterfaces (BLUE_FRAMEWORK_INTERFACES *ifaces) ;
  /**
   * Add an alias
   *
   * This allows a shortened name for a path
   *
   * Optional
   */
  BLUE_VOID BlueFrameworkAddMap (BLUE_FRAMEWORK_MAP *map) ;
  /**
   * Return the aliases
   */
  BLUE_FRAMEWORK_MAPS *BlueFrameworkGetMaps (BLUE_VOID) ;
  /**
   * Free the returned aliases
   */
  BLUE_VOID BlueFrameworkFreeMaps (BLUE_FRAMEWORK_MAPS *maps) ;
  /**
   * Reconfigure the stack 
   *
   * Used to propogate configuration changes to all components
   */
  BLUE_VOID BlueFrameworkUpdate (BLUE_VOID) ;
  /**
   * Dump the heap
   *
   * Used in debug mode only
   */
  BLUE_VOID BlueFrameworkDumpHeap (BLUE_VOID) ;
  BLUE_VOID BlueFrameworkSetWifiIP(BLUE_INT) ;
  BLUE_INT BlueFrameworkGetWifiIP(BLUE_VOID);

#if defined(__cplusplus)
}
#endif

#endif
