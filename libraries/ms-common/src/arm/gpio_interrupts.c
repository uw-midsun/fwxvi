/************************************************************************************************
 * @file   gpio_interrupts.c
 *
 * @brief  GPIO Interrupts Library Source Code
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
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT || event >= INVALID_EVENT) {
    return STATUS_CODE_INVALID_ARGS;
  } else if (s_gpio_it_interrupts[address->pin].task != NULL) {
    LOG_DEBUG("GPIO INTERRUPT INIT FAILED. Pin already being used");
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  // Register exti channel and enable interrupt
  status_ok_or_return(interrupt_exti_enable(address, settings));

  s_gpio_it_interrupts[address->pin].address = *address;
  s_gpio_it_interrupts[address->pin].settings = *settings;
  s_gpio_it_interrupts[address->pin].event = event;
  s_gpio_it_interrupts[address->pin].task = task;

  return STATUS_CODE_OK;
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

InterruptType gpio_it_get_class(const GpioAddress *address) {
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
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    return STATUS_CODE_INVALID_ARGS;
  }
  return interrupt_exti_set_mask(address->pin, masked);
}

StatusCode gpio_trigger_interrupt(const GpioAddress *address) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return interrupt_exti_trigger(address->pin);
}

static void s_gpio_callbacks(uint8_t lower_bound, uint8_t upper_bound) {
  uint8_t pending = 0;
  for (int i = lower_bound; i <= upper_bound; i++) {
    interrupt_exti_get_pending(i, &pending);
    if (pending && s_gpio_it_interrupts[i].task != NULL) {
      notify_from_isr(s_gpio_it_interrupts[i].task, s_gpio_it_interrupts[i].event);
    }
    interrupt_exti_clear_pending(i);
  }
}

/**
 * @brief   Interrupt handlers for EXTI 1-15
 */
void EXTI0_IRQHandler(void) {
  s_gpio_callbacks(0U, 0U);
}

void EXTI1_IRQHandler(void) {
  s_gpio_callbacks(1U, 1U);
}

void EXTI2_IRQHandler(void) {
  s_gpio_callbacks(2U, 2U);
}

void EXTI3_IRQHandler(void) {
  s_gpio_callbacks(3U, 3U);
}

void EXTI4_IRQHandler(void) {
  s_gpio_callbacks(4U, 4U);
}

void EXTI9_5_IRQHandler(void) {
  s_gpio_callbacks(5U, 9U);
}

void EXTI15_10_IRQHandler(void) {
  s_gpio_callbacks(10U, 15U);
}
