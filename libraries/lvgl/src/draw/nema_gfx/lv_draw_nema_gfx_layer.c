/************************************************************************************************
 * @file    lv_draw_nema_gfx_layer.c
 *
 * @brief   Lv Draw Nema Gfx Layer
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "lv_draw_nema_gfx.h"

/* Intra-component Headers */

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_NEMA_GFX

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_nema_gfx_layer(lv_draw_task_t * t, const lv_draw_image_dsc_t * draw_dsc, const lv_area_t * coords)
{
    lv_layer_t * layer_to_draw = (lv_layer_t *)draw_dsc->src;

    /*It can happen that nothing was draw on a layer and therefore its buffer is not allocated.
     *In this case just return. */
    if(layer_to_draw->draw_buf == NULL) return;

    lv_draw_image_dsc_t new_draw_dsc = *draw_dsc;
    new_draw_dsc.src = layer_to_draw->draw_buf;

    lv_draw_nema_gfx_img(t, &new_draw_dsc, coords);
}

#endif /*LV_USE_NEMA_GFX*/
