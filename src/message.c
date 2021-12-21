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
#include "ofc/config.h"
#include "ofc/libc.h"
#include "ofc/net.h"
#include "ofc/message.h"
#include "ofc/lock.h"
#include "ofc/thread.h"
#include "ofc/process.h"

#include "ofc/heap.h"
#include "ofc/persist.h"

#if defined(BLUE_PARAM_MESSAGE_DEBUG)
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

BLUE_CORE_LIB BLUE_MESSAGE *
BlueMessageCreate (MSG_ALLOC_TYPE msgType, BLUE_SIZET msgDataLength,
		   BLUE_VOID *msgData) 
{
  BLUE_MESSAGE *msg ;

  msg = BlueHeapMalloc (sizeof (BLUE_MESSAGE)) ;
  if (msg == BLUE_NULL)
    BlueProcessCrash ("BlueMessage: Couldn't alloc message\n") ;
  else
    {
      msg->length = msgDataLength ;
      msg->destroy_after_send = BLUE_FALSE ;
      msg->offset = 0 ;
      msg->context = BLUE_NULL ;
      msg->send_size = msgDataLength ;
      msg->count = msg->send_size ;
      msg->alloc = msgType ;
      msg->endian = MSG_ENDIAN_BIG ;
      msg->FIFO1base = 0 ;
      msg->FIFO1 = 0 ;
      msg->FIFO1size = 0 ;
      msg->FIFO1rem = 0 ;
      msg->base = 0 ;
      if (msgData == BLUE_NULL)
	{
	  if (msg->alloc == MSG_ALLOC_HEAP)
	    {
	      msg->msg = BlueHeapMalloc (msg->length) ;
	      if (msg->msg == BLUE_NULL)
		{
		  BlueHeapFree (msg) ;
		  msg = BLUE_NULL ;
		}
	    }
	  else
	    msg->msg = BLUE_NULL ;
	}
      else
	msg->msg = msgData ;
#if defined(BLUE_PARAM_MESSAGE_DEBUG)
      if (msg != BLUE_NULL)
	BlueMessageDebugAlloc (msg, RETURN_ADDRESS()) ;
#endif
    }
  return (msg) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessageRealloc (BLUE_MESSAGE *msg, BLUE_SIZET msgDataLength)
{
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  if (msg->alloc == MSG_ALLOC_HEAP)
    {
      msg->length = msgDataLength ;
      msg->msg = BlueHeapRealloc (msg->msg, msg->length) ;
      if (msg->msg != BLUE_NULL)
	ret = BLUE_TRUE ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageSetAddr (BLUE_MESSAGE *msg, BLUE_IPADDR *ip, BLUE_UINT16 port)
{
  if (ip != BLUE_NULL)
    BlueCmemcpy (&msg->ip, ip, sizeof (BLUE_IPADDR)) ;
  msg->port = port ;
}

BLUE_CORE_LIB BLUE_MESSAGE *
BlueDatagramCreate (MSG_ALLOC_TYPE msgType, BLUE_SIZET msgDataLength,
		    BLUE_CHAR *msgData, BLUE_IPADDR *ip, BLUE_UINT16 port) 
{
  BLUE_MESSAGE *msg ;

  msg = BlueMessageCreate (msgType, msgDataLength, msgData) ;
  if (msg != BLUE_NULL)
    BlueMessageSetAddr (msg, ip, port) ;

  return (msg) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageDestroy (BLUE_MESSAGE *msg)
{
  if (msg->msg != BLUE_NULL)
    {
      if (msg->alloc == MSG_ALLOC_HEAP)
	BlueHeapFree (msg->msg) ;
#if defined(BLUE_PARAM_MESSAGE_DEBUG)
      BlueMessageDebugFree (msg) ;
#endif
    }
  BlueHeapFree (msg) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessageDone (BLUE_MESSAGE *msg)
{
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  if (msg->count == 0)
    ret = BLUE_TRUE ;
  if (msg->count < 0)
    {
      BlueHeapDumpChunk (msg) ;
      BlueProcessCrash ("Message has negative count\n") ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID *
BlueMessageData (BLUE_MESSAGE *msg)
{
  return (msg->msg) ;
}

BLUE_CORE_LIB BLUE_VOID *
BlueMessageUnloadData (BLUE_MESSAGE *msg)
{
  BLUE_VOID *data ;

  data = msg->msg ;
  msg->msg = BLUE_NULL ;

  return (data) ;
}

BLUE_CORE_LIB BLUE_INT 
BlueMessageOffset (BLUE_MESSAGE *msg) 
{
  return (msg->offset) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageSetSendSize (BLUE_MESSAGE *msg, BLUE_SIZET size)
{
  msg->send_size = size ;
  msg->count = msg->send_size ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageAddr (BLUE_MESSAGE *msg, BLUE_IPADDR *ip, BLUE_UINT16 *port) 
{
  if (ip != BLUE_NULL)
    BlueCmemcpy (ip, &msg->ip, sizeof (BLUE_IPADDR)) ;
  if (port != BLUE_NULL)
    *port = msg->port ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageReset (BLUE_MESSAGE *msg)
{
  msg->count = msg->send_size ;
  msg->offset = 0 ;
  msg->FIFO1 = msg->FIFO1base ;
  msg->FIFO1rem = msg->FIFO1size ;
  msg->endian = MSG_ENDIAN_BIG ;
  msg->base = 0 ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageSetBase (BLUE_MESSAGE *msg, BLUE_INT base)
{
  msg->base = base ;
}

BLUE_CORE_LIB BLUE_INT 
BlueMessageGetBase (BLUE_MESSAGE *msg)
{
  return (msg->base) ;
}

BLUE_CORE_LIB BLUE_SIZET 
BlueMessageGetLength (BLUE_MESSAGE *msg)
{
  return (msg->length) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessagePutU8 (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_UINT8 value) 
{
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  offset += msg->base ;
  if (offset + SIZEOF_U8 <= msg->length)
    {
      ret = BLUE_TRUE ;
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

BLUE_CORE_LIB BLUE_BOOL 
BlueMessagePutU16 (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_UINT16 value) 
{
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  offset += msg->base ;
  if (offset + SIZEOF_U16 <= msg->length)
    {
      ret = BLUE_TRUE ;
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

BLUE_CORE_LIB BLUE_BOOL 
BlueMessagePutU32 (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_UINT32 value) 
{
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  offset += msg->base ;
  if (offset + SIZEOF_U32 <= msg->length)
    {
      ret = BLUE_TRUE ;
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

BLUE_CORE_LIB BLUE_BOOL 
BlueMessagePutU64 (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_UINT64 *value) 
{
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  offset += msg->base ;
  if (offset + SIZEOF_U64 <= msg->length)
    {
      ret = BLUE_TRUE ;
#if defined (BLUE_PARAM_64BIT_INTEGER)
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

BLUE_CORE_LIB BLUE_BOOL 
BlueMessagePutTStr (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_LPCTSTR str)
{
  BLUE_BOOL ret ;
  BLUE_SIZET len ;
  BLUE_CTCHAR *p ;
  BLUE_INT i ;

  ret = BLUE_TRUE ;
  if (str != BLUE_NULL)
    {
      len = (BlueCtstrlen (str) + 1) * sizeof (BLUE_WORD) ;

      p = str ;
      for (i = 0, p = str ; i < len && ret == BLUE_TRUE ; 
	   i+=sizeof(BLUE_WORD), p++)
	{
#if defined(BLUE_PARAM_UNICODE)
	  ret = BlueMessagePutU16 (msg, offset + i, *p) ;
#else
	  ret = BlueMessagePutU8 (msg, offset + i, *p) ;
#endif
	}
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_SIZET 
BlueMessageGetTStringLen (BLUE_MESSAGE *msg, BLUE_INT offset, 
			  BLUE_SIZET max_len)
{
  BLUE_INT i ;
  BLUE_TCHAR c ;
  BLUE_BOOL done ;

  /*
   * Include the EOS in the string len
   */
  done = BLUE_FALSE ;
  /*
   * This loop counts the EOS if the string is less then max_len
   * which is what we want
   */
  for (i = 0 ; i < max_len && !done ; i++ )
    {
      c = BlueMessageGetU16 (msg, offset + (i * sizeof (BLUE_WORD))) ;
      if (c == TCHAR_EOS)
	done = BLUE_TRUE ;
    }
  return (i) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessageGetTStr (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_LPTSTR *str)
{
  BLUE_BOOL ret ;
  BLUE_TCHAR *p ;
  BLUE_INT i ;
  BLUE_SIZET string_len ;

  ret = BLUE_TRUE ;


  string_len = BlueMessageGetTStringLen (msg, offset, msg->length - offset) ;

  *str = BlueHeapMalloc (string_len * sizeof (BLUE_TCHAR)) ;

  for (i = 0, p = *str ; i < string_len ; i++, p++)
    {
      *p = BlueMessageGetU16 (msg, offset + (i * sizeof (BLUE_WORD))) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessagePutTStrn (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_LPCTSTR str,
		     BLUE_SIZET len)
{
  BLUE_BOOL ret ;
  BLUE_CTCHAR *p ;
  BLUE_INT i ;

  len = BLUE_C_MIN (len, BlueCtstrnlen (str, len)) * sizeof (BLUE_WORD) ;

  ret = BLUE_TRUE ;
  p = str ;
  for (i = 0, p = str ; i < len && ret == BLUE_TRUE ; 
       i+=sizeof(BLUE_WORD), p++)
    {
      ret = BlueMessagePutU16 (msg, offset + i, *p) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessageGetTStrn (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_LPTSTR *str,
		     BLUE_SIZET max_len)
{
  BLUE_BOOL ret ;
  BLUE_TCHAR *p ;
  BLUE_INT i ;
  BLUE_SIZET string_len ;

  ret = BLUE_TRUE ;

  string_len = BlueMessageGetTStringLen (msg, offset, max_len) ;
  *str = BlueHeapMalloc ((string_len+1) * sizeof (BLUE_TCHAR)) ;

  for (i = 0, p = *str ; i < string_len ; i++, p++)
    {
      *p = BlueMessageGetU16 (msg, offset + (i * sizeof (BLUE_WORD))) ;
    }
  *p = TCHAR_EOS ;

  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessageGetTStrnx (BLUE_MESSAGE *msg, BLUE_INT offset, 
		      BLUE_LPTSTR str, BLUE_SIZET max_len)
{
  BLUE_BOOL ret ;
  BLUE_TCHAR *p ;
  BLUE_INT i ;
  BLUE_SIZET string_len ;

  ret = BLUE_TRUE ;

  string_len = BlueMessageGetTStringLen (msg, offset, max_len) ;

  for (i = 0, p = str ; i < string_len ; i++, p++)
    {
      *p = BlueMessageGetU16 (msg, offset + (i * sizeof (BLUE_WORD))) ;
    }

  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessagePutCStr (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_LPCSTR str)
{
  BLUE_BOOL ret ;
  BLUE_SIZET len ;
  BLUE_CCHAR *p ;
  BLUE_INT i ;

  ret = BLUE_TRUE ;
  if (str != BLUE_NULL)
    {
      len = BlueCstrlen (str) + 1 ;

      p = str ;

      for (i = 0, p = str ; i < len && ret == BLUE_TRUE ; i++, p++)
	{
	  ret = BlueMessagePutU8 (msg, offset + i, *p) ;
	}
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_UINT8 
BlueMessageGetU8 (BLUE_MESSAGE *msg, BLUE_INT offset)
{
  BLUE_UINT8 value ;

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

BLUE_CORE_LIB BLUE_UINT16 
BlueMessageGetU16 (BLUE_MESSAGE *msg, BLUE_INT offset)
{
  BLUE_UINT16 value ;

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

BLUE_CORE_LIB BLUE_UINT32 
BlueMessageGetU32 (BLUE_MESSAGE *msg, BLUE_INT offset)
{
  BLUE_UINT32 value ;

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

BLUE_CORE_LIB BLUE_VOID 
BlueMessageGetU64 (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_UINT64 *value)
{
#if defined(BLUE_PARAM_64BIT_INTEGER)
  *value = 0 ;
#else
  value->low = 0 ;
  value->high = 0 ;
#endif
  offset += msg->base ;
  if (offset + SIZEOF_U64 <= msg->length)
    {
#if defined (BLUE_PARAM_64BIT_INTEGER)
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

BLUE_CORE_LIB BLUE_VOID *
BlueMessageGetPointer (BLUE_MESSAGE *msg, BLUE_INT offset)
{
  offset += msg->base ;
  return (msg->msg + offset) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageFIFO1Set (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_SIZET size)
{
  msg->FIFO1base = offset ;
  msg->FIFO1 = offset ;
  msg->FIFO1size = (BLUE_INT) size ;
  msg->FIFO1rem = size ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageFIFO1Align (BLUE_MESSAGE *msg, BLUE_INT align) 
{
  BLUE_INT mask ;
  BLUE_INT pad ;

  mask = align - 1 ;
  if (msg->FIFO1 & mask)
    {
      pad = align - (msg->FIFO1 & mask) ;
      msg->FIFO1 += pad ;
      msg->FIFO1rem -= pad ;
    }
}

BLUE_CORE_LIB BLUE_VOID *
BlueMessageFIFO1Push (BLUE_MESSAGE *msg, BLUE_SIZET size) 
{
  BLUE_VOID *ret ;

  ret = BLUE_NULL ;
  if (size <= msg->FIFO1rem)
    {
      ret = BlueMessageGetPointer (msg, msg->FIFO1) ;
      msg->FIFO1 += size ;
      msg->FIFO1rem -= size ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID *
BlueMessageFIFO1Pop (BLUE_MESSAGE *msg, BLUE_SIZET size) 
{
  BLUE_VOID *ret ;

  ret = BLUE_NULL ;
  if (size <= msg->FIFO1rem)
    {
      ret = BlueMessageGetPointer (msg, msg->FIFO1) ;
      msg->FIFO1 += size ;
      msg->FIFO1rem -= size ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_UINT8 
BlueMessageFIFO1PopU8 (BLUE_MESSAGE *msg)
{
  BLUE_UINT8 ret ;

  ret = 0 ;
  if (SIZEOF_U8 <= msg->FIFO1rem)
    {
      ret = BlueMessageGetU8 (msg, msg->FIFO1) ;
      msg->FIFO1 += SIZEOF_U8 ;
      msg->FIFO1rem -= SIZEOF_U8 ;
    }
  return (ret) ;
}  

BLUE_CORE_LIB BLUE_UINT16 
BlueMessageFIFO1PopU16 (BLUE_MESSAGE *msg)
{
  BLUE_UINT16 ret ;

  ret = 0 ;
  if (SIZEOF_U16 <= msg->FIFO1rem)
    {
      ret = BlueMessageGetU16 (msg, msg->FIFO1) ;
      msg->FIFO1 += SIZEOF_U16 ;
      msg->FIFO1rem -= SIZEOF_U16 ;
    }
  return (ret) ;
}  

BLUE_CORE_LIB BLUE_UINT32 
BlueMessageFIFO1PopU32 (BLUE_MESSAGE *msg)
{
  BLUE_UINT32 ret ;

  ret = 0 ;
  if (SIZEOF_U32 <= msg->FIFO1rem)
    {
      ret = BlueMessageGetU32 (msg, msg->FIFO1) ;
      msg->FIFO1 += SIZEOF_U32 ;
      msg->FIFO1rem -= SIZEOF_U32 ;
    }
  return (ret) ;
}  

BLUE_CORE_LIB BLUE_VOID 
BlueMessageFIFO1PopU64 (BLUE_MESSAGE *msg, BLUE_UINT64 *val)
{
#if defined(BLUE_PARAM_64BIT_INTEGER)
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

BLUE_CORE_LIB BLUE_CHAR *
BlueMessageFIFO1PopCString (BLUE_MESSAGE *msg)
{
  BLUE_CHAR *ret ;
  BLUE_SIZET len ;
  
  ret = BlueMessageGetPointer (msg, msg->FIFO1) ;

  if (ret != BLUE_NULL)
    {
      len = BlueCstrlen (ret) + 1 ;
      if (len <= msg->FIFO1rem)
	{
	  msg->FIFO1 += len ;
	  msg->FIFO1rem -= len ;
	}
      else
	ret = BLUE_NULL ;
    }
  return (ret) ;
}
	  
BLUE_CORE_LIB BLUE_TCHAR *
BlueMessageFIFO1PopTStr (BLUE_MESSAGE *msg)
{
  BLUE_LPTSTR ret ;
  BLUE_SIZET string_len ;
  
  ret = BLUE_NULL ;

  string_len = 
    BlueMessageGetTStringLen (msg, BlueMessageFIFO1Get (msg),
			      (BLUE_SIZET) 
			      (msg->FIFO1rem / sizeof (BLUE_WORD))) ;

  ret = BlueHeapMalloc (string_len * sizeof (BLUE_TCHAR)) ;

  if (ret != BLUE_NULL)
    {
      BlueMessageGetTStrnx (msg, BlueMessageFIFO1Get(msg), ret, 
			    string_len) ;

      ret[string_len-1] = TCHAR_EOS ;

      msg->FIFO1 += string_len * sizeof (BLUE_WORD) ;
      msg->FIFO1rem -= string_len * sizeof (BLUE_WORD) ;
    }

  return (ret) ;
}
	  
BLUE_CORE_LIB BLUE_LPTSTR 
BlueMessageFIFO1PopTStrn (BLUE_MESSAGE *msg, BLUE_SIZET max_len)
{
  BLUE_LPTSTR ret ;
  BLUE_SIZET string_len ;
  
  ret = BLUE_NULL ;
  string_len = BlueMessageGetTStringLen (msg, BlueMessageFIFO1Get (msg),
					 max_len) ;
  string_len = 
    BLUE_C_MIN (string_len,
		(BLUE_SIZET) (msg->FIFO1rem / sizeof (BLUE_WORD))) ;

  BlueMessageGetTStrn (msg, BlueMessageFIFO1Get(msg), &ret, string_len) ;

  msg->FIFO1 += string_len * sizeof (BLUE_WORD) ;
  msg->FIFO1rem -= string_len * sizeof (BLUE_WORD) ;

  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageFIFO1PopTStrnx (BLUE_MESSAGE *msg, BLUE_LPTSTR str, 
			   BLUE_SIZET max_len)
{
  BLUE_SIZET string_len ;
  
  BlueMessageFIFO1Align (msg, 2) ;

  string_len = BlueMessageGetTStringLen (msg, BlueMessageFIFO1Get (msg),
					 max_len) ;
  string_len = 
    BLUE_C_MIN (string_len,
		(BLUE_SIZET) (msg->FIFO1rem / sizeof (BLUE_WORD))) ;

  BlueMessageGetTStrnx (msg, BlueMessageFIFO1Get(msg), str, string_len) ;

  msg->FIFO1 += string_len * sizeof (BLUE_WORD) ;
  msg->FIFO1rem -= string_len * sizeof (BLUE_WORD) ;
}

BLUE_CORE_LIB BLUE_LPCSTR 
BlueMessageFIFO1PopCStrn (BLUE_MESSAGE *msg, BLUE_SIZET len)
{
  BLUE_LPSTR ret ;
  BLUE_LPSTR p ;
  int i ;
  
  ret = (BLUE_LPSTR) BlueMessageGetPointer (msg, msg->FIFO1) ;

  if (ret != BLUE_NULL)
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
	ret = BLUE_NULL ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessageFIFO1PushU8 (BLUE_MESSAGE *msg, BLUE_UINT8 value) 
{
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  if (SIZEOF_U8 <= msg->FIFO1rem)
    {
      ret = BlueMessagePutU8 (msg, msg->FIFO1, value) ;
      if (ret == BLUE_TRUE)
	{
	  msg->FIFO1 += SIZEOF_U8 ;
	  msg->FIFO1rem -= SIZEOF_U8 ;
	}
    }
  return (ret) ;
}
    
BLUE_CORE_LIB BLUE_BOOL 
BlueMessageFIFO1PushU16 (BLUE_MESSAGE *msg, BLUE_UINT16 value) 
{
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  if (SIZEOF_U16 <= msg->FIFO1rem)
    {
      ret = BlueMessagePutU16 (msg, msg->FIFO1, value) ;
      if (ret == BLUE_TRUE)
	{
	  msg->FIFO1 += SIZEOF_U16 ;
	  msg->FIFO1rem -= SIZEOF_U16 ;
	}
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessageFIFO1PushU32 (BLUE_MESSAGE *msg, BLUE_UINT32 value) 
{
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  if (SIZEOF_U32 <= msg->FIFO1rem)
    {
      ret = BlueMessagePutU32 (msg, msg->FIFO1, value) ;
      if (ret == BLUE_TRUE)
	{
	  msg->FIFO1 += SIZEOF_U32 ;
	  msg->FIFO1rem -= SIZEOF_U32 ;
	}
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessageFIFO1PushU64 (BLUE_MESSAGE *msg, BLUE_UINT64 *value) 
{
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  if (SIZEOF_U64 <= msg->FIFO1rem)
    {
      ret = BlueMessagePutU64 (msg, msg->FIFO1, value) ;
      if (ret == BLUE_TRUE)
	{
	  msg->FIFO1 += SIZEOF_U64 ;
	  msg->FIFO1rem -= SIZEOF_U64 ;
	}
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessageFIFO1PushTStr (BLUE_MESSAGE *msg, BLUE_LPCTSTR str)
{
  BLUE_BOOL ret ;
  BLUE_SIZET len ;

  ret = BLUE_FALSE ;

  len = (BlueCtstrlen (str) + 1) * sizeof (BLUE_WORD) ;
  if (len <= msg->FIFO1rem)
    {
      ret = BlueMessagePutTStr (msg, msg->FIFO1, str) ;
      if (ret == BLUE_TRUE)
	{
	  msg->FIFO1 += len ;
	  msg->FIFO1rem -= len ;
	}
    }
  return (ret) ;
}
  
BLUE_CORE_LIB BLUE_BOOL 
BlueMessageFIFO1PushTStrn (BLUE_MESSAGE *msg, BLUE_LPCTSTR str,
			   BLUE_SIZET len) 
{
  BLUE_BOOL ret ;
  BLUE_SIZET tlen ;

  ret = BLUE_FALSE ;

  len = BLUE_C_MIN (len, BlueCtstrnlen (str, len)) ;
  tlen = len * sizeof(BLUE_WORD) ;

  if (tlen <= msg->FIFO1rem)
    {
      ret = BlueMessagePutTStrn (msg, msg->FIFO1, str, len) ;
      if (ret == BLUE_TRUE)
	{
	  msg->FIFO1 += tlen ;
	  msg->FIFO1rem -= tlen ;
	}
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessageFIFO1PushCStr (BLUE_MESSAGE *msg, BLUE_LPCSTR str)
{
  BLUE_BOOL ret ;
  BLUE_SIZET len ;

  ret = BLUE_FALSE ;

  len = BlueCstrlen (str) + 1 ;
  if (len <= msg->FIFO1rem)
    {
      ret = BlueMessagePutCStr (msg, msg->FIFO1, str) ;
      if (ret == BLUE_TRUE)
	{
	  msg->FIFO1 += len ;
	  msg->FIFO1rem -= len ;
	}
    }
  return (ret) ;
}
  
BLUE_CORE_LIB BLUE_INT 
BlueMessageFIFO1Get (BLUE_MESSAGE *msg)
{
  BLUE_INT ret ;

  ret = msg->FIFO1 ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageFIFO1Update (BLUE_MESSAGE *msg, BLUE_INT offset)
{
  BLUE_INT distance ;

  if (offset < msg->FIFO1size)
    {
      distance = offset - msg->FIFO1 ;
      msg->FIFO1 += distance ;
      msg->FIFO1rem -= distance ;
    }
}

BLUE_CORE_LIB BLUE_INT 
BlueMessageFIFO1Rem (BLUE_MESSAGE *msg)
{
  BLUE_INT rem ;

  rem = (BLUE_INT) msg->FIFO1rem ;
  return (rem) ;
}

BLUE_CORE_LIB BLUE_INT 
BlueMessageFIFO1Size (BLUE_MESSAGE *msg)
{
  BLUE_INT size ;

  size = msg->FIFO1size ;
  return (size) ;
}

BLUE_CORE_LIB BLUE_INT 
BlueMessageFIFO1Len (BLUE_MESSAGE *msg)
{
  BLUE_INT len ;

  len = msg->FIFO1 - msg->FIFO1base ;
  return (len) ;
}

BLUE_CORE_LIB BLUE_INT 
BlueMessageFIFO1Base (BLUE_MESSAGE *msg)
{
  BLUE_INT base ;

  base = msg->FIFO1base ;
  return (base) ;
}

BLUE_CORE_LIB BLUE_CHAR *
BlueMessageFIFO1BasePointer (BLUE_MESSAGE *msg)
{
  BLUE_CHAR *ret ;

  ret = BlueMessageGetPointer (msg, msg->FIFO1base) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessageFIFO1Valid (BLUE_MESSAGE *msg)
{
  return (msg->FIFO1rem >= 0) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageSetEndian (BLUE_MESSAGE *msg, MSG_ENDIAN endianess)
{
  msg->endian = endianess ;
}
    

BLUE_CORE_LIB BLUE_VOID 
BlueMessageParamSet (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_SIZET len) 
{
  msg->param_offset = offset ;
  msg->param_len = len ;
}

BLUE_CORE_LIB BLUE_INT 
BlueMessageParamGetOffset (BLUE_MESSAGE *msg)
{
  return (msg->param_offset) ;
}

BLUE_CORE_LIB BLUE_INT 
BlueMessageParamGetLen (BLUE_MESSAGE *msg)
{
  return ((BLUE_INT) msg->param_len) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessageParamGetTStr (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_LPTSTR *str)
{
  return (BlueMessageGetTStr (msg, msg->param_offset+ offset, str)) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessageParamPutTStr (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_LPTSTR str)
{
  return (BlueMessagePutTStr (msg, msg->param_offset+ offset, str)) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessageParamPutTStrn (BLUE_MESSAGE *msg, BLUE_INT offset, 
			  BLUE_LPCTSTR str, BLUE_SIZET len)
{
  return (BlueMessagePutTStrn (msg, msg->param_offset+ offset, str, len)) ;
}

BLUE_CORE_LIB BLUE_VOID *
BlueMessageParamGetPointer (BLUE_MESSAGE *msg, BLUE_INT offset) 
{
  return (BlueMessageGetPointer (msg, msg->param_offset + offset)) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageParamPutU8 (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_UINT8 value)
{
  if ((offset + SIZEOF_U8) <= msg->param_len)
    BlueMessagePutU8 (msg, msg->param_offset + offset, value) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageParamPutU16 (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_UINT16 value)
{
  if ((offset + SIZEOF_U16) <= msg->param_len)
    BlueMessagePutU16 (msg, msg->param_offset + offset, value) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageParamPutU32 (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_UINT32 value)
{
  if ((offset + SIZEOF_U32) <= msg->param_len)
    BlueMessagePutU32 (msg, msg->param_offset + offset, value) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageParamPutU64 (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_UINT64 *value)
{
  if ((offset + SIZEOF_U64) <= msg->param_len)
    BlueMessagePutU64 (msg, msg->param_offset + offset, value) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
    }
}

BLUE_CORE_LIB BLUE_UINT8 
BlueMessageParamGetU8 (BLUE_MESSAGE *msg, BLUE_INT offset)
{
  BLUE_UINT8 ret ;

  if ((offset + SIZEOF_U8) <= msg->param_len)
    ret = BlueMessageGetU8 (msg, msg->param_offset + offset) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
      ret = 0 ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_UINT16 
BlueMessageParamGetU16 (BLUE_MESSAGE *msg, BLUE_INT offset)
{
  BLUE_UINT16 ret ;

  if ((offset + SIZEOF_U16) <= msg->param_len)
    ret = BlueMessageGetU16 (msg, msg->param_offset + offset) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
      ret = 0 ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_UINT32 
BlueMessageParamGetU32 (BLUE_MESSAGE *msg, BLUE_INT offset)
{
  BLUE_UINT32 ret ;

  if ((offset + SIZEOF_U32) <= msg->param_len)
    ret = BlueMessageGetU32 (msg, msg->param_offset + offset) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
      ret = 0 ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueMessageParamGetU64 (BLUE_MESSAGE *msg, BLUE_INT offset, BLUE_UINT64 *value)
{
  if ((offset + SIZEOF_U64) <= msg->param_len)
    BlueMessageGetU64 (msg, msg->param_offset + offset, value) ;
  else
    {
      BlueProcessCrash ("Param offset out of bounds\n") ;
#if defined(BLUE_PARAM_64BIT_INTEGER)
      *value = 0 ;
#else
      value->low = 0 ;
      value->high = 0 ;
#endif
    }
}

BLUE_CORE_LIB BLUE_BOOL 
BlueMessageFromSubnet (BLUE_MESSAGE *msg, BLUE_INT iface, BLUE_IPADDR *intip)
{
  BLUE_IPADDR fromip ;
  BLUE_UINT16 port ;
  BLUE_BOOL ret ;
  BLUE_IPADDR mask ;

  BlueMessageAddr (msg, &fromip, &port) ;

  BlueConfigInterfaceAddr (iface, intip, BLUE_NULL, &mask) ;

  ret = BlueNetSubnetMatch (&fromip, intip, &mask) ;

  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID
BlueMessageSetContext (BLUE_MESSAGE *msg, BLUE_VOID *context)
{
  msg->context = context ;
}

BLUE_CORE_LIB BLUE_VOID *
BlueMessageGetContext (BLUE_MESSAGE *msg)
{
  return (msg->context) ;
}

BLUE_CORE_LIB BLUE_BOOL BlueMessageDestroyAfterSend (BLUE_MESSAGE *msg)
{
  BLUE_BOOL ret ;
  ret = msg->destroy_after_send ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID BlueMessageSetDestroyAfterSend (BLUE_MESSAGE *msg)
{
  msg->destroy_after_send = BLUE_TRUE ;
}
