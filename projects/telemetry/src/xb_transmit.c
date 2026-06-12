/************************************************************************************************
 * @file    xb_transmit.c
 *
 * @brief   XBee transmit header file
 *
 * @date    2026-02-11
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "can.h"
#include "global_enums.h"
#include "log.h"
#include "tasks.h"
#include "uart.h"

/* Intra-component Headers */
#include "can_cache.h"
#include "datagram.h"
#include "telemetry.h"
#include "ws22_motor_can.h"
#include "xb_transmit.h"

static TelemetryStorage *s_telemetry_storage = NULL;

static size_t s_sched_idx[NUM_CAN_MSG_PRIORITIES];

/* Written by Task A, snapshot+reset by Task B every second. */
static volatile uint32_t s_can_rx_count = 0U;

/* Task B only — no cross-task sync needed. */
static uint32_t s_xbee_tx_count = 0U;
static uint32_t s_xbee_bytes_sent = 0U;

#define FC_RECENTLY_WRITTEN_MS 5000U
#define TELEMETRY_STATS_INTERVAL_MS 1000U
#define WS22_UPDATE_INTERVAL_MS 500U
#define XBEE_MAX_BYTES_PER_SEC (230400U / 10U) /* 8N1: 10 bits per byte */

typedef struct {
  uint32_t can_id;
  const char *name;
} FcMsgName;

static const FcMsgName s_fc_names[] = {
  { 1365U, "drive_status" },
  { 1525U, "fc_power_group_A" },
  { 1573U, "fc_power_lights_group" },
  { 1621U, "fc_power_group_B" },
};

/* Sends next pending entry from tier; returns bytes transmitted, 0 if none pending.
   Entries with tx_cooldown_ticks > 0 are skipped until the cooldown has elapsed. */
static size_t s_try_send_from_tier(CanMessageCache *cache, size_t size, size_t *start_idx) {
  if (size == 0U) {
    return 0U;
  }

  uint32_t now = (uint32_t)xTaskGetTickCount();

  for (size_t checked = 0U; checked < size; checked++) {
    size_t i = (*start_idx + checked) % size;

    bool was_pending = false;
    uint32_t saved_id = 0U;
    uint8_t saved_dlc = 0U;
    uint8_t saved_data[8] = { 0 };

    taskENTER_CRITICAL();
    if (cache[i].pending) {
      if (cache[i].tx_cooldown_ticks > 0U && (now - cache[i].last_tx_tick) < cache[i].tx_cooldown_ticks) {
        taskEXIT_CRITICAL();
        continue; /* cooldown not elapsed — skip, leave pending */
      }
      was_pending = true;
      saved_id = cache[i].can_id;
      saved_dlc = cache[i].length;
      memcpy(saved_data, cache[i].data, sizeof(saved_data));
      cache[i].pending = false;
      cache[i].last_tx_tick = now;
    }
    taskEXIT_CRITICAL();

    if (was_pending) {
      *start_idx = (i + 1U) % size;
      Datagram tx_datagram = { 0 };
      if (encode_datagram(&tx_datagram, saved_id, saved_dlc, saved_data) == STATUS_CODE_OK) {
        size_t datagram_length = saved_dlc + DATAGRAM_METADATA_SIZE;
        uart_tx(s_telemetry_storage->config->uart_port, (uint8_t *)&tx_datagram, datagram_length);
        return datagram_length;
      }
    }
  }
  return 0U;
}

static void s_update_ws22_entry(uint32_t can_id, const uint8_t *data) {
  for (size_t i = 0U; i < g_can_cache_low_size; i++) {
    if (g_can_cache_low[i].can_id == can_id) {
      taskENTER_CRITICAL();
      memcpy(g_can_cache_low[i].data, data, WS22_TELEMETRY_DLC);
      g_can_cache_low[i].pending = true;
      taskEXIT_CRITICAL();
      return;
    }
  }
}

static void s_update_ws22_cache(void) {
  Ws22MotorTelemetryData *tel = ws22_motor_can_get_telemetry_data();
  Ws22MotorControlData *ctrl = ws22_motor_can_get_control_data();
  uint8_t buf[WS22_TELEMETRY_DLC];
  uint16_t val;

  /* Status: error_flags (u16) | limit_flags (u16) */
  val = tel->error_flags;
  buf[0] = (uint8_t)(val & 0xFFU);
  buf[1] = (uint8_t)(val >> 8U);
  val = tel->limit_flags;
  buf[2] = (uint8_t)(val & 0xFFU);
  buf[3] = (uint8_t)(val >> 8U);
  s_update_ws22_entry(WS22_TELEMETRY_STATUS_ID, buf);

  /* Bus: voltage in centivolt (u16) | current in centiamp (i16) */
  val = (uint16_t)(tel->bus_voltage * 100.0f);
  buf[0] = (uint8_t)(val & 0xFFU);
  buf[1] = (uint8_t)(val >> 8U);
  val = (uint16_t)((int16_t)(tel->bus_current * 100.0f));
  buf[2] = (uint8_t)(val & 0xFFU);
  buf[3] = (uint8_t)(val >> 8U);
  s_update_ws22_entry(WS22_TELEMETRY_BUS_ID, buf);

  /* Velocity: motor_velocity rpm×10 (i16) | vehicle_velocity kph×100 (u16) */
  val = (uint16_t)((int16_t)(tel->motor_velocity * 10.0f));
  buf[0] = (uint8_t)(val & 0xFFU);
  buf[1] = (uint8_t)(val >> 8U);
  val = (uint16_t)(tel->vehicle_velocity_kph * 100.0f);
  buf[2] = (uint8_t)(val & 0xFFU);
  buf[3] = (uint8_t)(val >> 8U);
  s_update_ws22_entry(WS22_TELEMETRY_VELOCITY_ID, buf);

  /* Temperature: motor_temp °C×10 (i16) | heatsink_temp °C×10 (i16) */
  val = (uint16_t)((int16_t)(tel->motor_temp * 10.0f));
  buf[0] = (uint8_t)(val & 0xFFU);
  buf[1] = (uint8_t)(val >> 8U);
  val = (uint16_t)((int16_t)(tel->heat_sink_temp * 10.0f));
  buf[2] = (uint8_t)(val & 0xFFU);
  buf[3] = (uint8_t)(val >> 8U);
  s_update_ws22_entry(WS22_TELEMETRY_TEMP_ID, buf);

  /* Drive command: current×10000 (u16) | velocity_rpm (i16) */
  val = (uint16_t)(ctrl->current * 10000.0f);
  buf[0] = (uint8_t)(val & 0xFFU);
  buf[1] = (uint8_t)(val >> 8U);
  val = (uint16_t)((int16_t)ctrl->velocity);
  buf[2] = (uint8_t)(val & 0xFFU);
  buf[3] = (uint8_t)(val >> 8U);
  s_update_ws22_entry(WS22_TELEMETRY_DRIVE_CMD_ID, buf);
}

static void s_update_stats_cache(uint16_t can_rx_rate, uint16_t xbee_tx_rate, uint8_t xbee_load_pct) {
  uint8_t stats[TELEMETRY_STATS_DLC];
  stats[0] = (uint8_t)(can_rx_rate & 0xFFU);
  stats[1] = (uint8_t)(can_rx_rate >> 8U);
  stats[2] = (uint8_t)(xbee_tx_rate & 0xFFU);
  stats[3] = (uint8_t)(xbee_tx_rate >> 8U);
  stats[4] = xbee_load_pct;
  stats[5] = 0U;

  size_t low_size = g_can_cache_low_size;
  for (size_t i = 0U; i < low_size; i++) {
    if (g_can_cache_low[i].can_id == TELEMETRY_STATS_CAN_ID) {
      taskENTER_CRITICAL();
      memcpy(g_can_cache_low[i].data, stats, TELEMETRY_STATS_DLC);
      g_can_cache_low[i].pending = true;
      taskEXIT_CRITICAL();
      break;
    }
  }
}

/* Task A: drains CAN RX queue into the message cache. */
TASK(can_cache_updater, TASK_STACK_512) {
  CanMessageCache *caches[] = { g_can_cache_high, g_can_cache_medium, g_can_cache_low };
  const size_t sizes[] = { g_can_cache_high_size, g_can_cache_medium_size, g_can_cache_low_size };
  CanMessage message = { 0 };

  while (true) {
    while (queue_receive(&s_telemetry_storage->can_storage->rx_queue.queue, &message, QUEUE_DELAY_BLOCKING) != STATUS_CODE_OK) {
    }

    taskENTER_CRITICAL();
    s_can_rx_count++;
    taskEXIT_CRITICAL();

    bool is_ws22 = IS_WS22_CAN_ID(message.id.raw);

    if (is_ws22) {
      ws22_motor_can_process_rx(message.data_u8, message.id.raw, 8U);
    } else {
      for (int p = 0; p < NUM_CAN_MSG_PRIORITIES; p++) {
        bool matched = false;
        for (size_t i = 0U; i < sizes[p]; i++) {
          if (caches[p][i].can_id == message.id.raw) {
            taskENTER_CRITICAL();
            caches[p][i].last_updated_tick = (uint32_t)xTaskGetTickCount();
            if (!caches[p][i].pending) {
              memcpy(caches[p][i].data, message.data_u8, sizeof(caches[p][i].data));
              caches[p][i].pending = true;
            }
            taskEXIT_CRITICAL();
            matched = true;
            break;
          }
        }

        if (matched) {
          break;
        }
      }
    }
  }
}

/* Task B: round-robin HIGH→MEDIUM→LOW scheduler; updates bus-load stats cache every second. */
TASK(can_cache_scheduler, TASK_STACK_512) {
  CanMessageCache *caches[] = { g_can_cache_high, g_can_cache_medium, g_can_cache_low };
  const size_t sizes[] = { g_can_cache_high_size, g_can_cache_medium_size, g_can_cache_low_size };
  uint32_t last_stats_tick = (uint32_t)xTaskGetTickCount();
  uint32_t last_ws22_tick = (uint32_t)xTaskGetTickCount();

  while (true) {
    for (int p = 0; p < NUM_CAN_MSG_PRIORITIES; p++) {
      size_t bytes = s_try_send_from_tier(caches[p], sizes[p], &s_sched_idx[p]);
      if (bytes > 0U) {
        s_xbee_tx_count++;
        s_xbee_bytes_sent += (uint32_t)bytes;
        break;
      }
    }

    uint32_t now = (uint32_t)xTaskGetTickCount();
    if ((now - last_stats_tick) >= TELEMETRY_STATS_INTERVAL_MS) {
      uint32_t rx_snapshot;
      taskENTER_CRITICAL();
      rx_snapshot = s_can_rx_count;
      s_can_rx_count = 0U;
      taskEXIT_CRITICAL();

      uint8_t load_pct = (uint8_t)((s_xbee_bytes_sent * 100U) / XBEE_MAX_BYTES_PER_SEC);
      s_update_stats_cache((uint16_t)rx_snapshot, (uint16_t)s_xbee_tx_count, load_pct);
      s_xbee_tx_count = 0U;
      s_xbee_bytes_sent = 0U;
      last_stats_tick = now;
    }

    if ((now - last_ws22_tick) >= WS22_UPDATE_INTERVAL_MS) {
      s_update_ws22_cache();
      last_ws22_tick = now;
    }

    /* 1ms gap prevents HAL semaphore desync at 230400 baud. */
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

#ifdef LOG_DEBUG_SUMMARY
/* Task C: logs front_controller cache state once per second (debug only). */
TASK(can_cache_summary, TASK_STACK_512) {
  CanMessageCache *caches[] = { g_can_cache_high, g_can_cache_medium, g_can_cache_low };
  const size_t sizes[] = { g_can_cache_high_size, g_can_cache_medium_size, g_can_cache_low_size };
  static const char *prio_str[] = { "HIGH  ", "MEDIUM", "LOW   " };

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    uint32_t now = (uint32_t)xTaskGetTickCount();
    LOG_DEBUG("=== front_controller CAN cache ===\r\n");
    for (size_t m = 0; m < sizeof(s_fc_names) / sizeof(s_fc_names[0]); m++) {
      for (int p = 0; p < NUM_CAN_MSG_PRIORITIES; p++) {
        for (size_t i = 0; i < sizes[p]; i++) {
          if (caches[p][i].can_id != s_fc_names[m].can_id) continue;
          bool pending;
          uint32_t last_tick;
          taskENTER_CRITICAL();
          pending = caches[p][i].pending;
          last_tick = caches[p][i].last_updated_tick;
          taskEXIT_CRITICAL();
          if (last_tick == 0U) {
            LOG_DEBUG("  %-24s ID=%-4lu [%s] new=%c last=never\r\n", s_fc_names[m].name, (unsigned long)s_fc_names[m].can_id, prio_str[p], pending ? 'Y' : 'N');
          } else {
            uint32_t elapsed = now - last_tick;
            LOG_DEBUG("  %-24s ID=%-4lu [%s] new=%c last=%lums [%s]\r\n", s_fc_names[m].name, (unsigned long)s_fc_names[m].can_id, prio_str[p], pending ? 'Y' : 'N', (unsigned long)elapsed,
                      elapsed < FC_RECENTLY_WRITTEN_MS ? "recent" : "stale");
          }
        }
      }
    }
    LOG_DEBUG("==================================\r\n");
  }
}
#endif

StatusCode xb_transmit_init(TelemetryStorage *storage, TelemetryConfig *config) {
  if (storage == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_telemetry_storage = storage;
  s_telemetry_storage->config = config;
  tasks_init_task(can_cache_updater, TASK_PRIORITY(2), NULL);
  tasks_init_task(can_cache_scheduler, TASK_PRIORITY(2), NULL);
#ifdef LOG_DEBUG_SUMMARY
  tasks_init_task(can_cache_summary, TASK_PRIORITY(1), NULL);
#endif

  return STATUS_CODE_OK;
}
