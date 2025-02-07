#pragma once

/************************************************************************************************
 * @file   relays.h
 *
 * @brief  Header file for Relays
 *
 * @date   2025-02-06
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "bms_carrier.h"
#include "gpio.h"
#include "gpio_interrupts.h"
#include "interrupts.h"

/* Intra-component Headers */

/**
 * @defgroup bms_carrier
 * @brief    bms_carrier Firmware
 * @{
 */

/** @brief  Delay between consecutive relays being closed */
#define BMS_CLOSE_RELAYS_DELAY_MS 250U
/** @brief  Number of BMS relays */
#define NUM_BMS_RELAYS            3U
/** @brief  Killswitch interrupt notification */
#define KILLSWITCH_EVENT_IT       3U

/**
 * @brief   Relay storage
 */
struct RelayStorage {
  GpioAddress pos_relay_en;         /**< Positive relay enable */
  GpioAddress pos_relay_sense;      /**< Positive relay sense */

  GpioAddress neg_relay_en;         /**< Negative relay enable */
  GpioAddress neg_relay_sense;      /**< Negative relay sense */

  GpioAddress solar_relay_en;       /**< Solar relay enable */
  GpioAddress solar_relay_sense;    /**< Solar relay sense */

  GpioAddress killswitch_sense;     /**< Killswitch sense */
};

/**
 * @brief   Close POS, NEG, and SOLAR relays in sequence
 * @details There shall be a delay of BMS_CLOSE_RELAYS_DELAY_MS between each relay closure
 *          The killswitch interrupt will also be configured, and if it is already pressed, a fault shall be thrown
 * @param   storage Pointer to the BMS storage
 * @return  STATUS_CODE_OK if state of charge initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode relays_init(BmsStorage *storage);

/**
 * @brief   Open the SOLAR relay
 * @details This is used to prevent overcharging of the pack
 */
void bms_open_solar();

/**
 * @brief   Close the SOLAR relay
 * @details This is used to recover from overcharging of the pack
 */
void bms_close_solar();

/**
 * @brief   Open the POS, NEG and SOLAR relay
 * @details This is called when any BMS fault occurs to disconnect the main pack
 */
void bms_relay_fault(void);

/** @} */
