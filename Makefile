WORKBENCH_DIR ?= .workbench
BUILD_DIR ?= $(WORKBENCH_DIR)/build
JOBS ?=
CC ?= cc
CXX ?= c++
PKG_CONFIG ?= pkg-config
LINUX32_CC ?= gcc
LINUX32_CXX ?= g++
LINUX32_PKG_CONFIG ?= env PKG_CONFIG_LIBDIR=/usr/lib/i386-linux-gnu/pkgconfig:/usr/lib/pkgconfig:/usr/share/pkgconfig pkg-config
LINUX64_CC ?= gcc
LINUX64_CXX ?= g++
LINUX64_PKG_CONFIG ?= pkg-config
AUTH ?= 0

MAKE_JOBS := $(if $(JOBS),-j $(JOBS),)
HOST_OS := $(shell uname -s)
HOST_MACHINE := $(shell uname -m)

CLIENT_NATIVE_EXT := so
ifneq ($(filter i386 i486 i586 i686,$(HOST_MACHINE)),)
CLIENT_NATIVE_ARCH := x86
else
CLIENT_NATIVE_ARCH := x86_64
endif

AUTH_DISABLE_1 := 0
AUTH_DISABLE_0 := 1
AUTH_DISABLE := $(AUTH_DISABLE_$(AUTH))
AUTH_NAME_1 := auth
AUTH_NAME_0 := noauth
AUTH_NAME := $(AUTH_NAME_$(AUTH))

ifeq ($(filter $(AUTH),0 1),)
$(error AUTH must be 1 or 0)
endif

CLIENT_NATIVE_DIR ?= $(BUILD_DIR)/client/native
CLIENT_ENGINE_OUTPUT := $(CLIENT_NATIVE_DIR)/CoDUOMP
CLIENT_BASEGAME_DIR := $(CLIENT_NATIVE_DIR)/uo
CLIENT_CGAME_OUTPUT := $(CLIENT_BASEGAME_DIR)/uo_cgame_mp_$(CLIENT_NATIVE_ARCH).$(CLIENT_NATIVE_EXT)
CLIENT_UI_OUTPUT := $(CLIENT_BASEGAME_DIR)/uo_ui_mp_$(CLIENT_NATIVE_ARCH).$(CLIENT_NATIVE_EXT)
# Client game modules select Windows behavior even on native Linux.
# Keep their objects and artifacts separate from Linux-behavior standalone
# server modules so incremental builds cannot silently reuse the wrong policy.
CLIENT_GAME_BUILD_DIR ?= $(BUILD_DIR)/game/client-native
CLIENT_GAME_OUTPUT := $(CLIENT_BASEGAME_DIR)/uo_game_mp_$(CLIENT_NATIVE_ARCH).$(CLIENT_NATIVE_EXT)
CLIENT_DATA_PATH ?=
CLIENT_WORK_DIR ?= $(CURDIR)/$(WORKBENCH_DIR)/runtime/client
CLIENT_TEST_WORK_DIR ?= $(CURDIR)/$(WORKBENCH_DIR)/runtime/client-test
CLIENT_TEST_HOME_PATH ?= $(CLIENT_TEST_WORK_DIR)/home
CLIENT_ARGS ?=
SOURCE_COMMIT ?= $(shell git rev-parse HEAD 2>/dev/null || printf unknown)
LINUX32_CLIENT_DIR ?= $(BUILD_DIR)/client/linux-i686
LINUX64_CLIENT_DIR ?= $(BUILD_DIR)/client/linux-x86_64
LINUX32_GAME_OUTPUT ?= $(BUILD_DIR)/game/client-linux-i386/uo_game_mp_x86.so
LINUX64_GAME_OUTPUT ?= $(BUILD_DIR)/game/client-linux-x86_64/uo_game_mp_x86_64.so
LINUX32_PACKAGE_OUTPUT ?= $(BUILD_DIR)/linux/opencoduo-linux-i686-$(shell printf '%s' "$(SOURCE_COMMIT)" | cut -c1-9).tar.gz
LINUX64_PACKAGE_OUTPUT ?= $(BUILD_DIR)/linux/opencoduo-linux-x86_64-$(shell printf '%s' "$(SOURCE_COMMIT)" | cut -c1-9).tar.gz

.PHONY: all policy-check client client-engine client-cgame client-ui client-game client-run client-test-run \
	client-linux32 client-linux64 client-linux32-package client-linux64-package \
	server server64 server32 server-engine64 server-engine32 \
	server-game64 server-game32 check-linux-host softfloat \
	clean help

all: client

policy-check:
	tools/check-source-policy.sh

softfloat:
	$(MAKE) -C vendor/softfloat/build/coduo-x87

client: client-engine client-cgame client-ui

client-engine: softfloat
	$(MAKE) $(MAKE_JOBS) -f build-mk/client-targets.mk \
		CC="$(CC)" CXX="$(CXX)" PKG_CONFIG="$(PKG_CONFIG)" \
		CODUOMP_NATIVE_BUILD_DIR="$(abspath $(CLIENT_NATIVE_DIR))" \
		CODUOMP_NATIVE_EXECUTABLE="$(abspath $(CLIENT_ENGINE_OUTPUT))" \
		coduomp-native-link

client-cgame: softfloat
	$(MAKE) -f build-mk/client-targets.mk \
		CC="$(CC)" \
		CLIENT_NATIVE_LIBRARY="$(abspath $(CLIENT_CGAME_OUTPUT))" \
		client-native-link

client-ui: softfloat
	$(MAKE) -f build-mk/ui.mk \
		CC="$(CC)" \
		NATIVE_LIBRARY="$(abspath $(CLIENT_UI_OUTPUT))" \
		native-link

client-game: softfloat
	$(MAKE) $(MAKE_JOBS) -f build-mk/game.mk \
		CC="$(CC)" CODUO_BEHAVIOR=windows CODUO_FP_FAITHFUL=relaxed \
		NATIVE64_SHARED_BUILD_DIR="$(abspath $(CLIENT_GAME_BUILD_DIR))" \
		NATIVE64_SHARED_TARGET="$(abspath $(CLIENT_GAME_OUTPUT))" \
		native64-shared

client-run: client
	@if test -z "$(CLIENT_DATA_PATH)" || \
	    test ! -d "$(CLIENT_DATA_PATH)/main" || \
	    test ! -d "$(CLIENT_DATA_PATH)/uo"; then \
		echo "CLIENT_DATA_PATH must name a retail root containing main/ and uo/"; \
		exit 2; \
	fi
	@case "$(abspath $(CLIENT_WORK_DIR))" in \
		"$(abspath $(WORKBENCH_DIR)/runtime)"/*) ;; \
		*) echo 'CLIENT_WORK_DIR must be inside $(abspath $(WORKBENCH_DIR)/runtime)' >&2; exit 2 ;; \
	esac
	@mkdir -p "$(CLIENT_WORK_DIR)"
	cd "$(CLIENT_WORK_DIR)" && "$(abspath $(CLIENT_ENGINE_OUTPUT))" \
		+set fs_basepath "$(abspath $(CLIENT_NATIVE_DIR))" \
		+set fs_cdpath "$(CLIENT_DATA_PATH)" $(CLIENT_ARGS)

client-test-run: client
	@if test -z "$(CLIENT_DATA_PATH)" || \
	    test ! -d "$(CLIENT_DATA_PATH)/main" || \
	    test ! -d "$(CLIENT_DATA_PATH)/uo"; then \
		echo "CLIENT_DATA_PATH must name a retail root containing main/ and uo/"; \
		exit 2; \
	fi
	@case "$(abspath $(CLIENT_TEST_WORK_DIR))" in \
		"$(abspath $(WORKBENCH_DIR)/runtime)"/*) ;; \
		*) echo 'CLIENT_TEST_WORK_DIR must be inside $(abspath $(WORKBENCH_DIR)/runtime)' >&2; exit 2 ;; \
	esac
	@case "$(abspath $(CLIENT_TEST_HOME_PATH))" in \
		"$(abspath $(WORKBENCH_DIR)/runtime)"/*) ;; \
		*) echo 'CLIENT_TEST_HOME_PATH must be inside $(abspath $(WORKBENCH_DIR)/runtime)' >&2; exit 2 ;; \
	esac
	@mkdir -p "$(CLIENT_TEST_WORK_DIR)" "$(CLIENT_TEST_HOME_PATH)"
	cd "$(CLIENT_TEST_WORK_DIR)" && "$(abspath $(CLIENT_ENGINE_OUTPUT))" \
		+set fs_basepath "$(abspath $(CLIENT_NATIVE_DIR))" \
		+set fs_cdpath "$(CLIENT_DATA_PATH)" $(CLIENT_ARGS) \
		+set fs_homepath "$(abspath $(CLIENT_TEST_HOME_PATH))"

check-linux-host:
	@if test "$(HOST_OS)" != Linux; then \
		echo 'error: Linux targets must be built on Linux' >&2; \
		exit 2; \
	fi

client-linux32: check-linux-host
	$(MAKE) client BUILD_DIR="$(BUILD_DIR)" \
		CLIENT_NATIVE_DIR="$(LINUX32_CLIENT_DIR)" \
		CLIENT_NATIVE_ARCH=x86 CLIENT_NATIVE_EXT=so \
		CC="$(LINUX32_CC) -m32 -msse" \
		CXX="$(LINUX32_CXX) -m32 -msse" \
		PKG_CONFIG="$(LINUX32_PKG_CONFIG)"
	$(MAKE) $(MAKE_JOBS) -f build-mk/game.mk \
		CC="$(LINUX32_CC)" CODUO_BEHAVIOR=windows \
		I386_SHARED_BUILD_DIR="$(abspath $(dir $(LINUX32_GAME_OUTPUT)))" \
		I386_SHARED_TARGET="$(abspath $(LINUX32_GAME_OUTPUT))" shared-i386

client-linux64:
	@if test "$(HOST_OS)" != Linux; then \
		echo 'error: client-linux64 must be built on Linux' >&2; \
		exit 2; \
	fi
	$(MAKE) client BUILD_DIR="$(BUILD_DIR)" \
		CLIENT_NATIVE_DIR="$(LINUX64_CLIENT_DIR)" \
		CLIENT_NATIVE_ARCH=x86_64 CLIENT_NATIVE_EXT=so \
		CC="$(LINUX64_CC) -m64" CXX="$(LINUX64_CXX) -m64" \
		PKG_CONFIG="$(LINUX64_PKG_CONFIG)"
	$(MAKE) $(MAKE_JOBS) -f build-mk/game.mk \
		CC="$(LINUX64_CC)" CODUO_BEHAVIOR=windows \
		NATIVE64_SHARED_BUILD_DIR="$(abspath $(dir $(LINUX64_GAME_OUTPUT)))" \
		NATIVE64_SHARED_TARGET="$(abspath $(LINUX64_GAME_OUTPUT))" native64-shared

client-linux32-package: client-linux32
	tools/linux/package-client.sh \
		"$(abspath $(LINUX32_CLIENT_DIR)/CoDUOMP)" \
		"$(abspath $(LINUX32_CLIENT_DIR)/uo/uo_cgame_mp_x86.so)" \
		"$(abspath $(LINUX32_CLIENT_DIR)/uo/uo_ui_mp_x86.so)" \
		"$(abspath $(LINUX32_GAME_OUTPUT))" \
		"$(abspath $(LINUX32_PACKAGE_OUTPUT))" \
		i686 x86 "$(SOURCE_COMMIT)"

client-linux64-package: client-linux64
	tools/linux/package-client.sh \
		"$(abspath $(LINUX64_CLIENT_DIR)/CoDUOMP)" \
		"$(abspath $(LINUX64_CLIENT_DIR)/uo/uo_cgame_mp_x86_64.so)" \
		"$(abspath $(LINUX64_CLIENT_DIR)/uo/uo_ui_mp_x86_64.so)" \
		"$(abspath $(LINUX64_GAME_OUTPUT))" \
		"$(abspath $(LINUX64_PACKAGE_OUTPUT))" \
		x86_64 x86_64 "$(SOURCE_COMMIT)"

server: server64

server64: server-engine64 server-game64

server32: server-engine32 server-game32

server-engine64: check-linux-host
	$(MAKE) $(MAKE_JOBS) -f build-mk/server.mk \
		CODUO_DISABLE_SERVER_AUTH=$(AUTH_DISABLE) \
		EXE_BUILD_DIR="$(BUILD_DIR)/server/linux-x86_64-$(AUTH_NAME)" exe

server-engine32: check-linux-host
	$(MAKE) $(MAKE_JOBS) -f build-mk/server.mk \
		CODUO_DISABLE_SERVER_AUTH=$(AUTH_DISABLE) \
		EXE_I386_BUILD_DIR="$(BUILD_DIR)/server/linux-i386-$(AUTH_NAME)" exe-i386

server-game64: check-linux-host
	$(MAKE) $(MAKE_JOBS) -f build-mk/game.mk \
		NATIVE64_SHARED_BUILD_DIR="$(BUILD_DIR)/game/linux-x86_64" \
		native64-shared

server-game32: check-linux-host
	$(MAKE) $(MAKE_JOBS) -f build-mk/game.mk \
		I386_SHARED_BUILD_DIR="$(BUILD_DIR)/game/linux-i386" shared-i386

clean:
	@case "$(abspath $(BUILD_DIR))" in \
		"$(abspath $(WORKBENCH_DIR))"/*) ;; \
		*) echo 'refusing to clean BUILD_DIR outside $(abspath $(WORKBENCH_DIR))' >&2; exit 2 ;; \
	esac
	@case "$(abspath $(CLIENT_WORK_DIR))" in \
		"$(abspath $(WORKBENCH_DIR))"/*) ;; \
		*) echo 'refusing to clean CLIENT_WORK_DIR outside $(abspath $(WORKBENCH_DIR))' >&2; exit 2 ;; \
	esac
	@case "$(abspath $(CLIENT_TEST_WORK_DIR))" in \
		"$(abspath $(WORKBENCH_DIR))"/*) ;; \
		*) echo 'refusing to clean CLIENT_TEST_WORK_DIR outside $(abspath $(WORKBENCH_DIR))' >&2; exit 2 ;; \
	esac
	@if test -d "$(BUILD_DIR)"; then rm -r "$(BUILD_DIR)"; fi
	@if test -d "$(CLIENT_WORK_DIR)"; then rm -r "$(CLIENT_WORK_DIR)"; fi
	@if test -d "$(CLIENT_TEST_WORK_DIR)"; then rm -r "$(CLIENT_TEST_WORK_DIR)"; fi

help:
	@printf '%s\n' \
	  'Client targets:' \
	  '  client (default) client-run client-test-run' \
	  '  client-linux32 client-linux64' \
	  '  client-linux32-package client-linux64-package' \
	  '  client-engine client-cgame client-ui client-game' \
	  'Server targets:' \
	  '  server server64 server32' \
	  '  server-engine64 server-engine32 server-game64 server-game32' \
	  'Options:' \
	  '  WORKBENCH_DIR=path BUILD_DIR=path JOBS=N AUTH=1|0' \
	  '  Server CD-key authorization is disabled by default; AUTH=1 enables it.' \
	  '  CODUO_FP_FAITHFUL=auto|strict|relaxed' \
	  '  CODUOMP_RENDERER_GPU_PROFILE=1 builds asynchronous renderer GPU timings' \
	  '  CLIENT_DATA_PATH=/path/to/retail/root CLIENT_ARGS=...' \
	  '  CLIENT_TEST_WORK_DIR=path CLIENT_TEST_HOME_PATH=path'
