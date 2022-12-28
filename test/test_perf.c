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

struct perf_context
{
  OFC_HANDLE hThread;
  struct perf_measurement *measurement;
  struct perf_queue *queue;
} ;

static OFC_DWORD perf_thread(OFC_HANDLE hThread, OFC_VOID *context)
{
  struct perf_context *queue_context = context;

  /*
   * The test thread should continue running until it is deleted
   */

  while (!ofc_thread_is_deleting(hThread))
    {
      int num = rand() % 10; /* average 5 */
      for (int i = 0 ; i < num ; i++)
	perf_request_start(queue_context->measurement,
			   queue_context->queue);
      /*
       * Wait for a 100ms
       */
      ofc_sleep(rand() % 200); /* average 100 */
      for (int i = 0 ; i < num ; i++)
	perf_request_stop(queue_context->measurement,
			  queue_context->queue, rand()%2048);
    }

    return (0);
}

TEST(perf, test_perf)
{
  static const OFC_INT NQUEUES = 5;
  static const OFC_TCHAR *description = TSTR("Test Queue");
  OFC_INT i;
  struct perf_measurement *measurement;
  struct perf_context perf_context[NQUEUES];
  OFC_HANDLE wait_event;

  measurement = measurement_alloc();

  for (i = 0 ; i < NQUEUES ; i++)
    {
      perf_context[i].measurement = measurement;
      perf_context[i].queue = perf_queue_create(measurement, description, i);

      perf_context[i].hThread = 
	ofc_thread_create(&perf_thread,
			  OFC_THREAD_THREAD_TEST, i,
			  &perf_context[i],
			  OFC_THREAD_JOIN,
                          OFC_HANDLE_NULL);
    }

  measurement_start(measurement);

  ofc_sleep(5000);

  wait_event = ofc_event_create(OFC_EVENT_AUTO);
  measurement_stop(measurement, wait_event);
  ofc_event_wait(wait_event);
  ofc_event_destroy(wait_event);

  for (i = 0; i < NQUEUES; i++)
    {
      ofc_thread_delete(perf_context[i].hThread);
      ofc_thread_wait(perf_context[i].hThread);
    }

  measurement_statistics(measurement);

  for (i = 0; i < NQUEUES; i++)
    {
      perf_queue_destroy(measurement, perf_context[i].queue);
    }

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
