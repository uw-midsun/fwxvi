## @file    can_simulator_cache.py
#  @date    2025-03-09
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Stores a cache class for all the CAN simulator data
#
#  @ingroup CANSimulator

class CanSimulatorDataCache:
    def __init__(self):
        self.data = {

            "rear_controller_test_data": {
                # test_data cache definitions
                "fault": 0,
                "fault_val": 0,
                "aux_batt_v": 0,
                "afe_status": 0,
            },

            "imu_gyro_data": {
                # gyro_data cache definitions
                "x_axis": 0,
                "y_axis": 0,
                "z_axis": 0,
            },

            "imu_accel_data": {
                # accel_data cache definitions
                "x_axis": 0,
                "y_axis": 0,
                "z_axis": 0,
            },

            "can_communication_fast_one_shot_msg": {
                # fast_one_shot_msg cache definitions
                "sig1": 0,
                "sig2": 0,
            },

            "can_communication_medium_one_shot_msg": {
                # medium_one_shot_msg cache definitions
                "sig1": 0,
                "sig2": 0,
            },

            "can_communication_slow_one_shot_msg": {
                # slow_one_shot_msg cache definitions
                "sig1": 0,
                "sig2": 0,
            },

            "steering_steering_state": {
                # steering_state cache definitions
                "target_velocity": 0,
                "drive_state": 0,
                "cruise_control": 0,
                "regen_braking": 0,
                "hazard_enabled": 0,
            },

            "telemetry_telemetry": {
                # telemetry cache definitions
                "telemetry_data": 0,
            },

            "front_controller_front_controller_pedal_data": {
                # front_controller_pedal_data cache definitions
                "percentage": 0,
                "brake_enabled": 0,
            }

        }

    def get(self, message_name, parameter):
        return self.data.get(message_name, {}).get(parameter, 0)
    
    def set(self, message_name, parameter, value):
        if message_name in self.data and parameter in self.data[message_name]:
            self.data[message_name][parameter] = value
            return True
        return False