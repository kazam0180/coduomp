.PHONY: native-link linux-i386-link \
	syntax-check check

ROOT := .
include build-mk/sources/ui.mk
CODUO_DEFAULT_BEHAVIOR := windows
override CODUO_BEHAVIOR := windows
include $(ROOT)/build-mk/platform-behavior.mk
CC ?= clang
WIDTH_CC ?= clang
RECOVERY_POLICY_CPPFLAGS :=
LINUX_I386_LIBRARY ?= .workbench/build/ui/linux-i386/libcoduo_ui.so
UI_DIR := $(ROOT)/src/client/ui
C_SOURCES := $(UI_C_SOURCES)
SHARED_C_SOURCES := $(ROOT)/src/qcommon/com_parse.c \
	$(ROOT)/src/qcommon/com_sprintf.c \
	$(ROOT)/src/qcommon/info.c \
	$(ROOT)/src/qcommon/q_bits.c \
	$(ROOT)/src/qcommon/q_endian.c \
	$(ROOT)/src/qcommon/q_localized_float.c \
	$(ROOT)/src/qcommon/q_path.c \
	$(ROOT)/src/qcommon/q_shared_misc.c \
	$(ROOT)/src/qcommon/q_string.c \
	$(ROOT)/src/qcommon/q_temp.c \
	$(ROOT)/src/compat/crt/atof_compat.c \
	$(ROOT)/src/compat/crt/format_compat.c \
	$(ROOT)/src/compat/crt/msvc_compat.c \
	$(ROOT)/src/compat/crt/qsort_compat.c \
	$(ROOT)/src/client/common/client_common.c \
	$(ROOT)/src/client/common/client_legacy_crt.c \
	$(ROOT)/src/client/common/localized_float.c \
	$(ROOT)/src/client/menu/ui_parse.c \
	$(ROOT)/src/client/menu/ui_bindings.c \
	$(ROOT)/src/client/menu/ui_runtime.c \
	$(ROOT)/src/client/menu/ui_geometry.c \
	$(ROOT)/src/client/menu/ui_focus.c \
	$(ROOT)/src/client/menu/ui_motion.c \
	$(ROOT)/src/client/menu/ui_capture.c \
	$(ROOT)/src/client/menu/ui_key.c \
	$(ROOT)/src/client/menu/ui_window_paint.c \
	$(ROOT)/src/client/menu/ui_menu_queries.c \
	$(ROOT)/src/client/menu/ui_menu_stack.c \
	$(ROOT)/src/client/menu/ui_menu_visibility.c \
	$(ROOT)/src/client/menu/ui_script_menu_navigation.c \
	$(ROOT)/src/client/menu/ui_script_cvar_commands.c \
	$(ROOT)/src/client/menu/ui_script_list_commands.c \
	$(ROOT)/src/client/menu/ui_script_visual_commands.c \
	$(ROOT)/src/client/menu/ui_script_dispatch.c \
	$(ROOT)/src/client/menu/ui_keyword_hash.c \
	$(ROOT)/src/client/menu/ui_keyword_tables.c \
	$(ROOT)/src/client/menu/ui_keyword_dispatch.c \
	$(ROOT)/src/client/menu/ui_content_cache.c \
	$(ROOT)/src/client/menu/ui_color.c \
	$(ROOT)/src/client/menu/ui_item_multi.c \
	$(ROOT)/src/client/menu/ui_item_yesno.c \
	$(ROOT)/src/client/menu/ui_item_slider.c \
	$(ROOT)/src/client/menu/ui_item_text_color.c \
	$(ROOT)/src/client/menu/ui_item_text_extents.c \
	$(ROOT)/src/client/menu/ui_item_text_autowrap.c \
	$(ROOT)/src/client/menu/ui_item_text_wrap.c \
	$(ROOT)/src/client/menu/ui_item_text.c \
	$(ROOT)/src/client/menu/ui_item_textfield_key.c \
	$(ROOT)/src/client/menu/ui_item_textfield_paint.c \
	$(ROOT)/src/client/menu/ui_item_enable_cvar.c \
	$(ROOT)/src/client/menu/ui_item_ownerdraw_key.c \
	$(ROOT)/src/client/menu/ui_item_ownerdraw_paint.c \
	$(ROOT)/src/client/menu/ui_item_action.c \
	$(ROOT)/src/client/menu/ui_item_asset_paint.c \
	$(ROOT)/src/client/menu/ui_item_model.c \
	$(ROOT)/src/client/menu/ui_item_listbox_geometry.c \
	$(ROOT)/src/client/menu/ui_item_listbox_key.c \
	$(ROOT)/src/client/menu/ui_item_listbox_paint.c \
	$(ROOT)/src/client/menu/ui_item_bind_key.c \
	$(ROOT)/src/client/menu/ui_item_bind_paint.c \
	$(ROOT)/src/client/menu/ui_item_paint.c \
	$(ROOT)/src/client/menu/ui_menu_paint_all.c \
	$(ROOT)/src/client/menu/ui_parse_direct_handlers.c \
	$(ROOT)/src/client/menu/ui_parse_color_range.c \
	$(ROOT)/src/client/menu/ui_item_parse_origin.c \
	$(ROOT)/src/client/menu/ui_item_parse_textfile.c \
	$(ROOT)/src/client/menu/ui_item_parse_model_animplay.c \
	$(ROOT)/src/client/menu/ui_item_parse_columns.c \
	$(ROOT)/src/client/menu/ui_item_parse_cvarstrlist.c \
	$(ROOT)/src/client/menu/ui_item_parse_cvarfloatlist.c \
	$(ROOT)/src/client/menu/ui_menu_parse_font.c \
	$(ROOT)/src/client/menu/ui_item_post_parse.c \
	$(ROOT)/src/client/menu/ui_menu_post_parse.c \
	$(ROOT)/src/client/menu/ui_memory.c \
	$(ROOT)/src/client/menu/ui_display_mouse.c \
	$(ROOT)/src/client/menu/ui_display_cursor.c \
	$(ROOT)/src/client/menu/ui_display_key.c \
	$(ROOT)/src/client/menu/ui_common_leaves.c \
	$(ROOT)/src/client/menu/ui_menu_feeder_scroll.c \
	$(ROOT)/src/client/menu/ui_menu_feeder_selection.c \
	$(ROOT)/src/client/math/client_geometry.c \
	$(ROOT)/src/client/math/client_dobj_matrix.c \
	$(ROOT)/src/client/math/client_quaternion.c \
	$(ROOT)/src/client/math/client_rotation.c \
	$(ROOT)/src/client/math/client_rounding.c \
	$(ROOT)/src/client/math/client_vector_util.c \
	$(ROOT)/src/math/q_math.c \
	$(ROOT)/src/math/box_distance.c \
	$(ROOT)/src/math/q_rint.c \
	$(ROOT)/src/math/vector_rotate_angles.c \
	$(ROOT)/src/math/float_round_nearest.c \
	$(ROOT)/src/math/round_float.c \
	$(ROOT)/src/math/lean_math.c \
	$(ROOT)/src/math/angle_vectors.c \
	$(ROOT)/src/math/angle_mod.c \
	$(ROOT)/src/math/angle_normalization.c \
	$(ROOT)/src/math/angle_subtract.c \
	$(ROOT)/src/math/radius_from_bounds.c \
	$(ROOT)/src/math/color_math.c \
	$(ROOT)/src/math/vector_normalization_geometry.c \
	$(ROOT)/src/math/direction_angles.c \
	$(ROOT)/src/math/axis_to_angles.c \
	$(ROOT)/src/math/angles_to_axis.c \
	$(ROOT)/src/math/pitch_for_yaw_on_normal.c \
	$(ROOT)/src/math/vector_snap.c \
	$(ROOT)/src/math/orientation_transform.c \
	$(ROOT)/src/math/matrix_transform_vector43_equals.c \
	$(ROOT)/src/math/vector_compare_epsilon.c \
	$(ROOT)/src/math/cross_product.c \
	$(ROOT)/src/math/quat_multiply.c \
	$(ROOT)/src/math/byte_directions.c \
	$(ROOT)/src/math/set_plane_signbits.c \
	$(ROOT)/src/math/box_on_plane_side.c \
	$(ROOT)/src/math/matrix_transpose_transform_vector.c \
	$(ROOT)/src/math/matrix_transform_vector43.c \
	$(ROOT)/src/math/local_matrix_transform_vector43.c \
	$(ROOT)/src/math/dobj_skel_matrix.c \
	$(ROOT)/src/math/matrix_inverse.c \
	$(ROOT)/src/math/matrix_inverse44.c \
	$(ROOT)/src/math/matrix_multiply.c \
	$(ROOT)/src/math/matrix_multiply43.c \
	$(ROOT)/src/math/matrix_transform_vector.c \
	$(ROOT)/src/math/vector_rotation.c \
	$(ROOT)/src/math/normal_to_latlong.c \
	$(ROOT)/src/math/vector_polar.c \
	$(ROOT)/src/math/q_random.c \
	$(ROOT)/src/math/q_float_primitives.c \
	$(ROOT)/src/math/vector_angle_multiply.c \
	$(ROOT)/src/math/axis_quaternion.c \
	$(ROOT)/src/math/convert_quat_to_mat.c \
	$(ROOT)/src/math/quaternion_trace.c \
	$(ROOT)/src/math/q_shared_random.c \
	$(ROOT)/src/math/vector_projection.c
ALL_C_SOURCES := $(C_SOURCES) $(SHARED_C_SOURCES)
# UI targets are client code and must never inherit the generic arm64
# software-x87 default through a shared translation unit's include order.
override CLIENT_FP_CPPFLAGS := -DEMULATE_X87=0
INCLUDE_FLAGS := -iquote $(UI_DIR)/bindings \
	-iquote $(ROOT)/src/qcommon/bindings/default \
	-I$(ROOT)/src

NATIVE_LIBRARY ?= .workbench/build/ui/native/libcoduo_ui.so
NATIVE_LDFLAGS := -shared -Wl,--no-undefined

# The retail MSVC target performs signed integer arithmetic in 32-bit
# registers. Preserve that defined target behavior without per-expression
# rollover adapters in recovered source.
UI_INTEGER_FLAGS := -fwrapv
UI_NATIVE_STORAGE_FLAGS := -fno-common

native-link:
	mkdir -p $(dir $(NATIVE_LIBRARY))
	$(CC) -std=c11 $(CLIENT_FP_CPPFLAGS) $(RECOVERY_POLICY_CPPFLAGS) $(PLATFORM_BEHAVIOR_CPPFLAGS) -Wall -Wextra -Werror -g -O0 -fPIC \
		$(UI_INTEGER_FLAGS) $(UI_NATIVE_STORAGE_FLAGS) -fvisibility=hidden $(INCLUDE_FLAGS) $(ALL_C_SOURCES) $(NATIVE_LDFLAGS) -lm \
		-o $(NATIVE_LIBRARY)

syntax-check:
	$(WIDTH_CC) -std=c11 $(CLIENT_FP_CPPFLAGS) $(RECOVERY_POLICY_CPPFLAGS) $(PLATFORM_BEHAVIOR_CPPFLAGS) -fsyntax-only $(UI_INTEGER_FLAGS) -Wall -Wextra -Werror \
		-Werror=implicit-function-declaration -Werror=incompatible-pointer-types \
		-Werror=int-conversion -Werror=shorten-64-to-32 \
		-Werror=pointer-to-int-cast -Werror=int-to-pointer-cast \
		-Werror=void-pointer-to-int-cast $(INCLUDE_FLAGS) $(ALL_C_SOURCES)

linux-i386-link:
	@mkdir -p $(dir $(LINUX_I386_LIBRARY))
	$(CC) -m32 -std=c11 $(CLIENT_FP_CPPFLAGS) $(RECOVERY_POLICY_CPPFLAGS) $(PLATFORM_BEHAVIOR_CPPFLAGS) $(UI_INTEGER_FLAGS) -Wall -Wextra -Werror \
		-g -O0 -fPIC -fvisibility=hidden \
		$(INCLUDE_FLAGS) $(ALL_C_SOURCES) -shared -Wl,--no-undefined -lm \
		-o $(LINUX_I386_LIBRARY)

check: syntax-check native-link
