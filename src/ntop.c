/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 */
/* Copyright (c) 1996 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/net.h"

#define IN6ADDRSZ 16
#define INADDRSZ 4
#define INT16SZ 2

static const OFC_CHAR *ofc_net_ntop4(const OFC_IPADDR *src, OFC_CHAR *dst, OFC_SIZET size);

static const OFC_CHAR *ofc_net_ntop6(const OFC_IPADDR *src, OFC_CHAR *dst, OFC_SIZET size);

const OFC_CHAR *
ofc_ntop(const OFC_IPADDR *src, OFC_CHAR *dst, OFC_SIZET size) {
    OFC_FAMILY_TYPE family;

    family = src->ip_version;
    switch (family) {
        case OFC_FAMILY_IP:
            return (ofc_net_ntop4(src, dst, size));
        case OFC_FAMILY_IPV6:
            return (ofc_net_ntop6(src, dst, size));
        default:
            return (OFC_NULL);
    }
}

static const OFC_CHAR *
ofc_net_ntop4(const OFC_IPADDR *ip, OFC_CHAR *dst, OFC_SIZET size) {
    static const OFC_CHAR fmt[] = "%u.%u.%u.%u";
    OFC_CHAR tmp[IPSTR_LEN];
    OFC_UCHAR *src;

    src = (OFC_UCHAR *) &ip->u.ipv4.addr;

    ofc_snprintf(tmp, IPSTR_LEN, fmt, src[3], src[2], src[1], src[0]);
    if (ofc_strnlen(tmp, IPSTR_LEN) > size) {
        return (OFC_NULL);
    }
    ofc_strcpy(dst, tmp);
    return (dst);
}

static const OFC_CHAR *
ofc_net_ntop6(const OFC_IPADDR *ip, OFC_CHAR *dst, OFC_SIZET size) {
    /*
     * Note that int32_t and int16_t need only be "at least" large enough
     * to contain a value of the specified size.  On some systems, like
     * Crays, there is no such thing as an integer variable with 16 bits.
     * Keep this in mind if you think this function should have been coded
     * to use pointer overlays.  All the world's not a VAX.
     */
    OFC_CHAR tmp[IP6STR_LEN], *tp;
    struct {
        OFC_INT base;
        OFC_INT len;
    } best, cur;
    OFC_UINT16 words[IN6ADDRSZ / INT16SZ];
    OFC_INT i;
    const OFC_UCHAR *src;
    OFC_IPADDR ipv4;

    src = (OFC_UCHAR *) &ip->u.ipv6._s6_addr;
    ofc_memset(tmp, '\0', IP6STR_LEN);
    /*
     * Preprocess:
     *Copy the input (bytewise) array into a wordwise array.
     *Find the longest run of 0x00's in src[] for :: shorthanding.
     */

    ofc_memset(words, '\0', sizeof words);
    for (i = 0; i < IN6ADDRSZ; i++)
        words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));
    best.base = -1;
    cur.base = -1;
    for (i = 0; i < (IN6ADDRSZ / INT16SZ); i++) {
        if (words[i] == 0) {
            if (cur.base == -1) {
                cur.base = i;
                cur.len = 1;
            } else
                cur.len++;
        } else {
            if (cur.base != -1) {
                if (best.base == -1 || cur.len > best.len)
                    best = cur;
                cur.base = -1;
            }
        }
    }
    if (cur.base != -1) {
        if (best.base == -1 || cur.len > best.len)
            best = cur;
    }
    if (best.base != -1 && best.len < 2)
        best.base = -1;

    /*
     * Format the result.
     */
    ofc_memset(tmp, '\0', IP6STR_LEN);
    tp = tmp;
    for (i = 0; i < (IN6ADDRSZ / INT16SZ); i++) {
        /* Are we inside the best run of 0x00's? */
        if (best.base != -1 && i >= best.base &&
            i < (best.base + best.len)) {
            if (i == best.base)
                *tp++ = ':';
            continue;
        }
        /* Are we following an initial run of 0x00s or any real hex? */
        if (i != 0)
            *tp++ = ':';
        /* Is this address an encapsulated IPv4? */
        if (i == 6 && best.base == 0 &&
            (best.len == 6 || (best.len == 5 && words[5] == 0xffff))) {
            ipv4.ip_version = OFC_FAMILY_IP;
            ipv4.u.ipv4.addr = OFC_NET_NTOL (src, 12);
            if (!ofc_net_ntop4(&ipv4, tp, sizeof tmp - (tp - tmp)))
                return (OFC_NULL);
            tp += ofc_strnlen(tp, sizeof tmp - (tp - tmp));
            break;
        }
        ofc_snprintf(tp,
                     IP6STR_LEN - ofc_strnlen(tp, sizeof tmp - (tp - tmp)),
                     "%x", words[i]);
        tp += ofc_strnlen(tp, sizeof tmp - (tp - tmp));
    }
    /* Was it a trailing run of 0x00's? */
    if (best.base != -1 && (best.base + best.len) == (IN6ADDRSZ / INT16SZ))
        *tp++ = ':';
    *tp++ = '\0';

    /*
     * Check for overflow, copy, and we're done.
     */
    if ((tp - tmp) > size) {
        return (OFC_NULL);
    }
    ofc_strcpy(dst, tmp);
    return (dst);
}


static OFC_BOOL ofc_net_pton4(const OFC_CHAR *src, OFC_IPADDR *dst);

static OFC_BOOL ofc_net_pton6(const OFC_CHAR *src, OFC_IPADDR *dst);

/* OFC_INT
 * inet_pton(af, src, dst)
 *convert from presentation format (which usually means ASCII printable)
 *to network format (which is usually some kind of binary format).
 * return:
 *1 if the address was valid for the specified address family
 *0 if the address wasn't valid (`dst' is untouched in this case)
 *-1 if some other error occurred (`dst' is untouched in this case, too)
 * author:
 *Paul Vixie, 1996.
 */
OFC_INT
ofc_pton(const OFC_CHAR *src, OFC_IPADDR *dst) {
    OFC_INT ret;

    if (ofc_strchr(src, ':') != OFC_NULL)
        ret = ofc_net_pton6(src, dst);
    else
        ret = ofc_net_pton4(src, dst);
    return (ret);

}

/* OFC_INT
 * inet_pton4(src, dst)
 *like inet_aton() but without all the hexadecimal, octal (with the
 *exception of 0) and shorthand.
 * return:
 *1 if `src' is a valid dotted quad, else 0.
 * notice:
 *does not touch `dst' unless it's returning 1.
 * author:
 *Paul Vixie, 1996.
 */
static OFC_BOOL
ofc_net_pton4(const OFC_CHAR *src, OFC_IPADDR *dst) {
    OFC_INT saw_digit, octets, ch;
    OFC_UCHAR tmp[INADDRSZ], *tp;

    saw_digit = 0;
    octets = 0;
    *(tp = tmp) = 0;
    while ((ch = *src++) != '\0') {

        if (ch >= '0' && ch <= '9') {
            OFC_UINT16 new = *tp * 10 + (ch - '0');

            if (saw_digit && *tp == 0)
                return (0);
            if (new > 255)
                return (0);
            *tp = (OFC_UCHAR) new;
            if (!saw_digit) {
                if (++octets > 4)
                    return (0);
                saw_digit = 1;
            }
        } else if (ch == '.' && saw_digit) {
            if (octets == 4)
                return (0);
            *++tp = 0;
            saw_digit = 0;
        } else
            return (0);
    }
    if (octets < 4)
        return (0);

    dst->ip_version = OFC_FAMILY_IP;
    dst->u.ipv4.addr = OFC_NET_NTOL (tmp, 0);
    return (1);
}

/* OFC_INT
 * inet_pton6(src, dst)
 *convert presentation level address to network order binary form.
 * return:
 *1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *(1) does not touch `dst' unless it's returning 1.
 *(2) :: in a full address is silently ignored.
 * credit:
 *inspired by Mark Andrews.
 * author:
 *Paul Vixie, 1996.
 */
static OFC_BOOL
ofc_net_pton6(const OFC_CHAR *src, OFC_IPADDR *dst) {
    static const OFC_CHAR xdigits[] = "0123456789abcdef";
    OFC_UCHAR tmp[IN6ADDRSZ], *tp, *endp, *colonp;
    const OFC_CHAR *curtok;
    OFC_UINT ch, saw_xdigit;
    OFC_UINT16 val;
    OFC_IPADDR ipv4;
    OFC_CHAR sch;

    tp = ofc_memset(tmp, '\0', IN6ADDRSZ);
    endp = tp + IN6ADDRSZ;
    colonp = OFC_NULL;
    /* Leading :: requires some special handling. */
    if (*src == ':')
        if (*++src != ':') {
            return (0);
        }
    curtok = src;
    saw_xdigit = 0;
    val = 0;
    while (*src != '\0') {
        const OFC_UCHAR *pch;

        sch = *src++;
        ch = OFC_TOLOWER (sch);
        pch = (OFC_UCHAR *) ofc_strchr(xdigits, ch);
        if (pch != OFC_NULL) {
            val <<= 4;
            val |= (pch - (OFC_UCHAR *) xdigits);
            saw_xdigit = 1;
            continue;
        }
        if (ch == ':') {
            curtok = src;
            if (!saw_xdigit) {
                if (colonp) {
                    return (0);
                }
                colonp = tp;
                continue;
            } else if (*src == '\0') {
                return (0);
            }
            if (tp + INT16SZ > endp) {
                return (0);
            }
            *tp++ = (OFC_UCHAR) (val >> 8) & 0xff;
            *tp++ = (OFC_UCHAR) val & 0xff;
            saw_xdigit = 0;
            val = 0;
            continue;
        }
        if (ch == '.' && ((tp + INADDRSZ) <= endp) &&
            ofc_net_pton4(curtok, &ipv4) > 0) {
            OFC_NET_LTON (tp, 0, ipv4.u.ipv4.addr);
            tp += INADDRSZ;
            saw_xdigit = 0;
            break;/* '\0' was seen by inet_pton4(). */
        }
        return (0);
    }
    if (saw_xdigit) {
        if (tp + INT16SZ > endp) {
            return (0);
        }
        *tp++ = (OFC_UCHAR) (val >> 8) & 0xff;
        *tp++ = (OFC_UCHAR) val & 0xff;
    }
    if (colonp != OFC_NULL) {
        /*
         * Since some memmove()'s erroneously fail to handle
         * overlapping regions, we'll do the shift by hand.
         */
        const OFC_INT n = (OFC_INT) (tp - colonp);
        OFC_INT i;

        if (tp == endp) {
            return (0);
        }
        for (i = 1; i <= n; i++) {
            endp[-i] = colonp[n - i];
            colonp[n - i] = 0;
        }
        tp = endp;
    }
    if (tp != endp) {
        return (0);
    }
    dst->ip_version = OFC_FAMILY_IPV6;
    ofc_memcpy(&dst->u.ipv6, tmp, IN6ADDRSZ);
    return (1);
}
