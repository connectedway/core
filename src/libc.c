/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __BLUE_CORE_DLL__

#include <stdarg.h>
#if defined(__APPLE__)
#include <string.h>
#include <errno.h>
#endif

#include "ofc/core.h"
#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/console.h"
#include "ofc/lock.h"
#include "ofc/socket.h"
#include "ofc/message.h"

#include "ofc/heap.h"

struct trace_t
{
  BLUE_INT BlueCTraceOffset ;
  BLUE_CHAR BlueCTraceBuf[BLUE_PARAM_TRACE_LEN] ;
  BLUE_OFFT BlueCTraceLock ;
} ;

/**
 * \internal
 * Determine if a character is a whitespace
 */
BLUE_CORE_LIB BLUE_INT 
BlueCstrcmp (BLUE_CCHAR *astr, BLUE_CCHAR *bstr)
{
  BLUE_CCHAR *pa ;
  BLUE_CCHAR *pb ;
  BLUE_INT ret ;

  if (astr == BLUE_NULL || bstr == BLUE_NULL)
    {
      if (astr == BLUE_NULL && bstr == BLUE_NULL)
	ret = 0 ;
      else
	ret = -1 ;
    }
  else
    {
      pa = astr ;
      pb = bstr ;

      /*
       * Step through the string until one of them reaches a NIL
       */
      while (*pa == *pb && *pa != '\0' && *pb != '\0')
	{
	  pa++ ;
	  pb++ ;
	}

      /*
       * Then if the character from a at that position is greater then the
       * respsective character in b, a is bigger.  If the a character is less
       * then a is less, and if they are equal (or both NIL), then the strings
       * are equal.
       */
      ret = *pa - *pb ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_INT 
BlueCstrcasecmp (BLUE_CCHAR *astr, BLUE_CCHAR *bstr)
{
  BLUE_CCHAR *pa ;
  BLUE_CCHAR *pb ;
  BLUE_INT ret ;

  if (astr == BLUE_NULL || bstr == BLUE_NULL)
    {
      if (astr == BLUE_NULL && bstr == BLUE_NULL)
	ret = 0 ;
      else
	ret = -1 ;
    }
  else
    {
      pa = astr ;
      pb = bstr ;

      /*
       * Step through the string until one of them reaches a NIL
       */
      while (BLUE_C_TOUPPER(*pa) == BLUE_C_TOUPPER(*pb) &&
	     *pa != '\0' && *pb != '\0')
	{
	  pa++ ;
	  pb++ ;
	}

      /*
       * Then if the character from a at that position is greater then the
       * respsective character in b, a is bigger.  If the a character is less
       * then a is less, and if they are equal (or both NIL), then the strings
       * are equal.
       */
      ret = BLUE_C_TOUPPER(*pa) - BLUE_C_TOUPPER(*pb) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_INT 
BlueCstrncmp (BLUE_CCHAR *astr, BLUE_CCHAR *bstr, BLUE_SIZET len)
{
  BLUE_CCHAR *pa ;
  BLUE_CCHAR *pb ;
  BLUE_INT ret ;

  if (astr == BLUE_NULL || bstr == BLUE_NULL)
    {
      if (astr == BLUE_NULL && bstr == BLUE_NULL)
	ret = 0 ;
      else
	ret = -1 ;
    }
  else
    {
      pa = astr ;
      pb = bstr ;
      ret = 0 ;
      /*
       * Step through the string until one of them reaches a NIL
       */
      while (*pa == *pb && *pa != '\0' && *pb != '\0' && len > 0)
	{
	  pa++ ;
	  pb++ ;
	  len-- ;
	}
      /*
       * Then if the character from a at that position is greater then the
       * respsective character in b, a is bigger.  If the a character is less
       * then a is less, and if they are equal (or both NIL), then the strings
       * are equal.
       */
      if (len > 0)
	ret = *pa - *pb ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_INT 
BlueCstrncasecmp (BLUE_CCHAR *astr, BLUE_CCHAR *bstr, BLUE_SIZET len)
{
  BLUE_CCHAR *pa ;
  BLUE_CCHAR *pb ;
  BLUE_INT ret ;

  if (astr == BLUE_NULL || bstr == BLUE_NULL)
    {
      if (astr == BLUE_NULL && bstr == BLUE_NULL)
	ret = 0 ;
      else
	ret = -1 ;
    }
  else
    {
      pa = astr ;
      pb = bstr ;
      ret = 0 ;

      /*
       * Step through the string until one of them reaches a NIL
       */
      while (BLUE_C_TOUPPER(*pa) == BLUE_C_TOUPPER(*pb) &&
	     *pa != '\0' && *pb != '\0' && len > 0)
	{
	  pa++ ;
	  pb++ ;
	  len-- ;
	}

      /*
       * Then if the character from a at that position is greater then the
       * respsective character in b, a is bigger.  If the a character is less
       * then a is less, and if they are equal (or both NIL), then the strings
       * are equal.
       */
      if (len > 0)
	ret = BLUE_C_TOUPPER(*pa) - BLUE_C_TOUPPER(*pb) ;
    }
  return (ret) ;
}

/**
 * \internal
 * Measure the size of a string
 */
BLUE_CORE_LIB BLUE_SIZET 
BlueCstrlen (BLUE_CCHAR *astr)
{
  BLUE_SIZET ret ;
  BLUE_CCHAR *pa ;

  if (astr == BLUE_NULL)
    ret = 0 ;
  else
    {
      ret = 0 ;
      pa = astr ;

      while (*pa != '\0')
	{
	  ret++ ;
	  pa++ ;
	}
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_SIZET 
BlueCstrnlen (BLUE_CCHAR *astr, BLUE_SIZET len)
{
  BLUE_SIZET ret ;
  BLUE_CCHAR *pa ;

  if (astr == BLUE_NULL)
    ret = 0 ;
  else
    {
      ret = 0 ;
      pa = astr ;

      while (*pa != '\0' && len > 0)
	{
	  ret++ ;
	  pa++ ;
	  len-- ;
	}
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_CHAR *
BlueCstrdup (BLUE_CCHAR *astr)
{
  BLUE_SIZET size ;
  BLUE_CHAR *ret ;

  if (astr == BLUE_NULL)
    ret = BLUE_NULL ;
  else
    {
      size = BlueCstrlen(astr) ;
      ret = BlueHeapMalloc (size + 1) ;
      BlueCstrcpy (ret, astr) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_CHAR *
BlueCstrndup (BLUE_CCHAR *astr, BLUE_SIZET len)
{
  BLUE_SIZET size ;
  BLUE_CHAR *ret ;
  BLUE_SIZET slen ;

  if (astr == BLUE_NULL)
    ret = BLUE_NULL ;
  else
    {
      slen = BlueCstrnlen(astr, len) ;
      size = BLUE_C_MIN(slen, len) + 1 ;
      ret = BlueHeapMalloc (size) ;
      BlueCstrncpy (ret, astr, size) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_TCHAR *
BlueCtstrndup (BLUE_CTCHAR *astr, BLUE_SIZET len)
{
  BLUE_SIZET size ;
  BLUE_TCHAR *ret ;
  BLUE_SIZET slen ;

  if (astr == BLUE_NULL)
    ret = BLUE_NULL ;
  else
    {
      slen = BlueCtstrnlen(astr, len) ;
      size = BLUE_C_MIN(slen, len) + 1 ;
      ret = BlueHeapMalloc (size * sizeof (BLUE_TCHAR)) ;
      BlueCtstrncpy (ret, astr, size) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_CHAR *
BlueCstrncpy (BLUE_CHAR *dst, BLUE_CCHAR *src, BLUE_SIZET len)
{
  BLUE_CCHAR *psrc;
  BLUE_CHAR *pdst ;
  BLUE_CHAR *ret ;

  if (src == BLUE_NULL || dst == BLUE_NULL)
    {
      ret = BLUE_NULL ;
    }
  else
    {
      psrc = src ;
      pdst = dst ;

      while (*psrc != '\0' && len > 0)
	{
	  *pdst = *psrc ;
	  pdst++ ;
	  psrc++ ;
	  len-- ;
	}
      if (len > 0)
	*pdst = '\0' ;

      ret = dst ;
    }

  return (ret) ;
}

BLUE_CHAR *BlueCstrnupr (BLUE_CHAR *src, BLUE_SIZET len)
{
  BLUE_CHAR *psrc;
  BLUE_CHAR *ret ;

  if (src == BLUE_NULL)
    {
      ret = BLUE_NULL ;
    }
  else
    {
      psrc = src ;

      while (*psrc != '\0' && len > 0)
	{
	  *psrc = BLUE_C_TOUPPER(*psrc) ;
	  psrc++ ;
	  len-- ;
	}
      ret = src ;
    }

  return (ret) ;
}

BLUE_TCHAR *BlueCtstrnupr (BLUE_TCHAR *src, BLUE_SIZET len)
{
  BLUE_TCHAR *psrc;
  BLUE_TCHAR *ret ;

  if (src == BLUE_NULL)
    {
      ret = BLUE_NULL ;
    }
  else
    {
      psrc = src ;

      while (*psrc != '\0' && len > 0)
	{
	  *psrc = BLUE_C_TOUPPER(*psrc) ;
	  psrc++ ;
	  len-- ;
	}
      ret = src ;
    }

  return (ret) ;
}

BLUE_CORE_LIB BLUE_TCHAR *
BlueCtstrncpy (BLUE_TCHAR *dst, BLUE_CTCHAR *src, BLUE_SIZET len)
{
  BLUE_CTCHAR *psrc;
  BLUE_TCHAR *pdst ;
  BLUE_TCHAR *ret ;

  if (src == BLUE_NULL || dst == BLUE_NULL)
    {
      ret = BLUE_NULL ;
    }
  else
    {
      psrc = src ;
      pdst = dst ;

      while (*psrc != TCHAR_EOS && len > 0)
	{
	  *pdst = *psrc ;
	  pdst++ ;
	  psrc++ ;
	  len-- ;
	}
      if (len > 0)
	*pdst = TCHAR_EOS ;

      ret = dst ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_CHAR *
BlueCstrcpy (BLUE_CHAR *dst, BLUE_CCHAR *src)
{
  BLUE_CCHAR *psrc;
  BLUE_CHAR *pdst ;
  BLUE_CHAR *ret ;

  if (src == BLUE_NULL || dst == BLUE_NULL)
    ret = BLUE_NULL ;
  else
    {
      psrc = src ;
      pdst = dst ;

      while (*psrc != '\0')
	{
	  *pdst = *psrc ;
	  pdst++ ;
	  psrc++ ;
	}
      *pdst = '\0' ;
      ret = dst ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_CHAR *
BlueCstrcat (BLUE_CHAR *dst, BLUE_CCHAR *src)
{
  BLUE_CCHAR *psrc;
  BLUE_CHAR *pdst ;
  BLUE_CHAR *ret ;

  if (src == BLUE_NULL || dst == BLUE_NULL)
    ret = BLUE_NULL ;
  else
    {
      psrc = src ;
      pdst = dst + BlueCstrlen(dst) ;

      while (*psrc != '\0')
	{
	  *pdst = *psrc ;
	  pdst++ ;
	  psrc++ ;
	}
      *pdst = '\0' ;
      ret = dst ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_CHAR *
BlueCstrncat (BLUE_CHAR *dst, BLUE_CCHAR *src, BLUE_SIZET size)
{
  BLUE_CCHAR *psrc;
  BLUE_CHAR *pdst ;
  BLUE_CHAR *ret ;

  if (src == BLUE_NULL || dst == BLUE_NULL)
    ret = BLUE_NULL ;
  else
    {
      psrc = src ;
      pdst = dst + BlueCstrlen(dst) ;

      while (*psrc != '\0' && size > 0)
	{
	  *pdst = *psrc ;
	  pdst++ ;
	  psrc++ ;
	  size-- ;
	}
      if (size > 0)
	*pdst = '\0' ;
      ret = dst ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_TCHAR *
BlueCtstrcpy (BLUE_TCHAR *dst, BLUE_CTCHAR *src)
{
  BLUE_CTCHAR *psrc;
  BLUE_TCHAR *pdst ;
  BLUE_TCHAR *ret ;

  if (src == BLUE_NULL || dst == BLUE_NULL)
    ret = BLUE_NULL ;
  else
    {
      psrc = src ;
      pdst = dst ;

      while (*psrc != TCHAR_EOS)
	{
	  *pdst = *psrc ;
	  pdst++ ;
	  psrc++ ;
	}
      *pdst = TCHAR_EOS ;
      ret = dst ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_SIZET 
BlueCtstrlen (BLUE_LPCTSTR str)
{
  BLUE_SIZET ret ;
  BLUE_LPCTSTR p ;

  if (str == BLUE_NULL)
    ret = 0 ;
  else
    for (ret = 0, p = str ; *p != TCHAR_EOS ; ret++, p++) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_SIZET 
BlueCtstrnlen (BLUE_LPCTSTR str, BLUE_SIZET len)
{
  BLUE_SIZET ret ;
  BLUE_LPCTSTR p ;

  if (str == BLUE_NULL)
    ret = 0 ;
  else
    for (ret = 0, p = str ; *p != TCHAR_EOS && len > 0 ; ret++, p++, len--) ;
  return (ret) ;
}


BLUE_CORE_LIB BLUE_LPTSTR 
BlueCtstrdup (BLUE_LPCTSTR str)
{
  BLUE_LPTSTR ret ;
  BLUE_SIZET len ;

  ret = BLUE_NULL ;
  if (str != BLUE_NULL)
    {
      len = (BlueCtstrlen(str) + 1) * sizeof (BLUE_TCHAR) ;
      ret = BlueHeapMalloc (len) ;
      BlueCmemcpy (ret, str, len) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_INT 
BlueCtstrcmp (BLUE_LPCTSTR astr, BLUE_LPCTSTR bstr)
{
  BLUE_CTCHAR *pa ;
  BLUE_CTCHAR *pb ;
  BLUE_INT ret ;

  if (astr == BLUE_NULL || bstr == BLUE_NULL)
    {
      if (astr == BLUE_NULL && bstr == BLUE_NULL)
	ret = 0 ;
      else
	ret = -1 ;
    }
  else
    {
      pa = astr ;
      pb = bstr ;

      /*
       * Step through the string until one of them reaches a NIL
       */
      while (*pa == *pb && *pa != '\0' && *pb != '\0')
	{
	  pa++ ;
	  pb++ ;
	}

      /*
       * Then if the character from a at that position is greater then the
       * respsective character in b, a is bigger.  If the a character is less
       * then a is less, and if they are equal (or both NIL), then the strings
       * are equal.
       */
      ret = *pa - *pb ;
    }
  return ret ;
}

BLUE_CORE_LIB BLUE_INT 
BlueCtstrcasecmp (BLUE_LPCTSTR astr, BLUE_LPCTSTR bstr)
{
  BLUE_CTCHAR *pa ;
  BLUE_CTCHAR *pb ;
  BLUE_INT ret ;

  if (astr == BLUE_NULL || bstr == BLUE_NULL)
    {
      if (astr == BLUE_NULL && bstr == BLUE_NULL)
	ret = 0 ;
      else
	ret = -1 ;
    }
  else
    {
      pa = astr ;
      pb = bstr ;

      /*
       * Step through the string until one of them reaches a NIL
       */
      while (BLUE_C_TTOUPPER(*pa) == BLUE_C_TTOUPPER(*pb) &&
	     *pa != TCHAR_EOS && *pb != TCHAR_EOS)
	{
	  pa++ ;
	  pb++ ;
	}

      /*
       * Then if the character from a at that position is greater then the
       * respsective character in b, a is bigger.  If the a character is less
       * then a is less, and if they are equal (or both NIL), then the strings
       * are equal.
       */
      ret =  BLUE_C_TTOUPPER(*pa) - BLUE_C_TTOUPPER(*pb) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_INT 
BlueCtstrncmp (BLUE_CTCHAR *astr, BLUE_CTCHAR *bstr, BLUE_SIZET len)
{
  BLUE_CTCHAR *pa ;
  BLUE_CTCHAR *pb ;
  BLUE_INT ret ;

  if (astr == BLUE_NULL || bstr == BLUE_NULL)
    {
      if (astr == BLUE_NULL && bstr == BLUE_NULL)
	ret = 0 ;
      else
	ret = -1 ;
    }
  else
    {
      pa = astr ;
      pb = bstr ;
      ret = 0 ;

      /*
       * Step through the string until one of them reaches a NIL
       */
      while (*pa == *pb && *pa != '\0' && *pb != '\0' && len > 0)
	{
	  pa++ ;
	  pb++ ;
	  len-- ;
	}

      /*
       * Then if the character from a at that position is greater then the
       * respsective character in b, a is bigger.  If the a character is less
       * then a is less, and if they are equal (or both NIL), then the strings
       * are equal.
       */
      if (len > 0)
	ret = *pa - *pb ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_INT 
BlueCtstrncasecmp (BLUE_CTCHAR *astr, BLUE_CTCHAR *bstr, BLUE_SIZET len)
{
  BLUE_CTCHAR *pa ;
  BLUE_CTCHAR *pb ;
  BLUE_INT ret ;

  if (astr == BLUE_NULL || bstr == BLUE_NULL)
    {
      if (astr == BLUE_NULL && bstr == BLUE_NULL)
	ret = 0 ;
      else
	ret = -1 ;
    }
  else
    {
      pa = astr ;
      pb = bstr ;
      ret = 0 ;

      /*
       * Step through the string until one of them reaches a NIL
       */
      while (BLUE_C_TTOUPPER(*pa) == BLUE_C_TTOUPPER(*pb) &&
	     *pa != TCHAR_EOS && *pb != TCHAR_EOS && len > 0)
	{
	  pa++ ;
	  pb++ ;
	  len-- ;
	}

      /*
       * Then if the character from a at that position is greater then the
       * respsective character in b, a is bigger.  If the a character is less
       * then a is less, and if they are equal (or both NIL), then the strings
       * are equal.
       */
      if (len > 0)
	ret = BLUE_C_TTOUPPER(*pa) - BLUE_C_TTOUPPER(*pb) ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_LPCTSTR 
BlueCtstrtok (BLUE_LPCTSTR str, BLUE_LPCTSTR terms)
{
  BLUE_LPCTSTR peek ;
  BLUE_BOOL hit ;
  BLUE_LPCTSTR ret ;

  if (str == BLUE_NULL || terms == BLUE_NULL)
    ret = BLUE_NULL ;
  else
    {
      for (peek = str, hit = BLUE_FALSE ; *peek != TCHAR_EOS && !hit ; )
	{
	  BLUE_INT i ;

	  for (i = 0 ; i < BlueCtstrlen(terms) && !hit ; i++)
	    {
	      if (*peek == terms[i])
		hit = BLUE_TRUE ;
	    }
	  if (!hit)
	    peek++ ;
	}
      ret = peek ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_LPSTR 
BlueCstrtok (BLUE_LPCSTR str, BLUE_LPCSTR terms)
{
  BLUE_LPCSTR peek ;
  BLUE_BOOL hit ;
  BLUE_LPSTR ret ;

  if (str == BLUE_NULL || terms == BLUE_NULL)
    ret = BLUE_NULL ;
  else
    {
      for (peek = str, hit = BLUE_FALSE ; *peek != '\0' && !hit ; )
	{
	  BLUE_INT i ;

	  for (i = 0 ; i < BlueCstrlen(terms) && !hit ; i++)
	    {
	      if (*peek == terms[i])
		hit = BLUE_TRUE ;
	    }
	  if (!hit)
	    peek++ ;
	}
      ret = (BLUE_LPSTR) peek ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_LPSTR
BlueCstrchr (BLUE_LPCSTR str, BLUE_CCHAR c) 
{
  BLUE_LPCSTR peek ;
  BLUE_LPSTR ret ;

  ret = BLUE_NULL ;
  if (str != BLUE_NULL)
    {
      for (peek = str ; *peek != c && *peek != '\0' ; peek++) ;
      if (*peek == c)
	ret = (BLUE_LPSTR) peek ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_LPSTR
BlueCmemchr (BLUE_LPCSTR str, BLUE_CCHAR c, BLUE_SIZET size) 
{
  BLUE_LPCSTR peek ;
  BLUE_LPSTR ret ;

  ret = BLUE_NULL ;
  if (str != BLUE_NULL)
    {
      for (peek = str ; *peek != c && size > 0 ; peek++, size--) ;
      if (*peek == c)
	ret = (BLUE_LPSTR) peek ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_LPCTSTR 
BlueCtstrrtok (BLUE_LPCTSTR str, BLUE_LPCTSTR terms)
{
  BLUE_LPCTSTR peek ;
  BLUE_BOOL hit ;
  BLUE_LPCTSTR ret ;

  if (str == BLUE_NULL || terms == BLUE_NULL)
    ret = BLUE_NULL ;
  else
    {
      peek = str + BlueCtstrlen(str) ;

      for (peek--, hit = BLUE_FALSE ; peek != str && !hit ; )
	{
	  BLUE_INT i ;

	  for (i = 0 ; i < BlueCtstrlen(terms) && !hit ; i++)
	    {
	      if (*peek == terms[i])
		hit = BLUE_TRUE ;
	    }
	  if (!hit)
	    peek -- ;
	}
      ret = peek ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_LPCSTR 
BlueCstrrtok (BLUE_LPCSTR str, BLUE_LPCSTR terms)
{
  BLUE_LPCSTR peek ;
  BLUE_BOOL hit ;
  BLUE_LPCSTR ret ;

  if (str == BLUE_NULL || terms == BLUE_NULL)
    ret = BLUE_NULL ;
  else
    {
      peek = str + BlueCstrlen(str) ;

      for (peek--, hit = BLUE_FALSE ; peek != str && !hit ; )
	{
	  BLUE_INT i ;

	  for (i = 0 ; i < BlueCstrlen(terms) && !hit ; i++)
	    {
	      if (*peek == terms[i])
		hit = BLUE_TRUE ;
	    }
	  if (!hit)
	    peek -- ;
	}
      ret = peek ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_CHAR *
BlueCtstr2cstr (BLUE_LPCTSTR str)
{
  BLUE_SIZET len ;
  BLUE_CHAR *cstr ;
  BLUE_INT i ;
  BLUE_CTCHAR *ptstr ;
  BLUE_CHAR *pcstr ;

  cstr = BLUE_NULL ;
  if (str != BLUE_NULL)
    {
      len = BlueCtstrlen (str) ;
      cstr = BlueHeapMalloc (len + 1) ;
      pcstr = cstr ;
      ptstr = str ;
      for (i = 0 ; i < len ; i++)
	*pcstr++ = (BLUE_CHAR) *ptstr++ ;
      *pcstr = '\0' ;
    }
  return (cstr) ;
}

BLUE_CORE_LIB BLUE_LPTSTR 
BlueCcstr2tstr (BLUE_CCHAR *str)
{
  BLUE_SIZET len ;
  BLUE_LPTSTR tstr ;
  BLUE_INT i ;
  BLUE_TCHAR *ptstr ;
  BLUE_CCHAR *pcstr ;

  tstr = BLUE_NULL ;
  if (str != BLUE_NULL)
    {
      len = BlueCstrlen (str) ;
      tstr = BlueHeapMalloc ((len + 1) * sizeof (BLUE_TCHAR)) ;
      ptstr = tstr ;
      pcstr = str ;
      for (i = 0 ; i < len ; i++)
	*ptstr++ = (BLUE_TCHAR) (*pcstr++) ;
      *ptstr = (BLUE_TCHAR) '\0' ;
    }
  return (tstr) ;
}

BLUE_CORE_LIB BLUE_INT 
BlueCmemcmp (BLUE_LPCVOID a, BLUE_LPCVOID b, BLUE_SIZET size)
{
  BLUE_INT ret ;
  BLUE_UINT32 *puint32 ;
  const BLUE_UINT32 *pcuint32 ;
  BLUE_UINT16 *puint16 ;
  const BLUE_UINT16 *pcuint16 ;
  BLUE_UINT8 *puint8 ;
  const BLUE_UINT8 *pcuint8 ;

  if (a == BLUE_NULL || b == BLUE_NULL)
    {
      if (a == BLUE_NULL && b == BLUE_NULL)
	ret = 0 ;
      else
	ret = -1 ;
    }
  else
    {
      ret = 0 ;
      if (!((BLUE_DWORD_PTR) a & 0x03) && !((BLUE_DWORD_PTR) b & 0x03))
	{
	  puint32 = (BLUE_UINT32 *) b ;
	  pcuint32 = (const BLUE_UINT32 *) a ;

	  while (size >= 4 && ret == 0)
	    {
	      ret = *pcuint32++ - *puint32++ ;
	      size -= 4 ;
	    }
	  puint8 = (BLUE_UINT8 *) puint32 ;
	  pcuint8 = (const BLUE_UINT8 *) pcuint32 ;
	  while (size != 0 && ret == 0)
	    {
	      ret = *pcuint8++ - *puint8++ ;
	      size -- ;
	    }

	}
      else if (!((BLUE_DWORD_PTR) a & 0x01) && !((BLUE_DWORD_PTR) b & 0x01))
	{
	  puint16 = (BLUE_UINT16 *) b ;
	  pcuint16 = (const BLUE_UINT16 *) a ;

	  while (size >= 2 && ret == 0)
	    {
	      ret = *pcuint16++ - *puint16++ ;
	      size -= 2 ;
	    }
	  puint8 = (BLUE_UINT8 *) puint16 ;
	  pcuint8 = (const BLUE_UINT8 *) pcuint16 ;
	  while (size != 0 && ret == 0)
	    {
	      ret = *pcuint8++ - *puint8++ ;
	      size -- ;
	    }
	}
      else
	{
	  puint8 = (BLUE_UINT8 *) b ;
	  pcuint8 = (const BLUE_UINT8 *) a ;

	  while (size != 0 && ret == 0)
	    {
	      ret = *pcuint8++ - *puint8++ ;
	      size -- ;
	    }
	}
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_LPVOID 
BlueCmemcpy (BLUE_LPVOID out, BLUE_LPCVOID in, BLUE_SIZET size)
{
  BLUE_LPVOID dst ;
  BLUE_UINT32 *puint32 ;
  const BLUE_UINT32 *pcuint32 ;
  BLUE_UINT16 *puint16 ;
  const BLUE_UINT16 *pcuint16 ;
  BLUE_UINT8 *puint8 ;
  const BLUE_UINT8 *pcuint8 ;

  if (out == BLUE_NULL || in == BLUE_NULL)
    dst = BLUE_NULL ;
  else
    {
      dst = out ;

      if (!((BLUE_DWORD_PTR) in & 0x03) && !((BLUE_DWORD_PTR) out & 0x03))
	{
	  puint32 = (BLUE_UINT32 *) out ;
	  pcuint32 = (const BLUE_UINT32 *) in ;

	  while (size >= 4)
	    {
	      *puint32++ = *pcuint32++ ;
	      size -= 4 ;
	    }
	  puint8 = (BLUE_UINT8 *) puint32 ;
	  pcuint8 = (const BLUE_UINT8 *) pcuint32 ;
	  while (size != 0)
	    {
	      *puint8++ = *pcuint8++ ;
	      size -- ;
	    }
	}
      else if (!((BLUE_DWORD_PTR)in & 0x01) && !((BLUE_DWORD_PTR)out & 0x01))
	{
	  puint16 = (BLUE_UINT16 *) out ;
	  pcuint16 = (const BLUE_UINT16 *) in ;

	  while (size >= 2)
	    {
	      *puint16++ = *pcuint16++ ;
	      size -= 2 ;
	    }
	  puint8 = (BLUE_UINT8 *) puint16 ;
	  pcuint8 = (const BLUE_UINT8 *) pcuint16 ;
	  while (size != 0)
	    {
	      *puint8++ = *pcuint8++ ;
	      size -- ;
	    }
	}
      else
	{
	  puint8 = (BLUE_UINT8 *) out ;
	  pcuint8 = (const BLUE_UINT8 *) in ;

	  while (size != 0)
	    {
	      *puint8++ = *pcuint8++ ;
	      size -- ;
	    }
	}
    }
  return (dst) ;
}

BLUE_CORE_LIB BLUE_LPVOID 
BlueCmemset (BLUE_LPVOID dst, BLUE_INT c, BLUE_SIZET size)
{
  BLUE_CHAR *pa ;
  BLUE_LPVOID ret ;

  if (dst == BLUE_NULL)
    ret = BLUE_NULL ;
  else
    {
      pa = (BLUE_CHAR *) dst ;

      while (size--)
	*pa++ = c ;
      ret = dst ;
    }

  return (ret) ;
}

static BLUE_UINT8 cvtIn[] =
  {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,		/* '0' - '9' */
    100, 100, 100, 100, 100, 100, 100,		/* punctuation */
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,	/* 'A' - 'Z' */
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35,
    100, 100, 100, 100, 100, 100,		/* punctuation */
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,	/* 'a' - 'z' */
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35
  } ;

BLUE_CORE_LIB BLUE_ULONG 
BlueCstrtoul (const BLUE_CHAR *string, BLUE_CHAR **endPtr, BLUE_INT base)
{
  const BLUE_CHAR *p;
  BLUE_ULONG result = 0 ;
  BLUE_UINT digit ;
  BLUE_INT anyDigits = 0 ;

  if (string == BLUE_NULL)
    result = 0 ;
  else
    {
      /*
       * Skip any leading blanks.
       */
      p = string ;
      while (BLUE_C_ISSPACE(*p))
	{
	  p += 1 ;
	}

      /*
       * If no base was provided, pick one from the leading characters
       * of the string.
       */
      if (base == 0)
	{
	  if (*p == '0')
	    {
	      p += 1 ;
	      if (*p == 'x')
		{
		  p += 1 ;
		  base = 16 ;
		}
	      else
		{
		  /*
		   * Must set anyDigits here, otherwise "0" produces a
		   * "no digits" error.
		   */
		  anyDigits = 1 ;
		  base = 8 ;
		}
	    }
	  else
	    base = 10 ;
	}
      else if (base == 16)
	{
	  /*
	   * Skip a leading "0x" from hex numbers.
	   */
	  if ((p[0] == '0') && (p[1] == 'x'))
	    {
	      p += 2 ;
	    }
	}

      /*
       * Sorry this code is so messy, but speed seems important.  Do
       * different things for base 8, 10, 16, and other.
       */
      if (base == 8)
	{
	  for ( ; ; p += 1)
	    {
	      digit = *p - '0' ;
	      if (digit > 7)
		{
		  break ;
		}
	      result = (result << 3) + digit ;
	      anyDigits = 1 ;
	    }
	}
      else if (base == 10)
	{
	  for ( ; ; p += 1)
	    {
	      digit = *p - '0' ;
	      if (digit > 9)
		{
		  break ;
		}
	      result = (10*result) + digit ;
	      anyDigits = 1 ;
	    }
	}
      else if (base == 16)
	{
	  for ( ; ; p += 1)
	    {
	      digit = *p - '0' ;
	      if (digit > ('z' - '0'))
		{
		  break ;
		}
	      digit = cvtIn[digit] ;
	      if (digit > 15)
		{
		  break ;
		}
	      result = (result << 4) + digit ;
	      anyDigits = 1 ;
	    }
	}
      else
	{
	  for ( ; ; p += 1)
	    {
	      digit = *p - '0' ;
	      if (digit > ('z' - '0'))
		{
		  break ;
		}
	      digit = cvtIn[digit] ;
	      if (digit >= (BLUE_UINT) base)
		{
		  break;
		}
	      result = result * base + digit ;
	      anyDigits = 1 ;
	    }
	}

      /*
       * See if there were any digits at all.
       */
      if (!anyDigits)
	{
	  p = string ;
	}

      if (endPtr != BLUE_NULL)
	{
	  *endPtr = (BLUE_CHAR *) p ;
	}
    }
  return result ;
}

BLUE_CORE_LIB BLUE_LONG 
BlueCstrtol (const BLUE_CHAR *string, BLUE_CHAR **endPtr, BLUE_INT base)
{
  const BLUE_CHAR *p;
  BLUE_LONG result;

  if (string == BLUE_NULL)
    result = 0 ;
  else
    {
      /*
       * Skip any leading blanks.
       */
      p = string;
      while (BLUE_C_ISSPACE(*p))
	{
	  p += 1;
	}

      /*
       * Check for a sign.
       */

      if (*p == '-')
	{
	  p += 1 ;
	  result = -((BLUE_LONG) BlueCstrtoul(p, endPtr, base)) ;
	}
      else
	{
	  if (*p == '+')
	    {
	      p += 1 ;
	    }
	  result = BlueCstrtoul(p, endPtr, base) ;
	}

      if ((result == 0) && (endPtr != 0) && (*endPtr == p))
	{
	  *endPtr = (BLUE_CHAR *) string ;
	}
    }
  return result ;
}

/*
 * Copyright Patrick Powell 1995
 * This code is based on code written by Patrick Powell (papowell@astart.com)
 * It may be used for any purpose as long as this notice remains intact
 * on all source code distributions
 */

static BLUE_SIZET dopr(BLUE_CHAR *buffer, BLUE_SIZET maxlen,
		       BLUE_CCHAR *format, va_list args) ;
static BLUE_VOID fmtstr(BLUE_CHAR *buffer, BLUE_SIZET *currlen,
			BLUE_SIZET maxlen, BLUE_CCHAR *value,
			BLUE_INT flags, BLUE_SIZET min, BLUE_SIZET max) ;
static BLUE_VOID fmttstr(BLUE_CHAR *buffer, BLUE_SIZET *currlen,
			 BLUE_SIZET maxlen, BLUE_CTCHAR *value,
			 BLUE_INT flags, BLUE_SIZET min, BLUE_SIZET max) ;
static BLUE_VOID fmttastr(BLUE_CHAR *buffer, BLUE_SIZET *currlen,
			 BLUE_SIZET maxlen, BLUE_CTACHAR *value,
			 BLUE_INT flags, BLUE_SIZET min, BLUE_SIZET max) ;
static BLUE_VOID fmtint(BLUE_CHAR *buffer, BLUE_SIZET *currlen,
			BLUE_SIZET maxlen, BLUE_LONG value, BLUE_INT base,
			BLUE_SIZET min, BLUE_SIZET max, BLUE_INT flags);
static BLUE_VOID dopr_outch(BLUE_CHAR *buffer, BLUE_SIZET *currlen,
			    BLUE_SIZET maxlen, BLUE_CHAR c);

/*
 * dopr(): poor man's version of doprintf
 */

/* format read states */
#define DP_S_DEFAULT 0
#define DP_S_FLAGS   1
#define DP_S_MIN     2
#define DP_S_DOT     3
#define DP_S_MAX     4
#define DP_S_MOD     5
#define DP_S_CONV    6
#define DP_S_DONE    7

/* format flags - Bits */
#define DP_F_MINUS      (1 << 0)
#define DP_F_PLUS       (1 << 1)
#define DP_F_SPACE      (1 << 2)
#define DP_F_NUM        (1 << 3)
#define DP_F_ZERO       (1 << 4)
#define DP_F_UP         (1 << 5)
#define DP_F_UNSIGNED   (1 << 6)

/* Conversion Flags */
#define DP_C_SHORT   1
#define DP_C_LONG    2

#define CHAR_TO_INT(p) ((p)- '0')

static BLUE_SIZET 
dopr(BLUE_CHAR *buffer, BLUE_SIZET maxlen, BLUE_CCHAR *format, va_list args)
{
  BLUE_CHAR ch ;
  BLUE_CCHAR *strvalue ;
  BLUE_CTCHAR *tstrvalue ;
  BLUE_CTACHAR *tastrvalue ;
  BLUE_LONG value ;
  BLUE_SIZET min ;
  BLUE_SIZET max ;
  BLUE_INT state ;
  BLUE_INT flags ;
  BLUE_INT cflags ;
  BLUE_SIZET currlen ;

  state = DP_S_DEFAULT ;
  currlen = flags = cflags = min = 0 ;
  max = -1 ;
  ch = *format++ ;

  while (state != DP_S_DONE)
    {
      if (ch == '\0')
	state = DP_S_DONE ;

      switch(state)
	{
	case DP_S_DEFAULT:
	  if (ch == '%')
	    {
	      state = DP_S_FLAGS ;
	    }
	  else
	    {
	      dopr_outch(buffer, &currlen, maxlen, ch) ;
	    }
	  ch = *format++ ;
	  break ;

	case DP_S_FLAGS:
	  switch (ch)
	    {
	    case '-':
	      flags |= DP_F_MINUS ;
	      ch = *format++ ;
	      break ;
	    case '+':
	      flags |= DP_F_PLUS ;
	      ch = *format++ ;
	      break ;
	    case ' ':
	      flags |= DP_F_SPACE ;
	      ch = *format++ ;
	      break ;
	    case '#':
	      flags |= DP_F_NUM ;
	      ch = *format++ ;
	      break ;
	    case '0':
	      flags |= DP_F_ZERO ;
	      ch = *format++ ;
	      break ;
	    default:
	      state = DP_S_MIN ;
	      break ;
	    }
	  break ;

	case DP_S_MIN:
	  if (BLUE_C_ISDIGIT((BLUE_UINT8)ch))
	    {
	      min = 10 * min + CHAR_TO_INT (ch) ;
	      ch = *format++ ;
	    }
	  else if (ch == '*')
	    {
	      min = va_arg (args, BLUE_INT) ;
	      ch = *format++ ;
	      state = DP_S_DOT ;
	    }
	  else
	    {
	      state = DP_S_DOT ;
	    }
	  break;
	case DP_S_DOT:
	  if (ch == '.')
	    {
	      state = DP_S_MAX ;
	      ch = *format++ ;
	    }
	  else
	    {
	      state = DP_S_MOD ;
	    }
	  break ;
	case DP_S_MAX:
	  if (BLUE_C_ISDIGIT((BLUE_UINT8)ch))
	    {
	      if (max < 0)
		max = 0 ;
	      max = 10 * max + CHAR_TO_INT (ch) ;
	      ch = *format++ ;
	    }
	  else if (ch == '*')
	    {
	      max = va_arg (args, BLUE_INT) ;
	      ch = *format++ ;
	      state = DP_S_MOD ;
	    }
	  else
	    {
	      state = DP_S_MOD ;
	    }
	  break ;

	case DP_S_MOD:
	  switch (ch)
	    {
	    case 'h':
	      cflags = DP_C_SHORT ;
	      ch = *format++ ;
	      break ;
	    case 'l':
	      cflags = DP_C_LONG ;
	      ch = *format++ ;
	      break ;
	    default:
	      break ;
	    }
	  state = DP_S_CONV ;
	  break ;
	case DP_S_CONV:
	  switch (ch)
	    {
	    case 'b':
	      flags |= DP_F_UNSIGNED ;
	      if (cflags == DP_C_SHORT)
		{
		  value = va_arg (args, BLUE_UINT) ;
		}
	      else if (cflags == DP_C_LONG)
		{
		  value = (BLUE_LONG)va_arg (args, BLUE_ULONG) ;
		}
	      else
		{
		  value = (BLUE_LONG)va_arg (args, BLUE_UINT) ;
		}
	      fmtint (buffer, &currlen, maxlen, value, 2, min, max, flags) ;
	      break ;
	    case 'd':
	    case 'i':
	      if (cflags == DP_C_SHORT)
		{
		  value = va_arg (args, BLUE_INT) ;
		}
	      else if (cflags == DP_C_LONG)
		{
		  value = va_arg (args, BLUE_LONG) ;
		}
	      else
		{
		  value = va_arg (args, BLUE_INT) ;
		}
	      fmtint (buffer, &currlen, maxlen, value, 10, min, max, flags) ;
	      break ;
	    case 'o':
	      flags |= DP_F_UNSIGNED ;
	      if (cflags == DP_C_SHORT)
		{
		  value = va_arg (args, BLUE_UINT) ;
		}
	      else if (cflags == DP_C_LONG)
		{
		  value = (BLUE_LONG)va_arg (args, BLUE_ULONG) ;
		}
	      else
		{
		  value = (BLUE_LONG)va_arg (args, BLUE_UINT) ;
		}
	      fmtint (buffer, &currlen, maxlen, value, 8, min, max, flags) ;
	      break ;
	    case 'u':
	      flags |= DP_F_UNSIGNED ;
	      if (cflags == DP_C_SHORT)
		{
		  value = va_arg (args, BLUE_UINT) ;
		}
	      else if (cflags == DP_C_LONG)
		{
		  value = (BLUE_LONG)va_arg (args, BLUE_ULONG) ;
		}
	      else
		{
		  value = (BLUE_LONG)va_arg (args, BLUE_UINT) ;
		}
	      fmtint (buffer, &currlen, maxlen, value, 10, min, max, flags) ;
	      break ;
	    case 'X':
	      flags |= DP_F_UP ;
	    case 'x':
	      flags |= DP_F_UNSIGNED ;
	      if (cflags == DP_C_SHORT)
		{
		  value = va_arg (args, BLUE_UINT) ;
		}
	      else if (cflags == DP_C_LONG)
		{
		  value = (BLUE_LONG)va_arg (args, BLUE_ULONG) ;
		}
	      else
		{
		  value = (BLUE_LONG)va_arg (args, BLUE_UINT) ;
		}
	      fmtint (buffer, &currlen, maxlen, value, 16, min, max, flags) ;
	      break ;
	    case 'c':
	      dopr_outch(buffer, &currlen, maxlen, va_arg (args, BLUE_INT)) ;
	      break ;
#if defined(__APPLE__)
	    case 'e':
	      strvalue = strerror (errno) ;
	      if (!strvalue)
		strvalue = "(no error)" ;
	      if (max == -1)
		{
		  max = BlueCstrlen(strvalue) ;
		}
	      if (min > 0 && max >= 0 && min > max)
		max = min ;
	      fmtstr(buffer, &currlen, maxlen, strvalue, flags, min, max) ;
	      break ;
#endif
	    case 's':
	      strvalue = va_arg (args, BLUE_CHAR *) ;
	      if (!strvalue)
		strvalue = "(null)" ;
	      if (max == -1)
		{
		  max = BlueCstrlen(strvalue) ;
		}
	      if (min > 0 && max >= 0 && min > max)
		max = min ;
	      fmtstr(buffer, &currlen, maxlen, strvalue, flags, min, max) ;
	      break ;
	    case 'S':
	      tstrvalue = va_arg (args, BLUE_TCHAR *) ;
	      if (!tstrvalue)
		tstrvalue = TSTR("(null)") ;
	      if (max == -1)
		{
		  max = BlueCtstrlen(tstrvalue) ;
		}
	      if (min > 0 && max >= 0 && min > max)
		max = min ;
	      fmttstr(buffer, &currlen, maxlen, tstrvalue, flags, min, max) ;
	      break ;
	    case 'A':
	      tastrvalue = va_arg (args, BLUE_TACHAR *) ;
	      if (!tastrvalue)
		tastrvalue = TASTR("(null)") ;
	      if (max == -1)
		{
		  max = BlueCtastrlen(tastrvalue) ;
		}
	      if (min > 0 && max >= 0 && min > max)
		max = min ;
	      fmttastr(buffer, &currlen, maxlen, tastrvalue, flags, min, max) ;
	      break ;
	    case 'p':
	      flags |= DP_F_UNSIGNED ;
	      strvalue = va_arg (args, BLUE_CHAR *) ;
	      fmtint (buffer, &currlen, maxlen,
		      (BLUE_ULONG)((BLUE_ULONG_PTR)strvalue & BLUE_ULONG_MAX),
		      16, min, max, flags) ;
	      break ;
	    case 'n':
	      if (cflags == DP_C_SHORT)
		{
		  BLUE_SHORT *num ;
		  num = va_arg (args, BLUE_SHORT *) ;
		  *num = (BLUE_SHORT) currlen ;
		}
	      else if (cflags == DP_C_LONG)
		{
		  BLUE_LONG *num ;
		  num = va_arg (args, BLUE_LONG *) ;
		  *num = (BLUE_LONG)currlen ;
		}
	      else
		{
		  BLUE_INT *num ;
		  num = va_arg (args, BLUE_INT *) ;
		  *num = (BLUE_INT) currlen ;
		}
	      break ;
	    case '%':
	      dopr_outch(buffer, &currlen, maxlen, ch) ;
	      break ;
	    case 'w':
	      /* not supported yet, treat as next char */
	      ch = *format++ ;
	      break ;
	    default:
	      /* Unknown, skip */
	      break ;
	    }
	  ch = *format++ ;
	  state = DP_S_DEFAULT ;
	  flags = cflags = min = 0 ;
	  max = -1 ;
	  break ;
	case DP_S_DONE:
	  break ;
	default:
	  /* hmm? */
	  break ; /* some picky compilers need this */
	}
    }
  if (maxlen != 0)
    {
      if (currlen < maxlen - 1)
	buffer[currlen] = '\0' ;
      else if (maxlen > 0)
	buffer[maxlen - 1] = '\0' ;
    }

  return currlen ;
}

static BLUE_VOID 
fmtstr(BLUE_CHAR *buffer, BLUE_SIZET *currlen, BLUE_SIZET maxlen, 
       BLUE_CCHAR *value, BLUE_INT flags, BLUE_SIZET min, BLUE_SIZET max)
{
  BLUE_SIZET padlen ;
  BLUE_SIZET strln ;
  BLUE_INT cnt = 0 ;

  strln  = BlueCstrlen (value) ;

  padlen = min - strln ;

  if (padlen < 0)
    padlen = 0 ;
  if (flags & DP_F_MINUS)
    padlen = -padlen; /* Left Justify */

  while ((padlen > 0) && (cnt < max))
    {
      dopr_outch(buffer, currlen, maxlen, ' ') ;
      --padlen ;
      ++cnt ;
    }
  while (*value && (cnt < max))
    {
      dopr_outch(buffer, currlen, maxlen, *value++) ;
      ++cnt ;
    }
  while ((padlen < 0) && (cnt < max))
    {
      dopr_outch(buffer, currlen, maxlen, ' ') ;
      ++padlen ;
      ++cnt ;
    }
}

static BLUE_VOID 
fmttstr(BLUE_CHAR *buffer, BLUE_SIZET *currlen, BLUE_SIZET maxlen, 
	BLUE_CTCHAR *value, BLUE_INT flags, BLUE_SIZET min, BLUE_SIZET max)
{
  BLUE_SIZET padlen ;
  BLUE_SIZET strln ;
  BLUE_INT cnt = 0 ;

  strln  = BlueCtstrlen (value) ;

  padlen = min - strln ;

  if (padlen < 0)
    padlen = 0 ;
  if (flags & DP_F_MINUS)
    padlen = -padlen; /* Left Justify */

  while ((padlen > 0) && (cnt < max))
    {
      dopr_outch(buffer, currlen, maxlen, ' ') ;
      --padlen ;
      ++cnt ;
    }
  while (*value && (cnt < max))
    {
      dopr_outch(buffer, currlen, maxlen, (BLUE_CCHAR) (*value++)) ;
      ++cnt ;
    }
  while ((padlen < 0) && (cnt < max))
    {
      dopr_outch(buffer, currlen, maxlen, ' ') ;
      ++padlen ;
      ++cnt ;
    }
}

static BLUE_VOID 
fmttastr(BLUE_CHAR *buffer, BLUE_SIZET *currlen, BLUE_SIZET maxlen, 
	 BLUE_CTACHAR *value, BLUE_INT flags, BLUE_SIZET min, BLUE_SIZET max)
{
  BLUE_SIZET padlen ;
  BLUE_SIZET strln ;
  BLUE_INT cnt = 0 ;

  strln  = BlueCtastrlen (value) ;

  padlen = min - strln ;

  if (padlen < 0)
    padlen = 0 ;
  if (flags & DP_F_MINUS)
    padlen = -padlen; /* Left Justify */

  while ((padlen > 0) && (cnt < max))
    {
      dopr_outch(buffer, currlen, maxlen, ' ') ;
      --padlen ;
      ++cnt ;
    }
  while (*value && (cnt < max))
    {
      dopr_outch(buffer, currlen, maxlen, (BLUE_CCHAR) (*value++)) ;
      ++cnt ;
    }
  while ((padlen < 0) && (cnt < max))
    {
      dopr_outch(buffer, currlen, maxlen, ' ') ;
      ++padlen ;
      ++cnt ;
    }
}

/* Have to handle DP_F_NUM (ie 0x and 0 alternates) */

static BLUE_VOID 
fmtint(BLUE_CHAR *buffer, BLUE_SIZET *currlen, BLUE_SIZET maxlen, 
       BLUE_LONG value, BLUE_INT base, BLUE_SIZET min, BLUE_SIZET max,
       BLUE_INT flags)
{
#define MAX_CONVERT_PLACES 40
  BLUE_INT signvalue = 0 ;
  BLUE_ULONG uvalue ;
  BLUE_CHAR  convert[MAX_CONVERT_PLACES] ;
  BLUE_INT place = 0 ;
  BLUE_SIZET spadlen = 0 ; /* amount to space pad */
  BLUE_SIZET zpadlen = 0 ; /* amount to zero pad */
  BLUE_INT caps = 0 ;

  if (max < 0)
    max = 0 ;

  uvalue = value ;

  if (!(flags & DP_F_UNSIGNED))
    {
      if (value < 0)
	{
	  signvalue = '-' ;
	  uvalue = -value ;
	}
      else
	{
	  if (flags & DP_F_PLUS)  /* Do a sign (+/i) */
	    signvalue = '+' ;
	  else if (flags & DP_F_SPACE)
	    signvalue = ' ' ;
	}
    }

  if (flags & DP_F_UP)
    caps = 1 ; /* Should characters be upper case? */

  do
    {
      convert[place++] =
	(caps ? "0123456789ABCDEF" : "0123456789abcdef")
	[uvalue % (BLUE_UINT) base] ;
      uvalue = (uvalue / (BLUE_UINT)base ) ;
    }
  while (uvalue && (place < MAX_CONVERT_PLACES)) ;

  if (place == MAX_CONVERT_PLACES)
    place-- ;
  convert[place] = 0 ;

  zpadlen = max - place ;
  spadlen = min - BLUE_C_MAX (max, place) - (signvalue ? 1 : 0) ;
  if (zpadlen < 0)
    zpadlen = 0 ;
  if (spadlen < 0)
    spadlen = 0 ;
  if (flags & DP_F_ZERO)
    {
      zpadlen = BLUE_C_MAX(zpadlen, spadlen) ;
      spadlen = 0 ;
    }
  if (flags & DP_F_MINUS)
    spadlen = -spadlen ; /* Left Justifty */

  /* Spaces */
  while (spadlen > 0)
    {
      dopr_outch(buffer, currlen, maxlen, ' ') ;
      --spadlen ;
    }

  /* Sign */
  if (signvalue)
    dopr_outch(buffer, currlen, maxlen, signvalue) ;

  /* Zeros */
  if (zpadlen > 0)
    {
      while (zpadlen > 0)
	{
	  dopr_outch(buffer, currlen, maxlen, '0') ;
	  --zpadlen ;
	}
    }

  /* Digits */
  while (place > 0)
    dopr_outch(buffer, currlen, maxlen, convert[--place]) ;

  /* Left Justified spaces */
  while (spadlen < 0)
    {
      dopr_outch(buffer, currlen, maxlen, ' ') ;
      ++spadlen ;
    }
}

static BLUE_VOID 
dopr_outch(BLUE_CHAR *buffer, BLUE_SIZET *currlen, BLUE_SIZET maxlen, 
	   BLUE_CHAR c)
{
  if (*currlen < maxlen)
    {
      buffer[(*currlen)] = c ;
    }
  (*currlen)++ ;
}

BLUE_CORE_LIB BLUE_SIZET 
BlueCvsnprintf (BLUE_CHAR *str, BLUE_SIZET count,
		BLUE_CCHAR *fmt, va_list args)
{
  BLUE_SIZET res ;

  if ((BLUE_INT)count < 0)
    count = 0 ;

  res = dopr(str, count, fmt, args) ;

  return (res) ;
}

BLUE_CORE_LIB BLUE_SIZET 
BlueCsnprintf(BLUE_CHAR *str, BLUE_SIZET count, BLUE_CCHAR *fmt,...)
{
  BLUE_SIZET ret ;
  va_list ap ;

  va_start(ap, fmt) ;

  ret = BlueCvsnprintf(str, count, fmt, ap) ;

  va_end(ap) ;
  return ret ;
}


BLUE_CORE_LIB BLUE_VOID 
BlueCprintf(BLUE_CCHAR *fmt,...)
{
  BLUE_CHAR *obuf ;
  BLUE_SIZET len ;

  va_list ap ;

  va_start(ap,fmt) ;
  len = BlueCvsnprintf(BLUE_NULL, 0, fmt, ap) ;
  va_end(ap) ;

  obuf = BlueHeapMalloc (len+1) ;
  va_start(ap,fmt) ;
  BlueCvsnprintf(obuf, len+1, fmt, ap) ;
  va_end(ap) ;

  BlueWriteStdOut (obuf, len) ;

  BlueHeapFree (obuf) ;
}

BLUE_CORE_LIB BLUE_CHAR *
BlueCsaprintf(BLUE_CCHAR *fmt,...)
{
  BLUE_CHAR *obuf ;
  BLUE_SIZET len ;

  va_list ap ;

  va_start(ap, fmt) ;
  len = BlueCvsnprintf(BLUE_NULL, 0, fmt, ap) ;
  va_end(ap) ;

  obuf = BlueHeapMalloc (len+1) ;
  va_start(ap,fmt) ;
  BlueCvsnprintf(obuf, len+1, fmt, ap) ;
  va_end(ap) ;

  return (obuf) ;
}

#if 0
BLUE_CORE_LIB BLUE_VOID 
BlueTraceInit (BLUE_VOID)
{
  struct trace_t *trace ;

  trace = BlueGetTrace() ;
  if (trace == BLUE_NULL)
    {
      trace = BlueHeapMalloc (sizeof (struct trace_t)) ;
      if (trace != BLUE_NULL)
	{
	  BlueCmemset (trace, '\0', sizeof (struct trace_t)) ;
	  trace->BlueCTraceOffset = 0 ;
	  trace->BlueCTraceLock = BlueLockInit() ;
	  BlueSetTrace (trace) ;
	  BlueCTrace ("Trace Buffer Initialized\n") ;
	}
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueTraceDestroy (BLUE_VOID)
{
  struct trace_t *trace ;

  trace = BlueGetTrace() ;
  if (trace != BLUE_NULL)
    {
      BlueLockDestroy(trace->BlueCTraceLock);
      BlueSetTrace(BLUE_NULL);
      BlueHeapFree (trace);
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueCTrace(BLUE_CCHAR *fmt,...)
{
  BLUE_CHAR *obuf ;
  BLUE_CHAR *obuf2 ;
  BLUE_SIZET len ;
  BLUE_SIZET olen ;
  struct trace_t *trace ;
  BLUE_PROCESS_ID pid ;

  va_list ap ;

  va_start(ap, fmt) ;

  len = BlueCvsnprintf(BLUE_NULL, 0, fmt, ap) ;
  va_end(ap) ;
  obuf = BlueHeapMalloc (len+1) ;
  va_start(ap, fmt) ;
  BlueCvsnprintf(obuf, len+1, fmt, ap) ;
  va_end(ap) ;

  /*
   * prepend process id
   */
  pid = BlueProcessGet() ;
  obuf2 = BlueCsaprintf ("%8d: %s\n", pid, obuf) ;
  len = BlueCstrlen (obuf2) ;
  BlueHeapFree (obuf) ;

  trace = BlueGetTrace() ;
  if (trace != BLUE_NULL && ((len + 1) < BLUE_PARAM_TRACE_LEN))
    {
      BlueLock (trace->BlueCTraceLock) ;
      olen = BLUE_C_MIN (len, BLUE_PARAM_TRACE_LEN - 
			 trace->BlueCTraceOffset - 1) ;
      BlueCmemcpy (trace->BlueCTraceBuf + trace->BlueCTraceOffset, 
		   obuf2, olen) ;
      *(trace->BlueCTraceBuf + trace->BlueCTraceOffset + olen) = '\0' ;

      trace->BlueCTraceOffset += (olen + 1) ;
      if ((trace->BlueCTraceOffset) == BLUE_PARAM_TRACE_LEN)
	trace->BlueCTraceOffset = 0 ;

      len -= olen ;
      if (len > 0)
	{
	  BlueCmemcpy (trace->BlueCTraceBuf + trace->BlueCTraceOffset, 
		       obuf2 + olen, len) ;
	  *(trace->BlueCTraceBuf + trace->BlueCTraceOffset + len) = 
	    '\0' ;
	  trace->BlueCTraceOffset += (len + 1) ;
	}
      BlueUnlock (trace->BlueCTraceLock) ;
    }
  BlueHeapFree (obuf2) ;
}

BLUE_CORE_LIB BLUE_VOID BlueCDumpTrace(BLUE_VOID)
{
  struct trace_t *trace ;

  BLUE_CHAR *obuf ;
  BLUE_SIZET olen ;
  BLUE_INT off ;

  trace = BlueGetTrace() ;
  if (trace != BLUE_NULL)
    {
      off = trace->BlueCTraceOffset ;
      do
	{
	  obuf = trace->BlueCTraceBuf + off ;
	  olen = BlueCstrlen (obuf) ;
	  BlueCprintf ("%s", obuf) ;
	  off += (olen + 1) ;
	  if (off == BLUE_PARAM_TRACE_LEN)
	    off = 0 ;
	} 
      while (off != trace->BlueCTraceOffset) ;
    }
}
#endif

/* xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx */
#define hextoa(a) ((a) >= 10 ? 'A' + ((a) - 10) : '0' + (a))
#define atohex(a) ((a) >= '0' && (a) <= '9' ? ((a) - '0') : \
    (a) >= 'a' && (a) <= 'f' ? ((a) - 'a' + 10) : ((a) - 'A' + 10))

BLUE_VOID 
BlueCuuidtoa (BLUE_UUID uuid, BLUE_CHAR *out)
{
  BLUE_INT i ;
  BLUE_INT val ;

  for (i = 0 ; i < 4 ; i++)
    {
      val = uuid[i] ;
      *out++ = hextoa(val>>4) ;
      *out++ = hextoa(val & 0x0F) ;
    }
  *out++ = '-' ;
  for (i = 4 ; i < 6 ; i++)
    {
      val = uuid[i] ;
      *out++ = hextoa(val>>4) ;
      *out++ = hextoa(val & 0x0F) ;
    }
  *out++ = '-' ;
  for (i = 6 ; i < 8 ; i++)
    {
      val = uuid[i] ;
      *out++ = hextoa(val>>4) ;
      *out++ = hextoa(val & 0x0F) ;
    }
  *out++ = '-' ;
  for (i = 8 ; i < 10 ; i++)
    {
      val = uuid[i] ;
      *out++ = hextoa(val>>4) ;
      *out++ = hextoa(val & 0x0F) ;
    }
  *out++ = '-' ;
  for (i = 10 ; i < 16 ; i++)
    {
      val = uuid[i] ;
      *out++ = hextoa(val>>4) ;
      *out++ = hextoa(val & 0x0F) ;
    }
  *out++ = '\0' ;
}

BLUE_VOID 
BlueCatouuid (const BLUE_CHAR *in, BLUE_UUID uuid)
{
  BLUE_INT i ;
  BLUE_INT val ;

  for (i = 0 ; i < 4 ; i++)
    {
      val = atohex(*in) << 4 ;
      in++ ;
      val |= atohex(*in) ;
      in++ ;
      uuid[i] = val ;
    }
  in++ ;
  for (i = 4 ; i < 6 ; i++)
    {
      val = atohex(*in) << 4 ;
      in++ ;
      val |= atohex(*in) ;
      in++ ;
      uuid[i] = val ;
    }
  in++ ;
  for (i = 6 ; i < 8 ; i++)
    {
      val = atohex(*in) << 4 ;
      in++ ;
      val |= atohex(*in) ;
      in++ ;
      uuid[i] = val ;
    }
  in++ ;
  for (i = 8 ; i < 10 ; i++)
    {
      val = atohex(*in) << 4 ;
      in++ ;
      val |= atohex(*in) ;
      in++ ;
      uuid[i] = val ;
    }
  in++ ;
  for (i = 10 ; i < 16 ; i++)
    {
      val = atohex(*in) << 4 ;
      in++ ;
      val |= atohex(*in) ;
      in++ ;
      uuid[i] = val ;
    }
}

