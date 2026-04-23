/************************************************************************************************
 * @file   gui_widgets.c
 *
 * @brief  Common high-level LVGL widget abstractions implementation
 *
 * @date   2026-03-10
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

#include "status.h"

/* Inter-component Headers */
#include "global_enums.h"
#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl.h"
#endif

/* Intra-component Headers */
#include "clut.h"
#include "gui_widgets.h"
#include "display_defs.h"

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl_screens.h"
#include "lvgl_widgets.h"

static BarWidget s_soc_bar;

static LabelWidget s_top_label;
static LabelWidget s_cells_stats_label;
static LabelWidget s_temps_stats_label;

static bool s_widgets_initialized;

static const char *s_get_bps_fault_text(uint16_t fault, bool *is_cell_fault) {
  if (is_cell_fault == NULL) {
    return "BPS FAULT";
  }

  *is_cell_fault = false;

  if (fault & BPS_FAULT_OVERVOLTAGE_MASK) {
    *is_cell_fault = true;
    return "OVERVOLTAGE";
  }
  if (fault & BPS_FAULT_UNBALANCE_MASK) {
    *is_cell_fault = true;
    return "UNBALANCE";
  }
  if (fault & BPS_FAULT_OVERTEMP_AMBIENT_MASK) {
    return "OVERTEMP AMBIENT";
  }
  if (fault & BPS_FAULT_COMMS_LOSS_AFE_MASK) {
    return "COMMS LOSS AFE";
  }
  if (fault & BPS_FAULT_COMMS_LOSS_CURR_SENSE_MASK) {
    return "COMMS LOSS CURR";
  }
  if (fault & BPS_FAULT_OVERTEMP_CELL_MASK) {
    *is_cell_fault = true;
    return "OVERTEMP CELL";
  }
  if (fault & BPS_FAULT_OVERCURRENT_MASK) {
    return "OVERCURRENT";
  }
  if (fault & BPS_FAULT_UNDERVOLTAGE_MASK) {
    *is_cell_fault = true;
    return "UNDERVOLTAGE";
  }
  if (fault & BPS_FAULT_KILLSWITCH_MASK) {
    return "KILLSWITCH";
  }
  if (fault & BPS_FAULT_RELAY_CLOSE_FAILED_MASK) {
    return "RELAY FAILED";
  }
  if (fault & BPS_FAULT_DISCONNECTED_MASK) {
    return "DISCONNECTED";
  }

  return "BPS FAULT";
}

static const char *s_get_ws22_flag_text(uint16_t flags) {
  if (flags & ERROR_FLAG_HARDWARE_OVERCURRENT_MASK) {
    return "HARDWARE_OVERCURRENT";
  }
  if (flags & ERROR_FLAG_SOFTWARE_OVERCURRENT_MASK) {
    return "SOFTWARE_OVERCURRENT";
  }
  if (flags & ERROR_FLAG_DC_BUS_OV_MASK) {
    return "DC_BUS_OV";
  }
  if (flags & ERROR_FLAG_BAD_HALL_SEQUENCE_MASK) {
    return "BAD_HALL_SEQUENCE";
  }
  if (flags & ERROR_FLAG_WATCHDOG_CAUSED_RESET_MASK) {
    return "WATCHDOG_CAUSED_RESET";
  }
  if (flags & ERROR_FLAG_CFG_READ_ERROR_MASK) {
    return "CFG_READ_ERROR";
  }
  if (flags & ERROR_FLAG_DESATURATION_FAULT_MASK) {
    return "DESATURATION_FAULT";
  }
  if (flags & ERROR_FLAG_MOTOR_OVER_SPEED_MASK) {
    return "MOTOR_OVER_SPEED";
  }
  if (flags & LIMIT_FLAG_OUTPUT_VOLTAGE_PWM_MASK) {
    return "OUTPUT_VOLTAGE_PWM_LIMIT";
  }
  if (flags & LIMIT_FLAG_MOTOR_CURRENT_MASK) {
    return "MOTOR_CURRENT";
  }
  if (flags & LIMIT_FLAG_VELOCITY_MASK) {
    return "VELOCITY_LIMIT";
  }
  if (flags & LIMIT_FLAG_BUS_CURRENT_MASK) {
    return "BUS_CURRENT_LIMIT";
  }
  if (flags & LIMIT_FLAG_BUS_VOLTAGE_UPPER_MASK) {
    return "BUS_VOLTAGE_UPPER_LIMIT";
  }
  if (flags & LIMIT_FLAG_BUS_VOLTAGE_LOWER_MASK) {
    return "BUS_VOLTAGE_LOWER_LIMIT";
  }
  if (flags & LIMIT_FLAG_TEMPERATURE_MASK) {
    return "TEMPERATURE_LIMIT";
  }

  return "WS22 FLAG";
}

static StatusCode s_create_soc_bar(GuiScreen *screen) {
  const BarWidgetConfig s_soc_bar_config = {
    .size = { .width = 80, .height = 20 },
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_IN_TOP_RIGHT, .x_offset = -30, .y_offset = 60 },
    },
    .label_text = "SOC",
    .label_alignment = WIDGET_ALIGN_OUT_TOP_LEFT,
    .orientation = WIDGET_ORIENTATION_HORIZONTAL,
    .indicator_color_id = GUI_COLOR_SOC_FILL,
  };

  return lvgl_widgets_create_bar(&s_soc_bar, &s_soc_bar_config, screen);
}

static StatusCode s_create_top_label(GuiScreen *screen) {
  const LabelWidgetConfig top_label_config = {
    .size = { .width = 300, .height = 30 },
    .position = { .type = WIDGET_POSITION_ALIGN, .value.align = { .align = WIDGET_ALIGN_IN_TOP_MID, .x_offset = 0, .y_offset = 0 } },
    .label_text = "Pack: 0 V   |   Mot: 0 V",
    .alignment = WIDGET_TEXT_ALIGN_CENTER,
    .text_color_id = GUI_COLOR_TEXT_PRIMARY,
    .font = GUI_SMALL_TEXT,
    .background_enabled = false,
    .background_color_id = 0,
    .border_enabled = true,
    .border_color_id = GUI_COLOR_LABEL_BORDER,
    .border_width = 1,
  };

  return lvgl_widgets_create_label(&s_top_label, &top_label_config, screen);
}

static StatusCode s_create_cell_stats_label(GuiScreen *screen) {
  const LabelWidgetConfig cell_stats_label_config = {
    .size = { .width = 100, .height = 50 },
    .position = { .type = WIDGET_POSITION_ALIGN, .value.align = { .align = WIDGET_ALIGN_IN_BOTTOM_RIGHT, .x_offset = -10, .y_offset = -25 } },
    .label_text = "Cells\n0.000 V\n0.000 V",
    .alignment = WIDGET_TEXT_ALIGN_LEFT,
    .text_color_id = GUI_COLOR_TEXT_PRIMARY,
    .font = GUI_SMALL_TEXT,
    .background_enabled = false,
    .background_color_id = 0,
    .border_enabled = false,
    .border_color_id = GUI_COLOR_LABEL_BORDER,
    .border_width = 0,
  };

  return lvgl_widgets_create_label(&s_cells_stats_label, &cell_stats_label_config, screen);
}

static StatusCode s_create_temps_stats_label(GuiScreen *screen) {
  const LabelWidgetConfig temps_stats_label_config = {
    .size = { .width = 100, .height = 50 },
    .position = { .type = WIDGET_POSITION_ALIGN, .value.align = { .align = WIDGET_ALIGN_IN_RIGHT_MID, .x_offset = -10, .y_offset = 10 } },
    .label_text = "Temps\nMot: 0 C\nMax C: 0 C",
    .alignment = WIDGET_TEXT_ALIGN_LEFT,
    .text_color_id = GUI_COLOR_TEXT_PRIMARY,
    .font = GUI_SMALL_TEXT,
    .background_enabled = false,
    .background_color_id = 0,
    .border_enabled = false,
    .border_color_id = GUI_COLOR_LABEL_BORDER,
    .border_width = 0,
  };

  return lvgl_widgets_create_label(&s_temps_stats_label, &temps_stats_label_config, screen);
}

StatusCode gui_widgets_init_screen(GuiScreen *screen) {
  if (screen == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_widgets_initialized) {
    return STATUS_CODE_ALREADY_INITIALIZED;
  }

  status_ok_or_return(s_create_top_label(screen));
  status_ok_or_return(s_create_cell_stats_label(screen));
  status_ok_or_return(s_create_temps_stats_label(screen));
  status_ok_or_return(s_create_soc_bar(screen));

  s_widgets_initialized = true;

  return STATUS_CODE_OK;
}

void gui_widgets_deinit(void) {
  s_soc_bar = (BarWidget){ 0 };
  s_top_label = (LabelWidget){ 0 };
  s_cells_stats_label = (LabelWidget){ 0 };
  s_temps_stats_label = (LabelWidget){ 0 };
  s_widgets_initialized = false;
}

StatusCode gui_widgets_init(void) {
  GuiScreen *screen = lvgl_get_active_screen();

  if (screen == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return gui_widgets_init_screen(screen);
}

StatusCode gui_widgets_set_top_label(uint16_t pack_voltage, uint16_t motor_bus_voltage, uint16_t bps_fault, uint8_t cell_at_fault, uint16_t ws22_flags) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  char text_buffer[LABEL_MAX_CHARS];

  if (bps_fault) {
    bool is_cell_fault = false;
    const char *fault_text = s_get_bps_fault_text(bps_fault, &is_cell_fault);

    if (is_cell_fault && cell_at_fault != 0U) {
      snprintf(text_buffer, sizeof(text_buffer), "%s, %u", fault_text, cell_at_fault);
    } else {
      snprintf(text_buffer, sizeof(text_buffer), "%s", fault_text);
    }
  } else if (ws22_flags) {
    const char *ws22_flag_text = s_get_ws22_flag_text(ws22_flags);
    snprintf(text_buffer, sizeof(text_buffer), "%s", ws22_flag_text);
  } else {
    snprintf(text_buffer, sizeof(text_buffer), "Pack: %u V   |   Mot: %u V", pack_voltage, motor_bus_voltage);
  }

  return lvgl_widgets_set_label_text(&s_top_label, text_buffer);
}

StatusCode gui_widgets_set_cell_stats_label(uint16_t min_cell_voltage_mv, uint16_t max_cell_voltage_mv) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  char text_buffer[LABEL_MAX_CHARS];
  snprintf(text_buffer, sizeof(text_buffer), "Cells\n%u.%03u V\n%u.%03u V", min_cell_voltage_mv / 10000U, (min_cell_voltage_mv % 10000U) / 10U, max_cell_voltage_mv / 10000U,
           (max_cell_voltage_mv % 10000U) / 10U);

  return lvgl_widgets_set_label_text(&s_cells_stats_label, text_buffer);
}

StatusCode gui_widgets_set_temps_stats_label(int16_t motor_temp_c, uint16_t max_cell_temp_c) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  char text_buffer[LABEL_MAX_CHARS];
  snprintf(text_buffer, sizeof(text_buffer), "Temps\nMot: %d C\nMax C: %u C", motor_temp_c, max_cell_temp_c);

  return lvgl_widgets_set_label_text(&s_temps_stats_label, text_buffer);
}

StatusCode gui_widgets_set_soc_bar(uint8_t soc_percent) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  return lvgl_widgets_set_bar_value(&s_soc_bar, soc_percent);
}

#else
StatusCode gui_widgets_init(void) {
  return STATUS_CODE_OK;
}

StatusCode gui_widgets_init_screen(GuiScreen *screen) {
  (void)screen;
  return STATUS_CODE_OK;
}

void gui_widgets_deinit(void) {}

StatusCode gui_widgets_set_soc_bar(uint8_t soc_percent) {
  (void)soc_percent;
  return STATUS_CODE_OK;
}

StatusCode gui_widgets_set_top_label(uint16_t pack_voltage, uint16_t motor_bus_voltage, uint16_t fault, uint8_t cell_at_fault, uint16_t ws22_flags) {
  (void)pack_voltage;
  (void)motor_bus_voltage;
  (void)fault;
  (void)cell_at_fault;
  (void)ws22_flags;
  return STATUS_CODE_OK;
}

StatusCode gui_widgets_set_cell_stats_label(uint16_t min_cell_voltage_mv, uint16_t max_cell_voltage_mv) {
  (void)min_cell_voltage_mv;
  (void)max_cell_voltage_mv;
  return STATUS_CODE_OK;
}

StatusCode gui_widgets_set_temps_stats_label(int16_t motor_temp_c, uint16_t max_cell_temp_c) {
  (void)motor_temp_c;
  (void)max_cell_temp_c;
  return STATUS_CODE_OK;
}

#endif
