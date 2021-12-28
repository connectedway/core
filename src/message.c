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
#include "ofc/message.h"
#include "ofc/lock.h"
#include "ofc/thread.h"
#include "ofc/process.h"

#include "ofc/heap.h"
#include "ofc/persist.h"

#if defined(OFC_MESSAGE_DEBUG)
static struct _BLUE_MESSAGE *BlueMessageAlloc ;
BLUE_LOCK BlueMessageDebugLock ;

BLUE_CORE_LIB BLUE_VOID 
BlueMessageDebugInit (BLUE_VOID)
{
  BlueMessageAlloc = BLUE_NULL ;
  BlueMessageDebugLock = BlueLockInit () ;
}

BLUE_CORE_LIB BLUE_VOID
BlueMessageDebugDestroy (BLUE_VOID)
{
  struct _BLUE_MESSAGE *msg ;

  for (msg = BlueMessageAlloc ;
       msg != BLUE_NULL ;
       msg = BlueMessageAlloc)
    {
      BlueMessageDebugFree (msg);
    }
  BlueLockDestroy(BlueMessageDebugLock);
}
  
BLUE_CORE_LIB BLUE_VOID 
BlueMessageDebugAlloc (struct _BLUE_MESSAGE *msg, BLUE_VOID *ret)
{
  /*
   * Put on the allocation queue
   */
  BlueLock (BlueMessageDebugLock) ;
  msg->dbgnext = BlueMessageAlloc ;
  if (BlueMessageAlloc != BLUE_NULL)
    BlueMessageAlloc->dbgprev = msg ;
  BlueMessageAlloc = msg ;
  msg->dbgprev = BLUE_NULL ;
  BlueUnlock (BlueMessageDebugLock) ;
#if defined(__GNUC__) && defined(BLUE_STACK_TRACE)
  msg->caller1 = __builtin_return_address(1) ;
  msg->caller2 = __builtin_return_address(2) ;
  msg->caller3 = __builtin_return_address(3) ;
  msg->caller4 = __builtin_return_address(4) ;
#else
  msg->caller = ret ;
#endif
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageDebugFree (struct _BLUE_MESSAGE *msg)
{
  /*
   * Pull off the allocation queue
   */
  BlueLock (BlueMessageDebugLock) ;
  if (msg->dbgprev != BLUE_NULL)
    msg->dbgprev->dbgnext = msg->dbgnext ;
  else
    BlueMessageAlloc = msg->dbgnext ;
  if (msg->dbgnext != BLUE_NULL)
    msg->dbgnext->dbgprev = msg->dbgprev ;
  BlueUnlock (BlueMessageDebugLock) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageDebugDump (BLUE_VOID)
{
  struct _BLUE_MESSAGE *msg ;

#if defined(__GNUC__)
  BlueCprintf ("%-10s %-10s %-10s %-10s %-10s %10s\n", 
	       "Address", "Size", "Caller1", "Caller2", "Caller3", "Caller4") ;

  for (msg = BlueMessageAlloc ; msg != BLUE_NULL ; msg = msg->dbgnext)
    {
      BlueCprintf ("%-10p %-10d %-10p %-10p %-10p %-10p\n", 
		   msg, msg->length, msg->caller1, msg->caller2, msg->caller3,
		   msg->caller4) ;
    }
#else
  BlueCprintf ("%-20s %-10s %-20s\n", "Address", "Size", "Caller") ;

  for (msg = BlueMessageAlloc ; msg != BLUE_NULL ; msg = msg->dbgnext)
    {
      BlueCprintf ("%-20p %-10d %-20p\n", msg, msg->length, msg->caller) ;
    }
#endif
  BlueCprintf ("\n") ;
}

#endif

OFC_CORE_LIB BLUE_MESSAGE *
BlueMessageCreate (MSG_ALLOC_TYPE msgType, OFC_SIZET msgDataLength,
                   OFC_VOID *msgData)
{
  BLUE_MESSAGE *msg ;

  msg = BlueHeapMalloc (sizeof (BLUE_MESSAGE)) ;
  if (msg == OFC_NULL)
    BlueProcessCrash ("BlueMessage: Couldn't alloc message\n") ;
  else
    {
      msg->length = msgDataLength ;
      msg->destroy_after_send = OFC_FALSE ;
      msg->offset = 0 ;
      msg->context = OFC_NULL ;
      msg->send_size = msgDataLength ;
      msg->count = msg->send_size ;
      msg->alloc = msgType ;
      msg->endian = MSG_ENDIAN_BIG ;
      msg->FIFO1base = 0 ;
      msg->FIFO1 = 0 ;
      msg->FIFO1size = 0 ;
      msg->FIFO1rem = 0 ;
      msg->base = 0 ;
      if (msgData == OFC_NULL)
	{
	  if (msg->alloc == MSG_ALLOC_HEAP)
	    {
	      msg->msg = BlueHeapMalloc (msg->length) ;
	      if (msg->msg == OFC_NULL)
		{
		  BlueHeapFree (msg) ;
		  msg = OFC_NULL ;
		}
	    }
	  else
	    msg->msg = OFC_NULL ;
	}
      else
	msg->msg = msgData ;
#if defined(OFC_MESSAGE_DEBUG)
      if (msg != BLUE_NULL)
	BlueMessageDebugAlloc (msg, RETURN_ADDRESS()) ;
#endif
    }
  return (msg) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessageRealloc (BLUE_MESSAGE *msg, OFC_SIZET msgDataLength)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (msg->alloc == MSG_ALLOC_HEAP)
    {
      msg->length = msgDataLength ;
      msg->msg = BlueHeapRealloc (msg->msg, msg->length) ;
      if (msg->msg != OFC_NULL)
	ret = OFC_TRUE ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueMessageSetAddr (BLUE_MESSAGE *msg, BLUE_IPADDR *ip, OFC_UINT16 port)
{
  if (ip != OFC_NULL)
    BlueCmemcpy (&msg->ip, ip, sizeof (BLUE_IPADDR)) ;
  msg->port = port ;
}

OFC_CORE_LIB BLUE_MESSAGE *
BlueDatagramCreate (MSG_ALLOC_TYPE msgType, OFC_SIZET msgDataLength,
                    OFC_CHAR *msgData, BLUE_IPADDR *ip, OFC_UINT16 port)
{
  BLUE_MESSAGE *msg ;

  msg = BlueMessageCreate (msgType, msgDataLength, msgData) ;
  if (msg != OFC_NULL)
    BlueMessageSetAddr (msg, ip, port) ;

  return (msg) ;
}

OFC_CORE_LIB OFC_VOID
BlueMessageDestroy (BLUE_MESSAGE *msg)
{
  if (msg->msg != OFC_NULL)
    {
      if (msg->alloc == MSG_ALLOC_HEAP)
	BlueHeapFree (msg->msg) ;
#if defined(OFC_MESSAGE_DEBUG)
      BlueMessageDebugFree (msg) ;
#endif
    }
  BlueHeapFree (msg) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessageDone (BLUE_MESSAGE *msg)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (msg->count == 0)
    ret = OFC_TRUE ;
  if (msg->count < 0)
    {
      BlueHeapDumpChunk (msg) ;
      BlueProcessCrash ("Message has negative count\n") ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID *
BlueMessageData (BLUE_MESSAGE *msg)
{
  return (msg->msg) ;
}

OFC_CORE_LIB OFC_VOID *
BlueMessageUnloadData (BLUE_MESSAGE *msg)
{
  OFC_VOID *data ;

  data = msg->msg ;
  msg->msg = OFC_NULL ;

  return (data) ;
}

OFC_CORE_LIB OFC_INT
BlueMessageOffset (BLUE_MESSAGE *msg) 
{
  return (msg->offset) ;
}

OFC_CORE_LIB OFC_VOID
BlueMessageSetSendSize (BLUE_MESSAGE *msg, OFC_SIZET size)
{
  msg->send_size = size ;
  msg->count = msg->send_size ;
}

OFC_CORE_LIB OFC_VOID
BlueMessageAddr (BLUE_MESSAGE *msg, BLUE_IPADDR *ip, OFC_UINT16 *port)
{
  if (ip != OFC_NULL)
    BlueCmemcpy (ip, &msg->ip, sizeof (BLUE_IPADDR)) ;
  if (port != OFC_NULL)
    *port = msg->port ;
}

OFC_CORE_LIB OFC_VOID
BlueMessageReset (BLUE_MESSAGE *msg)
{
  msg->count = msg->send_size ;
  msg->offset = 0 ;
  msg->FIFO1 = msg->FIFO1base ;
  msg->FIFO1rem = msg->FIFO1size ;
  msg->endian = MSG_ENDIAN_BIG ;
  msg->base = 0 ;
}

OFC_CORE_LIB OFC_VOID
BlueMessageSetBase (BLUE_MESSAGE *msg, OFC_INT base)
{
  msg->base = base ;
}

OFC_CORE_LIB OFC_INT
BlueMessageGetBase (BLUE_MESSAGE *msg)
{
  return (msg->base) ;
}

OFC_CORE_LIB OFC_SIZET
BlueMessageGetLength (BLUE_MESSAGE *msg)
{
  return (msg->length) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessagePutU8 (BLUE_MESSAGE *msg, OFC_INT offset, OFC_UINT8 value)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  offset += msg->base ;
  if (offset + SIZEOF_U8 <= msg->length)
    {
      ret = OFC_TRUE ;
      if (msg->endian == MSG_ENDIAN_BIG)
	{
	  BLUE_NET_CTON (msg->msg, offset, value) ;
	}
      else
	{
	  BLUE_NET_CTOSMB (msg->msg, offset, value) ;
	}
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessagePutU16 (BLUE_MESSAGE *msg, OFC_INT offset, OFC_UINT16 value)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  offset += msg->base ;
  if (offset + SIZEOF_U16 <= msg->length)
    {
      ret = OFC_TRUE ;
      if (msg->endian == MSG_ENDIAN_BIG)
	{
	  BLUE_NET_STON (msg->msg, offset, value) ;
	}
      else
	{
	  BLUE_NET_STOSMB (msg->msg, offset, value) ;
	}
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessagePutU32 (BLUE_MESSAGE *msg, OFC_INT offset, OFC_UINT32 value)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  offset += msg->base ;
  if (offset + SIZEOF_U32 <= msg->length)
    {
      ret = OFC_TRUE ;
      if (msg->endian == MSG_ENDIAN_BIG)
	{
	  BLUE_NET_LTON (msg->msg, offset, value) ;
	}
      else
	{
	  BLUE_NET_LTOSMB (msg->msg, offset, value) ;
	}
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessagePutU64 (BLUE_MESSAGE *msg, OFC_INT offset, OFC_UINT64 *value)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  offset += msg->base ;
  if (offset + SIZEOF_U64 <= msg->length)
    {
      ret = OFC_TRUE ;
#if defined (OFC_64BIT_INTEGER)
      if (msg->endian == MSG_ENDIAN_BIG)
	{
	  BLUE_NET_LLTON (msg->msg, offset, *value) ;
	}
      else
	{
	  BLUE_NET_LLTOSMB (msg->msg, offset, *value) ;
	}
#else
      if (msg->endian == MSG_ENDIAN_BIG)
	{
	  BLUE_NET_LTON (msg->msg, offset, value->low) ;
	  BLUE_NET_LTON (msg->msg, offset+4, value->high) ;
	}
      else
	{
	  BLUE_NET_LTOSMB (msg->msg, offset+4, value->low) ;
	  BLUE_NET_LTOSMB (msg->msg, offset, value->high) ;
	}
#endif
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessagePutTStr (BLUE_MESSAGE *msg, OFC_INT offset, OFC_LPCTSTR str)
{
  OFC_BOOL ret ;
  OFC_SIZET len ;
  OFC_CTCHAR *p ;
  OFC_INT i ;

  ret = OFC_TRUE ;
  if (str != OFC_NULL)
    {
      len = (BlueCtstrlen (str) + 1) * sizeof (OFC_WORD) ;

      p = str ;
      for (i = 0, p = str ; i < len && ret == OFC_TRUE ;
	   i+=sizeof(OFC_WORD), p++)
	{
#if defined(OFC_UNICODE)
	  ret = BlueMessagePutU16 (msg, offset + i, *p) ;
#else
	  ret = BlueMessagePutU8 (msg, offset + i, *p) ;
#endif
	}
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_SIZET
BlueMessageGetTStringLen (BLUE_MESSAGE *msg, OFC_INT offset,
                          OFC_SIZET max_len)
{
  OFC_INT i ;
  OFC_TCHAR c ;
  OFC_BOOL done ;

  /*
   * Include the EOS in the string len
   */
  done = OFC_FALSE ;
  /*
   * This loop counts the EOS if the string is less then max_len
   * which is what we want
   */
  for (i = 0 ; i < max_len && !done ; i++ )
    {
      c = BlueMessageGetU16 (msg, offset + (i * sizeof (OFC_WORD))) ;
      if (c == TCHAR_EOS)
	done = OFC_TRUE ;
    }
  return (i) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessageGetTStr (BLUE_MESSAGE *msg, OFC_INT offset, OFC_LPTSTR *str)
{
  OFC_BOOL ret ;
  OFC_TCHAR *p ;
  OFC_INT i ;
  OFC_SIZET string_len ;

  ret = OFC_TRUE ;


  string_len = BlueMessageGetTStringLen (msg, offset, msg->length - offset) ;

  *str = BlueHeapMalloc (string_len * sizeof (OFC_TCHAR)) ;

  for (i = 0, p = *str ; i < string_len ; i++, p++)
    {
      *p = BlueMessageGetU16 (msg, offset + (i * sizeof (OFC_WORD))) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessagePutTStrn (BLUE_MESSAGE *msg, OFC_INT offset, OFC_LPCTSTR str,
                     OFC_SIZET len)
{
  OFC_BOOL ret ;
  OFC_CTCHAR *p ;
  OFC_INT i ;

  len = BLUE_C_MIN (len, BlueCtstrnlen (str, len)) * sizeof (OFC_WORD) ;

  ret = OFC_TRUE ;
  p = str ;
  for (i = 0, p = str ; i < len && ret == OFC_TRUE ;
       i+=sizeof(OFC_WORD), p++)
    {
      ret = BlueMessagePutU16 (msg, offset + i, *p) ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessageGetTStrn (BLUE_MESSAGE *msg, OFC_INT offset, OFC_LPTSTR *str,
                     OFC_SIZET max_len)
{
  OFC_BOOL ret ;
  OFC_TCHAR *p ;
  OFC_INT i ;
  OFC_SIZET string_len ;

  ret = OFC_TRUE ;

  string_len = BlueMessageGetTStringLen (msg, offset, max_len) ;
  *str = BlueHeapMalloc ((string_len+1) * sizeof (OFC_TCHAR)) ;

  for (i = 0, p = *str ; i < string_len ; i++, p++)
    {
      *p = BlueMessageGetU16 (msg, offset + (i * sizeof (OFC_WORD))) ;
    }
  *p = TCHAR_EOS ;

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessageGetTStrnx (BLUE_MESSAGE *msg, OFC_INT offset,
                      OFC_LPTSTR str, OFC_SIZET max_len)
{
  OFC_BOOL ret ;
  OFC_TCHAR *p ;
  OFC_INT i ;
  OFC_SIZET string_len ;

  ret = OFC_TRUE ;

  string_len = BlueMessageGetTStringLen (msg, offset, max_len) ;

  for (i = 0, p = str ; i < string_len ; i++, p++)
    {
      *p = BlueMessageGetU16 (msg, offset + (i * sizeof (OFC_WORD))) ;
    }

  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessagePutCStr (BLUE_MESSAGE *msg, OFC_INT offset, OFC_LPCSTR str)
{
  OFC_BOOL ret ;
  OFC_SIZET len ;
  OFC_CCHAR *p ;
  OFC_INT i ;

  ret = OFC_TRUE ;
  if (str != OFC_NULL)
    {
      len = BlueCstrlen (str) + 1 ;

      p = str ;

      for (i = 0, p = str ; i < len && ret == OFC_TRUE ; i++, p++)
	{
	  ret = BlueMessagePutU8 (msg, offset + i, *p) ;
	}
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_UINT8
BlueMessageGetU8 (BLUE_MESSAGE *msg, OFC_INT offset)
{
  OFC_UINT8 value ;

  value = 0 ;
  offset += msg->base ;
  if (offset + SIZEOF_U8 <= msg->length)
    {
      if (msg->endian == MSG_ENDIAN_BIG)
	value = BLUE_NET_NTOC (msg->msg, offset) ;
      else
	value = BLUE_NET_SMBTOC (msg->msg, offset) ;
    }
  return (value) ;
}

OFC_CORE_LIB OFC_UINT16
BlueMessageGetU16 (BLUE_MESSAGE *msg, OFC_INT offset)
{
  OFC_UINT16 value ;

  value = 0 ;
  offset += msg->base ;
  if (offset + SIZEOF_U16 <= msg->length)
    {
      if (msg->endian == MSG_ENDIAN_BIG)
	value = BLUE_NET_NTOS (msg->msg, offset) ;
      else
	value = BLUE_NET_SMBTOS (msg->msg, offset) ;
    }
  return (value) ;
}

OFC_CORE_LIB OFC_UINT32
BlueMessageGetU32 (BLUE_MESSAGE *msg, OFC_INT offset)
{
  OFC_UINT32 value ;

  value = 0 ;
  offset += msg->base ;
  if (offset + SIZEOF_U32 <= msg->length)
    {
      if (msg->endian == MSG_ENDIAN_BIG)
	value = BLUE_NET_NTOL (msg->msg, offset) ;
      else
	value = BLUE_NET_SMBTOL (msg->msg, offset) ;
    }
  return (value) ;
}

OFC_CORE_LIB OFC_VOID
BlueMessageGetU64 (BLUE_MESSAGE *msg, OFC_INT offset, OFC_UINT64 *value)
{
#if defined(OFC_64BIT_INTEGER)
  *value = 0 ;
#else
  value->low = 0 ;
  value->high = 0 ;
#endif
  offset += msg->base ;
  if (offset + SIZEOF_U64 <= msg->length)
    {
#if defined (OFC_64BIT_INTEGER)
      if (msg->endian == MSG_ENDIAN_BIG)
	*value = BLUE_NET_NTOLL (msg->msg, offset) ;
      else
	*value = BLUE_NET_SMBTOLL (msg->msg, offset) ;
#else
      if (msg->endian == MSG_ENDIAN_BIG)
	{
	  value->low = BLUE_NET_NTOL (msg->msg, offset) ;
	  value->high = BLUE_NET_NTOL (msg->msg, offset+4) ;
	}
      else
	{
	  value->low = BLUE_NET_SMBTOL (msg->msg, offset+4) ;
	  value->high = BLUE_NET_SMBTOL (msg->msg, offset) ;
	}
#endif
    }
}

OFC_CORE_LIB OFC_VOID *
BlueMessageGetPointer (BLUE_MESSAGE *msg, OFC_INT offset)
{
  offset += msg->base ;
  return (msg->msg + offset) ;
}

OFC_CORE_LIB OFC_VOID
BlueMessageFIFO1Set (BLUE_MESSAGE *msg, OFC_INT offset, OFC_SIZET size)
{
  msg->FIFO1base = offset ;
  msg->FIFO1 = offset ;
  msg->FIFO1size = (OFC_INT) size ;
  msg->FIFO1rem = size ;
}

OFC_CORE_LIB OFC_VOID
BlueMessageFIFO1Align (BLUE_MESSAGE *msg, OFC_INT align)
{
  OFC_INT mask ;
  OFC_INT pad ;

  mask = align - 1 ;
  if (msg->FIFO1 & mask)
    {
      pad = align - (msg->FIFO1 & mask) ;
      msg->FIFO1 += pad ;
      msg->FIFO1rem -= pad ;
    }
}

OFC_CORE_LIB OFC_VOID *
BlueMessageFIFO1Push (BLUE_MESSAGE *msg, OFC_SIZET size)
{
  OFC_VOID *ret ;

  ret = OFC_NULL ;
  if (size <= msg->FIFO1rem)
    {
      ret = BlueMessageGetPointer (msg, msg->FIFO1) ;
      msg->FIFO1 += size ;
      msg->FIFO1rem -= size ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID *
BlueMessageFIFO1Pop (BLUE_MESSAGE *msg, OFC_SIZET size)
{
  OFC_VOID *ret ;

  ret = OFC_NULL ;
  if (size <= msg->FIFO1rem)
    {
      ret = BlueMessageGetPointer (msg, msg->FIFO1) ;
      msg->FIFO1 += size ;
      msg->FIFO1rem -= size ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_UINT8
BlueMessageFIFO1PopU8 (BLUE_MESSAGE *msg)
{
  OFC_UINT8 ret ;

  ret = 0 ;
  if (SIZEOF_U8 <= msg->FIFO1rem)
    {
      ret = BlueMessageGetU8 (msg, msg->FIFO1) ;
      msg->FIFO1 += SIZEOF_U8 ;
      msg->FIFO1rem -= SIZEOF_U8 ;
    }
  return (ret) ;
}  

OFC_CORE_LIB OFC_UINT16
BlueMessageFIFO1PopU16 (BLUE_MESSAGE *msg)
{
  OFC_UINT16 ret ;

  ret = 0 ;
  if (SIZEOF_U16 <= msg->FIFO1rem)
    {
      ret = BlueMessageGetU16 (msg, msg->FIFO1) ;
      msg->FIFO1 += SIZEOF_U16 ;
      msg->FIFO1rem -= SIZEOF_U16 ;
    }
  return (ret) ;
}  

OFC_CORE_LIB OFC_UINT32
BlueMessageFIFO1PopU32 (BLUE_MESSAGE *msg)
{
  OFC_UINT32 ret ;

  ret = 0 ;
  if (SIZEOF_U32 <= msg->FIFO1rem)
    {
      ret = BlueMessageGetU32 (msg, msg->FIFO1) ;
      msg->FIFO1 += SIZEOF_U32 ;
      msg->FIFO1rem -= SIZEOF_U32 ;
    }
  return (ret) ;
}  

OFC_CORE_LIB OFC_VOID
BlueMessageFIFO1PopU64 (BLUE_MESSAGE *msg, OFC_UINT64 *val)
{
#if defined(OFC_64BIT_INTEGER)
  *val = 0 ;
#else
  val->low = 0 ;
  val->high = 0 ;
#endif

  if (SIZEOF_U64 <= msg->FIFO1rem)
    {
      BlueMessageGetU64 (msg, msg->FIFO1, val) ;
      msg->FIFO1 += SIZEOF_U64 ;
      msg->FIFO1rem -= SIZEOF_U64 ;
    }
}  

OFC_CORE_LIB OFC_CHAR *
BlueMessageFIFO1PopCString (BLUE_MESSAGE *msg)
{
  OFC_CHAR *ret ;
  OFC_SIZET len ;
  
  ret = BlueMessageGetPointer (msg, msg->FIFO1) ;

  if (ret != OFC_NULL)
    {
      len = BlueCstrlen (ret) + 1 ;
      if (len <= msg->FIFO1rem)
	{
	  msg->FIFO1 += len ;
	  msg->FIFO1rem -= len ;
	}
      else
	ret = OFC_NULL ;
    }
  return (ret) ;
}
	  
OFC_CORE_LIB OFC_TCHAR *
BlueMessageFIFO1PopTStr (BLUE_MESSAGE *msg)
{
  OFC_LPTSTR ret ;
  OFC_SIZET string_len ;
  
  ret = OFC_NULL ;

  string_len = 
    BlueMessageGetTStringLen (msg, BlueMessageFIFO1Get (msg),
			      (OFC_SIZET)
			      (msg->FIFO1rem / sizeof (OFC_WORD))) ;

  ret = BlueHeapMalloc (string_len * sizeof (OFC_TCHAR)) ;

  if (ret != OFC_NULL)
    {
      BlueMessageGetTStrnx (msg, BlueMessageFIFO1Get(msg), ret, 
			    string_len) ;

      ret[string_len-1] = TCHAR_EOS ;

      msg->FIFO1 += string_len * sizeof (OFC_WORD) ;
      msg->FIFO1rem -= string_len * sizeof (OFC_WORD) ;
    }

  return (ret) ;
}
	  
OFC_CORE_LIB OFC_LPTSTR
BlueMessageFIFO1PopTStrn (BLUE_MESSAGE *msg, OFC_SIZET max_len)
{
  OFC_LPTSTR ret ;
  OFC_SIZET string_len ;
  
  ret = OFC_NULL ;
  string_len = BlueMessageGetTStringLen (msg, BlueMessageFIFO1Get (msg),
					 max_len) ;
  string_len = 
    BLUE_C_MIN (string_len,
		(OFC_SIZET) (msg->FIFO1rem / sizeof (OFC_WORD))) ;

  BlueMessageGetTStrn (msg, BlueMessageFIFO1Get(msg), &ret, string_len) ;

  msg->FIFO1 += string_len * sizeof (OFC_WORD) ;
  msg->FIFO1rem -= string_len * sizeof (OFC_WORD) ;

  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueMessageFIFO1PopTStrnx (BLUE_MESSAGE *msg, OFC_LPTSTR str,
                           OFC_SIZET max_len)
{
  OFC_SIZET string_len ;
  
  BlueMessageFIFO1Align (msg, 2) ;

  string_len = BlueMessageGetTStringLen (msg, BlueMessageFIFO1Get (msg),
					 max_len) ;
  string_len = 
    BLUE_C_MIN (string_len,
		(OFC_SIZET) (msg->FIFO1rem / sizeof (OFC_WORD))) ;

  BlueMessageGetTStrnx (msg, BlueMessageFIFO1Get(msg), str, string_len) ;

  msg->FIFO1 += string_len * sizeof (OFC_WORD) ;
  msg->FIFO1rem -= string_len * sizeof (OFC_WORD) ;
}

OFC_CORE_LIB OFC_LPCSTR
BlueMessageFIFO1PopCStrn (BLUE_MESSAGE *msg, OFC_SIZET len)
{
  OFC_LPSTR ret ;
  OFC_LPSTR p ;
  int i ;
  
  ret = (OFC_LPSTR) BlueMessageGetPointer (msg, msg->FIFO1) ;

  if (ret != OFC_NULL)
    {
      for (i = 0, p = ret ; i < len && *p != '\0' ; i++, p++) ;
      /*
       * If we still have room for the eos, let's add one
       */
      if (i < len)
	i++ ;

      len = i ;
      if (len <= msg->FIFO1rem)
	{
	  msg->FIFO1 += len ;
	  msg->FIFO1rem -= len ;
	}
      else
	ret = OFC_NULL ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessageFIFO1PushU8 (BLUE_MESSAGE *msg, OFC_UINT8 value)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (SIZEOF_U8 <= msg->FIFO1rem)
    {
      ret = BlueMessagePutU8 (msg, msg->FIFO1, value) ;
      if (ret == OFC_TRUE)
	{
	  msg->FIFO1 += SIZEOF_U8 ;
	  msg->FIFO1rem -= SIZEOF_U8 ;
	}
    }
  return (ret) ;
}
    
OFC_CORE_LIB OFC_BOOL
BlueMessageFIFO1PushU16 (BLUE_MESSAGE *msg, OFC_UINT16 value)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (SIZEOF_U16 <= msg->FIFO1rem)
    {
      ret = BlueMessagePutU16 (msg, msg->FIFO1, value) ;
      if (ret == OFC_TRUE)
	{
	  msg->FIFO1 += SIZEOF_U16 ;
	  msg->FIFO1rem -= SIZEOF_U16 ;
	}
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessageFIFO1PushU32 (BLUE_MESSAGE *msg, OFC_UINT32 value)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (SIZEOF_U32 <= msg->FIFO1rem)
    {
      ret = BlueMessagePutU32 (msg, msg->FIFO1, value) ;
      if (ret == OFC_TRUE)
	{
	  msg->FIFO1 += SIZEOF_U32 ;
	  msg->FIFO1rem -= SIZEOF_U32 ;
	}
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessageFIFO1PushU64 (BLUE_MESSAGE *msg, OFC_UINT64 *value)
{
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (SIZEOF_U64 <= msg->FIFO1rem)
    {
      ret = BlueMessagePutU64 (msg, msg->FIFO1, value) ;
      if (ret == OFC_TRUE)
	{
	  msg->FIFO1 += SIZEOF_U64 ;
	  msg->FIFO1rem -= SIZEOF_U64 ;
	}
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessageFIFO1PushTStr (BLUE_MESSAGE *msg, OFC_LPCTSTR str)
{
  OFC_BOOL ret ;
  OFC_SIZET len ;

  ret = OFC_FALSE ;

  len = (BlueCtstrlen (str) + 1) * sizeof (OFC_WORD) ;
  if (len <= msg->FIFO1rem)
    {
      ret = BlueMessagePutTStr (msg, msg->FIFO1, str) ;
      if (ret == OFC_TRUE)
	{
	  msg->FIFO1 += len ;
	  msg->FIFO1rem -= len ;
	}
    }
  return (ret) ;
}
  
OFC_CORE_LIB OFC_BOOL
BlueMessageFIFO1PushTStrn (BLUE_MESSAGE *msg, OFC_LPCTSTR str,
                           OFC_SIZET len)
{
  OFC_BOOL ret ;
  OFC_SIZET tlen ;

  ret = OFC_FALSE ;

  len = BLUE_C_MIN (len, BlueCtstrnlen (str, len)) ;
  tlen = len * sizeof(OFC_WORD) ;

  if (tlen <= msg->FIFO1rem)
    {
      ret = BlueMessagePutTStrn (msg, msg->FIFO1, str, len) ;
      if (ret == OFC_TRUE)
	{
	  msg->FIFO1 += tlen ;
	  msg->FIFO1rem -= tlen ;
	}
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessageFIFO1PushCStr (BLUE_MESSAGE *msg, OFC_LPCSTR str)
{
  OFC_BOOL ret ;
  OFC_SIZET len ;

  ret = OFC_FALSE ;

  len = BlueCstrlen (str) + 1 ;
  if (len <= msg->FIFO1rem)
    {
      ret = BlueMessagePutCStr (msg, msg->FIFO1, str) ;
      if (ret == OFC_TRUE)
	{
	  msg->FIFO1 += len ;
	  msg->FIFO1rem -= len ;
	}
    }
  return (ret) ;
}
  
OFC_CORE_LIB OFC_INT
BlueMessageFIFO1Get (BLUE_MESSAGE *msg)
{
  OFC_INT ret ;

  ret = msg->FIFO1 ;
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueMessageFIFO1Update (BLUE_MESSAGE *msg, OFC_INT offset)
{
  OFC_INT distance ;

  if (offset < msg->FIFO1size)
    {
      distance = offset - msg->FIFO1 ;
      msg->FIFO1 += distance ;
      msg->FIFO1rem -= distance ;
    }
}

OFC_CORE_LIB OFC_INT
BlueMessageFIFO1Rem (BLUE_MESSAGE *msg)
{
  OFC_INT rem ;

  rem = (OFC_INT) msg->FIFO1rem ;
  return (rem) ;
}

OFC_CORE_LIB OFC_INT
BlueMessageFIFO1Size (BLUE_MESSAGE *msg)
{
  OFC_INT size ;

  size = msg->FIFO1size ;
  return (size) ;
}

OFC_CORE_LIB OFC_INT
BlueMessageFIFO1Len (BLUE_MESSAGE *msg)
{
  OFC_INT len ;

  len = msg->FIFO1 - msg->FIFO1base ;
  return (len) ;
}

OFC_CORE_LIB OFC_INT
BlueMessageFIFO1Base (BLUE_MESSAGE *msg)
{
  OFC_INT base ;

  base = msg->FIFO1base ;
  return (base) ;
}

OFC_CORE_LIB OFC_CHAR *
BlueMessageFIFO1BasePointer (BLUE_MESSAGE *msg)
{
  OFC_CHAR *ret ;

  ret = BlueMessageGetPointer (msg, msg->FIFO1base) ;
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessageFIFO1Valid (BLUE_MESSAGE *msg)
{
  return (msg->FIFO1rem >= 0) ;
}

OFC_CORE_LIB OFC_VOID
BlueMessageSetEndian (BLUE_MESSAGE *msg, MSG_ENDIAN endianess)
{
  msg->endian = endianess ;
}
    

OFC_CORE_LIB OFC_VOID
BlueMessageParamSet (BLUE_MESSAGE *msg, OFC_INT offset, OFC_SIZET len)
{
  msg->param_offset = offset ;
  msg->param_len = len ;
}

OFC_CORE_LIB OFC_INT
BlueMessageParamGetOffset (BLUE_MESSAGE *msg)
{
  return (msg->param_offset) ;
}

OFC_CORE_LIB OFC_INT
BlueMessageParamGetLen (BLUE_MESSAGE *msg)
{
  return ((OFC_INT) msg->param_len) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessageParamGetTStr (BLUE_MESSAGE *msg, OFC_INT offset, OFC_LPTSTR *str)
{
  return (BlueMessageGetTStr (msg, msg->param_offset+ offset, str)) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessageParamPutTStr (BLUE_MESSAGE *msg, OFC_INT offset, OFC_LPTSTR str)
{
  return (BlueMessagePutTStr (msg, msg->param_offset+ offset, str)) ;
}

OFC_CORE_LIB OFC_BOOL
BlueMessageParamPutTStrn (BLUE_MESSAGE *msg, OFC_INT offset,
                          OFC_LPCTSTR str, OFC_SIZET len)
{
  return (BlueMessagePutTStrn (msg, msg->param_offset+ offset, str, len)) ;
}

OFC_CORE_LIB OFC_VOID *
BlueMessageParamGetPointer (BLUE_MESSAGE *msg, OFC_INT offset)
{
  return (BlueMessageGetPointer (msg, msg->param_offset + offset)) ;
}

OFC_CORE_LIB OFC_VOID
BlueMessageParamPutU8 (BLUE_MESSAGE *msg, OFC_INT offset, OFC_UINT8 value)
{
  if ((offset + SIZEOF_U8) <= msg->param_len)
    BlueMessagePutU8 (msg, msg->param_offset + offset, value) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueMessageParamPutU16 (BLUE_MESSAGE *msg, OFC_INT offset, OFC_UINT16 value)
{
  if ((offset + SIZEOF_U16) <= msg->param_len)
    BlueMessagePutU16 (msg, msg->param_offset + offset, value) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueMessageParamPutU32 (BLUE_MESSAGE *msg, OFC_INT offset, OFC_UINT32 value)
{
  if ((offset + SIZEOF_U32) <= msg->param_len)
    BlueMessagePutU32 (msg, msg->param_offset + offset, value) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueMessageParamPutU64 (BLUE_MESSAGE *msg, OFC_INT offset, OFC_UINT64 *value)
{
  if ((offset + SIZEOF_U64) <= msg->param_len)
    BlueMessagePutU64 (msg, msg->param_offset + offset, value) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
    }
}

OFC_CORE_LIB OFC_UINT8
BlueMessageParamGetU8 (BLUE_MESSAGE *msg, OFC_INT offset)
{
  OFC_UINT8 ret ;

  if ((offset + SIZEOF_U8) <= msg->param_len)
    ret = BlueMessageGetU8 (msg, msg->param_offset + offset) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
      ret = 0 ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_UINT16
BlueMessageParamGetU16 (BLUE_MESSAGE *msg, OFC_INT offset)
{
  OFC_UINT16 ret ;

  if ((offset + SIZEOF_U16) <= msg->param_len)
    ret = BlueMessageGetU16 (msg, msg->param_offset + offset) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
      ret = 0 ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_UINT32
BlueMessageParamGetU32 (BLUE_MESSAGE *msg, OFC_INT offset)
{
  OFC_UINT32 ret ;

  if ((offset + SIZEOF_U32) <= msg->param_len)
    ret = BlueMessageGetU32 (msg, msg->param_offset + offset) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
      ret = 0 ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueMessageParamGetU64 (BLUE_MESSAGE *msg, OFC_INT offset, OFC_UINT64 *value)
{
  if ((offset + SIZEOF_U64) <= msg->param_len)
    BlueMessageGetU64 (msg, msg->param_offset + offset, value) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
#if defined(OFC_64BIT_INTEGER)
      *value = 0 ;
#else
      value->low = 0 ;
      value->high = 0 ;
#endif
    }
}

OFC_CORE_LIB OFC_BOOL
BlueMessageFromSubnet (BLUE_MESSAGE *msg, OFC_INT iface, BLUE_IPADDR *intip)
{
  BLUE_IPADDR fromip ;
  OFC_UINT16 port ;
  OFC_BOOL ret ;
  BLUE_IPADDR mask ;

  BlueMessageAddr (msg, &fromip, &port) ;

  BlueConfigInterfaceAddr (iface, intip, OFC_NULL, &mask) ;

  ret = BlueNetSubnetMatch (&fromip, intip, &mask) ;

  return (ret) ;
}

OFC_CORE_LIB OFC_VOID
BlueMessageSetContext (BLUE_MESSAGE *msg, OFC_VOID *context)
{
  msg->context = context ;
}

OFC_CORE_LIB OFC_VOID *
BlueMessageGetContext (BLUE_MESSAGE *msg)
{
  return (msg->context) ;
}

OFC_CORE_LIB OFC_BOOL BlueMessageDestroyAfterSend (BLUE_MESSAGE *msg)
{
  OFC_BOOL ret ;
  ret = msg->destroy_after_send ;
  return (ret) ;
}

OFC_CORE_LIB OFC_VOID BlueMessageSetDestroyAfterSend (BLUE_MESSAGE *msg)
{
  msg->destroy_after_send = OFC_TRUE ;
}
