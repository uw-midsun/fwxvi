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
#include "brake_pedal.h"
#include "front_controller_state_manager.h"
#include "motor_can.h"
#include "opd.h"
#include "ws22_motor_can.h"

#define PRINT_RX_STATS 1U
#define PRINT_CONTROL_DATA 0U
#define MOCK_PEDAL_VALUES 0U

static FrontControllerStorage front_controller_storage = { 0 };

static FrontControllerConfig front_controller_config = { .accel_input_deadzone = FRONT_CONTROLLER_ACCEL_INPUT_DEADZONE,
                                                         .accel_input_remap_min = FRONT_CONTROLLER_ACCEL_REMAP_MIN,
                                                         .accel_input_curve_exponent = FRONT_CONTROLLER_ACCEL_CURVE_EXPONENT,
                                                         .accel_low_pass_filter_alpha = FRONT_CONTROLLER_ACCEL_LPF_ALPHA };

TASK(main_1000hz_cycle, TASK_STACK_1024) {
  StatusCode status = STATUS_CODE_OK;

  // Step 1: Check if the front controller can be initialized
  status = front_controller_init(&front_controller_storage, &front_controller_config);
  front_controller_storage.brake_enabled = false;

  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("front controller initialized\n\r");
  } else {
    LOG_DEBUG("front controller cannot be initialized\n\r");
  }
  delay_ms(500);

  while (true) {
    run_can_rx_all();
    adc_run();
    accel_pedal_run();
    // opd_run();
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

#if (PRINT_RX_STATS == 1)
TASK(display_ws22_tx_data, TASK_STACK_1024) {
  delay_ms(2000U);
  LOG_DEBUG("Displaying TX data\n\r");

  float current;
  uint32_t velocity;
  float back_emf_d;
  float back_emf_q;
  float bus_current;
  float bus_voltage;
  float current_d;
  float current_q;
  float heat_sink_temp;
  float motor_temp;
  uint16_t error_flags;
  uint16_t limit_flags;
  float motor_velocity;
  float phase_b_current;
  float phase_c_current;
  float rail_15v_supply;
  float vehicle_velocity;
  float voltage_d;
  float voltage_q;

  while (true) {
    current = front_controller_storage.ws22_motor_can_storage->control.current;
    velocity = front_controller_storage.ws22_motor_can_storage->control.velocity;
    back_emf_d = front_controller_storage.ws22_motor_can_storage->telemetry.back_emf_d;
    back_emf_q = front_controller_storage.ws22_motor_can_storage->telemetry.back_emf_q;
    bus_current = front_controller_storage.ws22_motor_can_storage->telemetry.bus_current;
    bus_voltage = front_controller_storage.ws22_motor_can_storage->telemetry.bus_voltage;
    current_d = front_controller_storage.ws22_motor_can_storage->telemetry.current_d;
    current_q = front_controller_storage.ws22_motor_can_storage->telemetry.current_q;
    heat_sink_temp = front_controller_storage.ws22_motor_can_storage->telemetry.heat_sink_temp;
    motor_temp = front_controller_storage.ws22_motor_can_storage->telemetry.motor_temp;
    error_flags = front_controller_storage.ws22_motor_can_storage->telemetry.error_flags;
    limit_flags = front_controller_storage.ws22_motor_can_storage->telemetry.limit_flags;
    motor_velocity = front_controller_storage.ws22_motor_can_storage->telemetry.motor_velocity;
    phase_b_current = front_controller_storage.ws22_motor_can_storage->telemetry.phase_b_current;
    phase_c_current = front_controller_storage.ws22_motor_can_storage->telemetry.phase_c_current;
    rail_15v_supply = front_controller_storage.ws22_motor_can_storage->telemetry.rail_15v_supply;
    vehicle_velocity = front_controller_storage.ws22_motor_can_storage->telemetry.vehicle_velocity;
    voltage_d = front_controller_storage.ws22_motor_can_storage->telemetry.voltage_d;
    voltage_q = front_controller_storage.ws22_motor_can_storage->telemetry.voltage_q;

    LOG_DEBUG("WS22 Motor TX:\n\r");
    delay_ms(20U);

    LOG_DEBUG("CTRL: I=%ldA Vel=%lu\n\r", (int32_t)(current * 1000), velocity);
    delay_ms(20U);

    LOG_DEBUG("ELEC: Vbus=%ldV Ibus=%ldA Id=%ldA Iq=%ldA Vd=%ldV Vq=%ldV\n\r", (int32_t)bus_voltage, (int32_t)(bus_current * 1000), (int32_t)(current_d * 1000), (int32_t)(current_q * 1000),
              (int32_t)voltage_d, (int32_t)voltage_q);
    delay_ms(20U);

    LOG_DEBUG("EMFd=%ldV EMFq=%ldV Ib=%ldA Ic=%ldA Rail15=%ldV \n\r", (int32_t)back_emf_d, (int32_t)back_emf_q, (int32_t)(phase_b_current * 1000), (int32_t)(phase_c_current * 1000),
              (int32_t)rail_15v_supply);
    delay_ms(20U);

    LOG_DEBUG("THERM: Hs=%ldC Mot=%ldC\n\r", (int32_t)(heat_sink_temp * 10), (int32_t)(motor_temp * 10));
    delay_ms(20U);

    LOG_DEBUG("MOTION: MotVel=%ld VehVel=%ld\n\r", (int32_t)motor_velocity, (int32_t)vehicle_velocity);
    delay_ms(20U);

    LOG_DEBUG("FLAGS: Err=0x%04X Lim=0x%04X\n\r", error_flags, limit_flags);
    delay_ms(20U);

    delay_ms(1000U);
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

    LOG_DEBUG("CTRL: I=%ldA Vel=%lu\n\r", (int32_t)(current * 1000), velocity);
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
#if (PRINT_RX_STATS == 1)
  tasks_init_task(display_ws22_tx_data, TASK_PRIORITY(2), NULL);
#endif
  tasks_init_task(main_10hz_cycle, TASK_PRIORITY(3), NULL);

#if (PRINT_CONTROL_DATA == 1)
  tasks_init_task(display_ws22_control_data, TASK_PRIORITY(2), NULL);
#endif

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
