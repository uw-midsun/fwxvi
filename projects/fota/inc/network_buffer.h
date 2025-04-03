#pragma once

/************************************************************************************************
 * @file   network_buffer.h
 *
 * @brief  Circular Buffer class implementation for firmware over the air (FOTA) updates
 *
 * @date   2025-03-16
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_error.h"

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

#define NETWORK_BUFFER_SIZE 1024U

/**
 * @brief struct containing details of circular buffer
 * @param data pointer to internally held data
 * @param size size of circular buffer
 * @param insert current insert index
 * @param head current read index
 */
typedef struct {
  uint8_t data[NETWORK_BUFFER_SIZE];
  uint16_t num_items;
  uint16_t write_index;
  uint16_t read_index;
} NetworkBuffer;

/**
 * @brief   Initalizes circular buffer for use, by initalizing assocated structs
 * @param   network_buffer Pointer to the network buffer
 * @return  FOTA_ERROR_SUCCESS if network buffer is initialized successfully
 *          FOTA_ERROR_INVALID_ARGS if one of the parameters are incorrect
 */
FotaError network_buffer_init(NetworkBuffer *network_buffer);

/**
 * @brief   Fetches the number of items in the network buffer
 * @param   network_buffer Pointer to the network buffer
 * @return  Number of items in the network buffer
 */
uint16_t network_buffer_num_items(NetworkBuffer *network_buffer);

/**
 * @brief   Fetches the number of empty slots in the network buffer
 * @param   network_buffer Pointer to the network buffer
 * @return  Number of empty slots in the network buffer
 */
uint16_t network_buffer_num_empty_slots(NetworkBuffer *network_buffer);

/**
 * @brief   Checks if the network buffer is empty
 * @param   network_buffer Pointer to the network buffer
 * @return  TRUE if network buffer is empty
 *          FALSE if network buffer has data
 */
bool network_buffer_empty(NetworkBuffer *network_buffer);

/**
 * @brief   Checks if the network buffer is full
 * @param   network_buffer Pointer to the network buffer
 * @return  TRUE if network buffer is full
 *          FALSE if network buffer is not full
 */
bool network_buffer_full(NetworkBuffer *network_buffer);

/**
 * @brief   Write data bytes
 * @param   network_buffer Pointer to the network buffer
 * @param   data Pointer to data byte to write
 * @return  FOTA_ERROR_SUCCESS if network buffer is initialized successfully
 *          FOTA_ERROR_INVALID_ARGS if one of the parameters are incorrect
 */
FotaError network_buffer_write(NetworkBuffer *network_buffer, uint8_t *data);

/**
 * @brief   Read data bytes
 * @param   network_buffer Pointer to the network buffer
 * @param   data Reads packet from circular buffer into a provided location
 * @return  FOTA_ERROR_SUCCESS if network buffer is initialized successfully
 *          FOTA_ERROR_INVALID_ARGS if one of the parameters are incorrect
 */
FotaError network_buffer_read(NetworkBuffer *network_buffer, uint8_t *data);

/** @} */
