/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OF_DCE_H__)
#define __OF_DCE_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/message.h"

/**
 * \defgroup DCE DCE handling for openfiles
 *
 * The DCE Module contains common routines for all DCE services.
 * \{ 
 */

/*
 * DCE Packet Types
 */
#define DCE_TYPE_REQUEST 0
#define DCE_TYPE_PING 1
#define DCE_TYPE_RESPONSE 2
#define DCE_TYPE_FAULT 3
#define DCE_TYPE_WORKING 4
#define DCE_TYPE_NOCALL 5
#define DCE_TYPE_REJECT 6
#define DCE_TYPE_ACK 7
#define DCE_TYPE_CLCANCEL 8
#define DCE_TYPE_FACK 9
#define DCE_TYPE_CANCEL_ACK 10
#define DCE_TYPE_BIND 11
#define DCE_TYPE_BIND_ACK 12
#define DCE_TYPE_BIND_NAK 13
#define DCE_TYPE_ALTER_CONTEXT 14
#define DCE_TYPE_ALTER_CONTEXT_RSP 15
#define DCE_TYPE_SHUTDOWN 17
#define DCE_TYPE_COCANCEL 18
#define DCE_TYPE_ORPHANED 19
/**
 * DCE Header
 */
#define DCE_HDR_VERS 0
#define DCE_HDR_VERS_MINOR 1
#define DCE_HDR_PACKET_TYPE 2
#define DCE_HDR_PACKET_FLAGS 3
#define DCE_HDR_DATA_REPRESENTATION 4
#define DCE_HDR_FRAG_LENGTH 8
#define DCE_HDR_AUTH_LENGTH 10
#define DCE_HDR_CALL_ID 12
#define DCE_HDR_SIZE 16
/**
 * DCE Versions
 */
#define DCE_PROTOCOL_MAJOR 5
#define DCE_PROTOCOL_MINOR 0
/**
 * DCE Interface Definitions
 */
#define DCE_INTERFACE_VERSION 2
#define DCE_INTERFACE_VERSION_MINOR 0
/**
 * DCE Bind Constants
 */
#define DCE_ASSOC_GROUP 0x000ae344
#define DCE_SEC_ADDR "\\PIPE\\srvsvc"
#define DCE_ACK_RESULT_ACCEPTANCE 0
#define DCE_ACK_RESULT_REJECTION 2
#define DCE_ACK_REASON_UNSUPPORTED 2
/**
 * DCE Flags
 */
#define DCE_FLAGS_OBJECT 0x80
#define DCE_FLAGS_MAYBE 0x40
#define DCE_FLAGS_NOEXECUTE 0x20
#define DCE_FLAGS_MULTIPLEX 0x10
#define DCE_FLAGS_RESERVED 0x08
#define DCE_FLAGS_CANCEL 0x04
#define DCE_FLAGS_LASTFRAG 0x02
#define DCE_FLAGS_FIRSTFRAG 0x01
/**
 * Data Representation
 */
#define DCE_DATA_REP_CHAR  0x0000000F
#define DCE_DATA_REP_INT   0x000000F0
#define DCE_DATA_REP_FLOAT 0x0000FF00
/**
 * Character Values
 */
#define DCE_DATA_REP_ASCII 0x00000000
#define DCE_DATA_REP_EBCDIC 0x00000001
/**
 * Integer Values
 */
#define DCE_DATA_REP_BE 0x00000000
#define DCE_DATA_REP_LE 0x00000010
/**
 * Float Values
 */
#define DCE_DATA_REP_IEEE 0x00000000
#define DCE_DATA_REP_VAX 0x00000100
#define DCE_DATA_REP_CRAY 0x00000200
#define DCE_DATA_REP_IBM 0x00000300
/**
 * DCE Requests
 */
#define DCE_REQUEST_NET_SHAREENUM 15
#define DCE_REQUEST_NET_SHAREGETINFO 16
#define DCE_REQUEST_NET_SERVERINFO 21
#define DCE_REQUEST_NET_WKSGETINFO 0
/**
 * DCE Share Types
 */
#define SHARE_TYPE_DIR 0
#define SHARE_TYPE_PRINTER 1
#define SHARE_TYPE_IPC 3
#define SHARE_TYPE_HIDDEN 0x80000000
/**
 * DCE Reference IDs
 */
#define REF_ID_SERVER_COMMENT 0x031fba22
#define REF_ID_SERVER_SHARE 0x01d7eb60
#define REF_ID_SHARE_COMMENT 0x024af656
#define REF_ID_SERVER 0x031fba10
#define REF_ID_SHARE_TYPE 0x024af646
#define REF_ID_SERVERINFO_101 0x031fb9f8
#define REF_ID_SHAREINFO_1 0x024ad658
#define REF_ID_SHARE_ENUM 0x035f7130
#define REF_ID_SHARE_INFO_ARRAY 0x03608f90
#define REF_ID_SHARE_INFO 0x0306af84
#define REF_ID_SHARE_NAME_INFO 0x0360af8e
#define REF_ID_NETSHARE_CTR1 0x00001
#define REF_ID_RESUME_HANDLE 0x00020044

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Push a Result Block into the data portion of the transaction.
   *
   * \param dceMessage
   * Pointer to the transaction 
   *
   * \param result
   * The result code
   * 
   * \param reason
   * The Reason code
   *
   * \param uuid
   * The UUID of the server
   */
  OFC_CORE_LIB OFC_VOID 
  of_dce_push_result (OFC_MESSAGE *dceMessage, OFC_UINT16 result,
		     OFC_UINT16 reason, OFC_UUID *uuid,
		     OFC_UINT16 version) ;
  /**
   * Push a bind header 
   *
   * \param dceMessage
   * The transaction
   *
   * \param xmit_frag
   * The maximum size of the transmit message
   *
   * \param recv_frag
   * The maximum size of a message we receive
   *
   * \param group_id
   * Not Sure
   */
  OFC_CORE_LIB OFC_VOID 
  of_dce_push_bind_header(OFC_MESSAGE *dceMessage, OFC_UINT16 xmit_frag,
			  OFC_UINT16 recv_frag, OFC_UINT32 group_id) ;

  OFC_CORE_LIB OFC_VOID 
  of_dce_pop_request_header(OFC_MESSAGE *dceMessage, OFC_UINT32 *alloc_hint,
			   OFC_UINT16 *context_id, OFC_UINT16 *opnum) ;

  OFC_CORE_LIB OFC_VOID 
  of_dce_push_request_header(OFC_MESSAGE *dceMessage, OFC_UINT32 alloc_hint,
			    OFC_UINT16 context_id, OFC_UINT16 opnum) ;

  OFC_CORE_LIB OFC_LPTSTR 
  of_dce_pop_tstr(OFC_MESSAGE *dceMessage) ;

  OFC_CORE_LIB OFC_VOID 
  of_dce_push_tstr(OFC_MESSAGE *dceMessage, OFC_LPCTSTR str) ;

  OFC_CORE_LIB OFC_VOID 
  of_dce_push_share_name_comment(OFC_MESSAGE *dceMessage, OFC_LPCTSTR name,
				 OFC_LPCTSTR comment) ;

  OFC_CORE_LIB OFC_VOID 
  of_dce_push_share(OFC_MESSAGE *dceMessage, OFC_UINT32 type,
		    OFC_LPCTSTR name, OFC_LPCTSTR comment) ;

  OFC_CORE_LIB OFC_MESSAGE *
  of_dce_bind_ack(OFC_UINT32 dceCall) ;

  OFC_CORE_LIB OFC_MESSAGE *
  of_dce_bind_request(OFC_UUID service, OFC_UINT16 major, 
		      OFC_UINT16 minor) ;

  OFC_CORE_LIB OFC_MESSAGE *
  of_dce_read_message(OFC_HANDLE hFile) ;

  OFC_CORE_LIB OFC_BOOL 
  of_dce_write_message(OFC_HANDLE hFile, OFC_MESSAGE *dceMessage) ;

  OFC_CORE_LIB OFC_MESSAGE *
  of_dce_transact(OFC_HANDLE hFile, OFC_MESSAGE *dceMessage) ;

#if defined(__cplusplus)
}
#endif

#endif
