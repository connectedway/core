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

#define __BLUE_CORE_DLL__

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

#if defined(BLUE_PARAM_PERSIST)
#include "ofc/dom.h"
#endif
#include "ofc/persist.h"

#include "ofc/file.h"

#if defined(BLUE_PARAM_PERSIST)
static BLUE_CHAR *strmode[BLUE_CONFIG_MODE_MAX] = 
  {
    "BMODE", 
    "PMODE", 
    "MMODE", 
    "HMODE"
  } ;

static BLUE_CHAR *striconfigtype[BLUE_CONFIG_ICONFIG_NUM] = 
  {
    "auto", 
    "manual" 
  } ;
#endif

/*
 * Who to notify on configuration events
 */
static BLUE_HANDLE event_queue ;

typedef struct 
{
  BLUE_UINT16 num_dns ; 
  BLUE_IPADDR *dns;		/* This structure can be allocated */
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
  BLUE_IPADDR ipaddress ;
  BLUE_IPADDR bcast ;
  BLUE_IPADDR mask ;
  BLUE_VOID *private ;
  BLUE_UINT16 num_wins ;
  BLUE_IPADDR *winslist ;	/* This structure can be allocated */
				/* dynamically with a larger array bound */
  BLUE_CHAR *master ;
} BLUE_CONFIG_ICONFIG ;

typedef struct
{
  BLUE_LOCK *config_lock ;
  BLUE_TCHAR *bp_workstation_name ;
  BLUE_TCHAR *bp_workstation_domain ;
  BLUE_TCHAR *bp_workstation_desc ;
  BLUE_CONFIG_ICONFIG_TYPE bp_iconfig_type ;
  BLUE_UINT16 bp_interface_count ;
  BLUE_UINT16 bp_ipv4_interface_count ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  DNSLIST bp_netbios_dns ;

  BLUE_BOOL enableAutoIP ;
  BLUE_BOOL loaded ;

  BLUE_UUID uuid ;
  BLUE_UINT32 update_count ;
} BLUE_CONFIG ;

static BLUE_VOID BlueConfigLock(BLUE_VOID) ;
static BLUE_VOID BlueConfigUnlock(BLUE_VOID) ;
#if defined(BLUE_PARAM_PERSIST)
static BLUE_DOMDocument *BlueConfigMakeDOM (BLUE_VOID) ;
static BLUE_VOID BlueConfigParseDOM (BLUE_DOMNode *bpconfig_dom) ;

static BLUE_CONFIG *g_config = NULL;

BLUE_CORE_LIB BLUE_VOID 
BlueSetConfig (BLUE_CONFIG *configp)
{
  g_config = configp;
}

BLUE_CORE_LIB BLUE_CONFIG *
BlueGetConfig (BLUE_VOID)
{
  BLUE_CONFIG *ret ;

  ret = g_config;
  return (ret) ;
}

static BLUE_DOMDocument *
BlueConfigMakeDOM (BLUE_VOID)
{
  BLUE_DOMDocument *doc ;
  BLUE_DOMNode *node ;
  BLUE_DOMNode *bpconfig_node ;
  BLUE_DOMNode *ip_node ;
  BLUE_DOMNode *interfaces_node ;
  BLUE_DOMNode *interface_node ;
  BLUE_DOMNode *dns_node ;
  BLUE_DOMNode *drives_node ;
  BLUE_DOMNode *wins_node ;
  BLUE_DOMNode *map_node ;
  BLUE_BOOL error_state ;
  BLUE_CHAR *cstr ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_IPADDR *iparray ;
  BLUE_INT i ;
  BLUE_INT j ;
  BLUE_CONFIG *BlueConfig ;
  BLUE_CHAR ip_addr[IP6STR_LEN] ;

  doc = BLUE_NULL ;
  BlueConfig = BlueGetConfig() ;

  if (BlueConfig != BLUE_NULL)
    {
      error_state = BLUE_FALSE ;

      doc = BlueDOMcreateDocument (BLUE_NULL, BLUE_NULL, BLUE_NULL) ;
      if (doc != BLUE_NULL)
	{
	  node = BlueDOMcreateProcessingInstruction 
	    (doc, "xml", "version=\"1.0\" encoding=\"utf-8\"") ;
	  if (node != BLUE_NULL)
	    BlueDOMappendChild (doc, node) ;
	  else
	    error_state = BLUE_TRUE ;
	}
      
      bpconfig_node = BLUE_NULL ;
      if (!error_state)
	{
	  bpconfig_node = BlueDOMcreateElement (doc, "of_core") ;
	  if (bpconfig_node != BLUE_NULL)
	    {
	      BlueDOMsetAttribute (bpconfig_node, "version", "0.0") ;
	      BlueDOMappendChild (doc, bpconfig_node) ;
	    }
	  else
	    error_state = BLUE_TRUE ;
	}

      if (!error_state)
	{
	  cstr = 
	    BlueCtstr2cstr (BlueConfig->bp_workstation_name) ;
	  node = BlueDOMcreateElementCDATA (doc, "devicename", cstr) ;
	  BlueHeapFree (cstr) ;
	  if (node != BLUE_NULL)
	    BlueDOMappendChild (bpconfig_node, node) ;
	  else
	    error_state = BLUE_TRUE ;
	}

      if (!error_state)
	{
	  cstr = BlueHeapMalloc (UUID_STR_LEN + 1) ;
	  BlueCuuidtoa (BlueConfig->uuid, cstr) ;
	  node = BlueDOMcreateElementCDATA (doc, "uuid", cstr) ;
	  BlueHeapFree (cstr) ;
	  if (node != BLUE_NULL)
	    BlueDOMappendChild (bpconfig_node, node) ;
	  else
	    error_state = BLUE_TRUE ;
	}

      if (!error_state)
	{
	  cstr = 
	    BlueCtstr2cstr (BlueConfig->bp_workstation_desc) ;
	  node = 
	    BlueDOMcreateElementCDATA (doc, "description", cstr) ;
	  BlueHeapFree (cstr) ;
	  if (node != BLUE_NULL)
	    BlueDOMappendChild (bpconfig_node, node) ;
	  else
	    error_state = BLUE_TRUE ;
	}

      ip_node = BLUE_NULL ;
      if (!error_state)
	{
	  ip_node = BlueDOMcreateElement (doc, "ip") ;
	  if (ip_node != BLUE_NULL)
	    BlueDOMappendChild (bpconfig_node, ip_node) ;
	  else
	    error_state = BLUE_TRUE ;
	}
      
      if (!error_state)
	{
	  node = BlueDOMcreateElementCDATA (doc, "autoip",
					    BlueConfig->enableAutoIP ? 
					    "yes" : "no") ;
	  if (node != BLUE_NULL)
	    BlueDOMappendChild (ip_node, node) ;
	  else
	    error_state = BLUE_TRUE ;
	}
      
      interfaces_node = BLUE_NULL ;
      if (!error_state)
	{
	  interfaces_node = BlueDOMcreateElement (doc, "interfaces") ;

	  if (interfaces_node != BLUE_NULL)
	    BlueDOMappendChild (ip_node, interfaces_node) ;
	  else
	    error_state = BLUE_TRUE ;
	}
      
      if (!error_state)
	{
	  if (BlueConfig->bp_iconfig_type < BLUE_CONFIG_ICONFIG_NUM)
	    {
	      node = 
		BlueDOMcreateElementCDATA (doc, "config",
					   striconfigtype
					   [BlueConfig->bp_iconfig_type]) ;
	    }
	  else
	    {
	      node = 
		BlueDOMcreateElementCDATA (doc, "config",
					   striconfigtype
					   [BLUE_CONFIG_ICONFIG_AUTO]) ;
	    }

	  if (node != BLUE_NULL)
	    BlueDOMappendChild (interfaces_node, node) ;
	  else
	    error_state = BLUE_TRUE ;
	}
      
      if (BlueConfig->bp_iconfig_type != BLUE_CONFIG_ICONFIG_AUTO)
	{
	  for (i = 0 ; 
	       !error_state && i < BlueConfig->bp_interface_count ; 
	       i++)
	    {
	      interface_node = BlueDOMcreateElement (doc, "interface") ;

	      if (interface_node != BLUE_NULL)
		{
		  BlueDOMappendChild (interfaces_node, interface_node) ;

		  bp_interface_config = BlueConfig->bp_interface_config ;

		  node = BlueDOMcreateElementCDATA 
		    (doc, "ipaddress", 
		     BlueNETntop (&bp_interface_config[i].ipaddress,
				  ip_addr, IP6STR_LEN)) ;

		  if (node != BLUE_NULL)
		    BlueDOMappendChild (interface_node, node) ;
		  else
		    error_state = BLUE_TRUE ;

		  if (!error_state)
		    {
		      node = BlueDOMcreateElementCDATA 
			(doc, "bcast", 
			 BlueNETntop (&bp_interface_config[i].bcast,
				      ip_addr, IP6STR_LEN)) ;

		      if (node != BLUE_NULL)
			BlueDOMappendChild (interface_node, node) ;
		      else
			error_state = BLUE_TRUE ;
		    }

		  if (!error_state)
		    {
		      node = BlueDOMcreateElementCDATA 
			(doc, "mask", 
			 BlueNETntop (&bp_interface_config[i].mask,
				      ip_addr, IP6STR_LEN)) ;

		      if (node != BLUE_NULL)
			BlueDOMappendChild (interface_node, node) ;
		      else
			error_state = BLUE_TRUE ;
		    }

		  if (!error_state)
		    {
		      node = BlueDOMcreateElementCDATA 
			(doc, "mode", 
			 strmode[bp_interface_config[i].bp_netbios_mode]) ;

		      if (node != BLUE_NULL)
			BlueDOMappendChild (interface_node, node) ;
		      else
			error_state = BLUE_TRUE ;
		    }

		  if (!error_state)
		    {
		      wins_node = BLUE_NULL ;
		      wins_node = BlueDOMcreateElement (doc, "winslist") ;
		      if (wins_node != BLUE_NULL)
			BlueDOMappendChild (interface_node, wins_node) ;
		      else
			error_state = BLUE_TRUE ;
      
		      iparray = bp_interface_config[i].winslist ;
		      for (j = 0 ; 
			   j < bp_interface_config[i].num_wins &&
			     !error_state ;
			   j++)
			{
			  node = BlueDOMcreateElementCDATA 
			    (doc, "wins", 
			 BlueNETntop (&iparray[j], ip_addr, IP6STR_LEN)) ;

			  if (node != BLUE_NULL)
			    BlueDOMappendChild (wins_node, node) ;
			  else
			    error_state = BLUE_TRUE ;
			}
		    }

		  if (!error_state)
		    {
		      node = BlueDOMcreateElementCDATA 
			(doc, "master", bp_interface_config[i].master) ;

		      if (node != BLUE_NULL)
			BlueDOMappendChild (interface_node, node) ;
		      else
			error_state = BLUE_TRUE ;
		    }
		}
	    }
	}

      dns_node = BLUE_NULL ;
      if (!error_state)
	{
	  dns_node = BlueDOMcreateElement (doc, "dnslist") ;
	  if (dns_node != BLUE_NULL)
	    BlueDOMappendChild (ip_node, dns_node) ;
	  else
	    error_state = BLUE_TRUE ;
	}
      
      for (i = 0 ; 
	   !error_state && i < BlueConfig->bp_netbios_dns.num_dns ; 
	   i++)
	{
	  iparray = BlueConfig->bp_netbios_dns.dns ;
	  node = BlueDOMcreateElementCDATA 
	    (doc, "dns", BlueNETntop (&iparray[i], ip_addr, IP6STR_LEN)) ;
	  if (node != BLUE_NULL)
	    BlueDOMappendChild (dns_node, node) ;
	  else
	    error_state = BLUE_TRUE ;
	}

      if (!error_state)
	{
	  BLUE_CTCHAR *prefix ;
	  BLUE_CTCHAR *desc ;
	  BLUE_PATH *path ;
	  BLUE_BOOL thumbnail ;

	  drives_node = BlueDOMcreateElement (doc, "drives") ;
	  if (drives_node != BLUE_NULL)
	    BlueDOMappendChild (bpconfig_node, drives_node) ;
	  else
	    error_state = BLUE_TRUE ;
      
	  for (i = 0 ; i < BLUE_PARAM_MAX_MAPS && !error_state; i++)
	    {
	      BluePathGetMapW (i, &prefix, &desc, &path, &thumbnail) ;
	      if (prefix != BLUE_NULL && BluePathRemote(path))
		{
		  map_node = BlueDOMcreateElement (doc, "map") ;
		  if (map_node != BLUE_NULL)
		    BlueDOMappendChild (drives_node, map_node) ;
		  else
		    error_state = BLUE_TRUE ;
		  
		  if (!error_state)
		    {
		      BLUE_CHAR *cprefix ;
		      cprefix = BlueCtstr2cstr (prefix) ;
		      node = BlueDOMcreateElementCDATA 
			(doc, "drive", cprefix) ;
		      if (node != BLUE_NULL)
			BlueDOMappendChild (map_node, node) ;
		      else
			error_state = BLUE_TRUE ;
		      BlueHeapFree (cprefix) ;
		    }
      
		  if (!error_state)
		    {
		      BLUE_CHAR *cdesc ;
		      cdesc = BlueCtstr2cstr (desc) ;
		      node = BlueDOMcreateElementCDATA 
			(doc, "description", cdesc) ;
		      if (node != BLUE_NULL)
			BlueDOMappendChild (map_node, node) ;
		      else
			error_state = BLUE_TRUE ;
		      BlueHeapFree (cdesc) ;
		    }
      
		  if (!error_state)
		    {
		      BLUE_SIZET rem ;
		      BLUE_SIZET size ;
		      BLUE_CHAR *filename ;

		      rem = 0 ;
		      size = BluePathPrintA (path, BLUE_NULL, &rem) ;
		      filename = BlueHeapMalloc (size+1) ;

		      rem = size+1 ;
		      BluePathPrintA (path, &filename, &rem) ;
		      node = BlueDOMcreateElementCDATA (doc, "path",filename) ;
		      BlueHeapFree (filename) ;

		      if (node != BLUE_NULL)
			BlueDOMappendChild (map_node, node) ;
		      else
			error_state = BLUE_TRUE ;
		    }

		  if (!error_state)
		    {
		      node = BlueDOMcreateElementCDATA (doc, "thumbnail",
							thumbnail ?
							"yes" : "no") ;
		      if (node != BLUE_NULL)
			BlueDOMappendChild (map_node, node) ;
		      else
			error_state = BLUE_TRUE ;
		    }
		}
	    }
	}

      if (error_state && doc != BLUE_NULL)
	{
	  BlueDOMdestroyDocument (doc) ;
	  doc = BLUE_NULL ;
	}
    }
  return (doc) ;
}
	      
static BLUE_VOID 
BlueConfigParseDOM (BLUE_DOMNode *bpconfig_dom)
{
  BLUE_BOOL error_state ;
  BLUE_DOMNode *bpconfig_node ;
  BLUE_DOMNode *ip_node ;
  BLUE_DOMNode *interfaces_node ;
  BLUE_DOMNode *dns_node ;
  BLUE_DOMNodelist *dns_nodelist ;
  BLUE_DOMNode *wins_node ;
  BLUE_DOMNodelist *wins_nodelist ;
  BLUE_DOMNode *drives_node ;
  BLUE_DOMNodelist *drives_nodelist ;
  BLUE_DOMNode *map_node ;
  BLUE_DOMNodelist *interface_nodelist ;
  BLUE_CHAR *version ;
  BLUE_CHAR *value ;
  BLUE_INT i ;
  BLUE_INT j ;
  BLUE_INT num_wins ;
  BLUE_IPADDR ipaddress ;
  BLUE_IPADDR bcast ;
  BLUE_IPADDR mask ;
  BLUE_CONFIG_MODE netbios_mode ;
  BLUE_CONFIG_ICONFIG_TYPE itype ;
  BLUE_TCHAR *tstr ;
  BLUE_CCHAR *cstr ;
  BLUE_IPADDR *iparray ;
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      error_state = BLUE_FALSE ;

      bpconfig_node = BlueDOMgetElement (bpconfig_dom, "of_core") ;

      if (bpconfig_node == BLUE_NULL)
	error_state = BLUE_TRUE ;

      if (!error_state)
	{
	  version = BlueDOMgetAttribute (bpconfig_node, "version") ;
	  if (version != BLUE_NULL)
	    {
	      if (BlueCstrcmp (version, "0.0") != 0)
		error_state = BLUE_TRUE ;
	    }
	  else
	    error_state = BLUE_TRUE ;
	}

      if (!error_state)
	{
	  value = BlueDOMgetElementCDATA (bpconfig_dom, "devicename") ;
	  if (value != BLUE_NULL)
	    {
	      BlueCprintf ("Device Name: %s\n", value) ;
	      tstr = BlueCcstr2tstr (value) ;
	      BlueConfig->bp_workstation_name = tstr ;
	    }

	  value = BlueDOMgetElementCDATA (bpconfig_dom, "uuid") ;
	  if (value != BLUE_NULL)
	    {
	      BlueCatouuid (value, BlueConfig->uuid) ;
	    }

	  value = BlueDOMgetElementCDATA (bpconfig_dom, "description") ;
	  if (value != BLUE_NULL)
	    {
	      tstr = BlueCcstr2tstr (value) ;
	      BlueConfig->bp_workstation_desc = tstr ;
	    }
	  ip_node = BlueDOMgetElement (bpconfig_dom, "ip") ;
	  if (ip_node != BLUE_NULL)
	    {
	      value = BlueDOMgetElementCDATA (ip_node, "autoip") ;
	      if (value != BLUE_NULL)
		{
		  if (BlueCstrcmp (value, "yes") == 0)
		    BlueConfig->enableAutoIP = BLUE_TRUE ;
		  else
		    BlueConfig->enableAutoIP = BLUE_FALSE ;
		}

	      interfaces_node = 
		BlueDOMgetElement (bpconfig_dom, "interfaces") ;
	      if (interfaces_node != BLUE_NULL)
		{
		  itype = BLUE_CONFIG_ICONFIG_MANUAL ;
		  value = BlueDOMgetElementCDATA (interfaces_node, "config") ;
		  if (value != BLUE_NULL)
		    {
		      if (BlueCstrcmp (value, "auto") == 0)
			itype = BLUE_CONFIG_ICONFIG_AUTO ;
		    }

		  BlueConfigSetInterfaceType (itype) ;

		  if (BlueConfig->bp_iconfig_type != BLUE_CONFIG_ICONFIG_AUTO)
		    {
		      interface_nodelist = 
			BlueDOMgetElementsByTagName (interfaces_node, 
						     "interface") ;
		      if (interface_nodelist != BLUE_NULL)
			{
			  for (i = 0 ; interface_nodelist[i] != BLUE_NULL ; 
			       i++) ;

			  BlueConfigSetInterfaceCount (i) ;

			  for (i = 0 ; i < BlueConfigInterfaceCount() ; i++)
			    {
			      value = 
				BlueDOMgetElementCDATA (interface_nodelist[i],
							"ipaddress") ;
			      if (value != BLUE_NULL)
				BlueNETpton (value, &ipaddress) ;
			      else
				{
				  ipaddress.ip_version = BLUE_FAMILY_IP ;
				  ipaddress.u.ipv4.addr = BLUE_INADDR_NONE ;
				}
			      value = 
				BlueDOMgetElementCDATA (interface_nodelist[i],
							"bcast") ;
			      if (value != BLUE_NULL)
				BlueNETpton (value, &bcast) ;
			      else
				{
				  bcast.ip_version = BLUE_FAMILY_IP ;
				  bcast.u.ipv4.addr = BLUE_INADDR_BROADCAST ;
				}
			      value = 
				BlueDOMgetElementCDATA (interface_nodelist[i],
							"mask") ;
			      if (value != BLUE_NULL)
				BlueNETpton (value, &mask) ;
			      else
				{
				  mask.ip_version = BLUE_FAMILY_IP ;
				  mask.u.ipv4.addr = BLUE_INADDR_ANY ;
				}

			      netbios_mode = BLUE_PARAM_DEFAULT_NETBIOS_MODE ;
			      value = 
				BlueDOMgetElementCDATA (interface_nodelist[i],
							"mode") ;
			      if (value != BLUE_NULL)
				{
				  for (j = 0 ; 
				       j < 4 && 
					 BlueCstrcmp (value, 
						      strmode[j]) != 0 ; 
				       j++) ;
				  if (j < 4)
				    netbios_mode = j ;
				}

			      iparray = BLUE_NULL ;
			      num_wins = 0 ;
			      wins_node = 
				BlueDOMgetElement (interface_nodelist[i], 
						   "winslist") ;
			      if (wins_node != BLUE_NULL)
				{
				  wins_nodelist = 
				    BlueDOMgetElementsByTagName (wins_node, 
								 "wins") ;
				  if (wins_nodelist != BLUE_NULL)
				    {
				      for (num_wins = 0 ; 
					   wins_nodelist[num_wins] != 
					     BLUE_NULL ; 
					   num_wins++) ;
				      iparray = 
					BlueHeapMalloc (sizeof (BLUE_IPADDR) *
							num_wins) ;
				      for (j = 0 ; j < num_wins ; j++)
					{
					  value = 
					    BlueDOMgetCDATA (wins_nodelist[j]) ;
					  BlueNETpton (value, &iparray[j]) ;
					}
				      BlueDOMdestroyNodelist (wins_nodelist) ;
				    }
				}

			      value = 
				BlueDOMgetElementCDATA (interface_nodelist[i],
							"master") ;

			      BlueConfigSetInterfaceConfig (i, netbios_mode, 
							    &ipaddress, 
							    &bcast, 
							    &mask,
							    value,
							    num_wins,
							    iparray) ;
			      BlueHeapFree (iparray) ;
			    }
			  BlueDOMdestroyNodelist (interface_nodelist) ;
			}
		    }
		}

	      dns_node = BlueDOMgetElement (ip_node, "dnslist") ;
	      if (dns_node != BLUE_NULL)
		{
		  dns_nodelist = 
		    BlueDOMgetElementsByTagName (dns_node, "dns") ;
		  if (dns_nodelist != BLUE_NULL)
		    {
		      for (i = 0 ; dns_nodelist[i] != BLUE_NULL ; i++) ;
		      BlueConfig->bp_netbios_dns.num_dns = i ;
		      if (BlueConfig->bp_netbios_dns.dns != 
			  BLUE_NULL)
			BlueHeapFree (BlueConfig->bp_netbios_dns.dns);
		      iparray = BlueHeapMalloc (sizeof (BLUE_IPADDR) * i) ;
		      BlueConfig->bp_netbios_dns.dns = iparray;
		      for (i = 0 ; i < BlueConfig->bp_netbios_dns.num_dns ; 
			   i++)
			{
			  value = BlueDOMgetCDATA (dns_nodelist[i]) ;
			  BlueNETpton (value, &iparray[i]) ;
			}
		      BlueDOMdestroyNodelist (dns_nodelist) ;
		    }
		}
	    }

	  drives_node = BlueDOMgetElement (bpconfig_dom, "drives") ;
	  if (drives_node != BLUE_NULL)
	    {
	      drives_nodelist = 
		BlueDOMgetElementsByTagName (drives_node, "map") ;
	      if (drives_nodelist != BLUE_NULL)
		{
		  for (i = 0 ; drives_nodelist[i] != BLUE_NULL ; i++) 
		    {
		      BLUE_LPCSTR lpBookmark ;
		      BLUE_LPCSTR lpDesc ;
		      BLUE_LPCSTR lpFile ;
		      BLUE_PATH *path ;
		      BLUE_BOOL thumbnail = BLUE_FALSE ;

		      map_node = drives_nodelist[i] ;
		      lpBookmark = 
			BlueDOMgetElementCDATA (map_node, "drive") ;
		      lpDesc = 
			BlueDOMgetElementCDATA (map_node, "description") ;

		      value = 
			BlueDOMgetElementCDATA (map_node, "thumbnail") ;
		      if (value != BLUE_NULL)
			{
			  if (BlueCstrcmp (value, "yes") == 0)
			    thumbnail = BLUE_TRUE ;
			  else
			    thumbnail = BLUE_FALSE ;
			}

		      if (lpDesc == BLUE_NULL)
			lpDesc = "Remote Share" ;
		      lpFile = BlueDOMgetElementCDATA (map_node, "path") ;
		      if (lpBookmark != BLUE_NULL && lpFile != BLUE_NULL)
			{
			  path = BluePathCreateA (lpFile) ;
			  if (BluePathRemote(path))
			    {
			      if (!BluePathAddMapA (lpBookmark, lpDesc,
						    path, BLUE_FS_CIFS,
						    thumbnail))
				BluePathDelete (path) ;
			    }
			  else
			    {
			      BluePathDelete (path) ;
			    }
			}
		    }
		  BlueDOMdestroyNodelist (drives_nodelist) ;
		}
	    }
	  BlueConfig->loaded = BLUE_TRUE ;
	}
    }
}
#endif
	      
static BLUE_VOID 
BlueConfigLock(BLUE_VOID)
{
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    BlueLock (BlueConfig->config_lock) ;
}

static BLUE_VOID 
BlueConfigUnlock(BLUE_VOID)
{
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    BlueUnlock (BlueConfig->config_lock) ;
}

#if defined(BLUE_PARAM_PERSIST)
BLUE_CORE_LIB BLUE_VOID 
BlueConfigPrint (BLUE_LPVOID *buf, BLUE_SIZET *len)
{
  BLUE_CONFIG *BlueConfig ;
  BLUE_DOMNode *bpconfig_dom ;

  *len = 0 ;
  *buf = BLUE_NULL ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      BlueConfigLock() ;

      bpconfig_dom = BlueConfigMakeDOM () ;

      if (bpconfig_dom != BLUE_NULL)
	{
	  *len = BlueDOMsprintDocument (BLUE_NULL, 0, bpconfig_dom) ;
	  *buf = (BLUE_LPVOID) BlueHeapMalloc (*len + 1) ;
	  BlueDOMsprintDocument (*buf, *len+1, bpconfig_dom) ;
	}
      BlueConfigUnlock() ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigSave (BLUE_LPCTSTR lpFileName)
{
  BLUE_LPVOID buf ;
  BLUE_HANDLE xml ;
  BLUE_DWORD dwLen ;
  BLUE_SIZET len ;

  BlueConfigPrint (&buf, &len) ;

  if (buf != BLUE_NULL)
    {
      dwLen = (BLUE_DWORD) len ;
      xml = BlueCreateFileW (lpFileName,
			     BLUE_GENERIC_WRITE,
			     BLUE_FILE_SHARE_READ,
			     BLUE_NULL,
			     BLUE_CREATE_ALWAYS,
			     BLUE_FILE_ATTRIBUTE_NORMAL,
			     BLUE_HANDLE_NULL) ;

      if (xml != BLUE_INVALID_HANDLE_VALUE)
	{
	  BlueWriteFile (xml, buf, dwLen, &dwLen, BLUE_HANDLE_NULL) ;
	  BlueCloseHandle (xml) ;
	}

      BlueHeapFree (buf) ;
    }
}

typedef struct
{
  BLUE_HANDLE handle ;
} FILE_CONTEXT ;

static BLUE_SIZET
readFile (BLUE_VOID *context, BLUE_LPVOID buf, BLUE_DWORD bufsize)
{
  FILE_CONTEXT *fileContext ;
  BLUE_DWORD bytes_read ;
  BLUE_SIZET ret ;

  fileContext = (FILE_CONTEXT *) context ;

  if (!BlueReadFile (fileContext->handle, buf, bufsize, 
		     &bytes_read, BLUE_HANDLE_NULL))
    ret = -1 ;
  else
    ret = bytes_read ;

  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigLoad (BLUE_LPCTSTR lpFileName)
{
  FILE_CONTEXT *fileContext ;
  BLUE_DOMNode *bpconfig_dom ;
  
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      BlueConfigLock() ;

      fileContext = (FILE_CONTEXT *) BlueHeapMalloc (sizeof (FILE_CONTEXT)) ;
  
      fileContext->handle = BlueCreateFileW (lpFileName,
					     BLUE_GENERIC_READ,
					     BLUE_FILE_SHARE_READ,
					     BLUE_NULL,
					     BLUE_OPEN_EXISTING,
					     BLUE_FILE_ATTRIBUTE_NORMAL,
					     BLUE_HANDLE_NULL) ;

      bpconfig_dom = BLUE_NULL ;

      if (fileContext->handle != BLUE_INVALID_HANDLE_VALUE &&
	  fileContext->handle != BLUE_HANDLE_NULL)
	{
	  bpconfig_dom = BlueDOMloadDocument (readFile, 
					      (BLUE_VOID *) fileContext) ;

	  BlueCloseHandle (fileContext->handle) ;
	}
      else
	BlueCprintf ("Could not load config file %S\n", lpFileName) ;

      BlueHeapFree (fileContext) ;
  
      if (bpconfig_dom != BLUE_NULL)
	{
	  BlueConfigParseDOM (bpconfig_dom) ;
	  BlueDOMdestroyDocument (bpconfig_dom) ;
	}

      BlueConfigUnlock() ;
    }
}
#endif

BLUE_CORE_LIB BLUE_VOID 
BlueConfigFree (BLUE_VOID)
{
  BLUE_INT i ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      if (BlueConfig->bp_workstation_name != BLUE_NULL)
	{
	  BlueHeapFree (BlueConfig->bp_workstation_name) ;
	  BlueConfig->bp_workstation_name = BLUE_NULL ;
	}
      if (BlueConfig->bp_workstation_domain != BLUE_NULL)
	{
	  BlueHeapFree (BlueConfig->bp_workstation_domain) ;
	  BlueConfig->bp_workstation_domain = BLUE_NULL ;
	}
      if (BlueConfig->bp_workstation_desc != BLUE_NULL)
	{
	  BlueHeapFree (BlueConfig->bp_workstation_desc) ;
	  BlueConfig->bp_workstation_desc = BLUE_NULL ;
	}
      if (BlueConfig->bp_interface_config != BLUE_NULL)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;

	  for (i = 0 ; i < BlueConfig->bp_interface_count ; i++)
	    {
	      if (bp_interface_config[i].winslist != BLUE_NULL)
		{
		  BlueHeapFree (bp_interface_config[i].winslist) ;
		  bp_interface_config[i].winslist = BLUE_NULL ;
		}
	      bp_interface_config[i].num_wins = 0 ;
	      BlueHeapFree (bp_interface_config[i].master) ;
	    }
	  BlueHeapFree (BlueConfig->bp_interface_config) ;
	  BlueConfig->bp_interface_config = BLUE_NULL ;
	  BlueConfig->bp_interface_count = 0 ;
	}

      if (BlueConfig->bp_netbios_dns.dns != BLUE_NULL)
	{
	  BlueHeapFree (BlueConfig->bp_netbios_dns.dns) ;
	  BlueConfig->bp_netbios_dns.num_dns = 0 ;
	  BlueConfig->bp_netbios_dns.dns = BLUE_NULL ;
	}
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigDefault (BLUE_VOID)
{
  BLUE_TCHAR *tstr ;

  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      BlueConfigLock() ;

      BlueConfigFree() ;

      tstr = BlueCcstr2tstr (BLUE_PARAM_DEFAULT_NAME) ;

      BlueConfig->bp_workstation_name = tstr;

      tstr = BlueCcstr2tstr (BLUE_PARAM_DEFAULT_DESCR) ;
      BlueConfig->bp_workstation_desc = tstr;
  
      BlueConfig->bp_interface_count = 0 ;

      BlueConfigSetInterfaceType (BLUE_CONFIG_ICONFIG_AUTO) ;

      tstr = BlueCcstr2tstr (BLUE_PARAM_DEFAULT_DOMAIN) ;
      BlueConfig->bp_workstation_domain = tstr;
    
      BlueConfigUnlock() ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigInit(BLUE_VOID)
{
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;

  if (BlueConfig == BLUE_NULL)
    {
      BlueConfig = BlueHeapMalloc (sizeof (BLUE_CONFIG)) ;
      if (BlueConfig != BLUE_NULL)
	{
	  /*
	   * Clear the configuration structure
	   */
	  BlueCmemset (BlueConfig, '\0', sizeof (BLUE_CONFIG)) ;

	  BlueConfig->update_count = 0 ;
	  BlueConfig->config_lock = BlueLockInit() ;
	  BlueConfig->loaded = BLUE_FALSE ;
	  BlueConfig->bp_workstation_name = BLUE_NULL ;
	  BlueConfig->bp_workstation_domain = BLUE_NULL ;
	  BlueConfig->bp_workstation_desc = BLUE_NULL ;
	  BlueConfig->bp_interface_config = BLUE_NULL ;
	  BlueConfig->bp_netbios_dns.dns = BLUE_NULL ;

	  BlueSetConfig (BlueConfig) ;

	  BlueConfigDefault() ;
	}
    }
  event_queue = BlueQcreate() ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigUnload (BLUE_VOID)
{
  BLUE_HANDLE hEvent ;
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      for (hEvent = (BLUE_HANDLE) BlueQdequeue (event_queue) ;
	   hEvent != BLUE_HANDLE_NULL ;
	   hEvent = (BLUE_HANDLE) BlueQdequeue (event_queue))
	{
	  BlueEventDestroy (hEvent) ;
	}
      BlueQdestroy (event_queue) ;

      BlueConfigFree() ;

      BlueLockDestroy(BlueConfig->config_lock);

      BlueHeapFree (BlueConfig) ;
      BlueSetConfig (BLUE_NULL) ;
    }
}

BLUE_CORE_LIB BLUE_BOOL 
BlueConfigLoaded (BLUE_VOID)
{
  BLUE_CONFIG *BlueConfig ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      ret = BlueConfig->loaded ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigResetInterfaceConfig (BLUE_VOID)
{
  BLUE_INT i ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      if (BlueConfig->bp_interface_config != BLUE_NULL)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;
	  for (i = 0 ; i < BlueConfig->bp_interface_count ; i++)
	    {
	      if (bp_interface_config[i].winslist != BLUE_NULL)
		{
		  BlueHeapFree (bp_interface_config[i].winslist) ;
		  bp_interface_config[i].winslist = BLUE_NULL ;
		}
	      bp_interface_config[i].num_wins = 0 ;
	      BlueHeapFree (bp_interface_config[i].master) ;
	    }
	  BlueHeapFree (bp_interface_config) ;
	}
      BlueConfig->bp_interface_count = 0 ;
      BlueConfig->bp_interface_config = BLUE_NULL ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigSetInterfaceType (BLUE_CONFIG_ICONFIG_TYPE itype)
{
  BLUE_CONFIG_MODE netbios_mode ;
  BLUE_INT i ;
  BLUE_IPADDR ipaddress ;
  BLUE_IPADDR bcast ;
  BLUE_IPADDR mask ;

  BLUE_CONFIG *BlueConfig ;
  BLUE_INT num_wins ;
  BLUE_IPADDR *winslist ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      BlueConfigResetInterfaceConfig () ;
  
      BlueConfig->bp_iconfig_type = itype ;
  
      if (BlueConfig->bp_iconfig_type == BLUE_CONFIG_ICONFIG_AUTO)
	{
	  BlueConfigSetInterfaceCount (BlueNetInterfaceCount()) ;
	  for (i = 0 ; i < BlueConfig->bp_interface_count ; i++)
	    {
	      netbios_mode = BLUE_PARAM_DEFAULT_NETBIOS_MODE ;
	      BlueNetInterfaceAddr (i, &ipaddress, &bcast, &mask) ;
	      BlueNetInterfaceWins (i, &num_wins, &winslist) ;
	      BlueConfigSetInterfaceConfig (i, netbios_mode,
					    &ipaddress, &bcast, &mask,
					    BLUE_NULL,
					    num_wins, winslist) ;
	      BlueHeapFree (winslist) ;
	    }
	}
    }
}

BLUE_CORE_LIB BLUE_CONFIG_ICONFIG_TYPE 
BlueConfigInterfaceConfig (BLUE_VOID)
{
  BLUE_CONFIG *BlueConfig ;
  BLUE_CONFIG_ICONFIG_TYPE itype ;

  itype = BLUE_CONFIG_ICONFIG_AUTO ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    itype = BlueConfig->bp_iconfig_type ;
  return (itype) ;
}

BLUE_CORE_LIB BLUE_INT 
BlueConfigWINSCount(BLUE_INT index)
{
  BLUE_INT ret ;
  BLUE_CONFIG *BlueConfig ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;

  ret = 0 ;
  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      if (index < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;
	  ret = bp_interface_config[index].num_wins ;
	}
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigWINSAddr(BLUE_INT xface, BLUE_INT index, BLUE_IPADDR *addr) 
{
  BLUE_IPADDR *iparray ;
  BLUE_CONFIG *BlueConfig ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;

  addr->ip_version = BLUE_FAMILY_IP ;
  addr->u.ipv4.addr = BLUE_INADDR_NONE ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      if (xface < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;

	  if (index < bp_interface_config[xface].num_wins)
	    {
	      iparray = bp_interface_config[xface].winslist ;
	      BlueCmemcpy (addr, &iparray[index], sizeof (BLUE_IPADDR)) ;
	    }
	}
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigSetInterfaceCount (BLUE_INT count)
{
  BLUE_INT i ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;

  BLUE_CONFIG *BlueConfig ;
  BLUE_INT old_count ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      bp_interface_config = BlueConfig->bp_interface_config ;

      old_count = BlueConfig->bp_interface_count ;
      BlueConfig->bp_interface_count = count ;

      for (i = count ; i < old_count ; i++)
	{
	  if (bp_interface_config[i].winslist != BLUE_NULL)
	    BlueHeapFree (bp_interface_config[i].winslist) ;
	  bp_interface_config[i].num_wins = 0 ;
	  bp_interface_config[i].winslist = BLUE_NULL ;
	  if (bp_interface_config[i].master != BLUE_NULL)
	    BlueHeapFree (bp_interface_config[i].master) ;
	  bp_interface_config[i].master = BLUE_NULL ;
	}

      bp_interface_config = 
	BlueHeapRealloc (BlueConfig->bp_interface_config, 
			sizeof (BLUE_CONFIG_ICONFIG) * count) ;
      BlueConfig->bp_interface_config = bp_interface_config ;

      for (i = old_count ; i < count ; i++)
	{
	  bp_interface_config[i].master = BLUE_NULL ;
	  bp_interface_config[i].winslist = BLUE_NULL ;
	  bp_interface_config[i].num_wins = 0 ;
	}
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigRemoveInterfaceConfig (BLUE_IPADDR *ip)
{
  BLUE_INT i ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;
  BLUE_INT idx ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      bp_interface_config = BlueConfig->bp_interface_config ;

      for (idx = 0 ; idx < BlueConfig->bp_interface_count &&
	     !BlueNETIsAddrEqual (&bp_interface_config[idx].ipaddress, ip) ;
	   idx++) ;
	  
      if (idx < BlueConfig->bp_interface_count)
	{
	  if (bp_interface_config[idx].winslist != BLUE_NULL)
	    BlueHeapFree (bp_interface_config[idx].winslist) ;
	  bp_interface_config[idx].num_wins = 0 ;
	  bp_interface_config[idx].winslist = BLUE_NULL ;
	  if (bp_interface_config[idx].master != BLUE_NULL)
	    BlueHeapFree (bp_interface_config[idx].master) ;
	  bp_interface_config[idx].master = BLUE_NULL ;

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
	    BlueHeapRealloc (BlueConfig->bp_interface_config, 
			    sizeof (BLUE_CONFIG_ICONFIG) * 
			    BlueConfig->bp_interface_count) ;
	  BlueConfig->bp_interface_config = bp_interface_config;
	}
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigSetInterfaceConfig (BLUE_INT i,
			      BLUE_CONFIG_MODE netbios_mode,
			      BLUE_IPADDR *ipaddress,
			      BLUE_IPADDR *bcast,
			      BLUE_IPADDR *mask,
			      BLUE_CHAR *master,
			      BLUE_INT num_wins,
			      BLUE_IPADDR *winslist)
{
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CHAR *cstr ;
  BLUE_CONFIG *BlueConfig ;
  BLUE_INT j ;
  BLUE_IPADDR *iwinslist ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      if (i < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;

	  bp_interface_config[i].bp_netbios_mode = netbios_mode ;
	  if (ipaddress != BLUE_NULL)
	    {
	      bp_interface_config[i].ipaddress = *ipaddress ;
	    }
	  if (bcast != BLUE_NULL)
	    bp_interface_config[i].bcast = *bcast ;
	  if (mask != BLUE_NULL)
	    bp_interface_config[i].mask = *mask ;

	  if (bp_interface_config[i].winslist != BLUE_NULL)
	    BlueHeapFree (bp_interface_config[i].winslist) ;
	  bp_interface_config[i].num_wins = 0 ;
	  bp_interface_config[i].winslist = BLUE_NULL ;
	  if (num_wins != 0 && winslist != BLUE_NULL)
	    {
	      bp_interface_config[i].num_wins = num_wins ;
	      iwinslist = BlueHeapMalloc (sizeof (BLUE_IPADDR) * num_wins) ;
	      bp_interface_config[i].winslist = iwinslist ;
	      for (j = 0 ; j < num_wins ; j++)
		iwinslist[j] = winslist[j] ;
	    }

	  if (master != BLUE_NULL)
	    {
	      if (bp_interface_config[i].master != BLUE_NULL)
		BlueHeapFree (bp_interface_config[i].master) ;

	      cstr = BlueCstrdup (master) ;
	      bp_interface_config[i].master = cstr ;
	    }
	}
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigSetPrivate (BLUE_INT index, BLUE_VOID *private)
{
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      if (index < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;
	  bp_interface_config[index].private = private ;
	}
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigPrivate (BLUE_INT index, BLUE_VOID **private)
{
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;

  *private = BLUE_NULL ;
  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      if (index < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;
	  *private = bp_interface_config[index].private ;
	}
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigSetLocalMaster (BLUE_INT index, BLUE_LPCSTR local_master)
{
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CHAR *cstr ;
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      if (index < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;

	  if (bp_interface_config[index].master != BLUE_NULL)
	    BlueHeapFree (bp_interface_config[index].master) ;
	  bp_interface_config[index].master = BLUE_NULL ;
	  if (local_master != BLUE_NULL)
	    {
	      cstr = BlueCstrdup (local_master) ;
	      bp_interface_config[index].master = cstr ;
	    }
	}
    }
}

BLUE_CORE_LIB BLUE_INT 
BlueConfigInterfaceCount (BLUE_VOID) 
{
  BLUE_INT ret ;
  BLUE_CONFIG *BlueConfig ;

  ret = 0 ;
  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      ret = BlueConfig->bp_interface_count ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigLocalMaster (BLUE_INT index, BLUE_LPCSTR *local_master)
{
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;

  *local_master = BLUE_NULL ;
  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      if (index < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;
	  *local_master = bp_interface_config[index].master ;
	}
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigInterfaceAddr (BLUE_INT index, BLUE_IPADDR *addr,
			 BLUE_IPADDR *pbcast, BLUE_IPADDR *pmask)
{
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;

  addr->ip_version = BLUE_FAMILY_IP ;
  addr->u.ipv4.addr = BLUE_INADDR_NONE ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      if (index < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;

	  if (addr != BLUE_NULL)
	    BlueCmemcpy (addr, &bp_interface_config[index].ipaddress,
			 sizeof (BLUE_IPADDR)) ;
	  if (pbcast != BLUE_NULL)
	    BlueCmemcpy (pbcast, &bp_interface_config[index].bcast,
			 sizeof (BLUE_IPADDR)) ;
	  if (pmask != BLUE_NULL)
	    BlueCmemcpy (pmask, &bp_interface_config[index].mask,
			 sizeof (BLUE_IPADDR)) ;
	}
    }
}

BLUE_CORE_LIB BLUE_CONFIG_MODE 
BlueConfigInterfaceMode (BLUE_INT index, BLUE_INT *num_wins,
			 BLUE_IPADDR **winslist)
{
  BLUE_CONFIG_MODE mode ;
  BLUE_CONFIG_ICONFIG *bp_interface_config ;
  BLUE_CONFIG *BlueConfig ;
  BLUE_IPADDR *iwinslist ;
  BLUE_INT i ;

  mode = BLUE_PARAM_DEFAULT_NETBIOS_MODE ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      if (index < BlueConfig->bp_interface_count)
	{
	  bp_interface_config = BlueConfig->bp_interface_config ;
	  mode = bp_interface_config[index].bp_netbios_mode ;
	  if (num_wins != BLUE_NULL)
	    {
	      *num_wins = bp_interface_config[index].num_wins ;
	      if (winslist != BLUE_NULL)
		{
		  *winslist = BLUE_NULL ;
		  if (*num_wins > 0)
		    {
		      *winslist = 
			BlueHeapMalloc (sizeof (BLUE_IPADDR) * *num_wins) ;
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

BLUE_CORE_LIB BLUE_VOID 
BlueConfigNodeName (BLUE_LPCTSTR *name, 
		    BLUE_LPCTSTR *workgroup,
		    BLUE_LPCTSTR *desc) 
{
  BLUE_CONFIG *BlueConfig ;

  *name = BLUE_NULL ;
  *workgroup = BLUE_NULL ;
  *desc = BLUE_NULL ;
  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      *name = BlueConfig->bp_workstation_name ;
      *workgroup = BlueConfig->bp_workstation_domain ;
      *desc = BlueConfig->bp_workstation_desc ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigSetNodeName (BLUE_LPCTSTR name,
		       BLUE_LPCTSTR workgroup,
		       BLUE_LPCTSTR desc)
{
  BLUE_TCHAR *tstr ;
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      if (BlueConfig->bp_workstation_name != BLUE_NULL)
	BlueHeapFree (BlueConfig->bp_workstation_name) ;
      tstr = BlueCtstrdup(name) ;
      BlueConfig->bp_workstation_name = tstr ;
      if (BlueConfig->bp_workstation_domain != BLUE_NULL)
	BlueHeapFree (BlueConfig->bp_workstation_domain) ;
      tstr = BlueCtstrdup (workgroup) ;
      BlueConfig->bp_workstation_domain = tstr ;
      if (BlueConfig->bp_workstation_desc != BLUE_NULL)
	BlueHeapFree (BlueConfig->bp_workstation_desc) ;
      tstr = BlueCtstrdup (desc) ;
      BlueConfig->bp_workstation_desc = tstr ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigUUID (BLUE_UUID *uuid) 
{
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      BlueCmemcpy (uuid, BlueConfig->uuid, BLUE_UUID_LEN) ;
    }
  else
    BlueCmemset (uuid, '\0', BLUE_UUID_LEN) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigSetUUID (BLUE_UUID *uuid) 
{
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      BlueCmemcpy (BlueConfig->uuid, uuid, BLUE_UUID_LEN) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigRegisterUpdate (BLUE_HANDLE hEvent) 
{
  BlueQenqueue (event_queue, (BLUE_VOID *) hEvent) ;
  /*
   * Everyone who registers get's an initial event
   */
  BlueEventSet (hEvent) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigUnregisterUpdate (BLUE_HANDLE hEvent) 
{
  BlueQunlink (event_queue, (BLUE_VOID *) hEvent) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigNotify (BLUE_VOID)
{
  BLUE_HANDLE hEvent ;

  for (hEvent = (BLUE_HANDLE) BlueQfirst (event_queue) ;
       hEvent != BLUE_HANDLE_NULL ;
       hEvent = (BLUE_HANDLE) BlueQnext (event_queue, (BLUE_VOID *) hEvent))
    {
      BlueEventSet (hEvent) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueConfigUpdate (BLUE_VOID)
{
  BLUE_CONFIG *BlueConfig ;

  BlueConfig = BlueGetConfig() ;
  if (BlueConfig != BLUE_NULL)
    {
      BlueConfigLock() ;
      BlueConfig->update_count++ ;
      BlueConfigNotify() ;
      BlueConfigUnlock() ;
    }
}

