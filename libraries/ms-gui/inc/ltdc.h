#pragma once

/************************************************************************************************
 * @file   ltdc.h
 *
 * @brief  Header file for the platform-agnostic LTDC API
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "gpio.h"
#include "status.h"

/* Intra-component Headers */
#include "clut.h"
#include "framebuffer.h"

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

/**
 * @brief LTDC timing configuration
 */
typedef struct {
  uint16_t hsync; /**< Horizontal sync width */
  uint16_t vsync; /**< Vertical sync width */
  uint16_t hbp;   /**< Horizontal back porch */
  uint16_t vbp;   /**< Vertical back porch */
  uint16_t hfp;   /**< Horizontal front porch */
  uint16_t vfp;   /**< Vertical front porch */
} LtdcTimingConfig;

/**
 * @brief LTDC GPIO pin configuration
 */
typedef struct {
  GpioAddress clk;        /**< LCD Clock pin */
  GpioAddress hsync;      /**< Horizontal sync pin */
  GpioAddress vsync;      /**< Vertical sync pin */
  GpioAddress de;         /**< Data enable pin */
  GpioAddress r[8U];      /**< Red data pins (R0-R7) */
  GpioAddress g[8U];      /**< Green data pins (G0-G7) */
  GpioAddress b[8U];      /**< Blue data pins (B0-B7) */
  uint8_t num_red_bits;   /**< Number of red bits to use */
  uint8_t num_green_bits; /**< Number of green bits to use */
  uint8_t num_blue_bits;  /**< Number of blue bits to use */
} LtdcGpioConfig;

/**
 * @brief LTDC configuration structure
 */
typedef struct {
  uint16_t width;             /**< Display width in pixels */
  uint16_t height;            /**< Display height in pixels */
  uint8_t *framebuffer;       /**< Pointer to the framebuffer (8-bit indexed) */
  ClutEntry *clut;            /**< Pointer to the CLUT entries */
  uint16_t clut_size;         /**< Number of CLUT entries */
  LtdcTimingConfig timing;    /**< LTDC timing parameters */
  LtdcGpioConfig gpio_config; /**< GPIO configuration */
} LtdcSettings;

/**
 * @brief   Initialize LTDC with settings
 * @param   settings Pointer to the LTDC settings
 * @return  STATUS_CODE_OK on success, error code otherwise
 */
StatusCode ltdc_init(LtdcSettings *settings);

/**
 * @brief   Refresh the framebuffer on the display
 * @details This should trigger a framebuffer update or refresh the layer
 * @return  STATUS_CODE_OK on success, error code otherwise
 */
StatusCode ltdc_draw(void);

/**
 * @brief   Set pixel in the framebuffer (8-bit index into CLUT)
 * @param   x X coordinate
 * @param   y Y coordinate
 * @param   color_index CLUT color index
 * @return  STATUS_CODE_OK on success, error code otherwise
 */
StatusCode ltdc_set_pixel(uint16_t x, uint16_t y, ColorIndex color_index);

#ifdef MS_PLATFORM_X86

/**
 * @brief   Get the SDL renderer for advanced rendering operations
 * @details Allows direct access to SDL renderer for custom drawing or debugging
 * @return  Pointer to SDL_Renderer (cast from void*)
 */
void *ltdc_get_renderer(void);

/**
 * @brief   Save current framebuffer to a BMP file
 * @details Captures the current display state for visual regression testing or debugging
 * @param   filename Path to output BMP file (e.g., "test_output/frame.bmp")
 * @note    Creates directory if it doesn't exist
 */
void save_ltdc_frame(const char *filename);

/**
 * @brief   Process SDL events (keyboard, mouse, window close)
 * @details Call this in your main loop to handle user input and window events
 * @return  true if application should continue, false if user closed window
 */
bool ltdc_process_events(void);

/**
 * @brief   Delay to maintain target frame rate
 * @details Calculates and applies appropriate delay to achieve consistent FPS
 * @param   fps Target frames per second (e.g., 30, 60)
 */
void ltdc_delay_frame(uint32_t fps);

/**
 * @brief   Cleanup LTDC simulation resources
 * @details Destroys SDL window, renderer, and quits SDL subsystem
 * @note    Call this before program exit to prevent resource leaks
 */
void ltdc_cleanup(void);

#endif

/** @} */
