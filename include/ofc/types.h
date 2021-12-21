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
#if !defined(__BLUE_TYPES_H__)
#define __BLUE_TYPES_H__

#include "ofc/config.h"

/**
 * \defgroup BlueTypes Global Types Used by all Components
 *
 * Blue Share is designed as a highly portable subsystem.  As such, it is
 * necessary to define all types we use consistently throughout the product.
 * Any platform generic code should use only types defined within this 
 * group.  APIs to all Blue Share components will use types either defined
 * in this group, or in some other facility dependent on this group.
 *
 * When porting Blue Share to a new platform, some attention should be
 * paid to the definition of these types.  In most cases, no modification
 * to any of these definitions is necessary.  
 */

/** \{ */

/**
 * Represents the size of a string or structure
 */
typedef long int BLUE_SIZET ;
/**
 * Represents a generic platform default integer
 */
typedef int BLUE_INT ;
/**
 * Represents a generic platform default unsigned integer
 */
typedef unsigned int BLUE_UINT ;
/**
 * Represents a 32 bit signed integer
 */
#if defined(BLUE_PARAM_LONGINT_64)
typedef int BLUE_INT32 ;
#else
typedef long int BLUE_INT32 ;
#endif
/**
 * Represents a 16 bit signed integer
 */
typedef short int BLUE_INT16 ;
/**
 * Represents an 8 bit signed integer
 */
typedef char BLUE_INT8 ;
/**
 * Represents a character byte
 */
typedef char BLUE_CHAR ;
/**
 * Another representation of a character byte
 */
typedef char BLUE_BYTE ;
/**
 * A pointer to a character byte array
 */
typedef BLUE_BYTE *BLUE_LPBYTE ;
/**
 * Represents an unsigned 8 bit character
 */
typedef unsigned char BLUE_UCHAR ;
/**
 * Represents a generic VOID 
 */
typedef void BLUE_VOID ;
/**
 * Represents a generic long value
 */
typedef long BLUE_LONG ;
/**
 * Represents a pointer to a generic long value
 */
typedef BLUE_LONG *BLUE_PLONG ;
/**
 * Represents a generic short value
 */
typedef short BLUE_SHORT ;
/**
 * Represents a 32 bit unsigned integer
 */
#if defined(BLUE_PARAM_LONGINT_64)
typedef unsigned int BLUE_UINT32 ;
#else
typedef unsigned long int BLUE_UINT32 ;
#endif
/**
 * Represents a 16 bit unsigned integer
 */
typedef unsigned short int BLUE_UINT16 ;
/**
 * Represents an 8 bit unsigned integer
 */
typedef unsigned char BLUE_UINT8 ;

#if defined(BLUE_PARAM_64BIT_INTEGER)
/**
 * Represents an unsigned 64 bit value
 */
typedef unsigned long long int BLUE_UINT64 ;
/**
 * Represents a signed 64 bit value
 */
typedef long long int BLUE_INT64 ;
#else
/**
 * Represents a signed 64 bit value
 */
typedef struct
{
  /**
   * The low order 32 bits
   */
  BLUE_UINT32 low ;
  /**
   * The high order 32 bits
   */
  BLUE_INT32 high ;
} BLUE_INT64 ;
/**
 * Represents an unsigned 64 bit value
 */
typedef struct
{
  /**
   * The low order 32 bits
   */
  BLUE_UINT32 low ;
  /**
   * The high order 32 bits
   */
  BLUE_UINT32 high ;
} BLUE_UINT64 ;

#endif

/**
 * Represents a generic large integer
 */
typedef BLUE_INT64 BLUE_LARGE_INTEGER ;

#if defined(BLUE_PARAM_64BIT_INTEGER)
/**
 * Return the high order 32 bits of a large integer
 */
#define BLUE_LARGE_INTEGER_HIGH(x) ((BLUE_UINT32)((x)>>32))
/**
 * Return the low order 32 bits of a large integer
 */
#define BLUE_LARGE_INTEGER_LOW(x) ((BLUE_UINT32)((x)&0xFFFFFFFF))
/**
 * Assign one large integer to another
 */
#define BLUE_LARGE_INTEGER_ASSIGN(x,y) x=y
/**
 * Initialize a large integer from two 32 bit integers
 */
#define BLUE_LARGE_INTEGER_SET(x, y, z) (x)=(BLUE_LARGE_INTEGER)(z)<<32|(y)
/*
 * Compare two large integers
 */
#define BLUE_LARGE_INTEGER_EQUAL(x,y) (x==y)

#define BLUE_LARGE_INTEGER_INCR(x) x++

#define BLUE_LARGE_INTEGER_AND(x, y, z) (x)&=((BLUE_LARGE_INTEGER)(z)<<32|(y))

#define BLUE_LARGE_INTEGER_INIT(x,y) (BLUE_LARGE_INTEGER)(y)<<32|(x)

#else
/**
 * Return the high order 32 bits of a large integer
 */
#define BLUE_LARGE_INTEGER_HIGH(x) ((x).high)
/**
 * Return the low order 32 bits of a large integer
 */
#define BLUE_LARGE_INTEGER_LOW(x) ((x).low)
/**
 * Assign one large integer to another
 */
#define BLUE_LARGE_INTEGER_ASSIGN(x,y) BlueCmemcpy(&x,&y, sizeof(BLUE_LARGE_INTEGER))
/**
 * Initialize a large integer from two 32 bit integers
 */
#define BLUE_LARGE_INTEGER_SET(x, y, z) {(x).low = (y); (x).high=(z);}

#define BLUE_LARGE_INTEGER_INIT(x,y) {x; y}

/*
 * Compare two large integers
 */
#define BLUE_LARGE_INTEGER_EQUAL(x,y) ((x).high==(y).high && (x).low==(y).low)
/*
 * Increment
 */
#define BLUE_LARGE_INTEGER_INCR(x) ((x).low==0xFFFFFFFF?\
				    (x).high++,(x).low=0:(x).low++)

#define BLUE_LARGE_INTEGER_AND(x, y, z) {(x).low &= (y); (x).high&=(z);}

//#define BLUE_LARGE_INTEGER_ADD(x,y,z) ((z).low=(x).low+(y).low,(z).high=(x).high+(y).high,(z).low<(x).low||(z).low<(y).low?(z).high++) 

#endif

/**
 * Represents an offset into a structure
 */
typedef BLUE_LARGE_INTEGER BLUE_OFFT ;
/**
 * Represents a generic unsigned long value
 */
typedef unsigned long BLUE_ULONG ;
/**
 * Represents the maximum unsigned long value
 */
#define	BLUE_ULONG_MAX ((BLUE_ULONG)(~0L))
/**
 * Represents the maximum positive long value
 */
#define	BLUE_LONG_MAX ((BLUE_LONG)(BLUE_ULONG_MAX >> 1))
/**
 * Represents the minimum negative long value
 */
#define	BLUE_LONG_MIN ((BLUE_LONG)(~BLUE_LONG_MAX))
/**
 * Represents a pointer to an Unsigned Long
 */
typedef BLUE_ULONG *BLUE_LPULONG ;

#if defined(BLUE_PARAM_64BIT_POINTER)
/**
 * Represents a pointer to a 64 bit value
 */
typedef BLUE_UINT64 BLUE_ULONG_PTR ;
#else
/**
 * Represents a pointer to a 64 bit value
 */
typedef BLUE_UINT32 BLUE_ULONG_PTR ;
#endif

/**
 * Represents a void constant
 */
typedef const BLUE_VOID BLUE_CVOID ;
/**
 * Represents a pointer to a wide character string
 */
typedef BLUE_WCHAR *BLUE_LPWSTR ;
/**
 * Another representation of a pointer to a wide character string
 */
typedef BLUE_WCHAR *BLUE_LMSTR ;
/**
 * Represents a constant wide character
 */
typedef const BLUE_WCHAR BLUE_CWCHAR ;
/**
 * Represents a pointer to a constant wide character string
 */
typedef const BLUE_WCHAR *BLUE_LPCWSTR ;
/**
 * Represents a constant character
 */
typedef const BLUE_CHAR BLUE_CCHAR ;
/**
 * Represents a pointer to a character string
 */
typedef BLUE_CHAR *BLUE_LPSTR ;
/**
 * Represents a pointer to a constant character string
 */
typedef const BLUE_CHAR *BLUE_LPCSTR ;
/**
 * Represents a Generic WORD
 */
typedef BLUE_UINT16 BLUE_WORD ;
/**
 * Represents a generic double word
 */
typedef BLUE_UINT32 BLUE_DWORD ;
/**
 * The mask to associate with a double word
 */
#define BLUE_DWORD_MASK 0xFFFFFFFF 
/**
 * Represents a pointer to an array of double words
 */
typedef BLUE_DWORD *BLUE_LPDWORD ;
#if defined(BLUE_PARAM_64BIT_POINTER)
/**
 * Represents a pointer to a double word
 */
typedef BLUE_UINT64 BLUE_DWORD_PTR ;
#else
/**
 * Represents a pointer to a double word
 */
typedef BLUE_UINT32 BLUE_DWORD_PTR ;
#endif
/**
 * Represents a pointer to a void element
 */
typedef BLUE_VOID *BLUE_LPVOID ;
/**
 * Another representation of a pointer to a void element
 */
typedef BLUE_VOID *BLUE_PVOID ;
/**
 * Represents a pointer to a constant element
 */
typedef const BLUE_VOID *BLUE_LPCVOID ;

/**
 * Represents a wide character
 */
typedef BLUE_WCHAR BLUE_TCHAR ;
/**
 * Represents a wide character constant
 */
typedef const BLUE_WCHAR BLUE_CTCHAR ;

#if defined(BLUE_PARAM_UNICODE_API)
/**
 * Represents a wide character
 */
typedef BLUE_TCHAR BLUE_TACHAR ;
/**
 * Represents a wide character constant
 */
typedef const BLUE_TCHAR BLUE_CTACHAR ;
#else
/**
 * Represents a wide character
 */
typedef BLUE_CHAR BLUE_TACHAR ;
/**
 * Represents a wide character constant
 */
typedef const BLUE_CHAR BLUE_CTACHAR ;
#endif

/**
 * Represents a Pointer to a wide character string
 */
typedef BLUE_TCHAR *BLUE_LPTSTR ;
typedef BLUE_TACHAR *BLUE_LPTASTR ;
/**
 * Represents a pointer to a wide character constant string
 */
typedef const BLUE_TCHAR *BLUE_LPCTSTR ;
typedef const BLUE_TACHAR *BLUE_LPCTASTR ;
/**
 * Represents a Millisecond time value
 */
typedef BLUE_INT32 BLUE_MSTIME ;

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
#define TSTR(x) (const BLUE_TCHAR *) L##x
/**
 * A wide character
 */
#define TCHAR(x) (const BLUE_TCHAR) L##x

#if defined(BLUE_PARAM_UNICODE_API)
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
#define BLUE_UUID_LEN 16
/**
 * Represents a UUID
 */
typedef BLUE_UCHAR BLUE_UUID[BLUE_UUID_LEN] ;

/**
 * Represents a Boolean value
 *
 * This shoudl be a 8 bit value on a windows system to be compatable with
 * Windows BOOL
 */
typedef BLUE_UINT8 BLUE_BOOL ;
enum
  {
    /**
     * The value is false
     */
    BLUE_FALSE = 0,
    /**
     * The value is true
     */
    BLUE_TRUE = 1
  } ;

/**
 * A NULL Pointer
 */
#define BLUE_NULL ((BLUE_LPVOID) 0)
/**
 * An IO Vector used by SASL
 */
typedef struct _iovec {
    BLUE_LONG iov_len;
    BLUE_CHAR *iov_base;
} BLUE_IOVEC ;



/**
 * Container of macro
 */
#define container_of(ptr,type,member) \
  (type *)((BLUE_CHAR *)(ptr)-(BLUE_CHAR *)&((type *)0)->member)

#endif

/** \} */
