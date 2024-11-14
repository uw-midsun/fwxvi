#pragma once

/************************************************************************************************
 * master_tasks.h
 *
 * Header file for Master Tasks API. Supports 1KHz, 100Hz and 10Hz scheduling
 *
 * Created: 2024-11-04
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "log.h"
#include "tasks.h"

/* Intra-component Headers */

#ifndef MASTER_TASK_100HZ_SIZE
#define MASTER_TASK_100HZ_SIZE (TASK_STACK_256)
#endif

#ifndef MASTER_TASK_10HZ_SIZE
#define MASTER_TASK_10HZ_SIZE (TASK_STACK_256)
#endif

#ifndef MASTER_TASK_1HZ_SIZE
#define MASTER_TASK_1HZ_SIZE (TASK_STACK_256)
#endif

void run_fast_cycle();
void run_medium_cycle();
void run_slow_cycle();
void pre_loop_init();

StatusCode init_master_tasks();
Task *get_100hz_task();
Task *get_10hz_task();
Task *get_1hz_task();
