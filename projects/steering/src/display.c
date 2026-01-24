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
#include "clut.h"
#include "gpio.h"
#include "ltdc.h"
#include "pwm.h"

/* Intra-component Headers */
#include "display.h"
#include "steering_hw_defs.h"
#include "software_timer.h"


static SteeringStorage *steering_storage = NULL;

/* Enable display when high */
static GpioAddress s_display_ctrl = STEERING_DISPLAY_CTRL;
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

  LtdcTimingConfig timing_config = {
    .hsync = HORIZONTAL_SYNC_WIDTH, .vsync = VERTICAL_SYNC_WIDTH, .hbp = HORIZONTAL_BACK_PORCH, .vbp = VERTICAL_BACK_PORCH, .hfp = HORIZONTAL_FRONT_PORCH, .vfp = VERTICAL_FRONT_PORCH
  };

  LtdcGpioConfig gpio_config = { .clk = STEERING_DISPLAY_LTDC_CLOCK,
                                 .hsync = STEERING_DISPLAY_LTDC_HSYNC,
                                 .vsync = STEERING_DISPLAY_LTDC_VSYNC,
                                 .de = STEERING_DISPLAY_LTDC_DE,
                                 .r = STEERING_DISPLAY_LTDC_RED_PINS,
                                 .g = STEERING_DISPLAY_LTDC_GREEN_PINS,
                                 .b = STEERING_DISPLAY_LTDC_BLUE_PINS,
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

  setup_brightness_pwm();
  
  return ltdc_init(&settings);
}

/* control the PA1 pin to supply a current via PWM to dim/brighten the display */
/* current should not exceed 0.4A --> therefore 100% == 0.4A, and 0% == 0A */
/* create smoke test that dims and brightens display periodically */

static GpioAddress ltdc_pwm_pin = STEERING_DISPLAY_BRIGHTNESS; 
#define BRIGHTNESS_TIMER GPIO_ALT1_TIM2
#define BRIGHTNESS_CHANNEL PWM_CHANNEL_2

static void setup_brightness_pwm() {
  /* configure GPIO pin */
  status_ok_or_return(gpio_init_pin_af(&ltdc_pwm_pin, GPIO_ALTFN_PUSH_PULL, BRIGHTNESS_TIMER));

  /* initialize PWM timer with a period */
  /* 25kHz from datasheet, 1/25000 = 40 microseconds*/
  pwm_init(BRIGHTNESS_TIMER, 10); 

  /* set initial brightnes to 50% */
  pwm_set_dc(BRIGHTNESS_TIMER, 50, BRIGHTNESS_CHANNEL, false);
}

static void display_set_brightness(uint8_t percentage) {
  pwm_set_dc(BRIGHTNESS_TIMER, percentage, BRIGHTNESS_CHANNEL, false);
}

