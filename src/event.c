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
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/event.h"
#include "ofc/impl/eventimpl.h"

BLUE_CORE_LIB BLUE_HANDLE 
BlueEventCreate (BLUE_EVENT_TYPE eventType)
{
  return (BlueEventCreateImpl (eventType)) ;
}
  
BLUE_CORE_LIB BLUE_EVENT_TYPE 
BlueEventGetType (BLUE_HANDLE hEvent)
{
  return (BlueEventGetTypeImpl (hEvent)) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueEventSet (BLUE_HANDLE hEvent) 
{
  BlueEventSetImpl (hEvent) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueEventReset (BLUE_HANDLE hEvent) 
{
  BlueEventResetImpl (hEvent) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueEventDestroy (BLUE_HANDLE hEvent) 
{
  BlueEventDestroyImpl (hEvent) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueEventWait (BLUE_HANDLE hEvent) 
{
  BlueEventWaitImpl (hEvent) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueEventTest (BLUE_HANDLE hEvent) 
{
  return (BlueEventTestImpl (hEvent)) ;
}
