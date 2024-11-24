#pragma once

/************************************************************************************************
 * @file   interrupts.h
 *
 * @brief  Interrupts Library Header file
 *
 * @date   2024-10-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "gpio.h"
#include "status.h"

/**
 * @defgroup  Interrupts
 * @brief     Interrupt library
 * @{
 */

/**
 * @brief   Interrupt type runs a callback as soon as the interrupt is triggered
 *          Event type wakes the device without running a callback
 */
typedef enum {
  INTERRUPT_TYPE_INTERRUPT = 0,
  INTERRUPT_TYPE_EVENT,
  NUM_INTERRUPT_CLASSES,
} InterruptClass;

// Start counting at 1, as needs to be <= configMAX_SYSCALL_INTERRUPT_PRIORITY
// Priorities decrease with greater value (0 is highest priority, but is reserved)
typedef enum {
  INTERRUPT_PRIORITY_HIGH = 1,
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
  INTERRUPT_EDGE_RISING = 0,
  INTERRUPT_EDGE_FALLING,
  INTERRUPT_EDGE_RISING_FALLING,
  NUM_INTERRUPT_EDGES,
} InterruptEdge;

/**
 * @brief   Interrupt configuration structure
 */
typedef struct InterruptSettings {
  InterruptClass type;
  InterruptPriority priority;
  InterruptEdge edge;
} InterruptSettings;

/**
 * @brief   Indexed number of STM32L433 NVIC entires from data sheet and IRQn_Type enum
 */
#define NUM_STM32L433X_INTERRUPT_CHANNELS 83

/**
 * @brief   Number of external interrupt lines
 */
#define NUM_STM32L433X_EXTI_LINES 16

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
 * @return  STATUS_CODE_OK if the channel is succesfully initialized
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode interrupt_nvic_enable(uint8_t irq_channel, InterruptPriority priority);

/**
 * @brief   Enables an external interrupt line with the given settings
 * @param   address Pointer to the GPIO address
 * @param   settings Pointer to the interrupt settings
 * @return  STATUS_CODE_OK if the channel is succesfully initialized
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode interrupt_exti_enable(GpioAddress *address, const InterruptSettings *settings);

/**
 * @brief   Triggers a software interrupt on a given external interrupt channel
 * @param   line Numeric ID of the EXTI line (GPIO Pin number)
 * @return  STATUS_CODE_OK if the channel is succesfully initialized
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode interrupt_exti_trigger(uint8_t line);

/**
 * @brief   Get the pending flag for an external interrupt
 * @param   line Numeric ID of the EXTI line (GPIO Pin number)
 * @param   pending_bit Pointer to a variable that is updated with the pending bit
 * @return  STATUS_CODE_OK if the channel is succesfully initialized
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode interrupt_exti_get_pending(uint8_t line, uint8_t *pending_bit);

/**
 * @brief   Clears the pending flag for an external interrupt
 * @param   line Numeric ID of the EXTI line (GPIO Pin number)
 * @return  STATUS_CODE_OK if the channel is succesfully initialized
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode interrupt_exti_clear_pending(uint8_t line);

/**
 * @brief   Masks or clears the external interrupt on a given line
 * @param   line Numeric ID of the EXTI line (GPIO Pin number)
 * @param   masked 0: Clears the interrupt 1: Mask the lines interrupt
 * @return  STATUS_CODE_OK if the channel is succesfully initialized
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode interrupt_exti_mask_set(uint8_t line, bool masked);

/** @} */
