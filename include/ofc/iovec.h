/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_IOVEC_H__)
#define __OFC_IOVEC_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/config.h"

typedef enum {
    /**
     * Message is statically allocated and should not be freed
     */
    IOVEC_ALLOC_STATIC,
    /**
     * Message is allocated from the heap and should be freed when complete
     */
    IOVEC_ALLOC_HEAP,
    /**
     * No content
     */
    IOVEC_ALLOC_NONE,
      
} IOVEC_ALLOC_TYPE;

typedef OFC_VOID *OFC_IOMAP;

/*
 * Private, but needed within the platform layer
 */
struct iovec_entry {
  OFC_OFFT offset;
  IOVEC_ALLOC_TYPE type;
  OFC_UCHAR *data;
  OFC_SIZET length;
};

struct iovec_list {
  OFC_UINT num_vecs;
  OFC_OFFT end_offset;
  struct iovec_entry *iovecs;
};

#if defined(__cplusplus)
extern "C"
{
#endif
  OFC_IOMAP ofc_iovec_new(OFC_VOID);
  OFC_UCHAR * ofc_iovec_insert(OFC_IOMAP list, OFC_OFFT offset,
                               IOVEC_ALLOC_TYPE alloc_type,
                               OFC_UCHAR *data, OFC_SIZET length);
  OFC_UCHAR * ofc_iovec_append(OFC_IOMAP list,
                               IOVEC_ALLOC_TYPE alloc_type,
                               OFC_UCHAR *data, OFC_SIZET length);
  OFC_UCHAR * ofc_iovec_prepend(OFC_IOMAP iovec,
                                IOVEC_ALLOC_TYPE alloc_type,
                                OFC_UCHAR *data, OFC_SIZET length);
  OFC_VOID ofc_iovec_destroy(OFC_IOMAP list);
  OFC_SIZET ofc_iovec_length(OFC_IOMAP list);
  OFC_UCHAR *ofc_iovec_lookup(OFC_IOMAP list, OFC_OFFT offset,
                              OFC_SIZET len);
  /*
   * This is really private but needed for test
   */
  OFC_INT ofc_iovec_find(OFC_IOMAP list, OFC_OFFT offset);

#if defined(__APPLE__)
  OFC_VOID ofc_iovec_get(OFC_IOMAP inp, OFC_VOID **iovec,
                         OFC_INT *veclen);
#endif

#if defined(__cplusplus)
}
#endif

#endif

