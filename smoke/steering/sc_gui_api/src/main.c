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

#ifndef SHOW_SMOKE_TEST_LOGO_IMAGE
#define SHOW_SMOKE_TEST_LOGO_IMAGE 1
#endif
#ifndef SMOKE_TEST_LOGO_MARGIN_PX
#define SMOKE_TEST_LOGO_MARGIN_PX 8
#endif

/* Inter-component Headers */
#include "delay.h"
#include "display.h"
#if SHOW_SMOKE_TEST_LOGO_IMAGE
#include "gui_drive_screen.h"
#include "gui_screens.h"
#include "lvgl_image.h"
#endif
#include "log.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#if SHOW_SMOKE_TEST_LOGO_IMAGE
#include "ms_logo_image.h"
#endif

/* All lvgl wrappers are wrapped with this */
#ifndef STM32L4P5xx
#define STM32L4P5xx
#endif

#define GUI_UPDATE_PERIOD_MS 20U

static SteeringStorage s_demo_storage = { 0 };
static Ws22MotorCanStorage s_demo_motor_storage = { 0 };

#if SHOW_SMOKE_TEST_LOGO_IMAGE
static LvglImage s_smoke_logo_image = { 0 };

static StatusCode s_create_smoke_logo_image(GuiScreen *screen) {
  const LvglImageConfig logo_config = {
    .size = { .width = MS_LOGO_IMAGE_WIDTH, .height = MS_LOGO_IMAGE_HEIGHT },
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_IN_TOP_RIGHT, .x_offset = -SMOKE_TEST_LOGO_MARGIN_PX, .y_offset = SMOKE_TEST_LOGO_MARGIN_PX },
    },
    .source = &s_midnight_sun_logo_image,
    .recolor_enabled = false,
  };

  return lvgl_image_create(&s_smoke_logo_image, &logo_config, screen);
}

static StatusCode s_create_smoke_drive_screen(GuiScreen *screen) {
  status_ok_or_return(gui_drive_screen_init(screen));
  return s_create_smoke_logo_image(screen);
}

static void s_destroy_smoke_drive_screen(void) {
  gui_drive_screen_deinit();
  s_smoke_logo_image = (LvglImage){ 0 };
}

static StatusCode s_register_smoke_logo_screen(void) {
  const GuiScreenDescriptor smoke_drive_screen = {
    .id = GUI_SCREEN_DRIVE,
    .name = "Drive",
    .create = s_create_smoke_drive_screen,
    .destroy = s_destroy_smoke_drive_screen,
  };

  return gui_screens_register(&smoke_drive_screen);
}
#endif

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
  uint32_t fault_phase = (step / 120U) % 6U;

  /* Motor telemetry is read from the WS22 storage by the display render path */
  s_demo_motor_storage.telemetry.vehicle_velocity_kph = (float)s_triangle_wave(step, 400U, 160U);
  s_demo_motor_storage.telemetry.bus_voltage = (float)(110U + s_triangle_wave(step + 60U, 260U, 30U));
  s_demo_motor_storage.telemetry.motor_temp = (float)(25U + s_triangle_wave(step + 120U, 240U, 75U));

  s_demo_storage.display_data.pedal_percentage = (uint8_t)s_triangle_wave(step + 80U, 180U, 100U);
  s_demo_storage.display_data.brake_percentage = (uint8_t)s_triangle_wave(step + 100U, 200U, 100U);
  s_demo_storage.display_data.brake_enabled = ((step / 45U) % 6U) == 0U;
  s_demo_storage.display_data.pack_voltage = 120U + s_triangle_wave(step + 20U, 300U, 40U);
  s_demo_storage.display_data.pack_current = 5U + s_triangle_wave(step + 140U, 280U, 70U);
  s_demo_storage.display_data.min_cell_voltage_mv = 3300U + s_triangle_wave(step + 10U, 220U, 500U);
  s_demo_storage.display_data.max_cell_voltage_mv = 3600U + s_triangle_wave(step + 50U, 220U, 500U);
  s_demo_storage.display_data.max_cell_temp = 30U + s_triangle_wave(step + 90U, 250U, 35U);
  s_demo_storage.display_data.state_of_charge = (uint8_t)s_triangle_wave(step + 30U, 320U, 100U);

  /* Thermistor temperatures (C) for the Thermistors screen - stagger each channel so the table
     shows independent motion across a plausible ~20-45 C pack range */
  for (uint8_t i = 0U; i < NUMBER_OF_THERMISTORS; ++i) {
    s_demo_storage.display_data.thermistor_temp_c[i] = 20U + s_triangle_wave(step + (uint32_t)i * 20U, 200U, 25U);
  }

  /* Aux battery voltage in mV (~12.0-12.8 V) and net energy used ramp (Wh) */
  s_demo_storage.display_data.aux_voltage = (int16_t)(12000 + s_triangle_wave(step + 15U, 240U, 800U));
  s_demo_storage.display_data.energy_used_wh = (float)s_triangle_wave(step + 5U, 600U, 5000U);

  /* Cycle through the fault types so the pack-screen detail banner exercises every decode branch.
     Cell voltages are in 100uV units (42000 = 4.200 V), temperature in C, current in A. */
  BpsFaultData fault_data = { .raw = 0U };
  switch (fault_phase) {
    case 1: /* Overvoltage: cell + voltage */
      s_demo_storage.display_data.bps_fault = BPS_FAULT_OVERVOLTAGE_MASK;
      s_demo_storage.display_data.bps_fault_cell = 12U;
      fault_data.cell.cell_index = 12U;
      fault_data.cell.cell_voltage = (uint16_t)(42000U + s_triangle_wave(step, 96U, 200U));
      break;
    case 2: /* Unbalance: both cells + both voltages */
      s_demo_storage.display_data.bps_fault = BPS_FAULT_UNBALANCE_MASK;
      s_demo_storage.display_data.bps_fault_cell = 12U;
      fault_data.unbalance.max_cell_index = 12U;
      fault_data.unbalance.min_cell_index = 5U;
      fault_data.unbalance.max_cell_voltage = 42000U;
      fault_data.unbalance.min_cell_voltage = 31000U;
      break;
    case 3: /* Overtemp cell: cell + temperature */
      s_demo_storage.display_data.bps_fault = BPS_FAULT_OVERTEMP_CELL_MASK;
      s_demo_storage.display_data.bps_fault_cell = 7U;
      fault_data.temp.cell_index = 7U;
      fault_data.temp.temperature_c = (int16_t)(60 + s_triangle_wave(step, 96U, 8U));
      break;
    case 4: /* Overcurrent: signed pack current */
      s_demo_storage.display_data.bps_fault = BPS_FAULT_OVERCURRENT_MASK;
      s_demo_storage.display_data.bps_fault_cell = 0U;
      fault_data.current.current_a = 140.0f + (float)s_triangle_wave(step, 96U, 20U);
      break;
    case 5: /* Killswitch: name only, no numeric payload */
      s_demo_storage.display_data.bps_fault = BPS_FAULT_KILLSWITCH_MASK;
      s_demo_storage.display_data.bps_fault_cell = 0U;
      break;
    default: /* Healthy: banner hidden */
      s_demo_storage.display_data.bps_fault = 0U;
      s_demo_storage.display_data.bps_fault_cell = 0U;
      break;
  }
  s_demo_storage.display_data.bps_fault_data = fault_data;
}

TASK(sc_gui_api, TASK_STACK_2048) {
  s_demo_storage.ws22_motor_can_storage = &s_demo_motor_storage;

#if SHOW_SMOKE_TEST_LOGO_IMAGE
  StatusCode status = s_register_smoke_logo_screen();
  if (status != STATUS_CODE_OK) {
    while (true) {
      LOG_DEBUG("smoke logo screen registration failed: %u\r\n", status);
      delay_ms(10);
    }
  }

  status = display_init(&s_demo_storage);
#else
  StatusCode status = display_init(&s_demo_storage);
#endif

  if (status != STATUS_CODE_OK) {
    while (true) {
      LOG_DEBUG("display_init failed: %u\r\n", status);
      delay_ms(10);
    }
  }

  LOG_DEBUG("Initialization success!");
#ifdef MS_PLATFORM_X86
  LOG_DEBUG("Smoke controls: LEFT+RIGHT opens/closes menu, UP/DOWN navigate, ENTER selects, ESC closes\r\n");
#endif

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
