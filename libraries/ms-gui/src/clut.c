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
#include "ltdc.h"

/* Intra-component Headers */

static ClutEntry s_clut_table[NUM_COLOR_INDICES] = {
  [COLOR_INDEX_BLACK] = { .red = 0x00U, .green = 0x00U, .blue = 0x00U, .reserved = 0x00U },
  [COLOR_INDEX_WHITE] = { .red = 0xFFU, .green = 0xFFU, .blue = 0xFFU, .reserved = 0x00U },
  [COLOR_INDEX_RED] = { .red = 0xFFU, .green = 0x00U, .blue = 0x00U, .reserved = 0x00U },
  [COLOR_INDEX_GREEN] = { .red = 0x00U, .green = 0xFFU, .blue = 0x00U, .reserved = 0x00U },
  [COLOR_INDEX_BLUE] = { .red = 0x00U, .green = 0x00U, .blue = 0xFFU, .reserved = 0x00U },
  [COLOR_INDEX_YELLOW] = { .red = 0xFFU, .green = 0xFFU, .blue = 0x00U, .reserved = 0x00U },
  [COLOR_INDEX_CYAN] = { .red = 0x00U, .green = 0xFFU, .blue = 0xFFU, .reserved = 0x00U },
  [COLOR_INDEX_MAGENTA] = { .red = 0xFFU, .green = 0x00U, .blue = 0xFFU, .reserved = 0x00U },
  [COLOR_INDEX_TRANSPARENT] = { .red = 0x00U, .green = 0x00U, .blue = 0x00U, .reserved = 0x00U },
};

ClutEntry *clut_get_table(void) {
  return s_clut_table;
}
