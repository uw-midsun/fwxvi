/************************************************************************************************
 * @file   telemetry.c
 *
 * @brief  Source code for telemetry system
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

/* Inter-component Headers */
#include "can.h"
#include "datagram.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "persist.h"
#include "status.h"
#include "system_can.h"
#include "tasks.h"
#include "uart.h"

/* Intra-component Headers */
#include "bmi323.h"
#include "flash.h"
#include "imu.h"
#include "sd_card_interface.h"
#include "telemetry.h"
#include "telemetry_getters.h"
#include "telemetry_hw_defs.h"
#include "xb_transmit.h"

static GpioAddress s_telemetry_board_led = GPIO_TELEMETRY_BOARD_LED;
static GpioAddress s_xbee_sleep = GPIO_TELEMETRY_XBEE_SLEEP_RQ;
static GpioAddress s_xbee_reset = GPIO_TELEMETRY_XBEE_XRST;

static PersistStorage persist_storage = { 0U };
static RebootCount reboot_count = { 0U };

static StatusCode s_receive_ws22(CanMessage *message) {
  return ws22_motor_can_process_rx(message->data_u8, message->id.raw, message->dlc);
}

static char scratch_buffer[512];
static bool s_log_created;

static const CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_TELEMETRY,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = GPIO_TELEMETRY_CAN_TX,
  .rx = GPIO_TELEMETRY_CAN_RX,
  .loopback = false,
  .can_rx_all_cb = s_receive_ws22,
};

static StatusCode s_record_boot(void) {
  status_ok_or_return(flash_init());

  status_ok_or_return(persist_init(&persist_storage, NUM_FLASH_PAGES - 1U, &reboot_count, sizeof(reboot_count), false));

  if (reboot_count.reboot_number == UINT32_MAX) return STATUS_CODE_OUT_OF_RANGE;
  ++reboot_count.reboot_number;

  return persist_commit(&persist_storage);
}

StatusCode telemetry_init(TelemetryStorage *telemetry_storage, TelemetryConfig *config, Bmi323Storage *bmi323_storage, CanStorage *can_storage) {
  if (telemetry_storage == NULL || config == NULL || bmi323_storage == NULL || can_storage == NULL || bmi323_storage->settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_log_created = false;
  status_ok_or_return(s_record_boot());
  telemetry_storage->reboot_number = reboot_count;

  telemetry_storage->config = config;
  telemetry_storage->bmi323_storage = bmi323_storage;
  telemetry_storage->can_storage = can_storage;

  status_ok_or_return(uart_init(telemetry_storage->config->uart_port, &telemetry_storage->config->uart_settings));
  status_ok_or_return(can_init(telemetry_storage->can_storage, &s_can_settings));

  telemetry_storage->datagram_queue.item_size = sizeof(Datagram);
  telemetry_storage->datagram_queue.num_items = DATAGRAM_BUFFER_SIZE;
  telemetry_storage->datagram_queue.storage_buf = (uint8_t *)telemetry_storage->datagram_buffer;

  StatusCode queue_status = queue_init(&telemetry_storage->datagram_queue);
  if (queue_status != STATUS_CODE_OK) {
    LOG_DEBUG("Datagram queue initialization failed: %u\n", (unsigned)queue_status);
    return queue_status;
  }

  StatusCode sd_status = sd_card_link_driver(telemetry_storage->config->sd_spi_port, &telemetry_storage->config->sd_spi_settings);

  if (sd_status != STATUS_CODE_OK) {
    LOG_DEBUG("SD driver registration failed: %u\n", (unsigned)sd_status);
    return sd_status;
  }

  FRESULT mount_result = sd_card_mount();

  /* FRESULT -> StatusCode */
  if (mount_result != FR_OK) {
    return mount_result == FR_NOT_READY ? STATUS_CODE_UNINITIALIZED : STATUS_CODE_INTERNAL_ERROR;
  }

  status_ok_or_return(gpio_init_pin(&s_telemetry_board_led, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH));
  status_ok_or_return(gpio_init_pin(&s_xbee_sleep, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW));
  status_ok_or_return(gpio_init_pin(&s_xbee_reset, GPIO_OUTPUT_OPEN_DRAIN, GPIO_STATE_HIGH));

  return STATUS_CODE_OK;
}

/* All writes use the same bounded buffer; callers still close the file on failure */
static StatusCode s_write_csv(FIL *fd, int length) {
  if (length < 0 || (size_t)length >= sizeof(scratch_buffer)) {
    return STATUS_CODE_OUT_OF_RANGE;
  }
  UINT written = 0U;

  if (f_write(fd, scratch_buffer, (UINT)length, &written) != FR_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return written == (UINT)length ? STATUS_CODE_OK : STATUS_CODE_RESOURCE_EXHAUSTED;
}

StatusCode telemetry_log_sd(void) {
  const char *drive = sd_card_drive_path();
  if (drive == NULL || reboot_count.reboot_number == 0U) {
    return STATUS_CODE_UNINITIALIZED;
  }

  Ws22MotorTelemetryData *motor = ws22_motor_can_get_telemetry_data();
  Ws22MotorControlData *control = ws22_motor_can_get_control_data();
  if (motor == NULL || control == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  char path[32];
  int len = snprintf(path, sizeof(path), "%slogs_%lu.csv", drive, reboot_count.reboot_number);
  if (len < 0 || (size_t)len >= sizeof(path)) {
    return STATUS_CODE_OUT_OF_RANGE;
  }

  FIL fd = { 0 };
  BYTE mode = FA_WRITE | ((s_log_created == true) ? FA_OPEN_APPEND : FA_CREATE_NEW);
  if (f_open(&fd, path, mode) != FR_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  s_log_created = true;

  StatusCode status = STATUS_CODE_OK;
  if (f_size(&fd) == 0U) {
    len = snprintf(scratch_buffer, sizeof(scratch_buffer), "timestamp_ms,signal,value\r\n");
    status = s_write_csv(&fd, len);
  }

  uint32_t timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

  taskENTER_CRITICAL();
  /* yap */
  uint64_t fault_info = get_bps_fault_info_extra_info();
  const struct {
    const char *name;
    double value;
  } signals[] = {
    { "drive_status_pedal_percentage", get_drive_status_pedal_percentage() },
    { "drive_status_brake_percentage", get_drive_status_brake_percentage() },
    { "drive_status_state_data_raw", get_drive_status_state_data_raw() },
    { "drive_status_state_data_drive_state", get_drive_status_state_data_drive_state() },
    { "drive_status_state_data_brake_enabled", get_drive_status_state_data_brake_enabled() },
    { "drive_status_state_data_regen_enabled", get_drive_status_state_data_regen_enabled() },
    { "fc_power_group_A_rev_cam_current", get_fc_power_group_A_rev_cam_current() },
    { "fc_power_group_A_telem_current", get_fc_power_group_A_telem_current() },
    { "fc_power_group_A_steering_current", get_fc_power_group_A_steering_current() },
    { "fc_power_group_A_driver_fan_current", get_fc_power_group_A_driver_fan_current() },
    { "fc_power_group_B_horn_current", get_fc_power_group_B_horn_current() },
    { "fc_power_group_B_spare_current", get_fc_power_group_B_spare_current() },
    { "fc_power_lights_group_brake_light_sig_current", get_fc_power_lights_group_brake_light_sig_current() },
    { "fc_power_lights_group_bps_light_sig_current", get_fc_power_lights_group_bps_light_sig_current() },
    { "fc_power_lights_group_right_sig_current", get_fc_power_lights_group_right_sig_current() },
    { "fc_power_lights_group_left_sig_current", get_fc_power_lights_group_left_sig_current() },
    { "gyro_data_x_axis", get_gyro_data_x_axis() },
    { "gyro_data_y_axis", get_gyro_data_y_axis() },
    { "gyro_data_z_axis", get_gyro_data_z_axis() },
    { "accel_data_x_axis", get_accel_data_x_axis() },
    { "accel_data_y_axis", get_accel_data_y_axis() },
    { "accel_data_z_axis", get_accel_data_z_axis() },
    { "rear_controller_status_triggers_raw", get_rear_controller_status_triggers_raw() },
    { "rear_controller_status_triggers_bps_fault", get_rear_controller_status_triggers_bps_fault() },
    { "rear_controller_status_triggers_cell_at_fault", get_rear_controller_status_triggers_cell_at_fault() },
    { "rear_controller_status_triggers_solar_relay_closed", get_rear_controller_status_triggers_solar_relay_closed() },
    { "rear_controller_status_triggers_motor_relay_closed", get_rear_controller_status_triggers_motor_relay_closed() },
    { "rear_controller_status_triggers_pos_relay_closed", get_rear_controller_status_triggers_pos_relay_closed() },
    { "rear_controller_status_triggers_neg_relay_closed", get_rear_controller_status_triggers_neg_relay_closed() },
    { "rear_controller_status_triggers_power_state", get_rear_controller_status_triggers_power_state() },
    { "rear_controller_status_triggers_afe_status", get_rear_controller_status_triggers_afe_status() },
    { "rear_controller_status_triggers_motor_precharge_complete", get_rear_controller_status_triggers_motor_precharge_complete() },
    { "rear_controller_status_triggers_bps_fault_live", get_rear_controller_status_triggers_bps_fault_live() },
    { "battery_stats_A_pack_voltage_v", get_battery_stats_A_pack_voltage_v() },
    { "battery_stats_A_pack_soc", get_battery_stats_A_pack_soc() },
    { "battery_stats_B_max_cell_voltage", get_battery_stats_B_max_cell_voltage() },
    { "battery_stats_B_min_cell_voltage", get_battery_stats_B_min_cell_voltage() },
    { "battery_stats_B_max_temperature", get_battery_stats_B_max_temperature() },
    { "battery_stats_B_pack_current_a", get_battery_stats_B_pack_current_a() },
    { "power_input_stats_input_dcdc_voltage", get_power_input_stats_input_dcdc_voltage() },
    { "power_input_stats_input_dcdc_current", get_power_input_stats_input_dcdc_current() },
    { "power_input_stats_input_aux_voltage", get_power_input_stats_input_aux_voltage() },
    { "power_input_stats_input_aux_current", get_power_input_stats_input_aux_current() },
    { "AFE_discharge_bitset_AFE1_raw", get_AFE_discharge_bitset_AFE1_raw() },
    { "AFE_discharge_bitset_AFE1_cell_0", get_AFE_discharge_bitset_AFE1_cell_0() },
    { "AFE_discharge_bitset_AFE1_cell_1", get_AFE_discharge_bitset_AFE1_cell_1() },
    { "AFE_discharge_bitset_AFE1_cell_2", get_AFE_discharge_bitset_AFE1_cell_2() },
    { "AFE_discharge_bitset_AFE1_cell_3", get_AFE_discharge_bitset_AFE1_cell_3() },
    { "AFE_discharge_bitset_AFE1_cell_4", get_AFE_discharge_bitset_AFE1_cell_4() },
    { "AFE_discharge_bitset_AFE1_cell_5", get_AFE_discharge_bitset_AFE1_cell_5() },
    { "AFE_discharge_bitset_AFE1_cell_6", get_AFE_discharge_bitset_AFE1_cell_6() },
    { "AFE_discharge_bitset_AFE1_cell_7", get_AFE_discharge_bitset_AFE1_cell_7() },
    { "AFE_discharge_bitset_AFE1_cell_8", get_AFE_discharge_bitset_AFE1_cell_8() },
    { "AFE_discharge_bitset_AFE1_cell_9", get_AFE_discharge_bitset_AFE1_cell_9() },
    { "AFE_discharge_bitset_AFE1_cell_10", get_AFE_discharge_bitset_AFE1_cell_10() },
    { "AFE_discharge_bitset_AFE1_cell_11", get_AFE_discharge_bitset_AFE1_cell_11() },
    { "AFE_discharge_bitset_AFE1_cell_12", get_AFE_discharge_bitset_AFE1_cell_12() },
    { "AFE_discharge_bitset_AFE1_cell_13", get_AFE_discharge_bitset_AFE1_cell_13() },
    { "AFE_discharge_bitset_AFE1_cell_14", get_AFE_discharge_bitset_AFE1_cell_14() },
    { "AFE_discharge_bitset_AFE1_cell_15", get_AFE_discharge_bitset_AFE1_cell_15() },
    { "AFE_discharge_bitset_AFE2_raw", get_AFE_discharge_bitset_AFE2_raw() },
    { "AFE_discharge_bitset_AFE2_cell_0", get_AFE_discharge_bitset_AFE2_cell_0() },
    { "AFE_discharge_bitset_AFE2_cell_1", get_AFE_discharge_bitset_AFE2_cell_1() },
    { "AFE_discharge_bitset_AFE2_cell_2", get_AFE_discharge_bitset_AFE2_cell_2() },
    { "AFE_discharge_bitset_AFE2_cell_3", get_AFE_discharge_bitset_AFE2_cell_3() },
    { "AFE_discharge_bitset_AFE2_cell_4", get_AFE_discharge_bitset_AFE2_cell_4() },
    { "AFE_discharge_bitset_AFE2_cell_5", get_AFE_discharge_bitset_AFE2_cell_5() },
    { "AFE_discharge_bitset_AFE2_cell_6", get_AFE_discharge_bitset_AFE2_cell_6() },
    { "AFE_discharge_bitset_AFE2_cell_7", get_AFE_discharge_bitset_AFE2_cell_7() },
    { "AFE_discharge_bitset_AFE2_cell_8", get_AFE_discharge_bitset_AFE2_cell_8() },
    { "AFE_discharge_bitset_AFE2_cell_9", get_AFE_discharge_bitset_AFE2_cell_9() },
    { "AFE_discharge_bitset_AFE2_cell_10", get_AFE_discharge_bitset_AFE2_cell_10() },
    { "AFE_discharge_bitset_AFE2_cell_11", get_AFE_discharge_bitset_AFE2_cell_11() },
    { "AFE_discharge_bitset_AFE2_cell_12", get_AFE_discharge_bitset_AFE2_cell_12() },
    { "AFE_discharge_bitset_AFE2_cell_13", get_AFE_discharge_bitset_AFE2_cell_13() },
    { "AFE_discharge_bitset_AFE2_cell_14", get_AFE_discharge_bitset_AFE2_cell_14() },
    { "AFE_discharge_bitset_AFE2_cell_15", get_AFE_discharge_bitset_AFE2_cell_15() },
    { "AFE1_status_A_id", get_AFE1_status_A_id() },
    { "AFE1_status_A_voltage_0", get_AFE1_status_A_voltage_0() },
    { "AFE1_status_A_voltage_1", get_AFE1_status_A_voltage_1() },
    { "AFE1_status_A_voltage_2", get_AFE1_status_A_voltage_2() },
    { "AFE1_status_B_id", get_AFE1_status_B_id() },
    { "AFE1_status_B_voltage_3", get_AFE1_status_B_voltage_3() },
    { "AFE1_status_B_voltage_4", get_AFE1_status_B_voltage_4() },
    { "AFE1_status_B_voltage_5", get_AFE1_status_B_voltage_5() },
    { "AFE1_status_C_id", get_AFE1_status_C_id() },
    { "AFE1_status_C_voltage_6", get_AFE1_status_C_voltage_6() },
    { "AFE1_status_C_voltage_7", get_AFE1_status_C_voltage_7() },
    { "AFE1_status_C_voltage_8", get_AFE1_status_C_voltage_8() },
    { "AFE1_status_D_id", get_AFE1_status_D_id() },
    { "AFE1_status_D_voltage_9", get_AFE1_status_D_voltage_9() },
    { "AFE1_status_D_voltage_10", get_AFE1_status_D_voltage_10() },
    { "AFE1_status_D_voltage_11", get_AFE1_status_D_voltage_11() },
    { "AFE1_status_E_id", get_AFE1_status_E_id() },
    { "AFE1_status_E_voltage_12", get_AFE1_status_E_voltage_12() },
    { "AFE1_status_E_voltage_13", get_AFE1_status_E_voltage_13() },
    { "AFE1_status_E_voltage_14", get_AFE1_status_E_voltage_14() },
    { "AFE1_status_F_id", get_AFE1_status_F_id() },
    { "AFE1_status_F_voltage_15", get_AFE1_status_F_voltage_15() },
    { "AFE1_status_F_voltage_16", get_AFE1_status_F_voltage_16() },
    { "AFE1_status_F_voltage_17", get_AFE1_status_F_voltage_17() },
    { "AFE2_status_A_id", get_AFE2_status_A_id() },
    { "AFE2_status_A_voltage_0", get_AFE2_status_A_voltage_0() },
    { "AFE2_status_A_voltage_1", get_AFE2_status_A_voltage_1() },
    { "AFE2_status_A_voltage_2", get_AFE2_status_A_voltage_2() },
    { "AFE2_status_B_id", get_AFE2_status_B_id() },
    { "AFE2_status_B_voltage_3", get_AFE2_status_B_voltage_3() },
    { "AFE2_status_B_voltage_4", get_AFE2_status_B_voltage_4() },
    { "AFE2_status_B_voltage_5", get_AFE2_status_B_voltage_5() },
    { "AFE2_status_C_id", get_AFE2_status_C_id() },
    { "AFE2_status_C_voltage_6", get_AFE2_status_C_voltage_6() },
    { "AFE2_status_C_voltage_7", get_AFE2_status_C_voltage_7() },
    { "AFE2_status_C_voltage_8", get_AFE2_status_C_voltage_8() },
    { "AFE2_status_D_id", get_AFE2_status_D_id() },
    { "AFE2_status_D_voltage_9", get_AFE2_status_D_voltage_9() },
    { "AFE2_status_D_voltage_10", get_AFE2_status_D_voltage_10() },
    { "AFE2_status_D_voltage_11", get_AFE2_status_D_voltage_11() },
    { "AFE2_status_E_id", get_AFE2_status_E_id() },
    { "AFE2_status_E_voltage_12", get_AFE2_status_E_voltage_12() },
    { "AFE2_status_E_voltage_13", get_AFE2_status_E_voltage_13() },
    { "AFE2_status_E_voltage_14", get_AFE2_status_E_voltage_14() },
    { "AFE2_status_F_id", get_AFE2_status_F_id() },
    { "AFE2_status_F_voltage_15", get_AFE2_status_F_voltage_15() },
    { "AFE2_status_F_voltage_16", get_AFE2_status_F_voltage_16() },
    { "AFE2_status_F_voltage_17", get_AFE2_status_F_voltage_17() },
    { "AFE_temperature_id", get_AFE_temperature_id() },
    { "AFE_temperature_temperature_0", get_AFE_temperature_temperature_0() },
    { "AFE_temperature_temperature_1", get_AFE_temperature_temperature_1() },
    { "AFE_temperature_temperature_2", get_AFE_temperature_temperature_2() },
    { "AFE_temperature_temperature_3", get_AFE_temperature_temperature_3() },
    { "AFE_temperature_temperature_4", get_AFE_temperature_temperature_4() },
    { "AFE_temperature_temperature_5", get_AFE_temperature_temperature_5() },
    { "AFE_temperature_temperature_6", get_AFE_temperature_temperature_6() },
    { "steering_cruise_control_target_velocity", get_steering_cruise_control_target_velocity() },
    { "steering_buttons_raw", get_steering_buttons_raw() },
    { "steering_buttons_drive_state", get_steering_buttons_drive_state() },
    { "steering_buttons_lights", get_steering_buttons_lights() },
    { "steering_buttons_cruise_control_enabled", get_steering_buttons_cruise_control_enabled() },
    { "steering_buttons_hazard_enabled", get_steering_buttons_hazard_enabled() },
    { "steering_buttons_horn_enabled", get_steering_buttons_horn_enabled() },
    { "steering_buttons_regen_enabled", get_steering_buttons_regen_enabled() },
    { "steering_buttons_balancing_enabled", get_steering_buttons_balancing_enabled() },
    { "steering_buttons_bps_enabled", get_steering_buttons_bps_enabled() },
    { "ws22_control_current", control->current },
    { "ws22_control_velocity", control->velocity },
    { "ws22_error_flags", motor->error_flags },
    { "ws22_limit_flags", motor->limit_flags },
    { "ws22_merged_flags", motor->merged_flags },
    { "ws22_bus_current", motor->bus_current },
    { "ws22_bus_voltage", motor->bus_voltage },
    { "ws22_vehicle_velocity_kph", motor->vehicle_velocity_kph },
    { "ws22_motor_velocity", motor->motor_velocity },
    { "ws22_phase_b_current", motor->phase_b_current },
    { "ws22_phase_c_current", motor->phase_c_current },
    { "ws22_voltage_d", motor->voltage_d },
    { "ws22_voltage_q", motor->voltage_q },
    { "ws22_current_d", motor->current_d },
    { "ws22_current_q", motor->current_q },
    { "ws22_back_emf_d", motor->back_emf_d },
    { "ws22_back_emf_q", motor->back_emf_q },
    { "ws22_rail_15v_supply", motor->rail_15v_supply },
    { "ws22_heat_sink_temp", motor->heat_sink_temp },
    { "ws22_motor_temp", motor->motor_temp },
  };
  taskEXIT_CRITICAL();

  for (size_t i = 0; i < sizeof(signals) / sizeof(signals[0]); ++i) {
    len = snprintf(scratch_buffer, sizeof(scratch_buffer), "%lu,%s,%.10g\r\n", timestamp_ms, signals[i].name, signals[i].value);
    status = s_write_csv(&fd, len);

    if (status != STATUS_CODE_OK) {
      break;
    }
  }
  if (status == STATUS_CODE_OK) {
    /* why do we have 64 bit values... */
    char fault_text[21];
    char *digit = &fault_text[sizeof(fault_text) - 1U];
    *digit = '\0';

    do {
      *--digit = '0' + fault_info % 10U;
      fault_info /= 10U;
    } while (fault_info != 0U);

    len = snprintf(scratch_buffer, sizeof(scratch_buffer), "%lu,bps_fault_info_extra_info,%s\r\n", timestamp_ms, digit);
    status = s_write_csv(&fd, len);
  }

  FRESULT result = f_close(&fd);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  return result == FR_OK ? STATUS_CODE_OK : STATUS_CODE_INTERNAL_ERROR;
}
