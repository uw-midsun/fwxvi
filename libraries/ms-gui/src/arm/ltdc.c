/************************************************************************************************
 * @file   ltdc.c
 *
 * @brief  Source file for the LTDC driver on arm
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "stm32l4xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_ltdc.h"
#include "stm32l4xx_hal_rcc.h"

/* Intra-component Headers */
#include "clut.h"
#include "gpio.h"
#include "ltdc.h"

#ifdef STM32L4P5xx

static LtdcSettings *s_ltdc_settings;
static LTDC_HandleTypeDef s_ltdc_handle;

/**
 * @brief   Configure GPIO pins for LTDC
 */
static StatusCode s_configure_gpio(LtdcGpioConfig *config) {
  if (config == NULL) {
    return STATUS_CODE_OK; /* Skip if no GPIO config provided */
  }

  /* Note: GPIO alternate function for LTDC is typically AF14 on STM32L4R5 */
  const GpioAlternateFunctions ltdc_af = 0x0EU; /* AF14 */

  /* Configure control signals */
  status_ok_or_return(gpio_init_pin_af(&config->clk, GPIO_ALTFN_PUSH_PULL, ltdc_af));
  status_ok_or_return(gpio_init_pin_af(&config->hsync, GPIO_ALTFN_PUSH_PULL, ltdc_af));
  status_ok_or_return(gpio_init_pin_af(&config->vsync, GPIO_ALTFN_PUSH_PULL, ltdc_af));
  status_ok_or_return(gpio_init_pin_af(&config->de, GPIO_ALTFN_PUSH_PULL, ltdc_af));

  /* Configure data pins */
  for (uint8_t i = 0; i < config->num_red_bits; i++) {
    status_ok_or_return(gpio_init_pin_af(&config->r[i], GPIO_ALTFN_PUSH_PULL, ltdc_af));
  }

  for (uint8_t i = 0; i < config->num_green_bits; i++) {
    status_ok_or_return(gpio_init_pin_af(&config->g[i], GPIO_ALTFN_PUSH_PULL, ltdc_af));
  }

  for (uint8_t i = 0; i < config->num_blue_bits; i++) {
    status_ok_or_return(gpio_init_pin_af(&config->b[i], GPIO_ALTFN_PUSH_PULL, ltdc_af));
  }

  return STATUS_CODE_OK;
}

/**
 * @brief   Initialize LTDC peripheral and clocks
 */
static StatusCode s_init_ltdc_peripheral(void) {
  /* Enable LTDC clock */
  __HAL_RCC_LTDC_CLK_ENABLE();

  /* Configure LTDC handle with timing from settings */
  s_ltdc_handle.Instance = LTDC;

  /* Polarity configuration for ER-TFT043A3-3 */
  s_ltdc_handle.Init.HSPolarity = LTDC_HSPOLARITY_AL;  /* Active low */
  s_ltdc_handle.Init.VSPolarity = LTDC_VSPOLARITY_AL;  /* Active low */
  s_ltdc_handle.Init.DEPolarity = LTDC_DEPOLARITY_AL;  /* Active low */
  s_ltdc_handle.Init.PCPolarity = LTDC_PCPOLARITY_IPC; /* Input pixel clock */

  /* Timing parameters (values are -1 as per STM32 convention) */
  s_ltdc_handle.Init.HorizontalSync = s_ltdc_settings->timing.hsync - 1;
  s_ltdc_handle.Init.VerticalSync = s_ltdc_settings->timing.vsync - 1;

  s_ltdc_handle.Init.AccumulatedHBP = s_ltdc_settings->timing.hsync + s_ltdc_settings->timing.hbp - 1;
  s_ltdc_handle.Init.AccumulatedVBP = s_ltdc_settings->timing.vsync + s_ltdc_settings->timing.vbp - 1;

  s_ltdc_handle.Init.AccumulatedActiveW = s_ltdc_settings->timing.hsync + s_ltdc_settings->timing.hbp + s_ltdc_settings->width - 1;
  s_ltdc_handle.Init.AccumulatedActiveH = s_ltdc_settings->timing.vsync + s_ltdc_settings->timing.vbp + s_ltdc_settings->height - 1;

  s_ltdc_handle.Init.TotalWidth = s_ltdc_settings->timing.hsync + s_ltdc_settings->timing.hbp + s_ltdc_settings->width + s_ltdc_settings->timing.hfp - 1;
  s_ltdc_handle.Init.TotalHeigh = s_ltdc_settings->timing.vsync + s_ltdc_settings->timing.vbp + s_ltdc_settings->height + s_ltdc_settings->timing.vfp - 1;

  /* Background color (black) */
  s_ltdc_handle.Init.Backcolor.Blue = 0;
  s_ltdc_handle.Init.Backcolor.Green = 0;
  s_ltdc_handle.Init.Backcolor.Red = 0;

  /* Initialize LTDC peripheral */
  if (HAL_LTDC_Init(&s_ltdc_handle) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

/**
 * @brief   Configure LTDC layer for 8bpp indexed color
 */
static StatusCode s_configure_layer(void) {
  LTDC_LayerCfgTypeDef layer_cfg = { 0 };

  layer_cfg.WindowX0 = 0;
  layer_cfg.WindowX1 = s_ltdc_settings->width;
  layer_cfg.WindowY0 = 0;
  layer_cfg.WindowY1 = s_ltdc_settings->height;

  layer_cfg.PixelFormat = LTDC_PIXEL_FORMAT_L8; /* 8-bit indexed color */
  layer_cfg.FBStartAdress = (uint32_t)s_ltdc_settings->framebuffer;
  layer_cfg.Alpha = 255;
  layer_cfg.Alpha0 = 0;
  layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  layer_cfg.ImageWidth = s_ltdc_settings->width;
  layer_cfg.ImageHeight = s_ltdc_settings->height;
  layer_cfg.Backcolor.Blue = 0;
  layer_cfg.Backcolor.Green = 0;
  layer_cfg.Backcolor.Red = 0;

  if (HAL_LTDC_ConfigLayer(&s_ltdc_handle, &layer_cfg, 0) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

/**
 * @brief   Load CLUT into LTDC hardware
 */
static StatusCode s_load_clut(void) {
  if (s_ltdc_settings->clut == NULL || s_ltdc_settings->clut_size == 0U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_ltdc_settings->clut_size > NUM_COLOR_INDICES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Cast ClutEntry array directly. Assuming struct layout matches hardware expectation */
  uint32_t *clut_data = (uint32_t *)s_ltdc_settings->clut;

  if (HAL_LTDC_ConfigCLUT(&s_ltdc_handle, clut_data, s_ltdc_settings->clut_size, 0) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  if (HAL_LTDC_EnableCLUT(&s_ltdc_handle, 0) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode ltdc_init(LtdcSettings *settings) {
  if (settings == NULL || settings->framebuffer == NULL || settings->clut == NULL || settings->clut_size == 0) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (settings->width == 0 || settings->height == 0) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_ltdc_settings = settings;

  /* Configure GPIO pins */
  status_ok_or_return(s_configure_gpio(&settings->gpio_config));

  /* Initialize LTDC peripheral */
  status_ok_or_return(s_init_ltdc_peripheral());

  /* Configure LTDC Layer 0 for 8-bit indexed framebuffer */
  status_ok_or_return(s_configure_layer());

  /* Load CLUT into LTDC */
  status_ok_or_return(s_load_clut());

  return STATUS_CODE_OK;
}

StatusCode ltdc_draw(void) {
  if (HAL_LTDC_Reload(&s_ltdc_handle, LTDC_RELOAD_VERTICAL_BLANKING) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode ltdc_set_pixel(uint16_t x, uint16_t y, ColorIndex color_index) {
  if (s_ltdc_settings == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (x >= s_ltdc_settings->width || y >= s_ltdc_settings->height) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint32_t offset = (y * s_ltdc_settings->width) + x;
  s_ltdc_settings->framebuffer[offset] = (uint8_t)color_index;

  return STATUS_CODE_OK;
}

#else

StatusCode ltdc_init(LtdcSettings *settings) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltdc_draw(void) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltdc_set_pixel(uint16_t x, uint16_t y, ColorIndex color_index) {
  return STATUS_CODE_UNIMPLEMENTED;
}

#endif
