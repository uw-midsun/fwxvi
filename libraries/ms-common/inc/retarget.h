#pragma once

/************************************************************************************************
 * @file   retarget.h
 *
 * @brief  Retarget library for Hard fault handling
 *
 * @date   2025-01-28
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup MCU
 * @brief    MCU library
 * @{
 */

/**
 * @brief   Initialze the UART interface for hard fault register dumping
 */
void retarget_init();

/** @} */
