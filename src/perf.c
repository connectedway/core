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
#include "ofc/lock.h"

/*
 * Little's Law Brief

L = λ x W
Work in Progress (L) is the number of items in process in any system.
Throughput (λ) represents the rate at which items arrive in/out of the system.
Lead time (W) is the average time one item spends in the system.

WIP = Throughput x Lead Time
Throughput = WIP / Lead time

WIP(packets) is number of packets in the system
Throughput(packets) is the number of packets per second.
Lead(packets) is average time a packet spends in the system.

WIP(packets) = Number of requests = 250
Average Queue Depth x 1000 = 5000
Lead(packets) = Queue Basis(ms) / Average Queue Depth x 1000 = 5.06 (s)

Throughput(packets/sec) = WIP(packets) / Lead(packets) = 49.40 p/sec

Total Byte Count = 256000
Number of Requests = 250
average bytes per packet = Total Byte Count / Number of requests = 1024
Throughput(bytes/sec) = Throughput (packets/sec) * average bytes per packet
Throughput(bytes/sec) = 49.40 * 1024 = 50585 bytes/second

*/

struct perf_measurement *g_measurement;

OFC_VOID measurement_init(OFC_VOID)
{
  g_measurement = measurement_alloc();
}

OFC_VOID measurement_destroy(OFC_VOID)
{
  measurement_free(g_measurement);
}

OFC_VOID measurement_wait(struct perf_measurement *measurement)
{
  OFC_HANDLE wait_event;

  if (measurement != OFC_HANDLE_NULL)
    {
      wait_event = ofc_event_create(OFC_EVENT_AUTO);
      measurement_stop(measurement, wait_event);
      ofc_event_wait(wait_event);
      ofc_event_destroy(wait_event);
    }
}

struct perf_measurement *measurement_alloc (OFC_VOID)
{
  struct perf_measurement *measurement;

  measurement =
    (struct perf_measurement *) ofc_malloc(sizeof(struct perf_measurement));

  measurement->nqueues = 0;
  measurement->queues = ofc_queue_create();
  measurement->nrts = 0;
  measurement->rts = ofc_queue_create();
  measurement->notify = OFC_HANDLE_NULL;
  measurement->stop = OFC_FALSE;
  measurement->lock = ofc_lock_init();
  return (measurement);
}

OFC_VOID measurement_free(struct perf_measurement *measurement)
{
  struct perf_queue *queue;
  struct perf_rt *rt;

  if (measurement->hThread != OFC_NULL)
    {
      ofc_thread_delete(measurement->hThread);
      ofc_thread_wait(measurement->hThread);
    }

  for (queue = ofc_dequeue(measurement->queues);
       queue != OFC_NULL;
       queue = ofc_dequeue(measurement->queues))
    {
      perf_queue_destroy(measurement, queue);
    }
  ofc_queue_destroy(measurement->queues);
  
  for (rt = ofc_dequeue(measurement->rts);
       rt != OFC_NULL;
       rt = ofc_dequeue(measurement->rts))
    {
      perf_rt_destroy(measurement, rt);
    }
  ofc_queue_destroy(measurement->rts);
  ofc_lock_destroy(measurement->lock);
  ofc_free(measurement);
}

static OFC_DWORD measurement_thread(OFC_HANDLE hThread, OFC_VOID *context)
{
  struct perf_measurement *measurement;

  measurement = (struct perf_measurement *) context;

  while (!ofc_thread_is_deleting(hThread))
    {
      ofc_sleep(10);
      measurement_poll(measurement);
    }
  return(0);
}

OFC_BOOL measurement_start(struct perf_measurement *measurement)
{
  static OFC_UINT instance = 0;
  struct perf_queue *queue;
  struct perf_rt *rt;

  measurement->start_stamp = ofc_time_get_now();
  measurement->stop = OFC_FALSE;

  for (queue = ofc_queue_first(measurement->queues);
       queue != OFC_NULL;
       queue = ofc_queue_next(measurement->queues, queue))
    {
      perf_queue_reset(queue);
    }

  for (rt = ofc_queue_first(measurement->rts);
       rt != OFC_NULL;
       rt = ofc_queue_next(measurement->rts, rt))
    {
      perf_rt_reset(rt);
    }

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
  struct perf_rt *rt;
  struct perf_statistics statistics;

  static char *perf_stats_header =
    "%13s %8s %10s %9s %10s %10s %10s\n";
  ofc_printf(perf_stats_header,
	     "    Queue    ",
	     " Elapsed",
	     "Total Byte",
	     "Number of",
	     " Avg Queue",
	     "    Req   ",
	     "   Byte   ");
  ofc_printf(perf_stats_header,
	     "     Name    ",
	     "  Time  ",
	     "   Count  ",
	     "  Requests",
	     "   Depth  ",
	     "Throughput",
	     "Throughput");
  ofc_printf(perf_stats_header,
	     "             ",
	     "  (ms)  ",
	     "          ",
	     "         ",
	     "          ",
	     "   (pps)  ",
	     "  (Kbps)  ");

  for (queue = ofc_queue_first(measurement->queues);
       queue != OFC_NULL;
       queue = ofc_queue_next(measurement->queues, queue))
    {
      perf_queue_statistics(measurement, queue, &statistics);
      perf_statistics_print(&statistics);
    }
  ofc_printf("\n");

  static char *perf_rt_header = "%13s %11s\n";
  ofc_printf(perf_rt_header, "   Runtime   ", "  runtime ");
  ofc_printf(perf_rt_header, "     Name    ", "   (ms)   ");
  for (rt = ofc_queue_first(measurement->rts);
       rt != OFC_NULL;
       rt = ofc_queue_next(measurement->rts, rt))
    {
      static char *perf_rt_format = "%10.10S:%02d %7d.%03d\n";

      ofc_printf(perf_rt_format,
		 rt->description,
		 rt->instance,
		 rt->total / 1000, rt->total % 1000);
    }
  return OFC_TRUE;
}

OFC_VOID measurement_poll(struct perf_measurement *measurement)
{
  struct perf_queue *queue;

  ofc_lock(measurement->lock);
  for (queue = ofc_queue_first(measurement->queues);
       queue != OFC_NULL;
       queue = ofc_queue_next(measurement->queues, queue))
    {
      perf_queue_poll(measurement, queue);
    }
  ofc_unlock(measurement->lock);
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
    }
  return (ret);
}

OFC_VOID perf_queue_reset(struct perf_queue *queue)
{
  queue->basis = 0;
  queue->num_requests = 0;
  queue->total_byte_count = 0;
  queue->depth = 0;
  queue->total_depth = 0;
  queue->depth_samples = 0;
}

struct perf_queue *
perf_queue_create (struct perf_measurement *measurement,
		   OFC_CTCHAR *description,
		   OFC_INT instance)
{
  struct perf_queue *queue;

  queue = ofc_malloc(sizeof (struct perf_queue));

  queue->lock = ofc_lock_init();
  queue->description = description;
  queue->instance = instance;
  perf_queue_reset(queue);

  ofc_enqueue (measurement->queues, queue);
  measurement->nqueues++;
  return (queue);
}

OFC_VOID perf_queue_destroy(struct perf_measurement *measurement,
			    struct perf_queue *queue)
{
  ofc_queue_unlink (measurement->queues, queue);
  ofc_lock_destroy(queue->lock);
  measurement->nqueues--;
  ofc_free(queue);
}
				   
OFC_VOID perf_rt_reset(struct perf_rt *rt)
{
  rt->total = 0;
  rt->start = 0;
}

struct perf_rt *
perf_rt_create (struct perf_measurement *measurement,
		OFC_CTCHAR *description,
		OFC_INT instance)
{
  struct perf_rt *rt;

  rt = ofc_malloc(sizeof (struct perf_rt));

  rt->description = description;
  rt->instance = instance;
  perf_rt_reset(rt);

  ofc_enqueue (measurement->rts, rt);
  measurement->nrts++;
  return (rt);
}

OFC_VOID perf_rt_destroy(struct perf_measurement *measurement,
			 struct perf_rt *rt)
{
  ofc_queue_unlink (measurement->rts, rt);
  measurement->nrts--;
  ofc_free(rt);
}

OFC_VOID perf_rt_start(struct perf_rt *rt)
{
  rt->start = ofc_get_runtime();
}

OFC_VOID perf_rt_stop(struct perf_rt *rt)
{
  rt->total = ofc_get_runtime() - rt->start;
}
				   
OFC_VOID perf_statistics_print(struct perf_statistics *statistics)
{
  static char *perf_stats_format =
    "%10.10S:%02d %8d %10d %9d %6d.%03d %10d %10d\n";

  ofc_printf(perf_stats_format,
	     statistics->description,
	     statistics->instance,
	     statistics->elapsed_ms,
	     statistics->total_byte_count,
	     statistics->num_requests,
	     statistics->average_depth_x1000 / 1000,
	     statistics->average_depth_x1000 % 1000,
	     statistics->request_throughput,
	     (statistics->request_throughput * statistics->avg_packet_size) /
	     1000);
}

OFC_VOID perf_queue_statistics(struct perf_measurement *measurement,
			       struct perf_queue *queue,
			       struct perf_statistics *statistics)
{
  ofc_lock(queue->lock);
  statistics->description = queue->description;
  statistics->instance = queue->instance;
  statistics->elapsed_ms = measurement->stop_stamp - measurement->start_stamp;
  statistics->total_byte_count = queue->total_byte_count;
  statistics->num_requests = queue->num_requests;
  statistics->avg_packet_size = queue->total_byte_count / queue->num_requests;
  statistics->depth_samples = queue->depth_samples;
  statistics->total_depth = queue->total_depth;
  statistics->average_depth_x1000 = (queue->total_depth * 1000) /
    queue->depth_samples;
  statistics->basis = queue->basis;
  statistics->lead_x1000 = (queue->basis * 1000) /
    statistics->average_depth_x1000 ;
  statistics->request_throughput = (queue->num_requests * 1000) /
    statistics->lead_x1000;
  ofc_unlock(queue->lock);
}

OFC_VOID perf_request_start (struct perf_measurement *measurement,
			     struct perf_queue *queue)
{
  ofc_lock(queue->lock);
  if (!measurement->stop)
    {
      queue->basis -= ofc_time_get_now();
      queue->num_requests++;
      queue->depth++;
    }
  ofc_unlock(queue->lock);
}

OFC_VOID perf_request_stop (struct perf_measurement *measurement,
			    struct perf_queue *queue,
			    OFC_LONG byte_count)
{
  ofc_lock(queue->lock);
  if (queue->depth > 0)
    {
      queue->basis += ofc_time_get_now();
      queue->depth--;
      if (byte_count > 0)
	queue->total_byte_count += byte_count;

      measurement_notify(measurement);
    }
  ofc_unlock(queue->lock);
}

OFC_VOID perf_queue_poll(struct perf_measurement *measurement, 
			 struct perf_queue *queue)
{
  ofc_lock(queue->lock);
  if (queue->depth > 0)
    { 
      queue->total_depth += queue->depth; 
      queue->depth_samples++; 
    }
  ofc_unlock(queue->lock);
}
