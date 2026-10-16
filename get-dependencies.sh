#!/bin/sh

set -eu

[ -f Makefile ] && [ -d src ] || {
	echo "ERROR: run from the repository root." >&2
	exit 1
}

echo "Installing package dependencies..."
echo "---------------------------------------------------------------"
pacman -Syu --noconfirm \
	base-devel \
	minizip \
	nasm \
	openal

echo "Installing debloated packages..."
echo "---------------------------------------------------------------"
get-debloated-pkgs --add-mesa --prefer-nano libdecor-mini
make-aur-package libjpeg6-turbo
make-aur-package sdl2

echo "Building softfloat..."
echo "---------------------------------------------------------------"
make softfloat

echo "Building the client and dedicated server..."
echo "---------------------------------------------------------------"
make -j"$(nproc)" client-linux64 server64

echo "Staging AppImage inputs..."
echo "---------------------------------------------------------------"
rm -rf pkg
mkdir -p pkg/client/uo pkg/server/uo
cp .workbench/build/client/linux-x86_64/CoDUOMP pkg/client/opencoduo-client
cp .workbench/build/client/linux-x86_64/uo/uo_cgame_mp_x86_64.so pkg/client/uo/
cp .workbench/build/client/linux-x86_64/uo/uo_ui_mp_x86_64.so pkg/client/uo/
cp .workbench/build/game/client-linux-x86_64/uo_game_mp_x86_64.so pkg/client/uo/
cp .workbench/build/server/linux-x86_64-noauth/coduo_lnxded_recovered pkg/server/opencoduo-server
cp .workbench/build/game/linux-x86_64/game.mp.uo.x86_64.so pkg/server/uo/
chmod +x pkg/client/opencoduo-client pkg/server/opencoduo-server pkg/client/uo/*.so pkg/server/uo/*.so
