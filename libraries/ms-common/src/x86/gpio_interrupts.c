/************************************************************************************************
 * @file   gpio_interrupts.c
 *
 * @brief  GPIO Interrupts Library Source file
 *
 * @date   2024-11-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "gpio_interrupts.h"

StatusCode gpio_register_interrupt(const GpioAddress *address, const InterruptSettings *settings,
                                   const Event event, const Task *task) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode gpio_trigger_interrupt(const GpioAddress *address) {
  return STATUS_CODE_UNIMPLEMENTED;
}
