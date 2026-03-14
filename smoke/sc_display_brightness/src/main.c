/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for display_brightness
 *
 * @date   2026-01-24
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "ltdc.h"
#include "mcu.h"
#include "pwm.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */
#include "display.h"
#include "steering_hw_defs.h"

#define BRIGHTNESS_TIMER PWM_TIMER_2
#define BRIGHTNESS_CHANNEL PWM_CHANNEL_2
#define BRIGHTNESS_GPIO_ALTFN GPIO_ALT1_TIM2

#define NUMBER_OF_RED_BITS 8
#define NUMBER_OF_GREEN_BITS 8
#define NUMBER_OF_BLUE_BITS 8

static uint8_t framebuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT] __attribute__((aligned(32)));
static GpioAddress s_display_ctrl = GPIO_STEERING_DISPLAY_CTRL;
static GpioAddress s_display_pwm = GPIO_STEERING_BACKLIGHT;
static LtdcSettings settings = { 0 };

StatusCode ltdc_display_init() {
  // From: https://www.buydisplay.com/download/ic/ST7282.pdf
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

StatusCode current_pwm_init() {
  /* configure GPIO pin */
  status_ok_or_return(gpio_init_pin_af(&s_display_pwm, BRIGHTNESS_GPIO_ALTFN, BRIGHTNESS_TIMER));

  /* initialize PWM timer with a period */
  /* 25kHz from datasheet, 1/25000 = 40 microseconds*/
  pwm_init(BRIGHTNESS_TIMER, 10);

  /* set initial brightnes to 50% */
  pwm_set_dc(BRIGHTNESS_TIMER, 50, BRIGHTNESS_CHANNEL, false);
  return STATUS_CODE_OK;
}

TASK(display_brightness, TASK_STACK_1024) {
  StatusCode status = ltdc_display_init();
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("LTDC init failed: %d", status);
    delay_ms(1000U);
    return;
  }

  StatusCode ret = current_pwm_init();

  if (ret != STATUS_CODE_OK) {
    LOG_DEBUG("brightness_pwm could not be initialized");
    delay_ms(500U);
    return;
  }

  bool increasing_duty_cycle = true;
  uint8_t percent = 0;

  while (true) {
    if (increasing_duty_cycle) {
      /* Slowly brighten display */
      while (percent < 100) {
        ++percent;
        ret = display_set_brightness(percent);
        delay_ms(10);
      }
      increasing_duty_cycle = false;
    } else {
      /* Slowly Dim display */
      while (percent > 0) {
        --percent;
        ret = display_set_brightness(percent);
        delay_ms(10);
      }
      increasing_duty_cycle = true;
    }

    if (ret != STATUS_CODE_OK) {
      LOG_DEBUG("Error setting brightness");
      return;
    }
  }
}

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

  tasks_init_task(display_brightness, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
