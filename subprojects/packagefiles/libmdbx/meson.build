# Native meson.build for libmdbx
# Workaround for meson CMake module bug on macOS with CMake 4.x
# where mdbx.h++ header is incorrectly added to sources

project('libmdbx', ['c', 'cpp'],
  version : '0.13.10',
  default_options : [
    'c_std=c17',
    'cpp_std=c++23',
    'warning_level=1',
    'buildtype=release',
  ]
)

cc = meson.get_compiler('c')
cpp = meson.get_compiler('cpp')

# Thread dependency
thread_dep = dependency('threads')

# Source files (NOT headers)
libmdbx_sources = [
  'mdbx.c',
  'mdbx.c++',
]

# Compile definitions
libmdbx_defines = [
  '-DMDBX_BUILD_SHARED_LIBRARY=0',
  '-DMDBX_BUILD_CXX=1',
  '-DNDEBUG',
]

# Platform-specific flags
if host_machine.system() == 'darwin'
  libmdbx_c_args = [
    '-fexceptions',
    '-fno-common',
    '-Wno-unknown-pragmas',
    '-ffunction-sections',
    '-fdata-sections',
    '-fvisibility=hidden',
    '-ffast-math',
  ]
  libmdbx_cpp_args = libmdbx_c_args + ['-fcxx-exceptions']
elif host_machine.system() == 'linux'
  libmdbx_c_args = [
    '-fexceptions',
    '-fno-semantic-interposition',
    '-fno-common',
    '-Wno-unknown-pragmas',
    '-ffunction-sections',
    '-fdata-sections',
    '-fvisibility=hidden',
    '-ffast-math',
  ]
  libmdbx_cpp_args = libmdbx_c_args
else
  libmdbx_c_args = []
  libmdbx_cpp_args = []
endif

# Build the static library
libmdbx_static = static_library('mdbx',
  libmdbx_sources,
  c_args : libmdbx_c_args + libmdbx_defines,
  cpp_args : libmdbx_cpp_args + libmdbx_defines,
  dependencies : thread_dep,
  include_directories : include_directories('.'),
)

# Declare dependency for consumers
mdbx_dep = declare_dependency(
  link_with : libmdbx_static,
  include_directories : include_directories('.'),
  dependencies : thread_dep,
)
