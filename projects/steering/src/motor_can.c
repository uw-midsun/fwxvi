/************************************************************************************************
 * @file    motor_can.c
 *
 * @brief   Motor Can
 *
 * @date    2025-11-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "ws22_motor_can.h"

/* Intra-component Headers */
#include "motor_can.h"

StatusCode motor_can_process_rx(CanMessage *msg) {
  //   if (msg->id.raw == 0x82) {
  //     LOG_DEBUG("data: %x %x %x %x\r\n", msg->data_u8[0], msg->data_u8[1], msg->data_u8[2], msg->data_u8[3]);
  //   }

  return ws22_motor_can_process_rx(msg->data_u8, msg->id.raw, msg->dlc);
}
