project('neonsignal', 'cpp',
  version : '0.2.0',  # Project version (matches neonsignal binary version)
  default_options : [
    'cpp_std=c++23',
    'warning_level=3',
    'buildtype=release',
    'debug=false',
    'optimization=2'
  ]
)

if host_machine.system() == 'darwin'
  hardening_mode = (get_option('buildtype') == 'debug' or get_option('buildtype') == 'debugoptimized') ? '_LIBCPP_HARDENING_MODE_DEBUG' : '_LIBCPP_HARDENING_MODE_FAST'
  add_project_arguments(
    '-U_LIBCPP_ENABLE_ASSERTIONS',
    '-U_LIBCPP_HARDENING_MODE',
    '-D_LIBCPP_HARDENING_MODE=' + hardening_mode,
    language : 'cpp'
  )
endif

# Binary versions
neonsignal_version = '0.3.0'
neonsignal_redirect_version = '0.1.0'

neonsignal_inc = include_directories('include')

openssl_dep = dependency('openssl',
  version : '>=3.0.0',
  required : true
)

nghttp2_dep = dependency('libnghttp2', required : true)

mdbx_dep = dependency('libmdbx', required : false)
if not mdbx_dep.found()
  mdbx_proj = subproject('libmdbx')
  mdbx_dep = mdbx_proj.get_variable('mdbx_dep')
endif

ansi_dep = dependency('ansi', required : false, static : true)
if not ansi_dep.found()
  ansi_dep = dependency('ansi', fallback : ['ansi', 'ansi_dep'], static : true)
endif

args_dep = dependency('args', required : false)
if not args_dep.found()
  args_dep = dependency('args', fallback : ['args', 'args_dep'])
endif

subdir('src')
