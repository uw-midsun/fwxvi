#pragma once

/************************************************************************************************
 * @file   telemetry_log.h
 *
 * @brief  Buffered candump logging for received CAN frames
 *
 * @date   2026-10-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Inter-component Headers */
#include "can_msg.h"
#include "status.h"

#define TELEMETRY_LOG_QUEUE_SIZE 128U

/** @note Initialize before enabling the CAN receive callback. The drive must be mounted */
StatusCode telemetry_log_init(const char *drive, uint32_t reboot_number);

/** @brief Copy and timestamp a received frame in task context */
StatusCode telemetry_log_can(const CanMessage *message);

/** @brief Write queued frames to logs_<reboot_number>.log */
StatusCode telemetry_log_sd(void);

/** @brief Stop capture, drain pending frames and close the file */
StatusCode telemetry_log_close(void);

/** @brief Number of frames dropped because the SD logging queue was full */
uint32_t telemetry_log_dropped(void);
