/************************************************************************************************
 * @file   bootloader.c
 *
 * @brief  Source file for the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "bootloader.h"
#include "bootloader_can_datagram.h"
#include "bootloader_crc32.h"
#include "bootloader_flash.h"
#include "bootloader_packet_handler.h"
#include "bootloader_dfu.h"

/** @brief Store CAN traffic in 2048 byte buffer to write to flash */
static uint8_t *flash_buffer;

static BootloaderDatagram datagram; /**< Bootloader datagram */
PacketManager packet_manager;       /**< Store information to receive packets */
BootloaderStates current_state;     /**< The current state */

/** @brief Maps arbitration ID to a state */
static const uint16_t arbitration_id_handle[] = {
  [BOOTLOADER_CAN_START_ID] = BOOTLOADER_START,         
  [BOOTLOADER_CAN_SEQUENCING_ID] = BOOTLOADER_WAIT_SEQUENCING, 
  [BOOTLOADER_CAN_FLASH_ID] = BOOTLOADER_DATA_RECEIVE,
  [BOOTLOADER_CAN_JUMP_APPLICATION_ID] = BOOTLOADER_JUMP_APP, 
  [BOOTLOADER_CAN_PING_DATA_ID] = BOOTLOADER_PING,
};

/** @brief Executes instructions based on current state */
static BootloaderError bootloader_run_state(); 
/** @brief Switches bootloader states based on arbitration ID */
static BootloaderError bootloader_switch_states(BootloaderCanID arbitration_id); 

BootloaderError bootloader_init(void) {
  BootloaderError error = BOOTLOADER_ERROR_NONE; 

  error = packet_handler_init(&packet_manager); 
  error = bootloader_dfu_init(packet_manager.current_write_address); 
  error = boot_crc32_init();

  return error;
}

BootloaderError bootloader_run(Boot_CanMessage *msg) {
  BootloaderError ret = BOOTLOADER_ERROR_NONE;

  if (current_state == BOOTLOADER_UNINITIALIZED) {
    return BOOTLOADER_ERROR_UNINITIALIZED;
  }

  datagram = deserialize_datagram(msg, &packet_manager.target_nodes);

  // if (!(prv_bootloader.target_nodes & (1 << _node_id))) {
  //   // Not the target node
  //   return BOOTLOADER_ERROR_NONE;
  // }

  ret = bootloader_switch_states(msg->id);
  ret = bootloader_run_state();

  if (ret != BOOTLOADER_ERROR_NONE) {
    current_state = BOOTLOADER_FAULT; 
  }

  return ret;
}

static BootloaderError bootloader_switch_states(BootloaderCanID arbitration_id) { 
  BootloaderError return_err = BOOTLOADER_ERROR_NONE;
  BootloaderStates new_state = arbitration_id_handle[arbitration_id];

  if (current_state == new_state) {
    return return_err;
  }

  switch (current_state) {
    case BOOTLOADER_IDLE:
      if (new_state == BOOTLOADER_JUMP_APP || new_state == BOOTLOADER_START || new_state == BOOTLOADER_FAULT || new_state == BOOTLOADER_PING) {
        current_state = new_state;
      } else {
        return_err = BOOTLOADER_INVALID_ARGS;
        current_state = BOOTLOADER_FAULT;
      }
      break;

    case BOOTLOADER_START:
      if (new_state == BOOTLOADER_JUMP_APP || new_state == BOOTLOADER_WAIT_SEQUENCING || new_state == BOOTLOADER_FAULT) {
        current_state = new_state;
      } else {
        return_err = BOOTLOADER_INVALID_ARGS;
        current_state = BOOTLOADER_FAULT;
      }
      break;

    case BOOTLOADER_WAIT_SEQUENCING:
      // Should be able to go to all states
      current_state = new_state;
      break;

    case BOOTLOADER_DATA_RECEIVE:
      if (new_state == BOOTLOADER_START || new_state == BOOTLOADER_JUMP_APP || new_state == BOOTLOADER_WAIT_SEQUENCING || new_state == BOOTLOADER_FAULT) {
        current_state = new_state;
      } else {
        return_err = BOOTLOADER_INVALID_ARGS;
        current_state = BOOTLOADER_FAULT;
      }
      break;

    case BOOTLOADER_JUMP_APP:
      if (new_state == BOOTLOADER_FAULT) {
        current_state = new_state;
      } else {
        return_err = BOOTLOADER_INVALID_ARGS;
        current_state = BOOTLOADER_FAULT;
      }
      break;

    case BOOTLOADER_UNINITIALIZED:
      if (new_state == BOOTLOADER_IDLE) {
        current_state = new_state;
      } else {
        return_err = BOOTLOADER_INVALID_ARGS;
        current_state = BOOTLOADER_FAULT;
      }
      break;

    case BOOTLOADER_PING:
    if (new_state == BOOTLOADER_START || new_state == BOOTLOADER_JUMP_APP ||
        new_state == BOOTLOADER_WAIT_SEQUENCING || new_state == BOOTLOADER_FAULT ||
        new_state == BOOTLOADER_PING) {
      current_state = new_state;
    } else {
      return_err = BOOTLOADER_INVALID_ARGS;
      current_state = BOOTLOADER_FAULT;
    }
    break;

    default:
      return_err = BOOTLOADER_INVALID_ARGS;
      current_state = BOOTLOADER_FAULT;
      break;
  }

  return return_err;
}

static BootloaderError bootloader_run_state() {
  switch (current_state) {
    case BOOTLOADER_UNINITIALIZED:
      return BOOTLOADER_INVALID_ARGS;
    case BOOTLOADER_IDLE:
      return BOOTLOADER_ERROR_NONE;
    case BOOTLOADER_START:
      return bootloader_start(&packet_manager, &datagram); 
    case BOOTLOADER_WAIT_SEQUENCING:
      return bootloader_wait_sequencing(&packet_manager, &datagram);
    case BOOTLOADER_DATA_RECEIVE:
      return bootloader_receive_data(&packet_manager, &datagram, flash_buffer);
    case BOOTLOADER_JUMP_APP:
      return bootloader_jump_app();
    case BOOTLOADER_FAULT:
      return bootloader_fault();
    case BOOTLOADER_PING:
      return bootloader_ping(&packet_manager, &datagram, flash_buffer);
    default:
      return BOOTLOADER_INTERNAL_ERR;
  }
}

