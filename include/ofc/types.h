/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_TYPES_H__)
#define __OFC_TYPES_H__

#include "ofc/config.h"

/**
 * \defgroup OfcTypes Global Types Used by all Components
 *
 * Open Files is designed as a highly portable subsystem.  As such, it is
 * necessary to define all types we use consistently throughout the product.
 * Any platform generic code should use only types defined within this 
 * group.  APIs to all Open Files components will use types either defined
 * in this group, or in some other facility dependent on this group.
 *
 * When porting Open Files to a new platform, some attention should be
 * paid to the definition of these types.  In most cases, no modification
 * to any of these definitions is necessary.  
 */

/** \{ */

/**
 * Represents the size of a string or structure
 */
typedef long int OFC_SIZET ;
/**
 * Represents a generic platform default integer
 */
typedef int OFC_INT ;
/**
 * Represents a generic platform default unsigned integer
 */
typedef unsigned int OFC_UINT ;
/**
 * Represents a 32 bit signed integer
 */
#if defined(OFC_LONGINT_64)
typedef int OFC_INT32 ;
#else
typedef long int OFC_INT32 ;
#endif
/**
 * Represents a 16 bit signed integer
 */
typedef short int OFC_INT16 ;
/**
 * Represents an 8 bit signed integer
 */
typedef char OFC_INT8 ;
/**
 * Represents a character byte
 */
typedef char OFC_CHAR ;
/**
 * Another representation of a character byte
 */
typedef char OFC_BYTE ;
/**
 * A pointer to a character byte array
 */
typedef OFC_BYTE *OFC_LPBYTE ;
/**
 * Represents an unsigned 8 bit character
 */
typedef unsigned char OFC_UCHAR ;
/**
 * Represents a generic VOID 
 */
typedef void OFC_VOID ;
/**
 * Represents a generic long value
 */
typedef long OFC_LONG ;
/**
 * Represents a pointer to a generic long value
 */
typedef OFC_LONG *OFC_PLONG ;
/**
 * Represents a generic short value
 */
typedef short OFC_SHORT ;
/**
 * Represents a 32 bit unsigned integer
 */
#if defined(OFC_LONGINT_64)
typedef unsigned int OFC_UINT32 ;
#else
typedef unsigned long int OFC_UINT32 ;
#endif
/**
 * Represents a 16 bit unsigned integer
 */
typedef unsigned short int OFC_UINT16 ;
/**
 * Represents an 8 bit unsigned integer
 */
typedef unsigned char OFC_UINT8 ;

#if defined(OFC_64BIT_INTEGER)
/**
 * Represents an unsigned 64 bit value
 */
typedef unsigned long long int OFC_UINT64 ;
/**
 * Represents a signed 64 bit value
 */
typedef long long int OFC_INT64 ;
#else
/**
 * Represents a signed 64 bit value
 */
typedef struct
{
  /**
   * The low order 32 bits
   */
  OFC_UINT32 low ;
  /**
   * The high order 32 bits
   */
  OFC_INT32 high ;
} OFC_INT64 ;
/**
 * Represents an unsigned 64 bit value
 */
typedef struct
{
  /**
   * The low order 32 bits
   */
  OFC_UINT32 low ;
  /**
   * The high order 32 bits
   */
  OFC_UINT32 high ;
} OFC_UINT64 ;

#endif

/**
 * Represents a generic large integer
 */
typedef OFC_INT64 OFC_LARGE_INTEGER ;

#if defined(OFC_64BIT_INTEGER)
/**
 * Return the high order 32 bits of a large integer
 */
#define OFC_LARGE_INTEGER_HIGH(x) ((OFC_UINT32)((x)>>32))
/**
 * Return the low order 32 bits of a large integer
 */
#define OFC_LARGE_INTEGER_LOW(x) ((OFC_UINT32)((x)&0xFFFFFFFF))
/**
 * Assign one large integer to another
 */
#define OFC_LARGE_INTEGER_ASSIGN(x,y) x=y
/**
 * Initialize a large integer from two 32 bit integers
 */
#define OFC_LARGE_INTEGER_SET(x, y, z) (x)=(OFC_LARGE_INTEGER)(z)<<32|(y)
/*
 * Compare two large integers
 */
#define OFC_LARGE_INTEGER_EQUAL(x,y) (x==y)

#define OFC_LARGE_INTEGER_INCR(x) x++

#define OFC_LARGE_INTEGER_AND(x, y, z) (x)&=((OFC_LARGE_INTEGER)(z)<<32|(y))

#define OFC_LARGE_INTEGER_INIT(x,y) (OFC_LARGE_INTEGER)(y)<<32|(x)

#else
/**
 * Return the high order 32 bits of a large integer
 */
#define OFC_LARGE_INTEGER_HIGH(x) ((x).high)
/**
 * Return the low order 32 bits of a large integer
 */
#define OFC_LARGE_INTEGER_LOW(x) ((x).low)
/**
 * Assign one large integer to another
 */
#define OFC_LARGE_INTEGER_ASSIGN(x,y) ofc_memcpy(&x,&y, sizeof(OFC_LARGE_INTEGER))
/**
 * Initialize a large integer from two 32 bit integers
 */
#define OFC_LARGE_INTEGER_SET(x, y, z) {(x).low = (y); (x).high=(z);}

#define OFC_LARGE_INTEGER_INIT(x,y) {x; y}

/*
 * Compare two large integers
 */
#define OFC_LARGE_INTEGER_EQUAL(x,y) ((x).high==(y).high && (x).low==(y).low)
/*
 * Increment
 */
#define OFC_LARGE_INTEGER_INCR(x) ((x).low==0xFFFFFFFF?\
				    (x).high++,(x).low=0:(x).low++)

#define OFZ_LARGE_INTEGER_AND(x, y, z) {(x).low &= (y); (x).high&=(z);}

//#define OFC_LARGE_INTEGER_ADD(x,y,z) ((z).low=(x).low+(y).low,(z).high=(x).high+(y).high,(z).low<(x).low||(z).low<(y).low?(z).high++) 

#endif

/**
 * Represents an offset into a structure
 */
typedef OFC_LARGE_INTEGER OFC_OFFT ;
/**
 * Represents a generic unsigned long value
 */
typedef unsigned long OFC_ULONG ;
/**
 * Represents the maximum unsigned long value
 */
#define	OFC_ULONG_MAX ((OFC_ULONG)(~0L))
/**
 * Represents the maximum positive long value
 */
#define	OFC_LONG_MAX ((OFC_LONG)(OFC_ULONG_MAX >> 1))
/**
 * Represents the minimum negative long value
 */
#define	OFC_LONG_MIN ((OFC_LONG)(~OFC_LONG_MAX))
/**
 * Represents a pointer to an Unsigned Long
 */
typedef OFC_ULONG *OFC_LPULONG ;

#if defined(OFC_64BIT_POINTER)
/**
 * Represents a pointer to a 64 bit value
 */
typedef OFC_UINT64 OFC_ULONG_PTR ;
#else
/**
 * Represents a pointer to a 64 bit value
 */
typedef OFC_UINT32 OFC_ULONG_PTR ;
#endif

/**
 * Represents a void constant
 */
typedef const OFC_VOID OFC_CVOID ;
/**
 * Represents a pointer to a wide character string
 */
typedef OFC_WCHAR *OFC_LPWSTR ;
/**
 * Another representation of a pointer to a wide character string
 */
typedef OFC_WCHAR *OFC_LMSTR ;
/**
 * Represents a constant wide character
 */
typedef const OFC_WCHAR OFC_CWCHAR ;
/**
 * Represents a pointer to a constant wide character string
 */
typedef const OFC_WCHAR *OFC_LPCWSTR ;
/**
 * Represents a constant character
 */
typedef const OFC_CHAR OFC_CCHAR ;
/**
 * Represents a pointer to a character string
 */
typedef OFC_CHAR *OFC_LPSTR ;
/**
 * Represents a pointer to a constant character string
 */
typedef const OFC_CHAR *OFC_LPCSTR ;
/**
 * Represents a Generic WORD
 */
typedef OFC_UINT16 OFC_WORD ;
/**
 * Represents a generic double word
 */
typedef OFC_UINT32 OFC_DWORD ;
/**
 * The mask to associate with a double word
 */
#define OFC_DWORD_MASK 0xFFFFFFFF 
/**
 * Represents a pointer to an array of double words
 */
typedef OFC_DWORD *OFC_LPDWORD ;
#if defined(OFC_64BIT_POINTER)
/**
 * Represents a pointer to a double word
 */
typedef OFC_UINT64 OFC_DWORD_PTR ;
#else
/**
 * Represents a pointer to a double word
 */
typedef OFC_UINT32 OFC_DWORD_PTR ;
#endif
/**
 * Represents a pointer to a void element
 */
typedef OFC_VOID *OFC_LPVOID ;
/**
 * Another representation of a pointer to a void element
 */
typedef OFC_VOID *OFC_PVOID ;
/**
 * Represents a pointer to a constant element
 */
typedef const OFC_VOID *OFC_LPCVOID ;

/**
 * Represents a wide character
 */
typedef OFC_WCHAR OFC_TCHAR ;
/**
 * Represents a wide character constant
 */
typedef const OFC_WCHAR OFC_CTCHAR ;

#if defined(OFC_UNICODE_API)
/**
 * Represents a wide character
 */
typedef OFC_TCHAR OFC_TACHAR ;
/**
 * Represents a wide character constant
 */
typedef const OFC_TCHAR OFC_CTACHAR ;
#else
/**
 * Represents a wide character
 */
typedef OFC_CHAR OFC_TACHAR ;
/**
 * Represents a wide character constant
 */
typedef const OFC_CHAR OFC_CTACHAR ;
#endif

/**
 * Represents a Pointer to a wide character string
 */
typedef OFC_TCHAR *OFC_LPTSTR ;
typedef OFC_TACHAR *OFC_LPTASTR ;
/**
 * Represents a pointer to a wide character constant string
 */
typedef const OFC_TCHAR *OFC_LPCTSTR ;
typedef const OFC_TACHAR *OFC_LPCTASTR ;
/**
 * Represents a Millisecond time value
 */
typedef OFC_INT32 OFC_MSTIME ;

/**
 * A wide character backslash
 */
#define TCHAR_BACKSLASH L'\\'
/**
 * A wide character slash
 */
#define TCHAR_SLASH L'/'
/**
 * A wide character colon
 */
#define TCHAR_COLON L':'
/**
 * A wide character end of string
 */
#define TCHAR_EOS L'\0'
/**
 * A wide character ampersand
 */
#define TCHAR_AMP L'@'
/**
 * A wide character String
 */
#define TSTR(x) (const OFC_TCHAR *) L##x
/**
 * A wide character
 */
#define TCHAR(x) (const OFC_TCHAR) L##x

#if defined(OFC_UNICODE_API)
/**
 * A wide character backslash
 */
#define TACHAR_BACKSLASH TCHAR('\\')
/**
 * A wide character slash
 */
#define TACHAR_SLASH TCHAR('/')
/**
 * A wide character colon
 */
#define TACHAR_COLON TCHAR(':')
/**
 * A wide character end of string
 */
#define TACHAR_EOS TCHAR('\0')
/**
 * A wide character ampersand
 */
#define TACHAR_AMP TCHAR('@')
/**
 * A wide character String
 */
#define TASTR(x) TSTR(x)
/**
 * A wide character
 */
#define TACHAR(x) TCHAR(x)
#else
/**
 * A wide character backslash
 */
#define TACHAR_BACKSLASH '\\'
/**
 * A wide character slash
 */
#define TACHAR_SLASH '/'
/**
 * A wide character colon
 */
#define TACHAR_COLON ':'
/**
 * A wide character end of string
 */
#define TACHAR_EOS '\0'
/**
 * A wide character ampersand
 */
#define TACHAR_AMP '@'
/**
 * A wide character string
 */
#define TASTR(x) x
/**
 * A wide character
 */
#define TACHAR(x) x
#endif

/**
 * The length of a UUID
 */
#define OFC_UUID_LEN 16
/**
 * Represents a UUID
 */
typedef OFC_UCHAR OFC_UUID[OFC_UUID_LEN] ;

/**
 * Represents a Boolean value
 *
 * This shoudl be a 8 bit value on a windows system to be compatable with
 * Windows BOOL
 */
typedef OFC_UINT8 OFC_BOOL ;
enum
  {
    /**
     * The value is false
     */
    OFC_FALSE = 0,
    /**
     * The value is true
     */
    OFC_TRUE = 1
  } ;

/**
 * A NULL Pointer
 */
#define OFC_NULL ((OFC_LPVOID) 0)
/**
 * An IO Vector used by SASL
 */
typedef struct _iovec {
    OFC_LONG iov_len;
    OFC_CHAR *iov_base;
} OFC_IOVEC ;



/**
 * Container of macro
 */
#define container_of(ptr,type,member) \
  (type *)((OFC_CHAR *)(ptr)-(OFC_CHAR *)&((type *)0)->member)

#endif

/** \} */
