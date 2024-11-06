/************************************************************************************************
 * interrupts.c
 *
 * Interrupts Library Source file
 *
 * Created: 2024-11-03
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "stm32l4xx_hal_cortex.h"
#include "stm32l4xx_hal_exti.h"

/* Intra-component Headers */
#include "interrupts.h"

void interrupt_init(void) {}

StatusCode interrupt_nvic_enable(uint8_t irq_channel, InterruptPriority priority) {
  if (priority >= NUM_INTERRUPT_PRIORITIES || irq_channel >= NUM_STM32L433X_INTERRUPT_CHANNELS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  HAL_NVIC_SetPriority(irq_channel, priority, 0U);
  HAL_NVIC_EnableIRQ(irq_channel);
  return STATUS_CODE_OK;
}

StatusCode interrupt_exti_enable(GpioAddress *address, const InterruptSettings *settings) {
  if (settings == NULL || address == NULL || settings->type >= NUM_INTERRUPT_CLASSES ||
      settings->edge >= NUM_INTERRUPT_EDGES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}

StatusCode interrupt_exti_trigger(uint8_t line) {
  if (line > NUM_STM32L433X_EXTI_LINES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}

StatusCode interrupt_exti_get_pending(uint8_t line, uint8_t *pending_bit) {
  if (line >= NUM_STM32L433X_INTERRUPT_CHANNELS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}

StatusCode interrupt_exti_clear_pending(uint8_t line) {
  if (line >= NUM_STM32L433X_INTERRUPT_CHANNELS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}

StatusCode interrupt_exti_mask_set(uint8_t line, bool masked) {
  if (line >= NUM_STM32L433X_INTERRUPT_CHANNELS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}
