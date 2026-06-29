/************************************************************************************************
 * @file    can_cache.c
 *
 * @brief   Static CAN message cache definitions for telemetry
 *
 * @date    2026-06-10
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "can_cache.h"

/* CAN IDs come from can/tools/system_dbc.dbc. Priority tier is the message cycle rate
   in the source board's YAML (fast→HIGH, medium→MEDIUM, slow→LOW). */

/* No fast-cycle messages target telemetry — placeholder keeps the array valid. */
CanMessageCache g_can_cache_high[] = {
  { 0 },
};
const size_t g_can_cache_high_size = 0U;

CanMessageCache g_can_cache_medium[] = {
  /* rear_controller messages */
  { .can_id = 17U, .length = 4U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* rear_controller_status */
  { .can_id = 33U, .length = 6U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* battery_stats_A        */
  { .can_id = 49U, .length = 6U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* battery_stats_B        */
  { .can_id = 65U, .length = 8U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* power_input_stats      */
  /* steering */
  { .can_id = 96U, .length = 5U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* steering               */
  /* front_controller */
  { .can_id = 1365U, .length = 5U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* drive_status           */
  /* imu */
  { .can_id = 1827U, .length = 6U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* accel_data             */
  { .can_id = 1843U, .length = 6U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* gyro_data              */
  /* rear_controller AFE messages */
  { .can_id = 1825U, .length = 8U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* AFE_discharge_bitset   */
  { .can_id = 1841U, .length = 7U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* AFE1_status_A          */
  { .can_id = 1857U, .length = 7U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* AFE1_status_B          */
  { .can_id = 1873U, .length = 7U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* AFE1_status_C          */
  { .can_id = 1889U, .length = 7U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* AFE1_status_D          */
  { .can_id = 1905U, .length = 7U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* AFE1_status_E          */
  { .can_id = 1921U, .length = 7U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* AFE1_status_F          */
  { .can_id = 1937U, .length = 7U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* AFE2_status_A          */
  { .can_id = 1953U, .length = 7U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* AFE2_status_B          */
  { .can_id = 1969U, .length = 7U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* AFE2_status_C          */
  { .can_id = 1985U, .length = 7U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* AFE2_status_D          */
  { .can_id = 2001U, .length = 7U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* AFE2_status_E          */
  { .can_id = 2017U, .length = 7U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* AFE2_status_F          */
  { .can_id = 2033U, .length = 8U, .priority = CAN_MSG_PRIORITY_MEDIUM }, /* AFE_temperature        */
};
const size_t g_can_cache_medium_size = sizeof(g_can_cache_medium) / sizeof(g_can_cache_medium[0]);

CanMessageCache g_can_cache_low[] = {
  /* ---- CAN-received slow-cycle messages ---- */
  { .can_id = 1525U, .length = 8U, .priority = CAN_MSG_PRIORITY_LOW }, /* fc_power_group_A      */
  { .can_id = 1573U, .length = 8U, .priority = CAN_MSG_PRIORITY_LOW }, /* fc_power_lights_group */
  { .can_id = 1621U, .length = 4U, .priority = CAN_MSG_PRIORITY_LOW }, /* fc_power_group_B      */

  /* ---- Telemetry-internal messages (populated by scheduler, not CAN RX) ---- */
  { .can_id = TELEMETRY_STATS_CAN_ID, .length = TELEMETRY_STATS_DLC, .priority = CAN_MSG_PRIORITY_LOW }, /* telemetry_stats       */

  /* ---- WS22 synthetic messages (floats re-encoded as scaled integers) ---- */
  { .can_id = WS22_TELEMETRY_STATUS_ID, .length = WS22_TELEMETRY_DLC, .priority = CAN_MSG_PRIORITY_LOW },    /* ws22_status    */
  { .can_id = WS22_TELEMETRY_BUS_ID, .length = WS22_TELEMETRY_DLC, .priority = CAN_MSG_PRIORITY_LOW },       /* ws22_bus       */
  { .can_id = WS22_TELEMETRY_VELOCITY_ID, .length = WS22_TELEMETRY_DLC, .priority = CAN_MSG_PRIORITY_LOW },  /* ws22_velocity  */
  { .can_id = WS22_TELEMETRY_TEMP_ID, .length = WS22_TELEMETRY_DLC, .priority = CAN_MSG_PRIORITY_LOW },      /* ws22_temp      */
  { .can_id = WS22_TELEMETRY_DRIVE_CMD_ID, .length = WS22_TELEMETRY_DLC, .priority = CAN_MSG_PRIORITY_LOW }, /* ws22_drive_cmd */
};
const size_t g_can_cache_low_size = sizeof(g_can_cache_low) / sizeof(g_can_cache_low[0]);
