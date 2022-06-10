/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/time.h"
#include "ofc/timer.h"
#include "ofc/handle.h"
#include "ofc/heap.h"
#include "ofc/queue.h"
#include "ofc/perf.h"
#include "ofc/event.h"
#include "ofc/thread.h"

struct perf_measurement *measurement_alloc (OFC_VOID)
{
  struct perf_measurement *measurement;

  measurement =
    (struct perf_measurement *) ofc_malloc(sizeof(struct perf_measurement));

  measurement->nqueues = 0;
  measurement->queues = ofc_queue_create();
  measurement->notify = OFC_HANDLE_NULL;
  measurement->stop = OFC_FALSE;
  return (measurement);
}

OFC_VOID measurement_free(struct perf_measurement *measurement)
{
  struct perf_queue *queue;

  ofc_thread_delete(measurement->hThread);
  ofc_thread_wait(measurement->hThread);

  for (queue = ofc_dequeue(measurement->queues);
       queue != OFC_NULL;
       queue = ofc_dequeue(measurement->queues))
    {
      perf_queue_destroy(measurement, queue);
    }
  ofc_queue_destroy(measurement->queues);
  ofc_free(measurement);
}

static OFC_DWORD measurement_thread(OFC_HANDLE hThread, OFC_VOID *context)
{
  struct perf_measurement *measurement;

  measurement = (struct perf_measurement *) context;

  while (!ofc_thread_is_deleting(hThread))
    {
      ofc_sleep(20);
      measurement_poll(measurement);
    }
  return(0);
}

OFC_BOOL measurement_start(struct perf_measurement *measurement)
{
  static OFC_UINT instance = 0;

  measurement->start_stamp = ofc_time_get_now();
  measurement->stop = OFC_FALSE;

  measurement->instance = instance++;
  measurement->hThread = ofc_thread_create(&measurement_thread,
					   OFC_THREAD_MEASUREMENT_PERF,
					   measurement->instance,
					   measurement,
					   OFC_THREAD_JOIN,
					   OFC_HANDLE_NULL);
  return (OFC_TRUE);
}
  
OFC_BOOL measurement_stop(struct perf_measurement *measurement,
			  OFC_HANDLE notify)
{
  OFC_BOOL ret;

  if (! measurement->stop)
    {
      measurement->stop = OFC_TRUE;
    }

  measurement->notify = notify;
  ret = measurement_notify(measurement);

  return (ret);
}

OFC_BOOL measurement_statistics(struct perf_measurement *measurement)
{
  struct perf_queue *queue;
  struct perf_statistics statistics;

  for (queue = ofc_queue_first(measurement->queues);
       queue != OFC_NULL;
       queue = ofc_queue_next(measurement->queues, queue))
    {
      perf_queue_statistics(measurement, queue, &statistics);
      perf_statistics_print(&statistics);
    }
  return OFC_TRUE;
}

OFC_VOID measurement_poll(struct perf_measurement *measurement)
{
  struct perf_queue *queue;

  for (queue = ofc_queue_first(measurement->queues);
       queue != OFC_NULL;
       queue = ofc_queue_next(measurement->queues, queue))
    {
      perf_queue_poll(measurement, queue);
    }
}  

OFC_BOOL measurement_notify(struct perf_measurement *measurement)
{
  struct perf_queue *queue;
  OFC_BOOL ret = OFC_FALSE ;

  if (measurement->stop)
    {
      for (queue = ofc_queue_first(measurement->queues);
	   queue!= OFC_NULL && queue->depth == 0;
	   queue = ofc_queue_next(measurement->queues, queue));

      if (queue == OFC_NULL)
	{
	  measurement->stop_stamp = ofc_time_get_now();
	  if (measurement->notify != OFC_HANDLE_NULL)
	    ofc_event_set(measurement->notify);
	  ret = OFC_TRUE;
	}
      else
	{
	  ofc_printf("Waiting for %S, depth %d\n",
		     queue->description, queue->depth);
	}
    }
  return (ret);
}

struct perf_queue *
perf_queue_create (struct perf_measurement *measurement,
		   OFC_CTCHAR *description,
		   OFC_INT instance)
{
  struct perf_queue *queue;

  queue = ofc_malloc(sizeof (struct perf_queue));

  queue->description = description;
  queue->instance = instance;
  queue->basis = 0;
  queue->num_requests = 0;
  queue->total_byte_count = 0;
  queue->depth = 0;
  queue->total_depth = 0;
  queue->depth_samples = 0;

  ofc_enqueue (measurement->queues, queue);
  measurement->nqueues++;
  return (queue);
}

OFC_VOID perf_queue_destroy(struct perf_measurement *measurement,
			    struct perf_queue *queue)
{
  ofc_queue_unlink (measurement->queues, queue);
  measurement->nqueues--;
  ofc_free(queue);
}
				   
OFC_VOID perf_statistics_print(struct perf_statistics *statistics)
{
  ofc_printf("queue %S:%d\n", statistics->description, statistics->instance);
  ofc_printf("Elapsed MS %d\n", statistics->elapsed_ms);
  ofc_printf("Total Byte Count %d\n", statistics->total_byte_count);
  ofc_printf("Byte throughput %d Bps\n", statistics->byte_throughput);
  ofc_printf("Number of requests %d\n", statistics->num_requests);
  ofc_printf("Request Throughput %d REQps\n", statistics->request_throughput);
  ofc_printf("Average Queue Depth (x100) %d\n",
	     statistics->average_depth_x100) ;
  ofc_printf("Queue Basis %d\n", statistics->basis);
  ofc_printf("Queue Overhead %d ms/request\n", statistics->overhead);
  ofc_printf("Queue Throughput %d byte/sec\n",
	     statistics->normalized_throughput);
  ofc_printf("\n");
}

OFC_VOID perf_queue_statistics(struct perf_measurement *measurement,
			       struct perf_queue *queue,
			       struct perf_statistics *statistics)
{
  statistics->description = queue->description;
  statistics->instance = queue->instance;
  statistics->elapsed_ms = measurement->stop_stamp - measurement->start_stamp;
  statistics->total_byte_count = queue->total_byte_count;
  statistics->byte_throughput = (queue->total_byte_count * 1000) /
    statistics->elapsed_ms;
  statistics->num_requests = queue->num_requests;
  statistics->request_throughput = (queue->num_requests * 1000) /
    statistics->elapsed_ms;
  statistics->average_depth_x100 = (queue->total_depth * 100) /
    queue->depth_samples;
  statistics->basis = queue->basis;
  statistics->overhead = (queue->basis * 100) /
    statistics->average_depth_x100;
  statistics->normalized_throughput = (queue->total_byte_count *
				       queue->basis * 100) /
    statistics->average_depth_x100;
}

OFC_VOID perf_request_start (struct perf_measurement *measurement,
			     struct perf_queue *queue)
{
  if (!measurement->stop)
    {
      queue->basis -= ofc_time_get_now();
      queue->num_requests++;
      queue->depth++;
    }
}

OFC_VOID perf_request_stop (struct perf_measurement *measurement,
			    struct perf_queue *queue,
			    OFC_ULONG byte_count)
{
  queue->basis += ofc_time_get_now();
  queue->depth--;
  queue->total_byte_count += byte_count;

  measurement_notify(measurement);

}

OFC_VOID perf_queue_poll(struct perf_measurement *measurement,
			 struct perf_queue *queue)
{
  queue->total_depth += queue->depth;
  queue->depth_samples++;
}
