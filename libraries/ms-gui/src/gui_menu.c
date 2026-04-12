/************************************************************************************************
 * @file   gui_menu.c
 *
 * @brief  Overlay menu helpers for GUI navigation
 *
 * @date   2026-04-11
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "FreeRTOS.h"
#include "lvgl.h"
#include "task.h"
#endif

/* Intra-component Headers */
#include "clut.h"
#include "gui_menu.h"
#include "gui_screens.h"
#include "lvgl_screens.h"

/* TODO: Better abstraction over lvgl library */

#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#define GUI_MENU_ITEM_COUNT 5U
#define GUI_MENU_PANEL_WIDTH 240
#define GUI_MENU_PANEL_HEIGHT 180
#define GUI_MENU_ROW_WIDTH 188
#define GUI_MENU_ROW_HEIGHT 24

typedef enum {
  GUI_MENU_ITEM_SCREEN = 0,
  GUI_MENU_ITEM_ACTION,
} GuiMenuItemType;

typedef struct {
  GuiMenuItemType type;
  union {
    GuiScreenId screen_id;
    GuiMenuActionCallback callback;
  } target;
} GuiMenuItem;

typedef struct {
  bool initialized;
  bool is_open;
  uint8_t selected_index;
  GuiMenuActionCallback party_mode_callback;
  lv_obj_t *overlay;
  lv_obj_t *panel;
  lv_obj_t *title;
  lv_obj_t *rows[GUI_MENU_ITEM_COUNT];
  lv_obj_t *row_labels[GUI_MENU_ITEM_COUNT];
} GuiMenuState;

typedef struct {
  bool toggle_requested;
  bool select_requested;
  VehicleDriveState select_drive_state;
  uint8_t move_up_count;
  uint8_t move_down_count;
} GuiMenuPendingRequests;

static GuiMenuState s_menu;
static GuiMenuPendingRequests s_pending_requests;

static const GuiMenuItem s_menu_items[GUI_MENU_ITEM_COUNT] = {
  [0] = {
    .type = GUI_MENU_ITEM_SCREEN,
    .target.screen_id = GUI_SCREEN_DRIVE,
  },
  [1] = {
    .type = GUI_MENU_ITEM_SCREEN,
    .target.screen_id = GUI_SCREEN_TRIP_INFO,
  },
  [2] = {
    .type = GUI_MENU_ITEM_SCREEN,
    .target.screen_id = GUI_SCREEN_PACK_VOLTAGE,
  },
  [3] = {
    .type = GUI_MENU_ITEM_ACTION,
    .target.callback = NULL,
  },
  [4] = {
    .type = GUI_MENU_ITEM_SCREEN,
    .target.screen_id = GUI_SCREEN_PEDAL_CALIB,
  },
};

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
 * @brief   Get the visible label for a menu row
 * @param   index Zero-based menu item index
 * @return  Static label string for the requested row
 */
static const char *s_get_item_label(uint8_t index) {
  if (index >= GUI_MENU_ITEM_COUNT) {
    return "";
  }

  if (s_menu_items[index].type == GUI_MENU_ITEM_ACTION) {
    return "Party Mode";
  }

  return gui_screens_get_name(s_menu_items[index].target.screen_id);
}

/**
 * @brief   Determine whether a menu item may be activated in the current drive state
 * @param   item Menu item being evaluated
 * @param   drive_state Current confirmed vehicle drive state
 * @return  TRUE when the item may be activated, FALSE otherwise
 */
static bool s_is_item_selectable(const GuiMenuItem *item, VehicleDriveState drive_state) {
  if (item == NULL) {
    return false;
  }

#ifndef MS_PLATFORM_X86
  if (item->type == GUI_MENU_ITEM_SCREEN && item->target.screen_id == GUI_SCREEN_PEDAL_CALIB && drive_state != VEHICLE_DRIVE_STATE_NEUTRAL) {
    return false;
  }
#else
  (void)drive_state;
#endif

  return true;
}

/**
 * @brief   Restyle each menu row to reflect the current selection
 */
static void s_refresh_selection(void) {
  for (uint8_t i = 0U; i < GUI_MENU_ITEM_COUNT; i++) {
    bool selected = (i == s_menu.selected_index);

    lv_obj_set_style_bg_opa(s_menu.rows[i], selected ? LV_OPA_60 : LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_color(s_menu.rows[i], s_gui_palette_color(GUI_COLOR_MENU_ITEM_SELECTED_BACKGROUND), 0);
    lv_obj_set_style_border_color(s_menu.rows[i], s_gui_palette_color(GUI_COLOR_MENU_ITEM_SELECTED_BORDER), 0);
    lv_obj_set_style_border_width(s_menu.rows[i], selected ? 1 : 0, 0);
    lv_obj_set_style_text_color(s_menu.row_labels[i], s_gui_palette_color(GUI_COLOR_MENU_ITEM_TEXT), 0);
  }
}

StatusCode gui_menu_init(void) {
  GuiMenuActionCallback party_mode_callback = s_menu.party_mode_callback;

  if (s_menu.initialized) {
    return STATUS_CODE_ALREADY_INITIALIZED;
  }

  s_menu = (GuiMenuState){ 0 };
  s_pending_requests = (GuiMenuPendingRequests){ 0 };
  s_menu.party_mode_callback = party_mode_callback;
  s_menu.initialized = true;
  return STATUS_CODE_OK;
}

StatusCode gui_menu_set_party_mode_callback(GuiMenuActionCallback callback) {
  s_menu.party_mode_callback = callback;
  return STATUS_CODE_OK;
}

StatusCode gui_menu_request_toggle(void) {
  taskENTER_CRITICAL();
  s_pending_requests.toggle_requested = true;
  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}

StatusCode gui_menu_request_move_up(void) {
  taskENTER_CRITICAL();
  if (s_pending_requests.move_up_count < UINT8_MAX) {
    s_pending_requests.move_up_count++;
  }
  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}

StatusCode gui_menu_request_move_down(void) {
  taskENTER_CRITICAL();
  if (s_pending_requests.move_down_count < UINT8_MAX) {
    s_pending_requests.move_down_count++;
  }
  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}

StatusCode gui_menu_request_select(VehicleDriveState drive_state) {
  taskENTER_CRITICAL();
  s_pending_requests.select_requested = true;
  s_pending_requests.select_drive_state = drive_state;
  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}

StatusCode gui_menu_process_pending(void) {
  GuiMenuPendingRequests pending;
  StatusCode status;

  if (!s_menu.initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  taskENTER_CRITICAL();
  pending = s_pending_requests;
  s_pending_requests = (GuiMenuPendingRequests){ 0 };
  taskEXIT_CRITICAL();

  if (pending.toggle_requested) {
    status_ok_or_return(gui_menu_toggle());
  }

  while (pending.move_up_count-- > 0U) {
    status = gui_menu_move_up();
    if (status != STATUS_CODE_OK && status != STATUS_CODE_INCOMPLETE) {
      return status;
    }
  }

  while (pending.move_down_count-- > 0U) {
    status = gui_menu_move_down();
    if (status != STATUS_CODE_OK && status != STATUS_CODE_INCOMPLETE) {
      return status;
    }
  }

  if (pending.select_requested) {
    status = gui_menu_select(pending.select_drive_state);
    if (status != STATUS_CODE_INCOMPLETE) {
      return status;
    }
  }

  return STATUS_CODE_OK;
}

StatusCode gui_menu_open(void) {
  GuiScreen *top_layer;

  if (!s_menu.initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (s_menu.is_open) {
    return STATUS_CODE_OK;
  }

  top_layer = lvgl_get_top_layer();
  if (top_layer == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  s_menu.overlay = lv_obj_create(top_layer);
  if (s_menu.overlay == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  lv_obj_remove_style_all(s_menu.overlay);
  lv_obj_set_size(s_menu.overlay, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(s_menu.overlay, s_gui_palette_color(GUI_COLOR_MENU_OVERLAY), 0);
  lv_obj_set_style_bg_opa(s_menu.overlay, LV_OPA_60, 0);
  lv_obj_add_flag(s_menu.overlay, LV_OBJ_FLAG_CLICKABLE);

  s_menu.panel = lv_obj_create(s_menu.overlay);
  if (s_menu.panel == NULL) {
    gui_menu_close();
    return STATUS_CODE_INTERNAL_ERROR;
  }

  lv_obj_set_size(s_menu.panel, GUI_MENU_PANEL_WIDTH, GUI_MENU_PANEL_HEIGHT);
  lv_obj_center(s_menu.panel);
  lv_obj_clear_flag(s_menu.panel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(s_menu.panel, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_bg_color(s_menu.panel, s_gui_palette_color(GUI_COLOR_MENU_PANEL_BACKGROUND), 0);
  lv_obj_set_style_border_color(s_menu.panel, s_gui_palette_color(GUI_COLOR_MENU_PANEL_BORDER), 0);
  lv_obj_set_style_border_width(s_menu.panel, 1, 0);
  lv_obj_set_style_radius(s_menu.panel, 8, 0);

  s_menu.title = lv_label_create(s_menu.panel);
  if (s_menu.title == NULL) {
    gui_menu_close();
    return STATUS_CODE_INTERNAL_ERROR;
  }

  lv_label_set_text(s_menu.title, "Menu");
  lv_obj_set_style_text_color(s_menu.title, s_gui_palette_color(GUI_COLOR_MENU_TITLE_TEXT), 0);
  lv_obj_align(s_menu.title, LV_ALIGN_TOP_MID, 0, 10);

  for (uint8_t i = 0U; i < GUI_MENU_ITEM_COUNT; i++) {
    s_menu.rows[i] = lv_obj_create(s_menu.panel);
    s_menu.row_labels[i] = NULL;
    if (s_menu.rows[i] == NULL) {
      gui_menu_close();
      return STATUS_CODE_INTERNAL_ERROR;
    }

    lv_obj_set_size(s_menu.rows[i], GUI_MENU_ROW_WIDTH, GUI_MENU_ROW_HEIGHT);
    lv_obj_align(s_menu.rows[i], LV_ALIGN_TOP_MID, 0, 36 + (i * 26));
    lv_obj_clear_flag(s_menu.rows[i], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_menu.rows[i], LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(s_menu.rows[i], 4, 0);
    lv_obj_set_style_pad_left(s_menu.rows[i], 8, 0);
    lv_obj_set_style_pad_right(s_menu.rows[i], 8, 0);
    lv_obj_set_style_pad_top(s_menu.rows[i], 2, 0);
    lv_obj_set_style_pad_bottom(s_menu.rows[i], 2, 0);

    s_menu.row_labels[i] = lv_label_create(s_menu.rows[i]);
    if (s_menu.row_labels[i] == NULL) {
      gui_menu_close();
      return STATUS_CODE_INTERNAL_ERROR;
    }

    lv_label_set_text(s_menu.row_labels[i], s_get_item_label(i));
    lv_obj_set_style_text_color(s_menu.row_labels[i], s_gui_palette_color(GUI_COLOR_MENU_ITEM_TEXT), 0);
    lv_obj_align(s_menu.row_labels[i], LV_ALIGN_LEFT_MID, 0, 0);
  }

  s_menu.selected_index = 0U;
  s_menu.is_open = true;
  s_refresh_selection();

  return STATUS_CODE_OK;
}

StatusCode gui_menu_close(void) {
  if (!s_menu.initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (!s_menu.is_open) {
    return STATUS_CODE_OK;
  }

  lv_obj_delete(s_menu.overlay);
  s_menu.overlay = NULL;
  s_menu.panel = NULL;
  s_menu.title = NULL;
  for (uint8_t i = 0U; i < GUI_MENU_ITEM_COUNT; i++) {
    s_menu.rows[i] = NULL;
    s_menu.row_labels[i] = NULL;
  }
  s_menu.is_open = false;

  return STATUS_CODE_OK;
}

StatusCode gui_menu_toggle(void) {
  if (s_menu.is_open) {
    return gui_menu_close();
  }

  return gui_menu_open();
}

StatusCode gui_menu_move_up(void) {
  if (!s_menu.initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (!s_menu.is_open) {
    return STATUS_CODE_INCOMPLETE;
  }

  if (s_menu.selected_index == 0U) {
    s_menu.selected_index = GUI_MENU_ITEM_COUNT - 1U;
  } else {
    s_menu.selected_index--;
  }
  s_refresh_selection();

  return STATUS_CODE_OK;
}

StatusCode gui_menu_move_down(void) {
  if (!s_menu.initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (!s_menu.is_open) {
    return STATUS_CODE_INCOMPLETE;
  }

  s_menu.selected_index = (uint8_t)((s_menu.selected_index + 1U) % GUI_MENU_ITEM_COUNT);
  s_refresh_selection();

  return STATUS_CODE_OK;
}

StatusCode gui_menu_select(VehicleDriveState drive_state) {
  StatusCode status = STATUS_CODE_OK;
  const GuiMenuItem *item;

  if (!s_menu.initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (!s_menu.is_open) {
    return STATUS_CODE_INCOMPLETE;
  }

  item = &s_menu_items[s_menu.selected_index];
  if (!s_is_item_selectable(item, drive_state)) {
    return STATUS_CODE_INVALID_ARGS;
  }

  switch (item->type) {
    case GUI_MENU_ITEM_SCREEN:
      status_ok_or_return(gui_menu_close());
      return gui_screens_show(item->target.screen_id);

    case GUI_MENU_ITEM_ACTION:
      if (s_menu.party_mode_callback == NULL) {
        return STATUS_CODE_UNIMPLEMENTED;
      }

      status = s_menu.party_mode_callback();
      if (status != STATUS_CODE_OK) {
        return status;
      }

      return gui_menu_close();

    default:
      return STATUS_CODE_UNREACHABLE;
  }
}

bool gui_menu_is_open(void) {
  return s_menu.is_open;
}
#else
static bool s_menu_open;

StatusCode gui_menu_init(void) {
  return STATUS_CODE_OK;
}

StatusCode gui_menu_set_party_mode_callback(GuiMenuActionCallback callback) {
  (void)callback;
  return STATUS_CODE_OK;
}

StatusCode gui_menu_request_toggle(void) {
  return STATUS_CODE_OK;
}

StatusCode gui_menu_request_move_up(void) {
  return STATUS_CODE_OK;
}

StatusCode gui_menu_request_move_down(void) {
  return STATUS_CODE_OK;
}

StatusCode gui_menu_request_select(VehicleDriveState drive_state) {
  (void)drive_state;
  return STATUS_CODE_OK;
}

StatusCode gui_menu_process_pending(void) {
  return STATUS_CODE_OK;
}

StatusCode gui_menu_open(void) {
  s_menu_open = true;
  return STATUS_CODE_OK;
}

StatusCode gui_menu_close(void) {
  s_menu_open = false;
  return STATUS_CODE_OK;
}

StatusCode gui_menu_toggle(void) {
  s_menu_open = !s_menu_open;
  return STATUS_CODE_OK;
}

StatusCode gui_menu_move_up(void) {
  return STATUS_CODE_OK;
}

StatusCode gui_menu_move_down(void) {
  return STATUS_CODE_OK;
}

StatusCode gui_menu_select(VehicleDriveState drive_state) {
  (void)drive_state;
  return STATUS_CODE_OK;
}

bool gui_menu_is_open(void) {
  return s_menu_open;
}
#endif
