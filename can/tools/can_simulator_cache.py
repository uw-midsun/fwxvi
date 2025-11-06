## @file    can_simulator_cache.py
#  @date    2025-03-09
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Stores a cache class for all the CAN simulator data
#
#  @ingroup CANSimulator

class CanSimulatorDataCache:
    def __init__(self):
        self.data = {

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

            "front_controller_front_controller_pedal_data": {
                # front_controller_pedal_data cache definitions
                "percentage": 0,
                "brake_enabled": 0,
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

            "rear_controller_rear_controller_status": {
                # rear_controller_status cache definitions
                "bps_fault": 0,
                "relay_state": 0,
                "power_state": 0,
                "afe_status": 0,
            },

            "rear_controller_battery_stats_a": {
                # battery_stats_a cache definitions
                "pack_voltage": 0,
                "pack_current": 0,
                "pack_soc": 0,
            },

            "rear_controller_battery_stats_b": {
                # battery_stats_b cache definitions
                "max_cell_voltage": 0,
                "min_cell_voltage": 0,
                "max_temperature": 0,
                "motor_precharge_complete": 0,
            },

            "rear_controller_power_input_stats": {
                # power_input_stats cache definitions
                "input_dcdc_voltage": 0,
                "input_dcdc_current": 0,
                "input_aux_voltage": 0,
                "input_aux_current": 0,
            },

            "rear_controller_afe1_status_a": {
                # afe1_status_a cache definitions
                "id": 0,
                "voltage_0": 0,
                "voltage_1": 0,
                "voltage_2": 0,
            },

            "rear_controller_afe1_status_b": {
                # afe1_status_b cache definitions
                "id": 0,
                "voltage_0": 0,
                "voltage_1": 0,
                "voltage_2": 0,
            },

            "rear_controller_afe2_status_a": {
                # afe2_status_a cache definitions
                "id": 0,
                "voltage_0": 0,
                "voltage_1": 0,
                "voltage_2": 0,
            },

            "rear_controller_afe2_status_b": {
                # afe2_status_b cache definitions
                "id": 0,
                "voltage_0": 0,
                "voltage_1": 0,
                "voltage_2": 0,
            },

            "rear_controller_afe_temperature": {
                # afe_temperature cache definitions
                "id": 0,
                "temperature_0": 0,
                "temperature_1": 0,
                "temperature_2": 0,
                "temperature_3": 0,
                "temperature_4": 0,
                "temperature_5": 0,
                "temperature_6": 0,
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
            }

        }

    def get(self, message_name, parameter):
        return self.data.get(message_name, {}).get(parameter, 0)
    
    def set(self, message_name, parameter, value):
        if message_name in self.data and parameter in self.data[message_name]:
            self.data[message_name][parameter] = value
            return True
        return False