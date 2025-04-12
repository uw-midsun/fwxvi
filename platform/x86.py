import os

commonflags = [
    '-g',
    '-Os',
    '-Wall',
    '-Wextra',
    '-Wno-unused-variable',
    '-Wno-unused-parameter',
    '-Wpointer-arith',
    '-ffunction-sections',
    '-fdata-sections',
    '-I/usr/local/include',
]

cflags = [
    '-std=gnu11',
    '-Wno-discarded-qualifiers',
    '-Werror',
] + commonflags

cxxflags = [
    '-std=c++17',
] + commonflags

defines = [
    'MS_PLATFORM_X86',
    '_GNU_SOURCE',
]

define_flags = ['-D{}'.format(define) for define in defines]

link_flags = [
    '-lrt',
    '-lm',
    '-pthread',
]

x86_env = Environment(
    ENV = { 'PATH': os.environ['PATH'] },

    CC='gcc',
    CXX='g++',
    CCFLAGS=cflags + define_flags,
    CXXFLAGS=cxxflags + define_flags,
    CPPPATH=[],

    LINKFLAGS=link_flags,

    LIBS=[],
)

Return('x86_env')
