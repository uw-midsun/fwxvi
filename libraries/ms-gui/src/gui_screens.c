/************************************************************************************************
 * @file   gui_screens.c
 *
 * @brief  High-level screen registry and navigation implementation
 *
 * @date   2026-04-11
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */
#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl.h"
#endif

/* Intra-component Headers */
#include "clut.h"
#include "gui_drive_screen.h"
#include "gui_pack_screen.h"
#include "gui_screens.h"
#include "lvgl_screens.h"
#include "lvgl_widgets.h"

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
typedef struct {
  GuiScreenDescriptor descriptor;
  bool registered;
} GuiScreenSlot;

static GuiScreenSlot s_screen_slots[NUM_GUI_SCREENS];
static GuiScreen *s_screen_root;
static GuiScreenId s_current_screen = GUI_SCREEN_DRIVE;
static bool s_current_screen_created;
static bool s_screens_initialized;

/**
 * @brief   Convert a semantic GUI color ID to an LVGL color
 * @param   color_id Semantic color identifier
 * @return  LVGL color matching the requested palette entry
 */
static lv_color_t s_gui_palette_color(GuiColorId color_id) {
  ClutEntry color = clut_get_gui_color(color_id);
  return lv_color_make(clut_entry_red(color), clut_entry_green(color), clut_entry_blue(color));
}

/**
 * @brief   Create a centered placeholder screen with title and subtitle text
 * @param   screen Root screen object to populate
 * @param   title Primary placeholder title
 * @param   subtitle Secondary descriptive text
 * @return  STATUS_CODE_OK on success, error otherwise
 */
static StatusCode s_create_centered_placeholder(GuiScreen *screen, const char *title, const char *subtitle) {
  lv_obj_t *title_label;
  lv_obj_t *subtitle_label;

  if (screen == NULL || title == NULL || subtitle == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  status_ok_or_return(lvgl_set_background_color(screen, GUI_COLOR_SCREEN_BACKGROUND));

  title_label = lv_label_create(screen);
  if (title_label == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  lv_label_set_text(title_label, title);
  lv_obj_set_style_text_color(title_label, s_gui_palette_color(GUI_COLOR_TEXT_PRIMARY), 0);
  lv_obj_set_style_text_font(title_label, GUI_MEDIUM_TEXT, 0);
  lv_obj_align(title_label, LV_ALIGN_CENTER, 0, -18);

  subtitle_label = lv_label_create(screen);
  if (subtitle_label == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  lv_label_set_text(subtitle_label, subtitle);
  lv_obj_set_style_text_color(subtitle_label, s_gui_palette_color(GUI_COLOR_TEXT_PRIMARY), 0);
  lv_obj_set_style_text_font(subtitle_label, GUI_SMALL_TEXT, 0);
  lv_obj_set_style_text_align(subtitle_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(subtitle_label, LV_ALIGN_CENTER, 0, 22);

  return STATUS_CODE_OK;
}

/**
 * @brief   Build the main drive screen widgets
 * @param   screen Root screen object to populate
 * @return  STATUS_CODE_OK on success, error otherwise
 */
static StatusCode s_create_drive_screen(GuiScreen *screen) {
  return gui_drive_screen_init(screen);
}

/**
 * @brief   Reset drive-screen widget handles after the active root is cleaned
 */
static void s_destroy_drive_screen(void) {
  gui_drive_screen_deinit();
}

/**
 * @brief   Build the trip info placeholder screen
 * @param   screen Root screen object to populate
 * @return  STATUS_CODE_OK on success, error otherwise
 */
static StatusCode s_create_trip_info_screen(GuiScreen *screen) {
  return s_create_centered_placeholder(screen, "Trip Info", "Unimplemented");
}

/**
 * @brief   Build the pack voltage placeholder screen
 * @param   screen Root screen object to populate
 * @return  STATUS_CODE_OK on success, error otherwise
 */
static StatusCode s_create_pack_voltage_screen(GuiScreen *screen) {
  return gui_pack_screen_init(screen);
}

/**
 * @brief   Reset pack-voltage widget handles after the active root is cleaned
 */
static void s_destroy_pack_voltage_screen(void) {
  gui_pack_screen_deinit();
}

/**
 * @brief   Build the pedal calibration placeholder screen
 * @param   screen Root screen object to populate
 * @return  STATUS_CODE_OK on success, error otherwise
 */
static StatusCode s_create_pedal_calib_screen(GuiScreen *screen) {
  return s_create_centered_placeholder(screen, "Pedal Calib", "Unimplemented");
}

/**
 * @brief   Register the default built-in GUI screens
 * @return  STATUS_CODE_OK on success, error otherwise
 */
static StatusCode s_register_default_screens(void) {
  static const GuiScreenDescriptor s_default_screens[NUM_GUI_SCREENS] = {
    [GUI_SCREEN_DRIVE] = {
      .id = GUI_SCREEN_DRIVE,
      .name = "Drive",
      .create = s_create_drive_screen,
      .destroy = s_destroy_drive_screen,
    },
    [GUI_SCREEN_TRIP_INFO] = {
      .id = GUI_SCREEN_TRIP_INFO,
      .name = "Trip Info",
      .create = s_create_trip_info_screen,
    },
    [GUI_SCREEN_PACK_VOLTAGE] = {
      .id = GUI_SCREEN_PACK_VOLTAGE,
      .name = "Pack Voltage",
      .create = s_create_pack_voltage_screen,
      .destroy = s_destroy_pack_voltage_screen,
    },
    [GUI_SCREEN_PEDAL_CALIB] = {
      .id = GUI_SCREEN_PEDAL_CALIB,
      .name = "Pedal Calib",
      .create = s_create_pedal_calib_screen,
    },
  };

  for (size_t i = 0U; i < NUM_GUI_SCREENS; i++) {
    if (!s_screen_slots[i].registered) {
      status_ok_or_return(gui_screens_register(&s_default_screens[i]));
    }
  }

  return STATUS_CODE_OK;
}

/**
 * @brief   Lazily create a screen the first time it is requested
 * @param   screen_id Screen identifier to materialize
 * @return  STATUS_CODE_OK on success, error otherwise
 */
static StatusCode s_ensure_screen_created(GuiScreenId screen_id) {
  GuiScreenSlot *slot;
  StatusCode status;

  if (screen_id >= NUM_GUI_SCREENS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  slot = &s_screen_slots[screen_id];
  if (!slot->registered || slot->descriptor.create == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (s_current_screen_created && s_current_screen == screen_id) {
    return STATUS_CODE_OK;
  }

  if (s_current_screen_created) {
    GuiScreenSlot *current_slot = &s_screen_slots[s_current_screen];

    if (current_slot->descriptor.destroy != NULL) {
      current_slot->descriptor.destroy();
    }
    status_ok_or_return(lvgl_screens_clean(s_screen_root));
    s_current_screen_created = false;
  }

  status = slot->descriptor.create(s_screen_root);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  s_current_screen = screen_id;
  s_current_screen_created = true;

  return STATUS_CODE_OK;
}

StatusCode gui_screens_init(void) {
  if (s_screens_initialized) {
    return STATUS_CODE_ALREADY_INITIALIZED;
  }

  status_ok_or_return(s_register_default_screens());
  status_ok_or_return(lvgl_screens_create(&s_screen_root));
  status_ok_or_return(lvgl_screens_load(s_screen_root));
  s_screens_initialized = true;
  StatusCode status = gui_screens_show(GUI_SCREEN_DRIVE);
  if (status != STATUS_CODE_OK) {
    lvgl_screens_destroy(s_screen_root);
    s_screen_root = NULL;
    s_current_screen_created = false;
    s_screens_initialized = false;
    return status;
  }

  return STATUS_CODE_OK;
}

StatusCode gui_screens_register(const GuiScreenDescriptor *descriptor) {
  GuiScreenSlot *slot;

  if (descriptor == NULL || descriptor->name == NULL || descriptor->create == NULL || descriptor->id >= NUM_GUI_SCREENS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  slot = &s_screen_slots[descriptor->id];
  if (s_current_screen_created && s_current_screen == descriptor->id) {
    return STATUS_CODE_ALREADY_INITIALIZED;
  }

  slot->descriptor = *descriptor;
  slot->registered = true;
  return STATUS_CODE_OK;
}

StatusCode gui_screens_show(GuiScreenId screen_id) {
  if (!s_screens_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  status_ok_or_return(s_ensure_screen_created(screen_id));

  return STATUS_CODE_OK;
}

GuiScreenId gui_screens_get_current(void) {
  return s_current_screen;
}

GuiScreen *gui_screens_get_root(GuiScreenId screen_id) {
  if (screen_id >= NUM_GUI_SCREENS || !s_current_screen_created || s_current_screen != screen_id) {
    return NULL;
  }

  return s_screen_root;
}

const char *gui_screens_get_name(GuiScreenId screen_id) {
  if (screen_id >= NUM_GUI_SCREENS || !s_screen_slots[screen_id].registered) {
    return NULL;
  }

  return s_screen_slots[screen_id].descriptor.name;
}
#else
static GuiScreenId s_current_screen = GUI_SCREEN_DRIVE;

StatusCode gui_screens_init(void) {
  return STATUS_CODE_OK;
}

StatusCode gui_screens_register(const GuiScreenDescriptor *descriptor) {
  (void)descriptor;
  return STATUS_CODE_OK;
}

StatusCode gui_screens_show(GuiScreenId screen_id) {
  s_current_screen = screen_id;
  return STATUS_CODE_OK;
}

GuiScreenId gui_screens_get_current(void) {
  return s_current_screen;
}

GuiScreen *gui_screens_get_root(GuiScreenId screen_id) {
  (void)screen_id;
  return NULL;
}

const char *gui_screens_get_name(GuiScreenId screen_id) {
  (void)screen_id;
  return NULL;
}
#endif
