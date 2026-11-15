project('coduomp', ['c', 'cpp'],
  version: '0.0.0',
  meson_version: '>=1.0.0',
  default_options: [
    'buildtype=plain',
    'warning_level=0',
    'b_pie=false',
  ])

fs = import('fs')
cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

# Meson injects these for every C/C++ target.  The Make build defined neither,
# so undo them here to keep the effective compile options identical.
add_project_arguments('-U_FILE_OFFSET_BITS', language: ['c', 'cpp'])
add_project_arguments('-U_GLIBCXX_ASSERTIONS', language: 'cpp')

root = meson.project_source_root()
host_cpu = host_machine.cpu_family()
is_i386 = host_cpu == 'x86'
is_x86 = is_i386 or host_cpu == 'x86_64'
module_arch = is_i386 ? 'x86' : 'x86_64'

# The recovery path imports the stock i386 C++ runtime symbols `_Znaj`
# (operator new[](unsigned int)) and `_ZdlPv`.  A 64-bit libstdc++ exports
# `_Znam` instead, so the PB module-copy path cannot link there.  Fail at
# configure time rather than after a full build.
if get_option('server') and get_option('punkbuster') and not is_i386
  error('punkbuster=true requires a 32-bit x86 target because the recovery path imports the i386 C++ ABI symbol _Znaj. Configure with --cross-file cross/linux-i386.ini.')
endif

fp_faithful = get_option('fp_faithful')
software_x87_backend = get_option('software_x87_backend')

# The x87 policy is a target property, not a compiler preference.  Keep the
# same failure mode as the previous Make build: an exact build is required
# unless the caller explicitly accepts a non-exact one.
x87_float_args = []
x87_allow_args = []
x87_can_emit = false
if is_i386
  x87_can_emit = true
  x87_float_args = ['-O0', '-fexcess-precision=fast']
elif is_x86
  x87_can_emit = cc.has_argument('-mfpmath=387')
  x87_allow_args = ['-DCODUO_ENGINE_ALLOW_NON_X87_FLOAT=1']
  if fp_faithful != 'relaxed'
    if not x87_can_emit
      error('fp_faithful=' + fp_faithful + ' requires a compiler that accepts -mfpmath=387. Use -Dfp_faithful=relaxed for a non-exact build.')
    endif
    x87_float_args = ['-O0', '-mfpmath=387', '-fexcess-precision=fast']
  else
    x87_float_args = ['-O0']
  endif
else
  if fp_faithful != 'relaxed'
    error('fp_faithful=' + fp_faithful + ' requires an x86 or x86-64 target. Use -Dfp_faithful=relaxed for a non-exact build.')
  endif
  x87_allow_args = ['-DCODUO_ENGINE_ALLOW_NON_X87_FLOAT=1']
  x87_float_args = ['-O0']
endif

emulate_x87 = (not is_x86) and (fp_faithful == 'relaxed')
need_softfloat = (not is_x86)
if emulate_x87 and software_x87_backend == 'softfloat'
  need_softfloat = true
endif

m_dep = cc.find_library('m', required: false)
dl_dep = cc.find_library('dl', required: false)
# The Make build passed -pthread at link time but not when compiling the
# server.  Keep the link flag and drop the compile flag.
threads_dep = dependency('threads').partial_dependency(compile_args: false, link_args: true)
zlib_dep = dependency('zlib')

top_include = include_directories('src')
softfloat_include = include_directories('vendor/softfloat/source/include')

# Source manifests live in meson/sources/*.txt.  They are one path per line so
# they remain reviewable without turning meson.build into a thousand-line
# include list.  client_c.txt is the canonical client list; client_hot_c.txt
# and client_rest_c.txt partition it, and tools/check-client-manifests.py
# verifies that partition.
client_cxx = files(fs.read('meson/sources/client_cxx.txt').strip().split('\n'))
client_hot_c = files(fs.read('meson/sources/client_hot_c.txt').strip().split('\n'))
client_rest_c = files(fs.read('meson/sources/client_rest_c.txt').strip().split('\n'))
cgame_c = files(fs.read('meson/sources/cgame_c.txt').strip().split('\n'))
ui_c = files(fs.read('meson/sources/ui_c.txt').strip().split('\n'))
game_c = files(fs.read('meson/sources/game_c.txt').strip().split('\n'))
server_c = files(fs.read('meson/sources/server_c.txt').strip().split('\n'))
server_cxx = files(fs.read('meson/sources/server_cxx.txt').strip().split('\n'))
server_pb_disabled_c = files(fs.read('meson/sources/server_pb_disabled_c.txt').strip().split('\n'))
server_pb_server_c = files(fs.read('meson/sources/server_pb_server_c.txt').strip().split('\n'))
softfloat_c = files(fs.read('meson/sources/softfloat_c.txt').strip().split('\n'))

client_c_args = [
  '-std=c11',
  '-g',
  '-O0',
  '-D_GNU_SOURCE=1',
  '-DEMULATE_X87=0',
  '-DWINDOWS_BEHAVIOR',
  '-DAUDIO_BACKEND_MINIAUDIO',
  '-DAUDIO_BACKEND_OPENAL',
  '-fwrapv',
  '-ffp-contract=off',
  '-fno-common',
  '-Wall',
  '-Wextra',
  '-Wno-error',
  '-Wno-error=implicit-int',
  '-Wno-error=implicit-function-declaration',
  '-Wno-error=incompatible-pointer-types',
  '-Wno-error=int-conversion',
  '-Wno-error=return-type',
  '-iquote', root + '/src/client/engine/bindings',
  '-iquote', root + '/src/qcommon/bindings/default',
]
if get_option('renderer_gpu_profile')
  client_c_args += ['-DCODUOMP_RENDERER_GPU_PROFILE=1']
endif
if is_i386
  client_c_args += ['-msse']
endif

client_cxx_args = [
  '-std=c++17',
  '-g',
  '-O0',
  '-D_GNU_SOURCE=1',
  '-DEMULATE_X87=0',
  '-DWINDOWS_BEHAVIOR',
  '-DAUDIO_BACKEND_MINIAUDIO',
  '-DAUDIO_BACKEND_OPENAL',
  '-fwrapv',
  '-ffp-contract=off',
  '-fno-common',
  '-Wall',
  '-Wextra',
  '-Wno-error',
  '-iquote', root + '/src/client/engine/bindings',
  '-iquote', root + '/src/qcommon/bindings/default',
]
if get_option('renderer_gpu_profile')
  client_cxx_args += ['-DCODUOMP_RENDERER_GPU_PROFILE=1']
endif
if is_i386
  client_cxx_args += ['-msse']
endif

cgame_c_args = [
  '-std=c11',
  '-g',
  '-O0',
  '-DEMULATE_X87=0',
  '-DWINDOWS_BEHAVIOR',
  '-ffp-contract=off',
  '-fno-common',
  '-fPIC',
  '-fvisibility=hidden',
  '-Wall',
  '-Wextra',
  '-Wno-error',
  '-Wno-error=implicit-int',
  '-Wno-error=implicit-function-declaration',
  '-Wno-error=incompatible-pointer-types',
  '-Wno-error=int-conversion',
  '-Wno-error=return-type',
  '-iquote', root + '/src/client/cgame/bindings',
  '-iquote', root + '/src/qcommon/bindings/default',
]
if is_i386
  cgame_c_args += ['-msse']
endif

ui_c_args = [
  '-std=c11',
  '-g',
  '-O0',
  '-DEMULATE_X87=0',
  '-DWINDOWS_BEHAVIOR',
  '-fwrapv',
  '-fno-common',
  '-fPIC',
  '-fvisibility=hidden',
  '-Wall',
  '-Wextra',
  '-Wno-error',
  '-iquote', root + '/src/client/ui/bindings',
  '-iquote', root + '/src/qcommon/bindings/default',
]
if is_i386
  ui_c_args += ['-msse']
endif

server_c_args = [
  '-std=c11',
  '-O0',
  '-D_GNU_SOURCE',
  '-DLINUX_BEHAVIOR',
  '-DCODUO_ENABLE_PUNKBUSTER=' + (get_option('punkbuster') ? '1' : '0'),
  '-DCODUO_DISABLE_SERVER_AUTH=' + (get_option('server_auth') ? '0' : '1'),
  '-DEMULATE_X87=' + (emulate_x87 ? '1' : '0'),
  '-fsigned-char',
  '-fno-short-enums',
  '-fwrapv',
  '-Wall',
  '-Wextra',
  '-Wno-error',
  '-pedantic',
  '-iquote', root + '/src/server/standalone/bindings',
  '-iquote', root + '/src/qcommon/bindings/default',
] + x87_float_args + x87_allow_args
if emulate_x87
  if software_x87_backend == 'double'
    server_c_args += ['-DEMULATE_X87_BACKEND=EMU_X87_DOUBLE', '-ffp-contract=off']
  endif
endif
if is_i386
  server_c_args += ['-march=i386']
endif

server_cxx_args = [
  '-std=c++11',
  '-O0',
  '-D_GNU_SOURCE',
  '-DLINUX_BEHAVIOR',
  '-DCODUO_ENABLE_PUNKBUSTER=' + (get_option('punkbuster') ? '1' : '0'),
  '-DCODUO_DISABLE_SERVER_AUTH=' + (get_option('server_auth') ? '0' : '1'),
  '-DEMULATE_X87=' + (emulate_x87 ? '1' : '0'),
  '-fsigned-char',
  '-fno-short-enums',
  '-fwrapv',
  '-Wall',
  '-Wextra',
  '-Wno-error',
  # The shared recovered headers use C flexible-array members.  That is a GNU
  # extension, not ISO C++, so -pedantic rejects these two C++ translation
  # units.  Leave -pedantic off here rather than disable its warning category;
  # the remaining warnings stay enabled.
  '-iquote', root + '/src/server/standalone/bindings',
  '-iquote', root + '/src/qcommon/bindings/default',
] + x87_float_args + x87_allow_args
if emulate_x87
  if software_x87_backend == 'double'
    server_cxx_args += ['-DEMULATE_X87_BACKEND=EMU_X87_DOUBLE', '-ffp-contract=off']
  endif
endif
if is_i386
  server_cxx_args += ['-march=i386']
endif

# The game module is built once for the Windows-behavior listen server and once
# for the Linux-behavior dedicated server.  The client module deliberately uses
# the relaxed policy, matching the previous Make build.
game_common_args = [
  '-std=c99',
  '-O0',
  '-fsigned-char',
  '-fno-short-enums',
  '-fwrapv',
  '-fno-common',
  '-fPIC',
  '-Wall',
  '-Wextra',
  '-Wno-error',
  '-pedantic',
  '-iquote', root + '/src/qcommon/bindings/default',
] + x87_allow_args

game_client_args = game_common_args + [
  '-DWINDOWS_BEHAVIOR',
  '-DEMULATE_X87=0',
  '-DGAME_REQUIRE_X87_LONG_DOUBLE=0',
  '-DGAME_ALLOW_NON_EQUIVALENT_LONG_DOUBLE=1',
  '-iquote', root + '/src/server/game/bindings',
]
if is_i386
  game_client_args += ['-march=i386', '-fexcess-precision=fast']
endif

game_server_args = game_common_args + [
  '-DLINUX_BEHAVIOR',
  '-iquote', root + '/src/server/game/bindings',
] + x87_float_args
if fp_faithful == 'relaxed'
  game_server_args += [
    '-DGAME_REQUIRE_X87_LONG_DOUBLE=0',
    '-DGAME_ALLOW_NON_EQUIVALENT_LONG_DOUBLE=1',
    '-DEMULATE_X87=0',
  ]
endif
if is_i386
  game_server_args += ['-march=i386', '-fexcess-precision=fast']
endif
if emulate_x87
  if software_x87_backend == 'double'
    game_server_args += ['-DEMULATE_X87_BACKEND=EMU_X87_DOUBLE', '-ffp-contract=off']
  endif
endif

softfloat_c_args = [
  '-O2',
  '-DSOFTFLOAT_FAST_INT64',
  '-DSOFTFLOAT_ROUND_ODD',
  '-DINLINE_LEVEL=5',
  '-DSOFTFLOAT_FAST_DIV32TO16',
  '-DSOFTFLOAT_FAST_DIV64TO32',
  '-ffunction-sections',
  '-fdata-sections',
]
softfloat_lib = static_library('coduomp_softfloat', softfloat_c,
  c_args: softfloat_c_args,
  include_directories: [
    include_directories('vendor/softfloat/build/coduo-x87'),
    include_directories('vendor/softfloat/source/8086'),
    softfloat_include,
  ],
  pic: false,
  build_by_default: need_softfloat)

if get_option('client')
  libjpeg_dep = dependency('libjpeg')
  if not cc.compiles(
      '#include <stddef.h>\n#include <stdio.h>\n#include <jpeglib.h>\nint main(void) { return (int)JCS_EXT_RGBX; }',
      dependencies: libjpeg_dep,
      name: 'libjpeg JCS_EXT_RGBX support')
    error('libjpeg dependency lacks JCS_EXT_RGBX; install libjpeg-turbo or another libjpeg build with JCS extensions')
  endif
  client_deps = [
    dependency('sdl2'),
    libjpeg_dep,
    dependency('libcurl'),
    dependency('minizip'),
    zlib_dep,
    dependency('openal'),
    dependency('sndfile'),
    dependency('gl'),
    dependency('x11'),
    m_dep,
    dl_dep,
    threads_dep,
  ]

  # The Make build listed the filesystem and sound/alias loaders in its
  # selective-O2 manifest, but the pattern rules for those directories omitted
  # the optimization flag, so those seven files compiled at -O0.  Keep the
  # observed output here; moving them into the hot library is a separate
  # performance change that should be measured on its own.
  client_hot_lib = static_library('coduomp_client_hot', client_hot_c,
    c_args: client_c_args + ['-O2', '-fno-strict-aliasing'],
    include_directories: top_include,
    dependencies: client_deps,
    pic: false)

  coduomp_client = executable('CoDUOMP',
    client_rest_c + client_cxx,
    c_args: client_c_args,
    cpp_args: client_cxx_args,
    include_directories: top_include,
    dependencies: client_deps,
    link_with: client_hot_lib,
    install: true,
    install_dir: get_option('bindir'))

  subdir('uo')
endif

if get_option('server')
  server_sources = server_c + server_cxx
  if get_option('punkbuster')
    server_sources += server_pb_server_c
  else
    server_sources += server_pb_disabled_c
  endif

  server_deps = [m_dep, dl_dep, threads_dep, zlib_dep]
  server_link_with = []
  if emulate_x87 and software_x87_backend == 'softfloat'
    server_link_with += softfloat_lib
  endif

  subdir('server')
endif

# Keep the commonly used previous Make entry points as Meson aliases and run
# targets.  Architecture-specific builds come from a build directory configured
# with the matching cross file.  Build products stay under the configured build
# directory; packaging output stays under .workbench/build/linux/.
# The vendored SoftFloat implementation uses unsigned __int128, which GCC does
# not provide on 32-bit x86.  That target uses native x87 and never links
# SoftFloat, so the legacy alias is only meaningful on 64-bit hosts.
if not is_i386
  alias_target('softfloat', softfloat_lib)
else
  # The vendored SoftFloat implementation needs unsigned __int128, which GCC
  # does not provide for 32-bit x86.  Keep the legacy target name so a caller
  # gets the reason instead of Meson's generic "target not found" error.  The
  # 32-bit build uses native x87 and never links SoftFloat.
  run_target('softfloat',
    command: [
      find_program('bash'),
      '-c',
      'echo "error: softfloat is not available for 32-bit x86; this target uses native x87." >&2; exit 1',
    ])
endif
run_target('policy-check',
  command: [find_program('bash'), root + '/tools/check-source-policy.sh'])

if get_option('client')
  client_targets = [coduomp_client, cgame, ui]
  if get_option('game')
    client_targets += game_client
  endif
  alias_target('client', client_targets)
  alias_target(is_i386 ? 'client-linux32' : 'client-linux64', client_targets)
  alias_target('client-engine', [coduomp_client])
  alias_target('client-cgame', [cgame])
  alias_target('client-ui', [ui])
  if get_option('game')
    alias_target('client-game', [game_client])
  endif

  run_target('client-run',
    command: [root + '/tools/linux/client-run.sh', coduomp_client, meson.current_build_dir()],
    depends: client_targets)

  run_target('client-test-run',
    command: [root + '/tools/linux/client-test-run.sh', coduomp_client, meson.current_build_dir()],
    depends: client_targets)

  if get_option('game')
    package_arch = is_i386 ? 'i686' : 'x86_64'

    run_target(is_i386 ? 'client-linux32-package' : 'client-linux64-package',
      command: [
        root + '/tools/linux/package-client-target.sh',
        coduomp_client,
        cgame,
        ui,
        game_client,
        package_arch,
        module_arch,
      ],
      depends: client_targets)
  endif
endif

if get_option('server')
  server_targets = [server_exe]
  if get_option('game')
    server_targets += game_server
  endif
  alias_target('server', server_targets)
  alias_target(is_i386 ? 'server32' : 'server64', server_targets)
  alias_target('server-engine', [server_exe])
  alias_target(is_i386 ? 'server-engine32' : 'server-engine64', [server_exe])
  if get_option('game')
    alias_target('server-game', [game_server])
    alias_target(is_i386 ? 'server-game32' : 'server-game64', [game_server])
  endif
endif
