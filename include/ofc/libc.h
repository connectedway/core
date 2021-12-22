/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__BLUE_C_H__)
#define __BLUE_C_H__

#include <stdarg.h>

#include "ofc/core.h"
#include "ofc/types.h"
#if defined(DEBUG_FUNCTION_CALLS)
#define DBG_ENTRY() BlueCprintf("Function %s Entry\n", __FUNCTION__)
#define DBG_EXIT() BlueCprintf("Function %s Exit\n", __FUNCTION__)
#else
#define DBG_ENTRY()
#define DBG_EXIT() 
#endif

/**
 * Determine if a character is a whitespace
 *
 * \param c
 * The character to test
 *
 * \returns
 * BLUE_TRUE if whitespace, BLUE_FALSE otherwise.
 */
#define BLUE_C_ISSPACE(c) (c == ' ' || c == '\f' || c == '\n' || \
			   c == '\r' || c == '\t' || c == '\v') 
/**
 * Determine if a wide character is a whitespace
 *
 * \param c
 * The wide character to test
 *
 * \returns
 * BLUE_TRUE if whitespace, BLUE_FALSE otherwise.
 */
#define BLUE_C_TISSPACE(c) (c == TCHAR(' ') || c == TCHAR('\f') || \
			    c == TCHAR('\n') || c == TCHAR('\r') || \
			    c == TCHAR('\t') || c == TCHAR('\v')) 
/**
 * Determine if a character is a digit (0..9)
 *
 * \param c
 * The character to test
 *
 * \returns
 * BLUE_TRUE if a digit, BLUE_FALSE otherwise
 */
#define BLUE_C_ISDIGIT(c) (c >= '0' && c <= '9') 
/**
 * Determine if a wide character is a digit (0..9)
 *
 * \param c
 * The character to test
 *
 * \returns
 * BLUE_TRUE if a digit, BLUE_FALSE otherwise
 */
#define BLUE_C_TISDIGIT(c) (c >= TCHAR('0') && c <= TCHAR('9')) 
/**
 * Determine if a character is alphanumeric (upper or lower case)
 *
 * \param c
 * The character to test
 *
 * \returns
 * BLUE_TRUE if alphanumeric, BLUE_FALSE otherwise
 */
#define BLUE_C_ISALPHA(c) {BLUE_C_ISUPPER((c)) || BLUE_C_ISLOWER((c)))
#define BLUE_C_ISALNUM(c) (BLUE_C_ISUPPER(c)||BLUE_C_ISLOWER(c)||BLUE_C_ISDIGIT(c))
/**
 * Determine if a wide character is alphanumeric (upper or lower case)
 *
 * \param c
 * The character to test
 *
 * \returns
 * BLUE_TRUE if alphanumeric, BLUE_FALSE otherwise
 */
#define BLUE_C_TISALPHA(c) {BLUE_C_TISUPPER(c) || BLUE_C_TISLOWER(c))
/**
 * Determine if character is upper case (A..Z)
 *
 * \param c
 * character to test
 *
 * \returns
 * BLUE_TRUE if upper case, BLUE_FALSE otherwise
 */
#define BLUE_C_ISUPPER(c) (c >=  'A' && c <= 'Z')
/**
 * Determine if wide character is upper case (A..Z)
 *
 * \param c
 * character to test
 *
 * \returns
 * BLUE_TRUE if upper case, BLUE_FALSE otherwise
 */
#define BLUE_C_TISUPPER(c) (c >=  TCHAR('A') && c <= TCHAR('Z'))
/**
 * Convert a lower case character to upper case
 *
 * If the character is not a lower case character, it is left unchanged
 *
 * \param c
 * character to convert
 *
 * \returns
 * converted character
 */
#define BLUE_C_TOUPPER(c) (BLUE_C_ISLOWER(c)?c-('a'-'A'):c)
/**
 * Convert an upper case character to lower case
 *
 * If the character is not an upper case character, it is left unchanged
 *
 * \param c
 * character to convert
 *
 * \returns
 * converted character
 */
#define BLUE_C_TOLOWER(c) (BLUE_C_ISUPPER(c)?c+('a'-'A'):c)
/**
 * Convert a lower case wide character to upper case wide character
 *
 * If the character is not a lower case character, it is left unchanged
 *
 * \param c
 * character to convert
 *
 * \returns
 * converted character
 */
#define BLUE_C_TTOUPPER(c) (BLUE_C_TISLOWER(c)?c-(TCHAR('a')-TCHAR('A')):c)
/**
 * Determine if character is lower case (a..z)
 *
 * \param c
 * Character to test
 *
 * \returns
 * BLUE_TRUE if lower case, BLUE_FALSE otherwise
 */
#define BLUE_C_ISLOWER(c) (c >=  'a' && c <= 'z')
/**
 * Determine if wide character is lower case (a..z)
 *
 * \param c
 * Character to test
 *
 * \returns
 * BLUE_TRUE if lower case, BLUE_FALSE otherwise
 */
#define BLUE_C_TISLOWER(c) (c >=  TCHAR('a') && c <= TCHAR('z'))
/**
 * Return the minimum of two values
 *
 * \param a
 * First parameter to compare
 *
 * \param b
 * Second parameter to compare
 *
 * \returns
 * Minimum of a or b
 */
#define BLUE_C_MIN(a, b) ((a) < (b) ? (a) : (b))
/**
 * Return the maximum of two values
 *
 * \param a
 * First parameter to compare
 *
 * \param b
 * Second parameter to compare
 *
 * \returns
 * maximum of a or b
 */
#define BLUE_C_MAX(a, b) ((a) > (b) ? (a) : (b))

#define UUID_STR_LEN 36

#if defined(__cplusplus)
extern "C"
{
#endif
  /**
   * Compare two NULL terminated strings
   *
   * \param astr
   * Pointer to the first string
   *
   * \param bstr
   * Pointer to the second string
   *
   * \returns
   * a positive or negative integer or 0.  A positive result implies that
   * the first string is greater then the second string.  A negative result
   * implies the first string is less then the second string.  And a zero 
   * result implies both strings are identical.
   */
  BLUE_CORE_LIB BLUE_INT 
  BlueCstrcmp (BLUE_CCHAR *astr, BLUE_CCHAR *bstr) ;
  /**
   * Compare two NULL terminated strings ignoring the case of the characters
   *
   * \param astr
   * Pointer to the first string
   *
   * \param bstr
   * Pointer to the second string
   *
   * \returns
   * a positive or negative integer or 0.  A positive result implies that
   * the first string is greater then the second string.  A negative result
   * implies the first string is less then the second string.  And a zero 
   * result implies both strings are identical.
   */
  BLUE_CORE_LIB BLUE_INT 
  BlueCstrcasecmp (BLUE_CCHAR *astr, BLUE_CCHAR *bstr) ;
  /**
   * Compare two NULL terminated strings, restricting the comparison to the
   * specified length
   *
   * \param astr
   * Pointer to the first string
   *
   * \param bstr
   * Pointer to the second string
   *
   * \param len
   * Number of characters to compare
   *
   * \returns
   * a positive or negative integer or 0.  A positive result implies that
   * the first string is greater then the second string.  A negative result
   * implies the first string is less then the second string.  And a zero 
   * result implies both strings are identical.
   */
  BLUE_CORE_LIB BLUE_INT 
  BlueCstrncmp (BLUE_CCHAR *astr, BLUE_CCHAR *bstr, BLUE_SIZET len) ;
  /**
   * Compare two NULL terminated strings, restricting the comparison to the
   * specified length.  Ignore the case of the characters in the string.
   *
   * \param astr
   * Pointer to the first string
   *
   * \param bstr
   * Pointer to the second string
   *
   * \param len
   * Number of characters to compare
   *
   * \returns
   * a positive or negative integer or 0.  A positive result implies that
   * the first string is greater then the second string.  A negative result
   * implies the first string is less then the second string.  And a zero 
   * result implies both strings are identical.
   */
  BLUE_CORE_LIB BLUE_INT 
  BlueCstrncasecmp (BLUE_CCHAR *astr, BLUE_CCHAR *bstr, BLUE_SIZET len) ;
  /**
   * Find the length of a string
   *
   * \param astr
   * Pointer to the string to measure
   *
   * \returns
   * The size of the string not including the terminating NIL
   */
  BLUE_CORE_LIB BLUE_SIZET 
  BlueCstrlen (BLUE_CCHAR *astr) ;
  /**
   * Find the length of a string, but restrict the scan to the specified 
   * length.  If no end of string has been found by the specified length, 
   * the maximum length is returned.
   *
   * \param astr
   * Pointer to the string to measure
   *
   * \param len
   * The maximum number of characters to count
   *
   * \returns
   * The size of the string not including the terminating NIL
   */
  BLUE_CORE_LIB BLUE_SIZET 
  BlueCstrnlen (BLUE_CCHAR *astr, BLUE_SIZET len) ;
  /**
   * Duplicate a string
   *
   * \param astr
   * pointer to the string to duplicate
   *
   * \returns
   * Pointer to a copy of the original string.  This string includes the
   * terminating NIL.  The returned memory must be freed by the calling
   * application when it is no longer necessary.
   */
  BLUE_CORE_LIB BLUE_CHAR *
  BlueCstrdup (BLUE_CCHAR *astr) ;
  /**
   * Duplicate a string, but restrict by len
   *
   * \param astr
   * pointer to the string to duplicate
   *
   * \param len
   * Maximim size of string to duplicate
   *
   * \returns
   * Pointer to a copy of the original string.  This string includes the
   * terminating NIL.  The returned memory must be freed by the calling
   * application when it is no longer necessary.
   */
  BLUE_CORE_LIB BLUE_CHAR *
  BlueCstrndup (BLUE_CCHAR *astr, BLUE_SIZET len) ;
  BLUE_CORE_LIB BLUE_TCHAR *
  BlueCtstrndup (BLUE_CTCHAR *astr, BLUE_SIZET len) ;
  /**
   * Copy a source string to a destination string
   *
   * \param dst
   * Pointer to the destination string
   *
   * \param src
   * Pointer to the source string
   *
   * \returns
   * A pointer to the destination string
   *
   * \remarks
   * It is the caller's responsibility to insure that the destination string
   * is large enough to hold the entire contents of the source string 
   * including the terminating NIL.  If not, memory corruption will occur.
   */
  BLUE_CORE_LIB BLUE_CHAR *
  BlueCstrcpy (BLUE_CHAR *dst, BLUE_CCHAR *src) ;
  BLUE_CORE_LIB BLUE_CHAR *
  BlueCstrcat (BLUE_CHAR *dst, BLUE_CCHAR *src) ;
  BLUE_CORE_LIB BLUE_CHAR *
  BlueCstrncat (BLUE_CHAR *dst, BLUE_CCHAR *src, BLUE_SIZET size) ;
  /**
   * Copy a unicode source string to a destination string
   *
   * \param dst
   * Pointer to the destination string
   *
   * \param src
   * Pointer to the source unicode string
   *
   * \returns
   * A pointer to the destination unicode string
   *
   * \remarks
   * It is the caller's responsibility to insure that the destination string
   * is large enough to hold the entire contents of the source string 
   * including the terminating NIL.  If not, memory corruption will occur.
   */
  BLUE_CORE_LIB BLUE_TCHAR *
  BlueCtstrcpy (BLUE_TCHAR *dst, BLUE_CTCHAR *src) ;
  /**
   * Convert a string to upper case
   *
   * \param s
   * Pointer to string to convert
   *
   * \param len
   * Maximum number of characters to convert
   *
   * \returns
   * Pointer to converted string.  The pointer is the same as that passed in 
   * since the operation is perfmed in place.
   *
   * \remarks
   * The strupr() function will convert a string to upper case.  Only the 
   * lower case alphabetic characters [a .. z] are converted.  Non-alphabetic 
   * characters will not be changed
   */
  BLUE_CHAR *BlueCstrnupr (BLUE_CHAR *s, BLUE_SIZET len) ;
  BLUE_TCHAR *BlueCtstrnupr (BLUE_TCHAR *src, BLUE_SIZET len) ;
  /**
   * Copy a source string to a destination string where the size of the
   * destination string is given.
   *
   * \param dst
   * Pointer to the destination string
   *
   * \param src
   * Pointer to the source string
   *
   * \param len
   * size of the destination string
   *
   * \returns
   * A pointer to the destination string
   *
   * \remarks
   * No more then number of characters specified as the destination length will
   * be copied to the destination string.  The terminating NIL will not be
   * copied, unless the destination string is one byte larger then the
   * source string and the source string is terminated by a NIL.
   */
  BLUE_CORE_LIB BLUE_CHAR *
  BlueCstrncpy (BLUE_CHAR *dst, BLUE_CCHAR *src, BLUE_SIZET len) ;
  /**
   * Copy a source unicode string to a destination unicode string where the 
   * size of the destination string is given.
   *
   * \param dst
   * Pointer to the destination string
   *
   * \param src
   * Pointer to the source string
   *
   * \param len
   * size of the destination string
   *
   * \returns
   * A pointer to the destination string
   *
   * \remarks
   * No more then number of characters specified as the destination length will
   * be copied to the destination string.  The terminating NIL will not be
   * copied, unless the destination string is one byte larger then the
   * source string and the source string is terminated by a NIL.
   */
  BLUE_CORE_LIB BLUE_TCHAR *
  BlueCtstrncpy (BLUE_TCHAR *dst, BLUE_CTCHAR *src, BLUE_SIZET len) ;
  /**
   * Return the length of a path
   *
   * \param str
   * Pointer to a string.  Either UTF-16 or UTF-8
   *
   * \returns
   * size of string
   */
  BLUE_CORE_LIB BLUE_SIZET 
  BlueCtstrlen (BLUE_LPCTSTR str) ;
  /**
   * Return the length of a path, restricting the scan to the specified length.
   * If no EOS is found by the specified length, the maximum length is 
   * returned.
   *
   * \param str
   * Pointer to a string.  Either UTF-16 or UTF-8
   *
   * \param len
   * The maximum number of characters to count
   *
   * \returns
   * size of string
   */
  BLUE_CORE_LIB BLUE_SIZET 
  BlueCtstrnlen (BLUE_LPCTSTR str, BLUE_SIZET len) ;
  /**
   * Duplicate a path
   *
   * \param str
   * Pointer to string to duplicate (either UTF-16 or UTF-8)
   *
   * \returns
   * Duplicated string (UTF-16 or UTF-8)
   */
  BLUE_CORE_LIB BLUE_LPTSTR 
  BlueCtstrdup (BLUE_LPCTSTR str) ;
  /**
   * Compare two path strings
   *
   * \param astr
   * A pointer to the first string
   *
   * \param bstr
   * A pointer to the second string
   *
   * \returns
   * <0 if A < b, =0 if a == b, >0 if a > b
   */
  BLUE_CORE_LIB BLUE_INT 
  BlueCtstrcmp (BLUE_LPCTSTR astr, BLUE_LPCTSTR bstr) ;
  /**
   * Compare two NULL terminated TSTRS, restricting the comparison to the
   * specified length
   *
   * \param astr
   * Pointer to the first string (UTF-16 or UTF-8)
   *
   * \param bstr
   * Pointer to the second string
   *
   * \returns
   * a positive or negative integer or 0.  A positive result implies that
   * the first string is greater then the second string.  A negative result
   * implies the first string is less then the second string.  And a zero 
   * result implies both strings are identical.
   */
  BLUE_CORE_LIB BLUE_INT 
  BlueCtstrcasecmp (BLUE_LPCTSTR astr, BLUE_LPCTSTR bstr) ;
  /**
   * Compare two NULL terminated TSTRs, restricting the comparison to the
   * specified length.  Ignore the case of the characters in the string.
   *
   * \param astr
   * Pointer to the first string (UTF-16 or UTF-8)
   *
   * \param bstr
   * Pointer to the second string (UTF-16 or UTF-8)
   *
   * \param len
   * Number of characters to compare
   *
   * \returns
   * a positive or negative integer or 0.  A positive result implies that
   * the first string is greater then the second string.  A negative result
   * implies the first string is less then the second string.  And a zero 
   * result implies both strings are identical.
   */
  BLUE_CORE_LIB BLUE_INT 
  BlueCtstrncasecmp (BLUE_CTCHAR *astr, BLUE_CTCHAR *bstr, BLUE_SIZET len) ;
  /**
   * Scan a TSTR looking for a match on a character
   *
   * \param str
   * The string to scan
   *
   * \param terms
   * A string containing a list of characters to check for
   *
   * \returns
   * A pointer to first character matched.  If no character is matched,
   * a pointer to the end of string is returned.
   */
  BLUE_CORE_LIB BLUE_LPCTSTR 
  BlueCtstrtok (BLUE_LPCTSTR str, BLUE_LPCTSTR terms) ;

  /**
   * Scan a ascii string looking for a match on a character
   *
   * \param str
   * The string to scan
   *
   * \param terms
   * A string containing a list of characters to check for
   *
   * \returns
   * A pointer to first character matched.  If no character is matched,
   * a pointer to the end of string is returned.
   */
  BLUE_CORE_LIB BLUE_LPSTR 
  BlueCstrtok (BLUE_LPCSTR str, BLUE_LPCSTR terms) ;
  BLUE_CORE_LIB BLUE_LPSTR
  BlueCstrchr (BLUE_LPCSTR str, BLUE_CCHAR c) ;
  BLUE_CORE_LIB BLUE_LPSTR
  BlueCmemchr (BLUE_LPCSTR str, BLUE_CCHAR c, BLUE_SIZET size) ;
  /**
   * Scan a TSTR starting at the end and going to the beginning
   *
   * \param str
   * The string to scan
   *
   * \param terms
   * A string containing a list of characters to check for
   *
   * \returns
   * A pointer to first character matched.  If no character is matched,
   * a pointer to the beginning of the string is returned.
   */
  BLUE_CORE_LIB BLUE_LPCTSTR 
  BlueCtstrrtok (BLUE_LPCTSTR str, BLUE_LPCTSTR terms) ;
  /**
   * Scan a string starting at the end and going to the beginning
   *
   * \param str
   * The string to scan
   *
   * \param terms
   * A string containing a list of characters to check for
   *
   * \returns
   * A pointer to first character matched.  If no character is matched,
   * a pointer to the beginning of the string is returned.
   */
  BLUE_CORE_LIB BLUE_LPCSTR 
  BlueCstrrtok (BLUE_LPCSTR str, BLUE_LPCSTR terms) ;
  /**
   * Compare two NULL terminated TSTRs, restricting the comparison to the
   * specified length
   *
   * \param astr
   * Pointer to the first string
   *
   * \param bstr
   * Pointer to the second string
   *
   * \param len
   * Number of characters to compare
   *
   * \returns
   * a positive or negative integer or 0.  A positive result implies that
   * the first string is greater then the second string.  A negative result
   * implies the first string is less then the second string.  And a zero 
   * result implies both strings are identical.
   */
  BLUE_CORE_LIB BLUE_INT 
  BlueCtstrncmp (BLUE_CTCHAR *astr, BLUE_CTCHAR *bstr, BLUE_SIZET len) ;
  /**
   * Convert a TSTR to a normal C string
   *
   * This assumes the TSTR is a UTF-16 character string (wide characters).
   * This will not convert other types of Unicode strings.
   *
   * \param str
   * The UTF-16 string to convert
   *
   * \returns
   * Pointer to a UTF-8 character string.
   *
   * \note
   * The character string returned has been allocated from the heap.  It must
   * be freed by a call to BlueHeapFree
   */
  BLUE_CORE_LIB BLUE_CHAR *
  BlueCtstr2cstr (BLUE_LPCTSTR str) ;
  /**
   * Convert a normal C string to a TSTR.
   *
   * This assumes the TSTR is a UTF-16 character string (wide characters).
   * This will not convert other types of Unicode strings.
   *
   * \param str
   * The UTF-8 string to convert
   *
   * \returns
   * Pointer to a UTF-16 character string.
   *
   * \note
   * The character string returned has been allocated from the heap.  It must
   * be freed by a call to BlueHeapFree
   */
  BLUE_CORE_LIB BLUE_LPTSTR 
  BlueCcstr2tstr (BLUE_CCHAR *str)  ;
  /**
   * Copy a chunk of memory to a different location
   *
   * \param out
   * The destination memory
   *
   * \param in
   * Pointer to the source memory
   *
   * \param size
   * The number of bytes to transfer
   *
   * \returns
   * Nothing
   */
  BLUE_CORE_LIB BLUE_LPVOID 
  BlueCmemcpy (BLUE_LPVOID out, BLUE_LPCVOID in, BLUE_SIZET size) ;
  /**
   * Compare two strings
   *
   * \param a
   * One of the strings to compare
   *
   * \param b
   * The other string to compare
   *
   * \param size
   * The number of bytes to compare
   *
   * \returns
   * < 0 if a < b, 0 if a = b, > 0 if a > b
   */
  BLUE_CORE_LIB BLUE_INT 
  BlueCmemcmp (BLUE_LPCVOID a, BLUE_LPCVOID b, BLUE_SIZET size) ;
  /**
   * Set all bytes in a block of memory to a value
   *
   * \param dst
   * The location of the memory to set
   *
   * \param c
   * The value to set it to
   *
   * \param size
   * The number of bytes to set
   *
   * \returns
   * Nothing
   */
  BLUE_CORE_LIB BLUE_LPVOID 
  BlueCmemset (BLUE_LPVOID dst, BLUE_INT c, BLUE_SIZET size) ;
  /**
   * Convert an integer string to a long value
   *
   * \param str
   * Pointer to the integer string
   *
   * \param upd
   * Pointer to the where to store the address of the first character
   * not part of the integer string.
   *
   * \param base
   * The base of the integer string.  For instance 2 for binary, 8 for octal,
   * 10 for decimal or 16 for hex.
   *
   * \returns
   * The long integer value of the integer string
   */
  BLUE_CORE_LIB BLUE_LONG 
  BlueCstrtol (const BLUE_CHAR *str, BLUE_CHAR **upd, BLUE_INT base) ;
  /**
   * Convert an integer string to a unsigned long value
   *
   * \param string
   * Pointer to the unsigned integer string
   *
   * \param endPtr
   * Pointer to the where to store the address of the first character
   * not part of the integer string.
   *
   * \param base
   * The base of the integer string.  For instance 2 for binary, 8 for octal,
   * 10 for decimal or 16 for hex.
   *
   * \returns
   * The unsigned long integer value of the integer string
   */
  BLUE_CORE_LIB BLUE_ULONG 
  BlueCstrtoul (const BLUE_CHAR *string, BLUE_CHAR **endPtr, BLUE_INT base) ;
  /**
   * Formatted string output
   *
   * \param str
   * Pointer to destination string
   *
   * \param count
   * Size of the destination string
   * 
   * \param fmt
   * Pointer to the format string.  Typical printf format strings
   * are accepted.
   *
   * \param args
   * Variable argument list matching respective format characters in the
   * format string
   *
   * \returns
   * size of the formatted output.
   *
   * \remarks
   * The return size represents the number of characters that either have been
   * output or would have been output if the destination string were large
   * enough.  By calling BlueCvsnprintf with a 0 count and a NULL destination
   * string, the required size of the string will be returned (not including 
   * a terminating NIL.  This return value can be used to allocate a 
   * destination string and the BlueCvsnprintf can be called again with the
   * allocated memory block and size of that block.
   */
  BLUE_CORE_LIB BLUE_SIZET 
  BlueCvsnprintf (BLUE_CHAR *str, BLUE_SIZET count, 
		  BLUE_CCHAR *fmt, va_list args) ;
  /**
   * Formatted output
   *
   * \param str
   * pointer to the destination string
   *
   * \param count
   * size of the destination string
   *
   * \param fmt
   * The format string.  Typical C printf format strings are supported.
   *
   * \returns
   * size of the formatted output.
   *
   * \remark
   * This call takes any number of arguments each related to the respective
   * format character in the format string. \see BlueCvsnprintf
   */
  BLUE_CORE_LIB BLUE_SIZET 
  BlueCsnprintf(BLUE_CHAR *str, BLUE_SIZET count, BLUE_CCHAR *fmt,...) ;
  /**
   * Perform formated output to the system console
   *
   * \param fmt
   * The format string
   *
   * \remark
   * This call takes any number of arguments each related to the respective
   * format character in the format string. \see BlueCvsnprintf
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueCprintf(BLUE_CCHAR *fmt,...) ;

  BLUE_CORE_LIB BLUE_CHAR *
  BlueCsaprintf(BLUE_CCHAR *fmt,...) ;
  /**
   * Perform formated output to a trace buffer
   *
   * \param fmt
   * The format string
   *
   * \remark
   * This call takes any number of arguments each related to the respective
   * format character in the format string. \see BlueCvsnprintf.  This
   * call is a lot like BlueCprintf but the output goes to a trace buffer
   * instead.
   */
#if 0
  /**
   * Initialize the C Runtime Library
   *
   * The routine is called by the system startup code to initialize the 
   * C runtime.  This should only be called from within BlueInit
   */
  BLUE_CORE_LIB BLUE_VOID 
  BlueTraceInit (BLUE_VOID) ;
  BLUE_CORE_LIB BLUE_VOID 
  BlueTraceDestroy (BLUE_VOID);
  BLUE_CORE_LIB BLUE_VOID 
  BlueCTrace (BLUE_CCHAR *fmt,...) ;
  BLUE_CORE_LIB BLUE_VOID 
  BlueCDumpTrace(BLUE_VOID) ;
#else
#define BlueTraceInit()
#define BlueTraceDestroy()
#define BlueCTrace(fmt, ...)
#define BlueCDumpTrace()
#endif

  BLUE_VOID BlueCuuidtoa (BLUE_UUID uuid, BLUE_CHAR *out) ;

  BLUE_VOID BlueCatouuid (const BLUE_CHAR *in, BLUE_UUID uuid) ;

#if defined(__cplusplus)
}
#endif

#if defined(BLUE_PARAM_UNICODE_API)
#define BlueCtastrcpy(dst,src) BlueCtstrcpy(dst,src)
#define BlueCtastrncpy(dst,src,len) BlueCtstrncpy(dst,src,len)
#define BlueCtastrlen(str) BlueCtstrlen(str)
#define BlueCtastrnlen(str,len) BlueCtstrnlen(str,len)
#define BlueCtastrdup(str) (BLUE_LPTASTR)BlueCtstrdup(str)
#define BlueCtastrcmp(astr,bstr) BlueCtstrcmp(astr,bstr)
#define BlueCtastrcasecmp(astr,bstr) BlueCtstrcasecmp(astr,bstr)
#define BlueCtastrncasecmp(astr,bstr,len) BlueCtstrncasecmp(astr,bstr,len)
#define BlueCtastrtok(str,terms) BlueCtstrtok(str,terms)
#define BlueCtastrrtok(str,terms) BlueCtstrrtok(str,terms)
#define BlueCtastrncmp(astr,bstr,len) BlueCtstrncmp(astr,bstr,len)
#define BlueCtastr2cstr(str) BlueCtstr2cstr(str)
#define BlueCcstr2tastr(str) BlueCcstr2tstr(str)
#else
#define BlueCtastrcpy(dst,src) BlueCstrcpy(dst,src)
#define BlueCtastrncpy(dst,src,len) BlueCstrncpy(dst,src,len)
#define BlueCtastrlen(str) BlueCstrlen(str)
#define BlueCtastrnlen(str,len) BlueCstrnlen(str,len)
#define BlueCtastrdup(str) BlueCstrdup(str)
#define BlueCtastrcmp(astr,bstr) BlueCstrcmp(astr,bstr)
#define BlueCtastrcasecmp(astr,bstr) BlueCstrcasecmp(astr,bstr)
#define BlueCtastrncasecmp(astr,bstr,len) BlueCstrncasecmp(astr,bstr,len)
#define BlueCtastrtok(str,terms) BlueCstrtok(str,terms)
#define BlueCtastrrtok(str,terms) BlueCstrrtok(str,terms)
#define BlueCtastrncmp(astr,bstr,len) BlueCstrncmp(astr,bstr,len)
#define BlueCtastr2cstr(str) BlueCstrdup(str)
#define BlueCcstr2tastr(str) BlueCstrdup(str)
#endif

#endif

