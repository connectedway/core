/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_MESSAGE_H__)
#define __OFC_MESSAGE_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/config.h"
#include "ofc/net.h"

/**
 * \defgroup message Open Files Message Handling Facility
 *
 * The Message Handling Facility is designed to handle all aspects of network
 * message transfer.  This includes 
 *
 * - Byte Packing
 * - endianess
 * - heap allocation
 * - buffering
 * - dynamic formatted messages (sequential vs. fixed fields)
 * - data and parameter blocks
 *
 * Messages can be used by any application.  Within Open Files, though, they
 * are used by the Socket facility.  In other words, calls through the
 * Socket layer will send data from a message, and will receive data into
 * a message.
 *
 * Buffering is an important aspect to the messages.  For instance, with a 
 * stream socket, there is no guarantee that all the data wishing to be sent
 * will be sent in a single call.  Subsequent calls may be necessary for all
 * the data to be sent.  When using a 'message', pointers to the unsent
 * data and remaining byte count is maintained by the Socket layer freeing
 * the application from doing that bookkeeping.
 *
 * With receive, similar buffering can occur.  The application may wish not
 * to processing incoming data until a certain number of bytes are received
 * (i.e. the protocol packet).  One call may not return all the data so
 * subsequent calls may be necessary.  The Socket layer will update 
 * counts within the 'message' to reflect the amount of data received.
 */

/** \{ */

/**
 * Type of message allocation
 */
typedef enum {
    /**
     * Message is statically allocated and should not be freed
     */
    MSG_ALLOC_STATIC,
    /**
     * Message is allocated from the heap and should be freed when complete
     */
    MSG_ALLOC_HEAP,
    /**
     * Message is allocated on the stack and should be copied to an allocated
     * buffer if it must be queued.
     */
    MSG_ALLOC_AUTO,
} MSG_ALLOC_TYPE;

/**
 * Endianess of message
 */
typedef enum {
    /**
     * Message should be formatted Little Endian
     */
    MSG_ENDIAN_LITTLE,
    /**
     * Message should be formatted Big Endian
     */
    MSG_ENDIAN_BIG
} MSG_ENDIAN;

/**
 * Field Sizes
 */
#define SIZEOF_U8 1
#define SIZEOF_I8 1
#define SIZEOF_U16 2
#define SIZEOF_I16 2
#define SIZEOF_U32 4
#define SIZEOF_I32 4
#define SIZEOF_U64 8
#define SIZEOF_I64 8

/**
 * Format of a Message Header
 */
typedef struct _OFC_MESSAGE {
    /**
     * Type of Allocation
     */
    MSG_ALLOC_TYPE alloc;
    /**
     * Endianess of Message
     */
    MSG_ENDIAN endian;
    /**
     * Size of Message
     */
    OFC_SIZET length;
    /**
     * Source or destination IP address of Message for UDP
     */
    OFC_IPADDR ip;
    /**
     * Source or destination port of message for UDP
     */
    OFC_UINT16 port;
    /**
     * Message Data
     */
    OFC_CHAR *msg;
    /**
     * Current Pointer into Message
     */
    OFC_INT offset;
    /**
     * Current Size of Message
     */
    OFC_SIZET send_size;
    /**
     * Number of bytes still to be sent
     */
    OFC_SIZET count;
    /**
     * FIFO 1 Pointer.  FIFOs are useful for certain protocols that place
     * data sequentially at a particular offset in the message.
     */
    OFC_INT FIFO1;
    /**
     * The amount of space left in the fifo
     */
    OFC_SIZET FIFO1rem;
    /**
     * FIFO base
     */
    OFC_INT FIFO1base;
    /**
     * FIFO size
     */
    OFC_INT FIFO1size;
    /**
     * Parameter offset
     */
    OFC_INT param_offset;
    /**
     * Parameter Length
     */
    OFC_SIZET param_len;
    /**
     * Message Base
     */
    OFC_INT base;
#if defined(OFC_MESSAGE_DEBUG)
    /**
     * link to next message on debug lists
     */
    struct _OFC_MESSAGE * dbgnext ;
    /**
     * link to previous message on debug liests
     */
    struct _OFC_MESSAGE * dbgprev ;
    /**
     * Pointer to return address of caller that allocated message
     */
#if defined(__GNUC__)
    OFC_VOID *caller1 ;
    OFC_VOID *caller2 ;
    OFC_VOID *caller3 ;
    OFC_VOID *caller4 ;
#else
    OFC_VOID *caller ;
#endif
#endif
    OFC_VOID *context;
    OFC_BOOL destroy_after_send;

} OFC_MESSAGE;

#if defined(__cplusplus)
extern "C"
{
#endif
#if defined(OFC_MESSAGE_DEBUG)
/**
 * Initialize the message debugging facility
 */
OFC_CORE_LIB OFC_VOID
ofc_message_debug_init (OFC_VOID);
OFC_CORE_LIB OFC_VOID
ofc_message_debug_destroy(OFC_VOID);
#endif
/**
 * Create a TCP style message
 *
 * \param msgType
 * Allocation Type of Message
 *
 * \param msgDataLength
 * Length of message to allocate
 *
 * \param msgData
 * Message Data for static or stack allocated messages
 *
 * \returns
 * Pointer to message header
 */
OFC_CORE_LIB OFC_MESSAGE *
ofc_message_create(MSG_ALLOC_TYPE msgType,
                   OFC_SIZET msgDataLength,
                   OFC_VOID *msgData);
/**
 * Creates a UDP stlye message
 *
 * \param msgType
 * Allocation Type of Message
 *
 * \param msgDataLength
 * Length of message to allocate
 *
 * \param msgData
 * Message Data for static or stack allocated messages
 *
 * \param ip
 * IP address to send message to or received from
 *
 * \param port
 * Port to send message to or received from
 *
 * \returns
 * Pointer to message header
 */
OFC_CORE_LIB OFC_MESSAGE *
ofc_datagram_create(MSG_ALLOC_TYPE msgType,
                    OFC_SIZET msgDataLength,
                    OFC_CHAR *msgData,
                    OFC_IPADDR *ip,
                    OFC_UINT16 port);
/**
 * Destroy a message
 *
 * \param msg
 * The message header to destroy
 */
OFC_CORE_LIB OFC_VOID
ofc_message_destroy(OFC_MESSAGE *msg);
/**
 * Test whether all bytes have been sent or received.
 *
 * \param msg
 * The message to test whether the buffers have been drained
 *
 * \returns
 * OFC_TRUE if the message is done, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_done(OFC_MESSAGE *msg);
/**
 * Return the address of the begining of message buffer.
 *
 * \param msg
 * Message to get the buffer pointer from
 *
 * \returns
 * Pointer to the message buffer
 */
OFC_CORE_LIB OFC_VOID *
ofc_message_data(OFC_MESSAGE *msg);
/**
 * Return the data pointer, and remove from the message
 *
 * \param msg
 * Message to unload data from
 *
 * \returns
 * Pointer to the data
 */
OFC_CORE_LIB OFC_VOID *
ofc_message_unload_data(OFC_MESSAGE *msg);
/**
 * Get a pointer to an offset in a message
 *
 * \param msg
 * Message to get a pointer into
 *
 * \param offset
 * Offset into the message for the pointer
 *
 * \returns
 * Pointer into the message
 */
OFC_CORE_LIB OFC_VOID *
ofc_message_get_pointer(OFC_MESSAGE *msg, OFC_INT offset);
/**
 * Return the offset of the current byte pointer
 *
 * While data is being sent, the offset is the offset to the next unsent
 * byte.  While data is being read, the offset is the offset one past the
 * last byte received.
 *
 * \param msg
 * The message to return the offset for
 *
 * \returns
 * The offset
 */
OFC_CORE_LIB OFC_INT
ofc_message_offset(OFC_MESSAGE *msg);
/**
 * Set the send size.
 *
 * This sets the number of unsent bytes of a message.  This is useful
 * before passing the message on to the Socket layer.  The send size
 * is not always the size of the buffer.
 *
 * \param msg
 * The message to set the send size for
 *
 * \param size
 * The size of the send buffer
 */
OFC_CORE_LIB OFC_VOID
ofc_message_set_send_size(OFC_MESSAGE *msg, OFC_SIZET size);
/**
 * Get the IP address and port of a received message
 *
 * \param msg
 * The message to get the received IP address and port from
 *
 * \param ip
 * The ip that sent the message data
 *
 * \param port
 * the port that sent the message data
 */
OFC_CORE_LIB OFC_VOID
ofc_message_addr(OFC_MESSAGE *msg, OFC_IPADDR *ip, OFC_UINT16 *port);
/**
 * Set the destination IP address and port for a message
 *
 * \param msg
 * The message to set the IP address and port for
 *
 * \param ip
 * The destination ip
 *
 * \param port
 * the destination port
 */
OFC_CORE_LIB OFC_VOID
ofc_message_set_addr(OFC_MESSAGE *msg, OFC_IPADDR *ip, OFC_UINT16 port);
/**
 * Reset message buffer pointers
 *
 * This call sets the send count to the size of the message, the
 * next unsent or next to receive offset to the start of the message,
 * and resets the FIFOs to the start of the message.
 *
 * \param msg
 * The message to reset
 */
OFC_CORE_LIB OFC_VOID
ofc_message_reset(OFC_MESSAGE *msg);
/**
 * Set the base offset of a message.
 *
 * This is usually 0, but can be overridden to some other offset
 * to provide flexibility in servicing of messages
 *
 * \param msg
 * Pointer to message
 *
 * \param base
 * Offset to new base
 */
OFC_CORE_LIB OFC_VOID
ofc_message_set_base(OFC_MESSAGE *msg, OFC_INT base);
/**
 * Get the base of a message
 *
 * This will return the offset to current message base.  This will
 * usually be zero, but may be different if it has been overridden with
 * the SetBase call
 *
 * \param msg
 * The message to obtain the base of
 *
 * \returns
 * The message base
 */
OFC_CORE_LIB OFC_INT
ofc_message_get_base(OFC_MESSAGE *msg);
/**
 * Get the allocated size of the message buffer
 *
 * \param msg
 * Message to return the size for
 *
 * \returns
 * The allocated size of the message buffer
 */
OFC_CORE_LIB OFC_SIZET
ofc_message_get_length(OFC_MESSAGE *msg);
/**
 * Put a 16 bit value into the message in the correct endianess
 *
 * \param msg
 * The message to put the value in
 *
 * \param offset
 * The offset within the message to put the value
 *
 * \param value
 * The value to put in the message
 *
 * \returns
 * OFC_TRUE if the value was stored, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_put_u16(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT16 value);
/**
 * Put an 8 bit value into the message in the correct endianess
 *
 * \param msg
 * The message to put the value in
 *
 * \param offset
 * The offset within the message to put the value
 *
 * \param value
 * The value to put in the message
 *
 * \returns
 * OFC_TRUE if the value was stored, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_put_u8(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT8 value);
/**
 * Put a 32 bit value into the message in the correct endianess
 *
 * \param msg
 * The message to put the value in
 *
 * \param offset
 * The offset within the message to put the value
 *
 * \param value
 * The value to put in the message
 *
 * \returns
 * OFC_TRUE if the value was stored, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_put_u32(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT32 value);
/**
 * Put a 64 bit value into the message in the correct endianess
 *
 * \param msg
 * The message to put the value in
 *
 * \param offset
 * The offset within the message to put the value
 *
 * \param value
 * The value to put in the message
 *
 * \returns
 * OFC_TRUE if the value was stored, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_put_u64(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT64 *value);
/**
 * Put a wide or normal character string in the message
 *
 * \param msg
 * Message to put string into
 *
 * \param offset
 * Offset to put it at
 *
 * \param str
 * String to put
 *
 * \returns
 * OFC_TRUE if put was successful, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_put_tstr(OFC_MESSAGE *msg, OFC_INT offset, OFC_LPCTSTR str);
/**
 * Put a wide or normal character string in the message, but limit to
 * a maximum count
 *
 * \param msg
 * Message to put string into
 *
 * \param offset
 * Offset to put it at
 *
 * \param str
 * String to put
 *
 * \param len
 * Maximum characters to put
 *
 * \returns
 * OFC_TRUE if put was successful, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_put_tstrn(OFC_MESSAGE *msg, OFC_INT offset,
                      OFC_LPCTSTR str, OFC_SIZET len);
/**
 * Get a wide or normal character string from a message.  Limit the
 * characters returned by a value.  This call will allocate a buffer
 * which must be released.
 *
 * \param msg
 * Message to get string into
 *
 * \param offset
 * Offset to get it at
 *
 * \param str
 * Pointer to where to return the string pointer
 *
 * \param max_len
 * Maximum characters to get
 *
 * \returns
 * OFC_TRUE if put was successful, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_get_tstrn(OFC_MESSAGE *msg, OFC_INT offset,
                      OFC_LPTSTR *str, OFC_SIZET max_len);
/**
 * Get a wide or normal character string from a message.  Limit the
 * characters returned by a value.  This call will use a buffer provided
 * by the caller to return the string in
 *
 * \param msg
 * Message to get string into
 *
 * \param offset
 * Offset to get it at
 *
 * \param str
 * Pointer to where to return the string
 *
 * \param max_len
 * Maximum characters to get
 *
 * \returns
 * OFC_TRUE if put was successful, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_get_tstrnx(OFC_MESSAGE *msg, OFC_INT offset,
                       OFC_LPTSTR str, OFC_SIZET max_len);
/**
 * Get a wide or normal character string from a message.  The string
 * returned must be freed
 *
 * \param msg
 * Message to get string into
 *
 * \param offset
 * Offset to get it at
 *
 * \param str
 * Pointer to where to return the string pointer
 *
 * \returns
 * OFC_TRUE if put was successful, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_get_tstr(OFC_MESSAGE *msg, OFC_INT offset, OFC_LPTSTR *str);
/**
 * Put a string into a message
 *
 * \param msg
 * Pointer to message to put the string in
 *
 * \param offset
 * Offset of where to place the string
 *
 * \param str
 * Pointer to string to put
 *
 * \returns
 * OFC_TRUE if put was successful, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_put_cstr(OFC_MESSAGE *msg, OFC_INT offset, OFC_LPCSTR str);
/**
 * Return the size of a string at a particular offset
 *
 * \param msg
 * Pointer to message that string is within
 *
 * \param offset
 * Offset to string in message
 *
 * \param max_len
 * Maximum number of characters to check for for EOS
 *
 * \returns
 * size of string
 */
OFC_CORE_LIB OFC_SIZET
ofc_message_get_tstring_len(OFC_MESSAGE *msg, OFC_INT offset,
                            OFC_SIZET max_len);
/**
 * Get an 8 bit value from a message
 *
 * \param msg
 * The message to get the value from
 *
 * \param offset
 * An offset into the message buffer to get the value from
 *
 * \returns
 * The value obtained
 */
OFC_CORE_LIB OFC_UINT8
ofc_message_get_u8(OFC_MESSAGE *msg, OFC_INT offset);
/**
 * Get a 16 bit value from a message
 *
 * \param msg
 * The message to get the value from
 *
 * \param offset
 * An offset into the message buffer to get the value from
 *
 * \returns
 * The value obtained
 */
OFC_CORE_LIB OFC_UINT16
ofc_message_get_u16(OFC_MESSAGE *msg, OFC_INT offset);
/**
 * Get a 32 bit value from a message
 *
 * \param msg
 * The message to get the value from
 *
 * \param offset
 * An offset into the message buffer to get the value from
 *
 * \returns
 * The value obtained
 */
OFC_CORE_LIB OFC_UINT32
ofc_message_get_u32(OFC_MESSAGE *msg, OFC_INT offset);
/**
 * Get a 64 bit value from a message
 *
 * \param msg
 * The message to get the value from
 *
 * \param offset
 * An offset into the message buffer to get the value from
 *
 * \param val
 * Pointer to where to return the value
 */
OFC_CORE_LIB OFC_VOID
ofc_message_get_u64(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT64 *val);
/**
 * Set the offset for the FIFO1
 *
 * FIFOs are useful constructs for some protocols.  CIFS is a heavy
 * user of them.  This allows a protocol to use a portion of a message
 * as a stack.
 *
 * \param msg
 * Message to set the fifo for
 *
 * \param offset
 * Offset in the message to set the fifo to
 *
 * \param size
 * Size of the fifo
 */
OFC_CORE_LIB OFC_VOID
ofc_message_fifo_set(OFC_MESSAGE *msg, OFC_INT offset, OFC_SIZET size);
/**
 * Push an area onto the stack
 *
 * This is useful to push an arbitrary structure on the stack.  In this
 * case, an application would push the structure on the stack using this
 * call.  It returns the address of the structure that was pushed.  The
 * application then then manipulate the structure as desired.
 *
 * \param msg
 * message to push the structure into
 *
 * \param size
 * size of the message to push
 *
 * \returns
 * Address of the structure pushed
 */
OFC_CORE_LIB OFC_VOID *
ofc_message_fifo_push(OFC_MESSAGE *msg, OFC_SIZET size);
/**
 * Pop an area from the stack
 *
 * This call will pop an area from the stack and return a pointer to the
 * start of the area that was popped.
 *
 * \param msg
 * message to pop the structure from
 *
 * \param size
 * Size of the message to pop
 *
 * \returns
 * address of the structure popped.
 */
OFC_CORE_LIB OFC_VOID *
ofc_message_fifo_pop(OFC_MESSAGE *msg, OFC_SIZET size);
/**
 * Align the FIFO pointer in a message
 *
 * \param msg
 * Message whose fifo is to be aligned
 *
 * \param align
 * align size (2, 4, 8)
 */
OFC_CORE_LIB OFC_VOID
ofc_message_fifo_align(OFC_MESSAGE *msg, OFC_INT align);
/**
 * Push an 8 bit value onto the FIFO stack
 *
 * \param msg
 * Message to push the value into
 *
 * \param value
 * The value to push
 *
 * \returns
 * OFC_TRUE if the data could be pushed, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_push_u8(OFC_MESSAGE *msg, OFC_UINT8 value);
/**
 * Push a 16 bit value onto the FIFO stack
 *
 * \param msg
 * Message to push the value into
 *
 * \param value
 * The value to push
 *
 * \returns
 * OFC_TRUE if the data could be pushed, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_push_u16(OFC_MESSAGE *msg, OFC_UINT16 value);
/**
 * Push a 32 bit value onto the FIFO stack
 *
 * \param msg
 * Message to push the value into
 *
 * \param value
 * The value to push
 *
 * \returns
 * OFC_TRUE if the data could be pushed, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_push_u32(OFC_MESSAGE *msg, OFC_UINT32 value);
/**
 * Push a 64 bit value onto the FIFO stack
 *
 * \param msg
 * Message to push the value into
 *
 * \param value
 * The value to push
 *
 * \returns
 * OFC_TRUE if the data could be pushed, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_push_u64(OFC_MESSAGE *msg, OFC_UINT64 *value);
/**
 * Push a string (wide or normal) character string on the stack
 *
 * \param msg
 * Message to push the string into
 *
 * \param str
 * string to push
 *
 * \returns
 * OFC_TRUE if data could be pushed, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_push_tstr(OFC_MESSAGE *msg, OFC_LPCTSTR str);
/**
 * Push a wide or normal character string onto the fifo, but limit to
 * a maximum count
 *
 * \param msg
 * Message to put string into
 *
 * \param str
 * String to put
 *
 * \param len
 * Maximum characters to put
 *
 * \returns
 * OFC_TRUE if put was successful, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_push_tstrn(OFC_MESSAGE *msg, OFC_LPCTSTR str,
                            OFC_SIZET len);
/**
 * Push a normal character string onto the fifo.
 *
 * \param msg
 * Message to put string into
 *
 * \param str
 * String to put
 *
 * \returns
 * OFC_TRUE if put was successful, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_push_cstr(OFC_MESSAGE *msg, OFC_LPCSTR str);
/**
 * Pop an optionally unicode string off the stack.
 *
 * \param msg
 * message that holds the string
 *
 * \param len
 * Maximum number of characters to pop
 *
 * \returns
 * pointer to string returned.  This string must be released
 */
OFC_CORE_LIB OFC_LPTSTR
ofc_message_fifo_pop_tstrn(OFC_MESSAGE *msg, OFC_SIZET len);
/**
 * Pop a wide or normal character string from a fifo.  Limit the
 * characters returned by a value.  This call will use a buffer provided
 * by the caller to return the string in
 *
 * \param msg
 * Message to get string into
 *
 * \param str
 * Pointer to where to return the string
 *
 * \param len
 * Maximum characters to get
 */
OFC_CORE_LIB OFC_VOID
ofc_message_fifo_pop_tstrnx(OFC_MESSAGE *msg, OFC_LPTSTR str,
                            OFC_SIZET len);
/**
 * Pop a normal character string from a fifo.  Limit the
 * characters returned by a value.  This will return a pointer to a string
 * that must be released.
 *
 * \param msg
 * Message to get string into
 *
 * \param len
 * Maximum characters to get
 *
 * \returns
 * Pointer to string returned.
 */
OFC_CORE_LIB OFC_LPCSTR
ofc_message_fifo_pop_cstrn(OFC_MESSAGE *msg, OFC_SIZET len);
/**
 * Pop an 8 bit value from the message FIFO
 *
 * \param msg
 * Message to pop the value from
 *
 * \returns
 * The value popped
 */
OFC_CORE_LIB OFC_UINT8
ofc_message_fifo_pop_u8(OFC_MESSAGE *msg);
/**
 * Pop a 16 bit value from the message FIFO
 *
 * \param msg
 * Message to pop the value from
 *
 * \returns
 * The value popped
 */
OFC_CORE_LIB OFC_UINT16
ofc_message_fifo_pop_u16(OFC_MESSAGE *msg);
/**
 * Pop a 32 bit value from the message FIFO
 *
 * \param msg
 * Message to pop the value from
 *
 * \returns
 * The value popped
 */
OFC_CORE_LIB OFC_UINT32
ofc_message_fifo_pop_u32(OFC_MESSAGE *msg);
/**
 * Pop a 64 bit value from the message FIFO
 *
 * \param msg
 * Message to pop the value from
 *
 * \param val
 * Pointer to where to return the value
 */
OFC_CORE_LIB OFC_VOID
ofc_message_fifo_pop_u64(OFC_MESSAGE *msg, OFC_UINT64 *val);
/**
 * Pop a C string from the stack (NULL terminated CHAR)
 *
 * \param msg
 * Message to pop the value from
 *
 * \returns
 * Pointer to the string.  The stack pointer is passed the NULL at the EOS
 */
OFC_CORE_LIB OFC_CHAR *
ofc_message_fifo_pop_cstring(OFC_MESSAGE *msg);
/**
 * Pop a Unicode string from the stack (NULL terminated TCHAR)
 *
 * \param msg
 * Message to pop the value from
 *
 * \returns
 * Pointer to the string.  The stack pointer is passed the NULL at the EOS
 */
OFC_CORE_LIB OFC_TCHAR *
ofc_message_fifo_pop_tstr(OFC_MESSAGE *msg);
/**
 * Get the offset of the FIFO
 *
 * This is often useful to determine how much of the FIFO has been used.
 * This will usually indicate the overall size of the message.
 *
 * \param msg
 * Message to obtain the FIFO pointer from
 *
 * \returns
 * The FIFO offset value
 */
OFC_CORE_LIB OFC_INT
ofc_message_fifo_get(OFC_MESSAGE *msg);
/**
 * Update the FIFO pointer to some fixed offset
 *
 * This allows the fifo to be updated to some new offset.  Checks are made
 * to insure the fifo still points within the message.
 *
 * \param msg
 * Message to obtain the FIFO pointer from
 *
 * \param offset
 * The new fifo pointer
 */
OFC_CORE_LIB OFC_VOID
ofc_message_fifo_update(OFC_MESSAGE *msg, OFC_INT offset);
/**
 * Return the size of the remaining fifo
 *
 * \param msg
 * Pointer to message that contains the fifo
 *
 * \returns
 * Size of the remaining fifo
 */
OFC_CORE_LIB OFC_INT
ofc_message_fifo_rem(OFC_MESSAGE *msg);
/**
 * Return the full size of the FIFO
 *
 * \param msg
 * Pointer to message that contains the fifo
 *
 * \returns
 * size of fifo
 */
OFC_CORE_LIB OFC_INT
ofc_message_fifo_size(OFC_MESSAGE *msg);
/**
 * Return the used length of the fifo
 *
 * \param msg
 * Pointer to the message that contains the fifo
 *
 * \returns
 * Size of the used portion of the fifo
 */
OFC_CORE_LIB OFC_INT
ofc_message_fifo_len(OFC_MESSAGE *msg);
/**
 * Return the base offset of the fifo
 *
 * \param msg
 * Pointer to message that contains the fifo
 *
 * \returns
 * offset of the fifo in the message
 */
OFC_CORE_LIB OFC_INT
ofc_message_fifo_base(OFC_MESSAGE *msg);
/**
 * Get a pointer to the first byte of the FIFO
 *
 * \param msg
 * Pointer to message that contains the fifo
 *
 * \returns
 * Pointer to the start of the fifo
 */
OFC_CORE_LIB OFC_CHAR *
ofc_message_fifo_base_pointer(OFC_MESSAGE *msg);
/**
 * See if there is room in the FIFO
 *
 * \param msg
 * The message to check
 *
 * \returns
 * TRUE if there is room, FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_fifo_valid(OFC_MESSAGE *msg);
/**
 * Set the endianess of the message
 *
 * \param msg
 * Pointer to message to set the endianess to
 *
 * \param endianess
 * The endianess to set
 */
OFC_CORE_LIB OFC_VOID
ofc_message_set_endian(OFC_MESSAGE *msg, MSG_ENDIAN endianess);
/**
 * Put a 8 bit byte in the parameter block of a message
 *
 * \param msg
 * Pointer to message
 *
 * \param offset
 * Parameter offset
 *
 * \param value
 * 8 bit value to put
 */
OFC_CORE_LIB OFC_VOID
ofc_message_param_put_u8(OFC_MESSAGE *msg, OFC_INT offset, OFC_UINT8 value);
/**
 * Put a 16 bit byte in the parameter block of a message
 *
 * \param msg
 * Pointer to message
 *
 * \param offset
 * Parameter offset
 *
 * \param value
 * 16 bit value to put
 */
OFC_CORE_LIB OFC_VOID
ofc_message_param_put_u16(OFC_MESSAGE *msg, OFC_INT offset,
                          OFC_UINT16 value);
/**
 * Put a 32 bit byte in the parameter block of a message
 *
 * \param msg
 * Pointer to message
 *
 * \param offset
 * Parameter offset
 *
 * \param value
 * 32 bit value to put
 */
OFC_CORE_LIB OFC_VOID
ofc_message_param_put_u32(OFC_MESSAGE *msg, OFC_INT offset,
                          OFC_UINT32 value);
/**
 * Put a 64 bit byte in the parameter block of a message
 *
 * \param msg
 * Pointer to message
 *
 * \param offset
 * Parameter offset
 *
 * \param val
 * 64 bit value to put
 */
OFC_CORE_LIB OFC_VOID
ofc_message_param_put_u64(OFC_MESSAGE *msg, OFC_INT offset,
                          OFC_UINT64 *val);
/**
 * Get an 8 bit value out of a parameter block of a message
 *
 * \param msg
 * Pointer to message
 *
 * \param offset
 * Offset into parameter block to locate the 8 bit value
 *
 * \returns
 * The 8 bit value
 */
OFC_CORE_LIB OFC_UINT8
ofc_message_param_get_u8(OFC_MESSAGE *msg, OFC_INT offset);
/**
 * Get a 16 bit value out of a parameter block of a message
 *
 * \param msg
 * Pointer to message
 *
 * \param offset
 * Offset into parameter block to locate the 16 bit value
 *
 * \returns
 * The 16 bit value
 */
OFC_CORE_LIB OFC_UINT16
ofc_message_param_get_u16(OFC_MESSAGE *msg, OFC_INT offset);
/**
 * Get a 32 bit value out of a parameter block of a message
 *
 * \param msg
 * Pointer to message
 *
 * \param offset
 * Offset into parameter block to locate the 32 bit value
 *
 * \returns
 * The 32 bit value
 */
OFC_CORE_LIB OFC_UINT32
ofc_message_param_get_u32(OFC_MESSAGE *msg, OFC_INT offset);
/**
 * Get an 64 bit value out of a parameter block of a message
 *
 * \param msg
 * Pointer to message
 *
 * \param offset
 * Offset into parameter block to locate the 64 bit value
 *
 * \param val
 * Pointer to where to return the 64 bit value
 */
OFC_CORE_LIB OFC_VOID
ofc_message_param_get_u64(OFC_MESSAGE *msg, OFC_INT offset,
                          OFC_UINT64 *val);
/**
 * Initialize the parameter block of a message
 *
 * \param msg
 * The message to set
 *
 * \param offset
 * The offset in the message to the parameter block
 *
 * \param len
 * The size of the parameter block
 */
OFC_CORE_LIB OFC_VOID
ofc_message_param_set(OFC_MESSAGE *msg, OFC_INT offset, OFC_SIZET len);
/**
 * Get the offset of the parameter block
 *
 * \param msg
 * The message
 *
 * \returns
 * The offset to the parameter block
 */
OFC_CORE_LIB OFC_INT
ofc_message_param_get_offset(OFC_MESSAGE *msg);
/**
 * Get the size of the parameter block of a message
 *
 * \param msg
 * The message
 *
 * \returns
 * The length of tghe message
 */
OFC_CORE_LIB OFC_INT
ofc_message_param_get_len(OFC_MESSAGE *msg);
/**
 * Get a wide or normal string from the parameter block of a message
 *
 * \param msg
 * The message
 *
 * \param offset
 * Offset into the parameter block for the string
 *
 * \param str
 * Pointer to where to store the pointer to the string
 * This pointer must be freed when done using
 *
 * \returns
 * OFC_TRUE if successful, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_param_get_tstr(OFC_MESSAGE *msg, OFC_INT offset,
                           OFC_LPTSTR *str);
/**
 * Put a wide or normal string into the paramter block of a message
 *
 * \param msg
 * The message
 *
 * \param offset
 * Offset into the param block to where to place the message
 *
 * \param str
 * String to place
 *
 * \returns
 * OFC_TRUE if successful, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_param_put_tstr(OFC_MESSAGE *msg, OFC_INT offset,
                           OFC_LPTSTR str);

OFC_CORE_LIB OFC_BOOL
ofc_message_param_put_tstrn(OFC_MESSAGE *msg, OFC_INT offset,
                            OFC_LPCTSTR str, OFC_SIZET len);
/**
 * Get a pointer to an offset with the parameter block
 *
 * \param msg
 * Pointer to the message
 *
 * \param offset
 * Offset into the parameter block to find a pointer for
 *
 * \returns
 * Pointer to that offet within the parameter block
 */
OFC_CORE_LIB OFC_VOID *
ofc_message_param_get_pointer(OFC_MESSAGE *msg, OFC_INT offset);
/**
 * Reallocate a message
 *
 * \param msg
 * The message to realloc
 *
 * \param msgDataLength
 * The new size of the message
 *
 * \returns
 * OFC_TRUE if success, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_realloc(OFC_MESSAGE *msg, OFC_SIZET msgDataLength);
/**
 * Determine if a message has been received from a particular interface
 *
 * \param msg
 * The message to compae subnets for
 *
 * \param iface
 * The interface to compare against.  See \ref OfcNet for more information
 * about network interfaces
 *
 * \param intip
 * Pointer to return the IP of the interface to.  This is simply a
 * convienience.  If the ointer is NULL, no ip address is returned
 *
 * \returns
 * OFC_TRUE if it is in the subnet, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
ofc_message_from_subnet(OFC_MESSAGE *msg, OFC_INT iface,
                        OFC_IPADDR *intip);
/**
 * Set the context for a message
 *
 * \param msg
 * Message to set context for
 *
 * \param context
 * Context to set
 */
OFC_CORE_LIB OFC_VOID
ofc_message_set_context(OFC_MESSAGE *msg, OFC_VOID *context);
/**
 * Get the context of a message
 *
 * \param msg
 * Message to get context of
 *
 * \returns
 * Context
 */
OFC_CORE_LIB OFC_VOID *
ofc_message_get_context(OFC_MESSAGE *msg);

OFC_CORE_LIB OFC_BOOL ofc_message_destroy_after_send(OFC_MESSAGE *msg);

OFC_CORE_LIB OFC_VOID ofc_message_set_destroy_after_send(OFC_MESSAGE *msg);

#if defined(__cplusplus)
}
#endif
/** \} */
#endif

