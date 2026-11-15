#!/bin/sh

set -eu

[ -f meson.build ] && [ -d src ] || {
	echo "ERROR: run from the repository root." >&2
	exit 1
}

echo "Installing package dependencies..."
echo "---------------------------------------------------------------"
pacman -Syu --noconfirm \
	base-devel \
	meson \
	ninja \
	minizip \
	nasm \
	openal

echo "Installing debloated packages..."
echo "---------------------------------------------------------------"
get-debloated-pkgs --add-mesa --prefer-nano libdecor-mini
make-aur-package libjpeg-turbo
make-aur-package sdl2

build_dir=.workbench/build/meson

echo "Configuring the Meson build..."
echo "---------------------------------------------------------------"
if [ -d "$build_dir" ]; then
	meson setup --reconfigure "$build_dir"
else
	meson setup "$build_dir"
fi

echo "Building softfloat..."
echo "---------------------------------------------------------------"
meson compile -C "$build_dir" softfloat

echo "Building the client and dedicated server..."
echo "---------------------------------------------------------------"
meson compile -C "$build_dir"

echo "Staging AppImage inputs..."
echo "---------------------------------------------------------------"
rm -rf pkg
mkdir -p pkg/client/uo pkg/server/uo
cp "$build_dir/CoDUOMP" pkg/client/opencoduo-client
cp "$build_dir/uo/uo_cgame_mp_x86_64.so" pkg/client/uo/
cp "$build_dir/uo/uo_ui_mp_x86_64.so" pkg/client/uo/
cp "$build_dir/uo/uo_game_mp_x86_64.so" pkg/client/uo/
cp "$build_dir/server/coduo_lnxded_recovered" pkg/server/opencoduo-server
cp "$build_dir/server/uo/game.mp.uo.x86_64.so" pkg/server/uo/
chmod +x pkg/client/opencoduo-client pkg/server/opencoduo-server pkg/client/uo/*.so pkg/server/uo/*.so
