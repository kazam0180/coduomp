#!/bin/sh

set -eu

[ -f Makefile ] && [ -d src ] || {
	echo "ERROR: run from the repository root after ./get-dependencies.sh." >&2
	exit 1
}
[ -d packaging/appimage/AppDir ] || {
	echo "ERROR: packaging/appimage/AppDir missing — run from the repository root." >&2
	exit 1
}
for staged in \
	pkg/client/opencoduo-client \
	pkg/client/uo/uo_cgame_mp_x86_64.so \
	pkg/client/uo/uo_ui_mp_x86_64.so \
	pkg/client/uo/uo_game_mp_x86_64.so \
	pkg/server/opencoduo-server \
	pkg/server/uo/game.mp.uo.x86_64.so
do
	[ -f "$staged" ] || {
		echo "ERROR: $staged missing — run ./get-dependencies.sh first." >&2
		exit 1
	}
done
command -v quick-sharun >/dev/null || {
	echo "ERROR: quick-sharun not found." >&2
	exit 1
}

ARCH=$(uname -m)
if [ -n "${GITHUB_REF_NAME:-}" ] && [ "${GITHUB_REF_NAME}" != "master" ]; then
	VERSION=$GITHUB_REF_NAME
elif [ -n "${GITHUB_RUN_NUMBER:-}" ]; then
	VERSION="continuous-${GITHUB_RUN_NUMBER}"
else
	VERSION=$(git describe --tags --always 2>/dev/null || echo continuous)
fi
export VERSION=$(printf '%s' "$VERSION" | sed 's/^v//')
export ARCH
export OUTPATH=./dist
export MAIN_BIN=opencoduo-client
export APPDIR=${PWD}/AppDir
export DEPLOY_SDL=1
export DEPLOY_OPENGL=1
export DEPLOY_VULKAN=0
export DEPLOY_PULSE=1
export ANYLINUX_LIB=1

cp -r packaging/appimage/AppDir "$APPDIR"

quick-sharun \
	"$PWD/pkg/client/opencoduo-client"                  \
	"$PWD"/pkg/client/uo/uo_cgame_mp_x86_64.so           \
	"$PWD"/pkg/client/uo/uo_ui_mp_x86_64.so             \
	"$PWD"/pkg/client/uo/uo_game_mp_x86_64.so           \
	"$PWD/pkg/server/opencoduo-server"                  \
	"$PWD"/pkg/server/uo/game.mp.uo.x86_64.so

mkdir -p "$APPDIR"/uo
for m in \
	uo_cgame_mp_x86_64.so \
	uo_ui_mp_x86_64.so \
	uo_game_mp_x86_64.so \
	game.mp.uo.x86_64.so
do
	found=$(find "$APPDIR"/lib -name "$m" -print | head -n 1)
	if [ -z "$found" ]; then
		echo "ERROR: $m was not deployed!" >&2
		exit 1
	fi
	mv -f "$found" "$APPDIR"/uo/"$m"
done
find "$APPDIR"/lib -type d -empty -delete

"$APPDIR"/sharun -g

quick-sharun --make-appimage

quick-sharun --simple-test ./dist/*.AppImage
