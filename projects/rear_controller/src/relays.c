/************************************************************************************************
 * @file   relays.h
 *
 * @brief  Source file for Relays
 *
 * @date   2025-09-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"

/* Intra-component Headers */
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"
#include "relays.h"

/**
 * For testing purposes, to see pin behavior without plugging in relays, set this to 0U
 * Otherwise, keep this set to 1U. Note that LED behavior, especially ws22 may be inconsistent
 * if nothing is plugged in
 */
#define RELAYS_RESPECT_CURRENT_SENSE 0U

typedef struct {
  GpioAddress pos_relay_en;    /**< Positive relay enable */
  GpioAddress pos_relay_sense; /**< Positive relay sense */

  GpioAddress neg_relay_en;    /**< Negative relay enable */
  GpioAddress neg_relay_sense; /**< Negative relay sense */

  GpioAddress solar_relay_en;    /**< Solar relay enable */
  GpioAddress solar_relay_sense; /**< Solar relay sense */

  GpioAddress motor_relay_en;    /**< Motor relay enable */
  GpioAddress motor_relay_sense; /**< Motor relay sense */

  GpioAddress ws22_lv_en; /**< Wavesculptor 22 low-voltage enable */
} RelayStorage;

static RelayStorage s_relay_storage = {
  .pos_relay_en = GPIO_REAR_CONTROLLER_POS_RELAY_ENABLE,   /**< Positive relay enable */
  .pos_relay_sense = GPIO_REAR_CONTROLLER_POS_RELAY_SENSE, /**< Positive relay sense */

  .neg_relay_en = GPIO_REAR_CONTROLLER_NEG_RELAY_ENABLE,   /**< Negative relay enable */
  .neg_relay_sense = GPIO_REAR_CONTROLLER_NEG_RELAY_SENSE, /**< Negative relay sense */

  .solar_relay_en = GPIO_REAR_CONTROLLER_SOLAR_RELAY_ENABLE,   /**< Solar relay enable */
  .solar_relay_sense = GPIO_REAR_CONTROLLER_SOLAR_RELAY_SENSE, /**< Solar relay sense */

  .motor_relay_en = GPIO_REAR_CONTROLLER_MOTOR_RELAY_ENABLE,   /**< Motor relay enable */
  .motor_relay_sense = GPIO_REAR_CONTROLLER_MOTOR_RELAY_SENSE, /**< Motor relay sense */

  .ws22_lv_en = GPIO_REAR_CONTROLLER_MOTOR_LV_ENABLE, /**< Wavesculptor 22 low-voltage enable */
};

static RearControllerStorage *rear_controller_storage = NULL;

StatusCode relays_init(RearControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;

  gpio_init_pin(&s_relay_storage.pos_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&s_relay_storage.neg_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&s_relay_storage.solar_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&s_relay_storage.motor_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&s_relay_storage.ws22_lv_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  gpio_init_pin(&s_relay_storage.pos_relay_sense, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&s_relay_storage.neg_relay_sense, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&s_relay_storage.solar_relay_sense, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&s_relay_storage.motor_relay_sense, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);

  rear_controller_storage->pos_relay_closed = false;
  rear_controller_storage->neg_relay_closed = false;
  rear_controller_storage->solar_relay_closed = false;
  rear_controller_storage->motor_relay_closed = false;

  return STATUS_CODE_OK;
}

StatusCode relays_reset(void) {
  gpio_set_state(&s_relay_storage.pos_relay_en, GPIO_STATE_LOW);
  gpio_set_state(&s_relay_storage.neg_relay_en, GPIO_STATE_LOW);
  gpio_set_state(&s_relay_storage.solar_relay_en, GPIO_STATE_LOW);
  gpio_set_state(&s_relay_storage.motor_relay_en, GPIO_STATE_LOW);

  rear_controller_storage->pos_relay_closed = false;
  rear_controller_storage->neg_relay_closed = false;
  rear_controller_storage->solar_relay_closed = false;
  rear_controller_storage->motor_relay_closed = false;
  return STATUS_CODE_OK;
}

StatusCode relays_enable_ws22_lv(void) {
  return gpio_set_state(&s_relay_storage.ws22_lv_en, GPIO_STATE_HIGH);
}

StatusCode relays_disable_ws22_lv(void) {
  return gpio_set_state(&s_relay_storage.ws22_lv_en, GPIO_STATE_LOW);
}

StatusCode relays_close_motor(void) {
  gpio_set_state(&s_relay_storage.motor_relay_en, GPIO_STATE_HIGH);

#if RELAYS_RESPECT_CURRENT_SENSE != 0
  if (gpio_get_state(&s_relay_storage.motor_relay_sense) != GPIO_STATE_HIGH) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
#endif

  rear_controller_storage->motor_relay_closed = true;

  return STATUS_CODE_OK;
}

StatusCode relays_open_motor(void) {
  gpio_set_state(&s_relay_storage.motor_relay_en, GPIO_STATE_LOW);

#if RELAYS_RESPECT_CURRENT_SENSE != 0
  if (gpio_get_state(&s_relay_storage.motor_relay_sense) != GPIO_STATE_LOW) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
#endif

  rear_controller_storage->motor_relay_closed = false;

  return STATUS_CODE_OK;
}

StatusCode relays_close_solar(void) {
  gpio_set_state(&s_relay_storage.solar_relay_en, GPIO_STATE_HIGH);
  delay_ms(REAR_CLOSE_RELAYS_DELAY_MS);

#if RELAYS_RESPECT_CURRENT_SENSE != 0
  if (gpio_get_state(&s_relay_storage.solar_relay_sense) != GPIO_STATE_HIGH) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
#endif

  rear_controller_storage->solar_relay_closed = true;

  return STATUS_CODE_OK;
}

StatusCode relays_open_solar(void) {
  gpio_set_state(&s_relay_storage.solar_relay_en, GPIO_STATE_LOW);

#if RELAYS_RESPECT_CURRENT_SENSE != 0
  if (gpio_get_state(&s_relay_storage.solar_relay_sense) != GPIO_STATE_LOW) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
#endif

  rear_controller_storage->solar_relay_closed = false;

  return STATUS_CODE_OK;
}

StatusCode relays_close_pos(void) {
  gpio_set_state(&s_relay_storage.pos_relay_en, GPIO_STATE_HIGH);
  delay_ms(REAR_CLOSE_RELAYS_DELAY_MS);

#if RELAYS_RESPECT_CURRENT_SENSE != 0
  if (gpio_get_state(&s_relay_storage.pos_relay_sense) != GPIO_STATE_HIGH) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
#endif

  rear_controller_storage->pos_relay_closed = true;

  return STATUS_CODE_OK;
}

StatusCode relays_open_pos(void) {
  gpio_set_state(&s_relay_storage.pos_relay_en, GPIO_STATE_LOW);

#if RELAYS_RESPECT_CURRENT_SENSE != 0
  if (gpio_get_state(&s_relay_storage.pos_relay_sense) != GPIO_STATE_LOW) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
#endif

  rear_controller_storage->pos_relay_closed = false;

  return STATUS_CODE_OK;
}

StatusCode relays_close_neg(void) {
  gpio_set_state(&s_relay_storage.neg_relay_en, GPIO_STATE_HIGH);
  delay_ms(REAR_CLOSE_RELAYS_DELAY_MS);

#if RELAYS_RESPECT_CURRENT_SENSE != 0
  if (gpio_get_state(&s_relay_storage.neg_relay_sense) != GPIO_STATE_HIGH) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
#endif

  rear_controller_storage->neg_relay_closed = true;

  return STATUS_CODE_OK;
}

StatusCode relays_open_neg(void) {
  gpio_set_state(&s_relay_storage.neg_relay_en, GPIO_STATE_LOW);

#if RELAYS_RESPECT_CURRENT_SENSE != 0
  if (gpio_get_state(&s_relay_storage.neg_relay_sense) != GPIO_STATE_LOW) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
#endif

  rear_controller_storage->neg_relay_closed = false;

  return STATUS_CODE_OK;
}
