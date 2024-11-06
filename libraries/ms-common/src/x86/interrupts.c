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

/* Intra-component Headers */
#include "interrupts.h"

void interrupt_init(void) {}

StatusCode interrupt_nvic_enable(uint8_t irq_channel, InterruptPriority priority) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode interrupt_exti_enable(GpioAddress *address, const InterruptSettings *settings) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode interrupt_exti_trigger(uint8_t line) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode interrupt_exti_get_pending(uint8_t line, uint8_t *pending_bit) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode interrupt_exti_clear_pending(uint8_t line) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode interrupt_exti_mask_set(uint8_t line, bool masked) {
  return STATUS_CODE_UNIMPLEMENTED;
}
