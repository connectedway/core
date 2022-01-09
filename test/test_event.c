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
#include "ofc/timer.h"
#include "ofc/event.h"
#include "ofc/libc.h"
#include "ofc/sched.h"
#include "ofc/app.h"
#include "ofc/heap.h"
#include "ofc/event.h"
#include "ofc/core.h"
#include "ofc/framework.h"
#include "ofc/env.h"
#include "ofc/persist.h"

extern OFC_CHAR config_path[OFC_MAX_PATH+1];

extern OFC_HANDLE hScheduler;
extern OFC_HANDLE hDone;

OFC_VOID test_shutdown(OFC_VOID);
OFC_INT test_startup(OFC_VOID);

typedef enum {
    EVENT_TEST_STATE_IDLE,
    EVENT_TEST_STATE_RUNNING,
} EVENT_TEST_STATE;

#define EVENT_TEST_INTERVAL 2000

typedef struct {
    EVENT_TEST_STATE state;
    OFC_HANDLE hTimer;
    OFC_HANDLE hEvent;
    OFC_HANDLE scheduler;
    OFC_INT count;
} OFC_EVENT_TEST;

static OFC_VOID EventTestPreSelect(OFC_HANDLE app);

static OFC_HANDLE EventTestPostSelect(OFC_HANDLE app, OFC_HANDLE hEvent);

static OFC_VOID EventTestDestroy(OFC_HANDLE app);

#if defined(OFC_APP_DEBUG)
static OFC_VOID EventTestDump (OFC_HANDLE app) ;
#endif

static OFC_APP_TEMPLATE EventTestAppDef =
        {
                "Event Test Application",
                &EventTestPreSelect,
                &EventTestPostSelect,
                &EventTestDestroy,
#if defined(OFC_APP_DEBUG)
                &EventTestDump
#else
                OFC_NULL
#endif
        };

#if defined(OFC_APP_DEBUG)
static OFC_VOID EventTestDump (OFC_HANDLE app)
{
  OFC_EVENT_TEST *eventTest ;

  eventTest = ofc_app_get_data (app) ;
  if (eventTest != OFC_NULL)
    {
      ofc_printf ("%-20s : %d\n", "Event Test State", 
           (OFC_UINT16) eventTest->state) ;
      ofc_printf ("\n") ;
    }
}
#endif

static OFC_VOID EventTestPreSelect(OFC_HANDLE app) {
    OFC_EVENT_TEST *eventTest;
    EVENT_TEST_STATE entry_state;

    eventTest = ofc_app_get_data(app);
    if (eventTest != OFC_NULL) {
        do /* while eventTest->state != entry_state */
        {
            entry_state = eventTest->state;
            ofc_sched_clear_wait(eventTest->scheduler, app);

            switch (eventTest->state) {
                default:
                case EVENT_TEST_STATE_IDLE:
                    eventTest->hEvent = ofc_event_create(OFC_EVENT_AUTO);
                    if (eventTest->hEvent != OFC_HANDLE_NULL) {
                        ofc_sched_add_wait(eventTest->scheduler, app,
                                           eventTest->hEvent);
                        eventTest->hTimer = ofc_timer_create("EVENT");
                        if (eventTest->hTimer != OFC_HANDLE_NULL) {
                            ofc_timer_set(eventTest->hTimer, EVENT_TEST_INTERVAL);
                            ofc_sched_add_wait(eventTest->scheduler, app,
                                               eventTest->hTimer);
                            eventTest->state = EVENT_TEST_STATE_RUNNING;
                        }
                    }
                    break;

                case EVENT_TEST_STATE_RUNNING:
                    ofc_sched_add_wait(eventTest->scheduler, app, eventTest->hEvent);
                    ofc_sched_add_wait(eventTest->scheduler, app, eventTest->hTimer);
                    break;
            }
        } while (eventTest->state != entry_state);
    }
}

static OFC_HANDLE EventTestPostSelect(OFC_HANDLE app, OFC_HANDLE hEvent) {
    OFC_EVENT_TEST *eventTest;
    OFC_BOOL progress;

    eventTest = ofc_app_get_data(app);
    if (eventTest != OFC_NULL) {
        for (progress = OFC_TRUE; progress && !ofc_app_destroying(app);) {
            progress = OFC_FALSE;

            switch (eventTest->state) {
                default:
                case EVENT_TEST_STATE_IDLE:
                    break;

                case EVENT_TEST_STATE_RUNNING:
                    if (hEvent == eventTest->hEvent) {
                        ofc_printf("Event Triggered\n");
                        eventTest->count++;

                        if (eventTest->count >= 10)
                            ofc_app_kill(app);
                    } else if (hEvent == eventTest->hTimer) {
                        ofc_event_set(eventTest->hEvent);
                        ofc_timer_set(eventTest->hTimer, EVENT_TEST_INTERVAL);
                        ofc_printf("Timer Triggered\n");
                    }
                    break;
            }
        }
    }
    return (OFC_HANDLE_NULL);
}

static OFC_VOID EventTestDestroy(OFC_HANDLE app) {
    OFC_EVENT_TEST *eventTest;

    eventTest = ofc_app_get_data(app);
    if (eventTest != OFC_NULL) {
        switch (eventTest->state) {
            default:
            case EVENT_TEST_STATE_IDLE:
                break;

            case EVENT_TEST_STATE_RUNNING:
                ofc_timer_destroy(eventTest->hTimer);
                ofc_event_destroy(eventTest->hEvent);
                break;
        }

        ofc_free(eventTest);
    }
}

TEST_GROUP(event);

TEST_SETUP(event) {
    TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(event) {
    test_shutdown();
}

TEST(event, test_event) {
    OFC_EVENT_TEST *eventTest;
    OFC_HANDLE hApp;

    eventTest = ofc_malloc(sizeof(OFC_EVENT_TEST));
    eventTest->count = 0;
    eventTest->state = EVENT_TEST_STATE_IDLE;
    eventTest->scheduler = hScheduler;

    hApp = ofc_app_create(hScheduler, &EventTestAppDef, eventTest);

    if (hDone != OFC_HANDLE_NULL) {
        ofc_app_set_wait(hApp, hDone);
        ofc_event_wait(hDone);
    }
}

TEST_GROUP_RUNNER(event) {
    RUN_TEST_CASE(event, test_event);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(event);
}

int main(int argc, const char *argv[])
{
  if (argc >= 2) {
    if (ofc_strcmp(argv[1], "--config") == 0) {
      ofc_strncpy(config_path, argv[2], OFC_MAX_PATH);
    }
  }

  return UnityMain(argc, argv, runAllTests);
}
#endif
