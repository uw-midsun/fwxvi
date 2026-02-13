/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for Telemetry Board CAN RX
 *
 * @date   2025-10-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "ff.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "system_can.h"
#include "tasks.h"

/* Intra-component Headers */
#include "datagram.h"
#include "global_enums.h"
#include "sd_card_interface.h"
#include "sd_card_spi.h"
#include "telemetry.h"
#include "telemetry_hw_defs.h"

static TelemetryStorage *telemetry_storage;

static TelemetryConfig s_telemetry_config = {
  .uart_port = UART_PORT_2,
  .uart_settings = {
    .baudrate = 115200,
  },
  .message_transmit_frequency_hz = 10U,
};

static CanStorage s_can_storage = { 0 };

static const CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_TELEMETRY,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = GPIO_TELEMETRY_CAN_TX,
  .rx = GPIO_TELEMETRY_CAN_RX,
  .loopback = false,
  .can_rx_all_cb = NULL,
};

// FrontController
typedef struct {
  uint16_t bps_fault;
  uint8_t relay_state;
  uint8_t power_state;
  uint8_t afe_status;
  uint8_t killswitch_state;

  uint16_t pack_voltage;
  uint16_t pack_current;
  uint16_t pack_soc;

  uint16_t max_cell_voltage;
  uint16_t min_cell_voltage;
  uint16_t max_temperature;
  uint8_t motor_precharge_complete;

  uint16_t input_dcdc_voltage;
  uint16_t input_dcdc_current;
  uint16_t input_aux_voltage;
  uint16_t input_aux_current;

  uint8_t afe1_a_id;
  uint16_t afe1_a_voltage_0;
  uint16_t afe1_a_voltage_1;
  uint16_t afe1_a_voltage_2;

  uint8_t afe1_b_id;
  uint16_t afe1_b_voltage_0;
  uint16_t afe1_b_voltage_1;
  uint16_t afe1_b_voltage_2;

  uint8_t afe2_a_id;
  uint16_t afe2_a_voltage_0;
  uint16_t afe2_a_voltage_1;
  uint16_t afe2_a_voltage_2;

  uint8_t afe2_b_id;
  uint16_t afe2_b_voltage_0;
  uint16_t afe2_b_voltage_1;
  uint16_t afe2_b_voltage_2;

  uint8_t afe_temp_id;
  uint8_t temperature[7];

} RearControllerData;

typedef struct {
  uint32_t pedal_percentage;
  uint8_t brake_enabled;
  uint8_t regen_enabled;
  uint8_t drive_state;

  uint32_t vehicle_velocity;
  uint32_t motor_velocity;

  uint32_t heat_sink_temp;
  uint32_t motor_temp;
  uint16_t rev_cam_current;
  uint16_t telem_current;
  uint16_t steering_current;
  uint16_t driver_fan_current;

  uint16_t horn_current;
  uint16_t spare_current;

  uint16_t brake_light_sig_current;
  uint16_t bps_light_sig_current;
  uint16_t right_sig_current;
  uint16_t left_sig_current;
} FrontControllerData;

typedef struct {
  uint8_t drive_state;
  uint8_t cruise_control;
  uint8_t regen_braking;
  uint8_t hazard_enabled;
  uint8_t horn_enabled;
  uint8_t lights;
  uint8_t cruise_control_target_velocity;
} SteeringData;

static FrontControllerData s_front_controller = { 0 };
static RearControllerData s_rear_controller = { 0 };
static SteeringData s_steering = { 0 };

static const char *s_drive_state_strings[] = {
  [VEHICLE_DRIVE_STATE_INVALID] = "INVALID", [VEHICLE_DRIVE_STATE_NEUTRAL] = "NEUTRAL", [VEHICLE_DRIVE_STATE_DRIVE] = "DRIVE", [VEHICLE_DRIVE_STATE_REVERSE] = "REVERSE",
  [VEHICLE_DRIVE_STATE_CRUISE] = "CRUISE",   [VEHICLE_DRIVE_STATE_BRAKE] = "BRAKE",     [VEHICLE_DRIVE_STATE_REGEN] = "REGEN",
};

static const char *s_power_state_strings[] = {
  [VEHICLE_POWER_STATE_IDLE] = "IDLE",
  [VEHICLE_POWER_STATE_DRIVE] = "DRIVE",
  [VEHICLE_POWER_STATE_CHARGE] = "CHARGE",
  [VEHICLE_POWER_STATE_FAULT] = "FAULT",
};

static const char *s_bps_fault_strings[] = {
  [BPS_FAULT_OVERVOLTAGE] = "OVERVOLTAGE",
  [BPS_FAULT_UNBALANCE] = "UNBALANCE",
  [BPS_FAULT_OVERTEMP_AMBIENT] = "OVERTEMP_AMBIENT",
  [BPS_FAULT_COMMS_LOSS_AFE] = "COMMS_LOSS_AFE",
  [BPS_FAULT_COMMS_LOSS_CURR_SENSE] = "COMMS_LOSS_CURR_SENSE",
  [BPS_FAULT_OVERTEMP_CELL] = "OVERTEMP_CELL",
  [BPS_FAULT_OVERCURRENT] = "OVERCURRENT",
  [BPS_FAULT_UNDERVOLTAGE] = "UNDERVOLTAGE",
  [BPS_FAULT_KILLSWITCH] = "KILLSWITCH",
  [BPS_FAULT_RELAY_CLOSE_FAILED] = "RELAY_CLOSE_FAILED",
  [BPS_FAULT_DISCONNECTED] = "DISCONNECTED",
};

static void print_front_controller(const FrontControllerData *data) {
  LOG_DEBUG("  [FRONT CONTROLLER]\r\n");
  LOG_DEBUG("    Pedal %%:       %lu\r\n", (unsigned long)data->pedal_percentage);
  LOG_DEBUG("    Brake:         %s\r\n", data->brake_enabled ? "ON" : "OFF");
  LOG_DEBUG("    Regen:         %s\r\n", data->regen_enabled ? "ON" : "OFF");
  LOG_DEBUG("    Drive State:   %s\r\n", safe_drive_state_str(data->drive_state));
  LOG_DEBUG("    Vehicle Vel:   %lu\r\n", (unsigned long)data->vehicle_velocity);
  LOG_DEBUG("    Motor Vel:     %lu\r\n", (unsigned long)data->motor_velocity);
  LOG_DEBUG("    Heatsink Temp: %lu\r\n", (unsigned long)data->heat_sink_temp);
  LOG_DEBUG("    Motor Temp:    %lu\r\n", (unsigned long)data->motor_temp);
  LOG_DEBUG("    RevCam mA:     %u\r\n", data->rev_cam_current);
  LOG_DEBUG("    Telem mA:      %u\r\n", data->telem_current);
  LOG_DEBUG("    Steering mA:   %u\r\n", data->steering_current);
  LOG_DEBUG("    Driver Fan mA: %u\r\n", data->driver_fan_current);
  LOG_DEBUG("    Horn mA:       %u\r\n", data->horn_current);
  LOG_DEBUG("    Spare mA:      %u\r\n", data->spare_current);
  LOG_DEBUG("    Brake Light mA:%u\r\n", data->brake_light_sig_current);
  LOG_DEBUG("    BPS Light mA:  %u\r\n", data->bps_light_sig_current);
  LOG_DEBUG("    Right Sig mA:  %u\r\n", data->right_sig_current);
  LOG_DEBUG("    Left Sig mA:   %u\r\n", data->left_sig_current);
}

static void print_rear_controller(const RearControllerData *data) {
  LOG_DEBUG("  [REAR CONTROLLER]\r\n");
  LOG_DEBUG("    Power State:   %s\r\n", safe_power_state_str(data->power_state));
  LOG_DEBUG("    Relay State:   %u\r\n", data->relay_state);
  LOG_DEBUG("    AFE Status:    %u\r\n", data->afe_status);
  LOG_DEBUG("    Killswitch:    %s\r\n", data->killswitch_state ? "ACTIVE" : "INACTIVE");
  print_bps_faults(data->bps_fault);
  LOG_DEBUG("    Pack Voltage:  %u mV\r\n", data->pack_voltage);
  LOG_DEBUG("    Pack Current:  %u mA\r\n", data->pack_current);
  LOG_DEBUG("    Pack SOC:      %u %%\r\n", data->pack_soc);
  LOG_DEBUG("    Max Cell V:    %u mV\r\n", data->max_cell_voltage);
  LOG_DEBUG("    Min Cell V:    %u mV\r\n", data->min_cell_voltage);
  LOG_DEBUG("    Max Temp:      %u\r\n", data->max_temperature);
  LOG_DEBUG("    Precharge:     %s\r\n", data->motor_precharge_complete ? "COMPLETE" : "PENDING");
  LOG_DEBUG("    DCDC Voltage:  %u mV\r\n", data->input_dcdc_voltage);
  LOG_DEBUG("    DCDC Current:  %u mA\r\n", data->input_dcdc_current);
  LOG_DEBUG("    AUX Voltage:   %u mV\r\n", data->input_aux_voltage);
  LOG_DEBUG("    AUX Current:   %u mA\r\n", data->input_aux_current);
}

static void print_steering(const SteeringData *data) {
  LOG_DEBUG("  [STEERING]\r\n");
  LOG_DEBUG("    Drive State:   %s\r\n", safe_drive_state_str(data->drive_state));
  LOG_DEBUG("    Cruise Ctrl:   %s\r\n", data->cruise_control ? "ON" : "OFF");
  LOG_DEBUG("    Regen Brake:   %s\r\n", data->regen_braking ? "ON" : "OFF");
  LOG_DEBUG("    Hazard:        %s\r\n", data->hazard_enabled ? "ON" : "OFF");
  LOG_DEBUG("    Horn:          %s\r\n", data->horn_enabled ? "ON" : "OFF");
  LOG_DEBUG("    Lights:        %u\r\n", data->lights);
  LOG_DEBUG("    Target Vel:    %lu\r\n", (unsigned long)data->cruise_control_target_velocity);
}

// Validate data are correct

static void print_bps_faults(uint16_t fault_bitset) {
  if (fault_bitset == 0) {
    LOG_DEBUG("    Faults: NONE\r\n");
    return;
  }
  LOG_DEBUG("    Faults:\r\n");
  for (uint8_t i = 0; i < NUM_BPS_FAULTS; i++) {
    if (fault_bitset & (1U << i)) {
      LOG_DEBUG("      - %s\r\n", s_bps_fault_strings[i]);
    }
  }
}

static const char *safe_drive_state(uint8_t state) {
  if (state <= VEHICLE_DRIVE_STATE_REGEN) return s_drive_state_strings[state];
  return "UNKNOWN";
}

static const char *safe_power_state_str(uint8_t state) {
  if (state <= VEHICLE_POWER_STATE_FAULT) return s_power_state_strings[state];
  return "UNKNOWN";
}

// Read little-endian from data buffer in messages
static uint16_t read_u16(const uint8_t *data) {
  return (uint16_t)(data[0] | (data[1] << 8));
}

static uint32_t read_u32(const uint8_t *data) {
  return (uint32_t)(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

// Parse messages from front
static void parse_front_controller(uint16_t msg_id, uint8_t dlc, uint8_t *data) {
  switch (msg_id) {
    case 21: /* pedal_data (DLC: 7) */
      if (dlc >= 7) {
        s_front_controller.pedal_percentage = read_u32(&data[0]);
        s_front_controller.brake_enabled = data[4];
        s_front_controller.regen_enabled = data[5];
        s_front_controller.drive_state = data[6];
      }
      break;
    case 22: /* motor_velocity (DLC: 8) */
      if (dlc >= 8) {
        s_front_controller.vehicle_velocity = read_u32(&data[0]);
        s_front_controller.motor_velocity = read_u32(&data[4]);
      }
      break;
    case 23: /* motor_temperature (DLC: 8) */
      if (dlc >= 8) {
        s_front_controller.heat_sink_temp = read_u32(&data[0]);
        s_front_controller.motor_temp = read_u32(&data[4]);
      }
      break;
    case 31: /* fc_power_group_A (DLC: 8) */
      if (dlc >= 8) {
        s_front_controller.rev_cam_current = read_u16(&data[0]);
        s_front_controller.telem_current = read_u16(&data[2]);
        s_front_controller.steering_current = read_u16(&data[4]);
        s_front_controller.driver_fan_current = read_u16(&data[6]);
      }
      break;
    case 34: /* fc_power_lights_group (DLC: 8) */
      if (dlc >= 8) {
        s_front_controller.brake_light_sig_current = read_u16(&data[0]);
        s_front_controller.bps_light_sig_current = read_u16(&data[2]);
        s_front_controller.right_sig_current = read_u16(&data[4]);
        s_front_controller.left_sig_current = read_u16(&data[6]);
      }
      break;
    case 37: /* fc_power_group_B (DLC: 4) */
      if (dlc >= 4) {
        s_front_controller.horn_current = read_u16(&data[0]);
        s_front_controller.spare_current = read_u16(&data[2]);
      }
      break;
    default:
      break;
  }
  print_front_controller(&s_front_controller);
}

// Parse messages from rear
static void parse_rear_controller(uint16_t msg_id, const uint8_t *data, uint8_t dlc) {
  switch (msg_id) {
    case 1: /* rear_controller_status (DLC: 6) */
      if (dlc >= 6) {
        s_rear_controller.bps_fault = read_u16(&data[0]);
        s_rear_controller.relay_state = data[2];
        s_rear_controller.power_state = data[3];
        s_rear_controller.afe_status = data[4];
        s_rear_controller.killswitch_state = data[5];
      }
      break;
    case 2: /* battery_stats_A (DLC: 6) */
      if (dlc >= 6) {
        s_rear_controller.pack_voltage = read_u16(&data[0]);
        s_rear_controller.pack_current = read_u16(&data[2]);
        s_rear_controller.pack_soc = read_u16(&data[4]);
      }
      break;
    case 3: /* battery_stats_B (DLC: 7) */
      if (dlc >= 7) {
        s_rear_controller.max_cell_voltage = read_u16(&data[0]);
        s_rear_controller.min_cell_voltage = read_u16(&data[2]);
        s_rear_controller.max_temperature = read_u16(&data[4]);
        s_rear_controller.motor_precharge_complete = data[6];
      }
      break;
    case 4: /* power_input_stats (DLC: 8) */
      if (dlc >= 8) {
        s_rear_controller.input_dcdc_voltage = read_u16(&data[0]);
        s_rear_controller.input_dcdc_current = read_u16(&data[2]);
        s_rear_controller.input_aux_voltage = read_u16(&data[4]);
        s_rear_controller.input_aux_current = read_u16(&data[6]);
      }
      break;
    case 59: /* AFE1_status_A (DLC: 7) */
      if (dlc >= 7) {
        s_rear_controller.afe1_a_id = data[0];
        s_rear_controller.afe1_a_voltage_0 = read_u16(&data[1]);
        s_rear_controller.afe1_a_voltage_1 = read_u16(&data[3]);
        s_rear_controller.afe1_a_voltage_2 = read_u16(&data[5]);
      }
      break;
    case 60: /* AFE1_status_B (DLC: 7) */
      if (dlc >= 7) {
        s_rear_controller.afe1_b_id = data[0];
        s_rear_controller.afe1_b_voltage_0 = read_u16(&data[1]);
        s_rear_controller.afe1_b_voltage_1 = read_u16(&data[3]);
        s_rear_controller.afe1_b_voltage_2 = read_u16(&data[5]);
      }
      break;
    case 61: /* AFE2_status_A (DLC: 7) */
      if (dlc >= 7) {
        s_rear_controller.afe2_a_id = data[0];
        s_rear_controller.afe2_a_voltage_0 = read_u16(&data[1]);
        s_rear_controller.afe2_a_voltage_1 = read_u16(&data[3]);
        s_rear_controller.afe2_a_voltage_2 = read_u16(&data[5]);
      }
      break;
    case 62: /* AFE2_status_B (DLC: 7) */
      if (dlc >= 7) {
        s_rear_controller.afe2_b_id = data[0];
        s_rear_controller.afe2_b_voltage_0 = read_u16(&data[1]);
        s_rear_controller.afe2_b_voltage_1 = read_u16(&data[3]);
        s_rear_controller.afe2_b_voltage_2 = read_u16(&data[5]);
      }
      break;
    case 63: /* AFE_temperature (DLC: 8) */
      if (dlc >= 8) {
        s_rear_controller.afe_temp_id = data[0];
        for (uint8_t i = 0; i < 7; i++) {
          s_rear_controller.temperature[i] = data[1 + i];
        }
      }
      break;
    default:
      break;
  }
  print_rear_controller(&s_rear_controller);
}

// Parse messages from steering
static void parse_steering(uint16_t msg_id, const uint8_t *data, uint8_t dlc) {
  switch (msg_id) {
    case 5: /* steering_target_velocity (DLC: 4) */
      if (dlc >= 4) {
        s_steering.cruise_control_target_velocity = read_u32(&data[0]);
      }
      break;
    case 6: /* steering_buttons (DLC: 6) */
      if (dlc >= 6) {
        s_steering.drive_state = data[0];
        s_steering.cruise_control = data[1];
        s_steering.regen_braking = data[2];
        s_steering.hazard_enabled = data[3];
        s_steering.horn_enabled = data[4];
        s_steering.lights = data[5];
      }
      break;
    default:
      break;
  }
  print_steering(&s_steering);
}

// Read datagram, update structs accordingly
static void update_board_data(Datagram *datagram) {
  uint16_t msg_id = datagram->id;
  uint8_t source_id = (msg_id >> 5U) & 0x1FU;

  switch (source_id) {
    case SYSTEM_CAN_DEVICE_FRONT_CONTROLLER:
      if (datagram->dlc >= 1) s_front_controller.drive_state = (VehicleDriveState)datagram->data[0];
      if (datagram->dlc >= 2) s_front_controller.power_state = (VehiclePowerStates)datagram->data[1];
      if (datagram->dlc >= 3) s_front_controller.light_state = (SteeringLightState)datagram->data[2];
      if (datagram->dlc >= 4) s_front_controller.pd_state = (PDPowerState)datagram->data[3];
      print_front_controller(&s_front_controller);
      break;

    case SYSTEM_CAN_DEVICE_REAR_CONTROLLER:
      if (datagram->dlc >= 1) s_rear_controller.drive_state = (VehicleDriveState)datagram->data[0];
      if (datagram->dlc >= 2) s_rear_controller.power_state = (VehiclePowerStates)datagram->data[1];
      if (datagram->dlc >= 4) s_rear_controller.bps_fault_bitset = (uint16_t)(datagram->data[2] | (datagram->data[3] << 8));
      if (datagram->dlc >= 5) s_rear_controller.bps_light_state = (BpsLightState)datagram->data[4];
      print_rear_controller(&s_rear_controller);
      break;

    case SYSTEM_CAN_DEVICE_STEERING:
      if (datagram->dlc >= 1) s_steering.light_state = (SteeringLightState)datagram->data[0];
      if (datagram->dlc >= 2) s_steering.cruise_control = datagram->data[1];
      print_steering(&s_steering);
      break;

    default:
      LOG_DEBUG("  [Unknown device: %d]\r\n", source_id);
      break;
  }
}

static void print_all_boards(void) {
  LOG_DEBUG("\r\n================== BOARD STATUS SUMMARY ==================\r\n");
  print_front_controller(&s_front_controller);
  print_rear_controller(&s_rear_controller);
  print_steering(&s_steering);
  LOG_DEBUG("==========================================================\r\n\r\n");
}

TASK(telemetry_reader, TASK_STACK_1024) {
  Datagram datagram = { 0U };
  CanMessage message = { 0U };
  uint16_t msg_count = 0;
  StatusCode status = STATUS_CODE_OK;

  LOG_DEBUG("Telemetry smoke test");
  delay_ms(10U);

  while (true) {
    if (queue_receive(&s_can_storage.rx_queue.queue, &message, QUEUE_DELAY_BLOCKING) == STATUS_CODE_OK) {
      LOG_DEBUG("Received message\n");
      decode_can_message(&datagram, &message);
      msg_count++;
      LOG_DEBUG("[MSG #%lu] ID: 0x%04X | DLC: %d\r\n", msg_count, datagram.id, datagram.dlc);
      delay_ms(10U);
      /* Wait for new data to be in the queue */

      update_board_data(&datagram);

      if (msg_count % 50U == 0U) {
        print_all_boards();
      }
    }
  }
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  mcu_init();
  tasks_init();
  log_init();
  can_init(&s_can_storage, &s_can_settings);

  tasks_init_task(telemetry_reader, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
