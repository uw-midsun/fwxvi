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
#define PRECHARGE_MODE_COMPARE 1U
#define PRECHARGE_MODE_TIMEOUT 2U

#define PRECHARGE_MODE PRECHARGE_MODE_MANUAL

#define PRECHARGE_DEBUG 1U

#define PRECHARGE_VALID_CYCLES 50U

#if (PRECHARGE_MODE == PRECHARGE_MODE_MANUAL)

#define PRECHARGE_THRESHOLD_VOLTS 3U
#include "rear_controller_getters.h"
#endif

static RearControllerStorage *rear_controller_storage = NULL;

static GpioAddress precharge_address = GPIO_REAR_CONTROLLER_PRECHARGE_MONITOR_GPIO;

static GpioAddress s_rear_controller_board_led = GPIO_REAR_CONTROLLER_BOARD_LED;

static uint32_t notification;

static uint8_t valid_cycles = 0;

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

#if (PRECHARGE_MODE == PRECHARGE_MODE_COMPARE)
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  rear_controller_storage = storage;
  set_rear_controller_status_triggers_motor_precharge_complete(false);
  valid_cycles = 0;
#endif

  return STATUS_CODE_OK;
}

StatusCode precharge_run() {
  // If precharge is already complete, we are done
  if (rear_controller_storage->precharge_complete) {
#if (PRECHARGE_DEBUG == 1)
    LOG_DEBUG("Precharge complete\r\n");
    LOG_DEBUG("MOT: %u | BAT: %lu DIFF: %lu PC complete: %u\r\n", get_motor_stats_A_bus_voltage(), rear_controller_storage->pack_voltage,
              (uint32_t)abs(get_motor_stats_A_bus_voltage() - rear_controller_storage->pack_voltage), rear_controller_storage->precharge_complete);
#endif
    return STATUS_CODE_OK;
  }

#if (PRECHARGE_MODE == PRECHARGE_MODE_OPTO)
  notify_get(&notification);
  if (notification & (1 << REAR_CONTROLLER_PRECHARGE_EVENT)) {
    LOG_DEBUG("PRECHARGE COMPLETE\r\n");
    rear_controller_storage->precharge_complete = true;
    gpio_set_state(&s_rear_controller_board_led, GPIO_STATE_HIGH);
    set_rear_controller_status_triggers_motor_precharge_complete(true);
  }
#endif

#if (PRECHARGE_MODE == PRECHARGE_MODE_MANUAL)
#if (PRECHARGE_DEBUG == 1)
  // Report values
  LOG_DEBUG("MOT: %u | BAT: %lu DIFF: %lu PC complete: %u\r\n", get_motor_stats_A_bus_voltage(), rear_controller_storage->pack_voltage,
            (uint32_t)abs(get_motor_stats_A_bus_voltage() - rear_controller_storage->pack_voltage), rear_controller_storage->precharge_complete);
#endif

  // If either voltage reading is zero, we should exit immediately
  if (get_motor_stats_A_bus_voltage() == 0 || rear_controller_storage->pack_voltage == 0) {
    valid_cycles = 0;
    set_rear_controller_status_triggers_motor_precharge_complete(false);
    return STATUS_CODE_OK;
  }

  // Check difference in voltage
  if ((uint32_t)abs(get_motor_stats_A_bus_voltage() - rear_controller_storage->pack_voltage) < PRECHARGE_THRESHOLD_VOLTS) {
    valid_cycles++;

    // We should be within precharge threshold for a given amount of cycles
    if (valid_cycles >= PRECHARGE_VALID_CYCLES) {
      set_rear_controller_status_triggers_motor_precharge_complete(true);
      rear_controller_storage->precharge_complete = true;
      gpio_set_state(&s_rear_controller_board_led, GPIO_STATE_HIGH);
    }
  } else {
    valid_cycles = 0;
    set_rear_controller_status_triggers_motor_precharge_complete(false);
  }
#endif

#if (PRECHARGE_MODE == PRECHARGE_MODE_TIMEOUT)
  valid_cycles++;
  if (valid_cycles >= REAR_CONTROLLER_PRECHARGE_TIMEOUT_COUNTER) {
    LOG_DEBUG("Precharge timeout: %u\r\n", valid_cycles);
    set_rear_controller_status_triggers_motor_precharge_complete(true);
    rear_controller_storage->precharge_complete = true;
    gpio_set_state(&s_rear_controller_board_led, GPIO_STATE_HIGH);
  }
#endif

  return STATUS_CODE_OK;
}
