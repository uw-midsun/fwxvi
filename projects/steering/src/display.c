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
#include "gui_menu.h"
#include "gui_widgets.h"
#include "ltdc.h"
#include "party_mode.h"
#include "pwm.h"
#include "status.h"
#include "tasks.h"

#ifdef MS_PLATFORM_X86
#include <SDL2/SDL.h>
#endif

/* Intra-component Headers */
#include "display.h"
#include "steering_getters.h"
#include "steering_hw_defs.h"

static SteeringStorage *steering_storage = NULL;
static DisplayData *display_data = NULL;

/* Enable display when high */
static GpioAddress s_display_ctrl = GPIO_STEERING_DISPLAY_CTRL;
static GpioAddress s_display_pwm = GPIO_STEERING_BACKLIGHT;
static LtdcSettings settings = { 0 };
static uint8_t framebuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT * 2] __attribute__((aligned(32))); /* RGB565 */

#define NUMBER_OF_RED_BITS 8
#define NUMBER_OF_GREEN_BITS 8
#define NUMBER_OF_BLUE_BITS 8

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

  if (menu_status == STATUS_CODE_INVALID_ARGS) {
    buzzer_play_invalid();
  } else if (menu_status != STATUS_CODE_OK) {
    LOG_DEBUG("gui menu input failed: %u\r\n", menu_status);
  }
}

static StatusCode s_render_gui_step(void) {
  status_ok_or_return(gui_widgets_set_speed(display_data->vehicle_velocity));
  status_ok_or_return(gui_widgets_set_throttle_bar(display_data->pedal_percentage));
  status_ok_or_return(gui_widgets_set_brake_bar(display_data->brake_enabled ? 100 : 0));  // TODO change to % base when available
  status_ok_or_return(gui_widgets_set_brake_bar(display_data->brake_percentage));
  if (steering_storage->display_data.drive_state == VEHICLE_DRIVE_STATE_REGEN) {
    status_ok_or_return(gui_widgets_set_brake_bar_color(GUI_COLOR_REGEN_BRAKE_FILL));
  } else {
    status_ok_or_return(gui_widgets_set_brake_bar_color(GUI_COLOR_BRAKE_FILL));
  }
  status_ok_or_return(gui_widgets_set_top_label(display_data->pack_voltage, display_data->motor_bus_voltage, display_data->bps_fault, display_data->bps_fault_cell));
  status_ok_or_return(gui_widgets_set_cell_stats_label(display_data->min_cell_voltage_mv, display_data->max_cell_voltage_mv));
  status_ok_or_return(gui_widgets_set_temps_stats_label(display_data->motor_temp, display_data->max_cell_temp));
  status_ok_or_return(gui_widgets_set_soc_bar(display_data->state_of_charge));
  status_ok_or_return(gui_widgets_set_cc_speed(steering_storage->cruise_control_target_speed_kmh, steering_storage->cruise_control_enabled));

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
  status_ok_or_return(gui_menu_set_party_mode_callback(party_mode_toggle));

  status_ok_or_return(tasks_init_task(display_lvgl_task, TASK_PRIORITY(2), NULL));

  LOG_DEBUG("LVGL display initialized\r\n");
#endif
  return STATUS_CODE_OK;
}

StatusCode display_run() {
  return STATUS_CODE_OK;
}

StatusCode display_rx_slow() {
  return STATUS_CODE_OK;
}

StatusCode display_rx_medium() {
  display_data->precharge_complete = get_rear_controller_status_triggers_motor_precharge_complete();
  display_data->brake_enabled = get_drive_status_state_data_brake_enabled();
  display_data->regen_enabled = get_drive_status_state_data_regen_enabled();
  display_data->pedal_percentage = (uint8_t)get_drive_status_pedal_percentage();
  display_data->brake_percentage = (uint8_t)get_drive_status_brake_percentage();
  display_data->drive_state = (VehicleDriveState)get_drive_status_state_data_drive_state();

  display_data->bps_fault = get_rear_controller_status_triggers_bps_fault();
  display_data->bps_fault_cell = get_rear_controller_status_triggers_cell_at_fault();

  display_data->motor_heatsink_temp = (int16_t)get_motor_stats_B_heat_sink_temp();
  display_data->motor_temp = (int16_t)get_motor_stats_B_motor_temp();

  display_data->motor_bus_voltage = (uint16_t)get_motor_stats_A_bus_voltage();
  display_data->vehicle_velocity = (int16_t)get_motor_stats_B_vehicle_velocity();
  display_data->motor_velocity = (int16_t)get_motor_stats_B_motor_velocity();

  display_data->aux_voltage = (int16_t)get_power_input_stats_input_aux_voltage();
  display_data->aux_current = (int16_t)get_power_input_stats_input_aux_current();

  display_data->pack_voltage = (uint16_t)get_battery_stats_A_pack_voltage();
  display_data->pack_current = (int16_t)get_battery_stats_A_pack_current();
  display_data->min_cell_voltage_mv = (uint16_t)get_battery_stats_B_min_cell_voltage();
  display_data->max_cell_voltage_mv = (uint16_t)get_battery_stats_B_max_cell_voltage();
  display_data->max_cell_temp = (uint16_t)get_battery_stats_B_max_temperature();
  display_data->state_of_charge = (float)((uint16_t)get_battery_stats_A_pack_soc() / 100);

  return STATUS_CODE_OK;
}

StatusCode display_rx_fast() {
  return STATUS_CODE_OK;
}
