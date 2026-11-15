#!/usr/bin/env python3

"""Check that the client C manifests partition the canonical client list."""

import sys
from pathlib import Path


def read_manifest(directory, name):
    path = directory / name
    return [line.strip() for line in path.read_text().splitlines()
            if line.strip()]


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


def main():
    root = Path(__file__).resolve().parent.parent
    source_dir = root / "meson/sources"

    canonical = read_manifest(source_dir, "client_c.txt")
    hot = read_manifest(source_dir, "client_hot_c.txt")
    rest = read_manifest(source_dir, "client_rest_c.txt")

    failures = []
    report_duplicates("client_c.txt", canonical, failures)
    report_duplicates("client_hot_c.txt", hot, failures)
    report_duplicates("client_rest_c.txt", rest, failures)

    canonical_set = set(canonical)
    hot_set = set(hot)
    rest_set = set(rest)

    missing = sorted(canonical_set - (hot_set | rest_set))
    extra = sorted((hot_set | rest_set) - canonical_set)
    overlap = sorted(hot_set & rest_set)
    absent = sorted(path for path in canonical_set
                    if not (root / path).is_file())

    if missing:
        failures.append("client_c.txt entries missing from hot/rest:")
        failures.extend("  " + path for path in missing)
    if extra:
        failures.append("hot/rest entries missing from client_c.txt:")
        failures.extend("  " + path for path in extra)
    if overlap:
        failures.append("entries appear in both hot and rest:")
        failures.extend("  " + path for path in overlap)
    if absent:
        failures.append("manifest entries do not name files:")
        failures.extend("  " + path for path in absent)

    if failures:
        for line in failures:
            print("error: " + line, file=sys.stderr)
        return 1

    print("Client C manifests partition the canonical client source list")
    return 0


if __name__ == "__main__":
    sys.exit(main())
