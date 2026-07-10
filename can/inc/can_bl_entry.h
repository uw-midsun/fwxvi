#pragma once

/************************************************************************************************
 * @file    can_bl_entry.h
 *
 * @brief   AUTO GENERATED from board.toml, do not edit, CAN side bootloader entry ids
 *
 * @date    AUTO GENERATED
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/** @brief Arbitration id the enter command arrives on, matches the bootloader BL_ENTER_ID */
#define CAN_BL_ENTER_ID 0x010U

/** @brief Base of the bootloader transfer id block (FRAGMENT, RESPONSE, ACK), matches BL_XFER_ID_BASE */
#define CAN_BL_XFER_ID_BASE 0x100U
