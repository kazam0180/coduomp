#!/usr/bin/env python3

"""Check the Meson source manifests against the maintained source tree."""

import sys
from pathlib import Path


MANIFESTS = (
    "client_c.txt",
    "client_hot_c.txt",
    "client_rest_c.txt",
    "client_cxx.txt",
    "cgame_c.txt",
    "ui_c.txt",
    "game_c.txt",
    "server_c.txt",
    "server_cxx.txt",
    "server_pb_disabled_c.txt",
    "server_pb_server_c.txt",
)

SOURCE_SUFFIXES = (".c", ".cc", ".cpp", ".cxx")

# Directories whose complete C contents must appear in the named manifest.
# These are target-owned trees, plus shared trees that the target compiles in
# full.  A shared tree that each target compiles only in part is covered by
# the whole-tree check below instead.
COMPLETE_C_ROOTS = {
    "client_c.txt": (
        ("src/animation", ()),
        ("src/client/common", ()),
        ("src/client/engine", ()),
        ("src/collision", ()),
        ("src/compat/crt", ()),
        ("src/filesystem", ()),
        ("src/qcommon", ()),
        ("src/scripting", ("src/scripting/generated",)),
        ("src/server/engine", ()),
        ("src/sound/alias", ()),
    ),
    "cgame_c.txt": (
        ("src/bg", ()),
        ("src/client/cgame", ()),
        ("src/client/common", ()),
        ("src/client/math", ()),
        ("src/client/menu", ()),
        ("src/compat/crt", ()),
    ),
    "ui_c.txt": (
        ("src/client/common", ()),
        ("src/client/math", ()),
        ("src/client/menu", ()),
        ("src/client/ui", ()),
    ),
    "game_c.txt": (
        ("src/bg", ()),
        ("src/compat/crt", ()),
        ("src/compat/libm", ()),
        ("src/server/game", ()),
    ),
    "server_c.txt": (
        ("src/animation", ()),
        ("src/collision", ()),
        ("src/compat/crt", ()),
        ("src/filesystem", ()),
        ("src/qcommon", ()),
        ("src/scripting", ("src/scripting/generated",)),
        ("src/server/engine", ()),
        ("src/server/standalone", ("src/server/standalone/punkbuster",)),
        ("src/sound/alias", ()),
    ),
}

COMPLETE_CXX_ROOTS = {
    "client_cxx.txt": ("src/client/engine", "src/scripting"),
    "server_cxx.txt": ("src/scripting",),
}

# Directories whose complete contents are split across mutually exclusive
# variants.  Every file must appear in exactly one of the listed manifests.
COMPLETE_PARTITIONS = (
    ("src/server/standalone/punkbuster", (".c",),
     ("server_pb_disabled_c.txt", "server_pb_server_c.txt")),
)


def read_manifest(path):
    return [Path(line.strip()).as_posix()
            for line in path.read_text().splitlines() if line.strip()]


def report_duplicates(name, entries, failures):
    seen = set()
    duplicates = []
    for entry in entries:
        if entry in seen and entry not in duplicates:
            duplicates.append(entry)
        seen.add(entry)
    if duplicates:
        failures.append("{} has duplicate entries:".format(name))
        failures.extend("  " + entry for entry in duplicates)


def tree_sources(root, directory, suffixes, excluded=()):
    base = root / directory
    found = set()
    if not base.is_dir():
        return found
    for suffix in suffixes:
        for path in base.rglob("*" + suffix):
            if not path.is_file():
                continue
            relative = path.relative_to(root).as_posix()
            if any(relative == prefix or relative.startswith(prefix + "/")
                   for prefix in excluded):
                continue
            found.add(relative)
    return found


def main():
    root = Path(__file__).resolve().parent.parent
    source_dir = root / "meson/sources"

    failures = []
    manifests = {}
    for name in MANIFESTS:
        path = source_dir / name
        if not path.is_file():
            failures.append("missing manifest: meson/sources/{}".format(name))
            continue
        manifests[name] = read_manifest(path)
        report_duplicates(name, manifests[name], failures)

    for name, entries in manifests.items():
        absent = sorted(path for path in set(entries)
                        if not (root / path).is_file())
        if absent:
            failures.append("{} entries do not name files:".format(name))
            failures.extend("  " + path for path in absent)

    canonical = manifests.get("client_c.txt")
    hot = manifests.get("client_hot_c.txt")
    rest = manifests.get("client_rest_c.txt")
    if canonical is not None and hot is not None and rest is not None:
        canonical_set = set(canonical)
        hot_set = set(hot)
        rest_set = set(rest)

        missing = sorted(canonical_set - (hot_set | rest_set))
        extra = sorted((hot_set | rest_set) - canonical_set)
        overlap = sorted(hot_set & rest_set)

        if missing:
            failures.append("client_c.txt entries missing from hot/rest:")
            failures.extend("  " + path for path in missing)
        if extra:
            failures.append("hot/rest entries missing from client_c.txt:")
            failures.extend("  " + path for path in extra)
        if overlap:
            failures.append("entries appear in both hot and rest:")
            failures.extend("  " + path for path in overlap)

    listed = set()
    for entries in manifests.values():
        listed.update(entries)

    unlisted = sorted(tree_sources(root, "src", SOURCE_SUFFIXES) - listed)
    if unlisted:
        failures.append("source tree files are missing from every manifest:")
        failures.extend("  " + path for path in unlisted)

    for name, roots in COMPLETE_C_ROOTS.items():
        entries = manifests.get(name)
        if entries is None:
            continue
        listed = set(entries)
        absent = []
        for directory, excluded in roots:
            if not (root / directory).is_dir():
                failures.append(
                    "{} complete source tree does not exist: {}".format(
                        name, directory))
                continue
            absent.extend(tree_sources(root, directory, (".c",), excluded)
                          - listed)
        absent = sorted(set(absent))
        if absent:
            failures.append(
                "{} is missing C files from its complete source trees:"
                .format(name))
            failures.extend("  " + path for path in absent)

    for name, roots in COMPLETE_CXX_ROOTS.items():
        entries = manifests.get(name)
        if entries is None:
            continue
        listed = set(entries)
        absent = []
        for directory in roots:
            if not (root / directory).is_dir():
                failures.append(
                    "{} complete source tree does not exist: {}".format(
                        name, directory))
                continue
            absent.extend(
                tree_sources(root, directory, (".cc", ".cpp", ".cxx")) -
                listed)
        absent = sorted(set(absent))
        if absent:
            failures.append(
                "{} is missing C++ files from its complete source trees:"
                .format(name))
            failures.extend("  " + path for path in absent)

    for directory, suffixes, names in COMPLETE_PARTITIONS:
        if not (root / directory).is_dir():
            failures.append(
                "{} complete source tree does not exist: {}".format(
                    " / ".join(names), directory))
            continue
        tree = tree_sources(root, directory, suffixes)
        counts = {}
        for name in names:
            for entry in manifests.get(name, ()):
                counts[entry] = counts.get(entry, 0) + 1
        listed = set(counts)

        unlisted = sorted(tree - listed)
        repeated = sorted(entry for entry, count in counts.items()
                          if count > 1)
        foreign = sorted(listed - tree)

        if unlisted:
            failures.append(
                "{} files are missing from the partition manifests:"
                .format(directory))
            failures.extend("  " + path for path in unlisted)
        if repeated:
            failures.append(
                "{} files appear in more than one partition manifest:".format(
                    directory))
            failures.extend("  " + path for path in repeated)
        if foreign:
            failures.append(
                "partition manifests name files outside {}:".format(
                    directory))
            failures.extend("  " + path for path in foreign)

    if failures:
        for line in failures:
            print("error: " + line, file=sys.stderr)
        return 1

    print("Source manifests agree with the maintained source tree")
    return 0


if __name__ == "__main__":
    sys.exit(main())
