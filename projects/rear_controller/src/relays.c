#pragma once

/************************************************************************************************
 * @file   relays.h
 *
 * @brief  Header file for Relays
 *
 * @date   2025-09-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>
/* Inter-component Headers */
#include "rear_controller_hw_defs.h"
#include "gpio.h"
#include "gpio_interrupts.h"
#include "interrupts.h"
#include "status.h"
#include "delay.h"
#include "master_tasks.h"
#include "log.h"
#include "timers.h"

/* Intra-component Headers */
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"
#include "relays.h"

/**
 * @defgroup bms_carrier
 * @brief    bms_carrier Firmware
 * @{
 */

/** @brief  Delay between consecutive relays being closed */
#define BMS_CLOSE_RELAYS_DELAY_MS 250U
/** @brief  Number of BMS relays */
#define NUM_BMS_RELAYS 3U
/** @brief  Killswitch interrupt notification */
#define KILLSWITCH_EVENT_IT 3U


static RelayStorage relay_storage{
    .pos_relay_en=REAR_CONTROLLER_NEG_RELAY_ENABLE_GPIO,    /**< Positive relay enable */
    .pos_relay_sense=REAR_CONTROLLER_POS_RELAY_SENSE_GPIO, /**< Positive relay sense */
    
    .neg_relay_en=REAR_CONTROLLER_NEG_RELAY_ENABLE_GPIO,    /**< Negative relay enable */
    .neg_relay_sense=REAR_CONTROLLER_NEG_RELAY_SENSE_GPIO, /**< Negative relay sense */

    .solar_relay_en=REAR_CONTROLLER_SOLAR_RELAY_ENABLE_GPIO,    /**< Solar relay enable */
    .solar_relay_sense=REAR_CONTROLLER_SOLAR_RELAY_SENSE_GPIO, /**< Solar relay sense */

    .motor_relay_en=REAR_CONTROLLER_MOTOR_RELAY_ENABLE_GPIO,    /**< Motor relay enable */
    .motor_relay_sense=REAR_CONTROLLER_MOTOR_RELAY_SENSE_GPIO, /**< Motor relay sense */

    .killswitch_sense= REAR_CONTROLLER_KILLSWITCH_MONITOR_GPIO/**< Killswitch sense */

};

//Array of relay pins
static const GpioAddress *s_relays_sense[NUM_BMS_RELAYS] = {
  &relay_storage.pos_relay_sense,
  &relay_storage.neg_relay_sense,
  &relay_storage.solar_relay_sense,
  &relay_storage.motor_relay_sense
};
//Array of enable pins
static const GpioAddress *s_relays_enable[NUM_REAR_CONTROLLER_RELAYS] = {
  &relay_storage.pos_relay_en, 
  &relay_storage.neg_relay_en, 
  &relay_storage.solar_relay_en,
  &relay_storage.motor_relay_en
};
static RearControllerStorage *s_rear_controller_storage;

void rear_precharge_init(){
    precharge_init();
}

interrupt_init();
/*
 * @brief Close all main power relays (Positive, Negative, and Solar)
 */

StatusCode relays_init(RearControllerStorage *storage){
  if (s_rear_controller_storage != NULL) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  gpio_init_pin(&relay_storage.pos_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&relay_storage.neg_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&relay_storage.solar_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&relay_storage.motor_relay_en, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  gpio_init_pin(&relay_storage.pos_relay_sense, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&relay_storage.neg_relay_sense, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&relay_storage.solar_relay_sense, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&relay_storage.motor_relay_sense, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);

  gpio_init_pin(&relay_storage.killswitch_sense, GPIO_INPUT_PULL_UP, GPIO_STATE_LOW);

  //Killswitch interrupt init

  interrupt_init();

  InterruptSettings killswitch_settings = {
        .type = INTERRUPT_TYPE_INTERRUPT,
        .edge = INTERRUPT_EDGE_FALLING,
        .priority = INTERRUPT_PRIORITY_NORMAL,
};

gpio_register_interrupt(&relay_storage.killswitch_sense, &killswitch_settings, KILLSWITCH_EVENT_IT, get_1000hz_task());

if (gpio_get_state(&relay_storage.killswitch_sense) == GPIO_STATE_LOW) {
    LOG_DEBUG("Killswitch is active on startup\n");
    return STATUS_CODE_INTERNAL_ERROR;
  }

   s_rear_controller_storage->pos_relay_closed = false;
  s_rear_controller_storage->neg_relay_closed = false;
  s_rear_controller_storage->solar_relay_closed = false;
  s_rear_controller_storage->motor_relay_closed = false;
  //status_ok_or_return(s_close_main_relays());
    return STATUS_CODE_OK;
}
StatusCode relays_fault(void){

    gpio_set_state(&REAR_CONTROLLER_POS_RELAY_ENABLE_GPIO, GPIO_STATE_LOW);
    gpio_set_state(&REAR_CONTROLLER_NEG_RELAY_ENABLE_GPIO, GPIO_STATE_LOW);
    gpio_set_state(&REAR_CONTROLLER_SOLAR_RELAY_ENABLE_GPIO, GPIO_STATE_LOW);
    gpio_set_state(&REAR_CONTROLLER_MOTOR_RELAY_ENABLE_GPIO, GPIO_STATE_LOW);

    s_rear_controller_storage->pos_relay_closed = false;
    s_rear_controller_storage->neg_relay_closed = false;
    s_rear_controller_storage->solar_relay_closed = false;
    s_rear_controller_storage->motor_relay_closed = false;
    LOG_DEBUG("All relays opened!\n");
    return STATUS_CODE_OK;
}

StatusCode rear_close_motor(void){
    gpio_set_state(&relay_storage.motor_relay_en, GPIO_STATE_HIGH);
    delay_ms(REAR_CLOSE_RELAYS_DELAY_MS);

    if (gpio_get_state(&REAR_CONTROLLER_MOTOR_RELAY_SENSE_GPIO) != GPIO_STATE_HIGH){
        LOG_DEBUG("Motor relay failed to close\n");
        return STATUS_CODE_INTERNAL_ERROR;
    }
    s_rear_controller_storage->motor_relay_closed=true;
    return STATUS_CODE_OK;
}

StatusCode rear_open_motor(void){
    gpio_set_state(&relay_storage.motor_relay_en, GPIO_STATE_LOW);
    
    if(gpio_get_state(&REAR_CONTROLLER_MOTOR_RELAY_SENSE_GPIO) != GPIO_STATE_LOW){
        LOG_DEBUG("Motor relay failed to open\n");
        return STATUS_CODE_INTERNAL_ERROR;
    }
    s_rear_controller_storage->motor_relay_closed=false;
    return STATUS_CODE_OK;
}

StatusCode relay_close_solar(void) {
  gpio_set_state(&relay_storage.solar_relay_en, GPIO_STATE_HIGH);
  delay_ms(REAR_CLOSE_RELAYS_DELAY_MS);
  
  if (gpio_get_state(&relay_storage.solar_relay_sense) != GPIO_STATE_HIGH) {
    LOG_DEBUG("Failed to close solar relay\n");
    return STATUS_CODE_INTERNAL_ERROR;
  }
  
  s_rear_controller_storage->solar_relay_closed = true;
  LOG_DEBUG("Solar relay closed successfully\n");
  
  return STATUS_CODE_OK;
}

StatusCode relay_open_solar(void) {
  gpio_set_state(&relay_storage.solar_relay_en, GPIO_STATE_LOW);
  
  if (gpio_get_state(&relay_storage.solar_relay_sense) != GPIO_STATE_LOW) {
    LOG_DEBUG("Failed to open solar relay\n");
    return STATUS_CODE_INTERNAL_ERROR;
  }
  
  s_rear_controller_storage->solar_relay_closed = false;
  LOG_DEBUG("Solar relay opened successfully\n");
  
  return STATUS_CODE_OK;
}



bool relay_is_killswitch_active(void) {
  return (gpio_get_state(&relay_storage.killswitch_sense) == GPIO_STATE_LOW);
}


StatusCode relay_verify_states(void) {
  // Check positive relay
  if ((s_rear_controller_storage->pos_relay_closed && 
       (gpio_get_state(&relay_storage.pos_relay_sense) != GPIO_STATE_HIGH)) || 
      (!s_rear_controller_storage->pos_relay_closed && 
       (gpio_get_state(&relay_storage.pos_relay_sense) != GPIO_STATE_LOW))) {
    LOG_DEBUG("Positive relay state mismatch\n");
    return STATUS_CODE_INTERNAL_ERROR;
  }
  
  // Check negative relay
  if ((s_rear_controller_storage->neg_relay_closed && 
       (gpio_get_state(&relay_storage.neg_relay_sense) != GPIO_STATE_HIGH)) || 
      (!s_rear_controller_storage->neg_relay_closed && 
       (gpio_get_state(&relay_storage.neg_relay_sense) != GPIO_STATE_LOW))) {
    LOG_DEBUG("Negative relay state mismatch\n");
    return STATUS_CODE_INTERNAL_ERROR;
  }
  
  // Check solar relay
  if ((s_rear_controller_storage->solar_relay_closed && 
       (gpio_get_state(&relay_storage.solar_relay_sense) != GPIO_STATE_HIGH)) || 
      (!s_rear_controller_storage->solar_relay_closed && 
       (gpio_get_state(&relay_storage.solar_relay_sense) != GPIO_STATE_LOW))) {
    LOG_DEBUG("Solar relay state mismatch\n");
    return STATUS_CODE_INTERNAL_ERROR;
  }
  
  // Check motor relay
  if ((s_rear_controller_storage->motor_relay_closed && 
       (gpio_get_state(&relay_storage.motor_relay_sense) != GPIO_STATE_HIGH)) || 
      (!s_rear_controller_storage->motor_relay_closed && 
       (gpio_get_state(&relay_storage.motor_relay_sense) != GPIO_STATE_LOW))) {
    LOG_DEBUG("Motor relay state mismatch\n");
    return STATUS_CODE_INTERNAL_ERROR;
  }
  
  return STATUS_CODE_OK;
}