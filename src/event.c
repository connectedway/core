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

OFC_CORE_LIB OFC_HANDLE
ofc_event_create(OFC_EVENT_TYPE eventType) {
    return (ofc_event_create_impl(eventType));
}

OFC_CORE_LIB OFC_EVENT_TYPE
ofc_event_get_type(OFC_HANDLE hEvent) {
    return (ofc_event_get_type_impl(hEvent));
}

OFC_CORE_LIB OFC_VOID
ofc_event_set(OFC_HANDLE hEvent) {
    ofc_event_set_impl(hEvent);
}

OFC_CORE_LIB OFC_VOID
ofc_event_reset(OFC_HANDLE hEvent) {
    ofc_event_reset_impl(hEvent);
}

OFC_CORE_LIB OFC_VOID
ofc_event_destroy(OFC_HANDLE hEvent) {
    ofc_event_destroy_impl(hEvent);
}

OFC_CORE_LIB OFC_VOID
ofc_event_wait(OFC_HANDLE hEvent) {
    ofc_event_wait_impl(hEvent);
}

OFC_CORE_LIB OFC_BOOL
ofc_event_test(OFC_HANDLE hEvent) {
    return (ofc_event_test_impl(hEvent));
}
