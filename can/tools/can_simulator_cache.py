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

            "bms_carrier_battery_status": {
                # battery_status cache definitions
                "fault": 0,
                "fault_val": 0,
                "aux_batt_v": 0,
                "afe_status": 0,
            },

            "bms_carrier_battery_vt": {
                # battery_vt cache definitions
                "voltage": 0,
                "current": 0,
                "temperature": 0,
                "batt_perc": 0,
            },

            "bms_carrier_battery_info": {
                # battery_info cache definitions
                "fan1": 0,
                "fan2": 0,
                "max_cell_v": 0,
                "min_cell_v": 0,
            },

            "bms_carrier_mc_status": {
                # mc_status cache definitions
                "limit_bitset_l": 0,
                "error_bitset_l": 0,
                "limit_bitset_r": 0,
                "error_bitset_r": 0,
                "board_fault_bitset": 0,
                "overtemp_bitset": 0,
                "precharge_status": 0,
            },

            "bms_carrier_motor_controller_vc": {
                # motor_controller_vc cache definitions
                "mc_voltage_l": 0,
                "mc_current_l": 0,
                "mc_voltage_r": 0,
                "mc_current_r": 0,
            },

            "bms_carrier_motor_velocity": {
                # motor_velocity cache definitions
                "velocity_l": 0,
                "velocity_r": 0,
                "brakes_enabled": 0,
            },

            "bms_carrier_motor_sink_temps": {
                # motor_sink_temps cache definitions
                "motor_temp_l": 0,
                "heatsink_temp_l": 0,
                "motor_temp_r": 0,
                "heatsink_temp_r": 0,
            },

            "bms_carrier_dsp_board_temps": {
                # dsp_board_temps cache definitions
                "dsp_temp_l": 0,
                "dsp_temp_r": 0,
            },

            "bms_carrier_battery_relay_info": {
                # battery_relay_info cache definitions
                "state": 0,
            },

            "bms_carrier_afe1_status": {
                # afe1_status cache definitions
                "id": 0,
                "temp": 0,
                "v1": 0,
                "v2": 0,
                "v3": 0,
            },

            "bms_carrier_afe2_status": {
                # afe2_status cache definitions
                "id": 0,
                "temp": 0,
                "v1": 0,
                "v2": 0,
                "v3": 0,
            },

            "bms_carrier_afe3_status": {
                # afe3_status cache definitions
                "id": 0,
                "temp": 0,
                "v1": 0,
                "v2": 0,
                "v3": 0,
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