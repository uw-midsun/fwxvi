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

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

/**
 * @brief   CLUT color entry (RGB888 format)
 */
typedef struct {
  uint8_t blue;  /**< Blue component   [7:0] */
  uint8_t green; /**< Green component  [15:8] */
  uint8_t red;   /**< Red component    [23:16] */
} ClutEntry;

/**
 * @brief   Color index enumeration for CLUT entries (0–255)
 *          Each value corresponds to an index into the LTDC CLUT.
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

  // Reserved or unused slots can go here
  COLOR_INDEX_TRANSPARENT = 255,

  NUM_COLOR_INDICES = 256
} ColorIndex;

/**
 * @brief   Get pointer to the default CLUT table
 * @return  Pointer to a statically defined CLUT table
 */
ClutEntry *clut_get_table(void);

/** @} */
