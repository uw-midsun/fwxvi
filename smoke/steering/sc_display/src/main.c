/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for sc_display (Cycles through different checkerboard colors)
 *
 * @date   2026-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "clut.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "ltdc.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"
/* Intra-component Headers */
#include "display.h"
#include "steering_hw_defs.h"

#ifdef STM32L4P5xx         /* Framebuffer takes up too much RAM on other STMs otherwise*/
#define DISPLAY_WIDTH 480  /**< Width of the display */
#define DISPLAY_HEIGHT 272 /**< Height of the display */
#else
#define DISPLAY_WIDTH 1  /**< Width of the display */
#define DISPLAY_HEIGHT 1 /**< Height of the display */
#endif

#define NUMBER_OF_RED_BITS 8
#define NUMBER_OF_GREEN_BITS 8
#define NUMBER_OF_BLUE_BITS 8

static uint8_t framebuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT * 2] __attribute__((aligned(32)));
static GpioAddress s_display_ctrl = GPIO_STEERING_DISPLAY_CTRL;
static GpioAddress s_display_pwm = GPIO_STEERING_BACKLIGHT;
static LtdcSettings settings = { 0 };

StatusCode ltdc_display_init() {
  // From: https://www.buydisplay.com/download/ic/ST7282.pdf
  // TODO move values to macros
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
  return ltdc_init(&settings);
}

StatusCode draw_checkerboard(ColorIndex color1, ColorIndex color2, uint16_t square_size) {
  for (uint16_t y = 0; y < DISPLAY_HEIGHT; y++) {
    for (uint16_t x = 0; x < DISPLAY_WIDTH; x++) {
      uint16_t square_x = x / square_size;
      uint16_t square_y = y / square_size;
      ColorIndex color = ((square_x + square_y) % 2 == 0) ? color1 : color2;
      status_ok_or_return(ltdc_set_pixel(x, y, color));
    }
  }
  /* Apply changes */
  return ltdc_draw();
}

TASK(sc_display, TASK_STACK_1024) {
  StatusCode status = ltdc_display_init();
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("LTDC init failed: %d", status);
    delay_ms(1000U);
    return;
  }
  status = draw_checkerboard(COLOR_INDEX_YELLOW, COLOR_INDEX_BLUE, 16);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Draw failed: %d", status);
    delay_ms(1000U);
  }

  while (true) {
    delay_ms(1000);
    LOG_DEBUG("I'm alive");
    status = draw_checkerboard(COLOR_INDEX_BLACK, COLOR_INDEX_WHITE, 16);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Draw failed: %d", status);
      delay_ms(1000U);
    }
    status = draw_checkerboard(COLOR_INDEX_YELLOW, COLOR_INDEX_BLUE, 16);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Draw failed: %d", status);
      delay_ms(1000U);
    }
    status = draw_checkerboard(COLOR_INDEX_WHITE, COLOR_INDEX_RED, 16);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Draw failed: %d", status);
      delay_ms(1000U);
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

  tasks_init_task(sc_display, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
