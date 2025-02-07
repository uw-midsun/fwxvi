/************************************************************************************************
 * @file   precharge.c
 *
 * @brief  Source code for Precharge
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "log.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_interrupts.h"

/* Intra-component Headers */
#include "precharge.h"
#include "fault_bps.h"
#include "bms_hw_defs.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"

struct PrechargeSettings precharge_settings = {
  .motor_sw_en = BMS_PRECHARGE_RELAY_ENABLE_GPIO,
  .motor_sw_sense = BMS_PRECHARGE_RELAY_SENSE_GPIO,
  .precharge_monitor = BMS_PRECHARGE_STATUS_GPIO
};

static BmsStorage *bms_storage;

StatusCode precharge_init(BmsStorage *storage, Event event, const Task *task) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  bms_storage = storage;
  bms_storage->precharge_settings = &precharge_settings;

  /* Initialize motor relay sense and enable */
  gpio_init_pin(&bms_storage->precharge_settings->motor_sw_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&bms_storage->precharge_settings->motor_sw_sense, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);

  /* Initialize precharge monitor */
  gpio_init_pin(&bms_storage->precharge_settings->precharge_monitor, GPIO_INPUT_FLOATING, GPIO_STATE_HIGH);
  
  /* Debounce */
  delay_ms(10U);

  GpioState state = gpio_get_state(&bms_storage->precharge_settings->precharge_monitor);

  if (state == GPIO_STATE_HIGH) {
    set_mc_status_precharge_status(true);
    gpio_set_state(&bms_storage->precharge_settings->motor_sw_en, GPIO_STATE_HIGH);
    return STATUS_CODE_OK;
  } else if (state == GPIO_STATE_LOW) {
    InterruptSettings monitor_it_settings = {
      .type = INTERRUPT_TYPE_INTERRUPT,
      .priority = INTERRUPT_PRIORITY_NORMAL,
      .edge = INTERRUPT_EDGE_RISING,
    };

    set_mc_status_precharge_status(false);
    return gpio_register_interrupt(&bms_storage->precharge_settings->precharge_monitor, &monitor_it_settings, event, task);
  } else {
    LOG_WARN("Unexpected error\n");
    return STATUS_CODE_INVALID_ARGS;
  }
}
