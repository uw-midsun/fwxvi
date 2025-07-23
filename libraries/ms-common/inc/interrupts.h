#pragma once

/************************************************************************************************
 * @file   interrupts.h
 *
 * @brief  Interrupts Library Header file
 *
 * @date   2024-10-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "FreeRTOSConfig.h"

/* Intra-component Headers */
#include "gpio.h"
#include "status.h"

/**
 * @defgroup  Interrupts
 * @brief     Interrupts library
 * @{
 */

/**
 * @brief   Interrupt type selection
 */
typedef enum {
  INTERRUPT_TYPE_INTERRUPT = 0, /**< Interrupt type runs a callback as soon as the interrupt is triggered */
  INTERRUPT_TYPE_EVENT,         /**< Event type wakes the device without running a callback */
  NUM_INTERRUPT_TYPES,          /**< Number of supported interrupt classes */
} InterruptType;

/**
 * @brief   Interrupt priority level selection
 * @details Start counting at configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY,
 *          as ISR safe FreeRTOS API can only be called from interrupts that have
 *          a priority <= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY. Since priorities
 *          decrease with greater value (0 is highest priority, but is reserved), interrupt
 *          priorities must be greater than configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY.
 */
typedef enum {
  INTERRUPT_PRIORITY_HIGH = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY,
  INTERRUPT_PRIORITY_NORMAL,
  INTERRUPT_PRIORITY_LOW,
  NUM_INTERRUPT_PRIORITIES,
} InterruptPriority;

/**
 * @brief   Defines on what edge of an input signal the interrupt triggers on
 * @details Not necessarily applicable for all interrupts, however external interrupts
 *          will use them
 */
typedef enum {
  INTERRUPT_EDGE_RISING = 0, /**< Rising edge interrupt triggers when a signal goes from LOW to HIGH */
  INTERRUPT_EDGE_FALLING,    /**< Falling edge interrupt triggers when a signal goes from HIGH to LOW */
  INTERRUPT_EDGE_TRANSITION, /**< Transition interrupt triggers when a signal goes from LOW to HIGH or from HIGH to LOW */
  NUM_INTERRUPT_EDGES,       /**< Number of supported interrupt edges */
} InterruptEdge;

/**
 * @brief   Interrupt configuration structure
 */
typedef struct InterruptSettings {
  InterruptType type;         /**< Interrupt type selection */
  InterruptPriority priority; /**< Interrupt priority selection */
  InterruptEdge edge;         /**< Interrupt edge selection */
} InterruptSettings;

/** @brief   Indexed number of STM32L433 NVIC entires from data sheet and IRQn_Type enum */
#define NUM_STM32L433X_INTERRUPT_CHANNELS 83U

/** @brief   Number of external interrupt lines */
#define NUM_STM32L433X_EXTI_LINES 16U

/**
 * @brief   Initializes the interrupt internals
 * @details If called multiple times the subsequent attempts will clear everything
 *          resulting in needing to re-initialize all interrupts
 */
void interrupt_init(void);

/**
 * @brief   Enables the nested interrupt vector controller for a given channel
 * @param   irq_channel Numeric ID of the interrupt channel from the NVIC
 * @param   priority Priority level of the interrupt
 * @return  STATUS_CODE_OK if the channel is successfully initialized
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode interrupt_nvic_enable(uint8_t irq_channel, InterruptPriority priority);

/**
 * @brief   Enables an external interrupt line with the given settings
 * @param   address Pointer to the GPIO address
 * @param   settings Pointer to the interrupt settings
 * @return  STATUS_CODE_OK if the channel is successfully initialized
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode interrupt_exti_enable(GpioAddress *address, const InterruptSettings *settings);

/**
 * @brief   Triggers a software interrupt on a given external interrupt channel
 * @param   line Numeric ID of the EXTI line (GPIO Pin number)
 * @return  STATUS_CODE_OK if the channel is successfully initialized
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode interrupt_exti_trigger(uint8_t line);

/**
 * @brief   Get the pending flag for an external interrupt
 * @param   line Numeric ID of the EXTI line (GPIO Pin number)
 * @param   pending_bit Pointer to a variable that is updated with the pending bit
 * @return  STATUS_CODE_OK if the channel is successfully initialized
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode interrupt_exti_get_pending(uint8_t line, uint8_t *pending_bit);

/**
 * @brief   Clears the pending flag for an external interrupt
 * @param   line Numeric ID of the EXTI line (GPIO Pin number)
 * @return  STATUS_CODE_OK if the channel is successfully initialized
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode interrupt_exti_clear_pending(uint8_t line);

/**
 * @brief   Masks or clears the external interrupt on a given line
 * @param   line Numeric ID of the EXTI line (GPIO Pin number)
 * @param   masked 0: Clears the interrupt 1: Mask the lines interrupt
 * @return  STATUS_CODE_OK if the channel is successfully initialized
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode interrupt_exti_set_mask(uint8_t line, bool masked);

#ifdef MS_PLATFORM_X86

/** @brief  Function definition for interrupt handlers */
typedef void (*x86InterruptHandler)(uint8_t interrupt_id);

/**
 * @brief   Register a new NVIC interrupt and its handler
 * @details If the handler is left null, it will still be registered with a no-op callback
 * @param   irq_channel Numeric ID of the interrupt channel from the NVIC
 * @param   handler Function pointer to the interrupt handler. Can be left as NULL
 * @param   settings Pointer to the interrupt settings
 * @return  STATUS_CODE_OK if the interrupt handler is registered successfully
 *          STATUS_CODE_INVALID_ARGS if  one of the parameters are incorrect
 */
StatusCode interrupt_nvic_register_handler(uint8_t irq_channel, x86InterruptHandler handler, const InterruptSettings *settings);

/**
 * @brief   Register a new EXTI interrupt and its handler
 * @details If the handler is left null, it will still be registered with a no-op callback
 * @param   line Numeric ID of the EXTI line (GPIO Pin number)
 * @param   handler Function pointer to the interrupt handler. Can be left as NULL
 * @param   settings Pointer to the interrupt settings
 * @return  STATUS_CODE_OK if the interrupt handler is registered successfully
 *          STATUS_CODE_INVALID_ARGS if  one of the parameters are incorrect
 */
StatusCode interrupt_exti_register_handler(uint8_t line, x86InterruptHandler handler, const InterruptSettings *settings);

#endif

/** @} */
