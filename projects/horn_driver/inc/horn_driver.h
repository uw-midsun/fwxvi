#pragma once

/************************************************************************************************
 * @file   horn_driver.h
 *
 * @brief  Header file for horn_driver
 *
 * @date   2026-10-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */

/**
 * @defgroup horn_driver
 * @brief    horn_driver Firmware
 * @{
 */

StatusCode horn_driver_init();

StatusCode horn_driver_enable(bool is_enabled);

/** @} */