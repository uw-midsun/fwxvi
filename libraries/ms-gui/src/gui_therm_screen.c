/************************************************************************************************
 * @file   gui_therm_screen.c
 *
 * @brief  High-level thermistor monitor screen implementation
 *
 * @date   2026-07-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

#include "status.h"

/* Inter-component Headers */
#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl.h"
#endif

/* Intra-component Headers */
#include "clut.h"
#include "display_defs.h"
#include "gui_therm_screen.h"

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl_screens.h"
#include "lvgl_widgets.h"

static TableWidget s_therm_table;
static uint16_t s_thermistor_temp_c[NUMBER_OF_THERMISTORS];
static bool s_therm_widgets_initialized;

static void s_format_cell(char *buf, uint8_t idx) {
  snprintf(buf, THERM_CELL_TEXT_LEN, "T%02u\n%uC", idx + 1U, s_thermistor_temp_c[idx]);
}

static StatusCode s_create_title_label(GuiScreen *screen) {
  const LabelWidgetConfig title_config = {
    .size = { .width = DISPLAY_WIDTH, .height = 20 },
    .position = { .type = WIDGET_POSITION_ALIGN, .value.align = { .align = WIDGET_ALIGN_IN_TOP_MID, .x_offset = 0, .y_offset = 4 } },
    .label_text = "Thermistor Monitor (C)",
    .alignment = WIDGET_TEXT_ALIGN_CENTER,
    .text_color_id = GUI_COLOR_TEXT_PRIMARY,
    .font = GUI_SMALL_TEXT,
    .background_enabled = false,
    .background_color_id = 0,
    .border_enabled = false,
    .border_color_id = GUI_COLOR_LABEL_BORDER,
    .border_width = 0,
  };

  static LabelWidget s_title_label;
  return lvgl_widgets_create_label(&s_title_label, &title_config, screen);
}

static StatusCode s_create_table(GuiScreen *screen) {
  const TableWidgetConfig config = {
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_IN_TOP_MID, .x_offset = 0, .y_offset = 28 },
    },
    .row_count = THERM_TABLE_ROWS,
    .col_count = THERM_TABLE_COLS,
    .col_width = THERM_TABLE_COL_W,
    .text_color_id = GUI_COLOR_TEXT_PRIMARY,
    .font = GUI_SMALL_TEXT,
    .border_color_id = GUI_COLOR_LABEL_BORDER,
  };

  return lvgl_widgets_create_table(&s_therm_table, &config, screen);
}

StatusCode gui_therm_screen_init(GuiScreen *screen) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_therm_widgets_initialized) {
    return STATUS_CODE_ALREADY_INITIALIZED;
  }

  status_ok_or_return(lvgl_set_background_color(screen, GUI_COLOR_SCREEN_BACKGROUND));

  status_ok_or_return(s_create_title_label(screen));
  status_ok_or_return(s_create_table(screen));

  /* Initialize table with 0s */
  char buf[THERM_CELL_TEXT_LEN];
  for (uint8_t i = 0U; i < NUMBER_OF_THERMISTORS; ++i) {
    s_format_cell(buf, i);
    status_ok_or_return(lvgl_widgets_set_table_cell(&s_therm_table, i / THERM_TABLE_COLS, i % THERM_TABLE_COLS, buf));
  }

  s_therm_widgets_initialized = true;
  return STATUS_CODE_OK;
}

void gui_therm_screen_deinit(void) {
  s_therm_table = (TableWidget){ 0 };
  for (uint8_t i = 0U; i < NUMBER_OF_THERMISTORS; ++i) {
    s_thermistor_temp_c[i] = 0U;
  }
  s_therm_widgets_initialized = false;
}

StatusCode gui_therm_screen_widget_set_thermistor(uint8_t therm_idx, uint16_t thermistor_temp_c) {
  if (!s_therm_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (therm_idx >= NUMBER_OF_THERMISTORS) {
    return STATUS_CODE_OUT_OF_RANGE;
  }

  if (s_thermistor_temp_c[therm_idx] == thermistor_temp_c) {
    return STATUS_CODE_OK;
  }

  s_thermistor_temp_c[therm_idx] = thermistor_temp_c;

  char buf[THERM_CELL_TEXT_LEN];
  s_format_cell(buf, therm_idx);
  return lvgl_widgets_set_table_cell(&s_therm_table, therm_idx / THERM_TABLE_COLS, therm_idx % THERM_TABLE_COLS, buf);
}

#else

StatusCode gui_therm_screen_init(GuiScreen *screen) {
  (void)screen;
  return STATUS_CODE_OK;
}

void gui_therm_screen_deinit(void) {}

StatusCode gui_therm_screen_widget_set_thermistor(uint8_t therm_idx, uint16_t thermistor_temp_c) {
  (void)therm_idx;
  (void)thermistor_temp_c;
  return STATUS_CODE_OK;
}

#endif
