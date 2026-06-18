#pragma once

/************************************************************************************************
 * @file   can_cache.h
 *
 * @brief  CAN message cache for telemetry round-robin transmission
 *
 * @date   2026-06-10
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup telemetry
 * @brief    telemetry Firmware
 * @{
 */

/** @brief  Synthetic XBee-only ID for the telemetry bus-load stats datagram (device=4, msg_id=62). */
#define TELEMETRY_STATS_CAN_ID 2020U
/** @brief  DLC of the telemetry stats datagram (3 × uint16_t). */
#define TELEMETRY_STATS_DLC 6U

/** @brief  Synthetic XBee-only IDs for WS22 re-encoded telemetry (floats → scaled integers). */
#define WS22_TELEMETRY_STATUS_ID 2034U    /**< error_flags (u16) + limit_flags (u16) */
#define WS22_TELEMETRY_BUS_ID 2035U       /**< bus_voltage_cV (u16) + bus_current_cA (i16) */
#define WS22_TELEMETRY_VELOCITY_ID 2036U  /**< motor_velocity_rpm×10 (i16) + vehicle_kph×100 (u16) */
#define WS22_TELEMETRY_TEMP_ID 2037U      /**< motor_temp_°C×10 (i16) + heatsink_temp_°C×10 (i16) */
#define WS22_TELEMETRY_DRIVE_CMD_ID 2038U /**< drive_current×10000 (u16) + drive_velocity_rpm (i16) */
#define WS22_TELEMETRY_DLC 4U             /**< DLC for all WS22 synthetic entries (2 × uint16_t) */

/**
 * @brief   Transmission priority tier for a cached CAN message
 * @details Derived from the message's cycle rate in the board YAML:
 *          fast-cycle → HIGH, medium-cycle → MEDIUM, slow-cycle → LOW
 */
typedef enum {
  CAN_MSG_PRIORITY_HIGH = 0,
  CAN_MSG_PRIORITY_MEDIUM,
  CAN_MSG_PRIORITY_LOW,
  NUM_CAN_MSG_PRIORITIES,
} CanMsgPriority;

/**
 * @brief   Per-message cache entry
 * @details One entry exists for every CAN message that targets the telemetry board.
 *          Task A populates data and sets pending; Task B drains pending entries and transmits.
 */
typedef struct {
  uint32_t can_id;            /**< Raw 11-bit CAN bus ID used to match incoming messages */
  uint8_t length;             /**< DLC: number of valid data bytes */
  uint8_t data[8];            /**< Cached payload */
  CanMsgPriority priority;    /**< Priority tier this entry belongs to */
  bool pending;               /**< True when data has been updated but not yet transmitted */
  uint32_t last_updated_tick; /**< Tick count when this entry last received a CAN message; 0 = never */
  uint32_t last_tx_tick;      /**< Tick count when this entry was last transmitted; 0 = never */
  uint32_t tx_cooldown_ticks; /**< Minimum ticks between TX; 0 = no rate limit */
} CanMessageCache;

/**
 * @brief   High-priority (fast-cycle) message cache
 * @details Currently empty — no fast-cycle messages target telemetry.
 */
extern CanMessageCache g_can_cache_high[];
extern const size_t g_can_cache_high_size;

/**
 * @brief   Medium-priority (medium-cycle) message cache
 * @details Contains 22 entries from rear_controller, steering, front_controller, and IMU.
 */
extern CanMessageCache g_can_cache_medium[];
extern const size_t g_can_cache_medium_size;

/**
 * @brief   Low-priority (slow-cycle) message cache
 * @details Contains 3 entries from front_controller power group messages.
 */
extern CanMessageCache g_can_cache_low[];
extern const size_t g_can_cache_low_size;

/** @} */
