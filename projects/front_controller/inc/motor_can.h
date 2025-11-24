#pragma once

#include "ws22_motor_can.h"
#include "front_controller.h"
#include "status.h"

#define VEL_TO_RPM_RATIO 0.57147  // from MSXV

StatusCode motor_can_update_target_current_velocity();

StatusCode motor_can_init(FrontControllerStorage *storage);