#pragma once

/************************************************************************************************
 * @file    killswitch.h
 *
 * @brief   Vehicle killswitch handler header file
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

/**
 * @brief   Initializes the killswitch with interrupt handling
 * @param   event  Event to raise when killswitch is triggered
 * @param   task   Task to notify when event is raised
 * @return  StatusCode indicating success or failure of initialization
 */
StatusCode killswitch_init(Event event, const Task *task);

/** @} */
