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

system_can_message_front_controller_front_controller_pedal_data = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_FRONT_CONTROLLER_FRONT_CONTROLLER_PEDAL_DATA_ID,
data = bytearray(pack(can_data_cache.get("front_controller_front_controller_pedal_data", "percentage"), 32) + pack(can_data_cache.get("front_controller_front_controller_pedal_data", "brake_enabled"), 8)))

messages_medium.append(system_can_message_front_controller_front_controller_pedal_data)

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
data = bytearray(pack(can_data_cache.get("rear_controller_rear_controller_status", "bps_fault"), 16) + pack(can_data_cache.get("rear_controller_rear_controller_status", "relay_state"), 8) + pack(can_data_cache.get("rear_controller_rear_controller_status", "power_state"), 8) + pack(can_data_cache.get("rear_controller_rear_controller_status", "afe_status"), 8)))

messages_medium.append(system_can_message_rear_controller_rear_controller_status)

system_can_message_rear_controller_battery_stats_a = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_REAR_CONTROLLER_BATTERY_STATS_A_ID,
data = bytearray(pack(can_data_cache.get("rear_controller_battery_stats_a", "pack_voltage"), 16) + pack(can_data_cache.get("rear_controller_battery_stats_a", "pack_current"), 16) + pack(can_data_cache.get("rear_controller_battery_stats_a", "pack_soc"), 16)))

messages_medium.append(system_can_message_rear_controller_battery_stats_a)

system_can_message_rear_controller_battery_stats_b = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_REAR_CONTROLLER_BATTERY_STATS_B_ID,
data = bytearray(pack(can_data_cache.get("rear_controller_battery_stats_b", "max_cell_voltage"), 16) + pack(can_data_cache.get("rear_controller_battery_stats_b", "min_cell_voltage"), 16) + pack(can_data_cache.get("rear_controller_battery_stats_b", "max_temperature"), 16) + pack(can_data_cache.get("rear_controller_battery_stats_b", "motor_precharge_complete"), 8)))

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

system_can_message_steering_steering_state = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_STEERING_STEERING_STATE_ID,
data = bytearray(pack(can_data_cache.get("steering_steering_state", "target_velocity"), 32) + pack(can_data_cache.get("steering_steering_state", "drive_state"), 8) + pack(can_data_cache.get("steering_steering_state", "cruise_control"), 8) + pack(can_data_cache.get("steering_steering_state", "regen_braking"), 8) + pack(can_data_cache.get("steering_steering_state", "hazard_enabled"), 8)))

messages_medium.append(system_can_message_steering_steering_state)

system_can_message_telemetry_telemetry = Message(
arbitration_id = SystemCanMessageId.SYSTEM_CAN_MESSAGE_TELEMETRY_TELEMETRY_ID,
data = bytearray(pack(can_data_cache.get("telemetry_telemetry", "telemetry_data"), 64)))

messages_slow.append(system_can_message_telemetry_telemetry)
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