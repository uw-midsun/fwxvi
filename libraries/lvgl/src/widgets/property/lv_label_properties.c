/************************************************************************************************
 * @file    lv_label_properties.c
 *
 * @brief   Lv Label Properties
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../label/lv_label.h"

/* Intra-component Headers */

#if LV_USE_OBJ_PROPERTY && LV_USE_OBJ_PROPERTY_NAME

#if LV_USE_LABEL
/**
 * Label widget property names, name must be in order.
 * Generated code from properties.py
 */
/* *INDENT-OFF* */
const lv_property_name_t lv_label_property_names[4] = {
    {"long_mode",              LV_PROPERTY_LABEL_LONG_MODE,},
    {"text",                   LV_PROPERTY_LABEL_TEXT,},
    {"text_selection_end",     LV_PROPERTY_LABEL_TEXT_SELECTION_END,},
    {"text_selection_start",   LV_PROPERTY_LABEL_TEXT_SELECTION_START,},
};
#endif /*LV_USE_LABEL*/

/* *INDENT-ON* */
#endif
