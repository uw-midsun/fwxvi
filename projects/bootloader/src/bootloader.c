/************************************************************************************************
 * @file   bootloader.c
 *
 * @brief  Source file for the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <string.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "bootloader.h"
#include "bootloader_can.h"
#include "bootloader_can_datagram.h"
#include "bootloader_crc32.h"
#include "bootloader_flash.h"
#include "bootloader_packet_handler.h"
#include "bootloader_dfu.h"

#if BOOTLOADER_LOOPBACK_TESTS_ENABLE == 1
#ifndef BOOTLOADER_LOOPBACK_ENABLE_FLASH_WRITE_TEST
#define BOOTLOADER_LOOPBACK_ENABLE_FLASH_WRITE_TEST 1
#endif

#define LOOPBACK_RX_TIMEOUT_MS 100U
#define LOOPBACK_DRAIN_LIMIT   16U
#define LOOPBACK_NODE_MASK     0x0001U
#define LOOPBACK_TEST_DATA_LEN 8U

extern uint32_t HAL_GetTick(void);

#define BOOTLOADER_LOOPBACK_TEST_MAX 8U
#define BOOTLOADER_LOOPBACK_TEST_NO_ACK 0xFFU

typedef struct {
  const char *name;
  uint32_t tx_id;
  BootloaderError expected_result;
  BootloaderError actual_result;
  uint8_t expected_ack; /* For ack responses */
  uint8_t actual_ack;
  BootloaderError actual_ack_error;
  BootloaderStates state_after;
  uint32_t bytes_written;
  uint16_t expected_sequence_number;
  bool passed;
} BootloaderLoopbackTestStep;

typedef struct {
  uint32_t count;
  uint32_t passed;
  uint32_t failed;
  BootloaderError final_result;
  BootloaderLoopbackTestStep steps[BOOTLOADER_LOOPBACK_TEST_MAX];
} BootloaderLoopbackTestResults;

static BootloaderLoopbackTestResults s_bootloader_loopback_results;
#endif


/** @brief Store CAN traffic in 2048 byte buffer to write to flash */
static uint8_t flash_buffer[BOOTLOADER_PAGE_BYTES];

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

#if BOOTLOADER_LOOPBACK_TESTS_ENABLE == 1
static void s_pack_u16(uint8_t *data, uint16_t value) {
  data[0] = (uint8_t)value;
  data[1] = (uint8_t)(value >> 8);
}

static void s_pack_u32(uint8_t *data, uint32_t value) {
  data[0] = (uint8_t)value;
  data[1] = (uint8_t)(value >> 8);
  data[2] = (uint8_t)(value >> 16);
  data[3] = (uint8_t)(value >> 24);
}

static void s_drain_loopback_rx(Boot_CanMessage *msg) {
  for (uint32_t i = 0U; i < LOOPBACK_DRAIN_LIMIT; i++) {
    if (boot_can_receive(msg) != BOOTLOADER_ERROR_NONE) {
      return;
    }
  }
}

static BootloaderError s_receive_frame(Boot_CanMessage *msg, uint32_t timeout_ms) {
  uint32_t start = HAL_GetTick();
  while ((HAL_GetTick() - start) < timeout_ms) {
    if (boot_can_receive(msg) == BOOTLOADER_ERROR_NONE) {
      return BOOTLOADER_ERROR_NONE;
    }
  }

  return BOOTLOADER_CAN_RECEIVE_ERROR;
}

static BootloaderError s_send_and_run(uint32_t id, const uint8_t data[8], Boot_CanMessage *msg) {
  s_drain_loopback_rx(msg);

  BootloaderError err = boot_can_transmit(id, false, data, 8);
  if (err != BOOTLOADER_ERROR_NONE) {
    return err;
  }

  err = s_receive_frame(msg, LOOPBACK_RX_TIMEOUT_MS);
  if (err != BOOTLOADER_ERROR_NONE) {
    return err;
  }

  return bootloader_run(msg);
}

/* Since in loopback, any acks need to be received */
static void s_read_loopback_ack(BootloaderLoopbackTestStep *step, Boot_CanMessage *msg) {
  step->actual_ack = BOOTLOADER_LOOPBACK_TEST_NO_ACK;
  step->actual_ack_error = BOOTLOADER_ERROR_NONE;

  if (step->expected_ack == BOOTLOADER_LOOPBACK_TEST_NO_ACK) {
    return;
  }

  if (s_receive_frame(msg, LOOPBACK_RX_TIMEOUT_MS) != BOOTLOADER_ERROR_NONE ||
      msg->id != BOOTLOADER_CAN_ACK_ID) {
    step->actual_ack_error = BOOTLOADER_CAN_RECEIVE_ERROR;
    return;
  }

  step->actual_ack = msg->data_u8[0];
  step->actual_ack_error = (BootloaderError)(((uint16_t)msg->data_u8[1] << 8) |
                                             (uint16_t)msg->data_u8[2]);
}

static BootloaderError s_record_loopback_step(uint32_t index, const char *name, uint32_t tx_id,
                                              const uint8_t data[8], BootloaderError expected_result,
                                              uint8_t expected_ack, Boot_CanMessage *msg) {
  BootloaderLoopbackTestStep *step = &s_bootloader_loopback_results.steps[index];

  step->name = name;
  step->tx_id = tx_id;
  step->expected_result = expected_result;
  step->expected_ack = expected_ack;
  step->actual_result = s_send_and_run(tx_id, data, msg);
  s_read_loopback_ack(step, msg);
  step->state_after = current_state;
  step->bytes_written = packet_manager.bytes_written;
  step->expected_sequence_number = packet_manager.expected_sequence_number;

  step->passed = (step->actual_result == expected_result) &&
                 (step->expected_ack == BOOTLOADER_LOOPBACK_TEST_NO_ACK ||
                  (step->actual_ack == step->expected_ack &&
                   step->actual_ack_error == expected_result));

  if (step->passed) {
    s_bootloader_loopback_results.passed++;
    return BOOTLOADER_ERROR_NONE;
  }

  s_bootloader_loopback_results.failed++;
  return BOOTLOADER_INTERNAL_ERR;
}

static void s_build_start_frame(uint8_t data[8], uint32_t image_len) {
  memset(data, 0, 8U);
  s_pack_u16(&data[0], LOOPBACK_NODE_MASK);
  s_pack_u32(&data[2], image_len);
}

static void s_build_sequence_frame(uint8_t data[8], uint16_t sequence_num, uint32_t crc32) {
  memset(data, 0, 8U);
  s_pack_u16(&data[0], sequence_num);
  s_pack_u32(&data[2], crc32);
}

static BootloaderError s_prepare_started_session(Boot_CanMessage *msg, uint32_t image_len) {
  uint8_t start_frame[8] = { 0U };
  s_build_start_frame(start_frame, image_len);

  BootloaderError err = s_send_and_run(BOOTLOADER_CAN_START_ID, start_frame, msg);
  if (err != BOOTLOADER_ERROR_NONE) {
    return err;
  }

  BootloaderLoopbackTestStep ack_step = { .expected_ack = ACK };
  s_read_loopback_ack(&ack_step, msg);
  if (ack_step.actual_ack != ACK || ack_step.actual_ack_error != BOOTLOADER_ERROR_NONE) {
    return BOOTLOADER_CAN_RECEIVE_ERROR;
  }

  return BOOTLOADER_ERROR_NONE;
}

static BootloaderError s_prepare_sequence(Boot_CanMessage *msg, uint16_t sequence_num, uint32_t crc32) {
  uint8_t sequence_frame[8] = { 0U };
  s_build_sequence_frame(sequence_frame, sequence_num, crc32);

  return s_send_and_run(BOOTLOADER_CAN_SEQUENCING_ID, sequence_frame, msg);
}
#endif

/** @brief Switches bootloader states based on arbitration ID */
static BootloaderError s_switch_states(BootloaderCanID arbitration_id) {
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

/** @brief Executes instructions based on current state */
static BootloaderError s_run_state() {
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

BootloaderError bootloader_init(void) {
  BootloaderError error = BOOTLOADER_ERROR_NONE;
  current_state = BOOTLOADER_IDLE;

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

  ret = s_switch_states(msg->id);
  if (ret != BOOTLOADER_ERROR_NONE) {
    current_state = BOOTLOADER_FAULT;
  }

  ret = s_run_state();

  if (ret != BOOTLOADER_ERROR_NONE) {
    current_state = BOOTLOADER_FAULT;
  }

  return ret;
}

//TODO DELETE After actual testing is done.
#if BOOTLOADER_LOOPBACK_TESTS_ENABLE == 1
BootloaderError bootloader_loopback_tests(Boot_CanMessage *msg) {
  if (msg == NULL) {
    return BOOTLOADER_INVALID_ARGS;
  }

  memset(&s_bootloader_loopback_results, 0, sizeof(s_bootloader_loopback_results));
  BootloaderError final_result = BOOTLOADER_ERROR_NONE;
  uint32_t test_index = 0U;
  uint8_t frame[8] = { 0U };

  s_drain_loopback_rx(msg);

  bootloader_init();
  s_build_start_frame(frame, LOOPBACK_TEST_DATA_LEN);
  if (s_record_loopback_step(test_index++, "start_valid", BOOTLOADER_CAN_START_ID,
                             frame, BOOTLOADER_ERROR_NONE, ACK, msg) != BOOTLOADER_ERROR_NONE) {
    final_result = BOOTLOADER_INTERNAL_ERR;
  }

  /* reinit to start a clean slate */
  bootloader_init();
  s_build_start_frame(frame, LOOPBACK_TEST_DATA_LEN - 1U);
  if (s_record_loopback_step(test_index++, "start_unaligned", BOOTLOADER_CAN_START_ID,
                             frame, BOOTLOADER_DATA_NOT_ALIGNED, NACK, msg) != BOOTLOADER_ERROR_NONE) {
    final_result = BOOTLOADER_INTERNAL_ERR;
  }

  bootloader_init();
  if (s_prepare_started_session(msg, LOOPBACK_TEST_DATA_LEN) == BOOTLOADER_ERROR_NONE) {
    s_build_sequence_frame(frame, 0U, 0U);
    if (s_record_loopback_step(test_index++, "sequence_valid", BOOTLOADER_CAN_SEQUENCING_ID,
                               frame, BOOTLOADER_ERROR_NONE, BOOTLOADER_LOOPBACK_TEST_NO_ACK, msg) != BOOTLOADER_ERROR_NONE) {
      final_result = BOOTLOADER_INTERNAL_ERR;
    }
  } else {
    final_result = BOOTLOADER_INTERNAL_ERR;
  }

  bootloader_init();
  if (s_prepare_started_session(msg, LOOPBACK_TEST_DATA_LEN) == BOOTLOADER_ERROR_NONE) {
    s_build_sequence_frame(frame, 1U, 0U);
    if (s_record_loopback_step(test_index++, "sequence_wrong", BOOTLOADER_CAN_SEQUENCING_ID,
                               frame, BOOTLOADER_SEQUENCE_ERROR, NACK, msg) != BOOTLOADER_ERROR_NONE) {
      final_result = BOOTLOADER_INTERNAL_ERR;
    }
  } else {
    final_result = BOOTLOADER_INTERNAL_ERR;
  }

  bootloader_init();
  if (s_prepare_started_session(msg, LOOPBACK_TEST_DATA_LEN) == BOOTLOADER_ERROR_NONE &&
      s_prepare_sequence(msg, 0U, 0U) == BOOTLOADER_ERROR_NONE) {
    const uint8_t flash_frame[8] = { 0x12U, 0x34U, 0x56U, 0x78U, 0x9AU, 0xBCU, 0xDEU, 0xF0U };
    if (s_record_loopback_step(test_index++, "flash_bad_crc", BOOTLOADER_CAN_FLASH_ID,
                               flash_frame, BOOTLOADER_CRC_MISMATCH_BEFORE_WRITE, NACK, msg) != BOOTLOADER_ERROR_NONE) {
      final_result = BOOTLOADER_INTERNAL_ERR;
    }
  } else {
    final_result = BOOTLOADER_INTERNAL_ERR;
  }

#if BOOTLOADER_LOOPBACK_ENABLE_FLASH_WRITE_TEST == 1
  bootloader_init();
  const uint8_t valid_flash_frame[8] = { 0xABU, 0xCDU, 0xEFU, 0x12U, 0x34U, 0x56U, 0x78U, 0x90U };
  uint32_t crc32 = boot_crc32_calculate((const uint32_t *)valid_flash_frame,
                                        BYTES_TO_WORD(sizeof(valid_flash_frame)));
  if (s_prepare_started_session(msg, LOOPBACK_TEST_DATA_LEN) == BOOTLOADER_ERROR_NONE &&
      s_prepare_sequence(msg, 0U, crc32) == BOOTLOADER_ERROR_NONE) {
    if (s_record_loopback_step(test_index++, "flash_write_valid", BOOTLOADER_CAN_FLASH_ID,
                               valid_flash_frame, BOOTLOADER_ERROR_NONE, ACK, msg) != BOOTLOADER_ERROR_NONE) {
      final_result = BOOTLOADER_INTERNAL_ERR;
    }
  } else {
    final_result = BOOTLOADER_INTERNAL_ERR;
  }
#endif

  s_bootloader_loopback_results.count = test_index;
  s_bootloader_loopback_results.final_result = final_result;
  s_drain_loopback_rx(msg);

  return final_result;
}
#endif
