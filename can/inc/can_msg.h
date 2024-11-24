#pragma once

/************************************************************************************************
 * @file   can_msg.h
 *
 * @brief  CAN message definitions
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @brief   CAN message type
 */
typedef enum {
  CAN_MSG_TYPE_DATA = 0,
  NUM_CAN_MSG_TYPES,
} CanMsgType;

/**
 * @brief   CAN message ID
 */
typedef uint32_t CanMessageId;

/**
 * @brief   CAN message ID
 */
typedef union CanId {
  CanMessageId raw;
  struct {
    uint16_t source_id : 4;
    uint16_t type : 1;
    uint32_t msg_id : 24;
  };
} CanId;

/**
 * @brief   CAN message structure
 */
typedef struct CanMessage {
  CanId id;
  CanMsgType type;
  uint8_t extended;
  size_t dlc;
  union {
    uint64_t data;
    uint32_t data_u32[2];
    uint16_t data_u16[4];
    uint8_t data_u8[8];
  };
} CanMessage;
