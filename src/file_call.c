/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/config.h"
#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/message.h"
#include "ofc/file_call.h"
#include "ofc/process.h"

OFC_CORE_LIB OFC_MESSAGE *of_file_call_create(OFC_VOID)
{
  OFC_MESSAGE *msg;

  msg = ofc_message_create(MSG_ALLOC_HEAP, sizeof(FILE_CALL) + OFC_CALL_STACK_SIZE, OFC_NULL);
  ofc_assert(msg != OFC_NULL, "Could not allocate msg for file_call\n");

  ofc_message_param_set (msg, 0, sizeof(FILE_CALL));
  ofc_message_fifo_set (msg, (OFC_INT) sizeof(FILE_CALL), OFC_CALL_STACK_SIZE);

  return (msg);
}

