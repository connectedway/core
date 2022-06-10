/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#include "unity.h"
#include "unity_fixture.h"

#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/thread.h"
#include "ofc/timer.h"
#include "ofc/event.h"
#include "ofc/libc.h"
#include "ofc/heap.h"
#include "ofc/event.h"
#include "ofc/perf.h"
#include "ofc/core.h"

OFC_VOID test_shutdown(OFC_VOID);
OFC_INT test_startup(OFC_VOID);

TEST_GROUP(perf);

TEST_SETUP(perf) {
    TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(perf) {
    test_shutdown();
}

TEST(perf, test_perf)
{
  struct perf_measurement *measurement;
  static const OFC_INT NQUEUES = 5;
  static const OFC_TCHAR *description = TSTR("Performance Test Queue");
  OFC_INT i;
  OFC_INT j;
  struct perf_queue *queue[NQUEUES];
  OFC_HANDLE wait_event;

  measurement = measurement_alloc();

  for (i = 0 ; i < NQUEUES ; i++)
    {
      queue[i] = perf_queue_create(measurement, description, i);
    }

  measurement_start(measurement);

  for (i = 0; i < NQUEUES; i++)
    {
      for (j = i; j < NQUEUES; j++)
	{
	  perf_request_start(measurement, queue[j]);
	}
      ofc_printf("Sleeping 1 second\n");
      ofc_sleep(1000);
    }

  ofc_sleep(1000);

  for (i = 0; i < NQUEUES; i++)
    {
      for (j = i; j < NQUEUES; j++)
	{
	  perf_request_stop(measurement, queue[j], 1024);
	}
      ofc_printf("Sleeping 1 second\n");
      ofc_sleep(1000);
    }

  wait_event = ofc_event_create(OFC_EVENT_AUTO);
  measurement_stop(measurement, wait_event);
  ofc_event_wait(wait_event);
  ofc_event_destroy(wait_event);

  measurement_statistics(measurement);

  measurement_free(measurement);
}

TEST_GROUP_RUNNER(perf) {
    RUN_TEST_CASE(perf, test_perf);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(perf);
}

int main(int argc, const char *argv[])
{
  return UnityMain(argc, argv, runAllTests);
}
#endif
