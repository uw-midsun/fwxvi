/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for ws22_api
 *
 * @date   2025-09-24
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "status.h"
#include "delay.h"
#include "ws22_motor_can.h"

/* Intra-component Headers */

/************************************************************************************************
 * Storage definitions
 ************************************************************************************************/
static FrontControllerStorage *front_controller_storage;
static CanStorage s_can_storage = { 0 };
static Ws22MotorCanStorage smoke_ws22_motor_can_storage = { 0 };
// phase currents and dc voltage
// print rpm, current
// ws22_motor_can_transmit_drive_command call every 50ms

// initialize the can settings
static const CanSettings smoke_s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_FRONT_CONTROLLER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = FRONT_CONTROLLER_CAN_TX,
  .rx = FRONT_CONTROLLER_CAN_RX,
  .loopback = false,
  .can_rx_all_cb = ws22_motor_can_process_rx,
};

// What to run here?
TASK(ws22_api, TASK_STACK_1024) {
    while (true) {}
}

// run at 10hz
// MOTOR_COMMAND_TX task → Runs at a frequency < 50ms. Sets target velocity, sets target current using helper functions defined in ws22_motor_can.h. Then sends the drive command. also defined in ws22_motor_can.h
void run_10hz_cycle() {
    while (true) {
    }
}

// run at 100hz
//TASK: MOTOR_COMMAND_RX
void run_100hz_cycle() {
  Ws22MotorTelemetryData *telemetry = &smoke_ws22_motor_can_storage.telemetry;
  while (true) {
    // Prints out the Phase currents, DC voltage, Target Velocity, Target current, and Motor Error/fault. Can run at a frequency >50ms doesn’t matter
    LOG_DEBUG("\nTelemetry:\n");

    /* Status Information (0x401) */
    LOG_DEBUG("errors: %d", telemetry->error_flags);
    LOG_DEBUG("limit flags: %d", telemetry->limit_flags);

    /* Velocity Measurements (0x403) */
    LOG_DEBUG("vehicle velocity: %f", telemetry->vehicle_velocity);
    LOG_DEBUG("motor velocity: %f", telemetry->motor_velocity);

    /* Phase Currents (0x404) */
    LOG_DEBUG("phase current B: %f", telemetry->phase_b_current);
    LOG_DEBUG("phase current C: %f", telemetry->phase_c_current);

    /* Motor Voltages (0x405) */
    LOG_DEBUG("voltage D-axis: %f", telemetry->voltage_d);
    LOG_DEBUG("voltage Q-axis: %f", telemetry->voltage_q);

    /* Motor Currents (0x406) */
    LOG_DEBUG("current D-axis: %f", telemetry->current_d);
    LOG_DEBUG("current Q-axis: %f", telemetry->current_q);

    /* Motor Back EMF (0x407) */
    LOG_DEBUG("back EMF D-axis: %f", telemetry->back_emf_d);
    LOG_DEBUG("back EMF Q-axis: %f", telemetry->back_emf_q);

    /* Temperature Measurements (0x40B) */
    LOG_DEBUG("heat sink temp: %f", telemetry->heat_sink_temp);
    LOG_DEBUG("motor temp: %f", telemetry->motor_temp);

    // test velocity measurement
    // s_process_velocity_measurement(&s_ws22_motor_can_storage.telemetry, msg);
  }
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  log_init();
  can_init(&s_can_storage, &s_can_settings);
  mcu_init();
  tasks_init();
  accel_pedal_init(front_controller_storage);
  ws22_motor_can_init(front_controller_storage);
  pre_loop_init(); // init running at certain frequencies

  tasks_init_task(ws22_api, TASK_PRIORITY(3), NULL);
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
