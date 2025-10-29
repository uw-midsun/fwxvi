#pragma once

/************************************************************************************************
 * @file  fota_ack.h
 *
 * @brief  FOTA Acknowledgement handling
 *
 * @date   2025-07-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_datagram.h"
#include "fota_datagram_payloads.h"
#include "fota_error.h"
#include "packet_manager.h"

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

/**
 * @brief   Initialize DFU context
 * @param   packet_manager Pointer to packet manager instance
 * @param   staging_base Flash base address where firmware will be staged
 * @param   app_start_addr Address to jump to after flashing
 * @return  FOTA_ERROR_SUCCESS or FOTA_ERROR_INVALID_ARGS
 */
FotaError fota_dfu_init(PacketManager *packet_manager, uintptr_t staging_base, uintptr_t app_start_addr);

/** @} */
