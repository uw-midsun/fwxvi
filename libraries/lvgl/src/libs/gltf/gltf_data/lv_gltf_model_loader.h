#pragma once

/************************************************************************************************
 * @file    lv_gltf_model_loader.h
 *
 * @brief   Lv Gltf Model Loader
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../../lv_conf_internal.h"
#include "../../../misc/lv_types.h"

/* Intra-component Headers */

/**
 * @defgroup lv_gltf_model_loader
 * @brief    lv_gltf_model_loader Firmware
 * @{
 */

#ifndef LV_GLTF_MODEL_LOADER_H
#define LV_GLTF_MODEL_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_GLTF

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _lv_gltf_model_loader_t lv_gltf_model_loader_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a new glTF model loader instance
 * @return pointer to the newly created glTF model loader instance
 */
lv_gltf_model_loader_t * lv_gltf_model_loader_create(void);

/**
 * Store a texture ID associated with a texture hash in the loader
 * @param loader pointer to the glTF model loader instance
 * @param texture_hash hash value identifying the texture
 * @param texture_id the texture ID to associate with the hash
 */
void lv_gltf_model_loader_store_texture(lv_gltf_model_loader_t * loader, uint32_t texture_hash, uint32_t texture_id);

/**
 * Retrieve a texture ID by its hash from the loader
 * @param loader pointer to the glTF model loader instance
 * @param texture_hash hash value of the texture to retrieve
 * @return the texture ID associated with the hash, or 0 if not found
 */
uint32_t lv_gltf_model_loader_get_texture(lv_gltf_model_loader_t * loader, uint32_t texture_hash);

/**
 * Delete a glTF model loader instance and free its resources
 * @param loader pointer to the glTF model loader instance to delete
 */
void lv_gltf_model_loader_delete(lv_gltf_model_loader_t * loader);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_GLTF*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_GLTF_MODEL_LOADER_H*/

/** @} */
