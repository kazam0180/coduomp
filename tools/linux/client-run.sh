#!/bin/sh

# Launch a built client against a retail data root using the persistent system
# profile. Arguments are the engine binary and the build directory that holds
# the staged uo/ modules.

set -eu

if [ "$#" -ne 2 ]; then
    echo "usage: $0 engine build-directory" >&2
    exit 2
fi

engine=$1
build_dir=$2

if [ -z "${CLIENT_DATA_PATH:-}" ] || \
    [ ! -d "$CLIENT_DATA_PATH/main" ] || \
    [ ! -d "$CLIENT_DATA_PATH/uo" ]; then
    echo "CLIENT_DATA_PATH must name a retail root containing main/ and uo/" >&2
    exit 2
fi

root=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd -P)
runtime_root=$root/.workbench/runtime
work_dir=${CLIENT_WORK_DIR:-$runtime_root/client}

mkdir -p "$work_dir"
work_dir=$(CDPATH= cd -- "$work_dir" && pwd -P)
case "$work_dir" in
    "$runtime_root"/*) ;;
    *)
        echo "CLIENT_WORK_DIR must be inside $runtime_root" >&2
        exit 2
        ;;
esac

set -- "$engine" \
    +set fs_basepath "$build_dir" \
    +set fs_cdpath "$CLIENT_DATA_PATH"

if [ -n "${CLIENT_ARGS:-}" ]; then
    # Match the previous Make target: CLIENT_ARGS is split by the shell.
    set -f
    set -- "$@" $CLIENT_ARGS
    set +f
fi

cd "$work_dir"
exec "$@"
