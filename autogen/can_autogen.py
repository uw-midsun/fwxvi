## @file   can_autogen.py
#  @date    2024-12-21
#  @author  Aryan Kashem
#  @brief   YAML parsing and validation module for CAN message configurations
#
#  @details This module provides functionality to parse and validate CAN message
#           configurations from YAML files, ensuring they meet specific requirements
#           for message IDs, signal lengths, and naming conventions
#
#  @ingroup autogen

import re
import yaml
from pathlib    import Path
from datetime   import datetime

def check_yaml_file(data):
    """
    @brief Validates the content of CAN message YAML data
    
    @param data Dictionary containing the YAML data to be validated
    
    @exception Exception Various exceptions for validation failures:
        - Missing required fields (id, critical, cycle)
        - Duplicate message IDs
        - Invalid message IDs (must be 0-63)
        - Illegal characters in message/signal names
        - Too many signals in a message (max 8)
        - Invalid signal lengths (must be multiple of 8)
        - Total message length exceeding 64 bits
    """
    # These are all the illegal characters
    illegal_chars_regex = re.compile('[@!#$%^&*()<>?/\|}{~:]')
    message_ids = set()

    for message_name, message in data["Messages"].items():
        # Message has id
        if "id" not in message:
            raise Exception("Message " + message_name + " has no id")

        # No same ids for messages within a board
        if message["id"] in message_ids:
            raise Exception("Duplicate id for message" + message_name)
        # All ids are between 0-64
        elif message["id"] >= 64 or message["id"] < 0:
            raise Exception("Invalid message id")
        else:
            message_ids.add(message["id"])

        # No illegal characters in message names
        if (illegal_chars_regex.search(message_name) != None):
            raise Exception("Illegal character in message name")

        # Doesn't have more than 8 signals per message
        if len(message["signals"]) > 8:
            raise Exception("More than 8 signals in a message")

        message_length = 0
        for signal_name, signal in message["signals"].items():
            # No illegal characters in signal names
            if (illegal_chars_regex.search(signal_name) != None):
                raise Exception("Illegal character in signal name")
            
            if signal.get("type") == "bitfield":
                if "flags" not in signal:
                    raise Exception("Bitfield signal must have flags defined")
                if len(signal["flags"]) == 0:
                    raise Exception("Bitfield signal must have at least one flag defined")
                if len(signal["flags"]) > signal["length"]:
                    raise Exception("The number of flags is larger than the size of the bitfield")
                for flag in signal["flags"]:
                    if isinstance(flag, dict):
                        if (illegal_chars_regex.search(flag["name"]) != None):
                            raise Exception("Illegal character in bitfield flag")
                    else:
                        if (illegal_chars_regex.search(flag) != None):
                            raise Exception("Illegal character in bitfield flag")
                
            # All signals within a message are the same length
            if signal["length"] % 8 != 0:
                raise Exception("Signal length must be a multiple of 8")
            message_length += signal['length']

        if message_length > 64:
            raise Exception("Message must be 64 bits or less")
        
        if "critical" not in message:
            raise Exception("Critical is not defined for message " + message_name)
        if not isinstance(message["critical"], bool):
            raise Exception("Critical must be a boolean value for message " + message_name)

        if "cycle" not in message:
            raise Exception("Cycle speed is not defined for message " + message_name)
        if message["cycle"] not in ["medium", "fast", "slow"]:
            raise Exception("Cycle must be one of ['medium', 'fast', 'slow'] for message " + message_name)

def get_data(args):
    """
    @brief Extracts and processes CAN message data from YAML files
    
    @param args Arguments object containing output path and template information
    @return Dictionary containing processed CAN message data with keys:
            - boards: List of board names
            - messages: List of message configurations
            - board: Current board name
    
    Expected directory structure in args.output:
    .../<target-type>/<target-name>/can
    where <target-type> is one of libraries/projects/smoke
    """
    boards = []
    messages = []
    message_count = {
        "fast_cycle" : 0,
        "medium_cycle" : 0,
        "slow_cycle" : 0,
        "total" : 0,
    }

    for yaml_path in Path("can/boards").glob("*.yaml"):
        # read yaml
        with open(yaml_path, "r") as f:
            data = yaml.load(f, Loader=yaml.FullLoader)
            check_yaml_file(data)  # check data is valid

        sender = Path(yaml_path).stem
        boards.append(sender)

        for message_name, message in data["Messages"].items():
            signals = []
            start_bit = 0
            for signal_name, signal in message["signals"].items():
                
                signal_data ={
                    "name": signal_name,
                    "start_bit": start_bit,
                    "scale": 1,
                    "offset": 0,
                    "receiver": message["target"],
                }

                if signal.get("type") == "bitfield":
                    
                    num_flags = len(signal["flags"])
                    signal_data["type"] = "bitfield"
                    signal_data["length"] = ((num_flags + 7) // 8) * 8
                    
                    index = 0
                    flags = []
                    prev_offset = 0
                    prev_length = 0
                    for flag in signal["flags"]:
                        if isinstance(flag, dict):
                            curr_length = flag["length"]
                            curr_name = flag["name"]
                        elif isinstance(flag, str):
                            curr_length = 1
                            curr_name = flag
                        else:
                            raise Exception("Invalid type for flag")
                        
                        offset = prev_offset + prev_length
                        flags.append({"name": curr_name, "length": curr_length, "offset": offset})
                        prev_length = curr_length
                        prev_offset = offset
                    
                    signal_data["flags"] = flags
                else:
                    signal_data["type"] = "standard"
                    signal_data["length"] = signal["length"]
                    signal_data["min"] = 0
                    signal_data["max"] = (2 ** signal["length"]) - 1
                        
                signals.append(signal_data)
                start_bit += signal_data["length"]

            # Determine the message Id
            # Packet structure:
            # | PRIORITY (1 bit) | MESSAGE ID (6 bits) | SOURCE (4 bits) |

            messages.append({
                "id": message["id"],
                "critical": message["critical"],
                "cycle": message["cycle"],
                "name": message_name,
                "signals": signals,
                "sender": sender,
                "receiver": message["target"],
            })

            # Update message counter
            if message["cycle"] == "fast":
                message_count["fast_cycle"] += 1
            elif message["cycle"] == "medium":
                message_count["medium_cycle"] += 1
            elif message["cycle"] == "slow":
                message_count["slow_cycle"] += 1

            message_count["total"] +=1

    project_name = Path(args.output).parent.stem

    current_date = datetime.now()
    current_date = current_date.strftime("%Y-%m-%d")
    return {"boards": boards, "messages": messages, "message_count": message_count, "project_name": project_name, "current_date": current_date}
