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
#include "rear_controller_hw_defs.h"
#include "rear_controller_setters.h"

/* Intra-component Headers */
#include "status.h"

static GpioAddress killswitch_address = GPIO_REAR_CONTROLLER_KILLSWITCH_MONITOR;

static uint32_t notification;

static InterruptSettings killswitch_settings = {
  INTERRUPT_TYPE_INTERRUPT,
  INTERRUPT_PRIORITY_NORMAL,
  INTERRUPT_EDGE_RISING,
};

StatusCode killswitch_init(Event event, const Task *task) {
  set_rear_controller_status_killswitch_state(false);

  gpio_it_init(&killswitch_address, &killswitch_settings, GPIO_INPUT_PULL_UP, GPIO_STATE_HIGH);

  /* 10ms Debounce */
  delay_ms(10U);
  GpioState state = gpio_get_state(&killswitch_address);

  if (state == GPIO_STATE_LOW) {
    gpio_register_interrupt(&killswitch_address, &killswitch_settings, event, task);
  } else {
    LOG_DEBUG("KILLSWITCH PRESSED\r\n");
    set_rear_controller_status_killswitch_state(true);
  }

  return STATUS_CODE_OK;
}

StatusCode killswitch_run() {
  notify_get(&notification);
  if (notification & (1 << REAR_CONTROLLER_KILLSWITCH_EVENT)) {
    LOG_DEBUG("KILLSWITCH PRESSED\r\n");
    set_rear_controller_status_killswitch_state(true);
  }
  return STATUS_CODE_OK;
}
