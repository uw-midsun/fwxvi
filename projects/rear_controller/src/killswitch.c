/************************************************************************************************
 * @file    killswitch.c
 *
 * @brief   Vehicle killswitch handler source file
 *
 * @date    2025-09-02
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "killswitch.h"

#include "delay.h"
#include "gpio.h"
#include "interrupts.h"
#include "rear_hw_defs.h"

/* Intra-component Headers */
#include "status.h"

static GpioAddress killswitch_address = REAR_CONTROLLER_KILLSWITCH_GPIO;

static InterruptSettings killswitch_settings = {
  INTERRUPT_TYPE_INTERRUPT,
  INTERRUPT_PRIORITY_NORMAL,
  INTERRUPT_EDGE_RISING,
};

StatusCode killswitch_init(Event event, const Task *task) {
  gpio_init_pin(&killswitch_address, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);

  /* 10ms Debounce */
  delay_ms(10U);
  GpioState state = gpio_get_state(&killswitch_address);

  if (state == GPIO_STATE_LOW) {
    gpio_register_interrupt(&killswitch_address, &killswitch_settings, event, task);
  } else {
    /* TODO: Trigger Killswitch pressed events */
  }

  return STATUS_CODE_OK;
}
