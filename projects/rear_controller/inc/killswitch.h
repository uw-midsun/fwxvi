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
 * @brief   Initializes t
 * @param   event
 * @param   task
 * @return  StatusCode
 */
StatusCode killswitch_init(Event event, const Task *task);

/** @} */
