<<<<<<< HEAD
import struct
import time
from can import Message
from can.interface import Bus
from can.broadcastmanager import ThreadBasedCyclicSendTask
from enum import Enum
from threading import Lock, Thread, Event

# --- Configuration ---
RETRY_TRANSMIT_S = 0.05
SYSTEM_CAN_MESSAGE_ID_OFFSET = 4
SYSTEM_CAN_MESSAGE_PRIORITY_BIT = 0x400

class SystemCanDevice(Enum):
    SYSTEM_CAN_DEVICE_BMS_CARRIER = 0
    SYSTEM_CAN_DEVICE_CAN_COMMUNICATION = 1
    SYSTEM_CAN_DEVICE_CENTRE_CONSOLE = 2
    SYSTEM_CAN_DEVICE_TELEMETRY = 3
    NUM_SYSTEM_CAN_DEVICES = 4

class SystemCanMessageId(Enum):
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_STATUS_ID = (1 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER.value
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT_ID = (2 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER.value
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_INFO_ID = (3 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER.value
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE1_STATUS_ID = SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (61 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER.value
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE2_STATUS_ID = SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (62 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER.value
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE3_STATUS_ID = SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (63 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER.value
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_CONTROLLER_VC_ID = SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (57 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER.value
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_VELOCITY_ID = (58 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER.value
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_SINK_TEMPS_ID = SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (59 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER.value
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_DSP_BOARD_TEMPS_ID = SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (60 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER.value
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_MC_STATUS_ID = (8 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER.value
    SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_FAST_ONE_SHOT_MSG_ID = SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (56 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_CAN_COMMUNICATION.value
    SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_MEDIUM_ONE_SHOT_MSG_ID = SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (55 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_CAN_COMMUNICATION.value
    SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_SLOW_ONE_SHOT_MSG_ID = SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (54 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_CAN_COMMUNICATION.value
    SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_PEDAL_ID = (4 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_CENTRE_CONSOLE.value
    SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_INFO_ID = (5 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_CENTRE_CONSOLE.value
    SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_STEERING_ID = (6 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_CENTRE_CONSOLE.value
    SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_REGEN_PERCENTAGE_ID = (7 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_CENTRE_CONSOLE.value
    SYSTEM_CAN_MESSAGE_TELEMETRY_TELEMETRY_ID = SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (53 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_TELEMETRY.value
=======
## @defgroup CANSimulator CAN Simulation tools
#  This package contains CAN simulation tools for testing the full vehicle
#
## @file    can_simulator.py
#  @date    2025-03-09
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Main simulation script
#
#  @ingroup CANSimulator

import  struct
import  time
from    threading import Lock, Thread

from    can                     import Message
from    can.interface           import Bus
from    can.broadcastmanager    import ThreadBasedCyclicSendTask

from    system_can              import *
from    can_simulator_cache     import *

RETRY_TRANSMIT_S = 0.05

FAST_CYCLE_PERIOD = 0.5
MEDIUM_CYCLE_PERIOD = 5
SLOW_CYCLE_PERIOD = 10
>>>>>>> bed2dba89b527b04337236b4c1cd0c3393f1ae14

messages_fast = []
messages_medium = []
messages_slow = []

def pack(num, size):
    """Packs a number into bytes. Casts floats to int for integer format codes."""
    if isinstance(num, float) and size == 32:
<<<<<<< HEAD
        return struct.pack("<f", num)
    elif size == 64:
        return struct.pack("<q", int(num))
    elif size == 32:
        return struct.pack("<i", int(num))
    elif size == 16:
        return struct.pack("<h", int(num))
    elif size == 8:
        return struct.pack("<b", int(num))
    else:
        raise ValueError(f"Unknown size {size} for number {num}")

# --- Default Signal Values ---
fault = 0
fault_val = 0
aux_batt_v = 12.0
afe_status = 0
voltage = 100.0
current = 10.0
temperature = 25.0
batt_perc = 75.0
fan1 = 0
fan2 = 0
max_cell_v = 4.15
min_cell_v = 4.10
id = 1
temp = 25
v1 = 4.11
v2 = 4.12
v3 = 4.13
mc_voltage_l = 100.0
mc_current_l = 0.0
mc_voltage_r = 100.0
mc_current_r = 0.0
velocity_l = 0
velocity_r = 0
brakes_enabled = 0
motor_temp_l = 30
heatsink_temp_l = 30
motor_temp_r = 30
heatsink_temp_r = 30
dsp_temp_l = 40
dsp_temp_r = 40
limit_bitset_l = 0
error_bitset_l = 0
limit_bitset_r = 0
error_bitset_r = 0
board_fault_bitset = 0
overtemp_bitset = 0
precharge_status = 0
sig1 = 123
sig2 = 456
throttle_output = 0
brake_output = 0
target_velocity = 0
drive_state = 0
cruise_control = 0
regen_braking = 0
hazard_enabled = 0
input_cc = 0
input_lights = 0
percent = 0
data = 0

# --- Message Definitions ---
SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_STATUS = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_STATUS_ID.value,
    data=b''.join([pack(fault, 16), pack(fault_val, 16), pack(aux_batt_v, 16), pack(afe_status, 8)])
)
messages_medium.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_STATUS)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT_ID.value,
    data=b''.join([pack(voltage, 16), pack(current, 16), pack(temperature, 16), pack(batt_perc, 16)])
)
messages_fast.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_INFO = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_INFO_ID.value,
    data=b''.join([pack(fan1, 8), pack(fan2, 8), pack(max_cell_v, 16), pack(min_cell_v, 16)])
)
messages_slow.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_INFO)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE1_STATUS = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE1_STATUS_ID.value,
    data=b''.join([pack(id, 8), pack(temp, 8), pack(v1, 16), pack(v2, 16), pack(v3, 16)])
)
messages_slow.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE1_STATUS)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE2_STATUS = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE2_STATUS_ID.value,
    data=b''.join([pack(id, 8), pack(temp, 8), pack(v1, 16), pack(v2, 16), pack(v3, 16)])
)
messages_slow.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE2_STATUS)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE3_STATUS = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE3_STATUS_ID.value,
    data=b''.join([pack(id, 8), pack(temp, 8), pack(v1, 16), pack(v2, 16), pack(v3, 16)])
)
messages_slow.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE3_STATUS)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_CONTROLLER_VC = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_CONTROLLER_VC_ID.value,
    data=b''.join([pack(mc_voltage_l, 16), pack(mc_current_l, 16), pack(mc_voltage_r, 16), pack(mc_current_r, 16)])
)
messages_fast.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_CONTROLLER_VC)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_VELOCITY = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_VELOCITY_ID.value,
    data=b''.join([pack(velocity_l, 16), pack(velocity_r, 16), pack(brakes_enabled, 8)])
)
messages_fast.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_VELOCITY)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_SINK_TEMPS = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_SINK_TEMPS_ID.value,
    data=b''.join([pack(motor_temp_l, 16), pack(heatsink_temp_l, 16), pack(motor_temp_r, 16), pack(heatsink_temp_r, 16)])
)
messages_fast.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_SINK_TEMPS)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_DSP_BOARD_TEMPS = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_DSP_BOARD_TEMPS_ID.value,
    data=b''.join([pack(dsp_temp_l, 16), pack(dsp_temp_r, 16)])
)
messages_fast.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_DSP_BOARD_TEMPS)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_MC_STATUS = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MC_STATUS_ID.value,
    data=b''.join([pack(limit_bitset_l, 8), pack(error_bitset_l, 8), pack(limit_bitset_r, 8), pack(error_bitset_r, 8), pack(board_fault_bitset, 8), pack(overtemp_bitset, 8), pack(precharge_status, 8)])
)
messages_slow.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_MC_STATUS)

SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_FAST_ONE_SHOT_MSG = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_FAST_ONE_SHOT_MSG_ID.value,
    data=b''.join([pack(sig1, 16), pack(sig2, 16)])
)
messages_fast.append(SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_FAST_ONE_SHOT_MSG)

SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_MEDIUM_ONE_SHOT_MSG = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_MEDIUM_ONE_SHOT_MSG_ID.value,
    data=b''.join([pack(sig1, 16), pack(sig2, 16)])
)
messages_medium.append(SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_MEDIUM_ONE_SHOT_MSG)

SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_SLOW_ONE_SHOT_MSG = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_SLOW_ONE_SHOT_MSG_ID.value,
    data=b''.join([pack(sig1, 16), pack(sig2, 16)])
)
messages_slow.append(SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_SLOW_ONE_SHOT_MSG)

SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_PEDAL = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_PEDAL_ID.value,
    data=b''.join([pack(throttle_output, 32), pack(brake_output, 8)])
)
messages_fast.append(SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_PEDAL)

SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_INFO = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_INFO_ID.value,
    data=b''.join([pack(target_velocity, 32), pack(drive_state, 8), pack(cruise_control, 8), pack(regen_braking, 8), pack(hazard_enabled, 8)])
)
messages_medium.append(SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_INFO)

SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_STEERING = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_STEERING_ID.value,
    data=b''.join([pack(input_cc, 8), pack(input_lights, 8)])
)
messages_medium.append(SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_STEERING)

SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_REGEN_PERCENTAGE = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_REGEN_PERCENTAGE_ID.value,
    data=b''.join([pack(percent, 32)])
)
messages_medium.append(SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_REGEN_PERCENTAGE)

SYSTEM_CAN_MESSAGE_TELEMETRY_TELEMETRY = Message(
    arbitration_id=SystemCanMessageId.SYSTEM_CAN_MESSAGE_TELEMETRY_TELEMETRY_ID.value,
    data=b''.join([pack(data, 64)])
)
messages_slow.append(SYSTEM_CAN_MESSAGE_TELEMETRY_TELEMETRY)

def main():
    bus = Bus(channel='virtual', bustype='virtual')
    print("CAN simulator (static) starting on virtual bus")
    
    # OLD LIBRARY REQUIREMENT: Lock must be passed
    bus_lock = Lock()
    all_tasks = []

    # OLD LIBRARY REQUIREMENT: One task per unique Arbitration ID
    for msg in messages_fast:
        task = ThreadBasedCyclicSendTask(bus=bus, messages=msg, period=0.001, lock=bus_lock)
        all_tasks.append(task)
    
    for msg in messages_medium:
        task = ThreadBasedCyclicSendTask(bus=bus, messages=msg, period=0.1, lock=bus_lock)
        all_tasks.append(task)
    
    for msg in messages_slow:
        task = ThreadBasedCyclicSendTask(bus=bus, messages=msg, period=1.0, lock=bus_lock)
        all_tasks.append(task)

    for task in all_tasks:
        task.start()

    print(f"Simulator running with {len(all_tasks)} cyclic tasks. Press Ctrl+C to stop.")
=======
        return struct.pack("f", num)
    elif size == 32:
        return struct.pack("I", num)
    elif size == 16:
        return struct.pack("H", num)
    elif size == 8:
        return struct.pack("B", num)
    elif size == 64:
        return struct.pack("Q", num)
    else:
        raise ValueError(f"Unknown size {size} for number {num}")

can_data_cache = CanSimulatorDataCache()

system_can_message_can_communication_fast_one_shot_msg = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_FAST_ONE_SHOT_MSG_ID,
data = bytearray(pack(can_data_cache.get("can_communication_fast_one_shot_msg", "sig1"), 16) + pack(can_data_cache.get("can_communication_fast_one_shot_msg", "sig2"), 16)))

messages_fast.append(system_can_message_can_communication_fast_one_shot_msg)

system_can_message_can_communication_medium_one_shot_msg = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_MEDIUM_ONE_SHOT_MSG_ID,
data = bytearray(pack(can_data_cache.get("can_communication_medium_one_shot_msg", "sig1"), 16) + pack(can_data_cache.get("can_communication_medium_one_shot_msg", "sig2"), 16)))

messages_medium.append(system_can_message_can_communication_medium_one_shot_msg)

system_can_message_can_communication_slow_one_shot_msg = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_SLOW_ONE_SHOT_MSG_ID,
data = bytearray(pack(can_data_cache.get("can_communication_slow_one_shot_msg", "sig1"), 16) + pack(can_data_cache.get("can_communication_slow_one_shot_msg", "sig2"), 16)))

messages_slow.append(system_can_message_can_communication_slow_one_shot_msg)

system_can_message_front_controller_drive_status = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_FRONT_CONTROLLER_DRIVE_STATUS_ID,
data = bytearray(pack(can_data_cache.get("front_controller_drive_status", "pedal_percentage"), 32) + pack(can_data_cache.get("front_controller_drive_status", "state_data"), 8)))

messages_medium.append(system_can_message_front_controller_drive_status)

system_can_message_front_controller_motor_velocity = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_FRONT_CONTROLLER_MOTOR_VELOCITY_ID,
data = bytearray(pack(can_data_cache.get("front_controller_motor_velocity", "vehicle_velocity"), 32) + pack(can_data_cache.get("front_controller_motor_velocity", "motor_velocity"), 32)))

messages_medium.append(system_can_message_front_controller_motor_velocity)

system_can_message_front_controller_motor_temperature = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_FRONT_CONTROLLER_MOTOR_TEMPERATURE_ID,
data = bytearray(pack(can_data_cache.get("front_controller_motor_temperature", "heat_sink_temp"), 32) + pack(can_data_cache.get("front_controller_motor_temperature", "motor_temp"), 32)))

messages_medium.append(system_can_message_front_controller_motor_temperature)

system_can_message_front_controller_fc_power_group_a = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_FRONT_CONTROLLER_FC_POWER_GROUP_A_ID,
data = bytearray(pack(can_data_cache.get("front_controller_fc_power_group_a", "rev_cam_current"), 16) + pack(can_data_cache.get("front_controller_fc_power_group_a", "telem_current"), 16) + pack(can_data_cache.get("front_controller_fc_power_group_a", "steering_current"), 16) + pack(can_data_cache.get("front_controller_fc_power_group_a", "driver_fan_current"), 16)))

messages_slow.append(system_can_message_front_controller_fc_power_group_a)

system_can_message_front_controller_fc_power_group_b = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_FRONT_CONTROLLER_FC_POWER_GROUP_B_ID,
data = bytearray(pack(can_data_cache.get("front_controller_fc_power_group_b", "horn_current"), 16) + pack(can_data_cache.get("front_controller_fc_power_group_b", "spare_current"), 16)))

messages_slow.append(system_can_message_front_controller_fc_power_group_b)

system_can_message_front_controller_fc_power_lights_group = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_FRONT_CONTROLLER_FC_POWER_LIGHTS_GROUP_ID,
data = bytearray(pack(can_data_cache.get("front_controller_fc_power_lights_group", "brake_light_sig_current"), 16) + pack(can_data_cache.get("front_controller_fc_power_lights_group", "bps_light_sig_current"), 16) + pack(can_data_cache.get("front_controller_fc_power_lights_group", "right_sig_current"), 16) + pack(can_data_cache.get("front_controller_fc_power_lights_group", "left_sig_current"), 16)))

messages_slow.append(system_can_message_front_controller_fc_power_lights_group)

system_can_message_imu_gyro_data = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_IMU_GYRO_DATA_ID,
data = bytearray(pack(can_data_cache.get("imu_gyro_data", "x_axis"), 16) + pack(can_data_cache.get("imu_gyro_data", "y_axis"), 16) + pack(can_data_cache.get("imu_gyro_data", "z_axis"), 16)))

messages_medium.append(system_can_message_imu_gyro_data)

system_can_message_imu_accel_data = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_IMU_ACCEL_DATA_ID,
data = bytearray(pack(can_data_cache.get("imu_accel_data", "x_axis"), 16) + pack(can_data_cache.get("imu_accel_data", "y_axis"), 16) + pack(can_data_cache.get("imu_accel_data", "z_axis"), 16)))

messages_medium.append(system_can_message_imu_accel_data)

system_can_message_rear_controller_rear_controller_status = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_REAR_CONTROLLER_REAR_CONTROLLER_STATUS_ID,
data = bytearray(pack(can_data_cache.get("rear_controller_rear_controller_status", "triggers"), 8)))

messages_medium.append(system_can_message_rear_controller_rear_controller_status)

system_can_message_rear_controller_battery_stats_a = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_REAR_CONTROLLER_BATTERY_STATS_A_ID,
data = bytearray(pack(can_data_cache.get("rear_controller_battery_stats_a", "pack_voltage"), 16) + pack(can_data_cache.get("rear_controller_battery_stats_a", "pack_current"), 16) + pack(can_data_cache.get("rear_controller_battery_stats_a", "pack_soc"), 16)))

messages_medium.append(system_can_message_rear_controller_battery_stats_a)

system_can_message_rear_controller_battery_stats_b = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_REAR_CONTROLLER_BATTERY_STATS_B_ID,
data = bytearray(pack(can_data_cache.get("rear_controller_battery_stats_b", "max_cell_voltage"), 16) + pack(can_data_cache.get("rear_controller_battery_stats_b", "min_cell_voltage"), 16) + pack(can_data_cache.get("rear_controller_battery_stats_b", "max_temperature"), 16)))

messages_medium.append(system_can_message_rear_controller_battery_stats_b)

system_can_message_rear_controller_power_input_stats = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_REAR_CONTROLLER_POWER_INPUT_STATS_ID,
data = bytearray(pack(can_data_cache.get("rear_controller_power_input_stats", "input_dcdc_voltage"), 16) + pack(can_data_cache.get("rear_controller_power_input_stats", "input_dcdc_current"), 16) + pack(can_data_cache.get("rear_controller_power_input_stats", "input_aux_voltage"), 16) + pack(can_data_cache.get("rear_controller_power_input_stats", "input_aux_current"), 16)))

messages_medium.append(system_can_message_rear_controller_power_input_stats)

system_can_message_rear_controller_afe1_status_a = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_REAR_CONTROLLER_AFE1_STATUS_A_ID,
data = bytearray(pack(can_data_cache.get("rear_controller_afe1_status_a", "id"), 8) + pack(can_data_cache.get("rear_controller_afe1_status_a", "voltage_0"), 16) + pack(can_data_cache.get("rear_controller_afe1_status_a", "voltage_1"), 16) + pack(can_data_cache.get("rear_controller_afe1_status_a", "voltage_2"), 16)))

messages_medium.append(system_can_message_rear_controller_afe1_status_a)

system_can_message_rear_controller_afe1_status_b = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_REAR_CONTROLLER_AFE1_STATUS_B_ID,
data = bytearray(pack(can_data_cache.get("rear_controller_afe1_status_b", "id"), 8) + pack(can_data_cache.get("rear_controller_afe1_status_b", "voltage_0"), 16) + pack(can_data_cache.get("rear_controller_afe1_status_b", "voltage_1"), 16) + pack(can_data_cache.get("rear_controller_afe1_status_b", "voltage_2"), 16)))

messages_medium.append(system_can_message_rear_controller_afe1_status_b)

system_can_message_rear_controller_afe2_status_a = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_REAR_CONTROLLER_AFE2_STATUS_A_ID,
data = bytearray(pack(can_data_cache.get("rear_controller_afe2_status_a", "id"), 8) + pack(can_data_cache.get("rear_controller_afe2_status_a", "voltage_0"), 16) + pack(can_data_cache.get("rear_controller_afe2_status_a", "voltage_1"), 16) + pack(can_data_cache.get("rear_controller_afe2_status_a", "voltage_2"), 16)))

messages_medium.append(system_can_message_rear_controller_afe2_status_a)

system_can_message_rear_controller_afe2_status_b = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_REAR_CONTROLLER_AFE2_STATUS_B_ID,
data = bytearray(pack(can_data_cache.get("rear_controller_afe2_status_b", "id"), 8) + pack(can_data_cache.get("rear_controller_afe2_status_b", "voltage_0"), 16) + pack(can_data_cache.get("rear_controller_afe2_status_b", "voltage_1"), 16) + pack(can_data_cache.get("rear_controller_afe2_status_b", "voltage_2"), 16)))

messages_medium.append(system_can_message_rear_controller_afe2_status_b)

system_can_message_rear_controller_afe_temperature = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_REAR_CONTROLLER_AFE_TEMPERATURE_ID,
data = bytearray(pack(can_data_cache.get("rear_controller_afe_temperature", "id"), 8) + pack(can_data_cache.get("rear_controller_afe_temperature", "temperature_0"), 8) + pack(can_data_cache.get("rear_controller_afe_temperature", "temperature_1"), 8) + pack(can_data_cache.get("rear_controller_afe_temperature", "temperature_2"), 8) + pack(can_data_cache.get("rear_controller_afe_temperature", "temperature_3"), 8) + pack(can_data_cache.get("rear_controller_afe_temperature", "temperature_4"), 8) + pack(can_data_cache.get("rear_controller_afe_temperature", "temperature_5"), 8) + pack(can_data_cache.get("rear_controller_afe_temperature", "temperature_6"), 8)))

messages_medium.append(system_can_message_rear_controller_afe_temperature)

system_can_message_steering_steering = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_STEERING_STEERING_ID,
data = bytearray(pack(can_data_cache.get("steering_steering", "cruise_control_target_velocity"), 32) + pack(can_data_cache.get("steering_steering", "buttons"), 8)))

messages_medium.append(system_can_message_steering_steering)

system_can_message_telemetry_imu_data = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_TELEMETRY_IMU_DATA_ID,
data = bytearray(pack(can_data_cache.get("telemetry_imu_data", "g_force"), 16) + pack(can_data_cache.get("telemetry_imu_data", "roll"), 16) + pack(can_data_cache.get("telemetry_imu_data", "pitch"), 16) + pack(can_data_cache.get("telemetry_imu_data", "yaw"), 16)))

messages_medium.append(system_can_message_telemetry_imu_data)
def periodic_sender(bus, messages, period, stop_event):
    # Send messages with small delays between each one to avoid buffer overflow
    while not stop_event.is_set():
        start_time = time.time()

        # Calculate time per message to spread them over the period
        if len(messages) > 0:
            message_delay = min(0.01, period / (len(messages) * 2))  # Don't use more than half the period for delays
        else:
            message_delay = 0
            
        for msg in messages:
            try:
                bus.send(msg)
                time.sleep(message_delay)
            except Exception as e:
                print(f"Error sending message {msg.arbitration_id}: {e}")
                time.sleep(RETRY_TRANSMIT_S)
                try:
                    bus.send(msg)
                except Exception as e2:
                    print(f"Failed retry for {msg.arbitration_id}: {e2}")

        # Calculate remaining time in the period
        elapsed = time.time() - start_time
        sleep_time = max(0, period - elapsed)
        time.sleep(sleep_time)

def main():
    can_bus = Bus(channel='can0', interface='socketcan')

    stop_event = threading.Event()

    fast_thread = Thread(
        target=periodic_sender,
        args=(can_bus, messages_fast, FAST_CYCLE_PERIOD, stop_event),
        daemon=True
    )
    
    medium_thread = Thread(
        target=periodic_sender,
        args=(can_bus, messages_medium, MEDIUM_CYCLE_PERIOD, stop_event),
        daemon=True
    )
    
    slow_thread = Thread(
        target=periodic_sender,
        args=(can_bus, messages_slow, SLOW_CYCLE_PERIOD, stop_event),
        daemon=True
    )

    fast_thread.start()
    medium_thread.start()
    slow_thread.start()
    
>>>>>>> bed2dba89b527b04337236b4c1cd0c3393f1ae14
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
<<<<<<< HEAD
        print("\nStopping simulator...")
        for task in all_tasks:
            task.stop()
        bus.shutdown()
        print("Shutdown complete.")
=======
        print("Shutting down CAN simulator...")
        stop_event.set()
        time.sleep(0.5)
>>>>>>> bed2dba89b527b04337236b4c1cd0c3393f1ae14

if __name__ == "__main__":
    import threading
    main()