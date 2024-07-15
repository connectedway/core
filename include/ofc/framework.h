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
#include "ofc/file.h"

/**
 * \{
 * \defgroup framework Core Open Files Management APIs
 *
 * These APIs are for advanced use cases and manual initialization,
 * configuration, and startup.  These APIs are generally not needed
 * if the build is configured with INIT_ON_LOAD and OFC_PERSIST.
 * Embedded devices without a file system, or that wish to control
 * the stacks startup can build without those two build settings and
 * use the apis available in here.  For an example of manual
 * initialization, configuration, and startup, see \ref smbinit.c.
 *
 * There are three operations involved in starting up an Open Files
 * stack:
 *
 *   - Initialize the Open Files state
 *   - Configure the Open Files Stack
 *   - Start up the stack
 *
 * Open Files (ConnectSMB) can implicitly initialize, configure and startup
 * the stack, or the steps can be executed explicitly.
 *
 * The behavior is governed by two config variables.
 * The config file used in the build (<platform>-behavior.cfg) contains two
 * variables:
 *
 * INIT_ON_LOAD - Initialize libraries on load
 * OFC_PERSIST - Build in persistent configuration support.
 *
 * If INIT_ON_LOAD is ON, OpenFiles will be initialized upon
 * library load, the stack will be configured through the default configuration
 * file, and the stack will be subsequently be sstarted.
 *
 * If INIT_ON_LOAD is OFF, an application will need to explicitly initialize
 * both the core framework and the smb framework, configure the stack
 * either through the persistent framework or explicitly through API calls, and
 * then must manually startup the stack.
 *
 * A runtime application can examine the state of these configuration 
 * variables which will be defined within the include file "ofc/config.h".
 *
 * If INIT_ON_LOAD is defined, then the stack will be implicitly initialized
 * upon library load.  If INIT_ON_LOAD is undefined, the application must 
 * perform the initialization, configuration, and startup itself.
 *
 * If the application is performing explicit initialization, it needs to
 * configure the stack.  It can check whether OFC_PERSIST is defined or not.
 * If it is defined, then the ability to configure the stack through a
 * configuration file is supported.  Whether or not the configuration file
 * has been set up or not is a deployment consideration.  If the platform
 * supports OFC_PERSIST and the deployment has provided a configuration
 * file, then the configuration can be loaded and stack subsequently
 * configured from the loaded config, or the application can explicity
 * configure the stack through APIs.  If OFC_PERSIST is not defined, the
 * only way to configure the stack will be through the APIs.
 *
 * Lastly, if the application is performing it's own explicit initialization,
 * it must start the stack after it has been configured.
 *
 * If the build has been configured with INIT_ON_LOAD, and OFC_PERSIST has
 * also been configured so that runtime configuration of the stack is
 * restored from the runtime configuration file, there should be no need
 * for an application developer to call any of these routines.
 *
 * These APIs should be used if manual configuration of the stack is
 * required.
 *
 * Function | Description
 * ---------|------------
 * \ref ofc_framework_init | Initialize the OpenFiles Stack
 * \ref ofc_framework_destroy | Destory the OpenFiles Stack
 * \ref ofc_framework_startup | Startup the OpenFiles Components
 * \ref ofc_framework_shutdown | Shutdown the OpenFiles Components
 * \ref ofc_framework_load | Load a Configuration File
 * \ref ofc_framework_loadbuf | Load Configuration from Buffer
 * \ref ofc_framework_save | Save Configuration to File
 * \ref ofc_framework_savebuf | Save Configuration to a buffer
 * \ref ofc_get_config_dir | Return directory of Configuration File
 * \ref ofc_framework_set_host_name | Set the hostname
 * \ref ofc_framework_get_host_name | Get the host name of the system
 * \ref ofc_framework_free_host_name | Free hostname returned from ofc_get_host_name
 * \ref ofc_framework_get_workgroup | Get the workgroup of the system
 * \ref ofc_framework_free_workgroup | Free workgroup returned
 * \ref ofc_framework_get_description | Get the Description of the System
 * \ref ofc_framework_free_description | Free Description
 * \ref ofc_framework_set_uuid | Set UUID of system
 * \ref ofc_framework_get_uuid | Gets the UUID of the system
 * \ref ofc_framework_free_uuid | Free the uuid
 * \ref ofc_framework_get_root_dir | Get the root directory of stack
 * \ref ofc_framework_free_root_dir | Free the root directory string
 * \ref ofc_framework_set_logging | Set logging behavior
 * \ref ofc_framework_set_interface_discovery | Should stack get network config from underlying system
 * \ref ofc_framework_get_interface_discovery | Get state of discovery
 * \ref ofc_framework_add_interface | Add an interface to stack
 * \ref ofc_framework_remove_interface | Remove an interface
 * \ref ofc_framework_get_interfaces | Return interfaces configured
 * \ref ofc_framework_free_interfaces | Free Interfaces returned
 * \ref ofc_framework_add_map | Add a map (i.e. link)
 * \ref ofc_framework_get_maps | Return all maps
 * \ref ofc_framework_free_maps | Free returned maps
 * \ref ofc_framework_remove_map | Remove a map
 * \ref ofc_framework_update | Notify components of update
 * \ref ofc_framework_dump_heap | Dump outstanding allocations 
 * \ref ofc_framework_stats_heap | Dump Heap Statistics
 */

/**
 * WINS Abstraction
 */
typedef struct {
  OFC_INT num_wins;		//!< Number of Wins IP addresses
  OFC_IPADDR *winsaddr;		//!< Array of Wins IP addresses
} OFC_FRAMEWORK_WINSLIST;

/**
 * Netbios Modes
 *
 * NetBIOS can operate in a number of different modes that govern whether
 * names are registered and resolved by broadcast, or by querying a WINS
 * server.
 */
typedef enum {
    OFC_CONFIG_BMODE = 0,    /**< Broadcast Mode  */
    OFC_CONFIG_PMODE,        /**< WINS Mode  */
    OFC_CONFIG_MMODE,        /**< Mixed Mode, Broadcast first  */
    OFC_CONFIG_HMODE,        /**< Hybrid Mode, WINS first  */
    OFC_CONFIG_MODE_MAX    /**< Number of Modes  */
} OFC_CONFIG_MODE;

/**
 * An Interface Abstraction
 */
typedef struct {
    OFC_CONFIG_MODE netBiosMode; /*!< Netbios Mode  */
    OFC_IPADDR ip;		 /*!< IP address of Interface  */
    OFC_IPADDR bcast;		 /*!< Broadcast Address of Subnet  */
    OFC_IPADDR mask;		 /*!< NetMask of Subnet  */
    OFC_LPCSTR lmb;		 /*!< Local Master Browser of Subnet  */
    OFC_FRAMEWORK_WINSLIST wins; /*!< WINS List of Subnet  */
} OFC_FRAMEWORK_INTERFACE;

/**
 * Interfaces Structure
 */
typedef struct {
    OFC_UINT16 num_interfaces;	/*!< Number of interfaces in array  */
    OFC_FRAMEWORK_INTERFACE *iface; /*!< Array of Interfaces  */
} OFC_FRAMEWORK_INTERFACES;

/**
 * A map abstraction
 */
typedef struct {
    OFC_LPTSTR prefix;		/*!< Map Prefix (Alias) */
    OFC_LPTSTR desc;		/*!< Description of map  */
    OFC_LPTSTR path;		/*!< Maps Path  */
    OFC_FST_TYPE type;		/*!< File system Type of map  */
    OFC_BOOL thumbnail;		/*!< Thumbnail state (info only)  */
} OFC_FRAMEWORK_MAP;

/**
 * Array of Maps
 */
typedef struct {
    OFC_UINT16 numMaps;		/*!< Number of maps in array  */
    OFC_FRAMEWORK_MAP *map;	/*!< Map array  */
} OFC_FRAMEWORK_MAPS;

#if defined(__cplusplus)
extern "C"
{
#endif
/**
 * Initialize the Open iles Stack
 *
 * This routine should be called before any other OpenFiles
 * function.  It initializes heap and other variables
 */
OFC_CORE_LIB OFC_VOID
ofc_framework_init(OFC_VOID);

/**
 * Destroy the Open Files framework
 *
 * This will release all memory and destroy the heap.  The state of the
 * application should be as if Open Files has not been run.
 */
OFC_CORE_LIB OFC_VOID
ofc_framework_destroy(OFC_VOID);

/**
 * Start up the OpenFiles Stack
 *
 * This routine should be called after the stack has been
 * initialized and after all configuration has been performed.
 * It will start the various components running
 */
OFC_CORE_LIB OFC_VOID
ofc_framework_startup(OFC_VOID);

/**
 * \private
 * Start up the OpenFiles Stack with an Event
 *
 * This routine should be called after the stack has been
 * initialized and after all configuration has been performed.
 * It will start the various components running
 */
OFC_CORE_LIB OFC_VOID
ofc_framework_startup_ev(OFC_HANDLE hScheduler, OFC_HANDLE hEvent);

/**
 * Shutdown the OpenFiles Components
 */
OFC_CORE_LIB OFC_VOID
ofc_framework_shutdown(OFC_VOID);
/**
 * Load the ConnectedSMB configuration from a file
 *
 * This is an optional call mainly used for SMB Server configuration.
 * It is not needed by the client unless you have stored drive
 * maps in the configuration file
 *
 * \param filename
 * Path to filename to load configuration from
 */
OFC_CORE_LIB OFC_VOID ofc_framework_load(OFC_LPCTSTR filename);
/**
 * Load the ConnectedSMB configuration from a buffer
 *
 * This is an optional call mainly used for SMB Server configuration.
 * It is not needed by the client unless you have stored drive
 * maps in the configuration file
 *
 * \param buf
 * Buffer to load configuration from
 *
 * \param len
 * length of buffer
 */
OFC_CORE_LIB OFC_VOID ofc_framework_loadbuf(OFC_LPVOID buf, OFC_SIZET len);
/**
 * Save the current configuration to a file
 *
 * Useful if you've configured the stack using the various API calls
 * and wish to capture them so they can be later loaded.
 *
 * \param filename
 * Path to Filename to save configuration in
 */
OFC_CORE_LIB OFC_VOID ofc_framework_save(OFC_LPCTSTR filename);
/**
 * Save the current configuration to heap allocated buffer
 *
 * Useful if you've configured the stack using the various API calls
 * and wish to capture them so they can be later loaded.
 *
 * NOTE: buf must be freed by caller
 * 
 * \param buf
 * Buffer to store configuration in
 *
 * \param len
 * Length of buffer
 *
 * \returns
 * True if configuration stored
 */
OFC_CORE_LIB OFC_VOID ofc_framework_savebuf(OFC_LPVOID *buf, OFC_SIZET *len);

/**
 * Find the application directory (i.e. directory that config file is in
 *
 * returns a TSTR of the config directory that needs to be freed after use
 * 
 * \param config_dir
 * buffer to hold config directory
 *
 * \param len
 * Length of buffer
 *
 * \returns
 * True if config directory stored.
 */
OFC_CORE_LIB OFC_BOOL ofc_get_config_dir(OFC_TCHAR *config_dir,
					 OFC_SIZET len);
/**
 * Set Configuration File Path
 *
 * \param filename
 * Path to configuration file
 */
OFC_CORE_LIB OFC_VOID ofc_set_config_path(OFC_TCHAR *filename);

/**
 * Enable/Disable NetBIOS
 *
 * \param enabled
 * If OFC_TRUE, enable netbios, if OFC_FALSE, disable
 */
OFC_CORE_LIB OFC_VOID
ofc_framework_set_netbios(OFC_BOOL enabled);
/**
 * Set the host name of the running instance
 *
 * Mostly used to set the server name but this will also set the
 * client host name as well.
 *
 * \param name
 * Name of System
 * 
 * \param workgroup
 * Workgroup of System
 *
 * \param desc
 * Description of system
 */
OFC_CORE_LIB OFC_VOID
ofc_framework_set_host_name(OFC_LPCTSTR name, OFC_LPCTSTR workgroup,
                            OFC_LPCTSTR desc);
/**
 * Return the OpenFiles Host Name
 *
 * This returns a hostname allocated from the heap.  You must call
 * ofc_framework_free_host_name to free the returned name
 *
 * /returns
 * Hostname
 */
OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_host_name(OFC_VOID);
/**
 * Free the hostname returned from ofc_framework_get_host_name
 *
 * \param str
 * String to free
 */
OFC_CORE_LIB OFC_VOID ofc_framework_free_host_name(OFC_LPTSTR str);
/**
 * Get the workgroup that this instance is part of
 *
 * The returned workgroup name must be freed with ofc_framework_free_workgroup
 *
 * \returns 
 * Workgroup
 */
OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_workgroup(OFC_VOID);
/**
 * Free the workgroup name returned by ofc_framework_get_workgroup
 * 
 * \param str
 * Workgroup name to free
 */
OFC_CORE_LIB OFC_VOID ofc_framework_free_workgroup(OFC_LPTSTR str);
/**
 * Get the description of the host
 *
 * \returns
 * Description
 */
OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_description(OFC_VOID);
/**
 * Free the returned description of the host
 *
 * \param str
 * Description String to free
 */
OFC_CORE_LIB OFC_VOID ofc_framework_free_description(OFC_LPTSTR str);

/**
 * Set the UUID of the host
 *
 * This should be called to set the UUID of the host used in
 * SMB authentication.
 *
 * \param cuuid
 * UUID of system
 */
OFC_VOID ofc_framework_set_uuid(const OFC_CHAR *cuuid);

/**
 * Get the stacks UUID
 * \returns
 * UUID
 */
OFC_CHAR *ofc_framework_get_uuid(OFC_VOID);
/**
 * Free the UUID returned from GetUUID
 * \param str
 * String to free
 */
OFC_CORE_LIB OFC_VOID ofc_framework_free_uuid(OFC_LPSTR str);
/**
 * Get the Home/Root directory of the stack
 *
 * Used by the Android app only
 *
 * \returns
 * Root Directory
 */
OFC_CORE_LIB OFC_LPTSTR ofc_framework_get_root_dir(OFC_VOID);
/**
 * Free the string returned from getrootdir
 *
 * \param str
 * String to free
 */
OFC_CORE_LIB OFC_VOID ofc_framework_free_root_dir(OFC_LPTSTR str);

/**
 * Set the logging behavior of Openfiles
 *
 * \param log_level
 * The log level to capture.  All log levels less than or equal to this level will be captured.  Those greater
 * will be ignored. \sa OFC_LOG_LEVEL
 *
 * \param log_console
 * Set to OFC_TRUE if log messages should be output to console
 *
 * \note
 * On Linux, captured logs are output to the syslog.  On Android, captured logs are output to the application's
 * file directory.  On Windows, log file is written to the file "openfiles.log" in the current directory.
 * On other platforms, see ConnectedWay support
 */
OFC_CORE_LIB OFC_VOID ofc_framework_set_logging(OFC_UINT log_level, OFC_BOOL log_console);
/**
 * Set whether the stack should query the underlying platform for
 * available interfaces and IP addresses or whether the network
 * configuration is done manually or not
 *
 * \param on
 * True if interfaces should be discovered
 */
OFC_VOID ofc_framework_set_interface_discovery(OFC_BOOL on);
/**
 * \private
 * Set the Network Handle for use by the Android NDK code.
 * Not needed after API 31
 */
OFC_VOID ofc_framework_set_network_handle(OFC_UINT64 network_handle);
/**
 * Return the setting of interface discovery
 *
 * \returns
 * True if interface discovery is enabled
 */
OFC_BOOL ofc_framework_get_interface_discovery(OFC_VOID);

/**
 * Add an interface.
 *
 * Only useful if interface discovery is off
 *
 * iface
 * Interface description to add
 */
OFC_VOID ofc_framework_add_interface(OFC_FRAMEWORK_INTERFACE *iface);

/**
 * Remove an interface
 *
 * Only useful if interface discovery is off
 *
 * \param ip
 * IP of interface to remove
 */
OFC_VOID ofc_framework_remove_interface(OFC_IPADDR *ip);

/**
 * Get configured interfaces.  Interface array must subsequently be freed.
 * \returns
 * Array of interfaces
 */
OFC_FRAMEWORK_INTERFACES *ofc_framework_get_interfaces(OFC_VOID);

/**
 * Free interfaces returned from getinterfaces
 *
 * \param ifaces
 * Interface array to free
 */
OFC_VOID ofc_framework_free_interfaces(OFC_FRAMEWORK_INTERFACES *ifaces);

/**
 * Add an alias
 *
 * This allows a shortened name for a path
 *
 * \param map
 * Map to add
 *
 * \returns
 * Status of add
 */
OFC_BOOL ofc_framework_add_map(OFC_FRAMEWORK_MAP *map);

/**
 * Return the aliases, maps must subsequently be freed
 * 
 * \returns
 * Array of maps
 */
OFC_FRAMEWORK_MAPS *ofc_framework_get_maps(OFC_VOID);

/**
 * Free the returned aliases
 *
 * \param maps
 * Array of maps to free
 */
OFC_VOID ofc_framework_free_maps(OFC_FRAMEWORK_MAPS *maps);
/**
 * Remove a map
 *
 * \param tszPrefix
 * Prefix (i.e.alias) to remove
 */
OFC_VOID ofc_framework_remove_map(OFC_LPCTSTR tszPrefix);

/**
 * Reconfigure the stack
 *
 * Used to propogate configuration changes to all components
 *
 */
OFC_VOID ofc_framework_update(OFC_VOID);

/**
 * Dump the heap
 *
 * Used in debug mode only
 */
OFC_VOID ofc_framework_dump_heap(OFC_VOID);
  /**
   * \private
   */
OFC_VOID ofc_framework_set_wifi_ip(OFC_INT);
  /**
   * \private
   */
OFC_INT ofc_framework_get_wifi_ip(OFC_VOID);
  /**
   * Print Heap Statistics
   */
OFC_VOID ofc_framework_stats_heap(OFC_VOID);

#if defined(__cplusplus)
}
#endif
/** \} */
#endif
