/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for front_controller_motor_can
 *
 * @date   2026-01-18
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "adc.h"
#include "can.h"
#include "delay.h"
#include "front_controller.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "power_manager.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */
#include "accel_pedal.h"
#include "front_controller_getters.h"
#include "front_controller_state_manager.h"
#include "motor_can.h"
#include "opd.h"
#include "ws22_motor_can.h"

#define PRINT_CONTROL_DATA 1U
#define MOCK_PEDAL_VALUES 1U

static FrontControllerStorage front_controller_storage = { 0 };

static FrontControllerConfig front_controller_config = { .accel_input_deadzone = FRONT_CONTROLLER_ACCEL_INPUT_DEADZONE,
                                                         .accel_input_remap_min = FRONT_CONTROLLER_ACCEL_REMAP_MIN,
                                                         .accel_input_curve_exponent = FRONT_CONTROLLER_ACCEL_CURVE_EXPONENT,
                                                         .accel_low_pass_filter_alpha = FRONT_CONTROLLER_ACCEL_LPF_ALPHA };

static const float test_current_min = 0.0f;
static const float test_current_max = 1.0f;
static const float step_size = (test_current_max - test_current_min) / 5;

#if (MOCK_PEDAL_VALUES == 1)
static const char *print_state_str(VehicleDriveState state) {
  switch (state) {
    case VEHICLE_DRIVE_STATE_NEUTRAL:
      return "VEHICLE_DRIVE_STATE_NEUTRAL";
    case VEHICLE_DRIVE_STATE_DRIVE:
      return "VEHICLE_DRIVE_STATE_DRIVE";
    case VEHICLE_DRIVE_STATE_REVERSE:
      return "VEHICLE_DRIVE_STATE_REVERSE";
    case VEHICLE_DRIVE_STATE_CRUISE:
      return "VEHICLE_DRIVE_STATE_CRUISE";
    case VEHICLE_DRIVE_STATE_BRAKE:
      return "VEHICLE_DRIVE_STATE_BRAKE";
    case VEHICLE_DRIVE_STATE_REGEN:
      return "VEHICLE_DRIVE_STATE_REGEN";
    case VEHICLE_DRIVE_STATE_INVALID:
      return "VEHICLE_DRIVE_STATE_INVALID";
    default:
      return "UNKNOWN";
  }
}
#endif

TASK(main_1000hz_cycle, TASK_STACK_1024) {
  StatusCode status = STATUS_CODE_OK;
  delay_ms(500);

  while (true) {
    run_can_rx_all();
#if (MOCK_PEDAL_VALUES == 0)
    adc_run();
    accel_pedal_run();
    opd_run();
#endif
    motor_can_update_target_current_velocity();
    run_can_tx_fast();
    ws22_motor_can_transmit_drive_command();
    delay_ms(1U);
  }
}

TASK(main_10hz_cycle, TASK_STACK_1024) {
  delay_ms(2000U);
  LOG_DEBUG("Starting 10hz cycle\n\r");
  while (true) {
    run_can_tx_medium();
    front_controller_update_state_manager_medium_cycle();
    delay_ms(100U);
  }
}

TASK(main_1hz_cycle, TASK_STACK_1024) {
  delay_ms(2000U);
  LOG_DEBUG("Starting 1hz cycle\n\r");
  while (true) {
    run_can_tx_slow();
    delay_ms(1000U);
  }
}

#if (MOCK_PEDAL_VALUES == 1)
TASK(cycle_ws22, TASK_STACK_1024) {
  delay_ms(2000U);

  LOG_DEBUG("Starting to mock pedal values\n\r");

  VehicleDriveState current_state;

  while (true) {
    for (VehicleDriveState state = VEHICLE_DRIVE_STATE_NEUTRAL; state <= VEHICLE_DRIVE_STATE_REVERSE; state++) {
      front_controller_storage.brake_enabled = false;
      g_rx_struct.steering_buttons_drive_state = state;  // Mock drive state from steering

      for (float i = test_current_min; i <= test_current_max; i += step_size) {
        front_controller_storage.accel_percentage = i;
        motor_can_get_current_state(&current_state);
        LOG_DEBUG("TARGET STATE: %s | CURRENT STATE: %s\n\r", print_state_str(state), print_state_str(current_state));
        delay_ms(10U);
        LOG_DEBUG("MOCKING ACCEL PERCENTAGE TO: %ld\n\r", (long)(front_controller_storage.accel_percentage * 100));
        delay_ms(1000U);
      }
    }

    front_controller_storage.brake_enabled = true;
    motor_can_get_current_state(&current_state);
    LOG_DEBUG("CURRENT STATE: %s | MOCKING BRAKES\n\r", print_state_str(current_state));
    delay_ms(1000U);

    front_controller_storage.brake_enabled = false;
  }
}
#endif

#if (PRINT_CONTROL_DATA == 1)
TASK(display_ws22_control_data, TASK_STACK_1024) {
  delay_ms(2100U);
  LOG_DEBUG("Displaying control data\n\r");

  float current;
  uint32_t velocity;
  while (true) {
    current = front_controller_storage.ws22_motor_can_storage->control.current;
    velocity = front_controller_storage.ws22_motor_can_storage->control.velocity;

    LOG_DEBUG("CTRL: I=%ldA Vel=%lu\n\r", (long)(current * 1000), velocity);
    delay_ms(1000U);
  }
}
#endif

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(main_1000hz_cycle, TASK_PRIORITY(4), NULL);
#if (MOCK_PEDAL_VALUES == 1)
  tasks_init_task(cycle_ws22, TASK_PRIORITY(3), NULL);
#endif
  tasks_init_task(main_10hz_cycle, TASK_PRIORITY(3), NULL);
  tasks_init_task(main_1hz_cycle, TASK_PRIORITY(3), NULL);

#if (PRINT_CONTROL_DATA == 1)
  tasks_init_task(display_ws22_control_data, TASK_PRIORITY(2), NULL);
#endif

  StatusCode status = front_controller_init(&front_controller_storage, &front_controller_config);
  front_controller_storage.brake_enabled = false;

  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("front controller initialized\n\r");
  } else {
    LOG_DEBUG("front controller cannot be initialized\n\r");
  }

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
