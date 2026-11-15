#!/usr/bin/env bash

# Package a built client, deriving the commit stamp at run time.  The Meson
# build directory outlives a commit, so a configure-time stamp would label a
# later package with the commit that existed when Meson was set up.

set -eu

if test "$#" -ne 6; then
    echo "usage: $0 engine cgame ui game package-arch module-arch" >&2
    exit 2
fi

engine=$1
cgame=$2
ui=$3
game=$4
package_arch=$5
module_arch=$6

script_dir=$(CDPATH= cd "$(dirname "$0")" && pwd -P)
project_root=$(cd "$script_dir/../.." && pwd -P)

source_commit=$(git -C "$project_root" rev-parse HEAD 2>/dev/null || true)
if test -z "$source_commit"; then
    source_commit=unknown
fi

package_suffix=$(printf '%s' "$source_commit" | cut -c1-9)
output="$project_root/.workbench/build/linux/opencoduo-linux-$package_arch-$package_suffix.tar.gz"

exec "$project_root/tools/linux/package-client.sh" \
    "$engine" "$cgame" "$ui" "$game" \
    "$output" "$package_arch" "$module_arch" "$source_commit"
