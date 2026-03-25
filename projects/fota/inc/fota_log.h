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
#define LOG_FOTA_DEBUG(fmt, ...) printf("[FOTA DEBUG] %s:%u: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
/** @brief Warning log */
#define LOG_FOTA_WARNING(fmt, ...) printf("[FOTA WARNING] %s:%u: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
/** @brief Error log */
#define LOG_FOTA_ERROR(fmt, ...) printf("[FOTA ERROR] %s:%u: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

/** @} */
