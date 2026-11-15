# CoD:UO Dedicated Engine

This directory contains maintained source for the Call of Duty: United Offensive
dedicated-server engine. It covers server-side runtime systems such as
commands, cvars, filesystem access, networking, collision, animation/model
services, scripting support, and game-module host calls.

The exported tree is organized as normal buildable C/C++ source:

- `src/server/standalone/`: process and dedicated-platform code.
- `src/server/engine/`: server engine code shared with an embedded listen
  server.
- common subsystems live directly below `src/`.
- `meson.build` and `server/meson.build`: compile and link targets for the
  maintained engine.

## Build

```sh
meson setup .workbench/build/server -Dclient=false
meson compile -C .workbench/build/server server
```

This builds the dedicated executable and the server game module. The
executable is written to
`.workbench/build/server/server/coduo_lnxded_recovered`. The module is written
to `.workbench/build/server/server/uo/game.mp.uo.x86_64.so`.

The original Linux server is an i386 program. Build a 32-bit server with:

```sh
meson setup .workbench/build/server32 --cross-file cross/linux-i386.ini \
  -Dclient=false
meson compile -C .workbench/build/server32 server
```

That requires a Linux toolchain with 32-bit compiler and library support
installed.

## Game-type voting

The improved game module accepts a whitespace-separated `g_voteGameTypes`
allowlist. For example:

```text
set g_voteGameTypes "tdm dm ctf dom"
```

The list applies to both `callvote g_gametype` and `callvote typemap`, with
case-insensitive, whole-name matching. An empty value (the default) allows
every installed game type. Changes take effect without restarting the map.
The ordinary `g_allowVoteGameType` and `g_allowVoteTypeMap` switches still
control whether those vote commands are enabled. Direct administrator
commands and configured rotations are unaffected. Client menus can still
display installed game types that the server will reject for voting.
