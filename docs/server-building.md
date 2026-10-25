# Building

The public tree contains two independently useful target families:

- `build-mk/server.mk` builds the standalone server from
  `src/server/standalone/`, `src/server/engine/`, and the common engine source.
- `build-mk/game.mk` builds the server game module from `src/server/game/` and
  its shared BG/qcommon/math dependencies.

## CD-key Authorization

On `master`, dedicated and listen servers skip external CD-key authorization
by default. Dedicated-server builds can enable authorization with `AUTH=1`:

```sh
make server64 AUTH=1
make server32 AUTH=1
```

When invoking `build-mk/server.mk` directly, set
`CODUO_DISABLE_SERVER_AUTH=0` to enable authorization. The `stock` branch keeps
its original authorization defaults.

## Requirements

- `make`
- C and C++ compilers such as `cc`, `c++`, `clang`, or `gcc`
- `ar`

## Compile Checks

```sh
make -f build-mk/game.mk check CODUO_FP_FAITHFUL=relaxed
make -f build-mk/server.mk objects CODUO_FP_FAITHFUL=relaxed
```

The game-module check compiles source into a static archive. The
dedicated-engine check compiles maintained engine code into a static archive.

## Shared Module Builds

For server deployment, build the 32-bit Linux module:

```sh
make game32
```

Output:

```text
build/game/linux-i386/game.mp.uo.i386.so
```

This target uses `-m32`, so it requires a Linux toolchain with 32-bit compiler
and library support installed.

For local shared-object experiments using the host toolchain:

```sh
make game64
```

Output:

```text
build/game/linux-x86_64/game.mp.uo.x86_64.so
```

For a local dedicated-engine shared-object link check, use the target Makefile:

```sh
make -f build-mk/server.mk shared-check-link CODUO_FP_FAITHFUL=relaxed
```

For a native dedicated-server executable:

```sh
make server-engine64
```

Output:

```text
.workbench/build/server/linux-x86_64-noauth/coduo_lnxded_recovered
```

The original Linux game module is i386. Building the 32-bit shared module
requires a Linux toolchain with i386 support.

## Build Outputs

Build products are intentionally ignored. Clean them with ordinary non-forced
file deletion or by removing the generated build directories manually.
