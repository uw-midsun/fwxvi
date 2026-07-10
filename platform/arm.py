import os
import shutil
import subprocess

Import ('HARDWARE_TYPE')
Import ('FLASH_TYPE')
Import ('BUILD_CONFIG')

PLATFORM_DIR = os.getcwd()

compiler = 'arm-none-eabi-gcc'
ranlib = 'arm-none-eabi-gcc-ranlib'
objcopy = 'arm-none-eabi-objcopy'
ar = 'arm-none-eabi-gcc-ar'
ccache_path = 'ccache'

if shutil.which(ccache_path):
    compiler = ccache_path + ' ' + compiler

arch_cflags = [
    '-mlittle-endian',
    '-mcpu=cortex-m4',
    '-mfloat-abi=hard',
    '-mfpu=fpv4-sp-d16',
    '-mthumb'
]

common_defines = [
    'MS_PLATFORM_ARM',
    'HSE_VALUE=16000000',
    'LSE_VALUE=32768',
]

debug_defines = [
    "MS_DEBUG_LOG=1",
]

release_defines = [
    "MS_DEBUG_LOG=0",
],

hardware_defines = {
    'STM32L433CCU6': [
        'STM32L433xx',
        # TODO: Add more STM32L433CCU6 specific defines here
    ],
    'STM32L4P5VET6': [
        'STM32L4P5xx',
        # TODO: Add more STM32L4P5VET6 specific defines here
    ],
    'STM32L496RGT6': [
        'STM32L496xx',
        # TODO: Add more STM32L496RGT6 specific defines here
    ]
    # TODO: Add more hardware defines
}

debug_flags = [
    '-g3',
    '-ggdb3',
    '-O0',
    '-fno-inline',
    '-fstack-usage',
    '-DDEBUG'
]

release_flags = [
    '-Os',
    '-fomit-frame-pointer',
    '-mfloat-abi=hard',
    '-mfpu=fpv4-sp-d16',
    '-DNDEBUG'
]

common_flags = [
    '-ffreestanding',
    '-Wall',
    '-Wextra',
    '-Werror',
    '-Wno-discarded-qualifiers',
    '-Wno-unused-variable',
    '-Wno-unused-parameter',
    '-Wpointer-arith',
    '-Wundef',
    '-Wdouble-promotion',
    '-Wno-enum-conversion',
    '-ffunction-sections',
    '-fdata-sections',
    '-fsingle-precision-constant',
    '-fno-math-errno',
    '-Wl,--gc-sections',
    '-Wl,-Map=build/out.map',
    '--specs=nosys.specs',
    '--specs=nano.specs',
    '-mcpu=cortex-m4'
]

def board_config_path(hardware):
    """Path to the declarative board.toml that drives every generated artifact."""
    return os.path.join(PLATFORM_DIR, 'hardware', hardware, 'board.toml')


def run_board_generator(hardware, emitter, out_path):
    """Emit one artifact from the chip's board.toml via generate.py."""
    repo_root = os.path.dirname(PLATFORM_DIR)
    generator = os.path.join(repo_root, 'libraries', 'ms-bootloader', 'tools', 'generate.py')
    board = board_config_path(hardware)

    if not os.path.exists(board):
        raise FileNotFoundError(f"board.toml not found for {hardware}: {board}")

    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    subprocess.run(['python3', generator, emitter, board, '-o', out_path, '-q'], check=True)
    return out_path


def generate_default_memory_map(hardware):
    out_path = os.path.join(PLATFORM_DIR, 'hardware', hardware, 'default', 'memory_map.ld')
    return run_board_generator(hardware, 'linker', out_path)


def generate_board_headers(hardware):
    """Regenerate the C headers driven by board.toml (geometry knobs and app side CAN ids)."""
    repo_root = os.path.dirname(PLATFORM_DIR)
    run_board_generator(
        hardware, 'user-config',
        os.path.join(PLATFORM_DIR, 'hardware', hardware, 'bootloader_user_config.h'))
    run_board_generator(
        hardware, 'can-entry',
        os.path.join(repo_root, 'can', 'inc', 'can_bl_entry.h'))


def get_link_flags(hardware, flash_type='legacy'):
    supported_hardware = ['STM32L433CCU6', 'STM32L4P5VET6', 'STM32L496RGT6']
    if hardware not in supported_hardware:
        raise ValueError(f"Unsupported hardware '{hardware}'. Must be one of {supported_hardware}")

    memory_map_mode = 'default' if flash_type != 'legacy' else 'legacy'

    if memory_map_mode == 'default':
        # Generate the default map from the chip's board.toml (single source of geometry)
        memory_script_path = generate_default_memory_map(hardware)
    else:
        memory_script_path = os.path.join(PLATFORM_DIR, 'hardware', hardware, 'legacy', 'memory_map.ld')

    if not os.path.exists(memory_script_path):
        raise FileNotFoundError(f"Memory map linker script not found: {memory_script_path}")

    organization_scripts_map = {
        'bootloader': 'sections_bootloader.ld',
        'application': 'sections_app_active.ld',
        'bootstrap': 'sections_bootstrap.ld',
        'legacy': 'sections_app_active.ld',
    }

    organization_script_name = organization_scripts_map.get(flash_type)
    if not organization_script_name:
        organization_script_name = organization_scripts_map['application']

    organization_script_full_path = os.path.join(
        PLATFORM_DIR,
        'hardware',
        'templates',
        organization_script_name
    )

    if not os.path.exists(organization_script_full_path):
        raise FileNotFoundError(f"Organization linker script not found: {organization_script_full_path}")

    linker_search_paths = [
        os.path.dirname(memory_script_path),
        os.path.join(PLATFORM_DIR, 'hardware', 'templates'),
    ]

    link_flags = []
    for path in linker_search_paths:
        link_flags.append(f'-L{path}')

    link_flags.extend([
        f'-T{memory_script_path}',
        f'-T{organization_script_full_path}',
    ])

    return link_flags


def get_defines(hardware, build_config):
    """Generate the list of defines based on the hardware type."""

    defines = hardware_defines.get(hardware, [])
    defines = defines + common_defines

    if build_config == 'debug':
        defines += debug_defines
    else:
        defines += release_defines

    define_flags = ['-D{}'.format(define) for define in defines]
    return define_flags


def create_arm_env(hardware, flash_type='default', build_config='debug'):
    # board.toml is the single source: regenerate the derived C headers before any compile
    generate_board_headers(hardware)
    env_defines = get_defines(hardware, build_config)
    if build_config == 'debug':
        build_config_flags = debug_flags
    else:
        build_config_flags = release_flags
    return Environment(
        ENV = os.environ,

        CC=compiler,
        CCFLAGS=common_flags + build_config_flags + arch_cflags + env_defines,
        CPPPATH=[],

        AS=compiler,
        ASFLAGS=['-c'] + common_flags + build_config_flags + arch_cflags + env_defines,
        
        LINK=compiler,
        LINKFLAGS=common_flags + arch_cflags + get_link_flags(hardware, flash_type),

        AR=ar,
        RANLIB=ranlib,

        LIBS=['m'],
    )

bin_builder = Builder(action='{} -O binary -R .eh_frame $SOURCE $TARGET'.format(objcopy))
arm_env = create_arm_env(HARDWARE_TYPE, FLASH_TYPE, BUILD_CONFIG)
arm_env.Append(BUILDERS={'Bin': bin_builder})

Return('arm_env')
