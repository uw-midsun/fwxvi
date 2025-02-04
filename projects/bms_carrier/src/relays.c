/************************************************************************************************
 * @file   relays.h
 *
 * @brief  Header file for Relays
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "log.h"
#include "delay.h"
#include "timers.h"
#include "master_tasks.h"
#include "global_enums.h"

/* Intra-component Headers */
#include "relays.h"
#include "fault_bps.h"
#include "bms_hw_defs.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"

static struct RelayStorage relay_storage = {
  .pos_relay_en = BMS_POS_RELAY_ENABLE_GPIO,
  .pos_relay_sense = BMS_POS_RELAY_SENSE_GPIO,

  .neg_relay_en = BMS_NEG_RELAY_ENABLE_GPIO,
  .neg_relay_sense = BMS_NEG_RELAY_SENSE_GPIO,

  .solar_relay_en = BMS_SOLAR_RELAY_ENABLE_GPIO,
  .solar_relay_sense = BMS_SOLAR_RELAY_SENSE_GPIO,

  .killswitch_sense = BMS_KILLSWITCH_SENSE_GPIO
};

static BmsStorage *bms_storage;

static const GpioAddress *s_relays_sense[NUM_BMS_RELAYS] = {
  &relay_storage.pos_relay_sense,
  &relay_storage.neg_relay_sense,
  &relay_storage.solar_relay_sense
};

static StatusCode s_close_relays(void) {
  /* 250 MS Gap between each relay closing due to the excessive current draw */
  gpio_set_state(&bms_storage->relay_storage->pos_relay_en, GPIO_STATE_HIGH);
  delay_ms(BMS_CLOSE_RELAYS_DELAY_MS);

  gpio_set_state(&bms_storage->relay_storage->neg_relay_en, GPIO_STATE_HIGH);
  delay_ms(BMS_CLOSE_RELAYS_DELAY_MS);

  gpio_set_state(&bms_storage->relay_storage->solar_relay_en, GPIO_STATE_HIGH);
  delay_ms(BMS_CLOSE_RELAYS_DELAY_MS);

  for (uint8_t i = 0; i < NUM_BMS_RELAYS; i++) {
    if (gpio_get_state(s_relays_sense[i]) != GPIO_STATE_HIGH) {
      LOG_DEBUG("Relay %d not closed\n", i);
      fault_bps_set(BMS_RELAY_STATE_FAULT);
      bms_relay_fault();
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }
  return STATUS_CODE_OK;
}

inline void bms_open_solar() {
  gpio_set_state(&bms_storage->relay_storage->solar_relay_en, GPIO_STATE_LOW);
}

inline void bms_close_solar() {
  gpio_set_state(&bms_storage->relay_storage->solar_relay_en, GPIO_STATE_HIGH);
  delay_ms(BMS_CLOSE_RELAYS_DELAY_MS);
}

void bms_relay_fault() {
  LOG_DEBUG("Transitioned to RELAYS_FAULT\n");

  gpio_set_state(&bms_storage->relay_storage->pos_relay_en, GPIO_STATE_LOW);
  gpio_set_state(&bms_storage->relay_storage->neg_relay_en, GPIO_STATE_LOW);
  gpio_set_state(&bms_storage->relay_storage->solar_relay_en, GPIO_STATE_LOW);

  set_battery_relay_info_state(BMS_RELAY_STATE_FAULT);
}

StatusCode relays_init(BmsStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  bms_storage = storage;
  bms_storage->relay_storage = &relay_storage;

  /* Set up killswitch interrupt */
  interrupt_init();
  InterruptSettings it_settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .type = INTERRUPT_TYPE_INTERRUPT,
    .edge = INTERRUPT_EDGE_FALLING,
  };

  gpio_init_pin(&bms_storage->relay_storage->killswitch_sense, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  gpio_register_interrupt(&bms_storage->relay_storage->killswitch_sense, &it_settings, KILLSWITCH_IT, get_1000hz_task());
  
  /* Debounce startup killswitch state */
  delay_ms(10U);

  if (gpio_get_state(&bms_storage->relay_storage->killswitch_sense) == GPIO_STATE_LOW) {
    LOG_DEBUG("KILLSWITCH SET");
    fault_bps_set(BMS_FAULT_KILLSWITCH);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  gpio_init_pin(&bms_storage->relay_storage->pos_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&bms_storage->relay_storage->neg_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&bms_storage->relay_storage->solar_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  gpio_init_pin(&bms_storage->relay_storage->pos_relay_sense, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  gpio_init_pin(&bms_storage->relay_storage->neg_relay_sense, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  gpio_init_pin(&bms_storage->relay_storage->solar_relay_sense, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);

  status_ok_or_return(s_close_relays());
  set_battery_relay_info_state(BMS_RELAY_STATE_CLOSE);
  return STATUS_CODE_OK;
}