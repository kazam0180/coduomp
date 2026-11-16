# Building

The Meson build produces both server targets:

- The standalone dedicated server from `src/server/standalone/`,
  `src/server/engine/`, and the common engine source.
- The server game module from `src/server/game/` and its shared BG, qcommon,
  and math dependencies.

The source manifests live in `meson/sources/`. The target definitions are in
`meson.build` and `server/meson.build`.

## Requirements

- `meson` 1.0 or newer
- `ninja`
- C and C++ compilers such as `cc`, `c++`, `gcc`, or `g++`
- `pkg-config`
- `ar`
- zlib development headers

## Native x86-64 build

```sh
meson setup .workbench/build/server -Dclient=false
meson compile -C .workbench/build/server
```

Outputs:

```text
.workbench/build/server/server/coduo_lnxded_recovered
.workbench/build/server/server/uo/game.mp.uo.x86_64.so
```

To build only the server targets after configuring a combined build directory:

```sh
meson compile -C .workbench/build/server server
```

Pass `-Dclient=false` at setup time to omit the client executable, cgame, UI,
and listen-server game module.

`meson install` writes `coduo_lnxded_recovered` to `${prefix}/bin` and the server
game module to `${prefix}/bin/uo/game.mp.uo.<arch>.so`:

```sh
meson install -C .workbench/build/server
```

Pass `--destdir DIR` to stage an installation without writing to the system
prefix.

The game-module development checks preserved from the previous build are listed
in [Development checks](client-building.md#development-checks).

## CD-key Authorization

On `master`, dedicated and listen servers skip external CD-key authorization
by default. Enable it at configure time:

```sh
meson setup .workbench/build/server-auth -Dserver_auth=true
meson compile -C .workbench/build/server-auth server
```

The `stock` branch keeps its original authorization defaults.

## 32-bit build

The original Linux server and game module are i386 programs. Build them with
the i386 cross file:

```sh
meson setup .workbench/build/server32 --cross-file cross/linux-i386.ini \
  -Dclient=false
meson compile -C .workbench/build/server32 server
```

Outputs:

```text
.workbench/build/server32/server/coduo_lnxded_recovered
.workbench/build/server32/server/uo/game.mp.uo.i386.so
```

This requires a Linux toolchain with 32-bit compiler and library support
installed. A 32-bit package build also needs the 32-bit development packages
listed in the [client build guide](client-building.md).

## Build options

List and change options with:

```sh
meson configure .workbench/build/server
```

Relevant options:

- `-Dserver=true|false` controls the standalone server executable.
- `-Dgame=true|false` controls both game modules.
- `-Dserver_auth=true|false` controls external CD-key authorization.
- `-Dpunkbuster=true|false` selects the PunkBuster recovery source or the
  disabled stub. The recovery path is 32-bit x86 only because it imports the
  i386 C++ runtime symbol `_Znaj`; configure it with
  `--cross-file cross/linux-i386.ini`.
- `-Dfp_faithful=auto|strict|relaxed` controls the x87 faithfulness policy.
  The default requires a compiler that accepts `-mfpmath=387` on x86-64.

Changing an option after setup is possible with `meson configure`, but a fresh
build directory is clearer when switching architectures or compilers.

## Build Outputs

Build products are intentionally ignored. Remove a Meson build directory with
ordinary non-forced file deletion.
