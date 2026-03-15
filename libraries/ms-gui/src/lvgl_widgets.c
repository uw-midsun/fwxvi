/************************************************************************************************
 * @file   lvgl_widgets.c
 *
 * @brief  LVGL-specific widgets used by gui_widgets
 *
 * @date   2026-03-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "clut.h"
#include "lvgl_widgets.h"

/* Intra-component Headers */

static lv_style_t s_speedometer_main_style;
static lv_style_t s_speedometer_minor_style;
static lv_style_t s_speedometer_needle_style;
static bool s_speedometer_styles_initialized;

static lv_style_t s_vertical_bar_bg_style;
static bool s_vertical_bar_styles_initialized;

static lv_color_t s_gui_palette_color(GuiColorId color_id) {
  ClutEntry color = clut_get_gui_color(color_id);
  return lv_color_make(clut_entry_red(color), clut_entry_green(color), clut_entry_blue(color));
}

static void s_apply_position(lv_obj_t *obj, const WidgetPosition *position) {
  if (obj == NULL || position == NULL) {
    return;
  }

  switch (position->type) {
    case WIDGET_POSITION_ALIGN:
      lv_obj_align(obj, position->value.align.align, position->value.align.x_offset, position->value.align.y_offset);
      break;

    case WIDGET_POSITION_ABSOLUTE:
      lv_obj_set_pos(obj, position->value.absolute.x, position->value.absolute.y);
      break;

    default:
      break;
  }
}

static void s_init_speedometer_styles(void) {
  if (s_speedometer_styles_initialized) {
    return;
  }

  lv_style_init(&s_speedometer_main_style);
  lv_style_set_line_color(&s_speedometer_main_style, s_gui_palette_color(GUI_COLOR_SPEEDOMETER_TICK_MAJOR));
  lv_style_set_line_width(&s_speedometer_main_style, 2);
  lv_style_set_text_color(&s_speedometer_main_style, s_gui_palette_color(GUI_COLOR_TEXT_PRIMARY));
  lv_style_set_text_font(&s_speedometer_main_style, GUI_SMALL_TEXT);
  lv_style_set_length(&s_speedometer_main_style, 10);

  lv_style_init(&s_speedometer_minor_style);
  lv_style_set_line_color(&s_speedometer_minor_style, s_gui_palette_color(GUI_COLOR_SPEEDOMETER_TICK_MINOR));
  lv_style_set_line_width(&s_speedometer_minor_style, 1);
  lv_style_set_length(&s_speedometer_minor_style, 5);

  lv_style_init(&s_speedometer_needle_style);
  lv_style_set_line_color(&s_speedometer_needle_style, s_gui_palette_color(GUI_COLOR_SPEEDOMETER_NEEDLE));
  lv_style_set_line_width(&s_speedometer_needle_style, 3);
  lv_style_set_line_rounded(&s_speedometer_needle_style, true);

  s_speedometer_styles_initialized = true;
}

static void s_init_vertical_bar_styles(void) {
  if (s_vertical_bar_styles_initialized) {
    return;
  }

  lv_style_init(&s_vertical_bar_bg_style);
  lv_style_set_radius(&s_vertical_bar_bg_style, 0);
  lv_style_set_bg_color(&s_vertical_bar_bg_style, s_gui_palette_color(GUI_COLOR_BAR_BACKGROUND));
  lv_style_set_border_color(&s_vertical_bar_bg_style, s_gui_palette_color(GUI_COLOR_BAR_BORDER));
  lv_style_set_border_width(&s_vertical_bar_bg_style, 1);

  s_vertical_bar_styles_initialized = true;
}

static void s_vertical_bar_draw_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  lv_draw_label_dsc_t label_dsc;
  lv_draw_label_dsc_init(&label_dsc);
  label_dsc.font = GUI_SMALL_TEXT;
  label_dsc.color = s_gui_palette_color(GUI_COLOR_TEXT_PRIMARY);
  label_dsc.align = LV_TEXT_ALIGN_CENTER;

  char buf[8];
  snprintf(buf, sizeof(buf), "%d%%", (int)lv_bar_get_value(obj));
  label_dsc.text = buf;
  label_dsc.text_local = true;

  lv_area_t coords;
  lv_obj_get_coords(obj, &coords);

  lv_area_t label_area;
  label_area.x1 = coords.x1;
  label_area.x2 = coords.x2;
  label_area.y1 = coords.y1 + (lv_area_get_height(&coords) / 2) - 8;
  label_area.y2 = label_area.y1 + 16;

  lv_draw_label(lv_event_get_layer(e), &label_dsc, &label_area);
}

StatusCode lvgl_widgets_create_speedometer(SpeedometerWidget *speedometer, const SpeedometerWidgetConfig *config, lv_obj_t *parent) {
  if (speedometer == NULL || config == NULL || parent == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (config->size.width <= 0 || config->size.height <= 0 || config->total_tick_count == 0 || config->major_tick_every == 0) {
    return STATUS_CODE_INVALID_ARGS;
  }

  *speedometer = (SpeedometerWidget){ 0 };
  s_init_speedometer_styles();

  speedometer->scale = lv_scale_create(parent);
  lv_obj_set_size(speedometer->scale, config->size.width, config->size.height);
  s_apply_position(speedometer->scale, &config->position);

  lv_scale_set_mode(speedometer->scale, LV_SCALE_MODE_ROUND_INNER);
  lv_scale_set_range(speedometer->scale, SPEEDOMETER_MIN_VALUE, SPEEDOMETER_MAX_VALUE);
  lv_scale_set_total_tick_count(speedometer->scale, config->total_tick_count);
  lv_scale_set_major_tick_every(speedometer->scale, config->major_tick_every);
  lv_scale_set_angle_range(speedometer->scale, config->angle_range);
  lv_scale_set_rotation(speedometer->scale, config->rotation);

  lv_obj_add_style(speedometer->scale, &s_speedometer_main_style, LV_PART_INDICATOR);
  lv_obj_add_style(speedometer->scale, &s_speedometer_minor_style, LV_PART_ITEMS);

  speedometer->needle = lv_line_create(speedometer->scale);
  lv_obj_add_style(speedometer->needle, &s_speedometer_needle_style, 0);
  lv_scale_set_line_needle_value(speedometer->scale, speedometer->needle, config->needle_length, SPEEDOMETER_MIN_VALUE);

  speedometer->label = lv_label_create(speedometer->scale);
  lv_label_set_text(speedometer->label, "0");
  lv_obj_set_style_text_color(speedometer->label, s_gui_palette_color(GUI_COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(speedometer->label, LV_ALIGN_CENTER, 0, 0);

  return STATUS_CODE_OK;
}

StatusCode lvgl_widgets_set_speed(SpeedometerWidget *speedometer, float speed_kmh) {
  if (speedometer == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (speedometer->scale == NULL || speedometer->needle == NULL || speedometer->label == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (speed_kmh < SPEEDOMETER_MIN_VALUE) {
    speed_kmh = SPEEDOMETER_MIN_VALUE;
  }
  if (speed_kmh > SPEEDOMETER_MAX_VALUE) {
    speed_kmh = SPEEDOMETER_MAX_VALUE;
  }

  lv_scale_set_line_needle_value(speedometer->scale, speedometer->needle, -15, (int32_t)speed_kmh);

  char buf[16];
  snprintf(buf, sizeof(buf), "%u", (unsigned int)speed_kmh);
  lv_label_set_text(speedometer->label, buf);

  return STATUS_CODE_OK;
}

StatusCode lvgl_widgets_create_bar(BarWidget *bar_widget, const BarWidgetConfig *config, lv_obj_t *parent) {
  if (bar_widget == NULL || config == NULL || parent == NULL || config->label_text == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (config->size.width <= 0 || config->size.height <= 0) {
    return STATUS_CODE_INVALID_ARGS;
  }

  *bar_widget = (BarWidget){ 0 };
  s_init_vertical_bar_styles();

  bar_widget->bar = lv_bar_create(parent);
  lv_obj_set_size(bar_widget->bar, config->size.width, config->size.height);
  s_apply_position(bar_widget->bar, &config->position);
  lv_bar_set_range(bar_widget->bar, BAR_MIN_VALUE, BAR_MAX_VALUE);
  lv_bar_set_value(bar_widget->bar, BAR_MIN_VALUE, LV_ANIM_OFF);
  lv_bar_set_orientation(bar_widget->bar, config->orientation);

  lv_obj_add_style(bar_widget->bar, &s_vertical_bar_bg_style, LV_PART_MAIN);
  lv_obj_set_style_bg_color(bar_widget->bar, s_gui_palette_color(config->indicator_color_id), LV_PART_INDICATOR);
  lv_obj_set_style_radius(bar_widget->bar, 0, LV_PART_INDICATOR);
  lv_obj_add_event_cb(bar_widget->bar, s_vertical_bar_draw_event_cb, LV_EVENT_DRAW_MAIN_END, NULL);

  bar_widget->label = lv_label_create(parent);
  lv_label_set_text(bar_widget->label, config->label_text);
  lv_obj_set_style_text_color(bar_widget->label, s_gui_palette_color(GUI_COLOR_TEXT_PRIMARY), 0);

  lv_obj_align_to(bar_widget->label, bar_widget->bar, config->label_text_alignment, 0, 0);

  return STATUS_CODE_OK;
}

StatusCode lvgl_widgets_set_bar_value(BarWidget *bar_widget, int32_t value) {
  if (bar_widget == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (bar_widget->bar == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (value < BAR_MIN_VALUE) {
    value = BAR_MIN_VALUE;
  }
  if (value > BAR_MAX_VALUE) {
    value = BAR_MAX_VALUE;
  }

  lv_bar_set_value(bar_widget->bar, value, LV_ANIM_ON);
  return STATUS_CODE_OK;
}
