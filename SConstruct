from SCons.Script import *
from scons.common import flash_run
import subprocess
import scons_compiledb
import os
import json

def set_target(option, opt, value, parser):
    if opt == '--project':
        target = f'projects/{value}'
    if opt == '--smoke':
        target = f'smoke/{value}'
    if opt == '--library':
        target = f'libraries/{value}'
    if opt == '--python' or opt == '--py':
        target = f'py/{value}'
    setattr(parser.values, option.dest, target)

###########################################################
# Build arguments
###########################################################
AddOption(
    '--platform',
    dest='platform',
    type='choice',
    choices=("arm", "x86"),
    default='arm',
    help="Specifies target platform. One of 'arm' or 'x86'. Defaults to 'arm' if not provided."
)

AddOption(
    '--library', '--project', '--smoke', '--python', '--py',
    type="string", dest="name",
    action='callback', callback=set_target,
    help="Specify the target.   e.g. --library=ms-common, --project=leds, --smoke=adc, --python=example",
)

AddOption(
    '--test',
    dest='testfile',
    type='string',
    action='store',
    help="Additionally specify the name of test to run for 'test' command."
)

AddOption(
    '--define',
    dest='define',
    type='string',
    action='store',
    help="Add CPP defines to a build.   e.g. --define='LOG_LEVEL=LOG_LEVEL_WARN'"
)

AddOption(
    '--task',
    dest='task',
    type='string',
    action='store'
)

AddOption(
    '--mem-report',
    dest='mem-report',
    action='store_true',
    help="(arm) Reports the memory space after a build."
)

AddOption(
    '--sanitizer',
    dest='sanitizer',
    type='choice',
    choices=("asan", "tsan"),
    default="none",
    help="(x86) Specifies the sanitizer. One of 'asan' for Address sanitizer or 'tsan' for Thread sanitizer. Defaults to none."
)

NUM_JOBS = os.cpu_count() or 4

if not GetOption('num_jobs'):
    AddOption('--jobs', dest='num_jobs', type='int', default=NUM_JOBS)

PLATFORM        = GetOption('platform')
TARGET          = GetOption('name')
TESTFILE        = GetOption('testfile')

###########################################################
# Load build preset
###########################################################

HARDWARE_TYPE   = "STM32L433CCU6"
FLASH_TYPE      = "legacy"
BUILD_CONFIG    = "debug"

build_presets_file = File('build_presets.json')

if build_presets_file.exists():
    with open(build_presets_file.abspath, 'r') as f:
        build_preset = json.load(f)
        selected_build_preset = build_preset.get("selected_preset")

    if selected_build_preset:
        # Retrieve the settings for the selected preset
        preset_settings = build_preset["presets"].get(selected_build_preset)

        if preset_settings:
            print(f"Loaded selected preset: '{selected_build_preset}'")
            print(f"    Hardware: {preset_settings['hardware']}")
            print(f"    Flash: {preset_settings['flash']}")
            print(f"    Build Config: {preset_settings['build_config']}")

            HARDWARE_TYPE = preset_settings['hardware']
            FLASH_TYPE = preset_settings['flash']
            BUILD_CONFIG = preset_settings['build_config']
        else:
            print(f"Error: Selected preset '{selected_build_preset}' not found in presets.")
    else:
        print("No 'selected_build_preset' field found in the configuration.")
else:
    print("build_presets.json does not exist. Please check update your branch or restore it!")

###########################################################
# Environment setup
###########################################################

COMMAND = COMMAND_LINE_TARGETS[0] if COMMAND_LINE_TARGETS else ""

# Force x86 if the command is for MPXE
if COMMAND == "mpxe":
    PLATFORM = "x86"

# Retrieve the construction environment from the appropriate platform script
env = SConscript(f'platform/{PLATFORM}.py', exports=['HARDWARE_TYPE', 'FLASH_TYPE', 'BUILD_CONFIG'])

database_name = f"compile_commands.json"
config = scons_compiledb.Config(db=database_name)
scons_compiledb.enable_with_cmdline(env, config)

VARS = {
    "PLATFORM": PLATFORM,
    "TARGET": TARGET,
    "HARDWARE_TYPE": HARDWARE_TYPE,
    "FLASH_TYPE": FLASH_TYPE,
    "BUILD_CONFIG": BUILD_CONFIG,
    "TESTFILE": TESTFILE,
    "env": env,
}

# Parse asan / tsan and Adding Sanitizer Argument to Environment Flags
# Note platform needs to be explicitly set to x86

SANITIZER = GetOption('sanitizer')

if SANITIZER == 'asan':
    env['CCFLAGS'] += ["-fsanitize=address"]
    env['CXXFLAGS'] += ["-fsanitize=address"]
    env['LINKFLAGS'] += ["-fsanitize=address"]
elif SANITIZER == 'tsan':
    env['CCFLAGS'] += ["-fsanitize=thread"]
    env['CXXFLAGS'] += ["-fsanitize=thread"]
    env['LINKFLAGS'] += ["-fsanitize=thread"]

env['CXXCOMSTR'] = "Compiling  $TARGET"
env['CCCOMSTR'] = "Compiling  $TARGET"
env['ARCOMSTR'] = "Archiving  $TARGET"
env['ASCOMSTR'] = "Assembling $TARGET"
env['LINKCOMSTR'] = "Linking    $TARGET"
env['RANLIBCOMSTR'] = "Indexing   $TARGET"

SetOption('implicit_cache', True)
SetOption('max_drift', 1)

env.Append(CPPDEFINES=[GetOption('define')])

###########################################################
# Directory setup
###########################################################

BUILD_DIR = Dir('#/build').Dir(PLATFORM)
BIN_DIR = BUILD_DIR.Dir('bin')
OBJ_DIR = BUILD_DIR.Dir('obj')

VariantDir(OBJ_DIR, '.', duplicate=0)

###########################################################
# Testing
###########################################################
if COMMAND == "test":
    # Add flags when compiling a test
    TEST_CFLAGS = ['-DMS_TEST']
    env['CCFLAGS'] += TEST_CFLAGS
    SConscript('scons/test.scons', exports='VARS')
    SConscript('scons/build.scons', exports='VARS')

###########################################################
# Helper targets
###########################################################
elif COMMAND == "new":
    SConscript('scons/new_target.scons', exports='VARS')

###########################################################
# HIL command
###########################################################
elif COMMAND == "hil":
    if not TEST_FILE:
        pass
    SConscript('scons/pytest.scons', exports='VARS')

###########################################################
# Clean
###########################################################
elif COMMAND == "clean":
    AlwaysBuild(Command('#/clean', [], 'rm -rf build/*'))

###########################################################
# Linting and Formatting
###########################################################
elif COMMAND == "lint" or COMMAND == "format":
    SConscript('scons/lint_format.scons', exports='VARS')

###########################################################
# Doxygen Generation
###########################################################
elif COMMAND == "doxygen":
    AlwaysBuild(Command('#/doxygen', [], 'doxygen doxygen/Doxyfile'))

###########################################################
# Cantools Autogeneration Script
###########################################################
elif COMMAND == "cantools":
    AlwaysBuild(Command('#/cantools', [], 'python3 -m autogen cantools -o can/tools'))

###########################################################
# Run MPXE Server
###########################################################
elif COMMAND == "mpxe":
    SConscript('scons/build.scons', exports='VARS')
    mpxe_elf = BIN_DIR.Dir("projects").File("mpxe_server")

    def mpxe_run(target, source, env):
        print('Running MPXE Server', mpxe_elf)
        subprocess.run(mpxe_elf.path)

    AlwaysBuild(Command('#/mpxe', mpxe_elf, mpxe_run))

###########################################################
# RUN MPXE GUI 
###########################################################
elif COMMAND == "mpxe-gui":
    # Ensure we’re on x86 for the desktop GUI
    PLATFORM = "x86"
    VARS["PLATFORM"] = PLATFORM

    # Build everything (including the GUI) using build.scons
    SConscript('scons/build.scons', exports='VARS')

    GUI_ELF = Dir('#/build').Dir(PLATFORM).Dir('bin').Dir('projects').File('mpxe_gui')

    def gui_run(target, source, env):
        print('Running MPXE GUI', GUI_ELF)
        subprocess.run(GUI_ELF.path)

    AlwaysBuild(Command('#/mpxe-gui', GUI_ELF, gui_run))
    Default('#/mpxe-gui')
    
###########################################################
# Build
###########################################################
else:  # command not recognised, default to build
     SConscript('scons/build.scons', exports='VARS')

###########################################################
# Helper targets for x86
###########################################################
if PLATFORM == 'x86' and TARGET:
    project_elf = BIN_DIR.File(TARGET)
    # os.exec the x86 project ELF file to simulate it

    def sim_run(target, source, env):
        print('Simulating', project_elf)
        subprocess.run([project_elf.path, TARGET.split("/")[-1]])

    AlwaysBuild(Command('#/sim', project_elf, sim_run))

    # open gdb with the elf file
    def gdb_run(target, source, env):
        subprocess.run(['/usr/bin/gdb', project_elf.path])

    AlwaysBuild(Command('#/gdb', project_elf, gdb_run))

###########################################################
# Helper targets for arm
###########################################################
if PLATFORM == 'arm' and TARGET:
    project_bin = BIN_DIR.File(TARGET + '.bin')
    # display memory info for the project
    if GetOption('mem-report'):
        AlwaysBuild(Command("#/mem-report", project_bin,
                            f"python3 scons/mem_report.py build/arm/bin/{TARGET}"))
        Default("#/mem-report")

    # flash the MCU using openocd
    def flash_run_target(target, source, env):
        serialData = flash_run(project_bin, HARDWARE_TYPE, FLASH_TYPE)
        exit(0)

    AlwaysBuild(Command('#/flash', project_bin, flash_run_target))
