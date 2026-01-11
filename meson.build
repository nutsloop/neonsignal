project('neonsignal', 'cpp',
  version : '0.1.0',
  default_options : [
    'cpp_std=c++23',
    'warning_level=3',
    'buildtype=release',
    'debug=false',
    'optimization=2'
  ]
)

neonsignal_inc = include_directories('include')

openssl_dep = dependency('openssl',
  version : '>=3.0.0',
  required : true
)

nghttp2_dep = dependency('libnghttp2', required : true)

mdbx_dep = dependency('libmdbx', required : false)
if not mdbx_dep.found()
  cmake = import('cmake')
  mdbx_opts = cmake.subproject_options()
  mdbx_opts.add_cmake_defines({
    'MDBX_BUILD_SHARED_LIBRARY': 'OFF',
    'MDBX_INSTALL': 'OFF',
    'GIT': 'GIT-NOTFOUND',
  })
  mdbx_proj = cmake.subproject('libmdbx', options : mdbx_opts)
  mdbx_dep = mdbx_proj.get_variable('mdbx_static_dep')
endif

ansi_dep = dependency('ansi', required : false, static : true)
if not ansi_dep.found()
  ansi_dep = dependency('ansi', fallback : ['ansi', 'ansi_dep'], static : true)
endif

subdir('src')
