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
#include "software_timer.h"
#include "steering_hw_defs.h"

static SteeringStorage *steering_storage = NULL;
static LtdcSettings s_settings = { 0 };
static uint8_t s_framebuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT] __attribute__((aligned(32))) = { 0 };

static GpioAddress s_display_ctrl = STEERING_DISPLAY_CTRL;       /* Enable display when high */
static GpioAddress s_ltdc_pwm_pin = STEERING_DISPLAY_BRIGHTNESS; /* Current control for brightness ctrl */

#define NUMBER_OF_RED_BITS 8
#define NUMBER_OF_GREEN_BITS 8
#define NUMBER_OF_BLUE_BITS 8

#define BRIGHTNESS_TIMER PWM_TIMER_2
#define BRIGHTNESS_CHANNEL PWM_CHANNEL_2
#define BRIGHTNESS_GPIO_ALTFN GPIO_ALT1_TIM2

/** @brief Initialize PWM */
static StatusCode s_current_pwm_init() {
  /* configure GPIO pin */
  status_ok_or_return(gpio_init_pin_af(&s_ltdc_pwm_pin, BRIGHTNESS_GPIO_ALTFN, BRIGHTNESS_TIMER));

  /* initialize PWM timer with a period */
  /* 25kHz from datasheet, 1/25000 = 40 microseconds*/
  pwm_init(BRIGHTNESS_TIMER, 10);

  /* set initial brightnes to 50% */
  pwm_set_dc(BRIGHTNESS_TIMER, 50, BRIGHTNESS_CHANNEL, false);
  return STATUS_CODE_OK;
}

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
  s_settings.width = DISPLAY_WIDTH;
  s_settings.height = DISPLAY_HEIGHT;
  s_settings.framebuffer = s_framebuffer;
  s_settings.clut = clut_get_table();
  s_settings.clut_size = NUM_COLOR_INDICES;
  s_settings.timing = timing_config;
  s_settings.gpio_config = gpio_config;

  gpio_init_pin(&s_display_ctrl, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  s_current_pwm_init();

  return ltdc_init(&s_settings);
}

StatusCode display_set_brightness(uint8_t percentage) {
  return pwm_set_dc(BRIGHTNESS_TIMER, percentage, BRIGHTNESS_CHANNEL, false);
}
