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

static const GpioAddress *s_relays_sense[]={
    &REAR_CONTROLLER_POS_RELAY_SENSE_GPIO,
    &REAR_CONTROLLER_NEG_RELAY_SENSE_GPIO,
    &REAR_CONTROLLER_SOLAR_RELAY_SENSE_GPIO
};
StatusCode relay_init(RearControllerStorage *storage){
    if (storage ==NULL) return STATUS_CODE_INVALID_ARGS;

    s_rear_controller_storage=storage;

    gpio_init_pin(&REAR_CONTROLLER_POS_RELAY_ENABLE_GPIO, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
    gpio_init_pin(&REAR_CONTROLLER_NEG_RELAY_ENABLE_GPIO, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
    gpio_init_pin(&REAR_CONTROLLER_SOLAR_RELAY_ENABLE_GPIO, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
    gpio_init_pin(&REAR_CONTROLLER_MOTOR_RELAY_ENABLE_GPIO, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

   gpio_init_pin(&REAR_CONTROLLER_POS_RELAY_SENSE_GPIO, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
    gpio_init_pin(&REAR_CONTROLLER_NEG_RELAY_SENSE_GPIO, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
    gpio_init_pin(&REAR_CONTROLLER_SOLAR_RELAY_SENSE_GPIO, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
        gpio_init_pin(&REAR_CONTROLLER_MOTOR_RELAY_SENSE_GPIO, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);

    //Close relays and verify they are closed
    gpio_set_state(&REAR_CONTROLLER_POS_RELAY_ENABLE_GPIO, GPIO_STATE_HIGH);
    gpio_set_state(&REAR_CONTROLLER_POS_RELAY_ENABLE_GPIO, GPIO_STATE_HIGH);
    gpio_set_state(&REAR_CONTROLLER_POS_RELAY_ENABLE_GPIO, GPIO_STATE_HIGH);

     if (gpio_get_state(&REAR_CONTROLLER_POS_RELAY_SENSE_GPIO) != GPIO_STATE_HIGH ||
        gpio_get_state(&REAR_CONTROLLER_NEG_RELAY_SENSE_GPIO) != GPIO_STATE_HIGH ||
        gpio_get_state(&REAR_CONTROLLER_SOLAR_RELAY_SENSE_GPIO) != GPIO_STATE_HIGH) {
        LOG_DEBUG("One or more relays failed to close\n");
        return STATUS_CODE_INTERNAL_ERROR;
    }
      s_rear_controller_storage->pos_relay_closed = true;
    s_rear_controller_storage->neg_relay_closed = true;
    s_rear_controller_storage->solar_relay_closed = true;

    LOG_DEBUG("Relays initialized and closed\n");
    return STATUS_CODE_OK;

}
StatusCode relay_fault(){ 
    gpio_set_state(&REAR_CONTROLLER_POS_RELAY_ENABLE_GPIO, GPIO_STATE_LOW);
    gpio_set_state(&REAR_CONTROLLER_NEG_RELAY_ENABLE_GPIO, GPIO_STATE_LOW);
    gpio_set_state(&REAR_CONTROLLER_SOLAR_RELAY_ENABLE_GPIO, GPIO_STATE_LOW);
    gpio_set_state(&REAR_CONTROLLER_MOTOR_RELAY_ENABLE_GPIO, GPIO_STATE_LOW);


    s_rear_controller_storage->pos_relay_closed = false;
    s_rear_controller_storage->neg_relay_closed = false;
    s_rear_controller_storage->solar_relay_closed = false;
    s_rear_controller_storage->motor_relay_closed = false;

    LOG_DEVUG("All relays opened!\n");

    return STATUS_CODE_OK;
} 
StatusCode relay_solar_close(){
    gpio_set_state(&REAR_CONTROLLER_SOLAR_RELAY_ENABLE_GPIO, GPIO_STATE_HIGH);
    
    if (gpio_get_state(&REAR_CONTROLLER_SOLAR_RELAY_SENSE_GPIO) != GPIO_STATE_HIGH){
        LOG_DEBUG("Solar relay failed to close\n");
        return STATUS_CODE_INTERNAL_ERROR;
    }
    s_rear_controller_storage->solar_relay_closed=false;

    LOG_DEBUG("Solar relay closed successfully\n");
    return STATUS_CODE_OK;

}; 
StatusCode relay_solar_open(){
    gpio_set_state(&REAR_CONTROLLER_SOLAR_RELAY_ENABLE_GPIO, GPIO_STATE_LOW);
    if (gpio_get_state(&REAR_CONTROLLER_SOLAR_RELAY_SENSE_GPIO) != GPIO_STATE_LOW){
        LOG_DEBUG("Solar relay failed to open\n");
        return STATUS_CODE_INTERNAL_ERROR;
    }
    s_rear_controller_storage->solar_relay_closed = false;
    LOG_DEBUG("Solar relay opened successfully\n");
    return STATUS_CODE_OK;
}
StatusCode relay_motor_close(){
        gpio_set_state(&REAR_CONTROLLER_MOTOR_RELAY_ENABLE_GPIO, GPIO_STATE_HIGH);
        if (gpio_get_state(&REAR_CONTROLLER_MOTOR_RELAY_SENSE_GPIO) != GPIO_STATE_HIGH) {
            LOG_DEBUG("Failed to close motor relay\n");
            return STATUS_CODE_INTERNAL_ERROR;
        }
         s_rear_controller_storage->motor_relay_closed = true;

    LOG_DEBUG("Motor relay closed successfully\n");
    return STATUS_CODE_OK;

}
StatusCode relay_motor_open(){
        gpio_set_state(&REAR_CONTROLLER_MOTOR_RELAY_ENABLE_GPIO, GPIO_STATE_LOW);
        if (gpio_get_state(&REAR_CONTROLLER_MOTOR_RELAY_SENSE_GPIO) != GPIO_STATE_LOW) {
            LOG_DEBUG("Failed to open motor relay\n");
            return STATUS_CODE_INTERNAL_ERROR;
        }
        s_rear_controller_storage->motor_relay_closed = false;

    LOG_DEBUG("Motor relay opened successfully\n");
    return STATUS_CODE_OK;

}