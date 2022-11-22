/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/config.h"
#include "ofc/libc.h"
#include "ofc/heap.h"
#include "ofc/process.h"
#include "ofc/iovec.h"

OFC_IOMAP ofc_iovec_new(OFC_VOID)
{
  struct iovec_list *iovec;
  iovec = ofc_malloc(sizeof(struct iovec_list));
  iovec->num_vecs = 0;
  iovec->end_offset = 0;
  iovec->iovecs = OFC_NULL;
  return (iovec);
}

/*
 * Find the index of the vector on or before the requsted offset
 * This is really private, but needed for test
 */
OFC_INT ofc_iovec_find(OFC_IOMAP list, OFC_OFFT offset)
{
  struct iovec_list *iovec = list;
  OFC_OFFT working_offset = 0;
  OFC_INT index;

  for (index = 0;
       index < iovec->num_vecs && offset > working_offset;)
    {
      OFC_INT next_offset = working_offset + iovec->iovecs[index].length;
        
      if (next_offset <= offset)
        {
          index++;
        }
      working_offset = next_offset;
    }
  return (index);
}

OFC_UCHAR * ofc_iovec_insert(OFC_IOMAP list, OFC_OFFT offset,
                             IOVEC_ALLOC_TYPE alloc_type,
                             OFC_UCHAR *data, OFC_SIZET length)
{
  struct iovec_list *iovec = list;
  OFC_INT index = ofc_iovec_find(iovec, offset);
  /*
   * index may be past the end of the list
   * may point to a structure where offset is equal to the begining of the
   *   block
   * may point to a structure where the offset is inside a block
   */
  if (index == iovec->num_vecs)
    {
      /*
       * the offset is past the existing vectors.  It may be
       * contiguous with the message, or it may require a hole
       * first create a block that is contiguous
       */
      iovec->num_vecs++;
      iovec->iovecs = ofc_realloc(iovec->iovecs,
                                  (iovec->num_vecs) *
                                  sizeof (struct iovec_entry));
      iovec->iovecs[index].offset = iovec->end_offset;
      if (offset > iovec->iovecs[index].offset)
        {
          /* create a hole */
          iovec->iovecs[index].type = IOVEC_ALLOC_NONE;
          iovec->iovecs[index].data = OFC_NULL;
          iovec->iovecs[index].length = offset - iovec->iovecs[index].offset;

          iovec->end_offset += iovec->iovecs[index].length;
          iovec->num_vecs++;
          iovec->iovecs = ofc_realloc(iovec->iovecs,
                                      (iovec->num_vecs) *
                                      sizeof (struct iovec_entry));
          index++;
        }
      /*
       * We want to initialize block at index with this
       * insertion
       */
      iovec->iovecs[index].offset = iovec->end_offset;
      iovec->iovecs[index].type = alloc_type;
      iovec->iovecs[index].length = length;
      if (iovec->iovecs[index].type == IOVEC_ALLOC_HEAP)
        data = ofc_malloc(iovec->iovecs[index].length);
      iovec->iovecs[index].data = data;
      iovec->end_offset += length;
    }
  else
    {
      /*
       * offset is somewhere in current message
       */
      if (offset == iovec->iovecs[index].offset)
        {
          /*
           * insert chunk before current index
           */
          iovec->num_vecs++;
          iovec->iovecs = ofc_realloc(iovec->iovecs,
                                      iovec->num_vecs *
                                      sizeof (struct iovec_entry));
          for (OFC_INT i = iovec->num_vecs; i > index; i--)
            {
              iovec->iovecs[i] = iovec->iovecs[i-1];
              /*
               * update offsets of all subsequent blocks to account
               * for what we are inserting
               */
              iovec->iovecs[i].offset += length;
            }
          /*
           * current blocks offset is already set from previous block
           */
          iovec->iovecs[index].type = alloc_type;
          iovec->iovecs[index].length = length;
          if (iovec->iovecs[index].type == IOVEC_ALLOC_HEAP)
            data = ofc_malloc(iovec->iovecs[index].length);
          iovec->iovecs[index].data = data;
          iovec->end_offset += length;
        }
      else
        {
          /*
           * Offset is in the middle of the chunk.  Need to split
           * current chunk
           */
          iovec->num_vecs += 2;
          iovec->iovecs = ofc_realloc(iovec->iovecs,
                                      iovec->num_vecs *
                                      sizeof(struct iovec_entry));
          for (OFC_INT i = iovec->num_vecs; i > (index + 1); i--)
            {
              iovec->iovecs[i] = iovec->iovecs[i-2];
              /*
               * update offsets of all subsequent blocks to account
               * for what we are inserting
               */
              iovec->iovecs[i].offset += length;
            }
          /*
           * split our iovec at index into two at 
           * say iovec at index is at offet 1020 and length of 100
           * and we are inserting 30 bytes at 1060.
           * we want to split 1020 with 40 bytes, we want to add
           * 1060 with 30 bytes, and we want to capture 1090 with 70 bytes.
           * next offset was 1120 and is moving to 1150.
           */
          /* offset = 1060, iovec->ioves[index].offset = 1020.
           * split_offset is 40
           */
          OFC_OFFT split_offset = offset - iovec->iovecs[index].offset;
          /*
           * iovec at index+2 has data that is 40 after the base of
           * the original pointer.  It's length is 40 less than the
           * original length.  The offset is the original offset, plus
           * the length we've inserted, plus the amount left in the original
           */
          iovec->iovecs[index+2].offset = iovec->iovecs[index].offset +
            length + split_offset;
          iovec->iovecs[index+2].type = IOVEC_ALLOC_STATIC;
          iovec->iovecs[index+2].data = iovec->iovecs[index].data +
            split_offset;
          iovec->iovecs[index+2].length = iovec->iovecs[index].length -
            split_offset;
          /*
           * Fill in the inserted block.  It's offset 
           */
          iovec->iovecs[index+1].offset = offset;
          iovec->iovecs[index+1].type = alloc_type;
          iovec->iovecs[index+1].length = length;
          if (iovec->iovecs[index+1].type == IOVEC_ALLOC_HEAP)
            data = ofc_malloc(iovec->iovecs[index+1].length);
          iovec->iovecs[index+1].data = data;
          /*
           * Now fill in the first part of the split
           * only thing that changes is the length
           */
          iovec->iovecs[index].length = split_offset;
          iovec->end_offset += length;
        }
    }
  return (data);
}
      
OFC_UCHAR * ofc_iovec_append(OFC_IOMAP list,
                             IOVEC_ALLOC_TYPE alloc_type,
                             OFC_UCHAR *data, OFC_SIZET length)
{
  struct iovec_list *iovec = list;
  return(ofc_iovec_insert(list, iovec->end_offset, alloc_type, data, length));
}

OFC_UCHAR * ofc_iovec_prepend(OFC_IOMAP iovec,
                              IOVEC_ALLOC_TYPE alloc_type,
                              OFC_UCHAR *data, OFC_SIZET length)
{
  return(ofc_iovec_insert(iovec, 0, alloc_type, data, length));
}

OFC_VOID ofc_iovec_destroy(OFC_IOMAP list)
{
  struct iovec_list *iovec = list;
  for (OFC_INT i = 0 ; i < iovec->num_vecs; i++)
    {
      if (iovec->iovecs[i].type == IOVEC_ALLOC_HEAP)
        ofc_free(iovec->iovecs[i].data);
    }
  ofc_free(iovec->iovecs);
  ofc_free(iovec);
}

OFC_SIZET ofc_iovec_length(OFC_IOMAP list)
{
  struct iovec_list *iovec = list;
  return (iovec->end_offset);
}

OFC_UCHAR *ofc_iovec_lookup(OFC_IOMAP list, OFC_OFFT offset,
                            OFC_SIZET len)
{
  struct iovec_list *iovec = list;
  OFC_INT index;
  OFC_OFFT inner_offset;
  OFC_UCHAR *data = OFC_NULL;

  index = ofc_iovec_find(iovec, offset);
  /* is it allocated */
  if (index < iovec->num_vecs)
    {
      /* does the len fit in the remainder */
      inner_offset = offset - iovec->iovecs[index].offset;
      ofc_assert(inner_offset >= 0, "IOVEC: bad offset");

      if (iovec->iovecs[index].type != IOVEC_ALLOC_NONE)
        {
          if (iovec->iovecs[index].length - inner_offset >= len)
            {
              data = iovec->iovecs[index].data + inner_offset;
            }
        }
    }
  return (data);
}
