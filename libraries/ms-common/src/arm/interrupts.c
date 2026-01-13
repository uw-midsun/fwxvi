/************************************************************************************************
 * @file   interrupts.c
 *
 * @brief  Interrupts Library Source file
 *
 * @date   2024-11-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "stm32l4xx.h"
#include "stm32l4xx_hal_cortex.h"
#include "stm32l4xx_hal_exti.h"

/* Intra-component Headers */
#include "interrupts.h"
#include "status.h"
#include "stm32l4xx_hal_gpio.h"

static EXTI_HandleTypeDef s_exti_handles[GPIO_PINS_PER_PORT];

StatusCode gpio_it_init(const GpioAddress *address, InterruptSettings *settings) {
  if (address == NULL || settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  // taskENTER_CRITICAL();
  GPIO_InitTypeDef init = { 0 };
  init.Pin = 1U << (address->pin);

  switch (settings->edge) {
    case INTERRUPT_EDGE_RISING:
      init.Mode = GPIO_MODE_IT_RISING;
      break;
    case INTERRUPT_EDGE_FALLING:
      init.Mode = GPIO_MODE_IT_FALLING;
      break;
    case INTERRUPT_EDGE_TRANSITION:
      init.Mode = GPIO_MODE_IT_RISING_FALLING;
      break;
    default:
      return STATUS_CODE_INVALID_ARGS;
  }

  //TODO Pass in, hardcoded for now
  init.Pull = GPIO_PULLUP;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)(AHB2PERIPH_BASE + (address->port * GPIO_ADDRESS_OFFSET));

  /* Initialize the GPIO pin for interrupts */
  HAL_GPIO_Init(gpio_port, &init);

  /* Initialize the EXTI line to handle interrupts */
  interrupt_exti_enable(address, settings);

  // taskEXIT_CRITICAL();

  return STATUS_CODE_OK;
}

void interrupt_init(void) {
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  for (int i = 0; i < 16; i++) {
    memset(&s_exti_handles[i], 0, sizeof(EXTI_HandleTypeDef));
  }
}

StatusCode interrupt_nvic_enable(uint8_t irq_channel, InterruptPriority priority) {
  if ((priority >= NUM_INTERRUPT_PRIORITIES && priority < configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY) || irq_channel >= NUM_STM32L433X_INTERRUPT_CHANNELS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  HAL_NVIC_SetPriority(irq_channel, priority, 0U);
  HAL_NVIC_EnableIRQ(irq_channel);
  return STATUS_CODE_OK;
}

StatusCode interrupt_exti_enable(GpioAddress *address, const InterruptSettings *settings) {
  if (settings == NULL || address == NULL || settings->type >= NUM_INTERRUPT_TYPES || settings->edge >= NUM_INTERRUPT_EDGES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  EXTI_ConfigTypeDef init = { 0U };

  /* The line will correspond to the pin. This means A2 and B2
   * will have the same interrupt line
   */
  init.Line = EXTI_GPIO | EXTI_REG1 | EXTI_EVENT | (address->pin);
  init.Mode = EXTI_MODE_INTERRUPT;

  switch (settings->edge) {
    case INTERRUPT_EDGE_RISING:
      init.Trigger = EXTI_TRIGGER_RISING;
      break;
    case INTERRUPT_EDGE_FALLING:
      init.Trigger = EXTI_TRIGGER_FALLING;
      break;
    case INTERRUPT_EDGE_TRANSITION:
      init.Trigger = EXTI_TRIGGER_RISING_FALLING;
      break;
    default:
      return STATUS_CODE_INVALID_ARGS;
  }

  init.GPIOSel = address->port;

  if (HAL_EXTI_SetConfigLine(&s_exti_handles[address->pin], &init) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  // /* Unmask the interrupt */
  // status_ok_or_return(interrupt_exti_set_mask(address->pin, false));

  return STATUS_CODE_OK;
}

StatusCode interrupt_exti_trigger(uint8_t line) {
  if (line > NUM_STM32L433X_EXTI_LINES) {
    return STATUS_CODE_INVALID_ARGS;
  }
  HAL_EXTI_GenerateSWI(&s_exti_handles[line]);
  return STATUS_CODE_OK;
}

StatusCode interrupt_exti_get_pending(uint8_t line, uint8_t *pending_bit) {
  if (line >= NUM_STM32L433X_INTERRUPT_CHANNELS || pending_bit == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  /* Returns 1 if set, 0 if not */
  *pending_bit = (uint8_t)HAL_EXTI_GetPending(&s_exti_handles[line], EXTI_TRIGGER_RISING_FALLING);
  return STATUS_CODE_OK;
}

StatusCode interrupt_exti_clear_pending(uint8_t line) {
  if (line >= NUM_STM32L433X_INTERRUPT_CHANNELS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  HAL_EXTI_ClearPending(&s_exti_handles[line], EXTI_TRIGGER_RISING_FALLING);
  return STATUS_CODE_OK;
}

StatusCode interrupt_exti_set_mask(uint8_t line, bool masked) {
  if (line >= NUM_STM32L433X_INTERRUPT_CHANNELS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (masked) {
    EXTI->IMR1 &= ~(1U << line);
  } else {
    EXTI->IMR1 |= (1U << line);
  }

  return STATUS_CODE_OK;
}
