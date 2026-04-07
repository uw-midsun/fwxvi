#pragma once

/************************************************************************************************
 * @file   master_tasks.h
 *
 * @brief  Header file for Master Tasks API. Supports 1KHz, 100Hz and 3HZ scheduling
 *
 * @date   2024-11-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "log.h"
#include "tasks.h"

/* Intra-component Headers */

/**
 * @defgroup Master_Tasks
 * @brief    Master Tasks Library
 * @{
 */

#ifndef MASTER_TASK_50HZ_SIZE
/** @brief  Number of bytes statically allocated for 50hz task */
#define MASTER_TASK_50HZ_SIZE (TASK_STACK_512)
#endif

#ifndef MASTER_TASK_3HZ_SIZE
/** @brief  Number of bytes statically allocated for 3hz task */
#define MASTER_TASK_3HZ_SIZE (TASK_STACK_512)
#endif

#ifndef MASTER_TASK_1HZ_SIZE
/** @brief  Number of bytes statically allocated for 1hz task */
#define MASTER_TASK_1HZ_SIZE (TASK_STACK_256)
#endif

#define MASTER_50HZ_TO_MS 20U
#define MASTER_3HZ_TO_MS  300U
#define MASTER_1HZ_TO_MS  1000U

/**
 * @brief   Run the 50hz cycle
 * @details If this cycle takes longer than 1ms it will throw a warning
 *          If this cycle takes longer than 1ms 5 times in a row, the scheduler will stop
 */
void run_1000hz_cycle();

/**
 * @brief   Run the 3hz cycle
 * @details If this cycle takes longer than 100ms it will throw a warning
 *          If this cycle takes longer than 1100ms 5 times in a row, the scheduler will stop
 */
void run_10hz_cycle();

/**
 * @brief   Run the 1hz cycle
 * @details If this cycle takes longer than 1s it will throw a warning
 *          If this cycle takes longer than 1s 5 times in a row, the scheduler will stop
 */
void run_1hz_cycle();

/**
 * @brief   Initialize the master tasks before the scheduler begins
 */
void pre_loop_init();

/**
 * @brief   Initialize all 3 master tasks
 * @details The three tasks run at 50hz (1ms), 3hz (100ms) and 1hz (1s)
 * @return  STATUS_CODE_OK if initialization succeeded for all 3 tasks
 *          STATUS_CODE_INVALID_ARGS if initialization failed for atleast 1 of the 3 tasks
 */
StatusCode init_master_tasks();

/**
 * @brief   Fetch the memory address of the 50hz task
 * @return  Pointer to the 50hz task
 */
Task *get_1000hz_task();

/**
 * @brief   Fetch the memory address of the 3hz task
 * @return  Pointer to the 3hz task
 */
Task *get_10hz_task();

/**
 * @brief   Fetch the memory address of the 1hz task
 * @return  Pointer to the 1hz task
 */
Task *get_1hz_task();

/** @} */
