#pragma once

/************************************************************************************************
 * @file    front_lights_signal.h
 *
 * @brief   Header file for front controller light signal receiver
 *
 * @date    2025-11-20
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

 #include <stdbool.h>
 #include "gpio.h"
 #include "software_timer.h"
 #include "front_controller_hw_defs.h"
 #include "status.h"
 #include "global_enums.h"

 #define FRONT_LIGHTs_BLINK_PERIOD_MS 600
 #define SYSTEM_CAN_MESSAGE_STEERING_BUTTONS 6

 /**
  * @brief Process recieved signal state 
  * @param state which is the state recieved by the CAN message
  */
 void front_lights_signal_process_event(SteeringLightState new_state);