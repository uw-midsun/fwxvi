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

static const GpioAddress pos_relay_en = BMS_POS_RELAY_ENABLE_GPIO;
static const GpioAddress pos_relay_sense = BMS_POS_RELAY_SENSE_GPIO;

static const GpioAddress neg_relay_en = BMS_NEG_RELAY_ENABLE_GPIO;
static const GpioAddress neg_relay_sense =  BMS_NEG_RELAY_SENSE_GPIO;

static const GpioAddress solar_relay_en = BMS_SOLAR_RELAY_ENABLE_GPIO;
static const GpioAddress solar_relay_sense = BMS_SOLAR_RELAY_SENSE_GPIO;

#define NUM_BMS_RELAYS 3

static const GpioAddress s_relays_sense[NUM_BMS_RELAYS] = { pos_relay_sense, neg_relay_sense,
                                                            solar_relay_sense };

static StatusCode prv_close_relays(void) {
  // 200 MS GAP BETWEEN EACH RELAY BC OF CURRENT DRAW
  gpio_set_state(&pos_relay_en, GPIO_STATE_HIGH);
  delay_ms(BMS_CLOSE_RELAYS_DELAY_MS);
  gpio_set_state(&neg_relay_en, GPIO_STATE_HIGH);
  delay_ms(BMS_CLOSE_RELAYS_DELAY_MS);
  gpio_set_state(&solar_relay_en, GPIO_STATE_HIGH);
  delay_ms(BMS_CLOSE_RELAYS_DELAY_MS);
  for (uint8_t i = 0; i < NUM_BMS_RELAYS; i++) {
    if (gpio_get_state(&s_relays_sense[i]) != GPIO_STATE_HIGH) {
      LOG_DEBUG("Relay %d not closed\n", i);
      // fault_bps_set(BMS_FAULT_RELAY_CLOSE_FAILED);
      // bms_relay_fault();
      // return STATUS_CODE_INTERNAL_ERROR;
    }
  }
  return STATUS_CODE_OK;
}

inline void bms_open_solar() {
  gpio_set_state(&solar_relay_en, GPIO_STATE_LOW);
}

inline void bms_close_solar() {
  gpio_set_state(&solar_relay_en, GPIO_STATE_HIGH);
  delay_ms(BMS_CLOSE_RELAYS_DELAY_MS);
}

void bms_relay_fault() {
  LOG_DEBUG("Transitioned to RELAYS_FAULT\n");

  gpio_set_state(&pos_relay_en, GPIO_STATE_LOW);
  gpio_set_state(&neg_relay_en, GPIO_STATE_LOW);
  gpio_set_state(&solar_relay_en, GPIO_STATE_LOW);
  // set_battery_relay_info_state(EE_RELAY_STATE_FAULT);
}

StatusCode init_bms_relays(GpioAddress *killswitch) {
  // Set up kill switch
  interrupt_init();
  InterruptSettings it_settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .type = INTERRUPT_TYPE_INTERRUPT,
    .edge = INTERRUPT_EDGE_FALLING,
  };

  gpio_init_pin(killswitch, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  gpio_register_interrupt(killswitch, &it_settings, KILLSWITCH_IT, get_1000hz_task());
  delay_ms(10);

  if (gpio_get_state(killswitch) == GPIO_STATE_LOW) {
    LOG_DEBUG("KILLSWITCH SET");
    delay_ms(5);
    fault_bps_set(BMS_FAULT_KILLSWITCH);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  gpio_init_pin(&pos_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&neg_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&solar_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  gpio_init_pin(&pos_relay_sense, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  gpio_init_pin(&neg_relay_sense, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  gpio_init_pin(&solar_relay_sense, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);

  status_ok_or_return(prv_close_relays());
  // set_battery_relay_info_state(EE_RELAY_STATE_CLOSE);
  return STATUS_CODE_OK;
}