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

/* Inter-component Headers */
#include "gpio.h"
#include "pwm.h"
#include "ltdc.h"
#include "clut.h" 

/* Intra-component Headers */
#include "display.h"
#include "steering_hw_defs.h"

static SteeringStorage *steering_storage = NULL;

/* Enable display when high */
static GpioAddress s_display_ctrl = STEERING_DISPLAY_CTRL;
static LtdcSettings settings = { 0 };
static uint8_t framebuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT] __attribute__((aligned(32)));

#define NUMBER_OF_RED_BITS   8
#define NUMBER_OF_GREEN_BITS 8
#define NUMBER_OF_BLUE_BITS  8

StatusCode display_init(SteeringStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  steering_storage = storage;

  LtdcTimingConfig timing_config = {
    .hsync = HORIZONTAL_SYNC_WIDTH, 
    .vsync = VERTICAL_SYNC_WIDTH, 
    .hbp = HORIZONTAL_BACK_PORCH, 
    .vbp = VERTICAL_BACK_PORCH, 
    .hfp = HORIZONTAL_FRONT_PORCH, 
    .vfp = VERTICAL_FRONT_PORCH 
  }

  LtdcGpioConfig gpio_config = { 
    .clk = STEERING_DISPLAY_LTDC_CLOCK, 
    .hsync = STEERING_DISPLAY_LTDC_HSYNC, 
    .vsync = STEERING_DISPLAY_LTDC_VSYNC, 
    .de = STEERING_DISPLAY_LTDC_DE, 
    .r = STEERING_DISPLAY_LTDC_RED_PINS, 
    .g = STEERING_DISPLAY_LTDC_GREEN_PINS, 
    .b = STEERING_DISPLAY_LTDC_BLUE_PINS, 
    .num_red_bits = NUMBER_OF_RED_BITS,
    .num_green_bits = NUMBER_OF_GREEN_BITS,
    .num_blue_bits = NUMBER_OF_BLUE_BITS
  };
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
