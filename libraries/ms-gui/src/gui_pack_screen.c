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
#include "log.h"
#include "delay.h"
/* Inter-component Headers */
#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl.h"
#endif

/* Intra-component Headers */
#include "clut.h"
#include "gui_pack_screen.h"
#include "gui_widgets.h"

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl_screens.h"
#include "lvgl_widgets.h"

#define NUMBER_OF_CELLS 36U
#define PACK_SCREEN_VISIBLE_CELL_COUNT 5U
#define PACK_SCREEN_X_MARGIN 10
#define PACK_SCREEN_Y_MARGIN 12
#define PACK_SCREEN_LABEL_MAX_CHARS 320

static LabelWidget s_pack_voltage_label;
static char s_pack_voltage_text[PACK_SCREEN_LABEL_MAX_CHARS];
static uint16_t s_cell_voltages[NUMBER_OF_CELLS];
static bool s_widgets_initialized;

static StatusCode s_format_pack_voltage_text(void) {
  size_t offset = 0U;

  s_pack_voltage_text[0] = '\0';
  for (uint8_t i = 0U; i < PACK_SCREEN_VISIBLE_CELL_COUNT; ++i) {
    int chars_written = snprintf(&s_pack_voltage_text[offset], sizeof(s_pack_voltage_text) - offset, i == 0U ? "%u.%03uV" : " %u.%03uV", s_cell_voltages[i] / 10000U, (s_cell_voltages[i] % 10000U) / 10U);
    if (chars_written < 0 || (size_t)chars_written >= (sizeof(s_pack_voltage_text) - offset)) {
      return STATUS_CODE_RESOURCE_EXHAUSTED;
    }

    offset += (size_t)chars_written;
  }

  lv_label_set_text_static(s_pack_voltage_label.label, s_pack_voltage_text);
  return STATUS_CODE_OK;
}

static StatusCode s_create_pack_voltage_label(GuiScreen *screen, WidgetPosition position) {
  const LabelWidgetConfig pack_voltage_label_config = {
    .size = { .width = 200, .height = 275 },
    .position = position,
    .label_text = "0.000V",
    .alignment = WIDGET_TEXT_ALIGN_LEFT,
    .text_color_id = GUI_COLOR_TEXT_PRIMARY,
    .font = GUI_SMALL_TEXT,
    .background_enabled = false,
    .background_color_id = 0,
    .border_enabled = false,
    .border_color_id = GUI_COLOR_LABEL_BORDER,
    .border_width = 0,
  };

  status_ok_or_return(lvgl_widgets_create_label(&s_pack_voltage_label, &pack_voltage_label_config, screen));

  return s_format_pack_voltage_text();
}

static bool s_pack_voltage_text_changed(const uint16_t *cell_voltages) {
  if (cell_voltages == NULL) {
    return false;
  }

  for (uint8_t i = 0U; i < PACK_SCREEN_VISIBLE_CELL_COUNT; ++i) {
    if (s_cell_voltages[i] != cell_voltages[i]) {
      return true;
    }
  }

  return false;
}

StatusCode gui_pack_screen_init(GuiScreen *screen) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_widgets_initialized) {
    return STATUS_CODE_ALREADY_INITIALIZED;
  }

  status_ok_or_return(lvgl_set_background_color(screen, GUI_COLOR_SCREEN_BACKGROUND));

  WidgetPosition position = {
    .type = WIDGET_POSITION_ALIGN,
    .value.align = {
      .align = WIDGET_ALIGN_IN_TOP_LEFT,
      .x_offset = PACK_SCREEN_X_MARGIN,
      .y_offset = PACK_SCREEN_Y_MARGIN,
    }
  };

  status_ok_or_return(s_create_pack_voltage_label(screen, position));

  s_widgets_initialized = true;

  return STATUS_CODE_OK;
}

StatusCode gui_widgets_set_pack_voltage(uint8_t cell_idx, uint16_t cell_voltage) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (cell_idx >= NUMBER_OF_CELLS) {
    return STATUS_CODE_OUT_OF_RANGE;
  }

  if (s_pack_voltage_label.label == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (s_cell_voltages[cell_idx] == cell_voltage) {
    return STATUS_CODE_OK;
  }

  s_cell_voltages[cell_idx] = cell_voltage;
  return s_format_pack_voltage_text();
}

StatusCode gui_pack_screen_set_voltages(const uint16_t *cell_voltages) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (cell_voltages == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_pack_voltage_label.label == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (!s_pack_voltage_text_changed(cell_voltages)) {
    return STATUS_CODE_OK;
  }

  for (uint8_t i = 0U; i < PACK_SCREEN_VISIBLE_CELL_COUNT; ++i) {
    s_cell_voltages[i] = cell_voltages[i];
  }

  if (s_format_pack_voltage_text() != STATUS_CODE_OK) {
    while(true) {

      LOG_DEBUG("BRKPT1: ERROR");
      delay_ms(10);
    }
  }

  return STATUS_CODE_OK;
}

#else

StatusCode gui_pack_screen_init(GuiScreen *screen) {
  (void)screen;
  return STATUS_CODE_OK;
}

StatusCode gui_pack_screen_set_voltages(const uint16_t *cell_voltages) {
  (void)cell_voltages;
  return STATUS_CODE_OK;
}

StatusCode gui_widgets_set_pack_voltage(uint8_t cell_idx, uint16_t cell_voltage) {
  (void)cell_idx;
  (void)cell_voltage;
  return STATUS_CODE_OK;
}

#endif
