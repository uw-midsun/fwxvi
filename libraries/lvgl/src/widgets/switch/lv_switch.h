#pragma once

/************************************************************************************************
 * @file    lv_switch.h
 *
 * @brief   Lv Switch
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../core/lv_obj.h"
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_switch
 * @brief    lv_switch Firmware
 * @{
 */

#ifndef LV_SWITCH_H
#define LV_SWITCH_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_SWITCH != 0

/*********************
 *      DEFINES
 *********************/

/** Switch knob extra area correction factor */
#define LV_SWITCH_KNOB_EXT_AREA_CORRECTION 2

LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_switch_class;

#if LV_USE_OBJ_PROPERTY
enum _lv_property_switch_id_t {
    LV_PROPERTY_ID(SWITCH, ORIENTATION, LV_PROPERTY_TYPE_INT, 0),
    LV_PROPERTY_SWITCH_END,
};
#endif

/**********************
 *      TYPEDEFS
 **********************/

typedef enum {
    LV_SWITCH_ORIENTATION_AUTO,
    LV_SWITCH_ORIENTATION_HORIZONTAL,
    LV_SWITCH_ORIENTATION_VERTICAL
} lv_switch_orientation_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a switch object
 * @param parent    pointer to an object, it will be the parent of the new switch
 * @return          pointer to the created switch
 */
lv_obj_t * lv_switch_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the orientation of switch.
 * @param obj           pointer to switch object
 * @param orientation   switch orientation from `lv_switch_orientation_t`
 */
void lv_switch_set_orientation(lv_obj_t * obj, lv_switch_orientation_t orientation);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the orientation of switch.
 * @param obj       pointer to switch object
 * @return          switch orientation from ::lv_switch_orientation_t
 */
lv_switch_orientation_t lv_switch_get_orientation(lv_obj_t * obj);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_SWITCH*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SWITCH_H*/

/** @} */
