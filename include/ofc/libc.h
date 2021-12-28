/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_C_H__)
#define __OFC_C_H__

#include <stdarg.h>

#include "ofc/core.h"
#include "ofc/types.h"
#if defined(DEBUG_FUNCTION_CALLS)
#define DBG_ENTRY() ofc_printf("Function %s Entry\n", __FUNCTION__)
#define DBG_EXIT() ofc_printf("Function %s Exit\n", __FUNCTION__)
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
 * OFC_TRUE if whitespace, OFC_FALSE otherwise.
 */
#define OFC_ISSPACE(c) (c == ' ' || c == '\f' || c == '\n' || \
			   c == '\r' || c == '\t' || c == '\v') 
/**
 * Determine if a wide character is a whitespace
 *
 * \param c
 * The wide character to test
 *
 * \returns
 * OFC_TRUE if whitespace, OFC_FALSE otherwise.
 */
#define OFC_TISSPACE(c) (c == TCHAR(' ') || c == TCHAR('\f') || \
			    c == TCHAR('\n') || c == TCHAR('\r') || \
			    c == TCHAR('\t') || c == TCHAR('\v')) 
/**
 * Determine if a character is a digit (0..9)
 *
 * \param c
 * The character to test
 *
 * \returns
 * OFC_TRUE if a digit, OFC_FALSE otherwise
 */
#define OFC_ISDIGIT(c) (c >= '0' && c <= '9') 
/**
 * Determine if a wide character is a digit (0..9)
 *
 * \param c
 * The character to test
 *
 * \returns
 * OFC_TRUE if a digit, OFC_FALSE otherwise
 */
#define OFC_TISDIGIT(c) (c >= TCHAR('0') && c <= TCHAR('9')) 
/**
 * Determine if a character is alphanumeric (upper or lower case)
 *
 * \param c
 * The character to test
 *
 * \returns
 * OFC_TRUE if alphanumeric, OFC_FALSE otherwise
 */
#define OFC_ISALPHA(c) {OFC_ISUPPER((c)) || OFC_ISLOWER((c)))
#define OFC_ISALNUM(c) (OFC_ISUPPER(c)||OFC_ISLOWER(c)||OFC_ISDIGIT(c))
/**
 * Determine if a wide character is alphanumeric (upper or lower case)
 *
 * \param c
 * The character to test
 *
 * \returns
 * OFC_TRUE if alphanumeric, OFC_FALSE otherwise
 */
#define OFC_TISALPHA(c) {OFC_TISUPPER(c) || OFC_TISLOWER(c))
/**
 * Determine if character is upper case (A..Z)
 *
 * \param c
 * character to test
 *
 * \returns
 * OFC_TRUE if upper case, OFC_FALSE otherwise
 */
#define OFC_ISUPPER(c) (c >=  'A' && c <= 'Z')
/**
 * Determine if wide character is upper case (A..Z)
 *
 * \param c
 * character to test
 *
 * \returns
 * OFC_TRUE if upper case, OFC_FALSE otherwise
 */
#define OFC_TISUPPER(c) (c >=  TCHAR('A') && c <= TCHAR('Z'))
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
#define OFC_TOUPPER(c) (OFC_ISLOWER(c)?c-('a'-'A'):c)
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
#define OFC_TOLOWER(c) (OFC_ISUPPER(c)?c+('a'-'A'):c)
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
#define OFC_TTOUPPER(c) (OFC_TISLOWER(c)?c-(TCHAR('a')-TCHAR('A')):c)
/**
 * Determine if character is lower case (a..z)
 *
 * \param c
 * Character to test
 *
 * \returns
 * OFC_TRUE if lower case, OFC_FALSE otherwise
 */
#define OFC_ISLOWER(c) (c >=  'a' && c <= 'z')
/**
 * Determine if wide character is lower case (a..z)
 *
 * \param c
 * Character to test
 *
 * \returns
 * OFC_TRUE if lower case, OFC_FALSE otherwise
 */
#define OFC_TISLOWER(c) (c >=  TCHAR('a') && c <= TCHAR('z'))
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
#define OFC_MIN(a, b) ((a) < (b) ? (a) : (b))
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
#define OFC_MAX(a, b) ((a) > (b) ? (a) : (b))

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
  OFC_CORE_LIB OFC_INT
  ofc_strcmp (OFC_CCHAR *astr, OFC_CCHAR *bstr) ;
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
  OFC_CORE_LIB OFC_INT
  ofc_strcasecmp (OFC_CCHAR *astr, OFC_CCHAR *bstr) ;
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
  OFC_CORE_LIB OFC_INT
  ofc_strncmp (OFC_CCHAR *astr, OFC_CCHAR *bstr, OFC_SIZET len) ;
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
  OFC_CORE_LIB OFC_INT
  ofc_strncasecmp (OFC_CCHAR *astr, OFC_CCHAR *bstr, OFC_SIZET len) ;
  /**
   * Find the length of a string
   *
   * \param astr
   * Pointer to the string to measure
   *
   * \returns
   * The size of the string not including the terminating NIL
   */
  OFC_CORE_LIB OFC_SIZET
  ofc_strlen (OFC_CCHAR *astr) ;
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
  OFC_CORE_LIB OFC_SIZET
  ofc_strnlen (OFC_CCHAR *astr, OFC_SIZET len) ;
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
  OFC_CORE_LIB OFC_CHAR *
  ofc_strdup (OFC_CCHAR *astr) ;
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
  OFC_CORE_LIB OFC_CHAR *
  ofc_strndup (OFC_CCHAR *astr, OFC_SIZET len) ;
  OFC_CORE_LIB OFC_TCHAR *
  ofc_tstrndup (OFC_CTCHAR *astr, OFC_SIZET len) ;
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
  OFC_CORE_LIB OFC_CHAR *
  ofc_strcpy (OFC_CHAR *dst, OFC_CCHAR *src) ;
  OFC_CORE_LIB OFC_CHAR *
  ofc_strcat (OFC_CHAR *dst, OFC_CCHAR *src) ;
  OFC_CORE_LIB OFC_CHAR *
  ofc_strncat (OFC_CHAR *dst, OFC_CCHAR *src, OFC_SIZET size) ;
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
  OFC_CORE_LIB OFC_TCHAR *
  ofc_tstrcpy (OFC_TCHAR *dst, OFC_CTCHAR *src) ;
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
  OFC_CHAR *ofc_strnupr (OFC_CHAR *s, OFC_SIZET len) ;
  OFC_TCHAR *ofc_tstrnupr (OFC_TCHAR *src, OFC_SIZET len) ;
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
  OFC_CORE_LIB OFC_CHAR *
  ofc_strncpy (OFC_CHAR *dst, OFC_CCHAR *src, OFC_SIZET len) ;
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
  OFC_CORE_LIB OFC_TCHAR *
  ofc_tstrncpy (OFC_TCHAR *dst, OFC_CTCHAR *src, OFC_SIZET len) ;
  /**
   * Return the length of a path
   *
   * \param str
   * Pointer to a string.  Either UTF-16 or UTF-8
   *
   * \returns
   * size of string
   */
  OFC_CORE_LIB OFC_SIZET
  ofc_tstrlen (OFC_LPCTSTR str) ;
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
  OFC_CORE_LIB OFC_SIZET
  ofc_tstrnlen (OFC_LPCTSTR str, OFC_SIZET len) ;
  /**
   * Duplicate a path
   *
   * \param str
   * Pointer to string to duplicate (either UTF-16 or UTF-8)
   *
   * \returns
   * Duplicated string (UTF-16 or UTF-8)
   */
  OFC_CORE_LIB OFC_LPTSTR
  ofc_tstrdup (OFC_LPCTSTR str) ;
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
  OFC_CORE_LIB OFC_INT
  ofc_tstrcmp (OFC_LPCTSTR astr, OFC_LPCTSTR bstr) ;
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
  OFC_CORE_LIB OFC_INT
  ofc_tstrcasecmp (OFC_LPCTSTR astr, OFC_LPCTSTR bstr) ;
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
  OFC_CORE_LIB OFC_INT
  ofc_tstrncasecmp (OFC_CTCHAR *astr, OFC_CTCHAR *bstr, OFC_SIZET len) ;
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
  OFC_CORE_LIB OFC_LPCTSTR
  ofc_tstrtok (OFC_LPCTSTR str, OFC_LPCTSTR terms) ;

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
  OFC_CORE_LIB OFC_LPSTR
  ofc_strtok (OFC_LPCSTR str, OFC_LPCSTR terms) ;
  OFC_CORE_LIB OFC_LPSTR
  ofc_strchr (OFC_LPCSTR str, OFC_CCHAR c) ;
  OFC_CORE_LIB OFC_LPSTR
  ofc_memchr (OFC_LPCSTR str, OFC_CCHAR c, OFC_SIZET size) ;
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
  OFC_CORE_LIB OFC_LPCTSTR
  ofc_tstrrtok (OFC_LPCTSTR str, OFC_LPCTSTR terms) ;
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
  OFC_CORE_LIB OFC_LPCSTR
  ofc_strrtok (OFC_LPCSTR str, OFC_LPCSTR terms) ;
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
  OFC_CORE_LIB OFC_INT
  ofc_tstrncmp (OFC_CTCHAR *astr, OFC_CTCHAR *bstr, OFC_SIZET len) ;
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
   * be freed by a call to ofc_heap_free
   */
  OFC_CORE_LIB OFC_CHAR *
  ofc_tstr2cstr (OFC_LPCTSTR str) ;
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
   * be freed by a call to ofc_heap_free
   */
  OFC_CORE_LIB OFC_LPTSTR
  ofc_cstr2tstr (OFC_CCHAR *str)  ;
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
  OFC_CORE_LIB OFC_LPVOID
  ofc_memcpy (OFC_LPVOID out, OFC_LPCVOID in, OFC_SIZET size) ;
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
  OFC_CORE_LIB OFC_INT
  ofc_memcmp (OFC_LPCVOID a, OFC_LPCVOID b, OFC_SIZET size) ;
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
  OFC_CORE_LIB OFC_LPVOID
  ofc_memset (OFC_LPVOID dst, OFC_INT c, OFC_SIZET size) ;
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
  OFC_CORE_LIB OFC_LONG
  ofc_strtol (const OFC_CHAR *str, OFC_CHAR **upd, OFC_INT base) ;
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
  OFC_CORE_LIB OFC_ULONG
  ofc_strtoul (const OFC_CHAR *string, OFC_CHAR **endPtr, OFC_INT base) ;
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
   * enough.  By calling ofc_vsnprintf with a 0 count and a NULL destination
   * string, the required size of the string will be returned (not including 
   * a terminating NIL.  This return value can be used to allocate a 
   * destination string and the ofc_vsnprintf can be called again with the
   * allocated memory block and size of that block.
   */
  OFC_CORE_LIB OFC_SIZET
  ofc_vsnprintf (OFC_CHAR *str, OFC_SIZET count,
                 OFC_CCHAR *fmt, va_list args) ;
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
   * format character in the format string. \see ofc_vsnprintf
   */
  OFC_CORE_LIB OFC_SIZET
  ofc_snprintf(OFC_CHAR *str, OFC_SIZET count, OFC_CCHAR *fmt, ...) ;
  /**
   * Perform formated output to the system console
   *
   * \param fmt
   * The format string
   *
   * \remark
   * This call takes any number of arguments each related to the respective
   * format character in the format string. \see ofc_vsnprintf
   */
  OFC_CORE_LIB OFC_VOID
  ofc_printf(OFC_CCHAR *fmt, ...) ;

  OFC_CORE_LIB OFC_CHAR *
  ofc_saprintf(OFC_CCHAR *fmt, ...) ;
  /**
   * Perform formated output to a trace buffer
   *
   * \param fmt
   * The format string
   *
   * \remark
   * This call takes any number of arguments each related to the respective
   * format character in the format string. \see ofc_vsnprintf.  This
   * call is a lot like ofc_printf but the output goes to a trace buffer
   * instead.
   */
#if 0
  /**
   * Initialize the C Runtime Library
   *
   * The routine is called by the system startup code to initialize the 
   * C runtime.  This should only be called from within ofc_init
   */
  OFC_CORE_LIB OFC_VOID 
  ofc_trace_init (OFC_VOID) ;
  OFC_CORE_LIB OFC_VOID 
  ofc_trace_destroy (OFC_VOID);
  OFC_CORE_LIB OFC_VOID 
  ofc_trace (OFC_CCHAR *fmt,...) ;
  OFC_CORE_LIB OFC_VOID 
  ofc_dump_trace(OFC_VOID) ;
#else
#define ofc_trace_init()
#define ofc_trace_destroy()
#define ofc_trace(fmt, ...)
#define ofc_dump_trace()
#endif

  OFC_VOID ofc_uuidtoa (OFC_UUID uuid, OFC_CHAR *out) ;

  OFC_VOID ofc_atouuid (const OFC_CHAR *in, OFC_UUID uuid) ;

#if defined(__cplusplus)
}
#endif

#if defined(OFC_UNICODE_API)
#define ofc_tastrcpy(dst,src) ofc_tstrcpy(dst,src)
#define ofc_tastrncpy(dst,src,len) ofc_tstrncpy(dst,src,len)
#define ofc_tastrlen(str) ofc_tstrlen(str)
#define ofc_tastrnlen(str,len) ofc_tstrnlen(str,len)
#define ofc_tastrdup(str) (OFC_LPTASTR)ofc_tstrdup(str)
#define ofc_tastrcmp(astr,bstr) ofc_tstrcmp(astr,bstr)
#define ofc_tastrcasecmp(astr,bstr) ofc_tstrcasecmp(astr,bstr)
#define ofc_tastrncasecmp(astr,bstr,len) ofc_tstrncasecmp(astr,bstr,len)
#define ofc_tastrtok(str,terms) ofc_tstrtok(str,terms)
#define ofc_tastrrtok(str,terms) ofc_tstrrtok(str,terms)
#define ofc_tastrncmp(astr,bstr,len) ofc_tstrncmp(astr,bstr,len)
#define ofc_tastr2cstr(str) ofc_tstr2cstr(str)
#define ofc_cstr2tastr(str) ofc_cstr2tstr(str)
#else
#define ofc_tastrcpy(dst,src) ofc_strcpy(dst,src)
#define ofc_tastrncpy(dst,src,len) ofc_strncpy(dst,src,len)
#define ofc_tastrlen(str) ofc_strlen(str)
#define ofc_tastrnlen(str,len) ofc_strnlen(str,len)
#define ofc_tastrdup(str) ofc_strdup(str)
#define ofc_tastrcmp(astr,bstr) ofc_strcmp(astr,bstr)
#define ofc_tastrcasecmp(astr,bstr) ofc_strcasecmp(astr,bstr)
#define ofc_tastrncasecmp(astr,bstr,len) ofc_strncasecmp(astr,bstr,len)
#define ofc_tastrtok(str,terms) ofc_strtok(str,terms)
#define ofc_tastrrtok(str,terms) ofc_strrtok(str,terms)
#define ofc_tastrncmp(astr,bstr,len) ofc_strncmp(astr,bstr,len)
#define ofc_tastr2cstr(str) ofc_strdup(str)
#define ofc_cstr2tastr(str) ofc_strdup(str)
#endif

#endif

