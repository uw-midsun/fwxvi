/************************************************************************************************
 * @file   gui.c
 *
 * @brief  Source file for the GUI library
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/
/* Standard library Headers */
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#include "clut.h"
#include "gui.h"
#include "gui_widgets.h"
#include "ltdc.h"
#include "lvgl_driver.h"

/* Intra-component Headers */
#include "log.h"
#include "status.h"

/* Standard Library Headers */

StatusCode gui_init(LtdcSettings *settings) {
  if (!settings) {
    return STATUS_CODE_INVALID_ARGS;
  }

  StatusCode ret;

  ret = ltdc_init(settings);
  if (ret != STATUS_CODE_OK) {
    LOG_DEBUG("ltdc_init error: %d\n", ret);
    return ret;
  }

  ret = lvgl_driver_init(settings);
  if (ret != STATUS_CODE_OK) {
    LOG_DEBUG("lvgl_driver_init error: %d\n", ret);
    return ret;
  }

  ret = gui_widgets_init();
  if (ret != STATUS_CODE_OK) {
    LOG_DEBUG("gui_widgets_init error: %d\n", ret);
  }

  return STATUS_CODE_OK;
}

StatusCode gui_render() {
  return lvgl_driver_process();
}
