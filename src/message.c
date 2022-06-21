/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/config.h"
#include "ofc/libc.h"
#include "ofc/net.h"
#include "ofc/net_internal.h"
#include "ofc/message.h"
#include "ofc/lock.h"
#include "ofc/thread.h"
#include "ofc/process.h"

#include "ofc/heap.h"
#include "ofc/persist.h"

#if defined(OFC_MESSAGE_DEBUG)
static struct _OFC_MESSAGE *OfcMessageAlloc ;
OFC_LOCK OfcMessageDebugLock ;

OFC_CORE_LIB OFC_VOID 
ofc_message_debug_init(OFC_VOID)
{
  OfcMessageAlloc = OFC_NULL ;
  OfcMessageDebugLock = ofc_lock_init () ;
}

OFC_CORE_LIB OFC_VOID
ofc_message_debug_destroy(OFC_VOID)
{
  struct _OFC_MESSAGE *msg ;

  for (msg = OfcMessageAlloc ;
       msg != OFC_NULL ;
       msg = OfcMessageAlloc)
    {
      ofc_message_debug_free (msg);
    }
  ofc_lock_destroy(OfcMessageDebugLock);
}
  
OFC_CORE_LIB OFC_VOID 
ofc_message_debug_alloc(struct _OFC_MESSAGE *msg, OFC_VOID *ret)
{
  /*
   * Put on the allocation queue
   */
  ofc_lock(OfcMessageDebugLock) ;
  msg->dbgnext = OfcMessageAlloc ;
  if (OfcMessageAlloc != OFC_NULL)
    OfcMessageAlloc->dbgprev = msg ;
  OfcMessageAlloc = msg ;
  msg->dbgprev = OFC_NULL ;
  ofc_unlock(OfcMessageDebugLock) ;
#if defined(__GNUC__) && defined(OFC_STACK_TRACE)
  msg->caller1 = __builtin_return_address(1) ;
  msg->caller2 = __builtin_return_address(2) ;
  msg->caller3 = __builtin_return_address(3) ;
  msg->caller4 = __builtin_return_address(4) ;
#else
  msg->caller = ret ;
#endif
}

OFC_CORE_LIB OFC_VOID 
ofc_message_debug_free(struct _OFC_MESSAGE *msg)
{
  /*
   * Pull off the allocation queue
   */
  ofc_lock(OfcMessageDebugLock) ;
  if (msg->dbgprev != OFC_NULL)
    msg->dbgprev->dbgnext = msg->dbgnext ;
  else
    OfcMessageAlloc = msg->dbgnext ;
  if (msg->dbgnext != OFC_NULL)
    msg->dbgnext->dbgprev = msg->dbgprev ;
  ofc_unlock(OfcMessageDebugLock) ;
}

OFC_CORE_LIB OFC_VOID 
ofc_message_debug_dump(OFC_VOID)
{
  struct _OFC_MESSAGE *msg ;

#if defined(__GNUC__)
  ofc_printf("%-10s %-10s %-10s %-10s %-10s %10s\n", 
           "Address", "Size", "Caller1", "Caller2", "Caller3", "Caller4") ;

  for (msg = OfcMessageAlloc ; msg != OFC_NULL ; msg = msg->dbgnext)
    {
      ofc_printf ("%-10p %-10d %-10p %-10p %-10p %-10p\n",
           msg, msg->map[0].length, msg->caller1, msg->caller2, msg->caller3,
           msg->caller4) ;
    }
#else
  ofc_printf ("%-20s %-10s %-20s\n", "Address", "Size", "Caller") ;

  for (msg = OfcMessageAlloc ; msg != OFC_NULL ; msg = msg->dbgnext)
    {
      ofc_printf ("%-20p %-10d %-20p\n", msg, msg->map[0].length, msg->caller) ;
    }
#endif
  ofc_printf ("\n") ;
}

#endif

OFC_CORE_LIB OFC_MESSAGE *
ofc_message_create(MSG_ALLOC_TYPE msgType, OFC_SIZET msgDataLength,
                   OFC_VOID *msgData) {
    OFC_MESSAGE *msg;
    OFC_INT i;

    msg = ofc_malloc(sizeof(OFC_MESSAGE));
    if (msg == OFC_NULL)
        ofc_process_crash("message: Couldn't alloc message\n");
    else {
        msg->destroy_after_send = OFC_FALSE;
        msg->offset = 0;
        msg->context = OFC_NULL;
        msg->send_size = msgDataLength;
        msg->count = msg->send_size;
        if (msg->count < 0)
          ofc_process_crash("here\n");
        msg->endian = MSG_ENDIAN_BIG;
        msg->FIFO1base = 0;
        msg->FIFO1 = 0;
        msg->FIFO1size = 0;
        msg->FIFO1rem = 0;
        msg->base = 0;
        for (i = 0 ; i < MAX_MAP; i++)
          {
            msg->map[i].ptr = OFC_NULL;
          }

        if (msgData == OFC_NULL)
          {
            if (msgType == MSG_ALLOC_HEAP)
              {
                msg->map[0].length = msgDataLength;
                msg->map[0].alloc = msgType;
                msg->map[0].ptr = ofc_malloc(msgDataLength);
                if (msg->map[0].ptr == OFC_NULL)
                  {
                    ofc_free(msg);
                    msg = OFC_NULL;
                  }
              }
            else
              {
                msg->map[0].length = msgDataLength;
                msg->map[0].alloc = msgType;
                msg->map[0].ptr = OFC_NULL;
              }
          }
        else
          {
            msg->map[0].length = msgDataLength;
            msg->map[0].alloc = msgType;
            msg->map[0].ptr = msgData;
          }

#if defined(OFC_MESSAGE_DEBUG)
        if (msg != OFC_NULL)
          ofc_message_debug_alloc(msg, RETURN_ADDRESS()) ;
#endif
    }
    return (msg);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_realloc(OFC_MESSAGE *msg, OFC_SIZET msgDataLength) {
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (msg->map[0].alloc == MSG_ALLOC_HEAP)
      {
        msg->map[0].length = msgDataLength;
        msg->map[0].ptr = ofc_realloc(msg->map[0].ptr, msg->map[0].length);
        if (msg->map[0].ptr != OFC_NULL)
            ret = OFC_TRUE;
    }
    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_message_set_addr(OFC_MESSAGE *msg, OFC_IPADDR *ip, OFC_UINT16 port) {
    if (ip != OFC_NULL)
        ofc_memcpy(&msg->ip, ip, sizeof(OFC_IPADDR));
    msg->port = port;
}

OFC_CORE_LIB OFC_MESSAGE *
ofc_datagram_create(MSG_ALLOC_TYPE msgType, OFC_SIZET msgDataLength,
                    OFC_CHAR *msgData, OFC_IPADDR *ip, OFC_UINT16 port) {
    OFC_MESSAGE *msg;

    msg = ofc_message_create(msgType, msgDataLength, msgData);
    if (msg != OFC_NULL)
        ofc_message_set_addr(msg, ip, port);

    return (msg);
}

OFC_CORE_LIB OFC_VOID
ofc_message_destroy(OFC_MESSAGE *msg)
{
  OFC_INT i;

  for (i = 0 ; msg->map[i].ptr != OFC_NULL ; i++)
    {
      if (msg->map[i].alloc == MSG_ALLOC_HEAP)
        ofc_free(msg->map[i].ptr);
    }
#if defined(OFC_MESSAGE_DEBUG)
  ofc_message_debug_free(msg) ;
#endif
  ofc_free(msg);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_done(OFC_MESSAGE *msg) {
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (msg->count == 0)
        ret = OFC_TRUE;
    if (msg->count < 0) {
        ofc_heap_dump_chunk(msg);
        ofc_process_crash("Message has negative count\n");
    }
    return (ret);
}

OFC_CORE_LIB OFC_VOID *
ofc_message_data(OFC_MESSAGE *msg) {
    return (msg->map[0].ptr);
}

OFC_CORE_LIB OFC_VOID *
ofc_message_unload_data(OFC_MESSAGE *msg) {
    OFC_VOID *data;

    data = msg->map[0].ptr;
    msg->map[0].ptr = OFC_NULL;

    return (data);
}

OFC_CORE_LIB OFC_INT
ofc_message_offset(OFC_MESSAGE *msg) {
    return (msg->offset);
}

OFC_CORE_LIB OFC_VOID
ofc_message_set_send_size(OFC_MESSAGE *msg, OFC_SIZET size) {
    msg->send_size = size;
    msg->count = msg->send_size;
    if (msg->count < 0)
      ofc_process_crash("here\n");
}

OFC_CORE_LIB OFC_VOID
ofc_message_addr(OFC_MESSAGE *msg, OFC_IPADDR *ip, OFC_UINT16 *port) {
    if (ip != OFC_NULL)
        ofc_memcpy(ip, &msg->ip, sizeof(OFC_IPADDR));
    if (port != OFC_NULL)
        *port = msg->port;
}

OFC_CORE_LIB OFC_VOID
ofc_message_reset(OFC_MESSAGE *msg) {
    msg->count = msg->send_size;
    if (msg->count < 0)
      ofc_process_crash("here\n");
    msg->offset = 0;
    msg->FIFO1 = msg->FIFO1base;
    msg->FIFO1rem = msg->FIFO1size;
    msg->endian = MSG_ENDIAN_BIG;
    msg->base = 0;
}

OFC_CORE_LIB OFC_VOID
ofc_message_fifo_reset(OFC_MESSAGE *msg) {
    msg->FIFO1 = msg->FIFO1base;
    msg->FIFO1rem = msg->FIFO1size;
}

OFC_CORE_LIB OFC_VOID
ofc_message_set_base(OFC_MESSAGE *msg, OFC_INT base) {
    msg->base = base;
}

OFC_CORE_LIB OFC_INT
ofc_message_get_base(OFC_MESSAGE *msg) {
    return (msg->base);
}

OFC_CORE_LIB OFC_SIZET
ofc_message_get_length(OFC_MESSAGE *msg)
{
  OFC_INT i;
  OFC_SIZET len;

  len = 0;
  for (i = 0 ; msg->map[i].ptr != OFC_NULL ; i++)
    {
      len += msg->map[i].length;
    }
  return (len);
}

OFC_VOID ofc_message_param_put_indirect(OFC_MESSAGE *msg,
                                        OFC_INT offset,
                                        OFC_VOID *buffer,
                                        OFC_SIZET size)
{
  ofc_message_put_indirect(msg, offset + msg->param_offset,
                           buffer, size);
}
  
OFC_VOID ofc_message_put_indirect(OFC_MESSAGE *msg,
                                  OFC_INT offset,
                                  OFC_VOID *buffer,
                                  OFC_SIZET size)
{
  OFC_INT i;
  OFC_INT j;
  OFC_SIZET loc;
  OFC_OFFT map_off;
  OFC_CHAR *rest;
  OFC_SIZET rem;

  loc = 0;
  offset += msg->base;

  for (i = 0; ((msg->map[i].ptr != OFC_NULL) &&
               (loc + msg->map[i].length <= offset));
       i++)
    loc += msg->map[i].length;

  map_off = offset - loc;

  /*
   * Insert segment here
   */
  for (j = i; msg->map[j].ptr != OFC_NULL ; j++);

  if (map_off == 0)
    {
      /*
       * If we're inserting right at the beginning of the buffer,
       * then we don't need to split it.  Just insert this buffer
       * in maps.  make room
       */
      for ( ; j > i; j--)
        {
          msg->map[j] = msg->map[j-1];
        }

      msg->map[i].ptr = buffer;
      msg->map[i].alloc = MSG_ALLOC_HEAP;
      msg->map[i].length = size;
    }
  else
    {
      /*
       * Insert two spots
       */
      j++;
      for ( ; j > (i+1); j--)
        {
          msg->map[j] = msg->map[j-2];
        }

      rem = msg->map[i].length - map_off - size;
      rest = (OFC_CHAR *) msg->map[i].ptr + map_off + size;
      msg->map[i].length = map_off;
      msg->map[i+1].ptr = buffer;
      msg->map[i+1].alloc = MSG_ALLOC_HEAP;
      msg->map[i+1].length = size;
      msg->map[i+2].ptr = rest;
      msg->map[i+2].alloc = MSG_ALLOC_STATIC;
      msg->map[i+2].length = rem;
    }
}  

OFC_VOID ofc_message_get_map(OFC_MESSAGE *msg,
                             OFC_SIZET total,
                             OFC_SIZET *num_elem,
                             const OFC_UCHAR **addr,
                             OFC_SIZET *len)
{
  OFC_INT i;
  OFC_SIZET offset = 0;

  offset += msg->base;

  for (i = 0 ; total > 0; i++)
    {
      OFC_SIZET seg_size;
      addr[i] = (const OFC_UCHAR *) message_map(msg, offset, 0, &seg_size);
      len[i] = OFC_MIN(seg_size, total);
      total -= len[i];
      offset += len[i];
    }
  *num_elem = i;
}

OFC_CHAR *message_map(OFC_MESSAGE *msg, OFC_INT offset, OFC_SIZET len,
                      OFC_SIZET *seg_size)
{
  OFC_INT i;
  OFC_SIZET loc;
  OFC_OFFT map_off;
  OFC_CHAR *ret;

  loc = 0;
  for (i = 0; ((msg->map[i].ptr != OFC_NULL) &&
               (loc + msg->map[i].length <= offset));
       i++)
    loc += msg->map[i].length;

  map_off = offset - loc;

  if ((msg->map[i].ptr != OFC_NULL) &&
      (map_off + len <= msg->map[i].length))
    {
      ret = msg->map[i].ptr + map_off;
      if (seg_size != OFC_NULL)
        *seg_size = msg->map[i].length - map_off;
    }
  else
    {
      ret = OFC_NULL;
      if (seg_size != OFC_NULL)
        *seg_size = 0;
    }

  return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_put_u8(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT8 value)
{
  OFC_BOOL ret;
  OFC_CHAR *ptr;

  ret = OFC_FALSE;
  offset += msg->base;
  ptr = message_map(msg, offset, sizeof(OFC_UINT8), OFC_NULL);

  if (ptr != OFC_NULL)
    {
      ret = OFC_TRUE;
      if (msg->endian == MSG_ENDIAN_BIG)
        {
          OFC_NET_CTON (ptr, 0, value);
        }
      else
        {
          OFC_NET_CTOSMB (ptr, 0, value);
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_put_u16(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT16 value)
{
  OFC_BOOL ret;
  OFC_CHAR *ptr;

  ret = OFC_FALSE;
  offset += msg->base;

  ptr = message_map(msg, offset, sizeof(OFC_UINT16), OFC_NULL);

  if (ptr != OFC_NULL)
    {
      ret = OFC_TRUE;
      if (msg->endian == MSG_ENDIAN_BIG)
        {
          OFC_NET_STON (ptr, 0, value);
        }
      else
        {
          OFC_NET_STOSMB (ptr, 0, value);
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_put_u32(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT32 value)
{
  OFC_BOOL ret;
  OFC_CHAR *ptr;

  ret = OFC_FALSE;
  offset += msg->base;

  ptr = message_map(msg, offset, sizeof(OFC_UINT32), OFC_NULL);

  if (ptr != OFC_NULL)
    {
      ret = OFC_TRUE;
      if (msg->endian == MSG_ENDIAN_BIG)
        {
          OFC_NET_LTON (ptr, 0, value);
        }
      else
        {
          OFC_NET_LTOSMB (ptr, 0, value);
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_put_u64(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT64 *value)
{
  OFC_BOOL ret;
  OFC_CHAR *ptr;

  ret = OFC_FALSE;
  offset += msg->base;

  ptr = message_map(msg, offset, sizeof(OFC_UINT64), OFC_NULL);

  if (ptr != OFC_NULL)
    {
      ret = OFC_TRUE;
#if defined (OFC_64BIT_INTEGER)
      if (msg->endian == MSG_ENDIAN_BIG)
        {
          OFC_NET_LLTON (ptr, 0, *value);
        }
      else
        {
          OFC_NET_LLTOSMB (ptr, 0, *value);
        }
#else
      if (msg->endian == MSG_ENDIAN_BIG)
        {
          OFC_NET_LTON (ptr, 0, value->low) ;
          OFC_NET_LTON (ptr, 4, value->high) ;
        }
      else
        {
          OFC_NET_LTOSMB (ptr, 4, value->low) ;
          OFC_NET_LTOSMB (ptr, 0, value->high) ;
        }
#endif
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_put_tstr(OFC_MESSAGE *msg, OFC_INT offset, OFC_LPCTSTR str)
{
  OFC_BOOL ret;
  OFC_SIZET len;
  OFC_CTCHAR *p;
  OFC_INT i;

  ret = OFC_TRUE;
  if (str != OFC_NULL)
    {
      len = (ofc_tstrlen(str) + 1) * sizeof(OFC_WORD);

      p = str;
      for (i = 0, p = str; i < len && ret == OFC_TRUE;
           i += sizeof(OFC_WORD), p++)
        {
          ret = ofc_message_put_u16(msg, offset + i, *p) ;
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_SIZET
ofc_message_get_tstring_len(OFC_MESSAGE *msg, OFC_INT offset,
                            OFC_SIZET max_len)
{
  OFC_INT i;
  OFC_TCHAR c;
  OFC_BOOL done;

  /*
   * Include the EOS in the string len
   */
  done = OFC_FALSE;
  /*
   * This loop counts the EOS if the string is less then max_len
   * which is what we want
   */
  for (i = 0; i < max_len && !done; i++)
    {
      c = ofc_message_get_u16(msg, offset + (i * sizeof(OFC_WORD)));
      if (c == TCHAR_EOS)
        done = OFC_TRUE;
    }
  return (i);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_get_tstr(OFC_MESSAGE *msg, OFC_INT offset, OFC_LPTSTR *str) {
    OFC_BOOL ret;
    OFC_TCHAR *p;
    OFC_INT i;
    OFC_SIZET string_len;
    OFC_SIZET max_len;

    ret = OFC_TRUE;


    max_len = ofc_message_get_length(msg);
    string_len = ofc_message_get_tstring_len(msg, offset, max_len - offset);

    *str = ofc_malloc(string_len * sizeof(OFC_TCHAR));

    for (i = 0, p = *str; i < string_len; i++, p++) {
        *p = ofc_message_get_u16(msg, offset + (i * sizeof(OFC_WORD)));
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_put_tstrn(OFC_MESSAGE *msg, OFC_INT offset, OFC_LPCTSTR str,
                      OFC_SIZET len) {
    OFC_BOOL ret;
    OFC_CTCHAR *p;
    OFC_INT i;

    len = OFC_MIN (len, ofc_tstrnlen(str, len)) * sizeof(OFC_WORD);

    ret = OFC_TRUE;
    p = str;
    for (i = 0, p = str; i < len && ret == OFC_TRUE;
         i += sizeof(OFC_WORD), p++) {
        ret = ofc_message_put_u16(msg, offset + i, *p);
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_get_tstrn(OFC_MESSAGE *msg, OFC_INT offset, OFC_LPTSTR *str,
                      OFC_SIZET max_len) {
    OFC_BOOL ret;
    OFC_TCHAR *p;
    OFC_INT i;
    OFC_SIZET string_len;

    ret = OFC_TRUE;

    string_len = ofc_message_get_tstring_len(msg, offset, max_len);
    *str = ofc_malloc((string_len + 1) * sizeof(OFC_TCHAR));

    for (i = 0, p = *str; i < string_len; i++, p++) {
        *p = ofc_message_get_u16(msg, offset + (i * sizeof(OFC_WORD)));
    }
    *p = TCHAR_EOS;

    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_get_tstrnx(OFC_MESSAGE *msg, OFC_INT offset,
                       OFC_LPTSTR str, OFC_SIZET max_len) {
    OFC_BOOL ret;
    OFC_TCHAR *p;
    OFC_INT i;
    OFC_SIZET string_len;

    ret = OFC_TRUE;

    string_len = ofc_message_get_tstring_len(msg, offset, max_len);

    for (i = 0, p = str; i < string_len; i++, p++) {
        *p = ofc_message_get_u16(msg, offset + (i * sizeof(OFC_WORD)));
    }

    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_put_cstr(OFC_MESSAGE *msg, OFC_INT offset, OFC_LPCSTR str) {
    OFC_BOOL ret;
    OFC_SIZET len;
    OFC_CCHAR *p;
    OFC_INT i;

    ret = OFC_TRUE;
    if (str != OFC_NULL) {
        len = ofc_strlen(str) + 1;

        p = str;

        for (i = 0, p = str; i < len && ret == OFC_TRUE; i++, p++) {
            ret = ofc_message_put_u8(msg, offset + i, *p);
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_UINT8
ofc_message_get_u8(OFC_MESSAGE *msg, OFC_INT offset)
{
  OFC_UINT8 value;
  OFC_CHAR *ptr;

  value = 0;
  offset += msg->base;

  ptr = message_map(msg, offset, sizeof(OFC_UINT8), OFC_NULL);

  if (ptr != OFC_NULL)
    {
      if (msg->endian == MSG_ENDIAN_BIG)
        value = OFC_NET_NTOC (ptr, 0);
      else
        value = OFC_NET_SMBTOC (ptr, 0);
    }
  return (value);
}

OFC_CORE_LIB OFC_UINT16
ofc_message_get_u16(OFC_MESSAGE *msg, OFC_INT offset)
{
  OFC_UINT16 value;
  OFC_CHAR *ptr;

  value = 0;
  offset += msg->base;

  ptr = message_map(msg, offset, sizeof(OFC_UINT16), OFC_NULL);

  if (ptr != OFC_NULL)
    {
      if (msg->endian == MSG_ENDIAN_BIG)
        value = OFC_NET_NTOS (ptr, 0);
      else
        value = OFC_NET_SMBTOS (ptr, 0);
    }
  return (value);
}

OFC_CORE_LIB OFC_UINT32
ofc_message_get_u32(OFC_MESSAGE *msg, OFC_INT offset)
{
  OFC_UINT32 value;
  OFC_VOID *ptr;

  value = 0;
  offset += msg->base;

  ptr = message_map(msg, offset, sizeof(OFC_UINT32), OFC_NULL);

  if (ptr != OFC_NULL)
    {
      if (msg->endian == MSG_ENDIAN_BIG)
        value = OFC_NET_NTOL (ptr, 0);
      else
        value = OFC_NET_SMBTOL (ptr, 0);
    }
  return (value);
}

OFC_CORE_LIB OFC_VOID
ofc_message_get_u64(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT64 *value)
{
  OFC_CHAR *ptr;

#if defined(OFC_64BIT_INTEGER)
  *value = 0;
#else
  value->low = 0 ;
  value->high = 0 ;
#endif
  offset += msg->base;

  ptr = message_map(msg, offset, sizeof(OFC_UINT64), OFC_NULL);

  if (ptr != OFC_NULL)
    {
#if defined (OFC_64BIT_INTEGER)
      if (msg->endian == MSG_ENDIAN_BIG)
        *value = OFC_NET_NTOLL (ptr, 0);
      else
        *value = OFC_NET_SMBTOLL (ptr, 0);
#else
      if (msg->endian == MSG_ENDIAN_BIG)
        {
          value->low = OFC_NET_NTOL (ptr, 0) ;
          value->high = OFC_NET_NTOL (ptr, 4) ;
        }
      else
        {
          value->low = OFC_NET_SMBTOL (ptr, 4) ;
          value->high = OFC_NET_SMBTOL (ptr, 0) ;
        }
#endif
    }
}

OFC_CORE_LIB OFC_VOID *
ofc_message_get_pointer(OFC_MESSAGE *msg, OFC_INT offset)
{
  OFC_CHAR *ptr;

  offset += msg->base;

  ptr = message_map(msg, offset, 0, OFC_NULL);

  return (ptr);
}

OFC_CORE_LIB OFC_VOID
ofc_message_fifo_set(OFC_MESSAGE *msg, OFC_INT offset, OFC_SIZET size) {
    msg->FIFO1base = offset;
    msg->FIFO1 = offset;
    msg->FIFO1size = (OFC_INT) size;
    msg->FIFO1rem = size;
}

OFC_CORE_LIB OFC_VOID
ofc_message_fifo_align(OFC_MESSAGE *msg, OFC_INT align) {
    OFC_INT mask;
    OFC_INT pad;

    mask = align - 1;
    if (msg->FIFO1 & mask) {
        pad = align - (msg->FIFO1 & mask);
        msg->FIFO1 += pad;
        msg->FIFO1rem -= pad;
    }
}

OFC_CORE_LIB OFC_VOID *
ofc_message_fifo_push(OFC_MESSAGE *msg, OFC_SIZET size) {
    OFC_VOID *ret;

    ret = OFC_NULL;
    if (size <= msg->FIFO1rem) {
        ret = ofc_message_get_pointer(msg, msg->FIFO1);
        msg->FIFO1 += size;
        msg->FIFO1rem -= size;
    }
    return (ret);
}

OFC_CORE_LIB OFC_VOID *
ofc_message_fifo_pop(OFC_MESSAGE *msg, OFC_SIZET size) {
    OFC_VOID *ret;

    ret = OFC_NULL;
    if (size <= msg->FIFO1rem) {
        ret = ofc_message_get_pointer(msg, msg->FIFO1);
        msg->FIFO1 += size;
        msg->FIFO1rem -= size;
    }
    return (ret);
}

OFC_CORE_LIB OFC_UINT8
ofc_message_fifo_pop_u8(OFC_MESSAGE *msg) {
    OFC_UINT8 ret;

    ret = 0;
    if (SIZEOF_U8 <= msg->FIFO1rem) {
        ret = ofc_message_get_u8(msg, msg->FIFO1);
        msg->FIFO1 += SIZEOF_U8;
        msg->FIFO1rem -= SIZEOF_U8;
    }
    return (ret);
}

OFC_CORE_LIB OFC_UINT16
ofc_message_fifo_pop_u16(OFC_MESSAGE *msg) {
    OFC_UINT16 ret;

    ret = 0;
    if (SIZEOF_U16 <= msg->FIFO1rem) {
        ret = ofc_message_get_u16(msg, msg->FIFO1);
        msg->FIFO1 += SIZEOF_U16;
        msg->FIFO1rem -= SIZEOF_U16;
    }
    return (ret);
}

OFC_CORE_LIB OFC_UINT32
ofc_message_fifo_pop_u32(OFC_MESSAGE *msg) {
    OFC_UINT32 ret;

    ret = 0;
    if (SIZEOF_U32 <= msg->FIFO1rem) {
        ret = ofc_message_get_u32(msg, msg->FIFO1);
        msg->FIFO1 += SIZEOF_U32;
        msg->FIFO1rem -= SIZEOF_U32;
    }
    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_message_fifo_pop_u64(OFC_MESSAGE *msg, OFC_UINT64 *val) {
#if defined(OFC_64BIT_INTEGER)
    *val = 0;
#else
    val->low = 0 ;
    val->high = 0 ;
#endif

    if (SIZEOF_U64 <= msg->FIFO1rem) {
        ofc_message_get_u64(msg, msg->FIFO1, val);
        msg->FIFO1 += SIZEOF_U64;
        msg->FIFO1rem -= SIZEOF_U64;
    }
}

OFC_CORE_LIB OFC_CHAR *
ofc_message_fifo_pop_cstring(OFC_MESSAGE *msg) {
    OFC_CHAR *ret;
    OFC_SIZET len;

    ret = ofc_message_get_pointer(msg, msg->FIFO1);

    if (ret != OFC_NULL) {
        len = ofc_strlen(ret) + 1;
        if (len <= msg->FIFO1rem) {
            msg->FIFO1 += len;
            msg->FIFO1rem -= len;
        } else
            ret = OFC_NULL;
    }
    return (ret);
}

OFC_CORE_LIB OFC_TCHAR *
ofc_message_fifo_pop_tstr(OFC_MESSAGE *msg) {
    OFC_LPTSTR ret;
    OFC_SIZET string_len;

    ret = OFC_NULL;

    string_len =
            ofc_message_get_tstring_len(msg, ofc_message_fifo_get(msg),
                                        (OFC_SIZET)
                                                (msg->FIFO1rem / sizeof(OFC_WORD)));

    ret = ofc_malloc(string_len * sizeof(OFC_TCHAR));

    if (ret != OFC_NULL) {
        ofc_message_get_tstrnx(msg, ofc_message_fifo_get(msg), ret,
                               string_len);

        ret[string_len - 1] = TCHAR_EOS;

        msg->FIFO1 += string_len * sizeof(OFC_WORD);
        msg->FIFO1rem -= string_len * sizeof(OFC_WORD);
    }

    return (ret);
}

OFC_CORE_LIB OFC_LPTSTR
ofc_message_fifo_pop_tstrn(OFC_MESSAGE *msg, OFC_SIZET max_len) {
    OFC_LPTSTR ret;
    OFC_SIZET string_len;

    ret = OFC_NULL;
    string_len = ofc_message_get_tstring_len(msg, ofc_message_fifo_get(msg),
                                             max_len);
    string_len =
            OFC_MIN (string_len,
                     (OFC_SIZET) (msg->FIFO1rem / sizeof(OFC_WORD)));

    ofc_message_get_tstrn(msg, ofc_message_fifo_get(msg), &ret, string_len);

    msg->FIFO1 += string_len * sizeof(OFC_WORD);
    msg->FIFO1rem -= string_len * sizeof(OFC_WORD);

    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_message_fifo_pop_tstrnx(OFC_MESSAGE *msg, OFC_LPTSTR str,
                            OFC_SIZET max_len) {
    OFC_SIZET string_len;

    ofc_message_fifo_align(msg, 2);

    string_len = ofc_message_get_tstring_len(msg, ofc_message_fifo_get(msg),
                                             max_len);
    string_len =
            OFC_MIN (string_len,
                     (OFC_SIZET) (msg->FIFO1rem / sizeof(OFC_WORD)));

    ofc_message_get_tstrnx(msg, ofc_message_fifo_get(msg), str, string_len);

    msg->FIFO1 += string_len * sizeof(OFC_WORD);
    msg->FIFO1rem -= string_len * sizeof(OFC_WORD);
}

OFC_CORE_LIB OFC_LPCSTR
ofc_message_fifo_pop_cstrn(OFC_MESSAGE *msg, OFC_SIZET len) {
    OFC_LPSTR ret;
    OFC_LPSTR p;
    int i;

    ret = (OFC_LPSTR) ofc_message_get_pointer(msg, msg->FIFO1);

    if (ret != OFC_NULL) {
        for (i = 0, p = ret; i < len && *p != '\0'; i++, p++);
        /*
         * If we still have room for the eos, let's add one
         */
        if (i < len)
            i++;

        len = i;
        if (len <= msg->FIFO1rem) {
            msg->FIFO1 += len;
            msg->FIFO1rem -= len;
        } else
            ret = OFC_NULL;
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_push_u8(OFC_MESSAGE *msg, OFC_UINT8 value) {
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (SIZEOF_U8 <= msg->FIFO1rem) {
        ret = ofc_message_put_u8(msg, msg->FIFO1, value);
        if (ret == OFC_TRUE) {
            msg->FIFO1 += SIZEOF_U8;
            msg->FIFO1rem -= SIZEOF_U8;
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_push_u16(OFC_MESSAGE *msg, OFC_UINT16 value) {
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (SIZEOF_U16 <= msg->FIFO1rem) {
        ret = ofc_message_put_u16(msg, msg->FIFO1, value);
        if (ret == OFC_TRUE) {
            msg->FIFO1 += SIZEOF_U16;
            msg->FIFO1rem -= SIZEOF_U16;
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_push_u32(OFC_MESSAGE *msg, OFC_UINT32 value) {
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (SIZEOF_U32 <= msg->FIFO1rem) {
        ret = ofc_message_put_u32(msg, msg->FIFO1, value);
        if (ret == OFC_TRUE) {
            msg->FIFO1 += SIZEOF_U32;
            msg->FIFO1rem -= SIZEOF_U32;
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_push_u64(OFC_MESSAGE *msg, OFC_UINT64 *value) {
    OFC_BOOL ret;

    ret = OFC_FALSE;
    if (SIZEOF_U64 <= msg->FIFO1rem) {
        ret = ofc_message_put_u64(msg, msg->FIFO1, value);
        if (ret == OFC_TRUE) {
            msg->FIFO1 += SIZEOF_U64;
            msg->FIFO1rem -= SIZEOF_U64;
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_push_tstr(OFC_MESSAGE *msg, OFC_LPCTSTR str) {
    OFC_BOOL ret;
    OFC_SIZET len;

    ret = OFC_FALSE;

    len = (ofc_tstrlen(str) + 1) * sizeof(OFC_WORD);
    if (len <= msg->FIFO1rem) {
        ret = ofc_message_put_tstr(msg, msg->FIFO1, str);
        if (ret == OFC_TRUE) {
            msg->FIFO1 += len;
            msg->FIFO1rem -= len;
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_push_tstrn(OFC_MESSAGE *msg, OFC_LPCTSTR str,
                            OFC_SIZET len) {
    OFC_BOOL ret;
    OFC_SIZET tlen;

    ret = OFC_FALSE;

    len = OFC_MIN (len, ofc_tstrnlen(str, len));
    tlen = len * sizeof(OFC_WORD);

    if (tlen <= msg->FIFO1rem) {
        ret = ofc_message_put_tstrn(msg, msg->FIFO1, str, len);
        if (ret == OFC_TRUE) {
            msg->FIFO1 += tlen;
            msg->FIFO1rem -= tlen;
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_push_cstr(OFC_MESSAGE *msg, OFC_LPCSTR str) {
    OFC_BOOL ret;
    OFC_SIZET len;

    ret = OFC_FALSE;

    len = ofc_strlen(str) + 1;
    if (len <= msg->FIFO1rem) {
        ret = ofc_message_put_cstr(msg, msg->FIFO1, str);
        if (ret == OFC_TRUE) {
            msg->FIFO1 += len;
            msg->FIFO1rem -= len;
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_INT
ofc_message_fifo_get(OFC_MESSAGE *msg) {
    OFC_INT ret;

    ret = msg->FIFO1;
    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_message_fifo_update(OFC_MESSAGE *msg, OFC_INT offset) {
    OFC_INT distance;

    if (offset < msg->FIFO1size) {
        distance = offset - msg->FIFO1;
        msg->FIFO1 += distance;
        msg->FIFO1rem -= distance;
    }
}

OFC_CORE_LIB OFC_INT
ofc_message_fifo_rem(OFC_MESSAGE *msg) {
    OFC_INT rem;

    rem = (OFC_INT) msg->FIFO1rem;
    return (rem);
}

OFC_CORE_LIB OFC_INT
ofc_message_fifo_size(OFC_MESSAGE *msg) {
    OFC_INT size;

    size = msg->FIFO1size;
    return (size);
}

OFC_CORE_LIB OFC_INT
ofc_message_fifo_len(OFC_MESSAGE *msg) {
    OFC_INT len;

    len = msg->FIFO1 - msg->FIFO1base;
    return (len);
}

OFC_CORE_LIB OFC_INT
ofc_message_fifo_base(OFC_MESSAGE *msg) {
    OFC_INT base;

    base = msg->FIFO1base;
    return (base);
}

OFC_CORE_LIB OFC_CHAR *
ofc_message_fifo_base_pointer(OFC_MESSAGE *msg) {
    OFC_CHAR *ret;

    ret = ofc_message_get_pointer(msg, msg->FIFO1base);
    return (ret);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_valid(OFC_MESSAGE *msg) {
    return (msg->FIFO1rem >= 0);
}

OFC_CORE_LIB OFC_VOID
ofc_message_set_endian(OFC_MESSAGE *msg, MSG_ENDIAN endianess) {
    msg->endian = endianess;
}


OFC_CORE_LIB OFC_VOID
ofc_message_param_set(OFC_MESSAGE *msg, OFC_INT offset, OFC_SIZET len) {
    msg->param_offset = offset;
    msg->param_len = len;
}

OFC_CORE_LIB OFC_INT
ofc_message_param_get_offset(OFC_MESSAGE *msg) {
    return (msg->param_offset);
}

OFC_CORE_LIB OFC_INT
ofc_message_param_get_len(OFC_MESSAGE *msg) {
    return ((OFC_INT) msg->param_len);
}

OFC_CORE_LIB OFC_BOOL
ofc_message_param_get_tstr(OFC_MESSAGE *msg, OFC_INT offset, OFC_LPTSTR *str) {
    return (ofc_message_get_tstr(msg, msg->param_offset + offset, str));
}

OFC_CORE_LIB OFC_BOOL
ofc_message_param_put_tstr(OFC_MESSAGE *msg, OFC_INT offset, OFC_LPTSTR str) {
    return (ofc_message_put_tstr(msg, msg->param_offset + offset, str));
}

OFC_CORE_LIB OFC_BOOL
ofc_message_param_put_tstrn(OFC_MESSAGE *msg, OFC_INT offset,
                            OFC_LPCTSTR str, OFC_SIZET len) {
    return (ofc_message_put_tstrn(msg, msg->param_offset + offset, str, len));
}

OFC_CORE_LIB OFC_VOID *
ofc_message_param_get_pointer(OFC_MESSAGE *msg, OFC_INT offset) {
    return (ofc_message_get_pointer(msg, msg->param_offset + offset));
}

OFC_CORE_LIB OFC_VOID
ofc_message_param_put_u8(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT8 value) {
    if ((offset + SIZEOF_U8) <= msg->param_len)
        ofc_message_put_u8(msg, msg->param_offset + offset, value);
    else {
        ofc_process_crash("Param offset out of bounds\n");
    }
}

OFC_CORE_LIB OFC_VOID
ofc_message_param_put_u16(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT16 value) {
    if ((offset + SIZEOF_U16) <= msg->param_len)
        ofc_message_put_u16(msg, msg->param_offset + offset, value);
    else {
        ofc_process_crash("Param offset out of bounds\n");
    }
}

OFC_CORE_LIB OFC_VOID
ofc_message_param_put_u32(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT32 value) {
    if ((offset + SIZEOF_U32) <= msg->param_len)
        ofc_message_put_u32(msg, msg->param_offset + offset, value);
    else {
        ofc_process_crash("Param offset out of bounds\n");
    }
}

OFC_CORE_LIB OFC_VOID
ofc_message_param_put_u64(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT64 *value) {
    if ((offset + SIZEOF_U64) <= msg->param_len)
        ofc_message_put_u64(msg, msg->param_offset + offset, value);
    else {
        ofc_process_crash("Param offset out of bounds\n");
    }
}

OFC_CORE_LIB OFC_UINT8
ofc_message_param_get_u8(OFC_MESSAGE *msg, OFC_INT offset) {
    OFC_UINT8 ret;

    if ((offset + SIZEOF_U8) <= msg->param_len)
        ret = ofc_message_get_u8(msg, msg->param_offset + offset);
    else {
        ofc_process_crash("Param offset out of bounds\n");
        ret = 0;
    }
    return (ret);
}

OFC_CORE_LIB OFC_UINT16
ofc_message_param_get_u16(OFC_MESSAGE *msg, OFC_INT offset) {
    OFC_UINT16 ret;

    if ((offset + SIZEOF_U16) <= msg->param_len)
        ret = ofc_message_get_u16(msg, msg->param_offset + offset);
    else {
        ofc_process_crash("Param offset out of bounds\n");
        ret = 0;
    }
    return (ret);
}

OFC_CORE_LIB OFC_UINT32
ofc_message_param_get_u32(OFC_MESSAGE *msg, OFC_INT offset) {
    OFC_UINT32 ret;

    if ((offset + SIZEOF_U32) <= msg->param_len)
        ret = ofc_message_get_u32(msg, msg->param_offset + offset);
    else {
        ofc_process_crash("Param offset out of bounds\n");
        ret = 0;
    }
    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_message_param_get_u64(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT64 *value) {
    if ((offset + SIZEOF_U64) <= msg->param_len)
        ofc_message_get_u64(msg, msg->param_offset + offset, value);
    else {
        ofc_process_crash("Param offset out of bounds\n");
#if defined(OFC_64BIT_INTEGER)
        *value = 0;
#else
        value->low = 0 ;
        value->high = 0 ;
#endif
    }
}

OFC_CORE_LIB OFC_BOOL
ofc_message_from_subnet(OFC_MESSAGE *msg, OFC_INT iface, OFC_IPADDR *intip) {
    OFC_IPADDR fromip;
    OFC_UINT16 port;
    OFC_BOOL ret;
    OFC_IPADDR mask;

    ofc_message_addr(msg, &fromip, &port);

    ofc_persist_interface_addr(iface, intip, OFC_NULL, &mask);

    ret = ofc_net_subnet_match(&fromip, intip, &mask);

    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_message_set_context(OFC_MESSAGE *msg, OFC_VOID *context) {
    msg->context = context;
}

OFC_CORE_LIB OFC_VOID *
ofc_message_get_context(OFC_MESSAGE *msg) {
    return (msg->context);
}

OFC_CORE_LIB OFC_BOOL ofc_message_destroy_after_send(OFC_MESSAGE *msg) {
    OFC_BOOL ret;
    ret = msg->destroy_after_send;
    return (ret);
}

OFC_CORE_LIB OFC_VOID ofc_message_set_destroy_after_send(OFC_MESSAGE *msg) {
    msg->destroy_after_send = OFC_TRUE;
}

