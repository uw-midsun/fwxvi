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
#include "notify.h"
#include "tasks.h"

/* Intra-component Headers */

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

#define REAR_CONTROLLER_PRECHARGE_EVENT 0U

/**
 * @brief   Initializes the precharge pin with interrupt handling
 * @param   event  Event to raise when precharge is complete
 * @param   task   Task to notify when event is raised
 * @return  StatusCode indicating success or failure of initialization
 */
StatusCode precharge_init(Event event, const Task *task);

/**
 * @brief   Run the program that checks precharge completion
 * @return  StatusCode indicating success or failure
 */
StatusCode precharge_run();

/** @} */
