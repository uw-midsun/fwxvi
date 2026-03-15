/************************************************************************************************
 * @file    lv_animimage_properties.c
 *
 * @brief   Lv Animimage Properties
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../animimage/lv_animimage.h"

/* Intra-component Headers */

#if LV_USE_OBJ_PROPERTY && LV_USE_OBJ_PROPERTY_NAME

#if LV_USE_ANIMIMAGE
/**
 * Animimage widget property names, name must be in order.
 * Generated code from properties.py
 */
/* *INDENT-OFF* */
const lv_property_name_t lv_animimage_property_names[4] = {
    {"duration",               LV_PROPERTY_ANIMIMAGE_DURATION,},
    {"repeat_count",           LV_PROPERTY_ANIMIMAGE_REPEAT_COUNT,},
    {"src",                    LV_PROPERTY_ANIMIMAGE_SRC,},
    {"src_count",              LV_PROPERTY_ANIMIMAGE_SRC_COUNT,},
};
#endif /*LV_USE_ANIMIMAGE*/

/* *INDENT-ON* */
#endif
