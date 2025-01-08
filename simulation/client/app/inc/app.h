#pragma once

/************************************************************************************************
 * @file   app.h
 *
 * @brief  Header file defining the Application for the client
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "gpio_manager.h"

/**
 * @defgroup ClientAppMain
 * @brief    Client Application Main Interface
 * @{
 */

#ifndef USE_NETWORK_TIME_PROTOCOL
/** @brief  Define to use network-time-protocol synchronization. 0: Disabled 1: Enabled */
#define USE_NETWORK_TIME_PROTOCOL 0U
#endif

/** @brief  Default project name to be used for the Metadata */
#define DEFAULT_PROJECT_NAME "Default Project"

/** @brief  Default hardware model to be used for the Metadata */
#define DEFAULT_HARDWARE_MODEL "MS16.0.0"

extern GpioManager clientGpioManager; /**< Global GPIO Manager */

/** @} */
