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

StatusCode precharge_init(Event event, const Task *task);

/** @} */
