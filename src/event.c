/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/event.h"
#include "ofc/impl/eventimpl.h"

OFC_CORE_LIB BLUE_HANDLE
ofc_event_create (OFC_EVENT_TYPE eventType)
{
  return (BlueEventCreateImpl (eventType)) ;
}
  
OFC_CORE_LIB OFC_EVENT_TYPE
ofc_event_get_type (BLUE_HANDLE hEvent)
{
  return (BlueEventGetTypeImpl (hEvent)) ;
}

OFC_CORE_LIB OFC_VOID
ofc_event_set (BLUE_HANDLE hEvent)
{
  BlueEventSetImpl (hEvent) ;
}

OFC_CORE_LIB OFC_VOID
ofc_event_reset (BLUE_HANDLE hEvent)
{
  BlueEventResetImpl (hEvent) ;
}

OFC_CORE_LIB OFC_VOID
ofc_event_destroy (BLUE_HANDLE hEvent)
{
  BlueEventDestroyImpl (hEvent) ;
}

OFC_CORE_LIB OFC_VOID
ofc_event_wait (BLUE_HANDLE hEvent)
{
  BlueEventWaitImpl (hEvent) ;
}

OFC_CORE_LIB OFC_BOOL
ofc_event_test (BLUE_HANDLE hEvent)
{
  return (BlueEventTestImpl (hEvent)) ;
}
