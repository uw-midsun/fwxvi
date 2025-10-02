/************************************************************************************************
 * @file    precharge.c
 *
 * @brief   Motor precharge handler source file
 *
 * @date    2025-09-02
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>
/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "interrupts.h"
#include "status.h"
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"
/* Intra-component Headers */
static RearControllerStorage *s_rear_controller_storage;



StatusCode relay_init(RearControllerStorage *storage){
    if (storage ==NULL) return STATUS_CODE_INVALID_ARGS;

    StatusCode rear_controller_init(RearControllerConfig *config) {
  if (config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_storage.config = config;
  
  // Initialize all status flags to default values
  s_storage.pos_relay_closed = false;
  s_storage.neg_relay_closed = false;
  s_storage.solar_relay_closed = false;
  s_storage.motor_relay_closed = false;
  s_storage.motor_lv_enabled = false;
  s_storage.precharge_complete = false;
  s_storage.killswitch_active = false;
  s_storage.pcs_valid = false;
  s_storage.aux_valid = false;
  
  // Initialize relays
  status_ok_or_return(relays_init(&s_storage));
  
  LOG_DEBUG("Rear controller initialized\n");
  return STATUS_CODE_OK;

}
StatusCode relay_fault(){ 
  LOG_DEBUG("Entering fault state\n");
  return relays_fault();
} 
StatusCode relay_solar_close(){
  LOG_DEBUG("Closing solar relay\n");
  return relay_close_solar();

}; 
StatusCode relay_solar_open(){
    LOG_DEBUG("Opening solar relay\n");
    return relay_open_solar();
}
StatusCode relay_motor_close(){
  LOG_DEBUG("Opening motor relay\n");
  return rear_open_motor();

}
StatusCode relay_motor_open(){
  LOG_DEBUG("Closing motor relay\n");
  return rear_close_motor();

}