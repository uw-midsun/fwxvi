/************************************************************************************************
 * @file    main.c
 *
 * @brief   Smoke test for our display gui
 * @details Calls functions in display.c and simulates values via a triangle wave
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "display.h"
#include "log.h"
#include "mcu.h"
#include "tasks.h"
#include "delay.h"

/* Intra-component Headers */

/* All lvgl wrappers are wrapped with this */
#ifndef STM32L4P5xx
#define STM32L4P5xx
#endif

#define GUI_UPDATE_PERIOD_MS 20U

static SteeringStorage s_demo_storage = { 0 };

/**
 * @brief   Generate a triangle-wave sample for the GUI smoke test
 * @param   step Current simulation step
 * @param   period_steps Number of steps in the rising half of the wave
 * @param   max_value Peak value returned by the wave
 * @return  Current triangle-wave sample in the range [0, max_value]
 */
static uint16_t s_triangle_wave(uint32_t step, uint32_t period_steps, uint16_t max_value) {
  if (period_steps == 0U || max_value == 0U) {
    return 0U;
  }

  uint32_t phase = step % (2U * period_steps);
  if (phase >= period_steps) {
    phase = (2U * period_steps) - phase;
  }

  return (uint16_t)((phase * max_value) / period_steps);
}

static void s_update_demo_display_data(uint32_t step) {
  uint32_t fault_phase = (step / 120U) % 4U;

  s_demo_storage.display_data.vehicle_velocity = (int16_t)s_triangle_wave(step, 400U, 160U);
  s_demo_storage.display_data.pedal_percentage = (uint8_t)s_triangle_wave(step + 80U, 180U, 100U);
  s_demo_storage.display_data.brake_enabled = ((step / 45U) % 6U) == 0U;
  s_demo_storage.display_data.motor_heatsink_temp = (int16_t)(25U + s_triangle_wave(step + 120U, 240U, 75U));
  s_demo_storage.display_data.motor_velocity = (int16_t)s_triangle_wave(step + 40U, 160U, 100U);
  s_demo_storage.display_data.pack_voltage = 120U + s_triangle_wave(step + 20U, 300U, 40U);
  s_demo_storage.display_data.motor_bus_voltage = 110U + s_triangle_wave(step + 60U, 260U, 30U);
  s_demo_storage.display_data.min_cell_voltage_mv = 3300U + s_triangle_wave(step + 10U, 220U, 500U);
  s_demo_storage.display_data.max_cell_voltage_mv = 3600U + s_triangle_wave(step + 50U, 220U, 500U);
  s_demo_storage.display_data.max_cell_temp = 30U + s_triangle_wave(step + 90U, 250U, 35U);
  s_demo_storage.display_data.state_of_charge = (uint8_t)s_triangle_wave(step + 30U, 320U, 100U);

  switch (fault_phase) {
    case 1:
      s_demo_storage.display_data.bps_fault = BPS_FAULT_OVERVOLTAGE_MASK;
      s_demo_storage.display_data.bps_fault_cell = 1U + (uint8_t)(s_triangle_wave(step, 96U, 23U));
      break;
    case 2:
      s_demo_storage.display_data.bps_fault = BPS_FAULT_KILLSWITCH_MASK;
      s_demo_storage.display_data.bps_fault_cell = 0U;
      break;
    case 3:
      s_demo_storage.display_data.bps_fault = BPS_FAULT_OVERTEMP_CELL_MASK;
      s_demo_storage.display_data.bps_fault_cell = 1U + (uint8_t)(s_triangle_wave(step + 25U, 96U, 23U));
      break;
    default:
      s_demo_storage.display_data.bps_fault = 0U;
      s_demo_storage.display_data.bps_fault_cell = 0U;
      break;
  }
}


TASK(sc_gui_api, TASK_STACK_2048) {
  StatusCode status = display_init(&s_demo_storage);

  if (status != STATUS_CODE_OK) {
    while (true) {
      LOG_DEBUG("display_init failed: %u\r\n", status);
      delay_ms(10);
    }
  }

  LOG_DEBUG("Initialization success!");

  uint32_t step = 0U;

  while (true) {
    s_update_demo_display_data(step++);
    delay_ms(GUI_UPDATE_PERIOD_MS);
  }

}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
  /* TODO: For some reason, this breaks, too much work to figure out why, and we don't need it */
#else
int main(void) {
#endif
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(sc_gui_api, TASK_PRIORITY(4), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\r\n");
  return 0;
}
