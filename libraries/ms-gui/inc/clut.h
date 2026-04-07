#pragma once

/************************************************************************************************
 * @file   clut.h
 *
 * @brief  Header file for the CLUT
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

/**
 * @brief   CLUT color entry
 */
typedef struct {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
} ClutEntry;

#define CLUT_RGB565_RED_SHIFT 11U
#define CLUT_RGB565_GREEN_SHIFT 5U

/**
 * @brief   Convert a color entry to RGB565
 * @param   color Input color entry
 * @return  RGB565 packed pixel value
 */
static inline uint16_t clut_entry_rgb565(ClutEntry color) {
  uint16_t red = ((uint16_t)color.red >> 3U) << CLUT_RGB565_RED_SHIFT;
  uint16_t green = ((uint16_t)color.green >> 2U) << CLUT_RGB565_GREEN_SHIFT;
  uint16_t blue = ((uint16_t)color.blue >> 3U);

  return (uint16_t)(red | green | blue);
}

/**
 * @brief   Legacy color indices used by indexed LTDC helpers and tests (Not used by LVGL)
 */
typedef enum {
  COLOR_INDEX_BLACK = 0,
  COLOR_INDEX_WHITE,
  COLOR_INDEX_RED,
  COLOR_INDEX_GREEN,
  COLOR_INDEX_BLUE,
  COLOR_INDEX_YELLOW,
  COLOR_INDEX_CYAN,
  COLOR_INDEX_MAGENTA,

  COLOR_INDEX_TRANSPARENT = 255,

  NUM_COLOR_INDICES = 256
} ColorIndex;

/**
 * @brief   Semantic GUI palette IDs for the LVGL widget layer
 */
typedef enum {
  GUI_COLOR_SCREEN_BACKGROUND = 0,
  GUI_COLOR_TEXT_PRIMARY,
  GUI_COLOR_SPEEDOMETER_TICK_MAJOR,
  GUI_COLOR_SPEEDOMETER_TICK_MINOR,
  GUI_COLOR_SPEEDOMETER_NEEDLE,
  GUI_COLOR_BAR_BACKGROUND,
  GUI_COLOR_BAR_BORDER,
  GUI_COLOR_THROTTLE_FILL,
  GUI_COLOR_BRAKE_FILL,
  GUI_COLOR_SOC_FILL,
  GUI_COLOR_LABEL_BORDER,

  NUM_GUI_COLOR_IDS,
} GuiColorId;

/**
 * @brief   Get a semantic GUI palette color
 * @param   color_id Semantic GUI color ID
 * @return  Palette entry for the requested GUI color
 */
ClutEntry clut_get_gui_color(GuiColorId color_id);

/**
 * @brief   Get the default indexed CLUT table
 * @return  Pointer to the default CLUT table
 */
ClutEntry *clut_get_table(void);

/**
 * @brief   Return the red component of a color entry
 */
static inline uint8_t clut_entry_red(ClutEntry color) {
  return color.red;
}

/**
 * @brief   Return the green component of a color entry
 */
static inline uint8_t clut_entry_green(ClutEntry color) {
  return color.green;
}

/**
 * @brief   Return the blue component of a color entry
 */
static inline uint8_t clut_entry_blue(ClutEntry color) {
  return color.blue;
}

/** @} */
