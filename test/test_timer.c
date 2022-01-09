/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#include "unity.h"
#include "unity_fixture.h"

#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/framework.h"
#include "ofc/persist.h"
#include "ofc/env.h"
#include "ofc/handle.h"
#include "ofc/app.h"
#include "ofc/sched.h"
#include "ofc/timer.h"
#include "ofc/libc.h"
#include "ofc/heap.h"
#include "ofc/event.h"

extern OFC_CHAR config_path[OFC_MAX_PATH+1];

extern OFC_HANDLE hScheduler;
extern OFC_HANDLE hDone;

OFC_VOID test_shutdown(OFC_VOID);
OFC_INT test_startup(OFC_VOID);

typedef enum {
    TIMER_TEST_STATE_IDLE,
    TIMER_TEST_STATE_RUNNING,
} TIMER_TEST_STATE;

#define TIMER_TEST_INTERVAL 1000
#define TIMER_TEST_COUNT 10

typedef struct {
    TIMER_TEST_STATE state;
    OFC_HANDLE hTimer;
    OFC_HANDLE scheduler;
    OFC_INT count;
} OFC_TIMER_TEST;

static OFC_VOID TimerTestPreSelect(OFC_HANDLE app);

static OFC_HANDLE TimerTestPostSelect(OFC_HANDLE app, OFC_HANDLE hEvent);

static OFC_VOID TimerTestDestroy(OFC_HANDLE app);

static OFC_APP_TEMPLATE TimerTestAppDef =
        {
                "Timer Test Application",
                &TimerTestPreSelect,
                &TimerTestPostSelect,
                &TimerTestDestroy,
#if defined(OFC_APP_DEBUG)
                OFC_NULL
#endif
        };

static OFC_VOID TimerTestPreSelect(OFC_HANDLE app) {
    OFC_TIMER_TEST *timerTest;
    TIMER_TEST_STATE entry_state;

    timerTest = ofc_app_get_data(app);
    if (timerTest != OFC_NULL) {
        do {
            entry_state = timerTest->state;
            ofc_sched_clear_wait(timerTest->scheduler, app);

            switch (timerTest->state) {
                default:
                case TIMER_TEST_STATE_IDLE:
                    timerTest->hTimer = ofc_timer_create("TIMER TEST");
                    if (timerTest->hTimer != OFC_HANDLE_NULL) {
                        ofc_timer_set(timerTest->hTimer, TIMER_TEST_INTERVAL);
                        timerTest->state = TIMER_TEST_STATE_RUNNING;
                        ofc_sched_add_wait(timerTest->scheduler, app,
                                           timerTest->hTimer);
                    }
                    break;

                case TIMER_TEST_STATE_RUNNING:
                    ofc_sched_add_wait(timerTest->scheduler, app, timerTest->hTimer);
                    break;
            }
        } while (timerTest->state != entry_state);
    }
}

static OFC_HANDLE TimerTestPostSelect(OFC_HANDLE app, OFC_HANDLE hEvent) {
    OFC_TIMER_TEST *timerTest;
    OFC_BOOL progress;

    timerTest = ofc_app_get_data(app);
    if (timerTest != OFC_NULL) {
        for (progress = OFC_TRUE; progress && !ofc_app_destroying(app);) {
            progress = OFC_FALSE;

            switch (timerTest->state) {
                default:
                case TIMER_TEST_STATE_IDLE:
                    break;

                case TIMER_TEST_STATE_RUNNING:
                    if (hEvent == timerTest->hTimer) {
                        ofc_printf("Timer Expired %d\n", timerTest->count);
                        timerTest->count++;
                        if (timerTest->count >= TIMER_TEST_COUNT) {
                            ofc_app_kill(app);
                        } else
                            ofc_timer_set(timerTest->hTimer, TIMER_TEST_INTERVAL);
                    }
                    break;
            }
        }
    }
    return (OFC_HANDLE_NULL);
}

static OFC_VOID TimerTestDestroy(OFC_HANDLE app) {
    OFC_TIMER_TEST *timerTest;

    timerTest = ofc_app_get_data(app);
    if (timerTest != OFC_NULL) {
        switch (timerTest->state) {
            default:
            case TIMER_TEST_STATE_IDLE:
                break;

            case TIMER_TEST_STATE_RUNNING:
                ofc_timer_destroy(timerTest->hTimer);
                break;
        }

        ofc_free(timerTest);
    }
}

TEST_GROUP(timer);

TEST_SETUP(timer) {
    TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(timer) {
    test_shutdown();
}

TEST(timer, test_timer) {
    OFC_TIMER_TEST *timerTest;
    OFC_HANDLE hApp;

    timerTest = ofc_malloc(sizeof(OFC_TIMER_TEST));
    timerTest->count = 0;
    timerTest->state = TIMER_TEST_STATE_IDLE;
    timerTest->scheduler = hScheduler;

    hApp = ofc_app_create(hScheduler, &TimerTestAppDef, timerTest);

    if (hDone != OFC_HANDLE_NULL) {
        ofc_app_set_wait(hApp, hDone);
        ofc_event_wait(hDone);
    }
}

TEST_GROUP_RUNNER(timer) {
    RUN_TEST_CASE(timer, test_timer);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(timer);
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
