## @file    mpxe_test_autogen.py
#  @date    2025-07-27
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Writing soon # TODO
#
#  @details Writing soon # TODO
#
#  @ingroup autogen

import re
import yaml
from pathlib    import Path
from datetime   import datetime

# Defintions
VALID_CAN_DEFAULT_KEYS = {
    "telemetry_telemetry_data",
    "fast_one_shot_msg_sig1",
    "fast_one_shot_msg_sig2",
    "medium_one_shot_msg_sig1",
    "medium_one_shot_msg_sig2",
    "slow_one_shot_msg_sig1",
    "slow_one_shot_msg_sig2",
    "gyro_data_x_axis",
    "gyro_data_y_axis",
    "gyro_data_z_axis",
    "accel_data_x_axis",
    "accel_data_y_axis",
    "accel_data_z_axis",
    "battery_status_fault",
    "battery_status_fault_val",
    "battery_status_aux_batt_v",
    "battery_status_afe_status",
    "battery_vt_voltage",
    "battery_vt_current",
    "battery_vt_temperature",
    "battery_vt_batt_perc",
    "battery_info_fan1",
    "battery_info_fan2",
    "battery_info_max_cell_v",
    "battery_info_min_cell_v",
    "mc_status_limit_bitset_l",
    "mc_status_error_bitset_l",
    "mc_status_limit_bitset_r",
    "mc_status_error_bitset_r",
    "mc_status_board_fault_bitset",
    "mc_status_overtemp_bitset",
    "mc_status_precharge_status",
    "motor_controller_vc_mc_voltage_l",
    "motor_controller_vc_mc_current_l",
    "motor_controller_vc_mc_voltage_r",
    "motor_controller_vc_mc_current_r",
    "motor_velocity_velocity_l",
    "motor_velocity_velocity_r",
    "motor_velocity_brakes_enabled",
    "motor_sink_temps_motor_temp_l",
    "motor_sink_temps_heatsink_temp_l",
    "motor_sink_temps_motor_temp_r",
    "motor_sink_temps_heatsink_temp_r",
    "dsp_board_temps_dsp_temp_l",
    "dsp_board_temps_dsp_temp_r",
    "battery_relay_info_state",
    "afe1_status_id",
    "afe1_status_temp",
    "afe1_status_v1",
    "afe1_status_v2",
    "afe1_status_v3",
    "afe2_status_id",
    "afe2_status_temp",
    "afe2_status_v1",
    "afe2_status_v2",
    "afe2_status_v3",
    "afe3_status_id",
    "afe3_status_temp",
    "afe3_status_v1",
    "afe3_status_v2",
    "afe3_status_v3",
    "cc_pedal_throttle_output",
    "cc_pedal_brake_output",
    "cc_info_target_velocity",
    "cc_info_drive_state",
    "cc_info_cruise_control",
    "cc_info_regen_braking",
    "cc_info_hazard_enabled",
    "cc_steering_input_cc",
    "cc_steering_input_lights",
    "cc_regen_percentage_percent",
}
VALID_CAN_ACTIONS = {"set"}

VALID_COMMANDS = {
    "GPIO": {
        "set_pin_state",
        "set_all_states",
        "get_pin_state",
        "get_all_states",
        "get_pin_mode",
        "get_all_modes",
        "get_pin_alt_function",
        "get_all_alt_functions",
    },
    "AFE": {
        "set_cell",
        "set_aux",
        "set_dev_cell",
        "set_dev_aux",
        "set_pack_cell",
        "set_pack_aux",
        "set_discharge",
        "get_cell",
        "get_aux",
        "get_dev_cell",
        "get_dev_aux",
        "get_pack_cell",
        "get_pack_aux",
        "get_discharge",
    },
    "CAN": VALID_CAN_ACTIONS,
    "LOG": set(),  
}

def check_yaml_file(data): 
    """
    @brief Validates the content of MPXE autogen YAML data

    @param data dict
            A dictionary loaded from the YAML file. Must contain "can_defaults" and "tasks"
    
    @exception Exception Various exceptions for validation failures:
               - Unknown keys or invalid values in can_defaults.
               - Missing or malformed tasks (not a list, invalid name, duplicate names).
               - Invalid period_ms (not a positive integer).
               - Commands list missing, empty, or containing malformed/unknown commands.
    """
    # ---------- can_defaults CHECK -------------
    default_setup = data.get("can_defaults", {})
    for command, value in default_setup.items(): 
        # Ensure only valid commands listed 
        if command not in VALID_CAN_DEFAULT_KEYS:
            raise Exception(f"Invalid command in command_defaults: {command}")
        
    
    # ---------- tasks CHECK -------------
    # These are all the illegal characters
    illegal_chars_regex = re.compile('[@!#$%^&*()<>?/\|}{~:]')
    seen_names = set()
    tasks = data.get("tasks", [])

    for i, task in enumerate(tasks, start=1):
        # ----- TASK NAME -----
        name = task.get("name")

        # Check for empty string and ensuring no invalid chars in task name
        if not name or illegal_chars_regex.search(name):
            raise Exception(f"Task has an invalid/missing name: {name}")
        # Duplicate names check
        if name in seen_names:
            raise Exception(f"Duplicate name: {name}")
        seen_names.add(name)

        # ----- PERIOD -----
        period = task.get("period_ms")
        # Period has to be an int or less than = to 0
        if not isinstance(period, int) or period <= 0:
            raise Exception(f"Invalid period: {period}")
        
        # ----- COMMANDS -----
        commands = task.get("commands")
        if not commands:
            raise Exception(f"Empty commands list: {commands}")
        
        for cmd in commands:
            tokens = cmd.strip().split()
            if len(tokens) < 2:
                raise Exception(f"Malformed command {cmd} in task {name}")

            cmd_type = tokens[0].upper()
            action   = tokens[1].lower()

            if cmd_type not in VALID_COMMANDS:
                raise Exception(f"Unknown command type: {cmd_type} in {cmd}")

            # Check if non-empty string is provided in LOG command
            if cmd_type == "LOG":
                msg = " ".join(tokens[1:])
                if not msg.strip():
                    raise Exception(f"LOG in task {name} needs a message")
                continue
            
            if cmd_type == "CAN":
                # Expect: CAN SET <signal> <value>
                if action not in VALID_CAN_ACTIONS:  # action is already lowercased
                    raise Exception(
                        f"Invalid CAN verb {tokens[1]} (expected one of {sorted(VALID_CAN_ACTIONS)})"
                    )
                if len(tokens) < 4:
                    raise Exception(f"Malformed CAN command (need 4 tokens): {cmd}")
                signal = tokens[2].lower()
                value  = tokens[3]
                if signal not in VALID_CAN_DEFAULT_KEYS:
                    raise Exception(f"Unknown CAN signal '{signal}' in {cmd}")
                continue
            # Checks if valid action (ex: set_all_states) is provided
            allowed = VALID_COMMANDS[cmd_type]
            if action.lower() not in allowed:
                raise Exception(f"Invalid action {action} for cmd_type {cmd_type}")
            
def get_data(args):
    """
    @brief Extracts and processes MPXE test YAMLs 
    @param args Arguments object (only uses args.output for project_name)
    @return Dictionary where:
        - can_defaults: Default can commands that will run at the beginning of main source file
        - tasks: The tasks to execute (Contains name, period, commands, and command_types used)
        - project_name: Name of project
        - current_date: Current date
    """
    base = Path("mpxe/test_scripts")
    yamls = sorted(base.glob("*.yaml"))
    if not yamls:
        raise Exception(f"No .yaml found in {base}")

    configs = []
    for path in yamls:
        data = yaml.safe_load(path.read_text())
        check_yaml_file(data)

        default_setup = data.get("can_defaults", {})
        can_defaults = {}
        for key, val in default_setup.items():
            can_defaults[key] = val
            
        tasks = []
        for t in data["tasks"]:
            cmds = [c.strip() for c in t["commands"] if c and c.strip()]
            used_types = []
            for c in cmds:
                ct = c.split()[0].upper()            # e.g., "GPIO", "AFE", "CAN", "LOG"
                if ct in ("LOG", "CAN"):             # strip LOG and CAN
                    continue
                ct_cap = ct.capitalize()             # "GPIO" -> "Gpio"
                if ct_cap not in used_types:
                    used_types.append(ct_cap)
            tasks.append({
                "name": t["name"],
                "period_ms": t["period_ms"],
                "commands": cmds,
                "command_types_used": used_types,
            })


    project_name = Path(args.output).parent.stem
    current_date = datetime.now().strftime("%Y-%m-%d")
    app_term_path = Path("mpxe/server/app/src/app_terminal.cc")
    text = app_term_path.read_text()

    # Will get handle_Commands from app_terminal.cc 
    handler_impls = {}
    for cmd_type in VALID_COMMANDS.keys():
        # ex: cmd_type is GPIO then fn_name is handleGpioCommands
        fn_name = f"handle{cmd_type.capitalize()}Commands"
        # match the entire out-of-line definition
        regex = (
            rf'(void\sTerminal::{fn_name}\s*\([^)]*\)\s*\{{'
            r'(?:[^{}]*\{[^{}]*\})*'     
            r'[^{}]*\})'
        )
        m = re.search(regex, text, re.DOTALL)
        if not m:
            continue
        impl = m.group(1)

        # strip the sendMessage/invalid-command/m_targetClient block
        impl = re.sub(
            r'\s*if\s*\(!message\.empty\(\)\)\s*\{[^}]*?\}\s*else\s*\{[^}]*?\}\s*m_targetClient\s*=\s*nullptr\s*;\s*',
            '',
            impl,
            flags=re.DOTALL
        )

        # change signature: void Terminal::handleGpioCommands(...) to std::string handleGpioCommands(...)
        impl = re.sub(
            rf'void\sTerminal::{fn_name}',
            f'std::string {fn_name}',
            impl
        )

        # ensure a `return message;` before the final brace
        if 'return message;' not in impl:
            impl = impl.rstrip('}') + '}\n  return message;\n}'

        handler_impls[fn_name] = impl
    
    project_name = Path(args.output).parent.stem
    current_date = datetime.now().strftime("%Y-%m-%d")
    
    return {
        "can_defaults":   can_defaults,
        "tasks":          tasks,
        "project_name":   project_name,
        "current_date":   current_date,
        "handler_impls":  handler_impls, 
    }