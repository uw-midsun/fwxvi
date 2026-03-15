/************************************************************************************************
 * @file    lv_profiler_builtin_posix.c
 *
 * @brief   Lv Profiler Builtin Posix
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
    #include <pthread.h>
    #include <sys/syscall.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <windows.h>
#include <stdio.h>
#include <time.h>

/* Inter-component Headers */
#include "lv_profiler_builtin_private.h"

/* Intra-component Headers */

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_PROFILER && LV_USE_PROFILER_BUILTIN && LV_USE_PROFILER_BUILTIN_POSIX

#if defined(_WIN32)
#else
#endif

#if defined(__linux__)
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static uint64_t tick_get_cb(void);
static void flush_cb(const char * buf);
static int tid_get_cb(void);
static int cpu_get_cb(void);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_profiler_builtin_posix_init(void)
{
    lv_profiler_builtin_config_t config;
    lv_profiler_builtin_config_init(&config);

    /* One second is equal to 1000000000 nanoseconds */
    config.tick_per_sec = 1000000000;
    config.tick_get_cb = tick_get_cb;
    config.flush_cb = flush_cb;
    config.tid_get_cb = tid_get_cb;
    config.cpu_get_cb = cpu_get_cb;
    lv_profiler_builtin_init(&config);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static uint64_t tick_get_cb(void)
{
#if defined(_WIN32)
    static LARGE_INTEGER frequency = {0};
    LARGE_INTEGER counter;

    if(frequency.QuadPart == 0) {
        if(!QueryPerformanceFrequency(&frequency)) {
            fprintf(stderr, "QueryPerformanceFrequency failed\n");
            return 0;
        }
    }

    if(!QueryPerformanceCounter(&counter)) {
        fprintf(stderr, "QueryPerformanceCounter failed\n");
        return 0;
    }

    /* Convert counter to nanoseconds */
    return counter.QuadPart * 1000000000ULL / frequency.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
#endif
}

static void flush_cb(const char * buf)
{
    printf("%s", buf);
}

static int tid_get_cb(void)
{
#if defined(__linux__)
    return (int)syscall(SYS_gettid);
#elif defined(_WIN32)
    return (int)GetCurrentThreadId();
#else
    return (int)(lv_intptr_t)pthread_self();
#endif
}

static int cpu_get_cb(void)
{
#if defined(__linux__)
    unsigned cpu;
    int result = syscall(SYS_getcpu, &cpu, NULL, NULL);
    if(result < 0) {
        fprintf(stderr, "getcpu failed\n");
        return -1;
    }
    return (int)cpu;
#else
    return 0;
#endif
}

#endif
