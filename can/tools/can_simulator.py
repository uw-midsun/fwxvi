## @defgroup CANSimulator CAN Simulation tools
#  This package contains CAN simulation tools for testing the full vehicle
#
## @file    can_simulator.py
#  @date    2025-03-09
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Main simulation script
#
#  @ingroup CANSimulator
import struct
import time
from threading import Lock, Thread
from can import Message
from can.interface import Bus
from can.broadcastmanager import ThreadBasedCyclicSendTask
from system_can import *
from can_simulator_cache import *

RETRY_TRANSMIT_S = 0.05

FAST_CYCLE_PERIOD = 0.5
MEDIUM_CYCLE_PERIOD = 5
SLOW_CYCLE_PERIOD = 10

messages_fast = []
messages_medium = []
messages_slow = []

# Dictionary to map message types to their respective sizes
message_type_sizes = {
    "telemetry_telemetry": 64,
    "can_communication_fast_one_shot_msg": 32,
    "can_communication_medium_one_shot_msg": 32,
    "can_communication_slow_one_shot_msg": 32,
    "imu_gyro_data": 48,
    "imu_accel_data": 48,
    "bms_carrier_battery_status": 56,
    "bms_carrier_battery_vt": 64,
    "bms_carrier_battery_info": 48,
    "bms_carrier_mc_status": 56,
    "bms_carrier_motor_controller_vc": 64,
    "bms_carrier_motor_velocity": 40,
    "bms_carrier_motor_sink_temps": 64,
    "bms_carrier_dsp_board_temps": 32,
    "bms_carrier_battery_relay_info": 8,
    "bms_carrier_afe1_status": 64,
    "bms_carrier_afe2_status": 64,
    "bms_carrier_afe3_status": 64,
    "centre_console_cc_pedal": 40,
    "centre_console_cc_info": 64,
    "centre_console_cc_steering": 16,
    "centre_console_cc_regen_percentage": 32
}

# Dictionary to map message types to their respective arbitration IDs
message_type_ids = {
    "telemetry_telemetry": SystemCanMessageId.SYSTEM_CAN_MESSAGE_TELEMETRY_TELEMETRY_ID,
    "can_communication_fast_one_shot_msg": SystemCanMessageId.SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_FAST_ONE_SHOT_MSG_ID,
    "can_communication_medium_one_shot_msg": SystemCanMessageId.SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_MEDIUM_ONE_SHOT_MSG_ID,
    "can_communication_slow_one_shot_msg": SystemCanMessageId.SYSTEM_CAN_MESSAGE_CAN_COMMUNICATION_SLOW_ONE_SHOT_MSG_ID,
    "imu_gyro_data": SystemCanMessageId.SYSTEM_CAN_MESSAGE_IMU_GYRO_DATA_ID,
    "imu_accel_data": SystemCanMessageId.SYSTEM_CAN_MESSAGE_IMU_ACCEL_DATA_ID,
    "bms_carrier_battery_status": SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_STATUS_ID,
    "bms_carrier_battery_vt": SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_VT_ID,
    "bms_carrier_battery_info": SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_INFO_ID,
    "bms_carrier_mc_status": SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MC_STATUS_ID,
    "bms_carrier_motor_controller_vc": SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_CONTROLLER_VC_ID,
    "bms_carrier_motor_velocity": SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_VELOCITY_ID,
    "bms_carrier_motor_sink_temps": SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_MOTOR_SINK_TEMPS_ID,
    "bms_carrier_dsp_board_temps": SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_DSP_BOARD_TEMPS_ID,
    "bms_carrier_battery_relay_info": SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_BATTERY_RELAY_INFO_ID,
    "bms_carrier_afe1_status": SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE1_STATUS_ID,
    "bms_carrier_afe2_status": SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE2_STATUS_ID,
    "bms_carrier_afe3_status": SystemCanMessageId.SYSTEM_CAN_MESSAGE_BMS_CARRIER_AFE3_STATUS_ID,
    "centre_console_cc_pedal": SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_PEDAL_ID,
    "centre_console_cc_info": SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_INFO_ID,
    "centre_console_cc_steering": SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_STEERING_ID,
    "centre_console_cc_regen_percentage": SystemCanMessageId.SYSTEM_CAN_MESSAGE_CENTRE_CONSOLE_CC_REGEN_PERCENTAGE_ID
}

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
            message_type = input("Enter message type: ")
            params = input("Enter params: ")
            message = int(input("Enter Message: "))
            print(f"Message type: {message_type}, Params: {params}, Message: {message}")

            message_cache = can_data_cache.get(message_type, params)
            if message_cache is not None:
                size = message_type_sizes.get(message_type, 32)
                arbitration_id = message_type_ids.get(message_type)
                if arbitration_id is None:
                    print("Invalid message type")
                    continue
                if can_data_cache.set(message_type, params, message):
                    data = bytearray(pack(message, size))
                    new_message = Message(
                        arbitration_id=arbitration_id,
                        data=data
                    )
                    can_bus.send(new_message)
                    print("Message sent successfully")
                else:
                    print("Failed to send message")
            else:
                print("Invalid message type or params")

    except KeyboardInterrupt:
        print("Shutting down CAN simulator...")
        stop_event.set()
        time.sleep(0.5)

if __name__ == "__main__":
    import threading
    main()
    