/************************************************************************************************
 * @file    lv_keyboard_properties.c
 *
 * @brief   Lv Keyboard Properties
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../keyboard/lv_keyboard.h"

/* Intra-component Headers */

#if LV_USE_OBJ_PROPERTY && LV_USE_OBJ_PROPERTY_NAME

#if LV_USE_KEYBOARD
/**
 * Keyboard widget property names, name must be in order.
 * Generated code from properties.py
 */
/* *INDENT-OFF* */
const lv_property_name_t lv_keyboard_property_names[4] = {
    {"mode",                   LV_PROPERTY_KEYBOARD_MODE,},
    {"popovers",               LV_PROPERTY_KEYBOARD_POPOVERS,},
    {"selected_button",        LV_PROPERTY_KEYBOARD_SELECTED_BUTTON,},
    {"textarea",               LV_PROPERTY_KEYBOARD_TEXTAREA,},
};
#endif /*LV_USE_KEYBOARD*/

/* *INDENT-ON* */
#endif
