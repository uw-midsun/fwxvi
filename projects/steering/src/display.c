/************************************************************************************************
 * @file   display.c
 *
 * @brief  Source file for display control
 *
 * @date   2025-07-22
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/* Inter-component Headers */
#include "buzzer.h"
#include "gpio.h"
#include "gui.h"
#include "gui_drive_screen.h"
#include "gui_menu.h"
#include "gui_pack_screen.h"
#include "gui_pedal_calib_screen.h"
#include "gui_screens.h"
#include "gui_therm_screen.h"
#include "gui_widgets.h"
#include "log.h"
#include "ltdc.h"
#include "pwm.h"
#include "status.h"
#include "tasks.h"
#ifdef MS_PLATFORM_X86
#include <SDL2/SDL.h>
#endif

/* Intra-component Headers */
#include "display.h"
#include "pedal_calib.h"
#include "steering_getters.h"
#include "steering_hw_defs.h"

static SteeringStorage *steering_storage = NULL;
static DisplayData *display_data = NULL;

/* BPS-fault takeover lifecycle. Instead of a dedicated fault screen (extra RAM), a live BPS fault
 * reuses the pedal-calib screen with fault styling forced on. */
typedef enum {
  FAULT_UI_IDLE = 0,     /* No fault handling in progress */
  FAULT_UI_ACTIVE,       /* Fault takeover shown on the pedal-calib screen */
  FAULT_UI_ACKNOWLEDGED, /* Driver dismissed the takeover (BPS disabled); waiting for the fault to clear */
} FaultUiState;

static FaultUiState s_fault_ui_state = FAULT_UI_IDLE;

/* Screen the driver was viewing before a BPS fault forced the takeover screen, restored on clear */
static GuiScreenId s_screen_before_fault = GUI_SCREEN_DRIVE;

/* Last fault detail pushed to the takeover screen, so we only re-render on change */
static uint16_t s_last_fault_code;
static uint8_t s_last_fault_cell;

/* Enable display when high */
static GpioAddress s_display_ctrl = GPIO_STEERING_DISPLAY_CTRL;
static GpioAddress s_display_pwm = GPIO_STEERING_BACKLIGHT;
static LtdcSettings settings = { 0 };
static uint8_t framebuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT * 2] __attribute__((aligned(32))); /* RGB565 */

#define NUMBER_OF_RED_BITS 8
#define NUMBER_OF_GREEN_BITS 8
#define NUMBER_OF_BLUE_BITS 8

/* Medium cycle runs at 10 Hz -> 0.1 s per sample. Converts pack power (V*A) to watt-hours. */
#define ENERGY_SAMPLE_PERIOD_H (0.1f / 3600.0f)

#ifdef MS_PLATFORM_X86
typedef struct {
  bool left_pressed;
  bool right_pressed;
  bool up_pressed;
  bool down_pressed;
  bool return_pressed;
  bool escape_pressed;
  bool menu_chord_latched;
} DisplayKeyboardState;

static DisplayKeyboardState s_keyboard_state = { 0 };

/**
 * @brief   Translate x86 keyboard input into overlay menu actions
 * @details This keeps all LVGL interaction on the display task and avoids cross-task UI access.
 */
static void s_process_x86_keyboard_input(void) {
  int num_keys = 0;
  const uint8_t *key_state = SDL_GetKeyboardState(&num_keys);
  bool left_pressed;
  bool right_pressed;
  bool up_pressed;
  bool down_pressed;
  bool return_pressed;
  bool escape_pressed;
  bool menu_chord_pressed;
  bool up_pressed_edge;
  bool down_pressed_edge;
  bool return_pressed_edge;
  bool escape_pressed_edge;

  if (key_state == NULL || num_keys <= SDL_SCANCODE_ESCAPE) {
    return;
  }

  left_pressed = key_state[SDL_SCANCODE_LEFT] != 0;
  right_pressed = key_state[SDL_SCANCODE_RIGHT] != 0;
  up_pressed = key_state[SDL_SCANCODE_UP] != 0;
  down_pressed = key_state[SDL_SCANCODE_DOWN] != 0;
  return_pressed = key_state[SDL_SCANCODE_RETURN] != 0;
  escape_pressed = key_state[SDL_SCANCODE_ESCAPE] != 0;

  up_pressed_edge = up_pressed && !s_keyboard_state.up_pressed;
  down_pressed_edge = down_pressed && !s_keyboard_state.down_pressed;
  return_pressed_edge = return_pressed && !s_keyboard_state.return_pressed;
  escape_pressed_edge = escape_pressed && !s_keyboard_state.escape_pressed;

  menu_chord_pressed = left_pressed && right_pressed;
  if (menu_chord_pressed && !s_keyboard_state.menu_chord_latched) {
    gui_menu_toggle();
    s_keyboard_state.menu_chord_latched = true;
  } else if (!menu_chord_pressed) {
    s_keyboard_state.menu_chord_latched = false;
  }

  if (gui_menu_is_open() && !menu_chord_pressed) {
    if (up_pressed_edge) {
      gui_menu_move_up();
    }

    if (down_pressed_edge) {
      gui_menu_move_down();
    }

    if (return_pressed_edge) {
      gui_menu_select(VEHICLE_DRIVE_STATE_INVALID);
    }

    if (escape_pressed_edge) {
      gui_menu_close();
    }
  } else if (!gui_menu_is_open() && gui_screens_get_current() == GUI_SCREEN_PEDAL_CALIB && !gui_pedal_calib_screen_is_fault_active()) {
    /* Ignore the start button while a BPS fault has taken over the pedal-calib screen */
    if (return_pressed_edge) {
      steering_pedal_calib_request(steering_storage);
    }
  }

  s_keyboard_state.left_pressed = left_pressed;
  s_keyboard_state.right_pressed = right_pressed;
  s_keyboard_state.up_pressed = up_pressed;
  s_keyboard_state.down_pressed = down_pressed;
  s_keyboard_state.return_pressed = return_pressed;
  s_keyboard_state.escape_pressed = escape_pressed;
}
#endif

static void s_process_pending_menu_input(void) {
  StatusCode menu_status = gui_menu_process_pending();

#ifndef MS_PLATFORM_X86
  if (menu_status == STATUS_CODE_INVALID_ARGS) {
    // TODO
  } else if (menu_status != STATUS_CODE_OK) {
    LOG_DEBUG("gui menu input failed: %u\r\n", menu_status);
  }
#endif
}

static StatusCode s_render_gui_step(void) {
  GuiScreenId current_screen = gui_screens_get_current();
  bool fault_present = (display_data->bps_fault != 0U);

  /* A live BPS fault takes over the whole display (ASC 2026 8.7.B). Rather than allocate a dedicated
   * fault screen, we reuse the pedal-calib screen with fault styling forced on. The driver dismisses
   * the takeover by navigating away (which disables BPS), or it clears automatically if the fault
   * goes away on its own. */
  switch (s_fault_ui_state) {
    case FAULT_UI_IDLE:
      if (fault_present) {
        s_screen_before_fault = current_screen;
        status_ok_or_return(gui_screens_show(GUI_SCREEN_PEDAL_CALIB));
        s_last_fault_code = display_data->bps_fault;
        s_last_fault_cell = display_data->bps_fault_cell;
        status_ok_or_return(gui_pedal_calib_screen_set_fault(true, display_data->bps_fault_live, s_last_fault_code, s_last_fault_cell));
        s_fault_ui_state = FAULT_UI_ACTIVE;
        return gui_render();
      }
      break;

    case FAULT_UI_ACTIVE:
      if (!fault_present) {
        /* Fault cleared on its own: drop the fault styling and return to the prior screen. */
        status_ok_or_return(gui_pedal_calib_screen_set_fault(false, false, 0U, 0U));
        status_ok_or_return(gui_screens_show(s_screen_before_fault));
        s_fault_ui_state = FAULT_UI_IDLE;
        current_screen = gui_screens_get_current();
        break;
      }
      if (current_screen != GUI_SCREEN_PEDAL_CALIB) {
        /* Driver navigated away = acknowledged the fault: force SECURE MODE off and stop forcing the
         * takeover until the fault re-asserts. The pedal-calib screen (and its fault flag) was torn
         * down by the navigation, so no styling teardown is needed here. */
        status_ok_or_return(steering_force_disable_bps());
        s_fault_ui_state = FAULT_UI_ACKNOWLEDGED;
        break;
      }
      /* Still on the takeover screen: refresh the fault detail only when it changes. */
      if (display_data->bps_fault != s_last_fault_code || display_data->bps_fault_cell != s_last_fault_cell) {
        s_last_fault_code = display_data->bps_fault;
        s_last_fault_cell = display_data->bps_fault_cell;
        status_ok_or_return(gui_pedal_calib_screen_set_fault(true, display_data->bps_fault_live, s_last_fault_code, s_last_fault_cell));
      }
      return gui_render();

    case FAULT_UI_ACKNOWLEDGED:
      if (!fault_present) {
        s_fault_ui_state = FAULT_UI_IDLE;
      }
      break;
  }

  if (current_screen == GUI_SCREEN_DRIVE || current_screen == GUI_SCREEN_PACK_VOLTAGE) {
    status_ok_or_return(gui_widgets_set_top_label(display_data->pack_voltage, display_data->pack_current, steering_storage->ws22_motor_can_storage->telemetry.bus_voltage,
                                                  steering_storage->ws22_motor_can_storage->telemetry.bus_current, display_data->bps_fault, display_data->bps_fault_cell,
                                                  steering_storage->ws22_motor_can_storage->telemetry.merged_flags));
    status_ok_or_return(gui_widgets_set_cell_stats_label(display_data->min_cell_voltage_mv, display_data->max_cell_voltage_mv));
    status_ok_or_return(gui_widgets_set_temps_stats_label(steering_storage->ws22_motor_can_storage->telemetry.motor_temp, display_data->max_cell_temp));
    status_ok_or_return(gui_widgets_set_soc_bar(display_data->state_of_charge));
    status_ok_or_return(gui_widgets_set_aux_energy_label(display_data->aux_voltage, display_data->energy_used_wh));
  }

  if (current_screen == GUI_SCREEN_DRIVE) {
    status_ok_or_return(gui_drive_screen_widget_set_speed(steering_storage->ws22_motor_can_storage->telemetry.vehicle_velocity_kph));
    status_ok_or_return(gui_drive_screen_widget_set_throttle_bar(display_data->pedal_percentage));
    status_ok_or_return(gui_drive_screen_widget_set_brake_bar(display_data->brake_percentage));
    if (steering_storage->display_data.drive_state == VEHICLE_DRIVE_STATE_REGEN) {
      gui_widgets_set_brake_bar_color(GUI_COLOR_REGEN_BRAKE_FILL);
    } else {
      gui_widgets_set_brake_bar_color(GUI_COLOR_BRAKE_FILL);
    }
    status_ok_or_return(gui_drive_screen_widget_set_cc_speed(steering_storage->cruise_control_target_speed_kmh, steering_storage->cruise_control_enabled));

  } else if (current_screen == GUI_SCREEN_PACK_VOLTAGE) {
    for (uint8_t i = 0; i < 36; ++i) status_ok_or_return(gui_pack_screen_widget_set_pack_voltage(i, display_data->cell_voltages[i]));

    status_ok_or_return(gui_pack_screen_widget_set_speed_label(steering_storage->ws22_motor_can_storage->telemetry.vehicle_velocity_kph));
    status_ok_or_return(gui_pack_screen_widget_set_cc_speed(steering_storage->cruise_control_target_speed_kmh, steering_storage->cruise_control_enabled));
    status_ok_or_return(gui_pack_screen_widget_set_fault(display_data->bps_fault, display_data->bps_fault_cell, display_data->bps_fault_data));

  } else if (current_screen == GUI_SCREEN_THERMISTORS) {
    for (uint8_t i = 0; i < NUMBER_OF_THERMISTORS; ++i) status_ok_or_return(gui_therm_screen_widget_set_thermistor(i, display_data->thermistor_temp_c[i]));

  } else if (current_screen == GUI_SCREEN_PEDAL_CALIB) {
    steering_pedal_calib_rx(steering_storage);
  }

  return gui_render();
}

TASK(display_lvgl_task, TASK_STACK_2048) {
#ifdef MS_PLATFORM_X86
  StatusCode init_status = gui_init(&settings);
  if (init_status != STATUS_CODE_OK) {
    LOG_DEBUG("gui_init failed: %u\r\n", init_status);
    vTaskEndScheduler();
    return;
  }

  LOG_DEBUG("LVGL display initialized\r\n");
#endif

  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (true) {
    s_process_pending_menu_input();

    StatusCode render_status = s_render_gui_step();
    if (render_status != STATUS_CODE_OK) {
      LOG_DEBUG("gui render step failed: %u\r\n", render_status);
    }

#ifdef MS_PLATFORM_X86
    if (!ltdc_process_events()) {
      ltdc_cleanup();
      vTaskEndScheduler();
      return;
    }

    s_process_x86_keyboard_input();
#endif
    xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5));
  }
}

StatusCode display_init(SteeringStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  steering_storage = storage;
  display_data = &(steering_storage->display_data);

  LtdcTimingConfig timing_config = {
    .hsync = HORIZONTAL_SYNC_WIDTH, .vsync = VERTICAL_SYNC_WIDTH, .hbp = HORIZONTAL_BACK_PORCH, .vbp = VERTICAL_BACK_PORCH, .hfp = HORIZONTAL_FRONT_PORCH, .vfp = VERTICAL_FRONT_PORCH
  };

  LtdcGpioConfig gpio_config = { .clk = GPIO_STEERING_DISPLAY_LTDC_CLOCK,
                                 .hsync = GPIO_STEERING_DISPLAY_LTDC_HSYNC,
                                 .vsync = GPIO_STEERING_DISPLAY_LTDC_VSYNC,
                                 .de = GPIO_STEERING_DISPLAY_LTDC_DE,
                                 .r = GPIO_STEERING_DISPLAY_LTDC_RED_PINS,
                                 .g = GPIO_STEERING_DISPLAY_LTDC_GREEN_PINS,
                                 .b = GPIO_STEERING_DISPLAY_LTDC_BLUE_PINS,
                                 .num_red_bits = NUMBER_OF_RED_BITS,
                                 .num_green_bits = NUMBER_OF_GREEN_BITS,
                                 .num_blue_bits = NUMBER_OF_BLUE_BITS };
  settings.width = DISPLAY_WIDTH;
  settings.height = DISPLAY_HEIGHT;
  settings.framebuffer = framebuffer;
  settings.clut = clut_get_table();
  settings.clut_size = NUM_COLOR_INDICES;
  settings.timing = timing_config;
  settings.gpio_config = gpio_config;

  gpio_init_pin(&s_display_ctrl, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&s_display_pwm, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

#ifdef MS_PLATFORM_X86
  status_ok_or_return(tasks_init_task(display_lvgl_task, TASK_PRIORITY(2), NULL));
#else
  status_ok_or_return(gui_init(&settings));

  status_ok_or_return(tasks_init_task(display_lvgl_task, TASK_PRIORITY(2), NULL));

  LOG_DEBUG("LVGL display initialized\r\n");
#endif
  return STATUS_CODE_OK;
}

StatusCode display_run() {
  return STATUS_CODE_OK;
}

static uint16_t prv_safe_cell_voltage(float raw) {
  if (raw < 20000.0f || raw > 50000.0f) {
    LOG_DEBUG("Cell voltage out of range: %d\n", (int)raw);
    return 0U;
  }
  return (uint16_t)(raw);
}

StatusCode display_rx_slow() {
  // persist_commit(steering_storage->persist_storage);

  return STATUS_CODE_OK;
}

StatusCode display_rx_medium() {
  // All math here in 47/16 fixed point :).
  TickType_t now = xTaskGetTickCount();

  TickType_t elapsed = now - display_data->display_rx_medium_last_start;
  display_data->display_rx_medium_last_start = now;

  // This line a little sketch.
  int64_t elapsed_ms = ((int64_t)(uint16_t)elapsed * portTICK_PERIOD_MS) << 16;
  int64_t elapsed_hr = (elapsed_ms) / ((int64_t)3600 * 1000);

  int64_t current = (int64_t)(display_data->pack_current * (1 << 16));
  int64_t voltage = (int64_t)(display_data->pack_voltage * (1 << 16));
  int64_t power = (current * voltage) >> 16;

  display_data->power_usage += (elapsed_hr * power) >> 16U;
  display_data->energy_used_wh = (float)(display_data->power_usage) / (1 << 16);

  display_data->precharge_complete = get_rear_controller_status_triggers_motor_precharge_complete();
  display_data->brake_enabled = get_drive_status_state_data_brake_enabled();
  display_data->regen_enabled = get_drive_status_state_data_regen_enabled();
  display_data->pedal_percentage = (uint8_t)get_drive_status_pedal_percentage();
  display_data->brake_percentage = (uint8_t)get_drive_status_brake_percentage();
  display_data->drive_state = (VehicleDriveState)get_drive_status_state_data_drive_state();

  display_data->bps_fault = get_rear_controller_status_triggers_bps_fault();
  display_data->bps_fault_cell = get_rear_controller_status_triggers_cell_at_fault();
  display_data->bps_fault_live = (bool)get_rear_controller_status_triggers_bps_fault_live();
  display_data->bps_fault_data.raw = get_bps_fault_info_extra_info();

  steering_storage->ws22_motor_can_storage->telemetry.motor_velocity = (float)(steering_storage->ws22_motor_can_storage->telemetry.motor_velocity * 3.141f * 0.558f * 0.001 * 60);

  display_data->aux_voltage = (int16_t)get_power_input_stats_input_aux_voltage();
  display_data->aux_current = (int16_t)get_power_input_stats_input_aux_current();

  display_data->pack_voltage = get_battery_stats_A_pack_voltage_v();
  display_data->pack_current = get_battery_stats_B_pack_current_a();

  display_data->min_cell_voltage_mv = (uint16_t)get_battery_stats_B_min_cell_voltage();
  display_data->max_cell_voltage_mv = (uint16_t)get_battery_stats_B_max_cell_voltage();
  display_data->max_cell_temp = (uint16_t)get_battery_stats_B_max_temperature();
  /* pack_soc is now a float percentage (0-100) straight off CAN */
  display_data->state_of_charge = get_battery_stats_A_pack_soc();

  /* Greatest piece of code ever written. */
  const uint16_t cell_voltages[36] = {
    prv_safe_cell_voltage(get_AFE1_status_A_voltage_0()),  prv_safe_cell_voltage(get_AFE1_status_A_voltage_1()),  prv_safe_cell_voltage(get_AFE1_status_A_voltage_2()),
    prv_safe_cell_voltage(get_AFE1_status_B_voltage_3()),  prv_safe_cell_voltage(get_AFE1_status_B_voltage_4()),  prv_safe_cell_voltage(get_AFE1_status_B_voltage_5()),
    prv_safe_cell_voltage(get_AFE1_status_C_voltage_6()),  prv_safe_cell_voltage(get_AFE1_status_C_voltage_7()),  prv_safe_cell_voltage(get_AFE1_status_C_voltage_8()),
    prv_safe_cell_voltage(get_AFE1_status_D_voltage_9()),  prv_safe_cell_voltage(get_AFE1_status_D_voltage_10()), prv_safe_cell_voltage(get_AFE1_status_D_voltage_11()),
    prv_safe_cell_voltage(get_AFE1_status_E_voltage_12()), prv_safe_cell_voltage(get_AFE1_status_E_voltage_13()), prv_safe_cell_voltage(get_AFE1_status_E_voltage_14()),
    prv_safe_cell_voltage(get_AFE1_status_F_voltage_15()), prv_safe_cell_voltage(get_AFE1_status_F_voltage_16()), prv_safe_cell_voltage(get_AFE1_status_F_voltage_17()),
    prv_safe_cell_voltage(get_AFE2_status_A_voltage_0()),  prv_safe_cell_voltage(get_AFE2_status_A_voltage_1()),  prv_safe_cell_voltage(get_AFE2_status_A_voltage_2()),
    prv_safe_cell_voltage(get_AFE2_status_B_voltage_3()),  prv_safe_cell_voltage(get_AFE2_status_B_voltage_4()),  prv_safe_cell_voltage(get_AFE2_status_B_voltage_5()),
    prv_safe_cell_voltage(get_AFE2_status_C_voltage_6()),  prv_safe_cell_voltage(get_AFE2_status_C_voltage_7()),  prv_safe_cell_voltage(get_AFE2_status_C_voltage_8()),
    prv_safe_cell_voltage(get_AFE2_status_D_voltage_9()),  prv_safe_cell_voltage(get_AFE2_status_D_voltage_10()), prv_safe_cell_voltage(get_AFE2_status_D_voltage_11()),
    prv_safe_cell_voltage(get_AFE2_status_E_voltage_12()), prv_safe_cell_voltage(get_AFE2_status_E_voltage_13()), prv_safe_cell_voltage(get_AFE2_status_E_voltage_14()),
    prv_safe_cell_voltage(get_AFE2_status_F_voltage_15()), prv_safe_cell_voltage(get_AFE2_status_F_voltage_16()), prv_safe_cell_voltage(get_AFE2_status_F_voltage_17()),
  };

  memcpy(display_data->cell_voltages, cell_voltages, sizeof(cell_voltages));

  /* AFE_temperature is paginated: each frame carries 7 thermistor readings for page `id`
     (global index = id*7 + n). The rear controller sends each reading already converted to a
     temperature in whole degrees C, so store the byte directly. */
  uint16_t therm_base = (uint16_t)get_AFE_temperature_id() * 7U;
  const uint8_t therm_page[7] = {
    get_AFE_temperature_temperature_0(), get_AFE_temperature_temperature_1(), get_AFE_temperature_temperature_2(), get_AFE_temperature_temperature_3(),
    get_AFE_temperature_temperature_4(), get_AFE_temperature_temperature_5(), get_AFE_temperature_temperature_6(),
  };
  for (uint8_t i = 0U; i < 7U; ++i) {
    uint16_t therm_idx = therm_base + i;
    if (therm_idx < NUMBER_OF_THERMISTORS) {
      display_data->thermistor_temp_c[therm_idx] = therm_page[i];
    }
  }

  return STATUS_CODE_OK;
}

StatusCode display_rx_fast() {
  return STATUS_CODE_OK;
}
