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
    "set_pack_voltage",
    "set_pack_aux",
    "set_discharge"
}

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
    "LOG": None,  
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
        
        # Value checks for can_defaults commands
        if command == "set_discharge":
            if str(value).lower() not in ("on", "off"):
                raise Exception(f"Invalid discharge state in command_defaults: {value}")
        else: 
            if not isinstance(value, (int, float)) or value < 0: 
                raise Exception(f"Invalid voltage, ensure it is above 0 mV or a number: {value}")
    
    # ---------- tasks CHECK -------------
    # These are all the illegal characters
    illegal_chars_regex = re.compile('[@!#$%^&*()<>?/\|}{~:]')
    seen_names = set()
    tasks = data.get("tasks", [])

    for i, task in enumerate(tasks, start=1):
        # ----- TASK NAME -----
        name = tasks.get("name")

        # Check for empty string and ensuring no invalid chars in task name
        if not name or illegal_chars_regex.search(name):
            raise Exception(f"Task has an invalid/missing name: {name}")
        # Duplicate names check
        if name in seen_names:
            raise Exception(f"Duplicate name: {name}")
        seen_names.add(name)

        # ----- PERIOD -----
        period = tasks.get("period_ms")
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

            cmd_type, action = tokens[0], tokens[1]

            if cmd_type not in VALID_COMMANDS:
                raise Exception(f"Unknown command type: {cmd_type} in {cmd}")

            # Check if non-empty string is provided in LOG command
            if cmd_type == "LOG":
                msg = " ".join(tokens[1:])
                if not msg.strip():
                    raise Exception(f"LOG in task {name} needs a message")
                continue
            
            # Checks if valid action (ex: set_all_states) is provided
            allowed = VALID_COMMANDS[cmd_type]
            if action.lower() not in allowed:
                raise Exception(f"Invalid action {action} for cmd_type {cmd_type}")
            
def get_data(args):
    """
    TODO
    """
    # locate and load YAML
    yaml_path = Path(args.input) / "config.yaml"
    if not yaml_path.exists():
        raise Exception(f"YAML file not found at {yaml_path}")
    with open(yaml_path, "r") as f:
        data = yaml.safe_load(f)

    # validate structure
    check_yaml_file(data)

    # extract defaults
    can_defaults = {
        "set_pack_voltage": data["can_defaults"]["set_pack_voltage"],
        "set_pack_aux":     data["can_defaults"]["set_pack_aux"],
        "set_discharge":    data["can_defaults"]["set_discharge"].upper()
    }

    # extract tasks
    tasks = []
    for task in data.get("tasks", []):
        tasks.append({
            "name":      task["name"],
            "period_ms": task["period_ms"],
            "commands":  [cmd.strip() for cmd in task["commands"]]
        })

    # timestamp
    current_date = datetime.now().strftime("%Y-%m-%d")

    project_name = Path(args.output).parent.stem

    return {
        "can_defaults": can_defaults,
        "tasks":        tasks,
        "project_name": project_name,
        "current_date": current_date
    }