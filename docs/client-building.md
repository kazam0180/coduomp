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
- `build-mk/`: explicit target source manifests and target build rules.

## Build

The native executable currently supports 32-bit and 64-bit x86 Linux.

### Debian 12

Install the compiler and development packages:

```sh
sudo apt update
sudo apt install build-essential pkg-config libsdl2-dev libjpeg-dev \
  libcurl4-openssl-dev libminizip-dev zlib1g-dev libgl1-mesa-dev \
  libopenal-dev libsndfile1-dev
```

For a 32-bit build on an x86-64 Debian host, also enable i386 packages and
install the multilib compilers and 32-bit dependencies:

```sh
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install gcc-multilib g++-multilib pkg-config libsdl2-dev:i386 \
  libjpeg-dev:i386 libcurl4-openssl-dev:i386 libminizip-dev:i386 \
  zlib1g-dev:i386 libgl1-mesa-dev:i386 libopenal-dev:i386 \
  libsndfile1-dev:i386
```

After installing the dependencies, build the executable and
its architecture-matched cgame, UI, and listen-server game modules:

```sh
make
```

The executable is written to `.workbench/build/client/native/CoDUOMP`, and its
modules are staged under `.workbench/build/client/native/uo/`.

On Linux, the explicit architecture targets build the complete executable,
cgame, UI, and listen-server game-module set. They use GCC multilib mode and
keep their outputs separate:

```sh
make client-linux32
make client-linux64
```

Create complete archives with all three architecture-matched multiplayer
modules under `uo/` with:

```sh
make client-linux32-package
make client-linux64-package
```

The resulting archives are written under `.workbench/build/linux/`. They
contain the reconstructed executable and modules, but no retail game data or
Linux system libraries. Package staging strips debug and local-symbol data
and checks for build debris and symlinks. It does not require a hostname utility
or scan embedded strings for the builder's identity or paths.

To launch it as the normal system game, point `CLIENT_DATA_PATH` at a legally
owned retail installation root containing both `main/` and `uo/`:

```sh
make client-run \
  CLIENT_DATA_PATH="/absolute/path/to/Call of Duty UO" \
  CLIENT_ARGS="+set r_fullscreen 0 +set r_mode 6"
```

`client-run` uses the persistent system profile, so normal configuration,
favorites, downloads, and screenshots remain available.

Automated and development tests must instead use `make client-test-run` with
the same `CLIENT_DATA_PATH` and optional `CLIENT_ARGS`. That target forces
`fs_homepath` below `.workbench/runtime/` and will not access the personal
profile. Agents must never invoke `client-run` for testing.

For compiler-gated OpenGL GPU timing, renderer-phase attribution, and batch
census captures, see [Renderer GPU profiling](client-renderer-gpu-profiling.md).

See [CD-key location on Linux](../CDKEY.md) for the native `codkey`
filename, default locations, custom `fs_homepath` behavior, and safe migration
guidance, including the optional `UO_CODKEY` environment override.

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
