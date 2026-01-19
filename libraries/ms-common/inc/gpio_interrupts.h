#pragma once

/************************************************************************************************
 * @file   gpio_interrupts.h
 *
 * @brief  GPIO Interrupts Library Header file
 *
 * @date   2024-11-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "gpio.h"
#include "interrupts.h"
#include "notify.h"
#include "status.h"
#include "tasks.h"

/**
 * @defgroup GPIO
 * @brief    GPIO library
 * @{
 */

/**
 * @brief   Storage class for GPIO interrupts
 */
typedef struct GpioInterrupt {
  InterruptSettings settings; /**< Interrupt settings storage */
  GpioAddress address;        /**< GPIO address storage */
  Event event;                /**< Event storage */
  Task *task;                 /**< Task storage */
} GpioInterrupt;

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
StatusCode gpio_register_interrupt(const GpioAddress *address, const InterruptSettings *settings, const Event event, const Task *task);

/**
 * @brief   Gets the GPIO interrupt edge
 * @param   address Pointer to the GPIO address
 * @return  The corresponding GPIO's interrupt edge
 *          NUM_INTERRUPT_EDGES if the interrupt has not been registered
 */
InterruptEdge gpio_it_get_edge(const GpioAddress *address);

/**
 * @brief   Gets the GPIO interrupt priority
 * @param   address Pointer to the GPIO address
 * @return  The corresponding GPIO's interrupt priority
 *          NUM_INTERRUPT_PRIORITIES if the interrupt has not been registered
 */
InterruptPriority gpio_it_get_priority(const GpioAddress *address);

/**
 * @brief   Gets the GPIO interrupt type
 * @param   address Pointer to the GPIO address
 * @return  The corresponding GPIO's interrupt type
 *          NUM_INTERRUPT_TYPES if the interrupt has not been registered
 */
InterruptType gpio_it_get_type(const GpioAddress *address);

/**
 * @brief   Gets the GPIO interrupt task
 * @param   address Pointer to the GPIO address
 * @return  The corresponding GPIO's interrupt task
 *          NULL if the interrupt has not been registered
 */
Task *gpio_it_get_target_task(const GpioAddress *address);

/**
 * @brief   Masks the GPIO interrupt.
 * @param   address Pointer to the GPIO address
 * @param   masked 0: Enables the GPIO interrupt 1: Mask the GPIO interrupt
 * @return  STATUS_CODE_OK if interrupt masking succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode gpio_it_mask_interrupt(const GpioAddress *address, bool masked);

/**
 * @brief   Software generated GPIO interrupt
 * @param   address Pointer to the GPIO address
 * @return  STATUS_CODE_OK if interrupt triggering succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode gpio_trigger_interrupt(const GpioAddress *address);

/** @} */
