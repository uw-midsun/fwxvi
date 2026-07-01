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
#include "display_defs.h"
#include "gui_pack_screen.h"
#include "gui_widgets.h"

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl_screens.h"
#include "lvgl_widgets.h"

static TableWidget s_pack_table;
static uint16_t s_cell_voltages[NUMBER_OF_CELLS];
static bool s_pack_widgets_initialized;

static LabelWidget s_speed_label;
static LabelWidget s_cc_label;

/* Cell voltage range used for the pack table gradient fill, in the same fixed-point mV units as s_cell_voltages */
#define PACK_CELL_GRADIENT_MIN_MV 25000U
#define PACK_CELL_GRADIENT_MAX_MV 45000U

static void s_format_cell(char *buf, uint8_t idx) {
  snprintf(buf, PACK_CELL_TEXT_LEN, "C%02u\n%u.%03u", idx + 1U, s_cell_voltages[idx] / 10000U, (s_cell_voltages[idx] % 10000U) / 10U);
}

/**
 * @brief   Interpolate the pack table cell fill color between the low/high gradient colors
 * @param   cell_voltage Cell voltage in the same fixed-point mV units as s_cell_voltages
 */
static lv_color_t s_cell_fill_color(uint16_t cell_voltage) {
  int32_t clamped = cell_voltage;
  if (clamped < (int32_t)PACK_CELL_GRADIENT_MIN_MV) {
    clamped = (int32_t)PACK_CELL_GRADIENT_MIN_MV;
  } else if (clamped > (int32_t)PACK_CELL_GRADIENT_MAX_MV) {
    clamped = (int32_t)PACK_CELL_GRADIENT_MAX_MV;
  }

  float fraction = (float)(clamped - (int32_t)PACK_CELL_GRADIENT_MIN_MV) / (float)(PACK_CELL_GRADIENT_MAX_MV - PACK_CELL_GRADIENT_MIN_MV);

  ClutEntry low = clut_get_gui_color(GUI_COLOR_CELL_VOLTAGE_LOW);
  ClutEntry high = clut_get_gui_color(GUI_COLOR_CELL_VOLTAGE_HIGH);

  uint8_t red = (uint8_t)((float)low.red + fraction * (float)(high.red - low.red));
  uint8_t green = (uint8_t)((float)low.green + fraction * (float)(high.green - low.green));
  uint8_t blue = (uint8_t)((float)low.blue + fraction * (float)(high.blue - low.blue));

  return lv_color_make(red, green, blue);
}

/**
 * @brief   Override each pack table cell's fill color based on its voltage
 * @details LVGL's per-item table styling doesn't support indexed styles in this vendored version,
 *          so the fill color is patched in via the draw task instead.
 */
static void s_table_cell_draw_event_cb(lv_event_t *e) {
  lv_draw_task_t *draw_task = lv_event_get_draw_task(e);
  lv_draw_dsc_base_t *base_dsc = lv_draw_task_get_draw_dsc(draw_task);

  if (base_dsc->part != LV_PART_ITEMS || lv_draw_task_get_type(draw_task) != LV_DRAW_TASK_TYPE_FILL) {
    return;
  }

  uint32_t cell_idx = (base_dsc->id1 * PACK_TABLE_COLS) + base_dsc->id2;
  if (cell_idx >= NUMBER_OF_CELLS) {
    return;
  }

  lv_draw_fill_dsc_t *fill_dsc = lv_draw_task_get_fill_dsc(draw_task);
  fill_dsc->color = s_cell_fill_color(s_cell_voltages[cell_idx]);
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

  status_ok_or_return(lvgl_widgets_create_table(&s_pack_table, &config, screen));

  lv_obj_set_style_bg_opa(s_pack_table.table, LV_OPA_COVER, LV_PART_ITEMS);
  lv_obj_add_flag(s_pack_table.table, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
  lv_obj_add_event_cb(s_pack_table.table, s_table_cell_draw_event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);

  return STATUS_CODE_OK;
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
