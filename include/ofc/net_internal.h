/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_NET_INTERNAL_H__)
#define __OFC_NET_INTERNAL_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/**
 * u8 - Reference a byte at a location.  The location is typically a short or
 * long member of some structure with a byte position.
 *
 * Accepts:
 *    a - reference to short or long value
 *    o - Byte position of the structure
 *
 * For example:
 *    mybyte = u8(myword, 1)    - return byte 1 of myword
 *    u8(mylong,3) = mybyte ;   - Set byte 3 of mylong to mybyte
 */
#define u8(a, o) *((OFC_UINT8*)(a)+o)
/**
 * utou8 - Map a byte from one location to another
 *
 * Take a byte from some position in a word or long and map it to a
 * some other short or long member of a structure.
 *
 * Accepts:
 *    d - reference to the destination short or long value
 *    a - short or long value as the source
 *    o - Byte position in the destination and source
 *
 * For example:
 *    utou8(smbmsg.wordvalue, myword, 1) - Map byte 1 of myword to wordvalue
 */
#define u16tou8(d, a, o) u8(d,(1-o))=((OFC_UINT8)((a>>(o*8))&0xFF))
#define u32tou8(d, a, o) u8(d,(3-o))=((OFC_UINT8)((a>>(o*8))&0xFF))
#if defined(OFC_64BIT_INTEGER)
#define u64tou8(d, a, o) u8(d,(7-o))=((OFC_UINT8)((a>>(o*8))&0xFF))
#endif
#define u16toxu8(d, a, o) u8(d,o)=((OFC_UINT8)((a>>(o*8))&0xFF))
#define u32toxu8(d, a, o) u8(d,o)=((OFC_UINT8)((a>>(o*8))&0xFF))
#if defined(OFC_64BIT_INTEGER)
#define u64toxu8(d, a, o) u8(d,o)=((OFC_UINT8)((a>>(o*8))&0xFF))
#endif
/**
 * u8tou16 - Return a word containing a byte from the source
 * u8tou32 - Return a long word containing a byte from the source
 *
 * Mask off a byte from a short or long
 *
 * Accepts:
 *    a - Reference short or long
 *    o - Byte position in short or long
 *
 * For Example:
 *    myword = u8tou16(smbmsg.word, 1) - Mask off the first byte of word
 *    mylong = u8tou32(smbmsg.long, 3) - Mask off the third byte
 */
#define u8tou16(a, o) ((OFC_UINT16)u8(a,o)<<((1-o)*8))
#define u8tou32(a, o) ((OFC_UINT32)u8(a,o)<<((3-o)*8))
#if defined(OFC_64BIT_INTEGER)
#define u8tou64(a, o) ((OFC_UINT64)u8(a,o)<<((7-o)*8))
#endif
#define u8toxu16(a, o) ((OFC_UINT16)u8(a,o)<<(o*8))
#define u8toxu32(a, o) ((OFC_UINT32)u8(a,o)<<(o*8))
#if defined(OFC_64BIT_INTEGER)
#define u8toxu64(a, o) ((OFC_UINT64)u8(a,o)<<(o*8))
#endif

/**
 * OFC_NET_NTOS - dereference a smb short value
 *
 * Accepts:
 *    a - reference to smb short value
 *
 * For Example:
 *    myshort = smbtos(smb.word) - Return the smb short value
 */
#define OFC_NET_NTOS(a, o) (u8tou16((OFC_UINT8*)a+o,0)|u8tou16((OFC_UINT8*)a+o,1))
/**
 * OFC_NET_NTOS - dereference a smb short value
 *
 * Accepts:
 *    a - reference to smb short value
 *
 * For Example:
 *    myshort = smbtos(smb.word) - Return the smb short value
 */
#define OFC_NET_NTOL(a, o) (u8tou32((OFC_UINT8*)a+o,0)|u8tou32((OFC_UINT8*)a+o,1)|u8tou32((OFC_UINT8*)a+o,2)|u8tou32((OFC_UINT8*)a+o,3))
#if defined(OFC_64BIT_INTEGER)
#define OFC_NET_NTOLL(a, o) (u8tou64((OFC_UINT8*)a+o,0)|u8tou64((OFC_UINT8*)a+o,1)|u8tou64((OFC_UINT8*)a+o,2)|u8tou64((OFC_UINT8*)a+o,3)|u8tou64((OFC_UINT8*)a+o,4)|u8tou64((OFC_UINT8*)a+o,5)|u8tou64((OFC_UINT8*)a+o,6)|u8tou64((OFC_UINT8*)a+o,7))
#endif
#define OFC_NET_NTOC(a, o) (u8((OFC_UINT8*)a+o,0))
#define OFC_NET_SMBTOS(a, o) (u8toxu16((OFC_UINT8*)a+o,0)|u8toxu16((OFC_UINT8*)a+o,1))
#define OFC_NET_SMBTOL(a, o) (u8toxu32((OFC_UINT8*)a+o,0)|u8toxu32((OFC_UINT8*)a+o,1)|u8toxu32((OFC_UINT8*)a+o,2)|u8toxu32((OFC_UINT8*)a+o,3))
#if defined(OFC_64BIT_INTEGER)
#define OFC_NET_SMBTOLL(a, o) (u8toxu64((OFC_UINT8*)a+o,0)|u8toxu64((OFC_UINT8*)a+o,1)|u8toxu64((OFC_UINT8*)a+o,2)|u8toxu64((OFC_UINT8*)a+o,3)|u8toxu64((OFC_UINT8*)a+o,4)|u8toxu64((OFC_UINT8*)a+o,5)|u8toxu64((OFC_UINT8*)a+o,6)|u8toxu64((OFC_UINT8*)a+o,7))
#endif
#define OFC_NET_SMBTOC(a, o) (u8((OFC_UINT8*)a+o,0))
/**
 * OFC_NET_STON - Store an smb short value
 * OFC_NET_LTON - Store an smb long value
 *
 * Accepts:
 *    d - destination smb value
 *    a - Value to store
 *
 * For Example:
 *    stosmb (smb.word, myword) ;
 *    ltosmb (smb.long, mylong) ;
 */
#define OFC_NET_STON(d, o, a) {u16tou8(((OFC_UINT8*)d+o),(a),0);u16tou8(((OFC_UINT8*)d+o),(a),1);}
#define OFC_NET_LTON(d, o, a) {u32tou8(((OFC_UINT8*)d+o),(a),0);u32tou8(((OFC_UINT8*)d+o),(a),1);u32tou8(((OFC_UINT8*)d+o),(a),2);u32tou8(((OFC_UINT8*)d+o),(a),3);}
#define OFC_NET_LLTON(d, o, a) {u64tou8(((OFC_UINT8*)d+o),(a),0);u64tou8(((OFC_UINT8*)d+o),(a),1);u64tou8(((OFC_UINT8*)d+o),(a),2);u64tou8(((OFC_UINT8*)d+o),(a),3);u64tou8(((OFC_UINT8*)d+o),(a),4);u64tou8(((OFC_UINT8*)d+o),(a),5);u64tou8(((OFC_UINT8*)d+o),(a),6);u64tou8(((OFC_UINT8*)d+o),(a),7);}
#define OFC_NET_CTON(d, o, a) (u8((OFC_UINT8*)d+o,0)=((OFC_UINT8)(a)&0xFF))
#define OFC_NET_STOSMB(d, o, a) {u16toxu8(((OFC_UINT8*)d+o),(a),0);u16toxu8(((OFC_UINT8*)d+o),(a),1);}
#define OFC_NET_LTOSMB(d, o, a) {u32toxu8(((OFC_UINT8*)d+o),(a),0);u32toxu8(((OFC_UINT8*)d+o),(a),1);u32toxu8(((OFC_UINT8*)d+o),(a),2);u32toxu8(((OFC_UINT8*)d+o),(a),3);}
#define OFC_NET_LLTOSMB(d, o, a) {u64toxu8(((OFC_UINT8*)d+o),(a),0);u64toxu8(((OFC_UINT8*)d+o),(a),1);u64toxu8(((OFC_UINT8*)d+o),(a),2);u64toxu8(((OFC_UINT8*)d+o),(a),3);u64toxu8(((OFC_UINT8*)d+o),(a),4);u64toxu8(((OFC_UINT8*)d+o),(a),5);u64toxu8(((OFC_UINT8*)d+o),(a),6);u64toxu8(((OFC_UINT8*)d+o),(a),7);}
#define OFC_NET_CTOSMB(d, o, a) (u8((OFC_UINT8*)d+o,0)=((OFC_UINT8)(a)&0xFF))

#endif
