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

void set_master_cycle_time(uint32_t time_ms);
void set_medium_cycle_count(uint32_t cycles);
void set_slow_cycle_count(uint32_t cycles);

void run_fast_cycle();
void run_medium_cycle();
void run_slow_cycle();
void pre_loop_init();

StatusCode init_master_task();
Task *get_master_task();
