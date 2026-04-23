/************************************************************************************************
 * @file   gui_pack_screen.c
 *
 * @brief  High-level pack voltage screen implementation
 *
 * @date   2026-04-18
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
#include "gui_pack_screen.h"
#include "gui_widgets.h"
#include "display_defs.h"

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl_screens.h"
#include "lvgl_widgets.h"

static TableWidget s_pack_table;
static uint16_t s_cell_voltages[NUMBER_OF_CELLS];
static bool s_pack_widgets_initialized;

static LabelWidget s_speed_label;
static LabelWidget s_cc_label;

static void s_format_cell(char *buf, uint8_t idx) {
  snprintf(buf, PACK_CELL_TEXT_LEN, "C%02u\n%u.%03u", idx + 1U, s_cell_voltages[idx] / 10000U, (s_cell_voltages[idx] % 10000U) / 10U);
}

static StatusCode s_create_table(GuiScreen *screen) {
  const TableWidgetConfig config = {
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_IN_LEFT_MID, .x_offset = 10, .y_offset = 20 },
    },
    .row_count = PACK_TABLE_ROWS,
    .col_count = PACK_TABLE_COLS,
    .col_width = PACK_TABLE_COL_W,
    .text_color_id = GUI_COLOR_TEXT_PRIMARY,
    .font = GUI_SMALL_TEXT,
    .border_color_id = GUI_COLOR_LABEL_BORDER,
  };

  return lvgl_widgets_create_table(&s_pack_table, &config, screen);
}

static StatusCode s_create_speed_label(GuiScreen *screen) {
  const LabelWidgetConfig speed_label_config = {
    .size = { .width = 75, .height = 50 },
    .position = { .type = WIDGET_POSITION_ALIGN, .value.align = { .align = WIDGET_ALIGN_IN_TOP_LEFT, .x_offset = 0, .y_offset = 0 } },
    .label_text = "0",
    .alignment = WIDGET_TEXT_ALIGN_CENTER,
    .text_color_id = GUI_COLOR_TEXT_PRIMARY,
    .font = GUI_BIG_TEXT,
    .background_enabled = false,
    .background_color_id = 0,
    .border_enabled = false,
    .border_color_id = GUI_COLOR_LABEL_BORDER,
    .border_width = 0,
  };

  return lvgl_widgets_create_label(&s_speed_label, &speed_label_config, screen);
}

static StatusCode s_create_cc_label(GuiScreen *screen) {
  const LabelWidgetConfig cruise_control_label_config = {
    .size = { .width = 75, .height = 20 },
    .position = { .type = WIDGET_POSITION_ALIGN, .value.align = { .align = WIDGET_ALIGN_IN_TOP_LEFT, .x_offset = 0, .y_offset = 40 } },
    .label_text = "0 km/h",
    .alignment = WIDGET_TEXT_ALIGN_CENTER,
    .text_color_id = GUI_COLOR_TEXT_PRIMARY,
    .font = GUI_SMALL_TEXT,
    .background_enabled = false,
    .background_color_id = 0,
    .border_enabled = false,
    .border_color_id = GUI_COLOR_LABEL_BORDER,
    .border_width = 0,
  };

  return lvgl_widgets_create_label(&s_cc_label, &cruise_control_label_config, screen);
}

StatusCode gui_pack_screen_init(GuiScreen *screen) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_pack_widgets_initialized) {
    return STATUS_CODE_ALREADY_INITIALIZED;
  }

  status_ok_or_return(lvgl_set_background_color(screen, GUI_COLOR_SCREEN_BACKGROUND));

  status_ok_or_return(s_create_speed_label(screen));
  status_ok_or_return(s_create_cc_label(screen));

  status_ok_or_return(s_create_table(screen));
  StatusCode status = gui_widgets_init_screen(screen);
  if (status != STATUS_CODE_OK) {
    s_pack_table = (TableWidget){ 0 };
    return status;
  }

  /* Initialize table with 0s */
  char buf[PACK_CELL_TEXT_LEN];
  for (uint8_t i = 0U; i < NUMBER_OF_CELLS; ++i) {
    s_format_cell(buf, i);
    status_ok_or_return(lvgl_widgets_set_table_cell(&s_pack_table, i / PACK_TABLE_COLS, i % PACK_TABLE_COLS, buf));
  }

  s_pack_widgets_initialized = true;
  return STATUS_CODE_OK;
}

void gui_pack_screen_deinit(void) {
  gui_widgets_deinit();
  s_pack_table = (TableWidget){ 0 };
  for (uint8_t i = 0U; i < NUMBER_OF_CELLS; ++i) {
    s_cell_voltages[i] = 0U;
  }
  s_pack_widgets_initialized = false;
}

StatusCode gui_pack_screen_widget_set_pack_voltage(uint8_t cell_idx, uint16_t cell_voltage) {
  if (!s_pack_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (cell_idx >= NUMBER_OF_CELLS) {
    return STATUS_CODE_OUT_OF_RANGE;
  }

  if (s_cell_voltages[cell_idx] == cell_voltage) {
    return STATUS_CODE_OK;
  }

  s_cell_voltages[cell_idx] = cell_voltage;

  char buf[PACK_CELL_TEXT_LEN];
  s_format_cell(buf, cell_idx);
  return lvgl_widgets_set_table_cell(&s_pack_table, cell_idx / PACK_TABLE_COLS, cell_idx % PACK_TABLE_COLS, buf);
}

StatusCode gui_pack_screen_widget_set_speed_label(int16_t speed_kmh) {
  if (!s_pack_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  char text_buffer[3];

  snprintf(text_buffer, sizeof(text_buffer), "%u", speed_kmh);

  return lvgl_widgets_set_label_text(&s_speed_label, text_buffer);
}

StatusCode gui_pack_screen_widget_set_cc_speed(uint16_t cruise_control_speed_kmh, bool is_cc_enabled) {
  if (!s_pack_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  char text_buffer[LABEL_MAX_CHARS];
  if (is_cc_enabled) {
    snprintf(text_buffer, sizeof(text_buffer), "%u km/h", cruise_control_speed_kmh);
  } else {
    snprintf(text_buffer, sizeof(text_buffer), "cc off");
  }

  return lvgl_widgets_set_label_text(&s_cc_label, text_buffer);
}

#else

StatusCode gui_pack_screen_init(GuiScreen *screen) {
  (void)screen;
  return STATUS_CODE_OK;
}

void gui_pack_screen_deinit(void) {}

StatusCode gui_pack_screen_widget_set_pack_voltage(uint8_t cell_idx, uint16_t cell_voltage) {
  (void)cell_idx;
  (void)cell_voltage;
  return STATUS_CODE_OK;
}

StatusCode gui_pack_screen_widget_set_speed_label(int16_t speed_kmh) {
  (void)speed_kmh;
  return STATUS_CODE_OK;
}

StatusCode gui_pack_screen_widget_set_cc_speed(uint16_t cruise_control_speed_kmh, bool is_cc_enabled) {
  (void)cruise_control_speed_kmh;
  (void)is_cc_enabled;
  return STATUS_CODE_OK;
}

#endif
