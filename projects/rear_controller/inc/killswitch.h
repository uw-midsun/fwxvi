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
 * @defgroup Rear Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

StatusCode killswitch_init(Event event, const Task *task);

/** @} */
