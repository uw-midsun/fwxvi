/************************************************************************************************
 * @file   gpio_interrupts.c
 *
 * @brief  GPIO Interrupts Library Source file
 *
 * @date   2024-11-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "log.h"

/* Intra-component Headers */
#include "gpio_interrupts.h"

static GpioInterrupt s_gpio_it_interrupts[GPIO_PINS_PER_PORT] = { 0U };

StatusCode gpio_register_interrupt(const GpioAddress *address, const InterruptSettings *settings, const Event event, const Task *task) {
  return STATUS_CODE_UNIMPLEMENTED;
}

InterruptEdge gpio_it_get_edge(const GpioAddress *address) {
  if (s_gpio_it_interrupts[address->pin].task != NULL) {
    return s_gpio_it_interrupts[address->pin].settings.edge;
  }
  return NUM_INTERRUPT_EDGES;
}

InterruptPriority gpio_it_get_priority(const GpioAddress *address) {
  if (s_gpio_it_interrupts[address->pin].task != NULL) {
    return s_gpio_it_interrupts[address->pin].settings.priority;
  }
  return NUM_INTERRUPT_PRIORITIES;
}

InterruptType gpio_it_get_type(const GpioAddress *address) {
  if (s_gpio_it_interrupts[address->pin].task != NULL) {
    return s_gpio_it_interrupts[address->pin].settings.type;
  }
  return NUM_INTERRUPT_TYPES;
}

Task *gpio_it_get_target_task(const GpioAddress *address) {
  if (s_gpio_it_interrupts[address->pin].task != NULL) {
    return s_gpio_it_interrupts[address->pin].task;
  }
  return NULL;
}

StatusCode gpio_it_mask_interrupt(const GpioAddress *address, bool masked) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode gpio_trigger_interrupt(const GpioAddress *address) {
  return STATUS_CODE_UNIMPLEMENTED;
}
