import json
import subprocess
import serial  # pip install pyserial
import glob
import os
import time
from sys import platform

# OpenOCD configuration constants
OPENOCD = 'openocd'
OPENOCD_SCRIPT_DIR = '/usr/local/share/openocd/scripts/'
PROBE = 'cmsis-dap'
PLATFORM_DIR = 'platform'


def get_device_params_mode(flash_type):
    """Get the device params directory based on flash type."""
    return 'default' if flash_type != 'legacy' else 'legacy'


def get_hardware_paths(hardware, flash_type):
    """Get paths for hardware-specific configuration files."""
    device_params_mode = get_device_params_mode(flash_type)
    return {
        'device_params': os.path.join(PLATFORM_DIR, 'hardware', hardware, device_params_mode, 'device_params.cfg'),
        'flash_procs': os.path.join(PLATFORM_DIR, 'hardware', 'templates', 'stm32_flash_procs.tcl'),
    }

def parse_config(entry):
    # Default config to empty for fields that don't exist
    ret = {
        'hardware': "STM32L433CCU6",
        'libs': [],
        'x86_libs': [],
        'arm_libs': [],
        'cflags': [],
        'mocks': {},
        "include_path" : [],
        'no_lint': False,
        "mpxe": False,
        "can": False,
        "arm_only": False
    }
    config_file = entry.File('config.json')
    if not config_file.exists():
        return ret
    with open(config_file.abspath, 'r') as f:
        config = json.load(f)
        for key, value in config.items():
            ret[key] = value
    return ret


def flash_run(entry, hardware, flash_type):
    '''Flash and run file, return a pyserial object which monitors the device serial output'''
    serialData = None

    try:
        output = subprocess.check_output(["ls", "/dev/serial/by-id/"])
        device_path = f"/dev/serial/by-id/{str(output, 'ASCII').strip()}"
        serialData = serial.Serial(device_path, 115200)
    except:
        print()
        print("Flashing requires a controller board to be connected, use --platform=x86 for x86 targets")

    paths = get_hardware_paths(hardware, flash_type)

    # Mapping of flash_type to the Tcl procedure name
    flash_proc_map = {
        'legacy': 'stm_flash_app_active',
        'bootstrap': 'stm_flash_bootstrap',
        'bootloader': 'stm_flash_bootloader',
        'application': 'stm_flash_app_active',
        'app_staging': 'stm_flash_app_staging',
        'fs_storage': 'stm_flash_fs_storage',
    }

    tcl_flash_proc = flash_proc_map.get(flash_type)
    if not tcl_flash_proc:
        raise ValueError(f"Invalid or unsupported flash_type: '{flash_type}' for flashing.")

    tcl_commands_string = (
        f'source "{paths["device_params"]}"; '
        f'source "{paths["flash_procs"]}"; '
        f'{tcl_flash_proc} "{entry}"; '
        'shutdown'
    )

    openocd_cmd = [
        "sudo", OPENOCD,
        "-s", OPENOCD_SCRIPT_DIR,
        "-f", f"interface/{PROBE}.cfg",
        "-f", "target/stm32l4x.cfg",
        "-c", f"stm32l4x.cpu configure -rtos FreeRTOS; {tcl_commands_string}"
    ]

    print(f"Executing flash command: {' '.join(openocd_cmd)}")
    try:
        subprocess.run(openocd_cmd, check=True)
        print("Flash complete")
    except subprocess.CalledProcessError as e:
        print(f"Flash failed with error code {e.returncode}")
        print(f"Command: {e.cmd}")
        print(f"Stderr: {e.stderr.decode() if e.stderr else 'N/A'}")
        raise
    except FileNotFoundError:
        print("Error: 'openocd' command not found. Ensure OpenOCD is installed and in your PATH.")
        raise
    except Exception as e:
        print(f"An unexpected error occurred during flashing: {e}")
        raise

    return serialData


def gdb_run(elf_path, hardware, flash_type):
    """Start OpenOCD and GDB for ARM debugging.
    
    Args:
        elf_path: Path to the ELF file to debug
        hardware: Hardware type (e.g., 'STM32L433CCU6')
        flash_type: Flash type (e.g., 'legacy')
    """
    paths = get_hardware_paths(hardware, flash_type)
    
    openocd_cmd = [
        "sudo", OPENOCD,
        "-s", OPENOCD_SCRIPT_DIR,
        "-f", f"interface/{PROBE}.cfg",
        "-f", "target/stm32l4x.cfg",
        "-f", paths['device_params'],
        "-f", paths['flash_procs'],
    ]
    
    print(f"Starting OpenOCD: {' '.join(openocd_cmd)}")
    openocd_proc = subprocess.Popen(openocd_cmd)
    
    try:
        # Give OpenOCD time to start
        time.sleep(2)
        
        gdb_cmd = [
            "arm-none-eabi-gdb",
            "-tui",
            str(elf_path),
            "-ex", "target extended-remote localhost:3333",
            "-ex", "monitor reset halt",
        ]
        
        print(f"Starting GDB: {' '.join(gdb_cmd)}")
        subprocess.run(gdb_cmd)
    finally:
        # Clean up OpenOCD when GDB exits
        print("Terminating OpenOCD...")
        subprocess.run(["sudo", "kill", str(openocd_proc.pid)])