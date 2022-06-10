/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/types.h"
#include "ofc/config.h"
#include "ofc/app.h"
#include "ofc/sched.h"
#include "ofc/net.h"
#include "ofc/timer.h"
#include "ofc/event.h"
#include "ofc/heap.h"
#include "ofc/persist.h"
#include "ofc/netmon.h"
#include "ofc/handle.h"
#include "ofc/sched.h"

#define NETMON_INTERVAL 10000

typedef enum 
  {
    NETMON_STATE_IDLE,
    NETMON_STATE_RUNNING
  } NETMON_STATE ;

typedef struct
{
  NETMON_STATE state ;
  OFC_HANDLE scheduler ;
  OFC_HANDLE hTimer ;
  OFC_HANDLE hEvent ;
} NETMON_CONTEXT;

static OFC_VOID NetMonPreSelect (OFC_HANDLE app) ;
static OFC_HANDLE NetMonPostSelect (OFC_HANDLE app, OFC_HANDLE hEvent) ;
static OFC_VOID NetMonDestroy (OFC_HANDLE app) ;

static OFC_APP_TEMPLATE NetMonAppDef =
  {
    "Network Monitor",
    &NetMonPreSelect,
    &NetMonPostSelect,
    &NetMonDestroy,
#if defined(OFC_APP_DEBUG)
    OFC_NULL
#endif
  } ;

OFC_VOID ofc_netmon_startup (OFC_HANDLE hScheduler, OFC_HANDLE hNotify)
{
  NETMON_CONTEXT *NetMon ;
  OFC_HANDLE hApp ;
  /*
   * Let's create an app
   */
  NetMon = ofc_malloc (sizeof (NETMON_CONTEXT)) ;
  if (NetMon != OFC_NULL)
    {
      NetMon->state = NETMON_STATE_IDLE ;
      NetMon->scheduler = hScheduler ;
      hApp = ofc_app_create (hScheduler, &NetMonAppDef, NetMon) ;
      if (hNotify != OFC_HANDLE_NULL)
	ofc_app_set_wait (hApp, hNotify) ;
    }
}

static OFC_VOID NetMonPreSelect (OFC_HANDLE app) 
{
  NETMON_CONTEXT *NetMon ;
  NETMON_STATE entry_state ;

  NetMon = ofc_app_get_data (app) ;
  if (NetMon != OFC_NULL)
    {
      do /* while NetMon->state != entry_state */
	{
	  entry_state = NetMon->state ;
	  ofc_sched_clear_wait (NetMon->scheduler, app) ;
	  
	  switch (NetMon->state)
	    {
	    default:
	    case NETMON_STATE_IDLE:
	      NetMon->hEvent = ofc_event_create(OFC_EVENT_AUTO) ;
	      if (NetMon->hEvent != OFC_HANDLE_NULL)
		{
		  ofc_net_register_config (NetMon->hEvent) ;
		}

	      NetMon->hTimer = ofc_timer_create("NETMON") ;
	      if (NetMon->hTimer != OFC_HANDLE_NULL)
		{
		  ofc_timer_set (NetMon->hTimer, NETMON_INTERVAL) ;
		  NetMon->state = NETMON_STATE_RUNNING ;
		}
	      ofc_sched_add_wait (NetMon->scheduler, app, NetMon->hTimer) ;
	      ofc_sched_add_wait (NetMon->scheduler, app, NetMon->hEvent) ;
	      break ;

	    case NETMON_STATE_RUNNING:
	      ofc_sched_add_wait (NetMon->scheduler, app, NetMon->hTimer) ;
	      ofc_sched_add_wait (NetMon->scheduler, app, NetMon->hEvent) ;
	      break ;
	    }
	}
      while (NetMon->state != entry_state) ;
    }
}

static OFC_HANDLE NetMonPostSelect (OFC_HANDLE app, OFC_HANDLE hEvent) 
{
  NETMON_CONTEXT *NetMon ;
  OFC_INT i ;
  OFC_INT j ;
  OFC_BOOL update ;
  OFC_IPADDR ip1 ;
  OFC_IPADDR bcast1 ;
  OFC_IPADDR mask1 ;
  OFC_IPADDR ip2 ;
  OFC_IPADDR bcast2 ;
  OFC_IPADDR mask2 ;
  OFC_BOOL found ;

  NetMon = ofc_app_get_data (app) ;
  if (NetMon != OFC_NULL)
    {
      switch (NetMon->state)
	{
	default:
	case NETMON_STATE_IDLE:
	  break ;

	case NETMON_STATE_RUNNING:
	  if (hEvent == NetMon->hTimer)
	    {
 	      ofc_heap_dump_stats() ;
#if defined(OFC_HANDLE_PERF)
	      ofc_sched_log_measure (NetMon->scheduler) ;
#endif
	      /*
	       * Timer Fired.  Manually Poll for change
	       *
	       * First check for deletes
	       */
	      update = OFC_FALSE ;
	      for (i = 0 ; i < ofc_persist_interface_count() && !update; i++) 
		{
		  ofc_persist_interface_addr (i, &ip1, &bcast1, &mask1) ;

		  found = OFC_FALSE ;
		  for (j = 0 ; j < ofc_net_interface_count() && !found;)
		    {
		      ofc_net_interface_addr (j, &ip2, &bcast2, &mask2) ;
		      if (ofc_net_is_addr_equal (&ip1, &ip2) &&
			  ofc_net_is_addr_equal (&bcast1, &bcast2) &&
			  ofc_net_is_addr_equal (&mask1, &mask2))
			found = OFC_TRUE ;
		      else
			j++ ;
		    }
		  if (!found)
		    /*
		     * An interface has been deleted
		     */
		    update = OFC_TRUE ;
		}
	      /*
	       * Now check if an interface has been added
	       */
	      for (i = 0 ; i < ofc_net_interface_count() && !update; i++) 
		{
		  ofc_net_interface_addr (i, &ip1, &bcast1, &mask1) ;

		  found = OFC_FALSE ;
		  for (j = 0 ; j < ofc_persist_interface_count() && !found;)
		    {
		      ofc_persist_interface_addr (j, &ip2, &bcast2, &mask2) ;
		      if (ofc_net_is_addr_equal (&ip1, &ip2) &&
			  ofc_net_is_addr_equal (&bcast1, &bcast2) &&
			  ofc_net_is_addr_equal (&mask1, &mask2))
			found = OFC_TRUE ;
		      else
			j++ ;
		    }

		  if (!found)
		    /*
		     * An interface has been added
		     */
		    update = OFC_TRUE ;
		}

	      if (update && 
		  ofc_persist_interface_config() == OFC_CONFIG_ICONFIG_AUTO)
		{
		  /*
		   * This routine reinitializes blue config interfaces
		   */
		  ofc_persist_set_interface_type (OFC_CONFIG_ICONFIG_AUTO) ;
		  ofc_persist_update() ;
		}

	      ofc_timer_set (NetMon->hTimer, NETMON_INTERVAL) ;
	    }
	  else if (hEvent == NetMon->hEvent &&
		   ofc_persist_interface_config() == OFC_CONFIG_ICONFIG_AUTO)
	    {
	      /*
	       * With the event, we don't have to poll
	       *
	       * This routine reinitializes blue config interfaces
	       */
	      ofc_persist_set_interface_type(OFC_CONFIG_ICONFIG_AUTO) ;
	      ofc_persist_update() ;
	    }
	  break ;
	}
    }
  return (OFC_HANDLE_NULL) ;
}

static OFC_VOID NetMonDestroy (OFC_HANDLE app)
{
  NETMON_CONTEXT *NetMon ;

  NetMon = ofc_app_get_data (app) ;
  if (NetMon != OFC_NULL)
    {
      switch (NetMon->state)
	{
	default:
	case NETMON_STATE_IDLE:
	  break ;

	case NETMON_STATE_RUNNING:
	  ofc_timer_destroy (NetMon->hTimer) ;
	  ofc_net_unregister_config (NetMon->hEvent) ;
	  ofc_event_destroy (NetMon->hEvent) ;
	  break ;
	}
      ofc_free (NetMon) ;
    }
}
