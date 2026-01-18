/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for front_controller_ws22
 *
 * @date   2026-01-18
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "front_controller.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "power_manager.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */
#include "motor_can.h"
#include "ws22_motor_can.h"

#define PRINT_TX_STATS 1U
#define MOCK_PEDAL_VALUES 1U

static FrontControllerStorage front_controller_storage = { 0 };

static FrontControllerConfig front_controller_config = { .accel_input_deadzone = FRONT_CONTROLLER_ACCEL_INPUT_DEADZONE,
                                                         .accel_input_remap_min = FRONT_CONTROLLER_ACCEL_REMAP_MIN,
                                                         .accel_input_curve_exponent = FRONT_CONTROLLER_ACCEL_CURVE_EXPONENT,
                                                         .accel_low_pass_filter_alpha = FRONT_CONTROLLER_ACCEL_LPF_ALPHA };

static const float test_current_min = 0.0f;
static const float test_current_max = 0.1f;
static const float step_size = (test_current_max - test_current_min) / 5;

TASK(main_cycle, TASK_STACK_1024) {
  StatusCode status = STATUS_CODE_OK;

  // Step 1: Check if the front controller can be initialized
  status = front_controller_init(&front_controller_storage, &front_controller_config);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("front controller initialized\n");
  } else {
    LOG_DEBUG("front controller cannot be initialized\n");
  }
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

#if (MOCK_PEDAL_VALUES == 1)
TASK(cycle_ws22, TASK_STACK_1024) {
  delay_ms(2000U);

  LOG_DEBUG("Starting to mock pedal values\n");

  while (true) {
    for (float i = test_current_min; i <= test_current_max; i += step_size) {
      front_controller_storage.brake_enabled = false;
      front_controller_storage.accel_percentage = i;

      LOG_DEBUG("MOCKING ACCEL PERCENTAGE TO: %f\n", (double)front_controller_storage.accel_percentage);
      delay_ms(1000U);
    }

    front_controller_storage.brake_enabled = true;
    LOG_DEBUG("MOCKING BRAKES\n");
    delay_ms(1000U);

    front_controller_storage.brake_enabled = false;
  }
}
#endif

#if (PRINT_TX_STATS == 1)
TASK(display_ws22_tx_data, TASK_STACK_1024) {
  delay_ms(2000U);
  LOG_DEBUG("Displaying TX data\n");

  while (true) {
    float current = front_controller_storage.ws22_motor_can_storage->control.current;
    uint32_t velocity = front_controller_storage.ws22_motor_can_storage->control.velocity;
    float back_emf_d = front_controller_storage.ws22_motor_can_storage->telemetry.back_emf_d;
    float back_emf_q = front_controller_storage.ws22_motor_can_storage->telemetry.back_emf_q;
    float bus_current = front_controller_storage.ws22_motor_can_storage->telemetry.bus_current;
    float bus_voltage = front_controller_storage.ws22_motor_can_storage->telemetry.bus_voltage;
    float current_d = front_controller_storage.ws22_motor_can_storage->telemetry.current_d;
    float current_q = front_controller_storage.ws22_motor_can_storage->telemetry.current_q;
    float heat_sink_temp = front_controller_storage.ws22_motor_can_storage->telemetry.heat_sink_temp;
    float motor_temp = front_controller_storage.ws22_motor_can_storage->telemetry.motor_temp;
    uint16_t error_flags = front_controller_storage.ws22_motor_can_storage->telemetry.error_flags;
    uint16_t limit_flags = front_controller_storage.ws22_motor_can_storage->telemetry.limit_flags;
    float motor_velocity = front_controller_storage.ws22_motor_can_storage->telemetry.motor_velocity;
    float phase_b_current = front_controller_storage.ws22_motor_can_storage->telemetry.phase_b_current;
    float phase_c_current = front_controller_storage.ws22_motor_can_storage->telemetry.phase_c_current;
    float rail_15v_supply = front_controller_storage.ws22_motor_can_storage->telemetry.rail_15v_supply;
    float vehicle_velocity = front_controller_storage.ws22_motor_can_storage->telemetry.vehicle_velocity;
    float voltage_d = front_controller_storage.ws22_motor_can_storage->telemetry.voltage_d;
    float voltage_q = front_controller_storage.ws22_motor_can_storage->telemetry.voltage_q;

    LOG_DEBUG(
        "WS22 Motor TX:\n"
        "CTRL: I=%.3fA Vel=%lu\n"
        "ELEC: Vbus=%.3fV Ibus=%.3fA Id=%.3fA Iq=%.3fA Vd=%.3fV Vq=%.3fV\n"
        "EMFd=%.3fV EMFq=%.3fV Ib=%.3fA Ic=%.3fA Rail15=%.3fV \n"
        "THERM: Hs=%.2fC Mot=%.2fC\n"
        "MOTION: MotVel=%.3f VehVel=%.3f\n"
        "FLAGS: Err=0x%04X Lim=0x%04X\n",
        (double)current, velocity, (double)bus_voltage, (double)bus_current, (double)current_d, (double)current_q, (double)voltage_d, (double)voltage_q, (double)back_emf_d, (double)back_emf_q,
        (double)phase_b_current, (double)phase_c_current, (double)rail_15v_supply, (double)heat_sink_temp, (double)motor_temp, (double)motor_velocity, (double)vehicle_velocity, error_flags,
        limit_flags);
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

  tasks_init_task(main_cycle, TASK_PRIORITY(4), NULL);
#if (MOCK_PEDAL_VALUES == 1)
  tasks_init_task(cycle_ws22, TASK_PRIORITY(3), NULL);
#endif
#if (PRINT_TX_STATS == 1)
  tasks_init_task(display_ws22_tx_data, TASK_PRIORITY(2), NULL);
#endif

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
