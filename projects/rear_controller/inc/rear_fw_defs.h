#pragma once

/************************************************************************************************
 * @file   rear_fw_defs.h
 *
 * @brief  Rear controller board hardware definitions
 *
 * @date   2025-05-28
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup Rear Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

/** @brief  Precharge pin */
#define REAR_CONTROLLER_PRECHARGE_GPIO \
  { .port = GPIO_PORT_A, .pin = 3 }

/** @brief  Killswitch pin */
#define REAR_CONTROLLER_KILLSWITCH_GPIO \
  { .port = GPIO_PORT_A, .pin = 11 }


/** @} */
