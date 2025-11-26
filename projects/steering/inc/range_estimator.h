#pragma once

/************************************************************************************************
 * @file   range_estimator.h
 *
 * @brief  Header file for estimating driving range based on remaining voltage
 *
 * @date   2025-10-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup steering
 * @brief    Csteering Firmware
 * @{
 */

/**
 * @brief   Estimate remaining kilometres based on cell voltage
 * @param   min_cell_voltage Minimum cell voltage from the battery telemetry
 * @return  Estimated remaining range (in kilometers)
 */
float estimate_remaining_range_km(void);

/** @} */
