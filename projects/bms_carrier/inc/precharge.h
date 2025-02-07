#pragma once

/************************************************************************************************
 * @file   precharge.h
 *
 * @brief  Header file for precharge
 *
 * @date   2025-02-06
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "gpio.h"
#include "notify.h"
#include "status.h"

/* Intra-component Headers */
#include "bms_carrier.h"

/**
 * @defgroup motor_controller
 * @brief    motor_controller Firmware
 * @{
 */

/** @brief  Precharge interrupt notification */
#define PRECHARGE_EVENT_IT  0U

/**
 * @brief   Precharge settings
 */
struct PrechargeSettings {
  GpioAddress motor_sw_en;          /**< Motor switch enable */
  GpioAddress motor_sw_sense;       /**< Motor switch sense */
  GpioAddress precharge_monitor;    /**< Precharge monitor */
};

/**
 * @brief   Initialize precharge settings
 * @details If precharge has already occurred during startup, an interrupt shall not be registered
 *          If precharge has not occurred, an interrupt shall be registered
 * @param   storage Pointer to the BMS storage
 * @return  STATUS_CODE_OK
 */
StatusCode precharge_init(BmsStorage *storage, Event event, const Task *task);

/** @} */
