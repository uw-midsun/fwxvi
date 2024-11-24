#pragma once

/************************************************************************************************
 * @file   gpio_interrupts.h
 *
 * @brief  GPIO Interrupts Library Header file
 *
 * @date   2024-11-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "gpio.h"
#include "interrupts.h"
#include "notify.h"
#include "status.h"
#include "tasks.h"

/**
 * @brief   Register a GPIO interrupt by passing in the pin address and task to notify
 * @param   address Pointer to the GPIO address
 * @param   settings Pointer to the interrupt settings
 * @param   event Event notification
 * @param   task Task to notify when interrupt fires
 * @return  STATUS_CODE_OK if interrupt initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_RESOURCE_EXHAUSTED if the pin is already being used
 */
StatusCode gpio_register_interrupt(const GpioAddress *address, const InterruptSettings *settings,
                                   const Event event, const Task *task);

/**
 * @brief   Software generated GPIO interrupt
 * @param   address Pointer to the GPIO address
 */
StatusCode gpio_trigger_interrupt(const GpioAddress *address);
