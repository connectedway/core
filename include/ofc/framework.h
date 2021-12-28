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
  OFC_IPADDR *winsaddr ;
} OFC_FRAMEWORK_WINSLIST ;

typedef struct 
{
  BLUE_CONFIG_MODE netBiosMode ;
  OFC_IPADDR ip ;
  OFC_IPADDR bcast ;
  OFC_IPADDR mask ;
  OFC_LPCSTR lmb ;
  OFC_FRAMEWORK_WINSLIST wins ;
} OFC_FRAMEWORK_INTERFACE ;

typedef struct
{
  OFC_UINT16 num_interfaces ;
  OFC_FRAMEWORK_INTERFACE *iface ;
} OFC_FRAMEWORK_INTERFACES ;

typedef struct
{
  OFC_LPTSTR prefix ;
  OFC_LPTSTR desc ;
  OFC_LPTSTR path ;
  OFC_FST_TYPE type ;
  OFC_BOOL thumbnail ;
} OFC_FRAMEWORK_MAP ;

typedef struct
{
  OFC_UINT16 numMaps ;
  OFC_FRAMEWORK_MAP *map ;
} OFC_FRAMEWORK_MAPS ;

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
    ofc_framework_init (OFC_VOID) ;
  OFC_CORE_LIB OFC_VOID
    ofc_framework_destroy (OFC_VOID) ;

  /**
   * Start up the Connected SMB Stack
   * 
   * This routine should be called after the stack has been 
   * initialized and after all configuration has been performed.
   * It will start the various components running
   */
  OFC_CORE_LIB OFC_VOID
    ofc_framework_startup (OFC_VOID) ;
  OFC_CORE_LIB OFC_VOID
  ofc_framework_startup_ev (OFC_HANDLE hScheduler, OFC_HANDLE hEvent);
  OFC_CORE_LIB OFC_VOID
  ofc_framework_shutdown (OFC_VOID) ;
  /**
   * Load the ConnectedSMB configuration from a file
   *
   * This is an optional call mainly used for SMB Server configuration.
   * It is not needed by the client unless you have stored drive
   * maps in the configuration file
   */
  OFC_CORE_LIB OFC_VOID ofc_framework_load(OFC_LPCTSTR filename) ;
  /**
   * Save the current configuration to a file
   *
   * Useful if you've configured the stack using the various API calls
   * and wish to capture them so they can be later loaded.
   */
  OFC_CORE_LIB OFC_VOID ofc_framework_save (OFC_LPCTSTR filename) ;
  /**
   * Set the host name of the running instance
   *
   * Mostly used to set the server name but this will also set the
   * client host name as well.
   */
  OFC_CORE_LIB OFC_VOID
    ofc_framework_set_host_name (OFC_LPCTSTR name, OFC_LPCTSTR workgroup,
                                 OFC_LPCTSTR desc) ;
  /**
   * Return the ConnectedSMB Host Name
   *
   * This returns a hostname allocated from the heap.  You must call
   * ofc_framework_free_host_name to free the returned name
   */
  OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_host_name (OFC_VOID) ;
  /**
   * Free the hostname returned from ofc_framework_get_host_name
   */
  OFC_CORE_LIB OFC_VOID ofc_framework_free_host_name (OFC_LPTSTR str) ;
  /**
   * Get the workgroup that this instance is part of
   *
   * The returned workgroup name must be freed with ofc_framework_free_workgroup
   */
  OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_workgroup (OFC_VOID) ;
  /**
   * Free the workgroup name returned by ofc_framework_get_workgroup
   */
  OFC_CORE_LIB OFC_VOID ofc_framework_free_workgroup (OFC_LPTSTR str) ;
  /**
   * Get the description of the host
   */
  OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_description (OFC_VOID) ;
  /**
   * Free the returned description of the host
   */
  OFC_CORE_LIB OFC_VOID ofc_framework_free_description (OFC_LPTSTR str) ;
  /**
   * Set the UUID of the host
   *
   * This should be called to set the UUID of the host used in
   * SMB authentication.  
   */
  OFC_VOID ofc_framework_set_uuid (const OFC_CHAR *cuuid) ;
  /**
   * Get the stacks UUID
   */
  OFC_CHAR *ofc_framework_get_uuid (OFC_VOID) ;
  /**
   * Free the UUID returned from GetUUID
   */
  OFC_CORE_LIB OFC_VOID ofc_framework_free_uuid (OFC_LPSTR str) ;
  /**
   * Get the Home/Root directory of the stack
   *
   * Used by the Android app only
   */
  OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_root_dir (OFC_VOID) ;
  /**
   * Free the string returned from getrootdir
   */
  OFC_CORE_LIB OFC_VOID ofc_framework_free_root_dir (OFC_LPTSTR str) ;
  /**
   * Set whether the stack should query the underlying platform for
   * available interfaces and IP addresses or whether the network
   * configuration is done manually or not
   */
  OFC_VOID ofc_framework_set_interface_discovery (OFC_BOOL on) ;
  /**
   * Return the setting of interface discovery
   */
  OFC_BOOL ofc_framework_get_interface_discovery (OFC_VOID) ;
  /**
   * Add an interface.
   *
   * Only useful if interface discovery is off
   */
  OFC_VOID ofc_framework_add_interface (OFC_FRAMEWORK_INTERFACE *iface) ;
  /**
   * Remove an interface
   *
   * Only useful if interface discovery is off
   */
  OFC_VOID ofc_framework_remove_interface (OFC_IPADDR *ip) ;
  /**
   * Get configured interfaces
   */
  OFC_FRAMEWORK_INTERFACES* ofc_framework_get_interfaces (OFC_VOID) ;
  /**
   * Free interfaces returned from getinterfaces
   */
  OFC_VOID ofc_framework_free_interfaces (OFC_FRAMEWORK_INTERFACES *ifaces) ;
  /**
   * Add an alias
   *
   * This allows a shortened name for a path
   *
   * Optional
   */
  OFC_VOID ofc_framework_add_map (OFC_FRAMEWORK_MAP *map) ;
  /**
   * Return the aliases
   */
  OFC_FRAMEWORK_MAPS *ofc_framework_get_maps (OFC_VOID) ;
  /**
   * Free the returned aliases
   */
  OFC_VOID ofc_framework_free_maps (OFC_FRAMEWORK_MAPS *maps) ;
  /**
   * Reconfigure the stack 
   *
   * Used to propogate configuration changes to all components
   */
  OFC_VOID ofc_framework_update (OFC_VOID) ;
  /**
   * Dump the heap
   *
   * Used in debug mode only
   */
  OFC_VOID ofc_framework_dump_heap (OFC_VOID) ;
  OFC_VOID ofc_framework_set_wifi_ip(OFC_INT) ;
  OFC_INT ofc_framework_get_wifi_ip(OFC_VOID);

#if defined(__cplusplus)
}
#endif

#endif
