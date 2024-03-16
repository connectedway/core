/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#include "unity.h"
#include "unity_fixture.h"

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/config.h"
#include "ofc/libc.h"
#include "ofc/heap.h"
#include "ofc/process.h"
#include "ofc/framework.h"
#include "ofc/iovec.h"

#if defined(__APPLE__)
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#endif

static OFC_INT test_startup(OFC_VOID) {
#if defined(INIT_ON_LOAD)
  volatile OFC_VOID *init = ofc_framework_init;
#else
    ofc_framework_init();
#endif
    return (0);
}

static OFC_VOID test_shutdown(OFC_VOID) {
#if !defined(INIT_ON_LOAD)
    ofc_framework_shutdown();
    ofc_framework_destroy();
#endif
}

TEST_GROUP(iovec);

TEST_SETUP(iovec) {
    TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(iovec) {
    test_shutdown();
}

TEST(iovec, test_iovec) {
  OFC_IOMAP list = ofc_iovec_new();
  OFC_UCHAR *data;
  OFC_UCHAR *lookup;
  OFC_IOVEC *iovec;
  OFC_INT veclen;


  data = ofc_iovec_append(list, IOVEC_ALLOC_HEAP, OFC_NULL, 100);
  ofc_assert(data != OFC_NULL, "IOVEC: Bad append");
  ofc_memset(data, 0x01, 100);

  data = ofc_iovec_append(list, IOVEC_ALLOC_HEAP, OFC_NULL, 100);
  ofc_assert(data != OFC_NULL, "IOVEC: Bad append");
  ofc_memset(data, 0x02, 100);

  data = ofc_iovec_append(list, IOVEC_ALLOC_HEAP, OFC_NULL, 100);
  ofc_assert(data != OFC_NULL, "IOVEC: Bad append");
  ofc_memset(data, 0x03, 100);

  data = ofc_iovec_append(list, IOVEC_ALLOC_HEAP, OFC_NULL, 100);
  ofc_assert(data != OFC_NULL, "IOVEC: Bad append");
  ofc_memset(data, 0x04, 100);

  data = ofc_iovec_append(list, IOVEC_ALLOC_HEAP, OFC_NULL, 100);
  ofc_assert(data != OFC_NULL, "IOVEC: Bad append");
  ofc_memset(data, 0x05, 100);

  ofc_assert(ofc_iovec_length(list) == 500, "IOVEC: Bad length");

  OFC_INT index;

  index = ofc_iovec_find(list, 0);
  ofc_assert(index == 0, "IOVEC: Bad index");
  index = ofc_iovec_find(list, 50);
  ofc_assert(index == 0, "IOVEC: Bad index");
  index = ofc_iovec_find(list, 99);
  ofc_assert(index == 0, "IOVEC: Bad index");
  index = ofc_iovec_find(list, 100);
  ofc_assert(index == 1, "IOVEC: Bad index");
  index = ofc_iovec_find(list, 199);
  ofc_assert(index == 1, "IOVEC: Bad index");
  index = ofc_iovec_find(list, 200);
  ofc_assert(index == 2, "IOVEC: Bad index");
  index = ofc_iovec_find(list, 499);
  ofc_assert(index == 4, "IOVEC: Bad index");
  index = ofc_iovec_find(list, 500);
  ofc_assert(index == 5, "IOVEC: Bad index");
  index = ofc_iovec_find(list, 800);
  ofc_assert(index == 5, "IOVEC: Bad index");

  lookup = ofc_iovec_lookup(list, 0, 100);
  ofc_assert(lookup != OFC_NULL, "IOVEC: Bad lookup");
  for (int i = 0; i < 100; i++)
    ofc_assert(lookup[i] == 0x01, "IOVEC: Bad data");

  lookup = ofc_iovec_lookup(list, 100, 100);
  ofc_assert(lookup != OFC_NULL, "IOVEC: Bad lookup");
  for (int i = 0; i < 100; i++)
    ofc_assert(lookup[i] == 0x02, "IOVEC: Bad data");
  
  lookup = ofc_iovec_lookup(list, 200, 100);
  ofc_assert(lookup != OFC_NULL, "IOVEC: Bad lookup");
  for (int i = 0; i < 100; i++)
    ofc_assert(lookup[i] == 0x03, "IOVEC: Bad data");

  lookup = ofc_iovec_lookup(list, 300, 100);
  ofc_assert(lookup != OFC_NULL, "IOVEC: Bad lookup");
  for (int i = 0; i < 100; i++)
    ofc_assert(lookup[i] == 0x04, "IOVEC: Bad data");

  lookup = ofc_iovec_lookup(list, 400, 100);
  ofc_assert(lookup != OFC_NULL, "IOVEC: Bad lookup");
  for (int i = 0; i < 100; i++)
    ofc_assert(lookup[i] == 0x05, "IOVEC: Bad data");

  lookup = ofc_iovec_lookup(list, 500, 100);
  ofc_assert(lookup == OFC_NULL, "IOVEC: Bad lookup");

  lookup = ofc_iovec_lookup(list, 98, 2);
  ofc_assert(lookup != OFC_NULL, "IOVEC: Bad lookup");
  for (int i = 0; i < 2; i++)
    ofc_assert(lookup[i] == 0x01, "IOVEC: Bad data");

  lookup = ofc_iovec_lookup(list, 98, 4);
  ofc_assert(lookup == OFC_NULL, "IOVEC: Bad lookup");

  lookup = ofc_iovec_lookup(list, 598, 4);
  ofc_assert(lookup == OFC_NULL, "IOVEC: Bad lookup");

  data = ofc_iovec_insert(list, 250, IOVEC_ALLOC_HEAP, OFC_NULL, 50);
  ofc_assert(data != OFC_NULL, "IOVEC: Bad insert");
  ofc_memset(data, 0x35, 50);

  lookup = ofc_iovec_lookup(list, 200, 50);
  ofc_assert (data != OFC_NULL, "IOVEC: Bad lookup");
  for (int i = 0 ; i < 50; i++)
    ofc_assert(lookup[i] == 0x03, "IOVEC: Bad data");

  lookup = ofc_iovec_lookup(list, 250, 50);
  ofc_assert (data != OFC_NULL, "IOVEC: Bad lookup");
  for (int i = 0; i < 50; i++)
    ofc_assert(lookup[i] == 0x35, "IOVEC: Bad data");

  lookup = ofc_iovec_lookup(list, 300, 50);
  ofc_assert (data != OFC_NULL, "IOVEC: Bad lookup");
  for (int i = 0; i < 50; i++)
    ofc_assert(lookup[i] == 0x03, "IOVEC: Bad data");

  lookup = ofc_iovec_lookup(list, 350, 100);
  ofc_assert (data != OFC_NULL, "IOVEC: Bad lookup");
  for (int i = 0; i < 50; i++)
    ofc_assert(lookup[i] == 0x04, "IOVEC: Bad data");

  ofc_iovec_get(list, 0, ofc_iovec_length(list),
                &iovec, &veclen);
  ofc_assert (veclen == 7, "IOVEC: Bad get");

  for (int i = 0; i < 7; i++)
    {
      if (i == 0)
        {
          ofc_assert(iovec[i].iov_len == 100, "IOVEC: Bad iovec");
          for (OFC_INT j = 0; j < 100; j++)
            {
              ofc_assert(((OFC_UCHAR *)(iovec[i].iov_base))[j] == 0x01,
                         "IOVEC: Bad iovec");
            }
        }
      else if (i == 1)
        {
          ofc_assert(iovec[i].iov_len == 100, "IOVEC: Bad iovec");
          for (OFC_INT j = 0; j < 100; j++)
            ofc_assert(((OFC_UCHAR *)(iovec[i].iov_base))[j] == 0x02,
                       "IOVEC: Bad iovec");
        }
      else if (i == 2)
        {
          ofc_assert(iovec[i].iov_len == 50, "IOVEC: Bad iovec");
          for (OFC_INT j = 0; j < 50; j++)
            ofc_assert(((OFC_UCHAR *)(iovec[i].iov_base))[j] == 0x03,
                       "IOVEC: Bad iovec");
        }
      else if (i == 3)
        {
          ofc_assert(iovec[i].iov_len == 50, "IOVEC: Bad iovec");
          for (OFC_INT j = 0; j < 50; j++)
            ofc_assert(((OFC_UCHAR *)(iovec[i].iov_base))[j] == 0x35,
                       "IOVEC: Bad iovec");
        }
      else if (i == 4)
        {
          ofc_assert(iovec[i].iov_len == 50, "IOVEC: Bad iovec");
          for (OFC_INT j = 0; j < 50; j++)
            ofc_assert(((OFC_UCHAR *)(iovec[i].iov_base))[j] == 0x03,
                       "IOVEC: Bad iovec");
        }
      else if (i == 5)
        {
          ofc_assert(iovec[i].iov_len == 100, "IOVEC: Bad iovec");
          for (OFC_INT j = 0; j < 100; j++)
            ofc_assert(((OFC_UCHAR *)(iovec[i].iov_base))[j] == 0x04,
                       "IOVEC: Bad iovec");
        }
      else if (i == 6)
        {
          ofc_assert(iovec[i].iov_len == 100, "IOVEC: Bad iovec");
          for (OFC_INT j = 0; j < 100; j++)
            ofc_assert(((OFC_UCHAR *)(iovec[i].iov_base))[j] == 0x05,
                       "IOVEC: Bad iovec");
        }
    }
  ofc_free(iovec);

  /*
   * This one should have the same number of vectors, but it should be
   * 100 bytes shorter.  50 less in the first one and 50 less in the last
   * one.
   */
  ofc_iovec_get(list, 50, 
                ofc_iovec_length(list)-50,
                &iovec, &veclen);
  ofc_assert (veclen == 7, "IOVEC: Bad get");

  for (int i = 0; i < 7; i++)
    {
      if (i == 0)
        {
          ofc_assert(iovec[i].iov_len == 50, "IOVEC: Bad iovec");
          for (OFC_INT j = 0; j < 50; j++)
            {
              ofc_assert(((OFC_UCHAR *)(iovec[i].iov_base))[j] == 0x01,
                         "IOVEC: Bad iovec");
            }
        }
      else if (i == 1)
        {
          ofc_assert(iovec[i].iov_len == 100, "IOVEC: Bad iovec");
          for (OFC_INT j = 0; j < 100; j++)
            ofc_assert(((OFC_UCHAR *)(iovec[i].iov_base))[j] == 0x02,
                       "IOVEC: Bad iovec");
        }
      else if (i == 2)
        {
          ofc_assert(iovec[i].iov_len == 50, "IOVEC: Bad iovec");
          for (OFC_INT j = 0; j < 50; j++)
            ofc_assert(((OFC_UCHAR *)(iovec[i].iov_base))[j] == 0x03,
                       "IOVEC: Bad iovec");
        }
      else if (i == 3)
        {
          ofc_assert(iovec[i].iov_len == 50, "IOVEC: Bad iovec");
          for (OFC_INT j = 0; j < 50; j++)
            ofc_assert(((OFC_UCHAR *)(iovec[i].iov_base))[j] == 0x35,
                       "IOVEC: Bad iovec");
        }
      else if (i == 4)
        {
          ofc_assert(iovec[i].iov_len == 50, "IOVEC: Bad iovec");
          for (OFC_INT j = 0; j < 50; j++)
            ofc_assert(((OFC_UCHAR *)(iovec[i].iov_base))[j] == 0x03,
                       "IOVEC: Bad iovec");
        }
      else if (i == 5)
        {
          ofc_assert(iovec[i].iov_len == 100, "IOVEC: Bad iovec");
          for (OFC_INT j = 0; j < 100; j++)
            ofc_assert(((OFC_UCHAR *)(iovec[i].iov_base))[j] == 0x04,
                       "IOVEC: Bad iovec");
        }
      else if (i == 6)
        {
          ofc_assert(iovec[i].iov_len == 50, "IOVEC: Bad iovec");
          for (OFC_INT j = 0; j < 50; j++)
            ofc_assert(((OFC_UCHAR *)(iovec[i].iov_base))[j] == 0x05,
                       "IOVEC: Bad iovec");
        }
    }
  
  ofc_free(iovec);
  
  ofc_iovec_destroy(list);
}          
    
TEST_GROUP_RUNNER(iovec) {
    RUN_TEST_CASE(iovec, test_iovec);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(iovec);
}

int main(int argc, const char *argv[])
{
  return UnityMain(argc, argv, runAllTests);
}
#endif
