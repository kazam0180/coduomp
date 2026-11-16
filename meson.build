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
game_module_arch = is_i386 ? 'i386' : 'x86_64'

# The recovery path imports the stock i386 C++ runtime symbols `_Znaj`
# (operator new[](unsigned int)) and `_ZdlPv`.  A 64-bit libstdc++ exports
# `_Znam` instead, so the PB module-copy path cannot link there.  Fail at
# configure time rather than after a full build.
if get_option('server') and get_option('punkbuster') and not is_i386
  error('punkbuster=true requires a 32-bit x86 target because the recovery path imports the i386 C++ ABI symbol _Znaj. Configure with --cross-file cross/linux-i386.ini.')
endif

fp_faithful = get_option('fp_faithful')

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


m_dep = cc.find_library('m', required: false)
dl_dep = cc.find_library('dl', required: false)
# The Make build passed -pthread at link time but not when compiling the
# server.  Keep the link flag and drop the compile flag.
threads_dep = dependency('threads').partial_dependency(compile_args: false, link_args: true)
zlib_dep = dependency('zlib')

top_include = include_directories('src')

# Source manifests live in meson/sources/*.txt.  They are one path per line so
# they remain reviewable without turning meson.build into a thousand-line
# include list.  client_c.txt is the canonical client list; client_hot_c.txt
# and client_rest_c.txt partition it.  tools/check-client-manifests.py verifies
# that partition and that every source file under src/ belongs to a manifest.
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

# GCC 14 promotes these C constraint violations to errors by default.  Keep
# older release toolchains equally strict so a downstream compiler cannot be
# the first to reject a source build.  These options are C-only; GCC rejects
# them for C++ translation units.
c_type_error_args = [
  '-Werror=implicit-int',
  '-Werror=implicit-function-declaration',
  '-Werror=incompatible-pointer-types',
  '-Werror=int-conversion',
  '-Werror=return-type',
]
strict_warning_args = ['-Wall', '-Wextra', '-Werror']
strict_c_warning_args = strict_warning_args + c_type_error_args

# The native SDL2 pkg-config file defines _GNU_SOURCE, but the i386 one does
# not.  The Make build passed SDL2's flags to both client architectures and
# received the define, so keep it explicit instead of depending on which
# pkg-config file the build directory resolves.
client_gnu_source_args = ['-D_GNU_SOURCE=1']


client_c_args = [
  '-std=c11',
  '-g',
  '-O0',
  '-DAUDIO_BACKEND_MINIAUDIO',
  '-DAUDIO_BACKEND_OPENAL',
  '-fwrapv',
  '-ffp-contract=off',
  '-fno-common',
] + client_gnu_source_args + strict_c_warning_args + [
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
  '-DAUDIO_BACKEND_MINIAUDIO',
  '-DAUDIO_BACKEND_OPENAL',
  '-fwrapv',
  '-ffp-contract=off',
  '-fno-common',
] + client_gnu_source_args + strict_warning_args + [
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
  '-ffp-contract=off',
  '-fno-common',
  '-fPIC',
  '-fvisibility=hidden',
] + strict_c_warning_args + [
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
  '-fwrapv',
  '-fno-common',
  '-fPIC',
  '-fvisibility=hidden',
] + strict_c_warning_args + [
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
  '-DCODUO_ENABLE_PUNKBUSTER=' + (get_option('punkbuster') ? '1' : '0'),
  '-DCODUO_DISABLE_SERVER_AUTH=' + (get_option('server_auth') ? '0' : '1'),
  '-fsigned-char',
  '-fno-short-enums',
  '-fwrapv',
] + strict_c_warning_args + [
  '-pedantic',
  '-iquote', root + '/src/server/standalone/bindings',
  '-iquote', root + '/src/qcommon/bindings/default',
] + x87_float_args + x87_allow_args
if is_i386
  server_c_args += ['-march=i386']
endif

server_cxx_args = [
  '-std=c++11',
  '-O0',
  '-D_GNU_SOURCE',
  '-DCODUO_ENABLE_PUNKBUSTER=' + (get_option('punkbuster') ? '1' : '0'),
  '-DCODUO_DISABLE_SERVER_AUTH=' + (get_option('server_auth') ? '0' : '1'),
  '-fsigned-char',
  '-fno-short-enums',
  '-fwrapv',
] + strict_warning_args + [
  # The shared recovered headers use C flexible-array members.  That is a GNU
  # extension, not ISO C++, so -pedantic rejects these two C++ translation
  # units.  Leave -pedantic off here rather than disable its warning category;
  # the remaining warnings stay enabled.
  '-iquote', root + '/src/server/standalone/bindings',
  '-iquote', root + '/src/qcommon/bindings/default',
] + x87_float_args + x87_allow_args
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
] + strict_c_warning_args + [
  '-pedantic',
  '-iquote', root + '/src/qcommon/bindings/default',
] + x87_allow_args

game_client_args = game_common_args + [
  '-fPIC',
  '-DGAME_REQUIRE_X87_LONG_DOUBLE=0',
  '-DGAME_ALLOW_NON_EQUIVALENT_LONG_DOUBLE=1',
  '-iquote', root + '/src/server/game/bindings',
]
if is_i386
  game_client_args += ['-march=i386', '-fexcess-precision=fast']
endif

game_server_args = game_common_args + [
  '-iquote', root + '/src/server/game/bindings',
] + x87_float_args
if fp_faithful == 'relaxed'
  game_server_args += [
    '-DGAME_REQUIRE_X87_LONG_DOUBLE=0',
    '-DGAME_ALLOW_NON_EQUIVALENT_LONG_DOUBLE=1',
  ]
endif
if is_i386
  game_server_args += ['-march=i386', '-fexcess-precision=fast']
endif

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

  subdir('server')
endif

# Keep the commonly used previous Make entry points as Meson aliases and run
# targets.  Architecture-specific builds come from a build directory configured
# with the matching cross file.  Build products stay under the configured build
# directory; packaging output stays under .workbench/build/linux/.
run_target('policy-check',
  command: [find_program('bash'), root + '/tools/check-source-policy.sh'])

if get_option('client')
  client_targets = [coduomp_client, cgame, ui]
  if get_option('game')
    client_targets += game_client
  endif
  alias_target('client', client_targets)
  if is_i386
    alias_target('client-linux32', client_targets)
    run_target('client-linux64',
      command: [find_program('bash'), '-c',
        'echo "error: client-linux64 requires a 64-bit build directory configured without the i386 cross file" >&2; exit 1'])
  else
    alias_target('client-linux64', client_targets)
    run_target('client-linux32',
      command: [find_program('bash'), '-c',
        'echo "error: client-linux32 requires a 32-bit build directory configured with --cross-file cross/linux-i386.ini" >&2; exit 1'])
  endif
  alias_target('client-engine', [coduomp_client])
  alias_target('client-cgame', [cgame])
  alias_target('client-ui', [ui])
  if get_option('game')
    alias_target('client-game', [game_client])
  endif

  # The previous UI syntax-check target used clang because
  # -Wshorten-64-to-32 is a clang-only warning.  Keep the width checks on the
  # UI sources even when the normal build uses GCC.
  clang = find_program('clang', required: false)
  if clang.found()
    run_target('ui-syntax-check',
      command: [clang, '-fsyntax-only', '-I', root + '/src'] + ui_c_args + [
        # Clang 23 warns about three retained endian dispatch pointers that the
        # old clang did not flag.  The check is for width bugs, not dead code.
        '-Wno-unused-but-set-global',
        '-Werror=shorten-64-to-32',
        '-Werror=pointer-to-int-cast',
        '-Werror=int-to-pointer-cast',
        '-Werror=void-pointer-to-int-cast',
      ] + ui_c)
  else
    run_target('ui-syntax-check',
      command: [find_program('bash'), '-c',
        'echo "error: ui-syntax-check requires clang with -Wshorten-64-to-32 support." >&2; exit 1'])
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

run_target('check-linux-host',
  command: [find_program('bash'), '-c',
    'if [ "$(uname -s)" != Linux ]; then echo "error: Linux targets must be built on Linux" >&2; exit 2; fi'])

if get_option('game')

  game_strict_check_targets = []
  if get_option('client')
    game_strict_check_client = shared_library(
      'game_mp_uo_strict_client_' + game_module_arch, game_c,
      c_args: game_client_args,
      include_directories: top_include,
      dependencies: [m_dep],
      name_prefix: '',
      name_suffix: 'so',
      override_options: ['b_lundef=true'],
      build_by_default: false)
    game_strict_check_targets += [game_strict_check_client]
  endif
  if get_option('server')
    game_strict_check_server = shared_library(
      'game_mp_uo_strict_server_' + game_module_arch, game_c,
      c_args: game_server_args + ['-fPIC'],
      include_directories: top_include,
      dependencies: [m_dep],
      name_prefix: '',
      name_suffix: 'so',
      override_options: ['b_lundef=true'],
      build_by_default: false)
    game_strict_check_targets += [game_strict_check_server]
  endif

  if game_strict_check_targets.length() > 0
    run_target('shared-check-link',
      command: [find_program('bash'), '-c',
        'echo "shared game module strict link passed"'],
      depends: game_strict_check_targets)
    run_target('shared-check',
      command: [find_program('bash'), '-c',
        'echo "shared game module strict link passed"'],
      depends: game_strict_check_targets)
  else
    run_target('shared-check-link',
      command: [find_program('bash'), '-c',
        'echo "error: shared-check requires a game module target" >&2; exit 1'])
    run_target('shared-check',
      command: [find_program('bash'), '-c',
        'echo "error: shared-check requires a game module target" >&2; exit 1'])
  endif

  if is_i386
    shared_i386_targets = []
    if get_option('client')
      shared_i386_targets += [game_client]
    endif
    if get_option('server')
      shared_i386_targets += [game_server]
    endif
    if shared_i386_targets.length() > 0
      run_target('shared-i386',
        command: [find_program('bash'), '-c',
          'echo "32-bit game modules built"'],
        depends: shared_i386_targets)
    else
      run_target('shared-i386',
        command: [find_program('bash'), '-c',
          'echo "error: shared-i386 requires a game module target" >&2; exit 1'])
    endif
  else
    run_target('shared-i386',
      command: [find_program('bash'), '-c',
        'echo "error: shared-i386 requires a 32-bit build directory configured with --cross-file cross/linux-i386.ini" >&2; exit 1'])
  endif

  if not is_i386
    game_native_check = static_library(
      'game_mp_uo_recovered_native_' + game_module_arch, game_c,
      c_args: game_server_args,
      include_directories: top_include,
      dependencies: [m_dep],
      pic: false,
      build_by_default: false)
    run_target('native64-check',
      command: [find_program('bash'), '-c',
        'echo "native 64-bit game archive built"'],
      depends: [game_native_check])
  else
    run_target('native64-check',
      command: [find_program('bash'), '-c',
        'echo "error: native64-check requires a 64-bit build directory" >&2; exit 1'])
  endif

  if not is_i386
    native64_shared_targets = []
    if get_option('client')
      native64_shared_targets += [game_client]
    endif
    if get_option('server')
      native64_shared_targets += [game_server]
    endif
    if native64_shared_targets.length() > 0
      run_target('native64-shared',
        command: [find_program('bash'), '-c',
          'echo "native 64-bit game modules built"'],
        depends: native64_shared_targets)
    else
      run_target('native64-shared',
        command: [find_program('bash'), '-c',
          'echo "error: native64-shared requires a game module target" >&2; exit 1'])
    endif
  else
    run_target('native64-shared',
      command: [find_program('bash'), '-c',
        'echo "error: native64-shared requires a 64-bit build directory" >&2; exit 1'])
  endif
endif
