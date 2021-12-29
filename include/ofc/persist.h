/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_CONFIG_H__)
#define __OFC_CONFIG_H__

#include "ofc/types.h"
#include "ofc/net.h"

/**
 * Network Configuration Modes
 *
 * This enum defines whether Open Files should query the underlying platform
 * for network configuration, or whether it will be configured manual by
 * an application
 */
typedef enum
  {
    OFC_CONFIG_ICONFIG_AUTO = 0, /**< Configure Network Automatically  */
    OFC_CONFIG_ICONFIG_MANUAL,	/**< Configure Network Manually  */
    OFC_CONFIG_ICONFIG_NUM
  } OFC_CONFIG_ICONFIG_TYPE ;

/**
 * Netbios Modes
 *
 * NetBIOS can operate in a number of different modes that govern whether
 * names are registered and resolved by broadcast, or by querying a WINS
 * server.
 */
typedef enum
  {
    OFC_CONFIG_BMODE = 0,	/**< Broadcast Mode  */
    OFC_CONFIG_PMODE,		/**< WINS Mode  */
    OFC_CONFIG_MMODE,		/**< Mixed Mode, Broadcast first  */
    OFC_CONFIG_HMODE,		/**< Hybrid Mode, WINS first  */
    OFC_CONFIG_MODE_MAX	/**< Number of Modes  */
  } OFC_CONFIG_MODE ;

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * \defgroup GeneralConfig General Configuration APIs
   * \ingroup persist
   *
   * General Configuration Routines.
   *
   * \{
   */
  /**
   * Initialize the Open Files Persist Facility
   *
   * This should be called when the deamon is loaded before starting up
   * any of the components that use the configuration or calling any of
   * the APIs to set configuration.
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_init(OFC_VOID) ;
  /**
   * Releases all Configuration Information
   *
   * This routine is essentially the inverse of ofc_persist_init.  It can
   * be called to cleanly shutdown the Configuration Facility.  This 
   * should not be called unless all persist components have exited.
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_free (OFC_VOID) ;
  /**
   * Set a Default Configuration
   *
   * This routine will configure Open Files using a hard coded default
   * configuration.  This generally should not be used unless you wish
   * to define your own hard coded configuration and modify the 
   * ofc_persist_default routine yourself.  Otherwise we recommend that you
   * either configure using the persistent configuration functions, or
   * by having your own configuration routine that uses the manual 
   * configuration APIs.
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_default (OFC_VOID) ;

#if defined(OFC_PERSIST)
  /**
   * Load a Persistent Configuration File
   *
   * This routine loads a persistent configuration.
   *
   * In order to use this routine, you must have OFC_PERSIST 
   * defined.  It will load the configuration from an XML file. 
   * This routine will only set those fields present in the XML file.
   * A developer may wish to set default configuration info before calling
   * the load routine so that default values for fields not specified in
   * the XML file can be present.
   *
   * \param lpFileName The XML File to load
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_load (OFC_LPCTSTR lpFileName) ;
  /**
   * Saves a Configuration to a Persisten File
   *
   * This routine saves an Open Files configuration
   *
   * All configuration info will be saved for later reload
   *
   * \param lpFileName Name of XML File to save
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_save (OFC_LPCTSTR lpFileName) ;
  /**
   * Print a configuration to a buffer
   *
   * \param buf
   * Pointer to location to store allocated buffer.  
   * This buffer must be freed by the caller
   *
   * \param len
   * Pointer to where to store the length of the allocated buffer (plus null)
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_print (OFC_LPVOID *buf, OFC_SIZET *len) ;
#endif
  /**
   * Configure the persistant Node Name
   *
   * The Node Name is used by the NetBIOS Cifs Client and Cifs Server
   * components of Open Files.
   *
   * \param name The Node Name
   * \param workgroup The Workgroup that this node exists within
   * \param desc The description of this node
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_set_node_name (OFC_LPCTSTR name, OFC_LPCTSTR workgroup,
                             OFC_LPCTSTR desc) ;
  /**
   * Obtain the Node Name Information for a Node
   *
   * This routine will return the node name information.  The pointers
   * returned are static.  They should not be modified or freed.
   * 
   * \param name Where to store a pointer to the name
   * \param workgroup Where to store a pointer to the workgroup
   * \param desc Where to store a pointer to the description
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_node_name (OFC_LPCTSTR *name, OFC_LPCTSTR *workgroup,
                         OFC_LPCTSTR *desc) ;
  /**
   * Set the Node's UUID
   *
   * Each node should have a unique UUID.  It is generally a good idea
   * to base this on the MAC address of the node or through some other
   * algorithm that returns a unique value.
   *
   * For testing and demonstration purposes, this does not need to be set.
   *
   * \param uuid Pointer to the uuid value to set
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_set_uuid (OFC_UUID *uuid) ;
  /**
   * Return the Node's UUID
   *
   * \param uuid Pointer to where to place the UUID
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_uuid (OFC_UUID *uuid) ;
  /**
   * Set the Interface Configuration Mode
   *
   * There are two modes to configure the network interfaces on the node.
   * They can be set using the configured network interfaces of the platform
   * or they can manually be set.  If they are automatically configured,
   * then each discovered interface is configured by default.  This call
   * will reset any previous interface configuration.
   *
   * \param itype Type of Interface Configuration Mode  
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_set_interface_type (OFC_CONFIG_ICONFIG_TYPE itype) ;
  /**
   * Return Interface Configuration Mode
   *
   * This routine will return the configuration mode of the Open Files
   * stack.  
   *
   * \returns Configuration Mode
   */
  OFC_CORE_LIB OFC_CONFIG_ICONFIG_TYPE
  ofc_persist_interface_config (OFC_VOID) ;
  /**
   * Remove an Interface
   *
   * \param ip IP Address of interface to remove
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_remove_interface_config (OFC_IPADDR *ip) ;
  /**
   * Define the number of interfaces supported by Open Files
   *
   * This routine is only meaningful if the interface configuration mode
   * has been set to manual.  It is a noop otherwise.  This call
   * will reset the previous configuration.  All IP addresses will be
   * cleared.
   *
   * \param i The number of interfaces supported
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_set_interface_count (OFC_INT i) ;
  /**
   * Configure an Interface
   *
   * This routine will set all information for an interface.
   * It is only meaningful if the interface configuration mode has been
   * set to manual.
   *
   * \param i The interface index of the interface
   * \param netbios_mode The NetBIOS mode of the interface
   * \param ipaddress Pointer to IP Address of Interface.  If OFC_NULL,
   * do not set.
   * \param bcast Pointer to Broadcast Address of Interface.  If OFC_NULL,
   * do not set
   * \param mask Pointer to the Mask of the interface.  If OFC_NULL,
   * do not set
   * \param master Pointer to the name of the master browser for the 
   * interface.  If OFC_NULL, do not set.
   * \param num_wins
   * Number of Wins Servers for this interface (if P, M, or H mode)
   * \param winslist
   * Pointer to the wins list to use for the interface
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_set_interface_config (OFC_INT i,
                                    OFC_CONFIG_MODE netbios_mode,
                                    OFC_IPADDR *ipaddress,
                                    OFC_IPADDR *bcast,
                                    OFC_IPADDR *mask,
                                    OFC_CHAR *master,
                                    OFC_INT num_wins,
                                    OFC_IPADDR *winslist) ;
  /**
   * Return the number of interfaces in use by Open Files
   *
   * This is either the number of interfaces on the platform if the 
   * configuration mode is AUTO, or the number of interfaces set if the 
   * configuration mode is MANUAL
   *
   * \returns Number of Interfaces configured
   */
  OFC_CORE_LIB OFC_INT
  ofc_persist_interface_count (OFC_VOID) ;
  /**
   * Return configuration information for an interface
   *
   * \param index Index of the interface to return info for
   * \param addr Pointer to where to store the IP address.  If NULL, do not
   * store.
   * \param pbcast Pointer to where to store the broadcast address.  If NULL,
   * do not store.
   * \param mask Pointer to where to store the mask for the interface.  If 
   * NULL, do not store
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_interface_addr (OFC_INT index, OFC_IPADDR *addr,
                              OFC_IPADDR *pbcast, OFC_IPADDR *mask) ;
  /**
   * Configure Only the Local Master Browser for an Interface
   *
   * A local master browser is used for browsing for workgroups, domains,
   * and servers on a LAN segment.  By default, the local master browser
   * is dynamically discovered by the Open Files browser software.  If
   * A local master browser does not exist on the LAN segment, it can be
   * set by the user application by using this function.  
   *
   * \param index Index of the Interface to set
   * \param local_master Name of the Local Master Browser
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_set_local_master (OFC_INT index, OFC_LPCSTR local_master) ;
  /**
   * Return the local master browser for an interface
   *
   * \param index Index of the interface to obtain the browser for
   * \param local_master Pointer to where to store the pointer to the
   * master browser name.  This pointer is to a static string.  It should
   * not be freed or modified.
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_local_master (OFC_INT index, OFC_LPCSTR *local_master) ;
  /**
   * Configure Only the Private Pointer for an Interface
   *
   * \param index Index of the Interface to set
   * \param priv Pointer to the private pointer
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_set_private (OFC_INT index, OFC_VOID *priv) ;
  /**
   * Return the private pointer for an interface
   *
   * \param index Index of the interface to obtain the private pointer for
   * \param priv Pointer to where to store the private pointer.
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_private (OFC_INT index, OFC_VOID **priv) ;

  /**
   * Get the count of WINS Servers
   *
   * \param index
   * Index of the interface to retrieve the account from
   *
   * \returns Number of WINS Servers Configured
   */
  OFC_CORE_LIB OFC_INT
  ofc_persist_wins_count(OFC_INT index) ;
  /**
   * Get the IP Address of one of the WINS Servers
   *
   * \param xface
   * Index of the interface to get the wins address from
   *
   * \param index The index of the WINS Server to return the IP address of
   *
   * \param addr A pointer to a where to put the IP address
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_wins_addr(OFC_INT xface, OFC_INT index, OFC_IPADDR *addr) ;
  /**
   * Get the NetBIOS Mode of an Interface
   *
   * Each interface on a Open Files system can support a different 
   * NetBIOS Mode.  This call returns the mode that is running.
   * The interface mode for an interface can be set using the
   * ofc_persist_set_interface_config Routine
   *
   * \param index The index of the Interface to obtain the NetBIOS mode on.
   * \param num_wins 
   * Pointer to where to return Number of wins nodes for this interface
   * \param winslist
   * Pointer to where to return the wins list for this interface.  This
   * should be freed using ofc_free after done
   * \returns The NetBIOS Mode of that interface
   */
  OFC_CORE_LIB OFC_CONFIG_MODE
  ofc_persist_interface_mode (OFC_INT index, OFC_INT *num_wins,
                              OFC_IPADDR **winslist) ;

  /**
   * Has the configuration been loaded
   *
   * \returns
   * OFC_TRUE if loaded, OFC_FALSE otherwise
   */
  OFC_CORE_LIB OFC_BOOL
  ofc_persist_loaded (OFC_VOID) ;
  /**
   * Register a Update When Configuration Changed
   *
   * \param hEvent
   * The event to notify when an update event occurs
   *
   * NOTE: Registers are guaranteed to get at least one notification after
   * registering that will contain adds for each configured interface.
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_register_update (OFC_HANDLE hEvent) ;
  /**
   * Unregister a Configuration Update Notification
   *
   * \param hEvent
   * The Event to remove 
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_unregister_update (OFC_HANDLE hEvent) ;
  /**
   * Initiate a configuration event
   *
   * The routine takes no arguments
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_update (OFC_VOID) ;
  /**
   * Unload the config library
   */
  OFC_CORE_LIB OFC_VOID
  ofc_persist_unload (OFC_VOID) ;

#if defined(__cplusplus)
}
#endif
#endif
