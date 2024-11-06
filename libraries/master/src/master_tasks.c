/************************************************************************************************
 * master_tasks.c
 *
 * Source file for Master Tasks API. Supports 1KHz, 100Hz and 10Hz scheduling
 *
 * Created: 2024-11-04
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "master_tasks.h"

static uint32_t MASTER_MS_CYCLE_TIME = 50;

#define MASTER_TASK_PRIORITY (configMAX_PRIORITIES - 2)
#define MAX_CYCLES_OVER 5

static uint8_t s_cycles_over = 0;
