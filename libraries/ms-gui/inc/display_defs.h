#pragma once

/************************************************************************************************
 * @file   display_defs.h
 *
 * @brief  Header file for display defintions
 *
 * @date   2026-03-10
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup display_defs
 * @brief    display_defs Firmware
 * @{
 */

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86) /* Framebuffer takes up too much RAM on other STMs otherwise*/
#define DISPLAY_WIDTH 480                            /**< Width of the display */
#define DISPLAY_HEIGHT 272                           /**< Height of the display */
#else
#define DISPLAY_WIDTH 1  /**< Width of the display */
#define DISPLAY_HEIGHT 1 /**< Height of the display */
#endif

/* Speedometer widget min and max vals */
#define SPEEDOMETER_MIN_VALUE 0
#define SPEEDOMETER_MAX_VALUE 120

/* Bar widget min and max vals */
#define BAR_MIN_VALUE 0
#define BAR_MAX_VALUE 100

/* Generic max number of characters */
#define LABEL_MAX_CHARS 40

/* For menu */
#define GUI_MENU_ITEM_COUNT 5U
#define GUI_MENU_PANEL_WIDTH 240
#define GUI_MENU_PANEL_HEIGHT 180
#define GUI_MENU_ROW_WIDTH 188
#define GUI_MENU_ROW_HEIGHT 24

/* For pack monitoring screen*/
#define NUMBER_OF_CELLS 36U
#define PACK_TABLE_ROWS 6U
#define PACK_TABLE_COLS 6U
#define PACK_TABLE_COL_W 51
#define PACK_CELL_TEXT_LEN 16U

/** @} */
