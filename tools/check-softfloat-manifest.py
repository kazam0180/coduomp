#!/usr/bin/env python3

"""Check that the Meson SoftFloat source list matches the vendored build."""

import re
import sys
from pathlib import Path


def object_names(text, variable):
    match = re.search(
        r"^" + variable + r"\s*=\s*\\\n(.*?)(?=\n\n|\n[A-Z_]+ =|\Z)",
        text,
        re.MULTILINE | re.DOTALL,
    )
    if match is None:
        raise SystemExit("missing {} in SoftFloat Makefile".format(variable))
    return re.findall(r"([A-Za-z0-9_]+)\$\(OBJ\)", match.group(1))


def main():
    root = Path(__file__).resolve().parent.parent
    makefile = root / "vendor/softfloat/build/coduo-x87/Makefile"
    manifest = root / "meson/sources/softfloat_c.txt"

    text = makefile.read_text()
    expected = []
    for name in object_names(text, "OBJS_PRIMITIVES"):
        expected.append("vendor/softfloat/source/{}.c".format(name))
    for name in object_names(text, "OBJS_SPECIALIZE"):
        expected.append("vendor/softfloat/source/8086/{}.c".format(name))
    for name in object_names(text, "OBJS_OTHERS"):
        expected.append("vendor/softfloat/source/{}.c".format(name))

    actual = [line.strip() for line in manifest.read_text().splitlines()
              if line.strip()]

    if actual == expected:
        print("SoftFloat manifest matches the vendored Makefile")
        return 0

    only_makefile = [path for path in expected if path not in actual]
    only_manifest = [path for path in actual if path not in expected]

    if only_makefile:
        print("error: Meson SoftFloat manifest is missing:", file=sys.stderr)
        for path in only_makefile:
            print("  " + path, file=sys.stderr)
    if only_manifest:
        print("error: Meson SoftFloat manifest has extra sources:", file=sys.stderr)
        for path in only_manifest:
            print("  " + path, file=sys.stderr)
    if not only_makefile and not only_manifest:
        print("error: Meson SoftFloat manifest order differs from the Makefile",
              file=sys.stderr)

    return 1


if __name__ == "__main__":
    sys.exit(main())
