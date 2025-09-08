import os
import shutil

cc_compiler = 'gcc'
cxx_compiler = 'g++'
ccache_path = 'ccache'

if shutil.which(ccache_path):
    cc_compiler = ccache_path + ' ' + cc_compiler
    cxx_compiler = ccache_path + ' ' + cxx_compiler

commonflags = [
    '-g',
    '-Os',
    '-Wall',
    '-Wextra',
    '-Wno-unused-variable',
    '-Wno-unused-parameter',
    '-Wno-unused-value',
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

    CC=cc_compiler,
    CXX=cxx_compiler,
    CCFLAGS=cflags + define_flags,
    CXXFLAGS=cxxflags + define_flags,
    CPPPATH=[],

    LINKFLAGS=link_flags,

    LIBS=[],
)

Return('x86_env')
