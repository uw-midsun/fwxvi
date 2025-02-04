import struct
import time
from can import Message
from can.interface import Bus
from can.broadcastmanager import ThreadBasedCyclicSendTask
from enum import Enum

SYSTEM_CAN_MESSAGE_ID_OFFSET = 4
SYSTEM_CAN_MESSAGE_PRIORITY_BIT = 0x400

class SystemCanDevice(Enum):
  SYSTEM_CAN_DEVICE_BMS_CARRIER = 0
  SYSTEM_CAN_DEVICE_CAN_COMMUNICATION = 1
  SYSTEM_CAN_DEVICE_CENTRE_CONSOLE = 2
  SYSTEM_CAN_DEVICE_TELEMETRY = 3
  NUM_SYSTEM_CAN_DEVICES = 4

  
class SystemCanMessageId(Enum):
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_STATUS_ID = (1 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT_ID = (2 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_INFO_ID = (3 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE1_STATUS_ID =  SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (61 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE2_STATUS_ID =  SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (62 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE3_STATUS_ID =  SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (63 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_CONTROLLER_VC_ID =  SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (57 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_VELOCITY_ID = (58 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_SINK_TEMPS_ID =  SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (59 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_DSP_BOARD_TEMPS_ID =  SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (60 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER
    SYSTEM_CAN_MESSAGE_BMS_CARRIER_MC_STATUS_ID = (8 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_BMS_CARRIER
    SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_FAST_ONE_SHOT_MSG_ID =  SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (56 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_CAN_COMMUNICATION
    SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_MEDIUM_ONE_SHOT_MSG_ID =  SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (55 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_CAN_COMMUNICATION
    SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_SLOW_ONE_SHOT_MSG_ID =  SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (54 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_CAN_COMMUNICATION
    SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_PEDAL_ID = (4 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_CENTRE_CONSOLE
    SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_INFO_ID = (5 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_CENTRE_CONSOLE
    SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_STEERING_ID = (6 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_CENTRE_CONSOLE
    SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_REGEN_PERCENTAGE_ID = (7 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_CENTRE_CONSOLE
    SYSTEM_CAN_MESSAGE_TELEMETRY_TELEMETRY_ID =  SYSTEM_CAN_MESSAGE_PRIORITY_BIT + (53 << SYSTEM_CAN_MESSAGE_ID_OFFSET) + SystemCanDevice.SYSTEM_CAN_DEVICE_TELEMETRY

messages_fast = []
messages_medium = []
messages_slow = []

def pack(num, size):
    if isinstance(num, float) and size == 32:
        return struct.pack("f", num).hex()
    elif size == 32:
        return struct.pack("i", num).hex()
    elif size == 16:
        return struct.pack("h", num).hex()
    elif size == 8:
        return struct.pack("b", num).hex()

SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_STATUS = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_STATUS_ID,
data = bytearray([pack(fault, 16) + pack(fault_val, 16) + pack(aux_batt_v, 16) + pack(afe_status, 8)]))

messages_medium.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_STATUS)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT_ID,
data = bytearray([pack(voltage, 16) + pack(current, 16) + pack(temperature, 16) + pack(batt_perc, 16)]))

messages_fast.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_INFO = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_INFO_ID,
data = bytearray([pack(fan1, 8) + pack(fan2, 8) + pack(max_cell_v, 16) + pack(min_cell_v, 16)]))

messages_slow.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_INFO)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE1_STATUS = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE1_STATUS_ID,
data = bytearray([pack(id, 8) + pack(temp, 8) + pack(v1, 16) + pack(v2, 16) + pack(v3, 16)]))

messages_slow.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE1_STATUS)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE2_STATUS = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE2_STATUS_ID,
data = bytearray([pack(id, 8) + pack(temp, 8) + pack(v1, 16) + pack(v2, 16) + pack(v3, 16)]))

messages_slow.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE2_STATUS)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE3_STATUS = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE3_STATUS_ID,
data = bytearray([pack(id, 8) + pack(temp, 8) + pack(v1, 16) + pack(v2, 16) + pack(v3, 16)]))

messages_slow.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE3_STATUS)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_CONTROLLER_VC = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_CONTROLLER_VC_ID,
data = bytearray([pack(mc_voltage_l, 16) + pack(mc_current_l, 16) + pack(mc_voltage_r, 16) + pack(mc_current_r, 16)]))

messages_fast.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_CONTROLLER_VC)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_VELOCITY = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_VELOCITY_ID,
data = bytearray([pack(velocity_l, 16) + pack(velocity_r, 16) + pack(brakes_enabled, 8)]))

messages_fast.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_VELOCITY)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_SINK_TEMPS = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_SINK_TEMPS_ID,
data = bytearray([pack(motor_temp_l, 16) + pack(heatsink_temp_l, 16) + pack(motor_temp_r, 16) + pack(heatsink_temp_r, 16)]))

messages_fast.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_SINK_TEMPS)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_DSP_BOARD_TEMPS = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_DSP_BOARD_TEMPS_ID,
data = bytearray([pack(dsp_temp_l, 16) + pack(dsp_temp_r, 16)]))

messages_fast.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_DSP_BOARD_TEMPS)

SYSTEM_CAN_MESSAGE_BMS_CARRIER_MC_STATUS = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MC_STATUS_ID,
data = bytearray([pack(limit_bitset_l, 8) + pack(error_bitset_l, 8) + pack(limit_bitset_r, 8) + pack(error_bitset_r, 8) + pack(board_fault_bitset, 8) + pack(overtemp_bitset, 8) + pack(precharge_status, 8)]))

messages_slow.append(SYSTEM_CAN_MESSAGE_BMS_CARRIER_MC_STATUS)

SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_FAST_ONE_SHOT_MSG = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_FAST_ONE_SHOT_MSG_ID,
data = bytearray([pack(sig1, 16) + pack(sig2, 16)]))

messages_fast.append(SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_FAST_ONE_SHOT_MSG)

SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_MEDIUM_ONE_SHOT_MSG = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_MEDIUM_ONE_SHOT_MSG_ID,
data = bytearray([pack(sig1, 16) + pack(sig2, 16)]))

messages_medium.append(SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_MEDIUM_ONE_SHOT_MSG)

SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_SLOW_ONE_SHOT_MSG = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_SLOW_ONE_SHOT_MSG_ID,
data = bytearray([pack(sig1, 16) + pack(sig2, 16)]))

messages_slow.append(SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_SLOW_ONE_SHOT_MSG)

SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_PEDAL = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_PEDAL_ID,
data = bytearray([pack(throttle_output, 32) + pack(brake_output, 8)]))

messages_fast.append(SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_PEDAL)

SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_INFO = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_INFO_ID,
data = bytearray([pack(target_velocity, 32) + pack(drive_state, 8) + pack(cruise_control, 8) + pack(regen_braking, 8) + pack(hazard_enabled, 8)]))

messages_medium.append(SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_INFO)

SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_STEERING = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_STEERING_ID,
data = bytearray([pack(input_cc, 8) + pack(input_lights, 8)]))

messages_medium.append(SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_STEERING)

SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_REGEN_PERCENTAGE = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_REGEN_PERCENTAGE_ID,
data = bytearray([pack(percent, 32)]))

messages_medium.append(SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_REGEN_PERCENTAGE)

SYSTEM_CAN_MESSAGE_TELEMETRY_TELEMETRY = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_TELEMETRY_TELEMETRY_ID,
data = bytearray([pack(data, 64)]))

messages_slow.append(SYSTEM_CAN_MESSAGE_TELEMETRY_TELEMETRY)

def main():
    bus = Bus(channel='virtual', bustype='socketcan')

    task_fast = ThreadBasedCyclicSendTask(bus=bus, messages=messages_fast, period=0.001)
    task_medium = ThreadBasedCyclicSendTask(bus=bus, messages=messages_medium, period=0.1)
    task_slow = ThreadBasedCyclicSendTask(bus=bus, messages=messages_slow, period=1.0)

    task_fast.start()
    task_medium.start()
    task_slow.start()

if __name__ == "__main__":
    main()