/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OF_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/message.h"
#include "ofc/file.h"
#include "ofc/dce.h"

OFC_CORE_LIB OFC_VOID 
of_dce_push_result(OFC_MESSAGE *dceMessage, OFC_UINT16 result,
		   OFC_UINT16 reason, OFC_UUID *uuid,
		   OFC_UINT16 version) 
{
  ofc_message_fifo_push_u16 (dceMessage, result) ;
  ofc_message_fifo_push_u16 (dceMessage, reason) ;
  ofc_memcpy (ofc_message_fifo_push (dceMessage, OFC_UUID_LEN), 
	       uuid, OFC_UUID_LEN) ;
  ofc_message_fifo_push_u16 (dceMessage, version) ;
  ofc_message_fifo_push_u16 (dceMessage, DCE_INTERFACE_VERSION_MINOR) ;
}

OFC_CORE_LIB OFC_VOID 
of_dce_push_bind_header(OFC_MESSAGE *dceMessage, OFC_UINT16 xmit_frag,
		       OFC_UINT16 recv_frag, OFC_UINT32 group_id)
{
  ofc_message_fifo_push_u16 (dceMessage, xmit_frag) ;
  ofc_message_fifo_push_u16 (dceMessage, recv_frag) ;
  ofc_message_fifo_push_u32 (dceMessage, group_id) ;
}

OFC_CORE_LIB OFC_VOID 
of_dce_pop_request_header(OFC_MESSAGE *dceMessage, OFC_UINT32 *alloc_hint,
			 OFC_UINT16 *context_id, OFC_UINT16 *opnum)
{
  *alloc_hint = ofc_message_fifo_pop_u32 (dceMessage) ;
  *context_id = ofc_message_fifo_pop_u16 (dceMessage) ;
  *opnum = ofc_message_fifo_pop_u16 (dceMessage) ;
}

OFC_CORE_LIB OFC_VOID 
of_dce_push_request_header(OFC_MESSAGE *dceMessage, OFC_UINT32 alloc_hint,
			  OFC_UINT16 context_id, OFC_UINT16 opnum)
{
  ofc_message_fifo_push_u32 (dceMessage, alloc_hint) ;
  ofc_message_fifo_push_u16 (dceMessage, context_id) ;
  ofc_message_fifo_push_u16 (dceMessage, opnum) ;
}

OFC_CORE_LIB OFC_LPTSTR 
of_dce_pop_tstr(OFC_MESSAGE *dceMessage)
{
  OFC_LPTSTR ret ;
  OFC_UINT32 count ;
  /*
   * Pop Max Count
   */
  ofc_message_fifo_pop_u32 (dceMessage) ;
  /*
   * Pop Offset
   */
  ofc_message_fifo_pop_u32 (dceMessage) ;
  /*
   * Pop Actual Count
   */
  count = ofc_message_fifo_pop_u32 (dceMessage) ;
  /*
   * Round up
   */
  ofc_message_fifo_align(dceMessage, 2) ;
  ret = ofc_message_fifo_pop_tstrn (dceMessage, count) ;
  if (count & 0x01)
    ofc_message_fifo_pop (dceMessage, sizeof (OFC_WORD)) ;

  return (ret) ;
 }

OFC_CORE_LIB OFC_VOID 
of_dce_push_tstr(OFC_MESSAGE *dceMessage, OFC_LPCTSTR str)
{
  OFC_UINT32 count ;
  /*
   * Push Count
   */
  count = (OFC_UINT32) ofc_tstrlen (str) ;
  ofc_message_fifo_push_u32 (dceMessage, count+1) ;
  /*
   * Push offset
   */
  ofc_message_fifo_push_u32 (dceMessage, 0) ;
  /*
   * Push Actual Count
   */
  ofc_message_fifo_push_u32 (dceMessage, count+1) ;
  /*
   * Round String
   */
  ofc_message_fifo_align (dceMessage, 2) ;
  ofc_message_fifo_push_tstrn (dceMessage, str, count) ;
  /*
   * Now push the NULL (Samba needs this, Windows deals with it)
   */
  ofc_message_fifo_push_u16 (dceMessage, TCHAR_EOS) ;
  count++ ;

  if (count & 0x01)
    ofc_message_fifo_push (dceMessage, sizeof (OFC_WORD)) ;
}

OFC_CORE_LIB OFC_VOID 
of_dce_push_share_name_comment(OFC_MESSAGE * dceMessage, OFC_LPCTSTR name,
			       OFC_LPCTSTR comment)
{
  ofc_message_fifo_push_u32 (dceMessage, REF_ID_SHARE_COMMENT) ;
  of_dce_push_tstr (dceMessage, name) ;
  of_dce_push_tstr (dceMessage, comment) ;
}

OFC_CORE_LIB OFC_VOID 
of_dce_push_share(OFC_MESSAGE *dceMessage, OFC_UINT32 type,
		  OFC_LPCTSTR name, OFC_LPCTSTR comment)
{
  ofc_message_fifo_push_u32 (dceMessage, REF_ID_SHARE_TYPE) ;
  ofc_message_fifo_push_u32 (dceMessage, type) ;
  of_dce_push_share_name_comment (dceMessage, name, comment) ;
}

OFC_CORE_LIB OFC_MESSAGE *
of_dce_bind_ack(OFC_UINT32 dceCall)
{
  OFC_MESSAGE *dceAck ;
  static OFC_UUID bit32ndr = { 0x04, 0x5d, 0x88, 0x8a, 0xeb, 0x1c, 0xc9, 0x11, 
				0x9f, 0xe8, 0x08, 0x00, 0x2b, 0x10, 0x48, 0x60 } ;
  static OFC_UUID pnio = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } ;
  /*
   * Let's send a bind ack
   */
  dceAck = ofc_message_create (MSG_ALLOC_HEAP, OFC_CALL_STACK_SIZE,
			      OFC_NULL) ;
  ofc_message_set_endian (dceAck, MSG_ENDIAN_LITTLE) ;
  ofc_message_fifo_set (dceAck, DCE_HDR_SIZE, 
		       OFC_CALL_STACK_SIZE - DCE_HDR_SIZE) ;

  /* In smbv2, dce_assoc_group is 0x000ae344 rather then 0x000053f0 */
  of_dce_push_bind_header (dceAck, 
			 OFC_MIN(OFC_CALL_STACK_SIZE,
				    32768),
			 OFC_MIN(OFC_CALL_STACK_SIZE,
				    32768),
			 DCE_ASSOC_GROUP) ;

  ofc_message_fifo_push_u16 (dceAck, 
			   (OFC_UINT16) ofc_strlen (DCE_SEC_ADDR) + 1) ;
  /* In smbv2, the dce_sec_addr is \PIPE\srvsvc rather then \PIPE\ntsvcs */
  ofc_message_fifo_push_cstr (dceAck, DCE_SEC_ADDR) ;
  /*
   * Result Count
   */
  ofc_message_fifo_align (dceAck, 2) ;
  /* In smbv2, we return 3 results */
  ofc_message_fifo_push_u32 (dceAck, 3) ;
  /*
   * Result 0
   */
  /* in smbv2, the requests are for 32 bit NDR, 64 bit NDR and bind time feature negotiation */
  /* an smbv2 server responds with 32 bit ndr, and rejects the other two */
  of_dce_push_result (dceAck, DCE_ACK_RESULT_ACCEPTANCE, 0, &bit32ndr, 2) ;
  of_dce_push_result (dceAck, DCE_ACK_RESULT_REJECTION, DCE_ACK_REASON_UNSUPPORTED, &pnio, 0) ;
  of_dce_push_result (dceAck, DCE_ACK_RESULT_REJECTION, DCE_ACK_REASON_UNSUPPORTED, &pnio, 0) ;
  return (dceAck) ;
}

static OFC_UUID transfer_syntax =
  {
    0x04, 0x5d, 0x88, 0x8a, 0xeb, 0x1c, 0xc9, 0x11,
    0x9f, 0xe8, 0x08, 0x00, 0x2b, 0x10, 0x48, 0x60
  } ;

OFC_CORE_LIB OFC_MESSAGE *
of_dce_bind_request(OFC_UUID service, OFC_UINT16 major, OFC_UINT16 minor)
{
  OFC_MESSAGE *dceBind ;
  /*
   * Let's send a bind 
   */
  dceBind = ofc_message_create (MSG_ALLOC_HEAP, 
			       OFC_CALL_STACK_SIZE, OFC_NULL) ;
  ofc_message_set_endian (dceBind, MSG_ENDIAN_LITTLE) ;
  ofc_message_fifo_set (dceBind, DCE_HDR_SIZE, 
		       OFC_CALL_STACK_SIZE - DCE_HDR_SIZE) ;

  of_dce_push_bind_header (dceBind, 
			 OFC_MIN(OFC_CALL_STACK_SIZE,
				    32768),
			 OFC_MIN(OFC_CALL_STACK_SIZE,
				    32768),
			 0) ;
  /*
   * Context Count
   */
  ofc_message_fifo_align (dceBind, 2) ;
  ofc_message_fifo_push_u32 (dceBind, 1) ;
  /*
   * Context 0
   */
  ofc_message_fifo_push_u16 (dceBind, 0) ;
  /*
   * One Item
   */
  ofc_message_fifo_push_u8 (dceBind, 1) ;
  /*
   * Align
   */
  ofc_message_fifo_align (dceBind, 2) ;
  /*
   * Store Abstract Syntax
   */
  ofc_memcpy (ofc_message_fifo_push (dceBind, OFC_UUID_LEN), 
	       service, OFC_UUID_LEN) ;
  ofc_message_fifo_push_u16 (dceBind, major) ;
  ofc_message_fifo_push_u16 (dceBind, minor) ;
  /*
   * Now we also put the syntax
   */
  ofc_memcpy (ofc_message_fifo_push (dceBind, OFC_UUID_LEN), 
	       transfer_syntax, OFC_UUID_LEN) ;
  ofc_message_fifo_push_u16 (dceBind, DCE_INTERFACE_VERSION) ;
  ofc_message_fifo_push_u16 (dceBind, DCE_INTERFACE_VERSION_MINOR) ;

  return (dceBind) ;
}

#if 0
OFC_CORE_LIB OFC_MESSAGE *
of_dce_read_message(OFC_HANDLE hFile)
{
  OFC_MESSAGE *dceMessage ;
  OFC_DWORD bytesRead ;
  OFC_UINT16 len ;
  OFC_BOOL status ;

  status = OFC_FALSE ;
  dceMessage = ofc_message_create (MSG_ALLOC_HEAP, DCE_HDR_SIZE, OFC_NULL) ;
  if (dceMessage != OFC_NULL)
    {
      ofc_message_set_endian (dceMessage, MSG_ENDIAN_LITTLE) ;
      /*
       * First read in the header
       */
      status = OfcReadFile (hFile, ofc_message_data(dceMessage), DCE_HDR_SIZE,
			    &bytesRead, OFC_HANDLE_NULL)  ;

      if (status == OFC_TRUE && bytesRead == DCE_HDR_SIZE)
	{
	  /*
	   * Let's read in the rest
	   */
	  len = ofc_message_get_u16 (dceMessage, DCE_HDR_FRAG_LENGTH) ;
	  if (ofc_message_realloc (dceMessage, len) == OFC_TRUE)
	    {
	      ofc_message_fifo_set (dceMessage, DCE_HDR_SIZE, len) ;
	      if (OfcReadFile (hFile, 
				ofc_message_data(dceMessage) + DCE_HDR_SIZE,
				len - DCE_HDR_SIZE, &bytesRead, 
				OFC_HANDLE_NULL) == OFC_TRUE &&
		  bytesRead == len - DCE_HDR_SIZE)
		status = OFC_TRUE ;
	    }
	}

      if (status == OFC_FALSE)
	{
	  ofc_message_destroy (dceMessage) ;
	  dceMessage = OFC_NULL ;
	}
    }
  return (dceMessage) ;
}
#else
OFC_CORE_LIB OFC_MESSAGE *
of_dce_read_message(OFC_HANDLE hFile)
{
  OFC_MESSAGE *dceMessage ;
  OFC_DWORD bytesRead ;
  OFC_BOOL status ;

  status = OFC_FALSE ;
  dceMessage = ofc_message_create (MSG_ALLOC_HEAP, 
				  OFC_CALL_STACK_SIZE, OFC_NULL) ;
  if (dceMessage != OFC_NULL)
    {
      ofc_message_set_endian (dceMessage, MSG_ENDIAN_LITTLE) ;
      /*
       * First read in the header
       */
      status = OfcReadFile (hFile, ofc_message_data(dceMessage), 
			     OFC_CALL_STACK_SIZE,
			     &bytesRead, OFC_HANDLE_NULL)  ;

      if (status == OFC_TRUE)
	{
	  ofc_message_fifo_set (dceMessage, DCE_HDR_SIZE, 
			       bytesRead - DCE_HDR_SIZE) ;
	}
      else
	{
	  ofc_message_destroy (dceMessage) ;
	  dceMessage = OFC_NULL ;
	}
    }
  return (dceMessage) ;
}
#endif

OFC_CORE_LIB OFC_BOOL 
of_dce_write_message(OFC_HANDLE hFile, OFC_MESSAGE *dceMessage)
{
  OFC_BOOL ret ;
  OFC_DWORD bytesWritten ;

  ret = OfcWriteFile (hFile, 
		       ofc_message_data (dceMessage),
		       ofc_message_fifo_get (dceMessage),
		       &bytesWritten, OFC_HANDLE_NULL) ;

  ofc_message_destroy (dceMessage) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_MESSAGE *
of_dce_transact(OFC_HANDLE hFile, OFC_MESSAGE *dceMessage)
{
  OFC_BOOL ret ;
  OFC_DWORD bytesRead ;
  OFC_MESSAGE *response ;
  
  response = ofc_message_create (MSG_ALLOC_HEAP, 
				OFC_CALL_STACK_SIZE, OFC_NULL) ;
  if (response != OFC_NULL)
    {
      ofc_message_set_endian (response, MSG_ENDIAN_LITTLE) ;

      ret = OfcTransactNamedPipe (hFile,
				  ofc_message_data (dceMessage),
				   ofc_message_fifo_get (dceMessage),
				   ofc_message_data (response),
				   OFC_CALL_STACK_SIZE,
				   &bytesRead,
				   OFC_HANDLE_NULL) ;

      if (ret == OFC_TRUE)
	{
	  ofc_message_fifo_set (response, DCE_HDR_SIZE, 
			       bytesRead - DCE_HDR_SIZE) ;
	}
      else
	{
	  ofc_message_destroy (response) ;
	  response = OFC_NULL ;
	}
    }
  ofc_message_destroy (dceMessage) ;
  return (response) ;
}
