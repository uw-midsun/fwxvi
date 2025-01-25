import os
Import ('FLASH_TYPE')
Import ('BUILD_CONFIG')

PLATFORM_DIR = os.getcwd()

compiler = 'arm-none-eabi-gcc'
ranlib = 'arm-none-eabi-gcc-ranlib'
objcopy = 'arm-none-eabi-objcopy'
ar = 'arm-none-eabi-gcc-ar'

arch_cflags = [
    '-mlittle-endian',
    '-mcpu=cortex-m4',
    '-mfloat-abi=hard',
    '-mfpu=fpv4-sp-d16',
    '-mthumb'
]

defines = [
    'MS_PLATFORM_ARM',
    'STM32L433xx',
    'HSE_VALUE=16000000',
    'LSE_VALUE=32768'
]

define_flags = ['-D{}'.format(define) for define in defines]

debug_flags = [
    '-g3',
    '-ggdb3',
    '-O0',
    '-fno-inline',
    '-fstack-usage',
    '-DDEBUG'
]

release_flags = [
    '-flto',
    '-Os',
    '-fomit-frame-pointer',
    '-mthumb',
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

def get_link_flags(flash_type='default'):
    linker_scripts = {
        'default': 'stm32l4_default.ld',
        'bootloader': 'stm32l4_bootloader.ld',
        'application': 'stm32l4_application.ld'
    }
    script = linker_scripts.get(flash_type, linker_scripts['default'])
    return [
        '-L{}/linker_scripts'.format(PLATFORM_DIR),
        '-T{}'.format(script),
    ]

def create_arm_env(flash_type='default', build_config='debug'):
    if build_config == 'debug':
        build_config_flags = debug_flags
    else:
        print(build_config)
        build_config_flags = release_flags

    return Environment(
        ENV = { 'PATH': os.environ['PATH'] },

        CC=compiler,
        CCFLAGS=common_flags + build_config_flags + arch_cflags + define_flags,
        CPPPATH=[],

        AS=compiler,
        ASFLAGS=['-c'] + common_flags + build_config_flags + arch_cflags + define_flags,
        
        LINK=compiler,
        LINKFLAGS=common_flags + arch_cflags + get_link_flags(flash_type),

        AR=ar,
        RANLIB=ranlib,

        LIBS=['m'],
    )

bin_builder = Builder(action='{} -O binary $SOURCE $TARGET'.format(objcopy))
arm_env = create_arm_env(FLASH_TYPE, BUILD_CONFIG)
arm_env.Append(BUILDERS={'Bin': bin_builder})

Return('arm_env')
