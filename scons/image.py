"""`scons image --project=<name>` orchestration.

Builds bootstrap, the matching bootloader (can_bootloader, or fota for telemetry) and the
target application, flashes all three in order, then stamps the CONFIG page with a BootConfig
record so the freshly flashed bootloader (see libraries/ms-bootloader) recognizes the app as
valid instead of looping bl_dfu_check_app() forever on a stale/blank record.

The node id is never typed in by hand: it is read from can/inc/system_can.h's SystemCanDevice
enum, the same id the running application already uses for its own CAN identity (can_hw.c), so
the bootloader build this triggers can never drift from the app it is meant to serve.
"""

import json
import os
import re
import struct
import subprocess
import sys
import zlib

from scons.common import flash_run

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# These are bootloader stages, not applications -- there is nothing to derive a node id for.
BOOTLOADER_STAGE_PROJECTS = {'bootstrap', 'can_bootloader', 'fota'}

# Project name -> SystemCanDevice name, only where they differ (fota is telemetry's gateway
# bootloader, not a board of its own).
DEVICE_NAME_ALIASES = {
    'fota': 'telemetry',
}

BOOTCONFIG_MAGIC = 0x424F4F54
BOOTCONFIG_SCHEMA = 1

# magic, schema_version, size, board_id, board_name[32], project_name[32], git_hash[16],
# fw_major, fw_minor, bl_size, bl_crc32, app_size, app_crc32, app_present -- packed, little
# endian, matches BootConfig in libraries/ms-bootloader/common/inc/bootloader_config.h exactly
# (calcsize is 109, the recorded config_crc32 trails it for a total of 113 bytes).
BODY_FMT = '<IHHH32s32s16sBBIIIIB'
BODY_LEN = struct.calcsize(BODY_FMT)


def _load_json(path):
    with open(path, 'r') as f:
        return json.load(f)


def _project_preset(project_name):
    """(hardware, flash_type) for a project, from its own config.json + build_presets.json."""
    config = _load_json(os.path.join(ROOT, 'projects', project_name, 'config.json'))
    preset_name = config['selected_preset']
    presets = _load_json(os.path.join(ROOT, 'build_presets.json'))['presets']
    preset = presets[preset_name]
    return preset['hardware'], preset['flash']


def _derive_node_id(project_name):
    """Look up the project's SystemCanDevice id, regenerating system_can.h first so it is fresh."""
    subprocess.run(['python3', '-m', 'autogen', 'system_can', '-o', 'can/inc'], cwd=ROOT, check=True)

    device_name = DEVICE_NAME_ALIASES.get(project_name, project_name).upper()
    with open(os.path.join(ROOT, 'can', 'inc', 'system_can.h'), 'r') as f:
        content = f.read()

    match = re.search(rf'SYSTEM_CAN_DEVICE_{device_name}\s*=\s*(\d+)', content)
    if not match:
        sys.exit(
            f"scons image: no SYSTEM_CAN_DEVICE_{device_name} in can/inc/system_can.h for "
            f"project '{project_name}' -- add a can/boards/<name>.yaml for it first")
    return int(match.group(1))


def _build_project(project_name, node_id):
    env = os.environ.copy()
    env['MS_BL_NODE_ID'] = str(node_id)
    print(f"--- scons image: building {project_name} (node id {node_id}) ---")
    subprocess.run(['scons', f'--project={project_name}'], cwd=ROOT, check=True, env=env)


def _bin_path(project_name):
    return os.path.join(ROOT, 'build', 'arm', 'bin', 'projects', f'{project_name}.bin')


def _size_and_crc32(path):
    with open(path, 'rb') as f:
        data = f.read()
    return len(data), zlib.crc32(data) & 0xFFFFFFFF


def _git_hash():
    try:
        return subprocess.check_output(['git', 'rev-parse', '--short=16', 'HEAD'], cwd=ROOT).decode().strip()
    except subprocess.CalledProcessError:
        return ''


def _build_boot_config(node_id, project_name, bootloader_bin, app_bin):
    """Pack a BootConfig record matching bootloader_config.c's bl_config_write() layout."""
    board_name = project_name.replace('_', ' ').title()
    bl_size, bl_crc32 = _size_and_crc32(bootloader_bin)
    app_size, app_crc32 = _size_and_crc32(app_bin)

    body = struct.pack(
        BODY_FMT,
        BOOTCONFIG_MAGIC,
        BOOTCONFIG_SCHEMA,
        BODY_LEN + 4,  # size: body plus the trailing config_crc32
        node_id,
        board_name.encode('ascii'),
        project_name.encode('ascii'),
        _git_hash().encode('ascii'),
        0, 0,  # fw_version_major/minor: not tracked yet
        bl_size, bl_crc32,
        app_size, app_crc32,
        1,  # app_present
    )
    config_crc32 = zlib.crc32(body) & 0xFFFFFFFF
    record = body + struct.pack('<I', config_crc32)

    # Flash programs in double words, pad like bl_config_write() does
    return record + b'\xFF' * ((-len(record)) % 8)


def image_run(target):
    """Entry point for the `scons image --project=<name>` command."""
    project_name = target.split('/')[-1]
    if project_name in BOOTLOADER_STAGE_PROJECTS:
        sys.exit(f"scons image: '{project_name}' is a bootloader stage, image an application project instead")

    node_id = _derive_node_id(project_name)
    bootloader_project = 'fota' if project_name == 'telemetry' else 'can_bootloader'

    for name in ('bootstrap', bootloader_project, project_name):
        _build_project(name, node_id)

    bootstrap_hw, bootstrap_flash = _project_preset('bootstrap')
    flash_run(_bin_path('bootstrap'), bootstrap_hw, bootstrap_flash)

    bl_hw, bl_flash = _project_preset(bootloader_project)
    flash_run(_bin_path(bootloader_project), bl_hw, bl_flash)

    app_hw, app_flash = _project_preset(project_name)
    flash_run(_bin_path(project_name), app_hw, app_flash)

    config_blob = _build_boot_config(node_id, project_name, _bin_path(bootloader_project), _bin_path(project_name))
    config_path = os.path.join(ROOT, 'build', 'arm', 'bin', 'projects', 'boot_config.bin')
    with open(config_path, 'wb') as f:
        f.write(config_blob)
    flash_run(config_path, app_hw, 'config')

    print(f"--- scons image: {project_name} imaged with node id {node_id} ---")
