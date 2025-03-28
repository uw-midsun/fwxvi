#pragma once

/************************************************************************************************
 * @file   centre_console.h
 *
 * @brief  Header file for centre_console
 *
 * @date   2025-01-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup centre_console
 * @brief    centre_console Firmware
 * @{
 */

struct SteeringStorage;

typedef struct {
    struct SteeringStorage *steering_storage;
} CentreConsoleStorage;


/** @} */
