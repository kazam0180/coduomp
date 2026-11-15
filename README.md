<p align="center">
  <img src="assets/coduomp-icon-master.png" alt="Blue star logo" width="280">
</p>

<h1 align="center">Open CoD:UO</h1>

<p align="center">
  Reconstructed source for the <em>Call of Duty: United Offensive</em> multiplayer client and dedicated-server components.
</p>

This repository contains source and public build dependencies only. It does
not contain game data, binaries, private binary-analysis records, generated
decompiler output, or recovery ledgers.

See [the changelog](CHANGELOG.md) for a concise list of user-visible differences from the stock game.

## Branches

- `stock` is the hardened stock-compatible baseline. It preserves legitimate
  original behavior while carrying shared hardening and required platform,
  ABI, and compatibility support.
- `master` descends from `stock` and contains the improved product: intentional
  product changes, ordinary original-game bug fixes, features, and optional
  enhancements.

See [the branch policy](docs/branch-policy.md) for the exact boundary. Run
`tools/check-source-policy.sh` (or the `policy-check` Meson target) before
committing or releasing either branch.

## Layout

- `src/client/engine/`: multiplayer client executable.
- `src/client/cgame/`: client-game module.
- `src/client/ui/`: user-interface module.
- `src/server/standalone/`: dedicated-server process and platform boundaries.
- `src/server/engine/`: server engine shared with the client's listen server.
- `src/server/game/`: server game module.
- `src/`: common qcommon, math, BG, collision, filesystem, animation,
  scripting, sound, and compatibility subsystems.
- `meson.build`, `meson/`, `uo/`, and `server/`: Meson build definitions and
  explicit source manifests.
- `vendor/`: tracked third-party source required by the builds.

## Build

A legally owned Call of Duty: United Offensive installation is needed to run
the client, but the retail game files are not part of the source build.

### Debian or Ubuntu Linux (x86-64)

Install the build tools and development packages:

```sh
sudo apt-get update
sudo apt-get install -y build-essential meson ninja-build pkg-config \
  libsdl2-dev libjpeg-dev libcurl4-openssl-dev libminizip-dev zlib1g-dev \
  libgl1-mesa-dev libopenal-dev libsndfile1-dev
```

`libjpeg-dev` provides libjpeg-turbo, which exports the `JCS_EXT_RGBX`
colorspace used by the client. libjpeg 6 is not required. Meson checks for
that symbol at configure time and rejects a libjpeg build without it.

Configure and build the native client executable, cgame, UI, listen-server game
module, dedicated server, and server game module:

```sh
meson setup .workbench/build/meson
meson compile -C .workbench/build/meson
```

The main outputs are:

```text
.workbench/build/meson/CoDUOMP
.workbench/build/meson/uo/uo_cgame_mp_x86_64.so
.workbench/build/meson/uo/uo_ui_mp_x86_64.so
.workbench/build/meson/uo/uo_game_mp_x86_64.so
.workbench/build/meson/server/coduo_lnxded_recovered
.workbench/build/meson/server/uo/game.mp.uo.x86_64.so
```

Build options are listed by:

```sh
meson configure .workbench/build/meson
```

Dedicated-server authorization is disabled by default. Enable it, or
instrument renderer GPU timing at setup time:

```sh
meson setup .workbench/build/server-auth -Dserver_auth=true
meson setup .workbench/build/gpu-profile -Drenderer_gpu_profile=true
```

The PunkBuster recovery path imports i386 C++ runtime symbols, so it is
32-bit only. Configure it with the i386 cross file:

```sh
meson setup .workbench/build/punkbuster \
  --cross-file cross/linux-i386.ini -Dpunkbuster=true
```

### Debian or Ubuntu Linux (32-bit on an x86-64 host)

Enable i386 packages and install the complete 32-bit toolchain and dependency
set:

```sh
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install -y build-essential gcc-multilib g++-multilib meson \
  ninja-build pkg-config libsdl2-dev:i386 libjpeg-dev:i386 \
  libcurl4-openssl-dev:i386 libminizip-dev:i386 zlib1g-dev:i386 \
  libgl1-mesa-dev:i386 libopenal-dev:i386 libsndfile1-dev:i386
```

Configure and build with the i386 cross file:

```sh
meson setup .workbench/build/meson32 --cross-file cross/linux-i386.ini
meson compile -C .workbench/build/meson32
```

The cross file limits pkg-config to 32-bit and architecture-independent
directories. Setup fails early if a 32-bit development package is missing
instead of linking a 64-bit library into the client.

The 32-bit modules are named `uo_cgame_mp_x86.so`, `uo_ui_mp_x86.so`,
`uo_game_mp_x86.so`, and `game.mp.uo.i386.so`.

### Packaging

Build a complete architecture-matched client archive:

```sh
meson compile -C .workbench/build/meson client-linux64-package
meson compile -C .workbench/build/meson32 client-linux32-package
```

Archives are written under `.workbench/build/linux/`. They contain the
reconstructed executable and modules, but no retail game data or Linux system
libraries. Package staging strips debug and local-symbol data and checks for
build debris and symlinks.

### Selecting Linux compilers

Meson reads the compiler from the environment when a build directory is
created. Use a fresh build directory when changing compilers or important
flags:

```sh
CC=gcc-16 CXX=g++-16 meson setup .workbench/build/meson-gcc16
meson compile -C .workbench/build/meson-gcc16
```

For the 32-bit cross build, set the compilers in `cross/linux-i386.ini` or pass
a modified cross file. Compiler commands may also name absolute paths or
compiler-wrapper commands.

Faithful x86 and x86-64 builds require a GCC-compatible compiler that supports
the x87 options selected by the build. Clang does not implement
`-mfpmath=387`, so the default fidelity policy rejects it for these targets.
`-Dfp_faithful=relaxed` permits such a build for portability work, but its
floating-point behavior is not the validated faithful configuration.

### Running the client

Build and run the native client as the normal system game by pointing it at a
game-data root containing `main/` and `uo/`:

```sh
CLIENT_DATA_PATH="/absolute/path/to/Call of Duty UO" \
CLIENT_ARGS="+set r_fullscreen 1" \
meson compile -C .workbench/build/meson client-run
```

`client-run` uses the system's persistent OpenCoDUO profile, including normal
configuration, favorites, downloads, and screenshots. `CLIENT_WORK_DIR` may
override its working directory, which must remain below `.workbench/runtime/`.

Automated and development tests must use the isolated launcher instead:

```sh
CLIENT_DATA_PATH="/absolute/path/to/Call of Duty UO" \
CLIENT_ARGS="+set r_fullscreen 0" \
meson compile -C .workbench/build/meson client-test-run
```

`client-test-run` always supplies an explicit `fs_homepath` below
`.workbench/runtime/`; it will refuse a runtime or home directory outside that
root. `CLIENT_TEST_WORK_DIR` and `CLIENT_TEST_HOME_PATH` override the isolated
directories. Agents must never use `client-run` for testing.

To launch an existing native build without using a Meson run target, run the
following from the repository root:

```sh
CODUOMP_ROOT="$(pwd -P)"
mkdir -p "$CODUOMP_ROOT/.workbench/runtime/manual/home"
cd "$CODUOMP_ROOT/.workbench/runtime/manual"

"$CODUOMP_ROOT/.workbench/build/meson/CoDUOMP" \
  +set fs_basepath "$CODUOMP_ROOT/.workbench/build/meson" \
  +set fs_cdpath "/absolute/path/to/Call of Duty UO" \
  +set fs_homepath "$CODUOMP_ROOT/.workbench/runtime/manual/home"
```

`fs_basepath` locates the reconstructed native cgame and UI modules staged
under `.workbench/build/meson/uo/`. `fs_cdpath` locates the game-data `main/`
and `uo/` directories. `fs_homepath` keeps generated configuration, logs,
downloads, and caches isolated from personal profiles.

Validate source-policy invariants before committing or releasing:

```sh
meson compile -C .workbench/build/meson policy-check
```

On the `master` branch, see [client gamma control](docs/client-gamma.md) for
the selectable output-gamma policies and provider order.

See [client audio controls](docs/client-audio.md) for the Effects Volume and
Music Volume settings, their saved cvars, and compatibility migration.

See [demo playback controls](docs/client-demo-playback.md) for playback
commands, direct controls, and the saved scrubber-overlay setting.

See [per-server client configuration](docs/client-server-configs.md) for the
console commands that promote the current server profile to the global config
or clear every isolated server config while preserving downloaded content.

See [client build notes](docs/client-building.md),
[server build notes](docs/server-building.md), and
[the project boundary](docs/project-boundary.md) for platform details and
repository scope.

## Server configuration

Player general chat is enabled by default. To disable `say` messages, add this
to the server configuration or enter it in the server console:

```cfg
set g_allowGlobalChat 0
```

The setting can be changed while the server is running. It affects only player
general chat; team, squad, private, scripted, and server-console messages remain
available. Set `g_allowGlobalChat 1` to enable general chat again.

## Status

The goal is functional equivalence rather than a byte-identical rebuild.
Source reconstruction is complete, while validation and original-product bug
triage continue.
