/************************************************************************************************
 * @file   clut.c
 *
 * @brief  Source file for the CLUT
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdlib.h>

/* Inter-component Headers */
#include "clut.h"

/* Intra-component Headers */

static ClutEntry s_clut_table[NUM_COLOR_INDICES] = {
  [COLOR_INDEX_BLACK] = { .red = 0x00U, .green = 0x00U, .blue = 0x00U },       [COLOR_INDEX_WHITE] = { .red = 0xFFU, .green = 0xFFU, .blue = 0xFFU },
  [COLOR_INDEX_RED] = { .red = 0xFFU, .green = 0x00U, .blue = 0x00U },         [COLOR_INDEX_GREEN] = { .red = 0x00U, .green = 0xFFU, .blue = 0x00U },
  [COLOR_INDEX_BLUE] = { .red = 0x00U, .green = 0x00U, .blue = 0xFFU },        [COLOR_INDEX_YELLOW] = { .red = 0xFFU, .green = 0xFFU, .blue = 0x00U },
  [COLOR_INDEX_CYAN] = { .red = 0x00U, .green = 0xFFU, .blue = 0xFFU },        [COLOR_INDEX_MAGENTA] = { .red = 0xFFU, .green = 0x00U, .blue = 0xFFU },
  [COLOR_INDEX_TRANSPARENT] = { .red = 0x00U, .green = 0x00U, .blue = 0x00U },
};

static const ClutEntry s_gui_color_table[NUM_GUI_COLOR_IDS] = {
  [GUI_COLOR_SCREEN_BACKGROUND] = { .red = 0x08U, .green = 0x0CU, .blue = 0x12U },
  [GUI_COLOR_TEXT_PRIMARY] = { .red = 0xF4U, .green = 0xF7U, .blue = 0xFBU },
  [GUI_COLOR_SPEEDOMETER_TICK_MAJOR] = { .red = 0xF4U, .green = 0xF7U, .blue = 0xFBU },
  [GUI_COLOR_SPEEDOMETER_TICK_MINOR] = { .red = 0x59U, .green = 0x66U, .blue = 0x74U },
  [GUI_COLOR_SPEEDOMETER_NEEDLE] = { .red = 0xFFU, .green = 0x6BU, .blue = 0x6BU },
  [GUI_COLOR_BAR_BACKGROUND] = { .red = 0x17U, .green = 0x20U, .blue = 0x29U },
  [GUI_COLOR_BAR_BORDER] = { .red = 0xD9U, .green = 0xE2U, .blue = 0xECU },
  [GUI_COLOR_THROTTLE_FILL] = { .red = 0x00U, .green = 0xC8U, .blue = 0x53U },
  [GUI_COLOR_BRAKE_FILL] = { .red = 0xFFU, .green = 0x70U, .blue = 0x43U },
  [GUI_COLOR_SOC_FILL] = { .red = 0x26U, .green = 0xC6U, .blue = 0xDAU },
  [GUI_COLOR_LABEL_BORDER] = { .red = 0xFFU, .green = 0xFFU, .blue = 0xFFU }
};

ClutEntry *clut_get_table(void) {
  return s_clut_table;
}

ClutEntry clut_get_gui_color(GuiColorId color_id) {
  if (color_id >= NUM_GUI_COLOR_IDS) {
    return s_gui_color_table[GUI_COLOR_SCREEN_BACKGROUND];
  }

  return s_gui_color_table[color_id];
}
