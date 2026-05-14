#pragma once

/************************************************************************************************
 * @file    precharge.h
 *
 * @brief   Motor precharge handler header
 *
 * @date    2025-09-02
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio_interrupts.h"
#include "master_tasks.h"
#include "notify.h"
#include "tasks.h"

/* Intra-component Headers */
#include "rear_controller.h"

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

#define REAR_CONTROLLER_PRECHARGE_EVENT 0U

#define REAR_CONTROLLER_PRECHARGE_CYCLE_DELAY_MS MASTER_1000HZ_TO_MS
#define REAR_CONTROLLER_PRECHARGE_TIMEOUT_COUNTER 5000 / REAR_CONTROLLER_PRECHARGE_CYCLE_DELAY_MS

/**
 * @brief   Initializes the precharge pin with interrupt handling
 * @param   event  Event to raise when precharge is complete
 * @param   task   Task to notify when event is raised
 * @return  StatusCode indicating success or failure of initialization
 */
StatusCode precharge_init(Event event, const Task *task, RearControllerStorage *storage);

/**
 * @brief   Run the program that checks precharge completion
 * @return  StatusCode indicating success or failure
 */
StatusCode precharge_run();

/** @} */
