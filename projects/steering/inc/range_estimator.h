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
 * @brief   Run the range estimator which estimates remaining kilometres based on cell voltage
 */
StatusCode range_estimator_run();

StatusCode range_estimator_init();

/** @} */
