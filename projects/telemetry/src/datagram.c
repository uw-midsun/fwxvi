/************************************************************************************************
 * @file   datagram.c
 *
 * @brief  Datagram source file for telemetry
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "delay.h"
#include "global_enums.h"
#include "log.h"

/* Intra-component Headers */
#include "datagram.h"
#include "telemetry.h"

/** @brief  Flip the endianess of 2 bytes. Used to flip the CAN ID from little-endian to big-endian */
#define FLIP_ENDIANESS_2BYTES(val) (((val & 0xFFU) << 8U) | ((val >> 8U) & 0xFFU))

StatusCode decode_can_message(Datagram *datagram, CanMessage *msg) {
  datagram->start_frame = DATAGRAM_START_FRAME;
  datagram->id = FLIP_ENDIANESS_2BYTES(msg->id.raw);

  if (IS_EXCLUDED_WS22_CAN_ID(msg->id.raw)) {
    return STATUS_CODE_INVALID_ARGS;
  }

  datagram->dlc = msg->dlc;

  for (size_t i = 0; i < msg->dlc; ++i) {
    datagram->data[i] = msg->data_u8[i];
  }

  datagram->data[datagram->dlc] = DATAGRAM_END_FRAME;

  return STATUS_CODE_OK;
}

void log_decoded_message(Datagram *datagram) {
  delay_ms(5);
  LOG_DEBUG("Start frame: 0x%0x. End frame: 0x%0x\n", datagram->start_frame, datagram->data[datagram->dlc]);
  delay_ms(10U);
  LOG_DEBUG("ID: 0x%0x\n", datagram->id);
  delay_ms(10U);
  LOG_DEBUG("Datagram DLC: 0x%0x\n", datagram->dlc);
  delay_ms(10U);
}
