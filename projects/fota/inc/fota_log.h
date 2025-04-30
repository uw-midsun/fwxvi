#pragma once

/************************************************************************************************
 * @file   fota_log.h
 *
 * @brief  Header file for FOTA logging system
 *
 * @date   2025-03-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

/** @brief  Debug log */
#define LOG_FOTA_DEBUG(msg) printf("[FOTA DEBUG]: %s\n");
/** @brief Warning log */
#define LOG_FOTA_WARNING(msg) printf("[FOTA WARNING]: %s\n");
/** @brief Error log */
#define LOG_FOTA_ERROR(msg) printf("[FOTA ERROR]: %s\n");

/** @} */
