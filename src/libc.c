/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include <stdarg.h>

#if defined(__APPLE__)

#include <string.h>
#include <errno.h>

#endif

#if defined(__linux__)
#include <stdlib.h>
#include <string.h>
#endif

#include "ofc/core.h"
#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/console.h"
#include "ofc/lock.h"
#include "ofc/socket.h"
#include "ofc/message.h"
#include "ofc/time.h"

#include "ofc/heap.h"

struct trace_t {
    OFC_INT trace_offset;
    OFC_CHAR trace_buf[OFC_TRACE_LEN];
    OFC_OFFT trace_lock;
};

/**
 * \internal
 * Determine if a character is a whitespace
 */
OFC_CORE_LIB OFC_INT
ofc_strcmp(OFC_CCHAR *astr, OFC_CCHAR *bstr) {
    OFC_CCHAR *pa;
    OFC_CCHAR *pb;
    OFC_INT ret;

    if (astr == OFC_NULL || bstr == OFC_NULL) {
        if (astr == OFC_NULL && bstr == OFC_NULL)
            ret = 0;
        else
            ret = -1;
    } else {
        pa = astr;
        pb = bstr;

        /*
         * Step through the string until one of them reaches a NIL
         */
        while (*pa == *pb && *pa != '\0' && *pb != '\0') {
            pa++;
            pb++;
        }

        /*
         * Then if the character from a at that position is greater then the
         * respsective character in b, a is bigger.  If the a character is less
         * then a is less, and if they are equal (or both NIL), then the strings
         * are equal.
         */
        ret = *pa - *pb;
    }
    return (ret);
}

OFC_CORE_LIB OFC_INT
ofc_strcasecmp(OFC_CCHAR *astr, OFC_CCHAR *bstr) {
    OFC_CCHAR *pa;
    OFC_CCHAR *pb;
    OFC_INT ret;

    if (astr == OFC_NULL || bstr == OFC_NULL) {
        if (astr == OFC_NULL && bstr == OFC_NULL)
            ret = 0;
        else
            ret = -1;
    } else {
        pa = astr;
        pb = bstr;

        /*
         * Step through the string until one of them reaches a NIL
         */
        while (OFC_TOUPPER(*pa) == OFC_TOUPPER(*pb) &&
               *pa != '\0' && *pb != '\0') {
            pa++;
            pb++;
        }

        /*
         * Then if the character from a at that position is greater then the
         * respsective character in b, a is bigger.  If the a character is less
         * then a is less, and if they are equal (or both NIL), then the strings
         * are equal.
         */
        ret = OFC_TOUPPER(*pa) - OFC_TOUPPER(*pb);
    }
    return (ret);
}

OFC_CORE_LIB OFC_INT
ofc_strncmp(OFC_CCHAR *astr, OFC_CCHAR *bstr, OFC_SIZET len) {
    OFC_CCHAR *pa;
    OFC_CCHAR *pb;
    OFC_INT ret;

    if (astr == OFC_NULL || bstr == OFC_NULL) {
        if (astr == OFC_NULL && bstr == OFC_NULL)
            ret = 0;
        else
            ret = -1;
    } else {
        pa = astr;
        pb = bstr;
        ret = 0;
        /*
         * Step through the string until one of them reaches a NIL
         */
        while (*pa == *pb && *pa != '\0' && *pb != '\0' && len > 0) {
            pa++;
            pb++;
            len--;
        }
        /*
         * Then if the character from a at that position is greater then the
         * respsective character in b, a is bigger.  If the a character is less
         * then a is less, and if they are equal (or both NIL), then the strings
         * are equal.
         */
        if (len > 0)
            ret = *pa - *pb;
    }
    return (ret);
}

OFC_CORE_LIB OFC_INT
ofc_strncasecmp(OFC_CCHAR *astr, OFC_CCHAR *bstr, OFC_SIZET len) {
    OFC_CCHAR *pa;
    OFC_CCHAR *pb;
    OFC_INT ret;

    if (astr == OFC_NULL || bstr == OFC_NULL) {
        if (astr == OFC_NULL && bstr == OFC_NULL)
            ret = 0;
        else
            ret = -1;
    } else {
        pa = astr;
        pb = bstr;
        ret = 0;

        /*
         * Step through the string until one of them reaches a NIL
         */
        while (OFC_TOUPPER(*pa) == OFC_TOUPPER(*pb) &&
               *pa != '\0' && *pb != '\0' && len > 0) {
            pa++;
            pb++;
            len--;
        }

        /*
         * Then if the character from a at that position is greater then the
         * respsective character in b, a is bigger.  If the a character is less
         * then a is less, and if they are equal (or both NIL), then the strings
         * are equal.
         */
        if (len > 0)
            ret = OFC_TOUPPER(*pa) - OFC_TOUPPER(*pb);
    }
    return (ret);
}

/**
 * \internal
 * Measure the size of a string
 */
OFC_CORE_LIB OFC_SIZET
ofc_strlen(OFC_CCHAR *astr) {
    OFC_SIZET ret;
    OFC_CCHAR *pa;

    if (astr == OFC_NULL)
        ret = 0;
    else {
        ret = 0;
        pa = astr;

        while (*pa != '\0') {
            ret++;
            pa++;
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_SIZET
ofc_strnlen(OFC_CCHAR *astr, OFC_SIZET len) {
    OFC_SIZET ret;
    OFC_CCHAR *pa;

    if (astr == OFC_NULL)
        ret = 0;
    else {
        ret = 0;
        pa = astr;

        while (*pa != '\0' && len > 0) {
            ret++;
            pa++;
            len--;
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_CHAR *
ofc_strdup(OFC_CCHAR *astr) {
    OFC_SIZET size;
    OFC_CHAR *ret;

    if (astr == OFC_NULL)
        ret = OFC_NULL;
    else {
        size = ofc_strlen(astr);
        ret = ofc_malloc(size + 1);
        ofc_strcpy(ret, astr);
    }
    return (ret);
}

OFC_CORE_LIB OFC_CHAR *
ofc_strndup(OFC_CCHAR *astr, OFC_SIZET len) {
    OFC_SIZET size;
    OFC_CHAR *ret;
    OFC_SIZET slen;

    if (astr == OFC_NULL)
        ret = OFC_NULL;
    else {
        slen = ofc_strnlen(astr, len);
        size = OFC_MIN(slen, len);
        ret = ofc_malloc(size+1);
        ofc_strncpy(ret, astr, size);
	ret[size] = TCHAR_EOS;
    }
    return (ret);
}

OFC_CORE_LIB OFC_TCHAR *
ofc_tstrndup(OFC_CTCHAR *astr, OFC_SIZET len) {
    OFC_SIZET size;
    OFC_TCHAR *ret;
    OFC_SIZET slen;

    if (astr == OFC_NULL)
        ret = OFC_NULL;
    else {
        slen = ofc_tstrnlen(astr, len);
        size = OFC_MIN(slen, len);
        ret = ofc_malloc((size+1) * sizeof(OFC_TCHAR));
        ofc_tstrncpy(ret, astr, size);
	ret[size] = TCHAR_EOS;
    }
    return (ret);
}

OFC_CORE_LIB OFC_CHAR *
ofc_strncpy(OFC_CHAR *dst, OFC_CCHAR *src, OFC_SIZET len) {
    OFC_CCHAR *psrc;
    OFC_CHAR *pdst;
    OFC_CHAR *ret;

    if (src == OFC_NULL || dst == OFC_NULL) {
        ret = OFC_NULL;
    } else {
        psrc = src;
        pdst = dst;

        while (*psrc != '\0' && len > 0) {
            *pdst = *psrc;
            pdst++;
            psrc++;
            len--;
        }
        if (len > 0)
            *pdst = '\0';

        ret = dst;
    }

    return (ret);
}

OFC_CHAR *ofc_strnupr(OFC_CHAR *src, OFC_SIZET len) {
    OFC_CHAR *psrc;
    OFC_CHAR *ret;

    if (src == OFC_NULL) {
        ret = OFC_NULL;
    } else {
        psrc = src;

        while (*psrc != '\0' && len > 0) {
            *psrc = OFC_TOUPPER(*psrc);
            psrc++;
            len--;
        }
        ret = src;
    }

    return (ret);
}

OFC_TCHAR *ofc_tstrnupr(OFC_TCHAR *src, OFC_SIZET len) {
    OFC_TCHAR *psrc;
    OFC_TCHAR *ret;

    if (src == OFC_NULL) {
        ret = OFC_NULL;
    } else {
        psrc = src;

        while (*psrc != '\0' && len > 0) {
            *psrc = OFC_TOUPPER(*psrc);
            psrc++;
            len--;
        }
        ret = src;
    }

    return (ret);
}

OFC_CORE_LIB OFC_TCHAR *
ofc_tstrncpy(OFC_TCHAR *dst, OFC_CTCHAR *src, OFC_SIZET len) {
    OFC_CTCHAR *psrc;
    OFC_TCHAR *pdst;
    OFC_TCHAR *ret;

    if (src == OFC_NULL || dst == OFC_NULL) {
        ret = OFC_NULL;
    } else {
        psrc = src;
        pdst = dst;

        while (*psrc != TCHAR_EOS && len > 0) {
            *pdst = *psrc;
            pdst++;
            psrc++;
            len--;
        }
        if (len > 0)
            *pdst = TCHAR_EOS;

        ret = dst;
    }
    return (ret);
}

OFC_CORE_LIB OFC_CHAR *
ofc_strcpy(OFC_CHAR *dst, OFC_CCHAR *src) {
    OFC_CCHAR *psrc;
    OFC_CHAR *pdst;
    OFC_CHAR *ret;

    if (src == OFC_NULL || dst == OFC_NULL)
        ret = OFC_NULL;
    else {
        psrc = src;
        pdst = dst;

        while (*psrc != '\0') {
            *pdst = *psrc;
            pdst++;
            psrc++;
        }
        *pdst = '\0';
        ret = dst;
    }
    return (ret);
}

OFC_CORE_LIB OFC_CHAR *
ofc_strcat(OFC_CHAR *dst, OFC_CCHAR *src) {
    OFC_CCHAR *psrc;
    OFC_CHAR *pdst;
    OFC_CHAR *ret;

    if (src == OFC_NULL || dst == OFC_NULL)
        ret = OFC_NULL;
    else {
        psrc = src;
        pdst = dst + ofc_strlen(dst);

        while (*psrc != '\0') {
            *pdst = *psrc;
            pdst++;
            psrc++;
        }
        *pdst = '\0';
        ret = dst;
    }
    return (ret);
}

OFC_CORE_LIB OFC_CHAR *
ofc_strncat(OFC_CHAR *dst, OFC_CCHAR *src, OFC_SIZET size) {
    OFC_CCHAR *psrc;
    OFC_CHAR *pdst;
    OFC_CHAR *ret;

    if (src == OFC_NULL || dst == OFC_NULL)
        ret = OFC_NULL;
    else {
        psrc = src;
        pdst = dst + ofc_strlen(dst);

        while (*psrc != '\0' && size > 0) {
            *pdst = *psrc;
            pdst++;
            psrc++;
            size--;
        }
        if (size > 0)
            *pdst = '\0';
        ret = dst;
    }
    return (ret);
}

OFC_CORE_LIB OFC_TCHAR *
ofc_tstrcpy(OFC_TCHAR *dst, OFC_CTCHAR *src) {
    OFC_CTCHAR *psrc;
    OFC_TCHAR *pdst;
    OFC_TCHAR *ret;

    if (src == OFC_NULL || dst == OFC_NULL)
        ret = OFC_NULL;
    else {
        psrc = src;
        pdst = dst;

        while (*psrc != TCHAR_EOS) {
            *pdst = *psrc;
            pdst++;
            psrc++;
        }
        *pdst = TCHAR_EOS;
        ret = dst;
    }
    return (ret);
}

OFC_CORE_LIB OFC_SIZET
ofc_tstrlen(OFC_LPCTSTR str) {
    OFC_SIZET ret;
    OFC_LPCTSTR p;

    if (str == OFC_NULL)
        ret = 0;
    else
        for (ret = 0, p = str; *p != TCHAR_EOS; ret++, p++);
    return (ret);
}

OFC_CORE_LIB OFC_SIZET
ofc_tstrnlen(OFC_LPCTSTR str, OFC_SIZET len) {
    OFC_SIZET ret;
    OFC_LPCTSTR p;

    if (str == OFC_NULL)
        ret = 0;
    else
        for (ret = 0, p = str; *p != TCHAR_EOS && len > 0; ret++, p++, len--);
    return (ret);
}


OFC_CORE_LIB OFC_LPTSTR
ofc_tstrdup(OFC_LPCTSTR str) {
    OFC_LPTSTR ret;
    OFC_SIZET len;

    ret = OFC_NULL;
    if (str != OFC_NULL) {
        len = (ofc_tstrlen(str) + 1) * sizeof(OFC_TCHAR);
        ret = ofc_malloc(len);
        ofc_memcpy(ret, str, len);
    }
    return (ret);
}

OFC_CORE_LIB OFC_INT
ofc_tstrcmp(OFC_LPCTSTR astr, OFC_LPCTSTR bstr) {
    OFC_CTCHAR *pa;
    OFC_CTCHAR *pb;
    OFC_INT ret;

    if (astr == OFC_NULL || bstr == OFC_NULL) {
        if (astr == OFC_NULL && bstr == OFC_NULL)
            ret = 0;
        else
            ret = -1;
    } else {
        pa = astr;
        pb = bstr;

        /*
         * Step through the string until one of them reaches a NIL
         */
        while (*pa == *pb && *pa != '\0' && *pb != '\0') {
            pa++;
            pb++;
        }

        /*
         * Then if the character from a at that position is greater then the
         * respsective character in b, a is bigger.  If the a character is less
         * then a is less, and if they are equal (or both NIL), then the strings
         * are equal.
         */
        ret = *pa - *pb;
    }
    return ret;
}

OFC_CORE_LIB OFC_INT
ofc_tstrcasecmp(OFC_LPCTSTR astr, OFC_LPCTSTR bstr) {
    OFC_CTCHAR *pa;
    OFC_CTCHAR *pb;
    OFC_INT ret;

    if (astr == OFC_NULL || bstr == OFC_NULL) {
        if (astr == OFC_NULL && bstr == OFC_NULL)
            ret = 0;
        else
            ret = -1;
    } else {
        pa = astr;
        pb = bstr;

        /*
         * Step through the string until one of them reaches a NIL
         */
        while (OFC_TTOUPPER(*pa) == OFC_TTOUPPER(*pb) &&
               *pa != TCHAR_EOS && *pb != TCHAR_EOS) {
            pa++;
            pb++;
        }

        /*
         * Then if the character from a at that position is greater then the
         * respsective character in b, a is bigger.  If the a character is less
         * then a is less, and if they are equal (or both NIL), then the strings
         * are equal.
         */
        ret = OFC_TTOUPPER(*pa) - OFC_TTOUPPER(*pb);
    }
    return (ret);
}

OFC_CORE_LIB OFC_INT
ofc_tstrncmp(OFC_CTCHAR *astr, OFC_CTCHAR *bstr, OFC_SIZET len) {
    OFC_CTCHAR *pa;
    OFC_CTCHAR *pb;
    OFC_INT ret;

    if (astr == OFC_NULL || bstr == OFC_NULL) {
        if (astr == OFC_NULL && bstr == OFC_NULL)
            ret = 0;
        else
            ret = -1;
    } else {
        pa = astr;
        pb = bstr;
        ret = 0;

        /*
         * Step through the string until one of them reaches a NIL
         */
        while (*pa == *pb && *pa != '\0' && *pb != '\0' && len > 0) {
            pa++;
            pb++;
            len--;
        }

        /*
         * Then if the character from a at that position is greater then the
         * respsective character in b, a is bigger.  If the a character is less
         * then a is less, and if they are equal (or both NIL), then the strings
         * are equal.
         */
        if (len > 0)
            ret = *pa - *pb;
    }
    return (ret);
}

OFC_CORE_LIB OFC_INT
ofc_tstrncasecmp(OFC_CTCHAR *astr, OFC_CTCHAR *bstr, OFC_SIZET len) {
    OFC_CTCHAR *pa;
    OFC_CTCHAR *pb;
    OFC_INT ret;

    if (astr == OFC_NULL || bstr == OFC_NULL) {
        if (astr == OFC_NULL && bstr == OFC_NULL)
            ret = 0;
        else
            ret = -1;
    } else {
        pa = astr;
        pb = bstr;
        ret = 0;

        /*
         * Step through the string until one of them reaches a NIL
         */
        while (OFC_TTOUPPER(*pa) == OFC_TTOUPPER(*pb) &&
               *pa != TCHAR_EOS && *pb != TCHAR_EOS && len > 0) {
            pa++;
            pb++;
            len--;
        }

        /*
         * Then if the character from a at that position is greater then the
         * respsective character in b, a is bigger.  If the a character is less
         * then a is less, and if they are equal (or both NIL), then the strings
         * are equal.
         */
        if (len > 0)
            ret = OFC_TTOUPPER(*pa) - OFC_TTOUPPER(*pb);
    }
    return (ret);
}

OFC_CORE_LIB OFC_LPCTSTR
ofc_tstrtok(OFC_LPCTSTR str, OFC_LPCTSTR terms) {
    OFC_LPCTSTR peek;
    OFC_BOOL hit;
    OFC_LPCTSTR ret;

    if (str == OFC_NULL || terms == OFC_NULL)
        ret = OFC_NULL;
    else {
        for (peek = str, hit = OFC_FALSE; *peek != TCHAR_EOS && !hit;) {
            OFC_INT i;

            for (i = 0; i < ofc_tstrlen(terms) && !hit; i++) {
                if (*peek == terms[i])
                    hit = OFC_TRUE;
            }
            if (!hit)
                peek++;
        }
        ret = peek;
    }
    return (ret);
}

OFC_CORE_LIB OFC_LPSTR
ofc_strtok(OFC_LPCSTR str, OFC_LPCSTR terms) {
    OFC_LPCSTR peek;
    OFC_BOOL hit;
    OFC_LPSTR ret;

    if (str == OFC_NULL || terms == OFC_NULL)
        ret = OFC_NULL;
    else {
        for (peek = str, hit = OFC_FALSE; *peek != '\0' && !hit;) {
            OFC_INT i;

            for (i = 0; i < ofc_strlen(terms) && !hit; i++) {
                if (*peek == terms[i])
                    hit = OFC_TRUE;
            }
            if (!hit)
                peek++;
        }
        ret = (OFC_LPSTR) peek;
    }
    return (ret);
}

OFC_CORE_LIB OFC_LPSTR
ofc_strchr(OFC_LPCSTR str, OFC_CCHAR c) {
    OFC_LPCSTR peek;
    OFC_LPSTR ret;

    ret = OFC_NULL;
    if (str != OFC_NULL) {
        for (peek = str; *peek != c && *peek != '\0'; peek++);
        if (*peek == c)
            ret = (OFC_LPSTR) peek;
    }
    return (ret);
}

OFC_CORE_LIB OFC_LPSTR
ofc_memchr(OFC_LPCSTR str, OFC_CCHAR c, OFC_SIZET size) {
    OFC_LPCSTR peek;
    OFC_LPSTR ret;

    ret = OFC_NULL;
    if (str != OFC_NULL) {
        for (peek = str; *peek != c && size > 0; peek++, size--);
        if (*peek == c)
            ret = (OFC_LPSTR) peek;
    }
    return (ret);
}

OFC_CORE_LIB OFC_LPCTSTR
ofc_tstrrtok(OFC_LPCTSTR str, OFC_LPCTSTR terms) {
    OFC_LPCTSTR peek;
    OFC_BOOL hit;
    OFC_LPCTSTR ret;

    if (str == OFC_NULL || terms == OFC_NULL)
        ret = OFC_NULL;
    else {
        peek = str + ofc_tstrlen(str);

        for (peek--, hit = OFC_FALSE; peek != str && !hit;) {
            OFC_INT i;

            for (i = 0; i < ofc_tstrlen(terms) && !hit; i++) {
                if (*peek == terms[i])
                    hit = OFC_TRUE;
            }
            if (!hit)
                peek--;
        }
        ret = peek;
    }
    return (ret);
}

OFC_CORE_LIB OFC_LPCSTR
ofc_strrtok(OFC_LPCSTR str, OFC_LPCSTR terms) {
    OFC_LPCSTR peek;
    OFC_BOOL hit;
    OFC_LPCSTR ret;

    if (str == OFC_NULL || terms == OFC_NULL)
        ret = OFC_NULL;
    else {
        peek = str + ofc_strlen(str);

        for (peek--, hit = OFC_FALSE; peek != str && !hit;) {
            OFC_INT i;

            for (i = 0; i < ofc_strlen(terms) && !hit; i++) {
                if (*peek == terms[i])
                    hit = OFC_TRUE;
            }
            if (!hit)
                peek--;
        }
        ret = peek;
    }
    return (ret);
}

OFC_CORE_LIB OFC_CHAR *
ofc_tstr2cstr(OFC_LPCTSTR str) {
    OFC_SIZET len;
    OFC_CHAR *cstr;
    OFC_INT i;
    OFC_CTCHAR *ptstr;
    OFC_CHAR *pcstr;

    cstr = OFC_NULL;
    if (str != OFC_NULL) {
        len = ofc_tstrlen(str);
        cstr = ofc_malloc(len + 1);
        pcstr = cstr;
        ptstr = str;
        for (i = 0; i < len; i++)
            *pcstr++ = (OFC_CHAR) *ptstr++;
        *pcstr = '\0';
    }
    return (cstr);
}

OFC_CORE_LIB OFC_LPTSTR
ofc_cstr2tstr(OFC_CCHAR *str) {
    OFC_SIZET len;
    OFC_LPTSTR tstr;
    OFC_INT i;
    OFC_TCHAR *ptstr;
    OFC_CCHAR *pcstr;

    tstr = OFC_NULL;
    if (str != OFC_NULL) {
        len = ofc_strlen(str);
        tstr = ofc_malloc((len + 1) * sizeof(OFC_TCHAR));
        ptstr = tstr;
        pcstr = str;
        for (i = 0; i < len; i++)
            *ptstr++ = (OFC_TCHAR) (*pcstr++);
        *ptstr = (OFC_TCHAR) '\0';
    }
    return (tstr);
}

OFC_CORE_LIB OFC_INT
ofc_memcmp(OFC_LPCVOID a, OFC_LPCVOID b, OFC_SIZET size) {
    OFC_INT ret;
    OFC_UINT32 *puint32;
    const OFC_UINT32 *pcuint32;
    OFC_UINT16 *puint16;
    const OFC_UINT16 *pcuint16;
    OFC_UINT8 *puint8;
    const OFC_UINT8 *pcuint8;

    if (a == OFC_NULL || b == OFC_NULL) {
        if (a == OFC_NULL && b == OFC_NULL)
            ret = 0;
        else
            ret = -1;
    } else {
        ret = 0;
        if (!((OFC_DWORD_PTR) a & 0x03) && !((OFC_DWORD_PTR) b & 0x03)) {
            puint32 = (OFC_UINT32 *) b;
            pcuint32 = (const OFC_UINT32 *) a;

            while (size >= 4 && ret == 0) {
                ret = *pcuint32++ - *puint32++;
                size -= 4;
            }
            puint8 = (OFC_UINT8 *) puint32;
            pcuint8 = (const OFC_UINT8 *) pcuint32;
            while (size != 0 && ret == 0) {
                ret = *pcuint8++ - *puint8++;
                size--;
            }

        } else if (!((OFC_DWORD_PTR) a & 0x01) && !((OFC_DWORD_PTR) b & 0x01)) {
            puint16 = (OFC_UINT16 *) b;
            pcuint16 = (const OFC_UINT16 *) a;

            while (size >= 2 && ret == 0) {
                ret = *pcuint16++ - *puint16++;
                size -= 2;
            }
            puint8 = (OFC_UINT8 *) puint16;
            pcuint8 = (const OFC_UINT8 *) pcuint16;
            while (size != 0 && ret == 0) {
                ret = *pcuint8++ - *puint8++;
                size--;
            }
        } else {
            puint8 = (OFC_UINT8 *) b;
            pcuint8 = (const OFC_UINT8 *) a;

            while (size != 0 && ret == 0) {
                ret = *pcuint8++ - *puint8++;
                size--;
            }
        }
    }
    return (ret);
}

OFC_CORE_LIB OFC_LPVOID
ofc_memcpy(OFC_LPVOID out, OFC_LPCVOID in, OFC_SIZET size) {
    OFC_LPVOID dst;
#if defined(__linux__)
    dst = memcpy (out, in, size);
#else
    OFC_UINT32 *puint32;
    const OFC_UINT32 *pcuint32;
    OFC_UINT16 *puint16;
    const OFC_UINT16 *pcuint16;
    OFC_UINT8 *puint8;
    const OFC_UINT8 *pcuint8;

    if (out == OFC_NULL || in == OFC_NULL)
        dst = OFC_NULL;
    else {
        dst = out;

        if (!((OFC_DWORD_PTR) in & 0x03) && !((OFC_DWORD_PTR) out & 0x03)) {
            puint32 = (OFC_UINT32 *) out;
            pcuint32 = (const OFC_UINT32 *) in;

            while (size >= 4) {
                *puint32++ = *pcuint32++;
                size -= 4;
            }
            puint8 = (OFC_UINT8 *) puint32;
            pcuint8 = (const OFC_UINT8 *) pcuint32;
            while (size != 0) {
                *puint8++ = *pcuint8++;
                size--;
            }
        } else if (!((OFC_DWORD_PTR) in & 0x01) && !((OFC_DWORD_PTR) out & 0x01)) {
            puint16 = (OFC_UINT16 *) out;
            pcuint16 = (const OFC_UINT16 *) in;

            while (size >= 2) {
                *puint16++ = *pcuint16++;
                size -= 2;
            }
            puint8 = (OFC_UINT8 *) puint16;
            pcuint8 = (const OFC_UINT8 *) pcuint16;
            while (size != 0) {
                *puint8++ = *pcuint8++;
                size--;
            }
        } else {
            puint8 = (OFC_UINT8 *) out;
            pcuint8 = (const OFC_UINT8 *) in;

            while (size != 0) {
                *puint8++ = *pcuint8++;
                size--;
            }
        }
    }
#endif
    return (dst);
}

OFC_CORE_LIB OFC_LPVOID
ofc_memset(OFC_LPVOID dst, OFC_INT c, OFC_SIZET size) {
    OFC_CHAR *pa;
    OFC_LPVOID ret;

    if (dst == OFC_NULL)
        ret = OFC_NULL;
    else {
        pa = (OFC_CHAR *) dst;

        while (size--)
            *pa++ = c;
        ret = dst;
    }

    return (ret);
}

static OFC_UINT8 cvtIn[] =
        {
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9,        /* '0' - '9' */
                100, 100, 100, 100, 100, 100, 100,        /* punctuation */
                10, 11, 12, 13, 14, 15, 16, 17, 18, 19,    /* 'A' - 'Z' */
                20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                30, 31, 32, 33, 34, 35,
                100, 100, 100, 100, 100, 100,        /* punctuation */
                10, 11, 12, 13, 14, 15, 16, 17, 18, 19,    /* 'a' - 'z' */
                20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                30, 31, 32, 33, 34, 35
        };

OFC_CORE_LIB OFC_ULONG
ofc_strtoul(const OFC_CHAR *string, OFC_CHAR **endPtr, OFC_INT base) {
    const OFC_CHAR *p;
    OFC_ULONG result = 0;
    OFC_UINT digit;
    OFC_INT anyDigits = 0;

    if (string == OFC_NULL)
        result = 0;
    else {
        /*
         * Skip any leading blanks.
         */
        p = string;
        while (OFC_ISSPACE(*p)) {
            p += 1;
        }

        /*
         * If no base was provided, pick one from the leading characters
         * of the string.
         */
        if (base == 0) {
            if (*p == '0') {
                p += 1;
                if (*p == 'x') {
                    p += 1;
                    base = 16;
                } else {
                    /*
                     * Must set anyDigits here, otherwise "0" produces a
                     * "no digits" error.
                     */
                    anyDigits = 1;
                    base = 8;
                }
            } else
                base = 10;
        } else if (base == 16) {
            /*
             * Skip a leading "0x" from hex numbers.
             */
            if ((p[0] == '0') && (p[1] == 'x')) {
                p += 2;
            }
        }

        /*
         * Sorry this code is so messy, but speed seems important.  Do
         * different things for base 8, 10, 16, and other.
         */
        if (base == 8) {
            for (;; p += 1) {
                digit = *p - '0';
                if (digit > 7) {
                    break;
                }
                result = (result << 3) + digit;
                anyDigits = 1;
            }
        } else if (base == 10) {
            for (;; p += 1) {
                digit = *p - '0';
                if (digit > 9) {
                    break;
                }
                result = (10 * result) + digit;
                anyDigits = 1;
            }
        } else if (base == 16) {
            for (;; p += 1) {
                digit = *p - '0';
                if (digit > ('z' - '0')) {
                    break;
                }
                digit = cvtIn[digit];
                if (digit > 15) {
                    break;
                }
                result = (result << 4) + digit;
                anyDigits = 1;
            }
        } else {
            for (;; p += 1) {
                digit = *p - '0';
                if (digit > ('z' - '0')) {
                    break;
                }
                digit = cvtIn[digit];
                if (digit >= (OFC_UINT) base) {
                    break;
                }
                result = result * base + digit;
                anyDigits = 1;
            }
        }

        /*
         * See if there were any digits at all.
         */
        if (!anyDigits) {
            p = string;
        }

        if (endPtr != OFC_NULL) {
            *endPtr = (OFC_CHAR *) p;
        }
    }
    return result;
}

OFC_CORE_LIB OFC_LONG
ofc_strtol(const OFC_CHAR *string, OFC_CHAR **endPtr, OFC_INT base) {
    const OFC_CHAR *p;
    OFC_LONG result;

    if (string == OFC_NULL)
        result = 0;
    else {
        /*
         * Skip any leading blanks.
         */
        p = string;
        while (OFC_ISSPACE(*p)) {
            p += 1;
        }

        /*
         * Check for a sign.
         */

        if (*p == '-') {
            p += 1;
            result = -((OFC_LONG) ofc_strtoul(p, endPtr, base));
        } else {
            if (*p == '+') {
                p += 1;
            }
            result = ofc_strtoul(p, endPtr, base);
        }

        if ((result == 0) && (endPtr != 0) && (*endPtr == p)) {
            *endPtr = (OFC_CHAR *) string;
        }
    }
    return result;
}

/*
 * Copyright Patrick Powell 1995
 * This code is based on code written by Patrick Powell (papowell@astart.com)
 * It may be used for any purpose as long as this notice remains intact
 * on all source code distributions
 */

static OFC_SIZET dopr(OFC_CHAR *buffer, OFC_SIZET maxlen,
                      OFC_CCHAR *format, va_list args);

static OFC_VOID fmtstr(OFC_CHAR *buffer, OFC_SIZET *currlen,
                       OFC_SIZET maxlen, OFC_CCHAR *value,
                       OFC_INT flags, OFC_SIZET min, OFC_SIZET max);

static OFC_VOID fmttstr(OFC_CHAR *buffer, OFC_SIZET *currlen,
                        OFC_SIZET maxlen, OFC_CTCHAR *value,
                        OFC_INT flags, OFC_SIZET min, OFC_SIZET max);

static OFC_VOID fmttastr(OFC_CHAR *buffer, OFC_SIZET *currlen,
                         OFC_SIZET maxlen, OFC_CTACHAR *value,
                         OFC_INT flags, OFC_SIZET min, OFC_SIZET max);

static OFC_VOID fmtint(OFC_CHAR *buffer, OFC_SIZET *currlen,
                       OFC_SIZET maxlen, OFC_INT64 value, OFC_INT base,
                       OFC_SIZET min, OFC_SIZET max, OFC_INT flags);

static OFC_VOID dopr_outch(OFC_CHAR *buffer, OFC_SIZET *currlen,
                           OFC_SIZET maxlen, OFC_CHAR c);

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

static OFC_SIZET
dopr(OFC_CHAR *buffer, OFC_SIZET maxlen, OFC_CCHAR *format, va_list args) {
    OFC_CHAR ch;
    OFC_CCHAR *strvalue;
    OFC_CTCHAR *tstrvalue;
    OFC_CTACHAR *tastrvalue;
    OFC_LONG value;
    OFC_SIZET min;
    OFC_SIZET max;
    OFC_INT state;
    OFC_INT flags;
    OFC_INT cflags;
    OFC_SIZET currlen;

    state = DP_S_DEFAULT;
    currlen = flags = cflags = min = 0;
    max = -1;
    ch = *format++;

    while (state != DP_S_DONE) {
        if (ch == '\0')
            state = DP_S_DONE;

        switch (state) {
            case DP_S_DEFAULT:
                if (ch == '%') {
                    state = DP_S_FLAGS;
                } else {
                    dopr_outch(buffer, &currlen, maxlen, ch);
                }
                ch = *format++;
                break;

            case DP_S_FLAGS:
                switch (ch) {
                    case '-':
                        flags |= DP_F_MINUS;
                        ch = *format++;
                        break;
                    case '+':
                        flags |= DP_F_PLUS;
                        ch = *format++;
                        break;
                    case ' ':
                        flags |= DP_F_SPACE;
                        ch = *format++;
                        break;
                    case '#':
                        flags |= DP_F_NUM;
                        ch = *format++;
                        break;
                    case '0':
                        flags |= DP_F_ZERO;
                        ch = *format++;
                        break;
                    default:
                        state = DP_S_MIN;
                        break;
                }
                break;

            case DP_S_MIN:
                if (OFC_ISDIGIT((OFC_UINT8) ch)) {
                    min = 10 * min + CHAR_TO_INT (ch);
                    ch = *format++;
                } else if (ch == '*') {
                    min = va_arg (args, OFC_INT);
                    ch = *format++;
                    state = DP_S_DOT;
                } else {
                    state = DP_S_DOT;
                }
                break;
            case DP_S_DOT:
                if (ch == '.') {
                    state = DP_S_MAX;
                    ch = *format++;
                } else {
                    state = DP_S_MOD;
                }
                break;
            case DP_S_MAX:
                if (OFC_ISDIGIT((OFC_UINT8) ch)) {
                    if (max < 0)
                        max = 0;
                    max = 10 * max + CHAR_TO_INT (ch);
                    ch = *format++;
                } else if (ch == '*') {
                    max = va_arg (args, OFC_INT);
                    ch = *format++;
                    state = DP_S_MOD;
                } else {
                    state = DP_S_MOD;
                }
                break;

            case DP_S_MOD:
                switch (ch) {
                    case 'h':
                        cflags = DP_C_SHORT;
                        ch = *format++;
                        break;
                    case 'l':
                        cflags = DP_C_LONG;
                        ch = *format++;
                        break;
                    default:
                        break;
                }
                state = DP_S_CONV;
                break;
            case DP_S_CONV:
                switch (ch) {
                    case 'b':
                        flags |= DP_F_UNSIGNED;
                        if (cflags == DP_C_SHORT) {
                            value = va_arg (args, OFC_UINT);
                        } else if (cflags == DP_C_LONG) {
                            value = (OFC_LONG) va_arg (args, OFC_ULONG);
                        } else {
                            value = (OFC_LONG) va_arg (args, OFC_UINT);
                        }
                        fmtint(buffer, &currlen, maxlen, value, 2, min, max, flags);
                        break;
                    case 'd':
                    case 'i':
                        if (cflags == DP_C_SHORT) {
                            value = va_arg (args, OFC_INT);
                        } else if (cflags == DP_C_LONG) {
                            value = va_arg (args, OFC_LONG);
                        } else {
                            value = va_arg (args, OFC_INT);
                        }
                        fmtint(buffer, &currlen, maxlen, value, 10, min, max, flags);
                        break;
                    case 'o':
                        flags |= DP_F_UNSIGNED;
                        if (cflags == DP_C_SHORT) {
                            value = va_arg (args, OFC_UINT);
                        } else if (cflags == DP_C_LONG) {
                            value = (OFC_LONG) va_arg (args, OFC_ULONG);
                        } else {
                            value = (OFC_LONG) va_arg (args, OFC_UINT);
                        }
                        fmtint(buffer, &currlen, maxlen, value, 8, min, max, flags);
                        break;
                    case 'u':
                        flags |= DP_F_UNSIGNED;
                        if (cflags == DP_C_SHORT) {
                            value = va_arg (args, OFC_UINT);
                        } else if (cflags == DP_C_LONG) {
                            value = (OFC_LONG) va_arg (args, OFC_ULONG);
                        } else {
                            value = (OFC_LONG) va_arg (args, OFC_UINT);
                        }
                        fmtint(buffer, &currlen, maxlen, value, 10, min, max, flags);
                        break;
                    case 'X':
                        flags |= DP_F_UP;
                    case 'x':
                        flags |= DP_F_UNSIGNED;
                        if (cflags == DP_C_SHORT) {
                            value = va_arg (args, OFC_UINT);
                        } else if (cflags == DP_C_LONG) {
                            value = (OFC_LONG) va_arg (args, OFC_ULONG);
                        } else {
                            value = (OFC_LONG) va_arg (args, OFC_UINT);
                        }
                        fmtint(buffer, &currlen, maxlen, value, 16, min, max, flags);
                        break;
                    case 'c':
                        dopr_outch(buffer, &currlen, maxlen, va_arg (args, OFC_INT));
                        break;
#if defined(__APPLE__)
                    case 'e':
                        strvalue = strerror(errno);
                        if (!strvalue)
                            strvalue = "(no error)";
                        if (max == -1) {
                            max = ofc_strlen(strvalue);
                        }
                        if (min > 0 && max >= 0 && min > max)
                            max = min;
                        fmtstr(buffer, &currlen, maxlen, strvalue, flags, min, max);
                        break;
#endif
                    case 's':
                        strvalue = va_arg (args, OFC_CHAR *);
                        if (!strvalue)
                            strvalue = "(null)";
                        if (max == -1) {
                            max = ofc_strlen(strvalue);
                        }
                        if (min > 0 && max >= 0 && min > max)
                            max = min;
                        fmtstr(buffer, &currlen, maxlen, strvalue, flags, min, max);
                        break;
                    case 'S':
                        tstrvalue = va_arg (args, OFC_TCHAR *);
                        if (!tstrvalue)
                            tstrvalue = TSTR("(null)");
                        if (max == -1) {
                            max = ofc_tstrlen(tstrvalue);
                        }
                        if (min > 0 && max >= 0 && min > max)
                            max = min;
                        fmttstr(buffer, &currlen, maxlen, tstrvalue, flags, min, max);
                        break;
                    case 'A':
                        tastrvalue = va_arg (args, OFC_TACHAR *);
                        if (!tastrvalue)
                            tastrvalue = TASTR("(null)");
                        if (max == -1) {
                            max = ofc_tastrlen(tastrvalue);
                        }
                        if (min > 0 && max >= 0 && min > max)
                            max = min;
                        fmttastr(buffer, &currlen, maxlen, tastrvalue, flags, min, max);
                        break;
                    case 'p':
                        flags |= DP_F_UNSIGNED;
                        strvalue = va_arg (args, OFC_CHAR *);
                        fmtint(buffer, &currlen, maxlen,
                               (OFC_ULONG_PTR) strvalue,
                               16, min, max, flags);
                        break;
                    case 'n':
                        if (cflags == DP_C_SHORT) {
                            OFC_SHORT *num;
                            num = va_arg (args, OFC_SHORT *);
                            *num = (OFC_SHORT) currlen;
                        } else if (cflags == DP_C_LONG) {
                            OFC_LONG *num;
                            num = va_arg (args, OFC_LONG *);
                            *num = (OFC_LONG) currlen;
                        } else {
                            OFC_INT *num;
                            num = va_arg (args, OFC_INT *);
                            *num = (OFC_INT) currlen;
                        }
                        break;
                    case '%':
                        dopr_outch(buffer, &currlen, maxlen, ch);
                        break;
                    case 'w':
                        /* not supported yet, treat as next char */
                        ch = *format++;
                        break;
                    default:
                        /* Unknown, skip */
                        break;
                }
                ch = *format++;
                state = DP_S_DEFAULT;
                flags = cflags = min = 0;
                max = -1;
                break;
            case DP_S_DONE:
                break;
            default:
                /* hmm? */
                break; /* some picky compilers need this */
        }
    }
    if (maxlen != 0) {
        if (currlen < maxlen - 1)
            buffer[currlen] = '\0';
        else if (maxlen > 0)
            buffer[maxlen - 1] = '\0';
    }

    return currlen;
}

static OFC_VOID
fmtstr(OFC_CHAR *buffer, OFC_SIZET *currlen, OFC_SIZET maxlen,
       OFC_CCHAR *value, OFC_INT flags, OFC_SIZET min, OFC_SIZET max) {
    OFC_SIZET padlen;
    OFC_SIZET strln;
    OFC_INT cnt = 0;

    strln = ofc_strlen(value);

    padlen = min - strln;

    if (padlen < 0)
        padlen = 0;
    if (flags & DP_F_MINUS)
        padlen = -padlen; /* Left Justify */

    while ((padlen > 0) && (cnt < max)) {
        dopr_outch(buffer, currlen, maxlen, ' ');
        --padlen;
        ++cnt;
    }
    while (*value && (cnt < max)) {
        dopr_outch(buffer, currlen, maxlen, *value++);
        ++cnt;
    }
    while ((padlen < 0) && (cnt < max)) {
        dopr_outch(buffer, currlen, maxlen, ' ');
        ++padlen;
        ++cnt;
    }
}

static OFC_VOID
fmttstr(OFC_CHAR *buffer, OFC_SIZET *currlen, OFC_SIZET maxlen,
        OFC_CTCHAR *value, OFC_INT flags, OFC_SIZET min, OFC_SIZET max) {
    OFC_SIZET padlen;
    OFC_SIZET strln;
    OFC_INT cnt = 0;

    strln = ofc_tstrlen(value);

    padlen = min - strln;

    if (padlen < 0)
        padlen = 0;
    if (flags & DP_F_MINUS)
        padlen = -padlen; /* Left Justify */

    while ((padlen > 0) && (cnt < max)) {
        dopr_outch(buffer, currlen, maxlen, ' ');
        --padlen;
        ++cnt;
    }
    while (*value && (cnt < max)) {
        dopr_outch(buffer, currlen, maxlen, (OFC_CCHAR) (*value++));
        ++cnt;
    }
    while ((padlen < 0) && (cnt < max)) {
        dopr_outch(buffer, currlen, maxlen, ' ');
        ++padlen;
        ++cnt;
    }
}

static OFC_VOID
fmttastr(OFC_CHAR *buffer, OFC_SIZET *currlen, OFC_SIZET maxlen,
         OFC_CTACHAR *value, OFC_INT flags, OFC_SIZET min, OFC_SIZET max) {
    OFC_SIZET padlen;
    OFC_SIZET strln;
    OFC_INT cnt = 0;

    strln = ofc_tastrlen (value);

    padlen = min - strln;

    if (padlen < 0)
        padlen = 0;
    if (flags & DP_F_MINUS)
        padlen = -padlen; /* Left Justify */

    while ((padlen > 0) && (cnt < max)) {
        dopr_outch(buffer, currlen, maxlen, ' ');
        --padlen;
        ++cnt;
    }
    while (*value && (cnt < max)) {
        dopr_outch(buffer, currlen, maxlen, (OFC_CCHAR) (*value++));
        ++cnt;
    }
    while ((padlen < 0) && (cnt < max)) {
        dopr_outch(buffer, currlen, maxlen, ' ');
        ++padlen;
        ++cnt;
    }
}

/* Have to handle DP_F_NUM (ie 0x and 0 alternates) */

static OFC_VOID
fmtint(OFC_CHAR *buffer, OFC_SIZET *currlen, OFC_SIZET maxlen,
       OFC_INT64 value, OFC_INT base, OFC_SIZET min, OFC_SIZET max,
       OFC_INT flags) {
#define MAX_CONVERT_PLACES 40
    OFC_INT signvalue = 0;
    OFC_UINT64 uvalue;
    OFC_CHAR convert[MAX_CONVERT_PLACES];
    OFC_INT place = 0;
    OFC_SIZET spadlen = 0; /* amount to space pad */
    OFC_SIZET zpadlen = 0; /* amount to zero pad */
    OFC_INT caps = 0;

    if (max < 0)
        max = 0;

    uvalue = value;

    if (!(flags & DP_F_UNSIGNED)) {
        if (value < 0) {
            signvalue = '-';
            uvalue = -value;
        } else {
            if (flags & DP_F_PLUS)  /* Do a sign (+/i) */
                signvalue = '+';
            else if (flags & DP_F_SPACE)
                signvalue = ' ';
        }
    }

    if (flags & DP_F_UP)
        caps = 1; /* Should characters be upper case? */

    do {
        convert[place++] =
                (caps ? "0123456789ABCDEF" : "0123456789abcdef")
                [uvalue % (OFC_UINT) base];
        uvalue = (uvalue / (OFC_UINT) base);
    } while (uvalue && (place < MAX_CONVERT_PLACES));

    if (place == MAX_CONVERT_PLACES)
        place--;
    convert[place] = 0;

    zpadlen = max - place;
    spadlen = min - OFC_MAX (max, place) - (signvalue ? 1 : 0);
    if (zpadlen < 0)
        zpadlen = 0;
    if (spadlen < 0)
        spadlen = 0;
    if (flags & DP_F_ZERO) {
        zpadlen = OFC_MAX(zpadlen, spadlen);
        spadlen = 0;
    }
    if (flags & DP_F_MINUS)
        spadlen = -spadlen; /* Left Justifty */

    /* Spaces */
    while (spadlen > 0) {
        dopr_outch(buffer, currlen, maxlen, ' ');
        --spadlen;
    }

    /* Sign */
    if (signvalue)
        dopr_outch(buffer, currlen, maxlen, signvalue);

    /* Zeros */
    if (zpadlen > 0) {
        while (zpadlen > 0) {
            dopr_outch(buffer, currlen, maxlen, '0');
            --zpadlen;
        }
    }

    /* Digits */
    while (place > 0)
        dopr_outch(buffer, currlen, maxlen, convert[--place]);

    /* Left Justified spaces */
    while (spadlen < 0) {
        dopr_outch(buffer, currlen, maxlen, ' ');
        ++spadlen;
    }
}

static OFC_VOID
dopr_outch(OFC_CHAR *buffer, OFC_SIZET *currlen, OFC_SIZET maxlen,
           OFC_CHAR c) {
    if (*currlen < maxlen) {
        buffer[(*currlen)] = c;
    }
    (*currlen)++;
}

OFC_CORE_LIB OFC_SIZET
ofc_vsnprintf(OFC_CHAR *str, OFC_SIZET count,
              OFC_CCHAR *fmt, va_list args) {
    OFC_SIZET res;

    if ((OFC_INT) count < 0)
        count = 0;

    res = dopr(str, count, fmt, args);

    return (res);
}

OFC_CORE_LIB OFC_SIZET
ofc_snprintf(OFC_CHAR *str, OFC_SIZET count, OFC_CCHAR *fmt, ...) {
    OFC_SIZET ret;
    va_list ap;

    va_start(ap, fmt);

    ret = ofc_vsnprintf(str, count, fmt, ap);

    va_end(ap);
    return ret;
}

OFC_CORE_LIB OFC_VOID
ofc_log(OFC_LOG_LEVEL level, OFC_CCHAR *fmt, ...)
{
  OFC_CHAR *obuf;
  OFC_SIZET len;

  va_list ap;

  va_start(ap, fmt);
  len = ofc_vsnprintf(OFC_NULL, 0, fmt, ap);
  va_end(ap);

  obuf = ofc_malloc(len + 1);
  va_start(ap, fmt);
  ofc_vsnprintf(obuf, len + 1, fmt, ap);
  va_end(ap);

  ofc_write_log(level, obuf, len);

  ofc_free(obuf);
}

OFC_CORE_LIB OFC_VOID
ofc_printf(OFC_CCHAR *fmt, ...) {
    OFC_CHAR *obuf;
    OFC_SIZET len;
    OFC_SIZET tlen;
    OFC_CHAR timestamp[20];

    va_list ap;

    va_start(ap, fmt);
    len = ofc_vsnprintf(OFC_NULL, 0, fmt, ap);
    va_end(ap);

    obuf = ofc_malloc(len + 1);
    va_start(ap, fmt);
    ofc_vsnprintf(obuf, len + 1, fmt, ap);
    va_end(ap);

    tlen = ofc_snprintf(timestamp, 20, "%u ", ofc_time_get_now());
    ofc_write_stdout(timestamp, tlen);

    ofc_write_stdout(obuf, len);

    ofc_free(obuf);
}

OFC_CORE_LIB OFC_CHAR *
ofc_saprintf(OFC_CCHAR *fmt, ...) {
    OFC_CHAR *obuf;
    OFC_SIZET len;

    va_list ap;

    va_start(ap, fmt);
    len = ofc_vsnprintf(OFC_NULL, 0, fmt, ap);
    va_end(ap);

    obuf = ofc_malloc(len + 1);
    va_start(ap, fmt);
    ofc_vsnprintf(obuf, len + 1, fmt, ap);
    va_end(ap);

    return (obuf);
}

#if 0
OFC_CORE_LIB OFC_VOID 
ofc_trace_init (OFC_VOID)
{
  struct trace_t *trace ;

  trace = ofc_get_trace() ;
  if (trace == OFC_NULL)
    {
      trace = ofc_malloc (sizeof (struct trace_t)) ;
      if (trace != OFC_NULL)
    {
      ofc_memset (trace, '\0', sizeof (struct trace_t)) ;
      trace->trace_offset = 0 ;
      trace->trace_lock = ofc_lock_init() ;
      ofc_set_trace (trace) ;
      ofc_trace ("Trace Buffer Initialized\n") ;
    }
    }
}

OFC_CORE_LIB OFC_VOID
ofc_trace_destroy (OFC_VOID)
{
  struct trace_t *trace ;

  trace = ofc_get_trace() ;
  if (trace != OFC_NULL)
    {
      ofc_lock_destroy(trace->trace_lock);
      ofc_set_trace(OFC_NULL);
      ofc_free (trace);
    }
}

OFC_CORE_LIB OFC_VOID
ofc_trace(OFC_CCHAR *fmt,...)
{
  OFC_CHAR *obuf ;
  OFC_CHAR *obuf2 ;
  OFC_SIZET len ;
  OFC_SIZET olen ;
  struct trace_t *trace ;
  OFC_PROCESS_ID pid ;

  va_list ap ;

  va_start(ap, fmt) ;

  len = ofc_vsnprintf(OFC_NULL, 0, fmt, ap) ;
  va_end(ap) ;
  obuf = ofc_malloc (len+1) ;
  va_start(ap, fmt) ;
  ofc_vsnprintf(obuf, len+1, fmt, ap) ;
  va_end(ap) ;

  /*
   * prepend process id
   */
  pid = ofc_process_get() ;
  obuf2 = ofc_saprintf ("%8d: %s\n", pid, obuf) ;
  len = ofc_strlen (obuf2) ;
  ofc_free (obuf) ;

  trace = ofc_get_trace() ;
  if (trace != OFC_NULL && ((len + 1) < OFC_TRACE_LEN))
    {
      ofc_lock (trace->trace_lock) ;
      olen = OFC_MIN (len, OFC_TRACE_LEN -
             trace->trace_offset - 1) ;
      ofc_memcpy (trace->trace_buf + trace->trace_offset,
           obuf2, olen) ;
      *(trace->trace_buf + trace->trace_offset + olen) = '\0' ;

      trace->trace_offset += (olen + 1) ;
      if ((trace->trace_offset) == OFC_TRACE_LEN)
    trace->trace_offset = 0 ;

      len -= olen ;
      if (len > 0)
    {
      ofc_memcpy (trace->trace_buf + trace->trace_offset,
               obuf2 + olen, len) ;
      *(trace->trace_buf + trace->trace_offset + len) =
        '\0' ;
      trace->trace_offset += (len + 1) ;
    }
      ofc_unlock (trace->trace_lock) ;
    }
  ofc_free (obuf2) ;
}

OFC_CORE_LIB OFC_VOID ofc_dump_trace(OFC_VOID)
{
  struct trace_t *trace ;

  OFC_CHAR *obuf ;
  OFC_SIZET olen ;
  OFC_INT off ;

  trace = ofc_get_trace() ;
  if (trace != OFC_NULL)
    {
      off = trace->trace_offset ;
      do
    {
      obuf = trace->trace_buf + off ;
      olen = ofc_strlen (obuf) ;
      ofc_printf ("%s", obuf) ;
      off += (olen + 1) ;
      if (off == OFC_TRACE_LEN)
        off = 0 ;
    }
      while (off != trace->trace_offset) ;
    }
}
#endif

/* xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx */
#define hextoa(a) ((a) >= 10 ? 'A' + ((a) - 10) : '0' + (a))
#define atohex(a) ((a) >= '0' && (a) <= '9' ? ((a) - '0') : \
    (a) >= 'a' && (a) <= 'f' ? ((a) - 'a' + 10) : ((a) - 'A' + 10))

OFC_VOID
ofc_uuidtoa(OFC_UUID uuid, OFC_CHAR *out) {
    OFC_INT i;
    OFC_INT val;

    for (i = 0; i < 4; i++) {
        val = uuid[i];
        *out++ = hextoa(val >> 4);
        *out++ = hextoa(val & 0x0F);
    }
    *out++ = '-';
    for (i = 4; i < 6; i++) {
        val = uuid[i];
        *out++ = hextoa(val >> 4);
        *out++ = hextoa(val & 0x0F);
    }
    *out++ = '-';
    for (i = 6; i < 8; i++) {
        val = uuid[i];
        *out++ = hextoa(val >> 4);
        *out++ = hextoa(val & 0x0F);
    }
    *out++ = '-';
    for (i = 8; i < 10; i++) {
        val = uuid[i];
        *out++ = hextoa(val >> 4);
        *out++ = hextoa(val & 0x0F);
    }
    *out++ = '-';
    for (i = 10; i < 16; i++) {
        val = uuid[i];
        *out++ = hextoa(val >> 4);
        *out++ = hextoa(val & 0x0F);
    }
    *out++ = '\0';
}

OFC_VOID
ofc_atouuid(const OFC_CHAR *in, OFC_UUID uuid) {
    OFC_INT i;
    OFC_INT val;

    for (i = 0; i < 4; i++) {
        val = atohex(*in) << 4;
        in++;
        val |= atohex(*in);
        in++;
        uuid[i] = val;
    }
    in++;
    for (i = 4; i < 6; i++) {
        val = atohex(*in) << 4;
        in++;
        val |= atohex(*in);
        in++;
        uuid[i] = val;
    }
    in++;
    for (i = 6; i < 8; i++) {
        val = atohex(*in) << 4;
        in++;
        val |= atohex(*in);
        in++;
        uuid[i] = val;
    }
    in++;
    for (i = 8; i < 10; i++) {
        val = atohex(*in) << 4;
        in++;
        val |= atohex(*in);
        in++;
        uuid[i] = val;
    }
    in++;
    for (i = 10; i < 16; i++) {
        val = atohex(*in) << 4;
        in++;
        val |= atohex(*in);
        in++;
        uuid[i] = val;
    }
}

/*
 * ofc_substr - Is string s2 in s1?
 *
 * \param s2
 * string to find inside of s1
 *
 * \param s1
 * string that may contain s2
 *
 * \returns
 * -1 if string s1 does not contain string s2
 * >= 0 location in s1 where string s2 starts
 */
OFC_CORE_LIB
OFC_INT ofc_substr(OFC_CCHAR *s2, OFC_CCHAR *s1)
{
  OFC_INT s2_index;
  OFC_INT s1_index;
  OFC_SIZET s1_len;
  OFC_SIZET s2_len;
  OFC_INT ret;

  s1_index = 0;
  s2_index = 0;
  s1_len = ofc_strlen(s1);
  s2_len = ofc_strlen(s2);

  for( ; (s1_index < s1_len) && (s2_index != s2_len); s1_index++)
    {
      if (s2[s2_index] == s1[s1_index])
        {
	  s2_index++;
        }
      else
        {
	  if (s2_index > 0)
            {
	      s1_index -= s2_index;
            }
	  s2_index = 0;
        }
    }

  if (s2_index < s2_len)
    ret = -1;
  else
    ret = s1_index - s2_index;

  return (ret);
}
