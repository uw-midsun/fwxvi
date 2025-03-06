#pragma once

/************************************************************************************************
 * @file   imu.h
 *
 * @brief  Header file for imu
 *
 * @date   2025-02-01
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "can.h"
#include "spi.h"

/* Intra-component Headers */
#include "bmi323.h"

/**
 * @defgroup imu
 * @brief    imu Firmware
 * @{
 */

StatusCode imu_init(Bmi323Storage *storage, Bmi323Settings *settings);

/** @} */
