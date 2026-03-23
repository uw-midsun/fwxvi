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
#include "delay.h"
#include "gpio.h"
#include "interrupts.h"
#include "status.h"

/* Intra-component Headers */
#include "precharge.h"
#include "rear_controller_hw_defs.h"
#include "rear_controller_setters.h"

#define PRECHARGE_MODE_OPTO 0U
#define PRECHARGE_MODE_MANUAL 1U

#define PRECHARGE_MODE PRECHARGE_MODE_OPTO

#if (PRECHARGE_MODE == PRECHARGE_MODE_MANUAL)
#define PRECHARGE_THRESHOLD_VOLTS 1U
#include "rear_controller_getters.h"
#endif

static RearControllerStorage *rear_controller_storage = NULL;

static GpioAddress precharge_address = GPIO_REAR_CONTROLLER_PRECHARGE_MONITOR_GPIO;

static uint32_t notification;

static InterruptSettings precharge_settings = {
  INTERRUPT_TYPE_INTERRUPT,
  INTERRUPT_PRIORITY_NORMAL,
  INTERRUPT_EDGE_FALLING,
};

StatusCode precharge_init(Event event, const Task *task, RearControllerStorage *storage) {
#if (PRECHARGE_MODE == PRECHARGE_MODE_OPTO)
  gpio_init_pin(&precharge_address, GPIO_INPUT_PULL_UP, GPIO_STATE_LOW);

  /* 10ms Debounce */
  delay_ms(10U);
  GpioState state = gpio_get_state(&precharge_address);

  if (state == GPIO_STATE_HIGH) {
    gpio_register_interrupt(&precharge_address, &precharge_settings, event, task);
  } else {
    set_rear_controller_status_triggers_motor_precharge_complete(true);
  }
#endif

#if (PRECHARGE_MODE == PRECHARGE_MODE_MANUAL)
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  rear_controller_storage = storage;

  if (abs(get_motor_stats_A_bus_voltage() - rear_controller_storage->pack_voltage) < PRECHARGE_THRESHOLD_VOLTS) {
    set_rear_controller_status_triggers_motor_precharge_complete(true);
  } else {
    set_rear_controller_status_triggers_motor_precharge_complete(false);
  }
#endif

  return STATUS_CODE_OK;
}

StatusCode precharge_run() {
#if (PRECHARGE_MODE == PRECHARGE_MODE_OPTO)
  notify_get(&notification);
  if (notification & (1 << REAR_CONTROLLER_PRECHARGE_EVENT)) {
    LOG_DEBUG("PRECHARGE COMPLETE\r\n");
    set_rear_controller_status_triggers_motor_precharge_complete(true);
  }
#endif

#if (PRECHARGE_MODE == PRECHARGE_MODE_MANUAL)
  if (abs(get_motor_stats_A_bus_voltage() - rear_controller_storage->pack_voltage) < PRECHARGE_THRESHOLD_VOLTS) {
    set_rear_controller_status_triggers_motor_precharge_complete(true);
  } else {
    set_rear_controller_status_triggers_motor_precharge_complete(false);
  }
#endif

  return STATUS_CODE_OK;
}
