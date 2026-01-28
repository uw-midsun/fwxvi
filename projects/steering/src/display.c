/************************************************************************************************
 * @file   display.c
 *
 * @brief  Source file for display control
 *
 * @date   2025-07-22
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <string.h>

/* Inter-component Headers */
#include "clut.h"
#include "gpio.h"
#include "ltdc.h"
#include "pwm.h"

/* Intra-component Headers */
#include "display.h"
#include "steering_getters.h"
#include "steering_hw_defs.h"

static SteeringStorage *steering_storage = NULL;
static DisplayData *display_data = NULL;

/* Enable display when high */
static GpioAddress s_display_ctrl = GPIO_STEERING_DISPLAY_CTRL;
static LtdcSettings settings = { 0 };
static uint8_t framebuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT] __attribute__((aligned(32)));

#define NUMBER_OF_RED_BITS 8
#define NUMBER_OF_GREEN_BITS 8
#define NUMBER_OF_BLUE_BITS 8

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

  return ltdc_init(&settings);
}

StatusCode display_rx_slow() {
  return STATUS_CODE_OK;
}

StatusCode display_rx_medium() {
  display_data->brake_enabled = get_pedal_data_brake_enabled();
  display_data->regen_enabled = get_pedal_data_regen_enabled();
  display_data->pedal_percentage = (uint8_t)get_pedal_data_percentage();
  display_data->drive_state = (VehicleDriveState)get_pedal_data_drive_state();

  display_data->bps_fault = get_rear_controller_status_bps_fault();

  display_data->motor_heatsink_temp = (int16_t)get_motor_temperature_heat_sink_temp();
  display_data->motor_temp = (int16_t)get_motor_temperature_motor_temp();

  display_data->vehicle_velocity = (int16_t)get_motor_velocity_vehicle_velocity();
  display_data->motor_velocity = (int16_t)get_motor_velocity_vehicle_velocity();

  display_data->aux_voltage = get_power_input_stats_input_aux_voltage();
  display_data->aux_current = get_power_input_stats_input_aux_current();

  display_data->pack_voltage = get_battery_stats_A_pack_voltage();
  display_data->pack_current = get_battery_stats_A_pack_current();
  display_data->state_of_charge = get_battery_stats_A_pack_soc();

  return STATUS_CODE_OK;
}

StatusCode display_rx_fast() {
  return STATUS_CODE_OK;
}
