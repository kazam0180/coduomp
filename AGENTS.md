# Linux Fork Agent Instructions

This repository is a personal, Linux-only fork of
[`opencoduo/coduomp`](https://github.com/opencoduo/coduomp). It is not the
upstream public product tree. This fork carries intentional personal additions
and modifications and is not stock-faithful: it does not preserve the stock
binary's behavior, defaults, quirks, or bugs for parity.

## Fork scope

- Supported build and runtime targets are 32-bit and 64-bit x86 Linux. macOS and
  Windows platform support was deliberately removed. Do not restore removed
  platform source, resources, build rules, packaging, or CI, and do not add a
  new non-Linux target, unless the user explicitly requests it.
- Client-side builds still select `CODUO_BEHAVIOR=windows` to preserve the
  original module behavior on Linux. That is compatibility behavior, not
  Windows platform support. Do not remove it as leftover Windows code.
- Fork-only behavior is maintained behavior. Preserve it through refactors,
  merges, and rebases. It includes:
  - server bot support: pathfinding, reconnect mode, random names, configured
    or random fake pings, and the `kickbots` operator command;
  - GSC builtins and player methods, including
    `connectionlessPacketToClient`, plus extended server chat commands;
  - the `jump_slowdownEnable` movement cvar;
  - the AppImage packaging workflow and its supporting scripts and assets.
- Do not revert a personal addition as cleanup. If a change looks wrong or
  unused, trace its callers and report it before removing it.
- Do not match stock behavior for its own sake. Match it only when a concrete
  compatibility requirement (a mod, protocol, data format, or module interface)
  requires it, and name that requirement in the change. Do not preserve an
  original-game defect or an accidental quirk to keep parity.
- Fix a defect in code you are already changing. Report unrelated defects
  instead of silently expanding scope.

## Branch and upstream model

- `master` is this fork's working branch. Make fork changes there unless the
  user directs otherwise.
- `docs/branch-policy.md` and the `stock` branch describe upstream's
  stock-compatible baseline and transcription workflow. They are reference
  material for upstream code, not requirements for this fork. Do not apply the
  stock-first workflow or stock-fidelity rules to personal work.
- Upstream (`https://github.com/opencoduo/coduomp`) is the source of the
  reconstructed baseline. Take useful corrections from it, but adapt or omit
  changes whose only purpose is stock parity.
- Before a large upstream sync, fetch upstream and review the commits in
  `master` that upstream does not contain. Resolve conflicts by preserving both
  the upstream correction and fork-only behavior. Never drop a personal
  addition or reintroduce removed platforms just to make a merge clean.
- Do not add a compile-time or runtime selector whose purpose is to switch
  between stock-faithful and fork behavior. This fork has one product behavior.
- Fork-specific changes are not upstream contributions. Do not push, propose, or
  report them upstream unless the user explicitly asks.

## Security and provenance

- Shared security hardening is welcome. A check that rejects legitimate maps,
  mods, packages, servers, or player behavior is a bug in the check.
- Public comments should state the maintained invariant directly: bounds,
  ownership, type or ABI requirements, sequencing, and failure behavior.
- Do not add exploit recipes, severity assessments, private report paths,
  vulnerable-retail trigger comparisons, or labels such as `SECURITY_PATCH`.
  Coordinate sensitive details with maintainers through a private channel.
- Preserve `NOT_FROM_ORIGINAL_SOURCE` on non-original helpers and adaptations.
  New non-original functions must carry that marker and use a lower-case
  project, compatibility, or adaptation namespace such as `coduomp_*`.
- Do not add private analysis records, retail binaries, generated decompiler
  output, source-recovery ledgers, or machine-specific workspace paths.

## Change discipline

- This is a maintained fork, not a scratchboard. Do not create or commit plans,
  proposed designs, task notes, audit write-ups, or working documents here
  unless the user explicitly requests that document here.
- Requests to evaluate, discuss, plan, or "design first" belong in the
  conversation by default. Naming this repository as an implementation target
  does not authorize adding design documents. Keep optional draft files outside
  this tree.
- Product documentation should describe implemented, supported behavior unless
  the user explicitly requests otherwise. Record user-visible fork changes in
  `CHANGELOG.md`.
- Do not delete an existing code comment unless the complete code section it
  describes is also removed. Rewrite an inaccurate comment in place.
- Keep nearby C/C++ formatting; do not mechanically hard-wrap unaffected code.
- Put generated builds, packages, logs, caches, and disposable runtime state
  under the repository-root `.workbench/`; do not create ignored output beside
  maintained source.
- Release tooling must not embed a maintainer's username, home directory,
  hostname, workspace name, network address, or private remote path. Keep
  machine-specific release settings as invocation-time values and run the
  maintained package privacy/import audits.
- Validate source-policy invariants with
  `meson compile -C BUILD_DIR policy-check` (or `tools/check-source-policy.sh`)
  before committing.
- Build and test every affected Linux target in proportion to the change: the
  native client with `meson compile`, and the relevant `client-linux32`,
  `client-linux64`, `server`, or 32-bit cross target.
- `client-run` is the user-facing normal-game launcher and intentionally uses
  the persistent system profile. Agents must never invoke it or create, edit,
  move, seed, restore, replace, or delete files in the profile it uses unless
  the user explicitly requests the exact operation and identifies the
  authoritative source for any restored or replacement data.
- Use the `client-test-run` Meson target for automated and development client
  runs; it keeps its runtime and home directory under `.workbench/runtime/`.
- Do not edit, refactor, rename, restructure, or otherwise change `client-run`,
  `client-test-run`, their dependency graph, or their configuration/profile/
  home-path routing unless the user explicitly requests that exact change.
