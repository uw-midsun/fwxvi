#pragma once


#include "status.h"
#include "gpio.h"
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"


#define NUM_REAR_RELAYS 3U

#define REAR_CLOSE_RELAYS_DELAY_MS 250U


typedef struct{

    GpioAddress pos_relay_en;    /**< Positive relay enable */
    GpioAddress pos_relay_sense; /**< Positive relay sense */

    GpioAddress neg_relay_en;    /**< Negative relay enable */
    GpioAddress neg_relay_sense; /**< Negative relay sense */

    GpioAddress solar_relay_en;    /**< Solar relay enable */
    GpioAddress solar_relay_sense; /**< Solar relay sense */

    GpioAddress motor_relay_en;    /**< Motor relay enable */
    GpioAddress motor_relay_sense; /**< Motor relay sense */
    GpioAddress killswitch_sense; /**< Killswitch sense */
}RelayStorage;


StatusCode relays_init(RearControllerStorage *storage);
StatusCode relay_fault();
StatusCode rear_close_motor();
StatusCode rear_open_motor();
StatusCode rear_close_solar();
StatusCode rear_open_solar();
bool relay_is_killswitch_active(void);
StatusCode relay_verify_states(void);