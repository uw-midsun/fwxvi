/************************************************************************************************
 * @file   gui_widgets.c
 *
 * @brief  High-level LVGL widget abstractions implementation
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

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl_screens.h"
#include "lvgl_widgets.h"

#define LABEL_MAX_CHARS 40

static SpeedometerWidget s_speedometer;

static BarWidget s_throttle_bar;
static BarWidget s_brake_bar;
static BarWidget s_soc_bar;

static LabelWidget s_top_label;
static LabelWidget s_cells_stats_label;
static LabelWidget s_temps_stats_label;
static LabelWidget s_cc_label;
static LabelWidget s_bps_fault;

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

static StatusCode s_create_speedometer(GuiScreen *screen) {
  const SpeedometerWidgetConfig speedometer_config = {
    .size = { .width = 220, .height = 220 },
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_CENTER, .x_offset = 0, .y_offset = 15 },
    },
    .total_tick_count = 41,
    .major_tick_every = 5,
    .angle_range = 270,
    .rotation = 135,
    .needle_length = -15,
  };

  return lvgl_widgets_create_speedometer(&s_speedometer, &speedometer_config, screen);
}

static StatusCode s_create_throttle_bar(GuiScreen *screen) {
  const BarWidgetConfig s_throttle_bar_config = {
    .size = { .width = 40, .height = 100 },
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_IN_TOP_LEFT, .x_offset = 15, .y_offset = 35 },
    },
    .label_text = "Throttle",
    .label_alignment = WIDGET_ALIGN_OUT_TOP_MID,
    .orientation = WIDGET_ORIENTATION_VERTICAL,
    .indicator_color_id = GUI_COLOR_THROTTLE_FILL,
  };

  return lvgl_widgets_create_bar(&s_throttle_bar, &s_throttle_bar_config, screen);
}

static StatusCode s_create_brake_bar(GuiScreen *screen) {
  const BarWidgetConfig s_brake_bar_config = {
    .size = { .width = 40, .height = 100 },
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_IN_BOTTOM_LEFT, .x_offset = 15, .y_offset = -35 },
     },
    .label_text = "Brake",
    .label_alignment = WIDGET_ALIGN_OUT_BOTTOM_MID,
    .orientation = WIDGET_ORIENTATION_VERTICAL,
    .indicator_color_id = GUI_COLOR_BRAKE_FILL,
  };

  return lvgl_widgets_create_bar(&s_brake_bar, &s_brake_bar_config, screen);
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
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_IN_TOP_MID,  .x_offset = 0, .y_offset = 0}
    },
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
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_IN_BOTTOM_RIGHT,  .x_offset = -10, .y_offset = -25}
    },
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
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_IN_RIGHT_MID,  .x_offset = -10, .y_offset = 10}
    },
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

static StatusCode s_create_cc_label(GuiScreen *screen) {
  const LabelWidgetConfig cruise_control_label_config = {
    .size = { .width = 100, .height = 20 },
    .position = {
      .type = WIDGET_POSITION_ALIGN,
      .value.align = { .align = WIDGET_ALIGN_IN_BOTTOM_MID,  .x_offset = 0, .y_offset = -30}
    },
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

StatusCode gui_widgets_init(void) {
  GuiScreen *screen = lvgl_get_active_screen();

  if (screen == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  status_ok_or_return(lvgl_set_background_color(screen, GUI_COLOR_SCREEN_BACKGROUND));

  /* Create widgets */
  status_ok_or_return(s_create_speedometer(screen));
  status_ok_or_return(s_create_throttle_bar(screen));
  status_ok_or_return(s_create_brake_bar(screen));
  status_ok_or_return(s_create_top_label(screen));
  status_ok_or_return(s_create_cell_stats_label(screen));
  status_ok_or_return(s_create_temps_stats_label(screen));
  status_ok_or_return(s_create_cc_label(screen));
  status_ok_or_return(s_create_soc_bar(screen));

  s_widgets_initialized = true;

  return STATUS_CODE_OK;
}

StatusCode gui_widgets_set_top_label(uint16_t pack_voltage, uint16_t motor_bus_voltage, uint16_t fault, uint8_t cell_at_fault) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  char text_buffer[LABEL_MAX_CHARS];

  if (fault) {
    bool is_cell_fault = false;
    const char *fault_text = s_get_bps_fault_text(fault, &is_cell_fault);

    if (is_cell_fault && cell_at_fault != 0U) {
      snprintf(text_buffer, sizeof(text_buffer), "%s, %u", fault_text, cell_at_fault);
    } else {
      snprintf(text_buffer, sizeof(text_buffer), "%s", fault_text);
    }
  }
  else {
    snprintf(text_buffer, sizeof(text_buffer), "Pack: %u V   |   Mot: %u V", pack_voltage,
           motor_bus_voltage);
  }

  return lvgl_widgets_set_label_text(&s_top_label, text_buffer);
}

StatusCode gui_widgets_set_cell_stats_label(uint16_t min_cell_voltage_mv, uint16_t max_cell_voltage_mv) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  char text_buffer[LABEL_MAX_CHARS];
  snprintf(text_buffer, sizeof(text_buffer), "Cells\n%u.%03u V\n%u.%03u V",
           min_cell_voltage_mv / 1000, min_cell_voltage_mv % 1000, max_cell_voltage_mv / 1000,
           max_cell_voltage_mv % 1000);

  return lvgl_widgets_set_label_text(&s_cells_stats_label, text_buffer);
}

StatusCode gui_widgets_set_temps_stats_label(int16_t motor_temp_c, uint16_t max_cell_temp_c) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  char text_buffer[LABEL_MAX_CHARS];
  snprintf(text_buffer, sizeof(text_buffer), "Temps\nMot: %d C\nMax C: %u C", motor_temp_c,
           max_cell_temp_c);

  return lvgl_widgets_set_label_text(&s_temps_stats_label, text_buffer);
}

StatusCode gui_widgets_set_speed(int16_t speed_kmh) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  return lvgl_widgets_set_speed(&s_speedometer, speed_kmh);
}

StatusCode gui_widgets_set_cc_speed(uint16_t cruise_control_speed_kmh, bool is_cc_enabled) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  char text_buffer[LABEL_MAX_CHARS];
  if (is_cc_enabled)
    snprintf(text_buffer, sizeof(text_buffer), "%u km/h", cruise_control_speed_kmh);
  else
    snprintf(text_buffer, sizeof(text_buffer), "cc off");

  return lvgl_widgets_set_label_text(&s_cc_label, text_buffer);
}

StatusCode gui_widgets_set_throttle_bar(uint8_t percent) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  return lvgl_widgets_set_bar_value(&s_throttle_bar, percent);
}

StatusCode gui_widgets_set_brake_bar(uint8_t percent) {
  if (!s_widgets_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  return lvgl_widgets_set_bar_value(&s_brake_bar, percent);
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

StatusCode gui_widgets_set_speed(int16_t speed_kmh) {
  (void)speed_kmh;
  return STATUS_CODE_OK;
}

StatusCode gui_widgets_set_throttle_bar(uint8_t percent) {
  (void)percent;
  return STATUS_CODE_OK;
}

StatusCode gui_widgets_set_brake_bar(uint8_t percent) {
  (void)percent;
  return STATUS_CODE_OK;
}

StatusCode gui_widgets_set_soc_bar(uint8_t soc_percent) {
  (void)soc_percent;
  return STATUS_CODE_OK;
}

StatusCode gui_widgets_set_top_label(uint16_t pack_voltage, uint16_t motor_bus_voltage, uint16_t fault, uint8_t cell_at_fault) {
  (void)pack_voltage;
  (void)motor_bus_voltage;
  (void)fault;
  (void)cell_at_fault;
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

StatusCode gui_widgets_set_cc_speed(uint16_t cruise_control_speed_kmh, bool is_cc_enabled) {
  (void) cruise_control_speed_kmh;

  return STATUS_CODE_OK;
}
#endif
