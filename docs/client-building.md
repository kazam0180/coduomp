# Client building

Work-in-progress source for the Call of Duty: United Offensive multiplayer
executable, client-game module, and user-interface module.

This package contains maintained C and C++ source only. It does not include the
retail game, game data, proprietary binaries, generated decompiler output, or
private recovery records. You need a legally owned Call of Duty: United
Offensive installation to run the resulting client.

## Included components

- `src/client/engine/`: multiplayer executable code, including the renderer,
  client networking, platform boundaries, and listen-server interface code.
- `src/client/cgame/`: client-game code responsible for snapshots, prediction,
  effects, sound, HUD behavior, and other in-game presentation.
- `src/client/ui/`: multiplayer menus, server browser, controls, and frontend
  UI.
- `src/client/common/`, `src/client/math/`, and `src/client/menu/`: code shared
  only by client-side targets.
- `src/qcommon/`, `src/math/`, `src/collision/`, `src/filesystem/`,
  `src/animation/`, `src/scripting/`, and `src/sound/alias/`: common engine
  subsystems used by the executable and modules.
- `src/server/engine/`: server engine code also used by the executable's
  embedded listen server.
- `meson.build`, `meson/sources/`, `uo/meson.build`: Meson target definitions
  and explicit source manifests.
- `cross/linux-i386.ini`: 32-bit x86 cross file.

## Build

The native executable currently supports 32-bit and 64-bit x86 Linux. Meson
and Ninja are the build system; no Makefile is used.

### Debian 12

Install the build tools, compiler, and development packages:

```sh
sudo apt update
sudo apt install build-essential meson ninja-build pkg-config libsdl2-dev \
  libjpeg-dev libcurl4-openssl-dev libminizip-dev zlib1g-dev libgl1-mesa-dev \
  libopenal-dev libsndfile1-dev
```

`libjpeg-dev` provides libjpeg-turbo, which exports the `JCS_EXT_RGBX`
colorspace used by the client. libjpeg 6 is not required. Meson checks for
that symbol at configure time and rejects a libjpeg build without it.

For a 32-bit build on an x86-64 Debian host, also enable i386 packages and
install the multilib compilers and 32-bit dependencies:

```sh
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install gcc-multilib g++-multilib meson ninja-build pkg-config \
  libsdl2-dev:i386 libjpeg-dev:i386 libcurl4-openssl-dev:i386 \
  libminizip-dev:i386 zlib1g-dev:i386 libgl1-mesa-dev:i386 \
  libopenal-dev:i386 libsndfile1-dev:i386
```

Build the executable and its architecture-matched cgame, UI, and listen-server
game modules:

```sh
meson setup .workbench/build/client
meson compile -C .workbench/build/client
```

The executable is written to `.workbench/build/client/CoDUOMP`, and its modules
are staged under `.workbench/build/client/uo/`. The dedicated server and server
game module are built by the same default configuration unless
`-Dserver=false` is passed.

Use a separate build directory for the 32-bit cross build:

```sh
meson setup .workbench/build/client32 --cross-file cross/linux-i386.ini
meson compile -C .workbench/build/client32
```

The cross file limits pkg-config to 32-bit and architecture-independent
directories. Setup fails early if a 32-bit development package is missing
instead of linking a 64-bit library into the client.

The build directory's architecture selects the `client-linux32` or
`client-linux64` alias. The other name reports that a differently configured
build directory is required.

Create complete archives with all three architecture-matched multiplayer
modules under `uo/` with:

```sh
meson compile -C .workbench/build/client client-linux64-package
meson compile -C .workbench/build/client32 client-linux32-package
```

The resulting archives are written under `.workbench/build/linux/`. They
contain the reconstructed executable and modules, but no retail game data or
Linux system libraries. Package staging strips debug and local-symbol data
and checks for build debris and symlinks. It does not require a hostname utility
or scan embedded strings for the builder's identity or paths.

### Installing

`meson install` writes `CoDUOMP` and `coduo_lnxded_recovered` to
`${prefix}/bin`. It writes the cgame, UI, client game, and server game modules
to `${prefix}/bin/uo/`:

```sh
meson install -C .workbench/build/client
```

The installed client resolves its modules relative to `fs_basepath`. Set
`fs_basepath` to the installed `bin` directory. The default Meson prefix is
`/usr/local`:

```sh
/usr/local/bin/CoDUOMP \
  +set fs_basepath /usr/local/bin \
  +set fs_cdpath "/absolute/path/to/Call of Duty UO" \
  +set fs_homepath "/absolute/path/to/writable/profile"
```

For a staged installation, pass `--destdir` and place `fs_basepath` below the
staged tree:

```sh
meson install -C .workbench/build/client --destdir /tmp/coduomp-install
# fs_basepath is /tmp/coduomp-install/usr/local/bin
```

Build options are set when the build directory is created:

```sh
meson setup .workbench/build/gpu-profile \
  -Drenderer_gpu_profile=true \
  -Dfp_faithful=relaxed
```

`fp_faithful=auto` and `strict` require a compiler that accepts
`-mfpmath=387` on x86-64. Use `relaxed` only for an acknowledged non-exact
build.

To launch it as the normal system game, point `CLIENT_DATA_PATH` at a legally
owned retail installation root containing both `main/` and `uo/`:

```sh
CLIENT_DATA_PATH="/absolute/path/to/Call of Duty UO" \
CLIENT_ARGS="+set r_fullscreen 0 +set r_mode 6" \
meson compile -C .workbench/build/client client-run
```

`client-run` uses the persistent system profile, so normal configuration,
favorites, downloads, and screenshots remain available.

Automated and development tests must instead use the `client-test-run` target
with the same `CLIENT_DATA_PATH` and optional `CLIENT_ARGS`. That target forces
`fs_homepath` below `.workbench/runtime/` and will not access the personal
profile. `CLIENT_TEST_WORK_DIR` and `CLIENT_TEST_HOME_PATH` select isolated
directories. Agents must never invoke `client-run` for testing.

`CLIENT_ARGS` is split on whitespace, and shell glob patterns are passed
literally rather than expanded against the working directory.

For compiler-gated OpenGL GPU timing, renderer-phase attribution, and batch
census captures, see [Renderer GPU profiling](client-renderer-gpu-profiling.md).

See [CD-key location on Linux](../CDKEY.md) for the native `codkey`
filename, default locations, custom `fs_homepath` behavior, and safe migration
guidance, including the optional `UO_CODKEY` environment override.

### Development checks

These Meson run targets preserve the development gates that the previous Make
build exposed. They build extra artifacts but do not change the package
archives or the normal build outputs.

- `meson compile -C BUILD_DIR policy-check` validates source-policy invariants
  against the tree.
- `meson compile -C BUILD_DIR check-linux-host` fails unless the host is Linux.
- `meson compile -C BUILD_DIR ui-syntax-check` runs Clang's `-fsyntax-only`
  over the UI sources with `-Werror=shorten-64-to-32`,
  `-Werror=pointer-to-int-cast`, `-Werror=int-to-pointer-cast`, and
  `-Werror=void-pointer-to-int-cast`. This target needs `clang` on `PATH`.
- `meson compile -C BUILD_DIR shared-check` and `shared-check-link` link the
  game module with all undefined symbols resolved. With both client and server
  enabled they check both game-module variants.
- `meson compile -C BUILD_DIR shared-i386` builds the 32-bit game modules. It
  requires a build directory configured with
  `--cross-file cross/linux-i386.ini`.
- `meson compile -C BUILD_DIR native64-check` builds a non-PIC static archive
  from the native game sources. `native64-shared` builds the native shared game
  modules. Both require a 64-bit build directory.

Native clients use the built-in Miniaudio backend by default. OpenAL remains a
deprecated backup backend on Linux; Linux uses libsndfile for its
streamed-audio decoding, and EFX reverb is enabled when the selected OpenAL
device supports it. The stock-named `mss_3d_provider` cvar selects the complete
audio backend on these clients.

Linux builds default `CASE_SENSITIVE_FS=1`. This preserves the original
case-insensitive game-file lookup on a case-sensitive host filesystem without
renaming retail or mod files.


### Developer packages and official releases

The platform package targets build distributable archives on their supported
hosts. They check package structure and runtime dependencies independently of
the builder's username, home directory, or hostname. Official releases use a
separate maintainer workflow with additional privacy checks on the configured
release hosts.

## Project status

The goal is functional equivalence with the original client, not a
byte-identical rebuild. Source reconstruction is complete, but the result is
still experimental and may contain reconstruction or original-product bugs.
Some original edge cases are deliberately preserved for compatibility.

Bug reports are most useful when they include the module, platform, exact build
command, and a short reproduction using an unmodified retail installation or a
clearly identified mod.
