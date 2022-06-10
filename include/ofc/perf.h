/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_PERF_H__)
#define __OFC_PERF_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/handle.h"

/** \{ */

struct perf_measurement {
  OFC_MSTIME start_stamp;
  OFC_MSTIME stop_stamp;
  OFC_BOOL stop;
  OFC_INT nqueues;
  OFC_HANDLE queues;
  OFC_HANDLE notify;
  OFC_HANDLE hThread;
  OFC_UINT instance;
} ;
  
struct perf_queue {
  OFC_MSTIME basis;
  OFC_ULONG num_requests;
  OFC_ULONG total_byte_count;
  OFC_UINT depth ;
  OFC_UINT total_depth;
  OFC_UINT depth_samples;
  OFC_CTCHAR *description;
  OFC_INT instance;
};

struct perf_statistics {
  OFC_CTCHAR *description;
  OFC_INT instance;
  OFC_LONG elapsed_ms;
  OFC_LONG total_byte_count;
  OFC_LONG byte_throughput;
  OFC_LONG num_requests;
  OFC_LONG request_throughput;
  OFC_LONG average_depth_x100;
  OFC_LONG basis;
  OFC_LONG overhead;
  OFC_LONG normalized_throughput;
};

#if defined(__cplusplus)
extern "C"
{
#endif
  struct perf_measurement *measurement_alloc (OFC_VOID);
  OFC_VOID measurement_free(struct perf_measurement *measurement);
  OFC_BOOL measurement_start(struct perf_measurement *measurement);
  OFC_BOOL measurement_statistics(struct perf_measurement *measurement);
  OFC_BOOL measurement_stop(struct perf_measurement *measurement,
			    OFC_HANDLE notify);
  OFC_VOID measurement_poll(struct perf_measurement *measurement);
  OFC_BOOL measurement_notify(struct perf_measurement *measurement);
  struct perf_queue *
  perf_queue_create (struct perf_measurement *measurement,
		     OFC_CTCHAR *description,
		     OFC_INT instance);
  OFC_VOID perf_queue_destroy(struct perf_measurement *measurement,
			      struct perf_queue *queue);
  OFC_VOID perf_queue_statistics(struct perf_measurement *measurement,
				 struct perf_queue *queue,
				 struct perf_statistics *statistics);
  OFC_VOID perf_request_start (struct perf_measurement *measurement,
			       struct perf_queue *queue);
  OFC_VOID perf_request_stop (struct perf_measurement *measurement,
			      struct perf_queue *queue,
			      OFC_ULONG byte_count);
  OFC_VOID perf_queue_poll(struct perf_measurement *measurement,
			   struct perf_queue *queue);
  OFC_VOID perf_statistics_print(struct perf_statistics *statistics);
#if defined(__cplusplus)
}
#endif
/** \} */
#endif

