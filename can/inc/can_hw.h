#pragma once

/************************************************************************************************
 * can_hw.h
 *
 * Header file for CAN HW Interface
 *
 * Created: 2024-11-02
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#include "gpio.h"
#include "status.h"

/* Intra-component Headers */
#include "can_queue.h"
#include "gpio.h"
#include "uart_mcu.h"

#ifdef CAN_HW_DEV_USE_CAN0
#define CAN_HW_DEV_INTERFACE "can0"
#else
#define CAN_HW_DEV_INTERFACE "vcan0"
#endif

typedef enum {
  CAN_HW_BUS_STATUS_OK = 0,
  CAN_HW_BUS_STATUS_ERROR,
  CAN_HW_BUS_STATUS_OFF
} CanHwBusStatus;

typedef enum {
  CAN_HW_BITRATE_125KBPS,
  CAN_HW_BITRATE_250KBPS,
  CAN_HW_BITRATE_500KBPS,
  CAN_HW_BITRATE_1000KBPS,
  NUM_CAN_HW_BITRATES
} CanHwBitrate;

typedef enum {
  CAN_CONTINUOUS = 0,
  CAN_ONE_SHOT_MODE,
  NUM_CAN_MODES
} CanMode;

typedef struct CanSettings {
  uint16_t device_id;
  CanHwBitrate bitrate;
  GpioAddress tx;
  GpioAddress rx;
  bool loopback;
  CanMode mode;
} CanSettings;

/**
 * @brief   Initialize the CAN interface
 * @param   rx_queue Pointer to the CAN RX queue
 * @param   settings Pointer to the CAN settings
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode can_hw_init(const CanQueue* rx_queue, const CanSettings *settings);

/**
 * @brief   Sets a filter on the CAN interface
 * @param   mask
 * @param   filter
 * @param   extended
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode can_hw_add_filter_in(uint32_t mask, uint32_t filter, bool extended);

/**
 * @brief   Fetches the CAN bus status
 * @return  CAN_HW_BUS_STATUS_OK if bus is running
 *          CAN_HW_BUS_STATUS_ERROR if error is detected in the CAN bus
 *          CAN_HW_BUS_STATUS_OFF if bus is not initialized
 */
CanHwBusStatus can_hw_bus_status(void);

/**
 * @brief   Transmits CAN data on the bus
 * @param   id CAN message ID
 * @param   extended Boolean to use CAN extended ID feature
 * @param   data Pointer to the data to transmit
 * @param   len Size of the data to transfer
 * @return  STATUS_CODE_OK if data is retrieved succesfully
 */
StatusCode can_hw_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len);

// Must be called within the RX handler, returns whether a message was processed
/**
 * @brief   Receives CAN data from the bus
 * @param   id
 * @param   extended
 * @param   data Pointer to a buffer to store data
 * @param   len Number of CAN messages to retrieve in 8-bytes
 * @return
 */
bool can_hw_receive(uint32_t *id, bool *extended, uint64_t *data, size_t *len);
