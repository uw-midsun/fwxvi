/************************************************************************************************
 * @file    precharge.c
 *
 * @brief   Motor precharge handler source file
 *
 * @date    2025-09-02
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "precharge.h"

#include "delay.h"
#include "gpio.h"
#include "interrupts.h"
#include "rear_controller_hw_defs.h"

/* Intra-component Headers */
#include "status.h"

static GpioAddress precharge_address = REAR_CONTROLLER_PRECHARGE_GPIO;

static InterruptSettings precharge_settings = {
  INTERRUPT_TYPE_INTERRUPT,
  INTERRUPT_PRIORITY_NORMAL,
  INTERRUPT_EDGE_FALLING,
};

StatusCode precharge_init(Event event, const Task *task) {
  gpio_init_pin(&precharge_address, GPIO_INPUT_PULL_UP, GPIO_STATE_LOW);

  /* 10ms Debounce */
  delay_ms(10U);
  GpioState state = gpio_get_state(&precharge_address);

  if (state == GPIO_STATE_HIGH) {
    gpio_register_interrupt(&precharge_address, &precharge_settings, event, task);
  } else {
    /* TODO: Handle precharge complete */
  }

  return STATUS_CODE_OK;
}
