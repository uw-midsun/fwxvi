/************************************************************************************************
 * @file   telemetry_log.c
 *
 * @brief  Buffered candump logging for received CAN frames
 *
 * @date   2026-10-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "ff.h"
#include "queues.h"
#include "task.h"

/* Intra-component Headers */
#include "telemetry_log.h"

#define LOG_BUFFER_SIZE 1024U
#define LOG_LINE_SIZE 64U
#define LOG_BATCH_MS 20U
#define LOG_WAIT_MS 100U
#define LOG_SYNC_MS 1000U

typedef struct {
  uint32_t seconds;
  uint32_t microseconds;
  uint32_t id;
  uint8_t dlc;
  bool extended;
  uint8_t data[8];
} CanLogRecord;

static CanLogRecord s_records[TELEMETRY_LOG_QUEUE_SIZE];
static Queue s_queue;
static FIL s_file;

static char s_path[32];
static char s_buffer[LOG_BUFFER_SIZE];

static bool s_initialized;
static bool s_accepting;
static bool s_file_open;
static StatusCode s_error;
static uint32_t s_dropped;

static TickType_t s_last_sync;

/** @note A full card is treated as a failure */
static StatusCode s_write(size_t length) {
  if (length == 0U) {
    return STATUS_CODE_OK;
  }

  UINT written = 0U;
  if (f_write(&s_file, s_buffer, length, &written) != FR_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  if (written != length) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  return STATUS_CODE_OK;
}

/** @brief Format one Classic CAN data frame */
static size_t s_format(const CanLogRecord *record, char *line) {
  /* ex: (<seconds>.<microsconds>) can0 <id> # */
  int length = snprintf(line, LOG_LINE_SIZE, "(%lu.%06lu) can0 %0*lu #", record->seconds, record->microseconds, record->extended ? 8 : 3, record->id);

  for (size_t i = 0U; i < record->dlc; ++i) {
    int written = snprintf(line + length, LOG_LINE_SIZE - (size_t)length, "%02X", (unsigned int)record->data[i]);
    if (written < 0 || (size_t)written >= LOG_LINE_SIZE - (size_t)length) {
      return 0U;
    }
    length += written;
  }
  line[length] = '\n';
  ++length;

  return (size_t)length;
}

StatusCode telemetry_log_init(const char *drive, uint32_t reboot_number) {
  if (drive == NULL || reboot_number == 0U) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (s_initialized) {
    return STATUS_CODE_ALREADY_INITIALIZED;
  }

  int length = snprintf(s_path, sizeof(s_path), "%slogs_%lu.log", drive, reboot_number);
  if (length < 0 || (size_t)length >= sizeof(s_path)) {
    return STATUS_CODE_OUT_OF_RANGE;
  }

  s_queue = (Queue){ .num_items = TELEMETRY_LOG_QUEUE_SIZE, .item_size = sizeof(CanLogRecord), .storage_buf = (uint8_t *)s_records };
  status_ok_or_return(queue_init(&s_queue));
  s_file_open = false;
  s_error = STATUS_CODE_OK;
  s_dropped = 0U;
  s_initialized = true;
  s_accepting = true;
  return STATUS_CODE_OK;
}

StatusCode telemetry_log_can(const CanMessage *message) {
  if (message == NULL || message->dlc > 8U) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (!s_accepting) {
    return STATUS_CODE_UNINITIALIZED;
  }

  /* Include the RTOS overflow count so long recordings survive tick wraparound.
   * Six decimal places match candump output; resolution is still one tick. */
  TimeOut_t now;
  vTaskSetTimeOutState(&now);

  uint64_t ticks = (uint64_t)(UBaseType_t)now.xOverflowCount * ((uint64_t)portMAX_DELAY + 1U) + now.xTimeOnEntering;
  CanLogRecord record = {
    .seconds = (uint32_t)(ticks / configTICK_RATE_HZ),
    .microseconds = (uint32_t)((ticks % configTICK_RATE_HZ) * 1000000U / configTICK_RATE_HZ),
    .id = message->id.raw,
    .dlc = message->dlc,
    .extended = message->extended,
  };
  memcpy(record.data, message->data_u8, message->dlc);

  StatusCode status = queue_send(&s_queue, &record, 0U);
  if (status != STATUS_CODE_OK) {
    taskENTER_CRITICAL();
    ++s_dropped;
    taskEXIT_CRITICAL();
  }
  return status;
}

StatusCode telemetry_log_sd(void) {
  if (!s_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }
  if (s_error != STATUS_CODE_OK) {
    return s_error;
  }

  if (!s_file_open) {
    if (f_open(&s_file, s_path, FA_WRITE | FA_CREATE_NEW) != FR_OK) {
      s_error = STATUS_CODE_INTERNAL_ERROR;
      return s_error;
    }
    s_file_open = true;
    s_last_sync = xTaskGetTickCount();
  }

  CanLogRecord record;
  size_t used = 0U;
  StatusCode status = queue_receive(&s_queue, &record, s_accepting ? LOG_WAIT_MS : 0U);
  TickType_t start = xTaskGetTickCount();

  /* Collect a short burst of CAN logs */
  for (size_t count = 0U; status == STATUS_CODE_OK && count < TELEMETRY_LOG_QUEUE_SIZE; ++count) {
    char line[LOG_LINE_SIZE];
    size_t length = s_format(&record, line);
    if (used + length > sizeof(s_buffer)) {
      s_error = s_write(used);
      if (s_error != STATUS_CODE_OK) {
        return s_error;
      }
      used = 0U;
    }
    memcpy(&s_buffer[used], line, length);
    used += length;

    TickType_t elapsed = xTaskGetTickCount() - start;
    if (count + 1U == TELEMETRY_LOG_QUEUE_SIZE || elapsed >= pdMS_TO_TICKS(LOG_BATCH_MS)) {
      break;
    }
    uint32_t wait_ms = s_accepting ? LOG_BATCH_MS - (uint32_t)(elapsed * portTICK_PERIOD_MS) : 0U;
    status = queue_receive(&s_queue, &record, wait_ms);
  }

  s_error = s_write(used);
  if (s_error != STATUS_CODE_OK) {
    return s_error;
  }

  TickType_t now = xTaskGetTickCount();
  if ((TickType_t)(now - s_last_sync) >= pdMS_TO_TICKS(LOG_SYNC_MS)) {
    if (f_sync(&s_file) != FR_OK) {
      s_error = STATUS_CODE_INTERNAL_ERROR;
      return s_error;
    }
    s_last_sync = now;
  }
  return STATUS_CODE_OK;
}

StatusCode telemetry_log_close(void) {
  if (!s_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  s_accepting = false;
  while (s_error == STATUS_CODE_OK && queue_get_spaces_available(&s_queue) < TELEMETRY_LOG_QUEUE_SIZE) {
    telemetry_log_sd();
  }

  if (s_file_open && f_close(&s_file) != FR_OK && s_error == STATUS_CODE_OK) {
    s_error = STATUS_CODE_INTERNAL_ERROR;
  }
  s_file_open = false;
  s_initialized = false;
  return s_error;
}

uint32_t telemetry_log_dropped(void) {
  taskENTER_CRITICAL();
  uint32_t dropped = s_dropped;
  taskEXIT_CRITICAL();
  return dropped;
}
