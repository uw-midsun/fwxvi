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

messages_fast = []
messages_medium = []
messages_slow = []

def pack(num, size):
    if isinstance(num, float) and size == 32:
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

system_can_message_telemetry_telemetry = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_TELEMETRY_TELEMETRY_ID,
data = bytearray(pack(can_data_cache.get("telemetry_telemetry", "data"), 64)))

messages_slow.append(system_can_message_telemetry_telemetry)

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

system_can_message_imu_gyro_data = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_IMU_GYRO_DATA_ID,
data = bytearray(pack(can_data_cache.get("imu_gyro_data", "x_axis"), 16) + pack(can_data_cache.get("imu_gyro_data", "y_axis"), 16) + pack(can_data_cache.get("imu_gyro_data", "z_axis"), 16)))

messages_medium.append(system_can_message_imu_gyro_data)

system_can_message_imu_accel_data = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_IMU_ACCEL_DATA_ID,
data = bytearray(pack(can_data_cache.get("imu_accel_data", "x_axis"), 16) + pack(can_data_cache.get("imu_accel_data", "y_axis"), 16) + pack(can_data_cache.get("imu_accel_data", "z_axis"), 16)))

messages_medium.append(system_can_message_imu_accel_data)

system_can_message_bms_carrier_battery_status = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_STATUS_ID,
data = bytearray(pack(can_data_cache.get("bms_carrier_battery_status", "fault"), 16) + pack(can_data_cache.get("bms_carrier_battery_status", "fault_val"), 16) + pack(can_data_cache.get("bms_carrier_battery_status", "aux_batt_v"), 16) + pack(can_data_cache.get("bms_carrier_battery_status", "afe_status"), 8)))

messages_medium.append(system_can_message_bms_carrier_battery_status)

system_can_message_bms_carrier_battery_vt = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT_ID,
data = bytearray(pack(can_data_cache.get("bms_carrier_battery_vt", "voltage"), 16) + pack(can_data_cache.get("bms_carrier_battery_vt", "current"), 16) + pack(can_data_cache.get("bms_carrier_battery_vt", "temperature"), 16) + pack(can_data_cache.get("bms_carrier_battery_vt", "batt_perc"), 16)))

messages_fast.append(system_can_message_bms_carrier_battery_vt)

system_can_message_bms_carrier_battery_info = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_INFO_ID,
data = bytearray(pack(can_data_cache.get("bms_carrier_battery_info", "fan1"), 8) + pack(can_data_cache.get("bms_carrier_battery_info", "fan2"), 8) + pack(can_data_cache.get("bms_carrier_battery_info", "max_cell_v"), 16) + pack(can_data_cache.get("bms_carrier_battery_info", "min_cell_v"), 16)))

messages_slow.append(system_can_message_bms_carrier_battery_info)

system_can_message_bms_carrier_mc_status = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MC_STATUS_ID,
data = bytearray(pack(can_data_cache.get("bms_carrier_mc_status", "limit_bitset_l"), 8) + pack(can_data_cache.get("bms_carrier_mc_status", "error_bitset_l"), 8) + pack(can_data_cache.get("bms_carrier_mc_status", "limit_bitset_r"), 8) + pack(can_data_cache.get("bms_carrier_mc_status", "error_bitset_r"), 8) + pack(can_data_cache.get("bms_carrier_mc_status", "board_fault_bitset"), 8) + pack(can_data_cache.get("bms_carrier_mc_status", "overtemp_bitset"), 8) + pack(can_data_cache.get("bms_carrier_mc_status", "precharge_status"), 8)))

messages_medium.append(system_can_message_bms_carrier_mc_status)

system_can_message_bms_carrier_motor_controller_vc = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_CONTROLLER_VC_ID,
data = bytearray(pack(can_data_cache.get("bms_carrier_motor_controller_vc", "mc_voltage_l"), 16) + pack(can_data_cache.get("bms_carrier_motor_controller_vc", "mc_current_l"), 16) + pack(can_data_cache.get("bms_carrier_motor_controller_vc", "mc_voltage_r"), 16) + pack(can_data_cache.get("bms_carrier_motor_controller_vc", "mc_current_r"), 16)))

messages_medium.append(system_can_message_bms_carrier_motor_controller_vc)

system_can_message_bms_carrier_motor_velocity = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_VELOCITY_ID,
data = bytearray(pack(can_data_cache.get("bms_carrier_motor_velocity", "velocity_l"), 16) + pack(can_data_cache.get("bms_carrier_motor_velocity", "velocity_r"), 16) + pack(can_data_cache.get("bms_carrier_motor_velocity", "brakes_enabled"), 8)))

messages_medium.append(system_can_message_bms_carrier_motor_velocity)

system_can_message_bms_carrier_motor_sink_temps = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_SINK_TEMPS_ID,
data = bytearray(pack(can_data_cache.get("bms_carrier_motor_sink_temps", "motor_temp_l"), 16) + pack(can_data_cache.get("bms_carrier_motor_sink_temps", "heatsink_temp_l"), 16) + pack(can_data_cache.get("bms_carrier_motor_sink_temps", "motor_temp_r"), 16) + pack(can_data_cache.get("bms_carrier_motor_sink_temps", "heatsink_temp_r"), 16)))

messages_medium.append(system_can_message_bms_carrier_motor_sink_temps)

system_can_message_bms_carrier_dsp_board_temps = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_DSP_BOARD_TEMPS_ID,
data = bytearray(pack(can_data_cache.get("bms_carrier_dsp_board_temps", "dsp_temp_l"), 16) + pack(can_data_cache.get("bms_carrier_dsp_board_temps", "dsp_temp_r"), 16)))

messages_medium.append(system_can_message_bms_carrier_dsp_board_temps)

system_can_message_bms_carrier_battery_relay_info = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_RELAY_INFO_ID,
data = bytearray(pack(can_data_cache.get("bms_carrier_battery_relay_info", "state"), 8)))

messages_slow.append(system_can_message_bms_carrier_battery_relay_info)

system_can_message_bms_carrier_afe1_status = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE1_STATUS_ID,
data = bytearray(pack(can_data_cache.get("bms_carrier_afe1_status", "id"), 8) + pack(can_data_cache.get("bms_carrier_afe1_status", "temp"), 8) + pack(can_data_cache.get("bms_carrier_afe1_status", "v1"), 16) + pack(can_data_cache.get("bms_carrier_afe1_status", "v2"), 16) + pack(can_data_cache.get("bms_carrier_afe1_status", "v3"), 16)))

messages_slow.append(system_can_message_bms_carrier_afe1_status)

system_can_message_bms_carrier_afe2_status = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE2_STATUS_ID,
data = bytearray(pack(can_data_cache.get("bms_carrier_afe2_status", "id"), 8) + pack(can_data_cache.get("bms_carrier_afe2_status", "temp"), 8) + pack(can_data_cache.get("bms_carrier_afe2_status", "v1"), 16) + pack(can_data_cache.get("bms_carrier_afe2_status", "v2"), 16) + pack(can_data_cache.get("bms_carrier_afe2_status", "v3"), 16)))

messages_slow.append(system_can_message_bms_carrier_afe2_status)

system_can_message_bms_carrier_afe3_status = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE3_STATUS_ID,
data = bytearray(pack(can_data_cache.get("bms_carrier_afe3_status", "id"), 8) + pack(can_data_cache.get("bms_carrier_afe3_status", "temp"), 8) + pack(can_data_cache.get("bms_carrier_afe3_status", "v1"), 16) + pack(can_data_cache.get("bms_carrier_afe3_status", "v2"), 16) + pack(can_data_cache.get("bms_carrier_afe3_status", "v3"), 16)))

messages_slow.append(system_can_message_bms_carrier_afe3_status)

system_can_message_centre_console_cc_pedal = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_PEDAL_ID,
data = bytearray(pack(can_data_cache.get("centre_console_cc_pedal", "throttle_output"), 32) + pack(can_data_cache.get("centre_console_cc_pedal", "brake_output"), 8)))

messages_fast.append(system_can_message_centre_console_cc_pedal)

system_can_message_centre_console_cc_info = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_INFO_ID,
data = bytearray(pack(can_data_cache.get("centre_console_cc_info", "target_velocity"), 32) + pack(can_data_cache.get("centre_console_cc_info", "drive_state"), 8) + pack(can_data_cache.get("centre_console_cc_info", "cruise_control"), 8) + pack(can_data_cache.get("centre_console_cc_info", "regen_braking"), 8) + pack(can_data_cache.get("centre_console_cc_info", "hazard_enabled"), 8)))

messages_medium.append(system_can_message_centre_console_cc_info)

system_can_message_centre_console_cc_steering = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_STEERING_ID,
data = bytearray(pack(can_data_cache.get("centre_console_cc_steering", "input_cc"), 8) + pack(can_data_cache.get("centre_console_cc_steering", "input_lights"), 8)))

messages_medium.append(system_can_message_centre_console_cc_steering)

system_can_message_centre_console_cc_regen_percentage = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_REGEN_PERCENTAGE_ID,
data = bytearray(pack(can_data_cache.get("centre_console_cc_regen_percentage", "percent"), 32)))

messages_medium.append(system_can_message_centre_console_cc_regen_percentage)
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
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Shutting down CAN simulator...")
        stop_event.set()
        time.sleep(0.5)

if __name__ == "__main__":
    import threading
    main()