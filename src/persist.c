/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/libc.h"
#include "ofc/net.h"
#include "ofc/lock.h"
#include "ofc/queue.h"
#include "ofc/event.h"
#include "ofc/thread.h"
#include "ofc/path.h"

#include "ofc/heap.h"

#if defined(OFC_PERSIST)
#include "ofc/dom.h"
#endif
#include "ofc/persist.h"

#include "ofc/file.h"

#if defined(OFC_PERSIST)
static OFC_CHAR *strmode[BLUE_CONFIG_MODE_MAX] =
  {
    "BMODE", 
    "PMODE", 
    "MMODE", 
    "HMODE"
  } ;

static OFC_CHAR *striconfigtype[BLUE_CONFIG_ICONFIG_NUM] =
  {
    "auto", 
    "manual" 
  } ;
#endif

/*
 * Who to notify on configuration events
 */
static OFC_HANDLE event_queue ;

typedef struct 
{
  OFC_UINT16 num_dns ;
  OFC_IPADDR *dns;		/* This structure can be allocated */
				/* dynamically with a larger array bound */
} DNSLIST ;

/**
 * Network Configuration
 *
 * This defines the format of the interface configuration that we maintain
 */
typedef struct
{
  BLUE_CONFIG_MODE bp_netbios_mode ;
  OFC_IPADDR ipaddress ;
  OFC_IPADDR bcast ;
  OFC_IPADDR mask ;
  OFC_VOID *private ;
  OFC_UINT16 num_wins ;
  OFC_IPADDR *winslist ;	/* This structure can be allocated */
				/* dynamically with a larger array bound */
  OFC_CHAR *master ;
} BLUE_CONFIG_ICONFIG ;

typedef struct
{
  OFC_LOCK *config_lock ;
  OFC_TCHAR *bp_workstation_name ;
  OFC_TCHAR *bp_workstation_domain ;
  OFC_TCHAR *bp_workstation_desc ;
  BLUE_CONFIG_ICONFIG_TYPE bp_iconfig_type ;
  OFC_UINT16 bp_interface_count ;
  OFC_UINT16 bp_ipv4_interface_count ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  DNSLIST bp_netbios_dns ;

  OFC_BOOL enableAutoIP ;
  OFC_BOOL loaded ;

  OFC_UUID uuid ;
  OFC_UINT32 update_count ;
} BLUE_CONFIG ;

static OFC_VOID BlueConfigLock(OFC_VOID) ;
static OFC_VOID BlueConfigUnlock(OFC_VOID) ;

static BLUE_CONFIG *g_config = NULL;

OFC_CORE_LIB OFC_VOID
BlueSetConfig (BLUE_CONFIG *configp)
{
  g_config = configp;
}

OFC_CORE_LIB BLUE_CONFIG *
BlueGetConfig (OFC_VOID)
{
  BLUE_CONFIG *ret ;

  ret = g_config;
  return (ret) ;
}

#if defined(OFC_PERSIST)
static OFC_DOMDocument *BlueConfigMakeDOM (OFC_VOID) ;
static OFC_VOID BlueConfigParseDOM (OFC_DOMNode *bpconfig_dom) ;

static OFC_DOMDocument *
BlueConfigMakeDOM (OFC_VOID)
{
  OFC_DOMDocument *doc ;
  OFC_DOMNode *node ;
  OFC_DOMNode *bpconfig_node ;
  OFC_DOMNode *ip_node ;
  OFC_DOMNode *interfaces_node ;
  OFC_DOMNode *interface_node ;
  OFC_DOMNode *dns_node ;
  OFC_DOMNode *drives_node ;
  OFC_DOMNode *wins_node ;
  OFC_DOMNode *map_node ;
  OFC_BOOL error_state ;
  OFC_CHAR *cstr ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  OFC_IPADDR *iparray ;
  OFC_INT i ;
  OFC_INT j ;
  BLUE_CONFIG *BlueConfig ;
  OFC_CHAR ip_addr[IP6STR_LEN] ;

  doc = OFC_NULL ;
  BlueConfig = BlueGetConfig() ;

  if (BlueConfig != OFC_NULL)
    {
      error_state = OFC_FALSE ;

      doc = ofc_dom_create_document(OFC_NULL, OFC_NULL, OFC_NULL) ;
      if (doc != OFC_NULL)
	{
	  node = ofc_dom_create_processing_instruction
              (doc, "xml", "version=\"1.0\" encoding=\"utf-8\"") ;
	  if (node != OFC_NULL)
          ofc_dom_append_child(doc, node) ;
	  else
	    error_state = OFC_TRUE ;
	}
      
      bpconfig_node = OFC_NULL ;
      if (!error_state)
	{
	  bpconfig_node = ofc_dom_create_element(doc, "of_core") ;
	  if (bpconfig_node != OFC_NULL)
	    {
            ofc_dom_set_attribute(bpconfig_node, "version", "0.0") ;
            ofc_dom_append_child(doc, bpconfig_node) ;
	    }
	  else
	    error_state = OFC_TRUE ;
	}

      if (!error_state)
	{
	  cstr = 
	    ofc_tstr2cstr (BlueConfig->bp_workstation_name) ;
	  node = ofc_dom_create_element_cdata (doc, "devicename", cstr) ;
	  ofc_free (cstr) ;
	  if (node != OFC_NULL)
          ofc_dom_append_child(bpconfig_node, node) ;
	  else
	    error_state = OFC_TRUE ;
	}

      if (!error_state)
	{
	  cstr = ofc_malloc (UUID_STR_LEN + 1) ;
	  ofc_uuidtoa (BlueConfig->uuid, cstr) ;
	  node = ofc_dom_create_element_cdata (doc, "uuid", cstr) ;
	  ofc_free (cstr) ;
	  if (node != OFC_NULL)
          ofc_dom_append_child(bpconfig_node, node) ;
	  else
	    error_state = OFC_TRUE ;
	}

      if (!error_state)
	{
	  cstr = 
	    ofc_tstr2cstr (BlueConfig->bp_workstation_desc) ;
	  node = 
	    ofc_dom_create_element_cdata (doc, "description", cstr) ;
	  ofc_free (cstr) ;
	  if (node != OFC_NULL)
          ofc_dom_append_child(bpconfig_node, node) ;
	  else
	    error_state = OFC_TRUE ;
	}

      ip_node = OFC_NULL ;
      if (!error_state)
	{
	  ip_node = ofc_dom_create_element(doc, "ip") ;
	  if (ip_node != OFC_NULL)
          ofc_dom_append_child(bpconfig_node, ip_node) ;
	  else
	    error_state = OFC_TRUE ;
	}
      
      if (!error_state)
	{
	  node = ofc_dom_create_element_cdata (doc, "autoip",
					    BlueConfig->enableAutoIP ? 
					    "yes" : "no") ;
	  if (node != OFC_NULL)
          ofc_dom_append_child(ip_node, node) ;
	  else
	    error_state = OFC_TRUE ;
	}
      
      interfaces_node = OFC_NULL ;
      if (!error_state)
	{
	  interfaces_node = ofc_dom_create_element(doc, "interfaces") ;

	  if (interfaces_node != OFC_NULL)
          ofc_dom_append_child(ip_node, interfaces_node) ;
	  else
	    error_state = OFC_TRUE ;
	}
      
      if (!error_state)
	{
	  if (BlueConfig->bp_iconfig_type < BLUE_CONFIG_ICONFIG_NUM)
	    {
	      node = 
		ofc_dom_create_element_cdata (doc, "config",
                                      striconfigtype
					   [BlueConfig->bp_iconfig_type]) ;
	    }
	  else
	    {
	      node = 
		ofc_dom_create_element_cdata (doc, "config",
                                      striconfigtype
					   [BLUE_CONFIG_ICONFIG_AUTO]) ;
	    }

	  if (node != OFC_NULL)
          ofc_dom_append_child(interfaces_node, node) ;
	  else
	    error_state = OFC_TRUE ;
	}
      
      if (BlueConfig->bp_iconfig_type != BLUE_CONFIG_ICONFIG_AUTO)
	{
	  for (i = 0 ; 
	       !error_state && i < BlueConfig->bp_interface_count ; 
	       i++)
	    {
	      interface_node = ofc_dom_create_element(doc, "interface") ;

	      if (interface_node != OFC_NULL)
		{
            ofc_dom_append_child(interfaces_node, interface_node) ;

		  bp_interface_config = BlueConfig->bp_interface_config ;

		  node = ofc_dom_create_element_cdata
		    (doc, "ipaddress",
             ofc_ntop (&bp_interface_config[i].ipaddress,
                       ip_addr, IP6STR_LEN)) ;

		  if (node != OFC_NULL)
              ofc_dom_append_child(interface_node, node) ;
		  else
		    error_state = OFC_TRUE ;

		  if (!error_state)
		    {
		      node = ofc_dom_create_element_cdata
			(doc, "bcast",
             ofc_ntop (&bp_interface_config[i].bcast,
                       ip_addr, IP6STR_LEN)) ;

		      if (node != OFC_NULL)
                  ofc_dom_append_child(interface_node, node) ;
		      else
			error_state = OFC_TRUE ;
		    }

		  if (!error_state)
		    {
		      node = ofc_dom_create_element_cdata
			(doc, "mask",
             ofc_ntop (&bp_interface_config[i].mask,
                       ip_addr, IP6STR_LEN)) ;

		      if (node != OFC_NULL)
                  ofc_dom_append_child(interface_node, node) ;
		      else
			error_state = OFC_TRUE ;
		    }

		  if (!error_state)
		    {
		      node = ofc_dom_create_element_cdata
			(doc, "mode", 
			 strmode[bp_interface_config[i].bp_netbios_mode]) ;

		      if (node != OFC_NULL)
                  ofc_dom_append_child(interface_node, node) ;
		      else
			error_state = OFC_TRUE ;
		    }

		  if (!error_state)
		    {
		      wins_node = OFC_NULL ;
		      wins_node = ofc_dom_create_element(doc, "winslist") ;
		      if (wins_node != OFC_NULL)
                  ofc_dom_append_child(interface_node, wins_node) ;
		      else
			error_state = OFC_TRUE ;
      
		      iparray = bp_interface_config[i].winslist ;
		      for (j = 0 ; 
			   j < bp_interface_config[i].num_wins &&
			     !error_state ;
			   j++)
			{
			  node = ofc_dom_create_element_cdata
			    (doc, "wins",
                 ofc_ntop (&iparray[j], ip_addr, IP6STR_LEN)) ;

			  if (node != OFC_NULL)
                  ofc_dom_append_child(wins_node, node) ;
			  else
			    error_state = OFC_TRUE ;
			}
		    }

		  if (!error_state)
		    {
		      node = ofc_dom_create_element_cdata
			(doc, "master", bp_interface_config[i].master) ;

		      if (node != OFC_NULL)
                  ofc_dom_append_child(interface_node, node) ;
		      else
			error_state = OFC_TRUE ;
		    }
		}
	    }
	}

      dns_node = OFC_NULL ;
      if (!error_state)
	{
	  dns_node = ofc_dom_create_element(doc, "dnslist") ;
	  if (dns_node != OFC_NULL)
          ofc_dom_append_child(ip_node, dns_node) ;
	  else
	    error_state = OFC_TRUE ;
	}
      
      for (i = 0 ; 
	   !error_state && i < BlueConfig->bp_netbios_dns.num_dns ; 
	   i++)
	{
	  iparray = BlueConfig->bp_netbios_dns.dns ;
	  node = ofc_dom_create_element_cdata
	    (doc, "dns", ofc_ntop (&iparray[i], ip_addr, IP6STR_LEN)) ;
	  if (node != OFC_NULL)
          ofc_dom_append_child(dns_node, node) ;
	  else
	    error_state = OFC_TRUE ;
	}

      if (!error_state)
	{
	  OFC_CTCHAR *prefix ;
	  OFC_CTCHAR *desc ;
	  OFC_PATH *path ;
	  OFC_BOOL thumbnail ;

	  drives_node = ofc_dom_create_element(doc, "drives") ;
	  if (drives_node != OFC_NULL)
          ofc_dom_append_child(bpconfig_node, drives_node) ;
	  else
	    error_state = OFC_TRUE ;
      
	  for (i = 0 ; i < OFC_MAX_MAPS && !error_state; i++)
	    {
	      ofc_path_get_mapW (i, &prefix, &desc, &path, &thumbnail) ;
	      if (prefix != OFC_NULL && ofc_path_remote(path))
		{
		  map_node = ofc_dom_create_element(doc, "map") ;
		  if (map_node != OFC_NULL)
              ofc_dom_append_child(drives_node, map_node) ;
		  else
		    error_state = OFC_TRUE ;
		  
		  if (!error_state)
		    {
		      OFC_CHAR *cprefix ;
		      cprefix = ofc_tstr2cstr (prefix) ;
		      node = ofc_dom_create_element_cdata
			(doc, "drive", cprefix) ;
		      if (node != OFC_NULL)
                  ofc_dom_append_child(map_node, node) ;
		      else
			error_state = OFC_TRUE ;
		      ofc_free (cprefix) ;
		    }
      
		  if (!error_state)
		    {
		      OFC_CHAR *cdesc ;
		      cdesc = ofc_tstr2cstr (desc) ;
		      node = ofc_dom_create_element_cdata
			(doc, "description", cdesc) ;
		      if (node != OFC_NULL)
                  ofc_dom_append_child(map_node, node) ;
		      else
			error_state = OFC_TRUE ;
		      ofc_free (cdesc) ;
		    }
      
		  if (!error_state)
		    {
		      OFC_SIZET rem ;
		      OFC_SIZET size ;
		      OFC_CHAR *filename ;

		      rem = 0 ;
		      size = ofc_path_printA (path, OFC_NULL, &rem) ;
		      filename = ofc_malloc (size + 1) ;

		      rem = size+1 ;
		      ofc_path_printA (path, &filename, &rem) ;
		      node = ofc_dom_create_element_cdata (doc, "path", filename) ;
		      ofc_free (filename) ;

		      if (node != OFC_NULL)
                  ofc_dom_append_child(map_node, node) ;
		      else
			error_state = OFC_TRUE ;
		    }

		  if (!error_state)
		    {
		      node = ofc_dom_create_element_cdata (doc, "thumbnail",
							thumbnail ?
							"yes" : "no") ;
		      if (node != OFC_NULL)
                  ofc_dom_append_child(map_node, node) ;
		      else
			error_state = OFC_TRUE ;
		    }
		}
	    }
	}

      if (error_state && doc != OFC_NULL)
	{
	  ofc_dom_destroy_document (doc) ;
	  doc = OFC_NULL ;
	}
    }
  return (doc) ;
}
	      
static OFC_VOID
BlueConfigParseDOM (OFC_DOMNode *bpconfig_dom)
{
  OFC_BOOL error_state ;
  OFC_DOMNode *bpconfig_node ;
  OFC_DOMNode *ip_node ;
  OFC_DOMNode *interfaces_node ;
  OFC_DOMNode *dns_node ;
  OFC_DOMNodelist *dns_nodelist ;
  OFC_DOMNode *wins_node ;
  OFC_DOMNodelist *wins_nodelist ;
  OFC_DOMNode *drives_node ;
  OFC_DOMNodelist *drives_nodelist ;
  OFC_DOMNode *map_node ;
  OFC_DOMNodelist *interface_nodelist ;
  OFC_CHAR *version ;
  OFC_CHAR *value ;
  OFC_INT i ;
  OFC_INT j ;
  OFC_INT num_wins ;
  OFC_IPADDR ipaddress ;
  OFC_IPADDR bcast ;
  OFC_IPADDR mask ;
  BLUE_CONFIG_MODE netbios_mode ;
  BLUE_CONFIG_ICONFIG_TYPE itype ;
  OFC_TCHAR *tstr ;
  OFC_CCHAR *cstr ;
  OFC_IPADDR *iparray ;
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      error_state = OFC_FALSE ;

      bpconfig_node = ofc_dom_get_element (bpconfig_dom, "of_core") ;

      if (bpconfig_node == OFC_NULL)
	error_state = OFC_TRUE ;

      if (!error_state)
	{
	  version = ofc_dom_get_attribute(bpconfig_node, "version") ;
	  if (version != OFC_NULL)
	    {
	      if (ofc_strcmp (version, "0.0") != 0)
		error_state = OFC_TRUE ;
	    }
	  else
	    error_state = OFC_TRUE ;
	}

      if (!error_state)
	{
	  value = ofc_dom_get_element_cdata(bpconfig_dom, "devicename") ;
	  if (value != OFC_NULL)
	    {
	      ofc_printf ("Device Name: %s\n", value) ;
	      tstr = ofc_cstr2tstr (value) ;
	      BlueConfig->bp_workstation_name = tstr ;
	    }

	  value = ofc_dom_get_element_cdata(bpconfig_dom, "uuid") ;
	  if (value != OFC_NULL)
	    {
	      ofc_atouuid (value, BlueConfig->uuid) ;
	    }

	  value = ofc_dom_get_element_cdata(bpconfig_dom, "description") ;
	  if (value != OFC_NULL)
	    {
	      tstr = ofc_cstr2tstr (value) ;
	      BlueConfig->bp_workstation_desc = tstr ;
	    }
	  ip_node = ofc_dom_get_element (bpconfig_dom, "ip") ;
	  if (ip_node != OFC_NULL)
	    {
	      value = ofc_dom_get_element_cdata(ip_node, "autoip") ;
	      if (value != OFC_NULL)
		{
		  if (ofc_strcmp (value, "yes") == 0)
		    BlueConfig->enableAutoIP = OFC_TRUE ;
		  else
		    BlueConfig->enableAutoIP = OFC_FALSE ;
		}

	      interfaces_node = 
		ofc_dom_get_element (bpconfig_dom, "interfaces") ;
	      if (interfaces_node != OFC_NULL)
		{
		  itype = BLUE_CONFIG_ICONFIG_MANUAL ;
		  value = ofc_dom_get_element_cdata(interfaces_node, "config") ;
		  if (value != OFC_NULL)
		    {
		      if (ofc_strcmp (value, "auto") == 0)
			itype = BLUE_CONFIG_ICONFIG_AUTO ;
		    }

		  BlueConfigSetInterfaceType (itype) ;

		  if (BlueConfig->bp_iconfig_type != BLUE_CONFIG_ICONFIG_AUTO)
		    {
		      interface_nodelist = 
			ofc_dom_get_elements_by_tag_name (interfaces_node,
                                              "interface") ;
		      if (interface_nodelist != OFC_NULL)
			{
			  for (i = 0 ; interface_nodelist[i] != OFC_NULL ;
			       i++) ;

			  BlueConfigSetInterfaceCount (i) ;

			  for (i = 0 ; i < BlueConfigInterfaceCount() ; i++)
			    {
			      value =
                          ofc_dom_get_element_cdata(interface_nodelist[i],
                                                    "ipaddress") ;
			      if (value != OFC_NULL)
				ofc_pton (value, &ipaddress) ;
			      else
				{
				  ipaddress.ip_version = OFC_FAMILY_IP ;
				  ipaddress.u.ipv4.addr = OFC_INADDR_NONE ;
				}
			      value =
                          ofc_dom_get_element_cdata(interface_nodelist[i],
                                                    "bcast") ;
			      if (value != OFC_NULL)
				ofc_pton (value, &bcast) ;
			      else
				{
				  bcast.ip_version = OFC_FAMILY_IP ;
				  bcast.u.ipv4.addr = OFC_INADDR_BROADCAST ;
				}
			      value =
                          ofc_dom_get_element_cdata(interface_nodelist[i],
                                                    "mask") ;
			      if (value != OFC_NULL)
				ofc_pton (value, &mask) ;
			      else
				{
				  mask.ip_version = OFC_FAMILY_IP ;
				  mask.u.ipv4.addr = OFC_INADDR_ANY ;
				}

			      netbios_mode = OFC_DEFAULT_NETBIOS_MODE ;
			      value =
                          ofc_dom_get_element_cdata(interface_nodelist[i],
                                                    "mode") ;
			      if (value != OFC_NULL)
				{
				  for (j = 0 ; 
				       j < 4 &&
                               ofc_strcmp (value,
                                           strmode[j]) != 0 ;
				       j++) ;
				  if (j < 4)
				    netbios_mode = j ;
				}

			      iparray = OFC_NULL ;
			      num_wins = 0 ;
			      wins_node = 
				ofc_dom_get_element (interface_nodelist[i],
                                     "winslist") ;
			      if (wins_node != OFC_NULL)
				{
				  wins_nodelist = 
				    ofc_dom_get_elements_by_tag_name (wins_node,
                                                      "wins") ;
				  if (wins_nodelist != OFC_NULL)
				    {
				      for (num_wins = 0 ;
                           wins_nodelist[num_wins] !=
                           OFC_NULL ;
					   num_wins++) ;
				      iparray = 
					ofc_malloc (sizeof (OFC_IPADDR) *
                                num_wins) ;
				      for (j = 0 ; j < num_wins ; j++)
					{
					  value =
                              ofc_dom_get_cdata(wins_nodelist[j]) ;
					  ofc_pton (value, &iparray[j]) ;
					}
				      ofc_dom_destroy_node_list (wins_nodelist) ;
				    }
				}

			      value =
                          ofc_dom_get_element_cdata(interface_nodelist[i],
                                                    "master") ;

			      BlueConfigSetInterfaceConfig (i, netbios_mode, 
							    &ipaddress, 
							    &bcast, 
							    &mask,
							    value,
							    num_wins,
							    iparray) ;
			      ofc_free (iparray) ;
			    }
			  ofc_dom_destroy_node_list (interface_nodelist) ;
			}
		    }
		}

	      dns_node = ofc_dom_get_element (ip_node, "dnslist") ;
	      if (dns_node != OFC_NULL)
		{
		  dns_nodelist = 
		    ofc_dom_get_elements_by_tag_name (dns_node, "dns") ;
		  if (dns_nodelist != OFC_NULL)
		    {
		      for (i = 0 ; dns_nodelist[i] != OFC_NULL ; i++) ;
		      BlueConfig->bp_netbios_dns.num_dns = i ;
		      if (BlueConfig->bp_netbios_dns.dns !=
                  OFC_NULL)
			ofc_free (BlueConfig->bp_netbios_dns.dns);
		      iparray = ofc_malloc (sizeof (OFC_IPADDR) * i) ;
		      BlueConfig->bp_netbios_dns.dns = iparray;
		      for (i = 0 ; i < BlueConfig->bp_netbios_dns.num_dns ; 
			   i++)
			{
			  value = ofc_dom_get_cdata(dns_nodelist[i]) ;
			  ofc_pton (value, &iparray[i]) ;
			}
		      ofc_dom_destroy_node_list (dns_nodelist) ;
		    }
		}
	    }

	  drives_node = ofc_dom_get_element (bpconfig_dom, "drives") ;
	  if (drives_node != OFC_NULL)
	    {
	      drives_nodelist = 
		ofc_dom_get_elements_by_tag_name (drives_node, "map") ;
	      if (drives_nodelist != OFC_NULL)
		{
		  for (i = 0 ; drives_nodelist[i] != OFC_NULL ; i++)
		    {
		      OFC_LPCSTR lpBookmark ;
		      OFC_LPCSTR lpDesc ;
		      OFC_LPCSTR lpFile ;
		      OFC_PATH *path ;
		      OFC_BOOL thumbnail = OFC_FALSE ;

		      map_node = drives_nodelist[i] ;
		      lpBookmark =
                      ofc_dom_get_element_cdata(map_node, "drive") ;
		      lpDesc =
                      ofc_dom_get_element_cdata(map_node, "description") ;

		      value =
                      ofc_dom_get_element_cdata(map_node, "thumbnail") ;
		      if (value != OFC_NULL)
			{
			  if (ofc_strcmp (value, "yes") == 0)
			    thumbnail = OFC_TRUE ;
			  else
			    thumbnail = OFC_FALSE ;
			}

		      if (lpDesc == OFC_NULL)
			lpDesc = "Remote Share" ;
		      lpFile = ofc_dom_get_element_cdata(map_node, "path") ;
		      if (lpBookmark != OFC_NULL && lpFile != OFC_NULL)
			{
			  path = ofc_path_createA (lpFile) ;
			  if (ofc_path_remote(path))
			    {
			      if (!ofc_path_add_mapA (lpBookmark, lpDesc,
                                          path, OFC_FST_SMB,
                                          thumbnail))
				ofc_path_delete (path) ;
			    }
			  else
			    {
			      ofc_path_delete (path) ;
			    }
			}
		    }
		  ofc_dom_destroy_node_list (drives_nodelist) ;
		}
	    }
	  BlueConfig->loaded = OFC_TRUE ;
	}
    }
}
#endif
	      
static OFC_VOID
BlueConfigLock(OFC_VOID)
{
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    ofc_lock (BlueConfig->config_lock) ;
}

static OFC_VOID
BlueConfigUnlock(OFC_VOID)
{
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    ofc_unlock (BlueConfig->config_lock) ;
}

#if defined(OFC_PERSIST)
OFC_CORE_LIB OFC_VOID
BlueConfigPrint (OFC_LPVOID *buf, OFC_SIZET *len)
{
  BLUE_CONFIG *BlueConfig ;
  OFC_DOMNode *bpconfig_dom ;

  *len = 0 ;
  *buf = OFC_NULL ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      BlueConfigLock() ;

      bpconfig_dom = BlueConfigMakeDOM () ;

      if (bpconfig_dom != OFC_NULL)
	{
	  *len = ofc_dom_sprint_document (OFC_NULL, 0, bpconfig_dom) ;
	  *buf = (OFC_LPVOID) ofc_malloc (*len + 1) ;
	  ofc_dom_sprint_document (*buf, *len + 1, bpconfig_dom) ;
	  ofc_dom_destroy_document(bpconfig_dom);
	}
      BlueConfigUnlock() ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueConfigSave (OFC_LPCTSTR lpFileName)
{
  OFC_LPVOID buf ;
  OFC_HANDLE xml ;
  OFC_DWORD dwLen ;
  OFC_SIZET len ;

  BlueConfigPrint (&buf, &len) ;

  if (buf != OFC_NULL)
    {
      dwLen = (OFC_DWORD) len ;
      xml = OfcCreateFileW (lpFileName,
                            OFC_GENERIC_WRITE,
                            OFC_FILE_SHARE_READ,
                            OFC_NULL,
                            OFC_CREATE_ALWAYS,
                            OFC_FILE_ATTRIBUTE_NORMAL,
                            OFC_HANDLE_NULL) ;

      if (xml != OFC_INVALID_HANDLE_VALUE)
	{
	  OfcWriteFile (xml, buf, dwLen, &dwLen, OFC_HANDLE_NULL) ;
	  OfcCloseHandle (xml) ;
	}

      ofc_free (buf) ;
    }
}

typedef struct
{
  OFC_HANDLE handle ;
} FILE_CONTEXT ;

static OFC_SIZET
readFile (OFC_VOID *context, OFC_LPVOID buf, OFC_DWORD bufsize)
{
  FILE_CONTEXT *fileContext ;
  OFC_DWORD bytes_read ;
  OFC_SIZET ret ;

  fileContext = (FILE_CONTEXT *) context ;

  if (!OfcReadFile (fileContext->handle, buf, bufsize,
                    &bytes_read, OFC_HANDLE_NULL))
    ret = -1 ;
  else
    ret = bytes_read ;

  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueConfigLoad (OFC_LPCTSTR lpFileName)
{
  FILE_CONTEXT *fileContext ;
  OFC_DOMNode *bpconfig_dom ;
  
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      BlueConfigLock() ;

      fileContext = (FILE_CONTEXT *) ofc_malloc (sizeof (FILE_CONTEXT)) ;
  
      fileContext->handle = OfcCreateFileW (lpFileName,
                                            OFC_GENERIC_READ,
                                            OFC_FILE_SHARE_READ,
                                            OFC_NULL,
                                            OFC_OPEN_EXISTING,
                                            OFC_FILE_ATTRIBUTE_NORMAL,
                                            OFC_HANDLE_NULL) ;

      bpconfig_dom = OFC_NULL ;

      if (fileContext->handle != OFC_INVALID_HANDLE_VALUE &&
          fileContext->handle != OFC_HANDLE_NULL)
	{
	  bpconfig_dom = ofc_dom_load_document (readFile,
                                            (OFC_VOID *) fileContext) ;

	  OfcCloseHandle (fileContext->handle) ;
	}
      else
	ofc_printf ("Could not load config file %S\n", lpFileName) ;

      ofc_free (fileContext) ;
  
      if (bpconfig_dom != OFC_NULL)
	{
	  BlueConfigFree ();
	  BlueConfigParseDOM (bpconfig_dom) ;
	  ofc_dom_destroy_document (bpconfig_dom) ;
	}

      BlueConfigUnlock() ;
    }
}
#endif

OFC_CORE_LIB OFC_VOID
BlueConfigFree (OFC_VOID)
{
  OFC_INT i ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      if (BlueConfig->bp_workstation_name != OFC_NULL)
	{
	  ofc_free (BlueConfig->bp_workstation_name) ;
	  BlueConfig->bp_workstation_name = OFC_NULL ;
	}
      if (BlueConfig->bp_workstation_domain != OFC_NULL)
	{
	  ofc_free (BlueConfig->bp_workstation_domain) ;
	  BlueConfig->bp_workstation_domain = OFC_NULL ;
	}
      if (BlueConfig->bp_workstation_desc != OFC_NULL)
	{
	  ofc_free (BlueConfig->bp_workstation_desc) ;
	  BlueConfig->bp_workstation_desc = OFC_NULL ;
	}
      if (BlueConfig->bp_interface_config != OFC_NULL)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;

	  for (i = 0 ; i < BlueConfig->bp_interface_count ; i++)
	    {
	      if (bp_interface_config[i].winslist != OFC_NULL)
		{
		  ofc_free (bp_interface_config[i].winslist) ;
		  bp_interface_config[i].winslist = OFC_NULL ;
		}
	      bp_interface_config[i].num_wins = 0 ;
	      ofc_free (bp_interface_config[i].master) ;
	    }
	  ofc_free (BlueConfig->bp_interface_config) ;
	  BlueConfig->bp_interface_config = OFC_NULL ;
	  BlueConfig->bp_interface_count = 0 ;
	}

      if (BlueConfig->bp_netbios_dns.dns != OFC_NULL)
	{
	  ofc_free (BlueConfig->bp_netbios_dns.dns) ;
	  BlueConfig->bp_netbios_dns.num_dns = 0 ;
	  BlueConfig->bp_netbios_dns.dns = OFC_NULL ;
	}
    }
}

OFC_CORE_LIB OFC_VOID
BlueConfigDefault (OFC_VOID)
{
  OFC_TCHAR *tstr ;

  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      BlueConfigLock() ;

      BlueConfigFree() ;

      tstr = ofc_cstr2tstr (OFC_DEFAULT_NAME) ;

      BlueConfig->bp_workstation_name = tstr;

      tstr = ofc_cstr2tstr (OFC_DEFAULT_DESCR) ;
      BlueConfig->bp_workstation_desc = tstr;
  
      BlueConfig->bp_interface_count = 0 ;

      BlueConfigSetInterfaceType (BLUE_CONFIG_ICONFIG_AUTO) ;

      tstr = ofc_cstr2tstr (OFC_DEFAULT_DOMAIN) ;
      BlueConfig->bp_workstation_domain = tstr;
    
      BlueConfigUnlock() ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueConfigInit(OFC_VOID)
{
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;

  if (BlueConfig == OFC_NULL)
    {
      BlueConfig = ofc_malloc (sizeof (BLUE_CONFIG)) ;
      if (BlueConfig != OFC_NULL)
	{
	  /*
	   * Clear the configuration structure
	   */
	  ofc_memset (BlueConfig, '\0', sizeof (BLUE_CONFIG)) ;

	  BlueConfig->update_count = 0 ;
	  BlueConfig->config_lock = ofc_lock_init() ;
	  BlueConfig->loaded = OFC_FALSE ;
	  BlueConfig->bp_workstation_name = OFC_NULL ;
	  BlueConfig->bp_workstation_domain = OFC_NULL ;
	  BlueConfig->bp_workstation_desc = OFC_NULL ;
	  BlueConfig->bp_interface_config = OFC_NULL ;
	  BlueConfig->bp_netbios_dns.dns = OFC_NULL ;

	  BlueSetConfig (BlueConfig) ;

	  BlueConfigDefault() ;
	}
    }
  event_queue = BlueQcreate() ;
}

OFC_CORE_LIB OFC_VOID
BlueConfigUnload (OFC_VOID)
{
  OFC_HANDLE hEvent ;
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      for (hEvent = (OFC_HANDLE) BlueQdequeue (event_queue) ;
           hEvent != OFC_HANDLE_NULL ;
	   hEvent = (OFC_HANDLE) BlueQdequeue (event_queue))
	{
	  ofc_event_destroy (hEvent) ;
	}
      BlueQdestroy (event_queue) ;

      BlueConfigFree() ;

      ofc_lock_destroy(BlueConfig->config_lock);

      ofc_free (BlueConfig) ;
      BlueSetConfig (OFC_NULL) ;
    }
}

OFC_CORE_LIB OFC_BOOL
BlueConfigLoaded (OFC_VOID)
{
  BLUE_CONFIG *BlueConfig ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      ret = BlueConfig->loaded ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueConfigResetInterfaceConfig (OFC_VOID)
{
  OFC_INT i ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      if (BlueConfig->bp_interface_config != OFC_NULL)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;
	  for (i = 0 ; i < BlueConfig->bp_interface_count ; i++)
	    {
	      if (bp_interface_config[i].winslist != OFC_NULL)
		{
		  ofc_free (bp_interface_config[i].winslist) ;
		  bp_interface_config[i].winslist = OFC_NULL ;
		}
	      bp_interface_config[i].num_wins = 0 ;
	      ofc_free (bp_interface_config[i].master) ;
	    }
	  ofc_free (bp_interface_config) ;
	}
      BlueConfig->bp_interface_count = 0 ;
      BlueConfig->bp_interface_config = OFC_NULL ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueConfigSetInterfaceType (BLUE_CONFIG_ICONFIG_TYPE itype)
{
  BLUE_CONFIG_MODE netbios_mode ;
  OFC_INT i ;
  OFC_IPADDR ipaddress ;
  OFC_IPADDR bcast ;
  OFC_IPADDR mask ;

  BLUE_CONFIG *BlueConfig ;
  OFC_INT num_wins ;
  OFC_IPADDR *winslist ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      BlueConfigResetInterfaceConfig () ;
  
      BlueConfig->bp_iconfig_type = itype ;
  
      if (BlueConfig->bp_iconfig_type == BLUE_CONFIG_ICONFIG_AUTO)
	{
	  BlueConfigSetInterfaceCount (ofc_net_interface_count()) ;
	  for (i = 0 ; i < BlueConfig->bp_interface_count ; i++)
	    {
	      netbios_mode = OFC_DEFAULT_NETBIOS_MODE ;
	      ofc_net_interface_addr (i, &ipaddress, &bcast, &mask) ;
	      ofc_net_interface_wins (i, &num_wins, &winslist) ;
	      BlueConfigSetInterfaceConfig (i, netbios_mode,
                                        &ipaddress, &bcast, &mask,
                                        OFC_NULL,
                                        num_wins, winslist) ;
	      ofc_free (winslist) ;
	    }
	}
    }
}

OFC_CORE_LIB BLUE_CONFIG_ICONFIG_TYPE
BlueConfigInterfaceConfig (OFC_VOID)
{
  BLUE_CONFIG *BlueConfig ;
  BLUE_CONFIG_ICONFIG_TYPE itype ;

  itype = BLUE_CONFIG_ICONFIG_AUTO ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    itype = BlueConfig->bp_iconfig_type ;
  return (itype) ;
}

OFC_CORE_LIB OFC_INT
BlueConfigWINSCount(OFC_INT index)
{
  OFC_INT ret ;
  BLUE_CONFIG *BlueConfig ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;

  ret = 0 ;
  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      if (index < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;
	  ret = bp_interface_config[index].num_wins ;
	}
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueConfigWINSAddr(OFC_INT xface, OFC_INT index, OFC_IPADDR *addr)
{
  OFC_IPADDR *iparray ;
  BLUE_CONFIG *BlueConfig ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;

  addr->ip_version = OFC_FAMILY_IP ;
  addr->u.ipv4.addr = OFC_INADDR_NONE ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      if (xface < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;

	  if (index < bp_interface_config[xface].num_wins)
	    {
	      iparray = bp_interface_config[xface].winslist ;
	      ofc_memcpy (addr, &iparray[index], sizeof (OFC_IPADDR)) ;
	    }
	}
    }
}

OFC_CORE_LIB OFC_VOID
BlueConfigSetInterfaceCount (OFC_INT count)
{
  OFC_INT i ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;

  BLUE_CONFIG *BlueConfig ;
  OFC_INT old_count ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      bp_interface_config = BlueConfig->bp_interface_config ;

      old_count = BlueConfig->bp_interface_count ;
      BlueConfig->bp_interface_count = count ;

      for (i = count ; i < old_count ; i++)
	{
	  if (bp_interface_config[i].winslist != OFC_NULL)
	    ofc_free (bp_interface_config[i].winslist) ;
	  bp_interface_config[i].num_wins = 0 ;
	  bp_interface_config[i].winslist = OFC_NULL ;
	  if (bp_interface_config[i].master != OFC_NULL)
	    ofc_free (bp_interface_config[i].master) ;
	  bp_interface_config[i].master = OFC_NULL ;
	}

      bp_interface_config = 
	ofc_realloc (BlueConfig->bp_interface_config,
			sizeof (BLUE_CONFIG_ICONFIG) * count) ;
      BlueConfig->bp_interface_config = bp_interface_config ;

      for (i = old_count ; i < count ; i++)
	{
	  bp_interface_config[i].master = OFC_NULL ;
	  bp_interface_config[i].winslist = OFC_NULL ;
	  bp_interface_config[i].num_wins = 0 ;
	}
    }
}

OFC_CORE_LIB OFC_VOID
BlueConfigRemoveInterfaceConfig (OFC_IPADDR *ip)
{
  OFC_INT i ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;
  OFC_INT idx ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      bp_interface_config = BlueConfig->bp_interface_config ;

      for (idx = 0 ; idx < BlueConfig->bp_interface_count &&
	     !ofc_net_is_addr_equal (&bp_interface_config[idx].ipaddress, ip) ;
	   idx++) ;
	  
      if (idx < BlueConfig->bp_interface_count)
	{
	  if (bp_interface_config[idx].winslist != OFC_NULL)
	    ofc_free (bp_interface_config[idx].winslist) ;
	  bp_interface_config[idx].num_wins = 0 ;
	  bp_interface_config[idx].winslist = OFC_NULL ;
	  if (bp_interface_config[idx].master != OFC_NULL)
	    ofc_free (bp_interface_config[idx].master) ;
	  bp_interface_config[idx].master = OFC_NULL ;

	  BlueConfig->bp_interface_count-- ;

	  for (i = idx ; i < BlueConfig->bp_interface_count ; i++)
	    {
	      bp_interface_config[i].bp_netbios_mode = 
		bp_interface_config[i+1].bp_netbios_mode ;
	      bp_interface_config[i].ipaddress = 
		bp_interface_config[i+1].ipaddress ;
	      bp_interface_config[i].bcast =
		bp_interface_config[i+1].bcast ;
	      bp_interface_config[i].mask =
		bp_interface_config[i+1].mask ;
	      bp_interface_config[i].num_wins = 
		bp_interface_config[i+1].num_wins ; 
	      bp_interface_config[i].winslist =
		bp_interface_config[i+1].winslist ;
	      bp_interface_config[i].master = 
		bp_interface_config[i+1].master ; 
	    }

	  bp_interface_config = 
	    ofc_realloc (BlueConfig->bp_interface_config,
			    sizeof (BLUE_CONFIG_ICONFIG) * 
			    BlueConfig->bp_interface_count) ;
	  BlueConfig->bp_interface_config = bp_interface_config;
	}
    }
}

OFC_CORE_LIB OFC_VOID
BlueConfigSetInterfaceConfig (OFC_INT i,
                              BLUE_CONFIG_MODE netbios_mode,
                              OFC_IPADDR *ipaddress,
                              OFC_IPADDR *bcast,
                              OFC_IPADDR *mask,
                              OFC_CHAR *master,
                              OFC_INT num_wins,
                              OFC_IPADDR *winslist)
{
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  OFC_CHAR *cstr ;
  BLUE_CONFIG *BlueConfig ;
  OFC_INT j ;
  OFC_IPADDR *iwinslist ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      if (i < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;

	  bp_interface_config[i].bp_netbios_mode = netbios_mode ;
	  if (ipaddress != OFC_NULL)
	    {
	      bp_interface_config[i].ipaddress = *ipaddress ;
	    }
	  if (bcast != OFC_NULL)
	    bp_interface_config[i].bcast = *bcast ;
	  if (mask != OFC_NULL)
	    bp_interface_config[i].mask = *mask ;

	  if (bp_interface_config[i].winslist != OFC_NULL)
	    ofc_free (bp_interface_config[i].winslist) ;
	  bp_interface_config[i].num_wins = 0 ;
	  bp_interface_config[i].winslist = OFC_NULL ;
	  if (num_wins != 0 && winslist != OFC_NULL)
	    {
	      bp_interface_config[i].num_wins = num_wins ;
	      iwinslist = ofc_malloc (sizeof (OFC_IPADDR) * num_wins) ;
	      bp_interface_config[i].winslist = iwinslist ;
	      for (j = 0 ; j < num_wins ; j++)
		iwinslist[j] = winslist[j] ;
	    }

	  if (master != OFC_NULL)
	    {
	      if (bp_interface_config[i].master != OFC_NULL)
		ofc_free (bp_interface_config[i].master) ;

	      cstr = ofc_strdup (master) ;
	      bp_interface_config[i].master = cstr ;
	    }
	}
    }
}

OFC_CORE_LIB OFC_VOID
BlueConfigSetPrivate (OFC_INT index, OFC_VOID *private)
{
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      if (index < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;
	  bp_interface_config[index].private = private ;
	}
    }
}

OFC_CORE_LIB OFC_VOID
BlueConfigPrivate (OFC_INT index, OFC_VOID **private)
{
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;

  *private = OFC_NULL ;
  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      if (index < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;
	  *private = bp_interface_config[index].private ;
	}
    }
}

OFC_CORE_LIB OFC_VOID
BlueConfigSetLocalMaster (OFC_INT index, OFC_LPCSTR local_master)
{
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  OFC_CHAR *cstr ;
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      if (index < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;

	  if (bp_interface_config[index].master != OFC_NULL)
	    ofc_free (bp_interface_config[index].master) ;
	  bp_interface_config[index].master = OFC_NULL ;
	  if (local_master != OFC_NULL)
	    {
	      cstr = ofc_strdup (local_master) ;
	      bp_interface_config[index].master = cstr ;
	    }
	}
    }
}

OFC_CORE_LIB OFC_INT
BlueConfigInterfaceCount (OFC_VOID)
{
  OFC_INT ret ;
  BLUE_CONFIG *BlueConfig ;

  ret = 0 ;
  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      ret = BlueConfig->bp_interface_count ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueConfigLocalMaster (OFC_INT index, OFC_LPCSTR *local_master)
{
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;

  *local_master = OFC_NULL ;
  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      if (index < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;
	  *local_master = bp_interface_config[index].master ;
	}
    }
}

OFC_CORE_LIB OFC_VOID
BlueConfigInterfaceAddr (OFC_INT index, OFC_IPADDR *addr,
                         OFC_IPADDR *pbcast, OFC_IPADDR *pmask)
{
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;

  addr->ip_version = OFC_FAMILY_IP ;
  addr->u.ipv4.addr = OFC_INADDR_NONE ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      if (index < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;

	  if (addr != OFC_NULL)
	    ofc_memcpy (addr, &bp_interface_config[index].ipaddress,
                    sizeof (OFC_IPADDR)) ;
	  if (pbcast != OFC_NULL)
	    ofc_memcpy (pbcast, &bp_interface_config[index].bcast,
                    sizeof (OFC_IPADDR)) ;
	  if (pmask != OFC_NULL)
	    ofc_memcpy (pmask, &bp_interface_config[index].mask,
                    sizeof (OFC_IPADDR)) ;
	}
    }
}

OFC_CORE_LIB BLUE_CONFIG_MODE
BlueConfigInterfaceMode (OFC_INT index, OFC_INT *num_wins,
                         OFC_IPADDR **winslist)
{
  BLUE_CONFIG_MODE mode ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;
  OFC_IPADDR *iwinslist ;
  OFC_INT i ;

  mode = OFC_DEFAULT_NETBIOS_MODE ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      if (index < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;
	  mode = bp_interface_config[index].bp_netbios_mode ;
	  if (num_wins != OFC_NULL)
	    {
	      *num_wins = bp_interface_config[index].num_wins ;
	      if (winslist != OFC_NULL)
		{
		  *winslist = OFC_NULL ;
		  if (*num_wins > 0)
		    {
		      *winslist = 
			ofc_malloc (sizeof (OFC_IPADDR) * *num_wins) ;
		      iwinslist = bp_interface_config[index].winslist ;
		      for (i = 0 ; i < *num_wins ; i++)
			{
			  (*winslist)[i] = iwinslist[i] ;
			}
		    }
		}
	    }
	}
    }

  return (mode) ;
}

OFC_CORE_LIB OFC_VOID
BlueConfigNodeName (OFC_LPCTSTR *name,
                    OFC_LPCTSTR *workgroup,
                    OFC_LPCTSTR *desc)
{
  BLUE_CONFIG *BlueConfig ;

  *name = OFC_NULL ;
  *workgroup = OFC_NULL ;
  *desc = OFC_NULL ;
  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      *name = BlueConfig->bp_workstation_name ;
      *workgroup = BlueConfig->bp_workstation_domain ;
      *desc = BlueConfig->bp_workstation_desc ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueConfigSetNodeName (OFC_LPCTSTR name,
                       OFC_LPCTSTR workgroup,
                       OFC_LPCTSTR desc)
{
  OFC_TCHAR *tstr ;
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      if (BlueConfig->bp_workstation_name != OFC_NULL)
	ofc_free (BlueConfig->bp_workstation_name) ;
      tstr = ofc_tstrdup(name) ;
      BlueConfig->bp_workstation_name = tstr ;
      if (BlueConfig->bp_workstation_domain != OFC_NULL)
	ofc_free (BlueConfig->bp_workstation_domain) ;
      tstr = ofc_tstrdup (workgroup) ;
      BlueConfig->bp_workstation_domain = tstr ;
      if (BlueConfig->bp_workstation_desc != OFC_NULL)
	ofc_free (BlueConfig->bp_workstation_desc) ;
      tstr = ofc_tstrdup (desc) ;
      BlueConfig->bp_workstation_desc = tstr ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueConfigUUID (OFC_UUID *uuid)
{
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      ofc_memcpy (uuid, BlueConfig->uuid, OFC_UUID_LEN) ;
    }
  else
    ofc_memset (uuid, '\0', OFC_UUID_LEN) ;
}

OFC_CORE_LIB OFC_VOID
BlueConfigSetUUID (OFC_UUID *uuid)
{
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      ofc_memcpy (BlueConfig->uuid, uuid, OFC_UUID_LEN) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueConfigRegisterUpdate (OFC_HANDLE hEvent)
{
  BlueQenqueue (event_queue, (OFC_VOID *) hEvent) ;
  /*
   * Everyone who registers get's an initial event
   */
  ofc_event_set (hEvent) ;
}

OFC_CORE_LIB OFC_VOID
BlueConfigUnregisterUpdate (OFC_HANDLE hEvent)
{
  BlueQunlink (event_queue, (OFC_VOID *) hEvent) ;
}

OFC_CORE_LIB OFC_VOID
BlueConfigNotify (OFC_VOID)
{
  OFC_HANDLE hEvent ;

  for (hEvent = (OFC_HANDLE) BlueQfirst (event_queue) ;
       hEvent != OFC_HANDLE_NULL ;
       hEvent = (OFC_HANDLE) BlueQnext (event_queue, (OFC_VOID *) hEvent))
    {
      ofc_event_set (hEvent) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueConfigUpdate (OFC_VOID)
{
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != OFC_NULL)
    {
      BlueConfigLock() ;
      BlueConfig->update_count++ ;
      BlueConfigNotify() ;
      BlueConfigUnlock() ;
    }
}

