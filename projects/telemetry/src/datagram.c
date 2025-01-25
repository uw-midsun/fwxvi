/************************************************************************************************
 * @file   datagram.c
 *
 * @brief  Datagram source file for telemetry
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "datagram.h"

#include "log.h"

/* Intra-component Headers */
#include "telemetry.h"

void decode_can_message(Datagram *datagram, CanMessage *msg) {
  datagram->start_frame = DATAGRAM_START_FRAME;
  datagram->id = msg->id.raw;
  datagram->dlc = msg->dlc;

  for (size_t i = 0; i < msg->dlc; ++i) {
    datagram->data[i] = msg->data_u8[i];
  }

  datagram->data[msg->dlc] = DATAGRAM_END_FRAME;
}
