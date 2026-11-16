#ifndef CODUOMP_RENDERER_GL_DISPATCH_TYPES_H
#define CODUOMP_RENDERER_GL_DISPATCH_TYPES_H

#include "compat/coduo_compiler.h"
#include "gl_sdk.h"

#if defined(_WIN32)
#define RENDERER_GL_API_CALL APIENTRY
#else
#define RENDERER_GL_API_CALL
#endif

/* The SDK owns parameter types, return types and calling conventions. Taking
 * a function's type does not link it: QGL still resolves every slot at runtime.
 * Apple's SDK omits several legacy ATI/NV/EXT interfaces; only those aliases
 * retain explicit declarations, checked against the Khronos extension ABI. */
typedef CODUO_FUNCTION_POINTER_TYPE(glAlphaFunc) renderer_gl_alpha_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glBegin) renderer_gl_begin_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glBindTexture) renderer_gl_bind_texture_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glBindProgramARB) renderer_gl_bind_program_arb_func_t;
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL *renderer_gl_bind_fragment_shader_ati_func_t)(
    uint32_t shader);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glBindFragmentShaderATI) renderer_gl_bind_fragment_shader_ati_func_t;
#endif
typedef CODUO_FUNCTION_POINTER_TYPE(glBindBufferARB) renderer_gl_bind_buffer_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glBufferDataARB) renderer_gl_buffer_data_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMapBufferARB) renderer_gl_map_buffer_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glUnmapBufferARB) renderer_gl_unmap_buffer_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glBlendFunc) renderer_gl_blend_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glCallList) renderer_gl_call_list_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glClear) renderer_gl_clear_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glClearDepth) renderer_gl_clear_depth_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glClearStencil) renderer_gl_clear_stencil_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4f) renderer_gl_color4f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4fv) renderer_gl_color4fv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColorPointer) renderer_gl_color_pointer_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glCullFace) renderer_gl_cull_face_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glDepthFunc) renderer_gl_depth_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glDepthMask) renderer_gl_depth_mask_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glDepthRange) renderer_gl_depth_range_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glDisable) renderer_gl_capability_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glDrawElements) renderer_gl_draw_elements_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glDrawRangeElementsEXT) renderer_gl_draw_range_elements_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glDrawBuffer) renderer_gl_draw_buffer_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glHint) renderer_gl_hint_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glFogf) renderer_gl_fogf_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glFogfv) renderer_gl_fogfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glFogi) renderer_gl_fogi_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPolygonMode) renderer_gl_polygon_mode_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glScissor) renderer_gl_scissor_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoordPointer) renderer_gl_tex_coord_pointer_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexEnvf) renderer_gl_tex_envf_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexEnvi) renderer_gl_tex_envi_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexParameterf) renderer_gl_tex_parameterf_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexParameteri) renderer_gl_tex_parameteri_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glNormalPointer) renderer_gl_normal_pointer_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexPointer) renderer_gl_vertex_pointer_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glViewport) renderer_gl_viewport_func_t;
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL *renderer_gl_set_fence_nv_func_t)(
    uint32_t fence, uint32_t condition);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glSetFenceNV) renderer_gl_set_fence_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL *renderer_gl_finish_fence_nv_func_t)(
    uint32_t fence);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glFinishFenceNV) renderer_gl_finish_fence_nv_func_t;
#endif
typedef CODUO_FUNCTION_POINTER_TYPE(glActiveTextureARB) renderer_gl_active_texture_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMultiTexCoord2fARB) renderer_gl_multi_tex_coord2f_arb_func_t;
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL *renderer_gl_lock_arrays_ext_func_t)(
    int32_t first, int32_t count);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glLockArraysEXT) renderer_gl_lock_arrays_ext_func_t;
#endif
typedef CODUO_FUNCTION_POINTER_TYPE(glEnd) renderer_gl_void_func_t;
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL *renderer_gl_pn_trianglesi_ati_func_t)(
    uint32_t parameter, int32_t value);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glPNTrianglesiATI) renderer_gl_pn_trianglesi_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL *renderer_gl_pn_trianglesf_ati_func_t)(
    uint32_t parameter, float value);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glPNTrianglesfATI) renderer_gl_pn_trianglesf_ati_func_t;
#endif
typedef CODUO_FUNCTION_POINTER_TYPE(glCompressedTexImage3DARB) renderer_gl_compressed_tex_image_3d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glCompressedTexImage2DARB) renderer_gl_compressed_tex_image_2d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glCompressedTexImage1DARB) renderer_gl_compressed_tex_image_1d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glCompressedTexSubImage3DARB) renderer_gl_compressed_tex_sub_image_3d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glCompressedTexSubImage2DARB) renderer_gl_compressed_tex_sub_image_2d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glCompressedTexSubImage1DARB) renderer_gl_compressed_tex_sub_image_1d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetCompressedTexImageARB) renderer_gl_get_compressed_tex_image_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glDeleteBuffersARB) renderer_gl_delete_buffers_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGenBuffersARB) renderer_gl_gen_buffers_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIsBufferARB) renderer_gl_is_buffer_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glBufferSubDataARB) renderer_gl_buffer_sub_data_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetBufferSubDataARB) renderer_gl_get_buffer_sub_data_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetBufferParameterivARB) renderer_gl_get_buffer_parameteriv_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetBufferPointervARB) renderer_gl_get_buffer_pointerv_arb_func_t;
#if defined(__APPLE__)
typedef uint32_t (RENDERER_GL_API_CALL
                  *renderer_gl_new_object_buffer_ati_func_t)(
    int32_t size, const void *data, uint32_t usage);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glNewObjectBufferATI) renderer_gl_new_object_buffer_ati_func_t;
#endif
#if defined(__APPLE__)
typedef uint8_t (RENDERER_GL_API_CALL
                 *renderer_gl_is_object_buffer_ati_func_t)(uint32_t buffer);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glIsObjectBufferATI) renderer_gl_is_object_buffer_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_update_object_buffer_ati_func_t)(
    uint32_t buffer, uint32_t offset, int32_t size, const void *data,
    uint32_t preserveMode);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glUpdateObjectBufferATI) renderer_gl_update_object_buffer_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_get_object_bufferfv_ati_func_t)(
    uint32_t buffer, uint32_t parameter, float *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGetObjectBufferfvATI) renderer_gl_get_object_bufferfv_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_get_object_bufferiv_ati_func_t)(
    uint32_t buffer, uint32_t parameter, int32_t *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGetObjectBufferivATI) renderer_gl_get_object_bufferiv_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL *renderer_gl_free_object_buffer_ati_func_t)(
    uint32_t buffer);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glFreeObjectBufferATI) renderer_gl_free_object_buffer_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL *renderer_gl_array_object_ati_func_t)(
    uint32_t array, int32_t size, uint32_t type, int32_t stride,
    uint32_t buffer, uint32_t offset);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glArrayObjectATI) renderer_gl_array_object_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_get_array_objectfv_ati_func_t)(
    uint32_t array, uint32_t parameter, float *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGetArrayObjectfvATI) renderer_gl_get_array_objectfv_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_get_array_objectiv_ati_func_t)(
    uint32_t array, uint32_t parameter, int32_t *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGetArrayObjectivATI) renderer_gl_get_array_objectiv_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_variant_array_object_ati_func_t)(
    uint32_t id, uint32_t type, int32_t stride, uint32_t buffer,
    uint32_t offset);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glVariantArrayObjectATI) renderer_gl_variant_array_object_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_get_variant_array_objectfv_ati_func_t)(
    uint32_t id, uint32_t parameter, float *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGetVariantArrayObjectfvATI) renderer_gl_get_variant_array_objectfv_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_get_variant_array_objectiv_ati_func_t)(
    uint32_t id, uint32_t parameter, int32_t *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGetVariantArrayObjectivATI) renderer_gl_get_variant_array_objectiv_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL *renderer_gl_element_pointer_ati_func_t)(
    uint32_t type, const void *pointer);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glElementPointerATI) renderer_gl_element_pointer_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_draw_element_array_ati_func_t)(
    uint32_t mode, int32_t count);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glDrawElementArrayATI) renderer_gl_draw_element_array_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_draw_range_element_array_ati_func_t)(
    uint32_t mode, uint32_t start, uint32_t end, int32_t count);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glDrawRangeElementArrayATI) renderer_gl_draw_range_element_array_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL *renderer_gl_vertex_array_range_nv_func_t)(
    int32_t length, const void *pointer);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexArrayRangeNV) renderer_gl_vertex_array_range_nv_func_t;
#endif

/* These two GL dispatch slots resolve WGL_NV_vertex_array_range exports. */
#if defined(_WIN32)
typedef PFNWGLALLOCATEMEMORYNVPROC renderer_gl_allocate_memory_nv_func_t;
#else
typedef void *(RENDERER_GL_API_CALL
               *renderer_gl_allocate_memory_nv_func_t)(
    int32_t size, float readFrequency, float writeFrequency, float priority);
#endif
#if defined(_WIN32)
typedef PFNWGLFREEMEMORYNVPROC renderer_gl_free_memory_nv_func_t;
#else
typedef void (RENDERER_GL_API_CALL *renderer_gl_free_memory_nv_func_t)(
    void *memory);
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL *renderer_gl_delete_fences_nv_func_t)(
    int32_t count, const uint32_t *fences);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glDeleteFencesNV) renderer_gl_delete_fences_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL *renderer_gl_gen_fences_nv_func_t)(
    int32_t count, uint32_t *fences);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGenFencesNV) renderer_gl_gen_fences_nv_func_t;
#endif
#if defined(__APPLE__)
typedef uint8_t (RENDERER_GL_API_CALL *renderer_gl_fence_test_nv_func_t)(
    uint32_t fence);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glIsFenceNV) renderer_gl_fence_test_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL *renderer_gl_get_fenceiv_nv_func_t)(
    uint32_t fence, uint32_t parameter, int32_t *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGetFenceivNV) renderer_gl_get_fenceiv_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_combiner_parameterfv_nv_func_t)(
    uint32_t parameter, const float *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glCombinerParameterfvNV) renderer_gl_combiner_parameterfv_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_combiner_parameterf_nv_func_t)(
    uint32_t parameter, float value);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glCombinerParameterfNV) renderer_gl_combiner_parameterf_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_combiner_parameteriv_nv_func_t)(
    uint32_t parameter, const int32_t *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glCombinerParameterivNV) renderer_gl_combiner_parameteriv_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_combiner_parameteri_nv_func_t)(
    uint32_t parameter, int32_t value);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glCombinerParameteriNV) renderer_gl_combiner_parameteri_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL *renderer_gl_combiner_input_nv_func_t)(
    uint32_t stage, uint32_t portion, uint32_t variable, uint32_t input,
    uint32_t mapping, uint32_t componentUsage);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glCombinerInputNV) renderer_gl_combiner_input_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL *renderer_gl_combiner_output_nv_func_t)(
    uint32_t stage, uint32_t portion, uint32_t abOutput, uint32_t cdOutput,
    uint32_t sumOutput, uint32_t scale, uint32_t bias,
    uint8_t abDotProduct, uint8_t cdDotProduct, uint8_t muxSum);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glCombinerOutputNV) renderer_gl_combiner_output_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_final_combiner_input_nv_func_t)(
    uint32_t variable, uint32_t input, uint32_t mapping,
    uint32_t componentUsage);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glFinalCombinerInputNV) renderer_gl_final_combiner_input_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_get_combiner_input_parameterfv_nv_func_t)(
    uint32_t stage, uint32_t portion, uint32_t variable, uint32_t parameter,
    float *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGetCombinerInputParameterfvNV) renderer_gl_get_combiner_input_parameterfv_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_get_combiner_input_parameteriv_nv_func_t)(
    uint32_t stage, uint32_t portion, uint32_t variable, uint32_t parameter,
    int32_t *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGetCombinerInputParameterivNV) renderer_gl_get_combiner_input_parameteriv_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_get_combiner_output_parameterfv_nv_func_t)(
    uint32_t stage, uint32_t portion, uint32_t parameter, float *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGetCombinerOutputParameterfvNV) renderer_gl_get_combiner_output_parameterfv_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_get_combiner_output_parameteriv_nv_func_t)(
    uint32_t stage, uint32_t portion, uint32_t parameter, int32_t *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGetCombinerOutputParameterivNV) renderer_gl_get_combiner_output_parameteriv_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_get_final_combiner_input_parameterfv_nv_func_t)(
    uint32_t variable, uint32_t parameter, float *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGetFinalCombinerInputParameterfvNV) renderer_gl_get_final_combiner_input_parameterfv_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_get_final_combiner_input_parameteriv_nv_func_t)(
    uint32_t variable, uint32_t parameter, int32_t *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGetFinalCombinerInputParameterivNV) renderer_gl_get_final_combiner_input_parameteriv_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_combiner_stage_parameterfv_nv_func_t)(
    uint32_t stage, uint32_t parameter, const float *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glCombinerStageParameterfvNV) renderer_gl_combiner_stage_parameterfv_nv_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_get_combiner_stage_parameterfv_nv_func_t)(
    uint32_t stage, uint32_t parameter, float *values);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGetCombinerStageParameterfvNV) renderer_gl_get_combiner_stage_parameterfv_nv_func_t;
#endif
#if defined(__APPLE__)
typedef uint32_t (RENDERER_GL_API_CALL
                  *renderer_gl_gen_fragment_shaders_ati_func_t)(
    uint32_t range);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glGenFragmentShadersATI) renderer_gl_gen_fragment_shaders_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_fragment_shader_texcoord_ati_func_t)(
    uint32_t destination, uint32_t coordinate, uint32_t swizzle);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glPassTexCoordATI) renderer_gl_fragment_shader_texcoord_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_color_fragment_op1_ati_func_t)(
    uint32_t operation, uint32_t destination, uint32_t destinationMask,
    uint32_t destinationModifier, uint32_t argument1,
    uint32_t argument1Replication, uint32_t argument1Modifier);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glColorFragmentOp1ATI) renderer_gl_color_fragment_op1_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_color_fragment_op2_ati_func_t)(
    uint32_t operation, uint32_t destination, uint32_t destinationMask,
    uint32_t destinationModifier, uint32_t argument1,
    uint32_t argument1Replication, uint32_t argument1Modifier,
    uint32_t argument2, uint32_t argument2Replication,
    uint32_t argument2Modifier);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glColorFragmentOp2ATI) renderer_gl_color_fragment_op2_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_color_fragment_op3_ati_func_t)(
    uint32_t operation, uint32_t destination, uint32_t destinationMask,
    uint32_t destinationModifier, uint32_t argument1,
    uint32_t argument1Replication, uint32_t argument1Modifier,
    uint32_t argument2, uint32_t argument2Replication,
    uint32_t argument2Modifier, uint32_t argument3,
    uint32_t argument3Replication, uint32_t argument3Modifier);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glColorFragmentOp3ATI) renderer_gl_color_fragment_op3_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_alpha_fragment_op1_ati_func_t)(
    uint32_t operation, uint32_t destination, uint32_t destinationModifier,
    uint32_t argument1, uint32_t argument1Replication,
    uint32_t argument1Modifier);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glAlphaFragmentOp1ATI) renderer_gl_alpha_fragment_op1_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_alpha_fragment_op2_ati_func_t)(
    uint32_t operation, uint32_t destination, uint32_t destinationModifier,
    uint32_t argument1, uint32_t argument1Replication,
    uint32_t argument1Modifier, uint32_t argument2,
    uint32_t argument2Replication, uint32_t argument2Modifier);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glAlphaFragmentOp2ATI) renderer_gl_alpha_fragment_op2_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_alpha_fragment_op3_ati_func_t)(
    uint32_t operation, uint32_t destination, uint32_t destinationModifier,
    uint32_t argument1, uint32_t argument1Replication,
    uint32_t argument1Modifier, uint32_t argument2,
    uint32_t argument2Replication, uint32_t argument2Modifier,
    uint32_t argument3, uint32_t argument3Replication,
    uint32_t argument3Modifier);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glAlphaFragmentOp3ATI) renderer_gl_alpha_fragment_op3_ati_func_t;
#endif
#if defined(__APPLE__)
typedef void (RENDERER_GL_API_CALL
              *renderer_gl_set_fragment_shader_constant_ati_func_t)(
    uint32_t destination, const float *value);
#else
typedef CODUO_FUNCTION_POINTER_TYPE(glSetFragmentShaderConstantATI) renderer_gl_set_fragment_shader_constant_ati_func_t;
#endif
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib1sARB) renderer_gl_vertex_attrib1s_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib1fARB) renderer_gl_vertex_attrib1f_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib1dARB) renderer_gl_vertex_attrib1d_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib2sARB) renderer_gl_vertex_attrib2s_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib2fARB) renderer_gl_vertex_attrib2f_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib2dARB) renderer_gl_vertex_attrib2d_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib3sARB) renderer_gl_vertex_attrib3s_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib3fARB) renderer_gl_vertex_attrib3f_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib3dARB) renderer_gl_vertex_attrib3d_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib4sARB) renderer_gl_vertex_attrib4s_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib4fARB) renderer_gl_vertex_attrib4f_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib4dARB) renderer_gl_vertex_attrib4d_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib4NubARB) renderer_gl_vertex_attrib4nub_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib4bvARB) renderer_gl_vertex_attrib_i8v_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib1svARB) renderer_gl_vertex_attrib_i16v_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib4ivARB) renderer_gl_vertex_attrib_i32v_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib4ubvARB) renderer_gl_vertex_attrib_u8v_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib4usvARB) renderer_gl_vertex_attrib_u16v_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib4uivARB) renderer_gl_vertex_attrib_u32v_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib1fvARB) renderer_gl_vertex_attrib_fv_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttrib1dvARB) renderer_gl_vertex_attrib_dv_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertexAttribPointerARB) renderer_gl_vertex_attrib_pointer_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEnableVertexAttribArrayARB) renderer_gl_vertex_attrib_array_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glProgramStringARB) renderer_gl_program_string_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glDeleteProgramsARB) renderer_gl_delete_programs_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGenProgramsARB) renderer_gl_gen_programs_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glProgramEnvParameter4fARB) renderer_gl_program_parameter4f_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glProgramEnvParameter4dARB) renderer_gl_program_parameter4d_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glProgramEnvParameter4fvARB) renderer_gl_program_parameterfv_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glProgramEnvParameter4dvARB) renderer_gl_program_parameterdv_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetProgramEnvParameterfvARB) renderer_gl_get_program_parameterfv_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetProgramEnvParameterdvARB) renderer_gl_get_program_parameterdv_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetProgramivARB) renderer_gl_get_programiv_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetProgramStringARB) renderer_gl_get_program_string_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetVertexAttribdvARB) renderer_gl_get_vertex_attribdv_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetVertexAttribfvARB) renderer_gl_get_vertex_attribfv_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetVertexAttribivARB) renderer_gl_get_vertex_attribiv_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetVertexAttribPointervARB) renderer_gl_get_vertex_attrib_pointerv_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIsProgramARB) renderer_gl_is_program_arb_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glAccum) renderer_gl_accum_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glAreTexturesResident) renderer_gl_are_textures_resident_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glArrayElement) renderer_gl_array_element_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glBitmap) renderer_gl_bitmap_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glCallLists) renderer_gl_call_lists_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glClearAccum) renderer_gl_clear_color4_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glClearIndex) renderer_gl_clear_index_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glClipPlane) renderer_gl_clip_plane_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3b) renderer_gl_color3b_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3bv) renderer_gl_color3bv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3d) renderer_gl_color3d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3dv) renderer_gl_color3dv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3f) renderer_gl_color3f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3fv) renderer_gl_color3fv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3i) renderer_gl_color3i_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3iv) renderer_gl_color3iv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3s) renderer_gl_color3s_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3sv) renderer_gl_color3sv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3ub) renderer_gl_color3ub_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3ubv) renderer_gl_color3ubv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3ui) renderer_gl_color3ui_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3uiv) renderer_gl_color3uiv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3us) renderer_gl_color3us_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor3usv) renderer_gl_color3usv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4b) renderer_gl_color4b_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4bv) renderer_gl_color4bv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4d) renderer_gl_color4d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4dv) renderer_gl_color4dv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4i) renderer_gl_color4i_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4iv) renderer_gl_color4iv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4s) renderer_gl_color4s_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4sv) renderer_gl_color4sv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4ub) renderer_gl_color4ub_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4ubv) renderer_gl_color4ubv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4ui) renderer_gl_color4ui_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4uiv) renderer_gl_color4uiv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4us) renderer_gl_color4us_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColor4usv) renderer_gl_color4usv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColorMask) renderer_gl_color_mask_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glColorMaterial) renderer_gl_color_material_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glCopyPixels) renderer_gl_copy_pixels_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glCopyTexImage1D) renderer_gl_copy_tex_image_1d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glCopyTexImage2D) renderer_gl_copy_tex_image_2d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glCopyTexSubImage1D) renderer_gl_copy_tex_sub_image_1d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glCopyTexSubImage2D) renderer_gl_copy_tex_sub_image_2d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glDeleteLists) renderer_gl_delete_lists_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glDeleteTextures) renderer_gl_delete_textures_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glDrawArrays) renderer_gl_draw_arrays_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glDrawPixels) renderer_gl_draw_pixels_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEdgeFlag) renderer_gl_edge_flag_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEdgeFlagPointer) renderer_gl_edge_flag_pointer_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEdgeFlagv) renderer_gl_edge_flagv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEvalCoord1d) renderer_gl_eval_coord1d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEvalCoord1dv) renderer_gl_eval_coord1dv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEvalCoord1f) renderer_gl_eval_coord1f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEvalCoord1fv) renderer_gl_eval_coord1fv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEvalCoord2d) renderer_gl_eval_coord2d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEvalCoord2dv) renderer_gl_eval_coord2dv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEvalCoord2f) renderer_gl_eval_coord2f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEvalCoord2fv) renderer_gl_eval_coord2fv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEvalMesh1) renderer_gl_eval_mesh1_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEvalMesh2) renderer_gl_eval_mesh2_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEvalPoint1) renderer_gl_eval_point1_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glEvalPoint2) renderer_gl_eval_point2_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glFeedbackBuffer) renderer_gl_feedback_buffer_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glFogiv) renderer_gl_fogiv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glFrontFace) renderer_gl_front_face_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glFrustum) renderer_gl_frustum_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGenLists) renderer_gl_gen_lists_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGenTextures) renderer_gl_gen_textures_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetBooleanv) renderer_gl_get_booleanv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetClipPlane) renderer_gl_get_clip_plane_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetDoublev) renderer_gl_get_doublev_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetError) renderer_gl_get_error_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetFloatv) renderer_gl_get_floatv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetIntegerv) renderer_gl_get_integerv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetLightfv) renderer_gl_get_lightfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetLightiv) renderer_gl_get_lightiv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetMapdv) renderer_gl_get_mapdv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetMapfv) renderer_gl_get_mapfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetMapiv) renderer_gl_get_mapiv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetMaterialfv) renderer_gl_get_materialfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetMaterialiv) renderer_gl_get_materialiv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetPixelMapfv) renderer_gl_get_pixel_mapfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetPixelMapuiv) renderer_gl_get_pixel_mapuiv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetPixelMapusv) renderer_gl_get_pixel_mapusv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetPointerv) renderer_gl_get_pointerv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetPolygonStipple) renderer_gl_get_polygon_stipple_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetString) renderer_gl_get_string_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetTexEnvfv) renderer_gl_get_tex_envfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetTexEnviv) renderer_gl_get_tex_enviv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetTexGendv) renderer_gl_get_tex_gendv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetTexGenfv) renderer_gl_get_tex_genfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetTexGeniv) renderer_gl_get_tex_geniv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetTexImage) renderer_gl_get_tex_image_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetTexLevelParameterfv) renderer_gl_get_tex_level_parameterfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetTexLevelParameteriv) renderer_gl_get_tex_level_parameteriv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetTexParameterfv) renderer_gl_get_tex_parameterfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glGetTexParameteriv) renderer_gl_get_tex_parameteriv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIndexMask) renderer_gl_index_mask_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIndexPointer) renderer_gl_index_pointer_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIndexd) renderer_gl_indexd_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIndexdv) renderer_gl_indexdv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIndexf) renderer_gl_indexf_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIndexfv) renderer_gl_indexfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIndexi) renderer_gl_indexi_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIndexiv) renderer_gl_indexiv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIndexs) renderer_gl_indexs_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIndexsv) renderer_gl_indexsv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIndexub) renderer_gl_indexub_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIndexubv) renderer_gl_indexubv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glInterleavedArrays) renderer_gl_interleaved_arrays_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIsEnabled) renderer_gl_is_enabled_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIsList) renderer_gl_is_list_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glIsTexture) renderer_gl_is_texture_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glLightModelf) renderer_gl_light_modelf_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glLightModelfv) renderer_gl_light_modelfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glLightModeli) renderer_gl_light_modeli_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glLightModeliv) renderer_gl_light_modeliv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glLightf) renderer_gl_lightf_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glLightfv) renderer_gl_lightfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glLighti) renderer_gl_lighti_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glLightiv) renderer_gl_lightiv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glLineStipple) renderer_gl_line_stipple_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glLineWidth) renderer_gl_line_width_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glListBase) renderer_gl_list_base_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glLoadMatrixd) renderer_gl_load_matrixd_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glLoadMatrixf) renderer_gl_load_matrixf_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glLoadName) renderer_gl_load_name_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glLogicOp) renderer_gl_logic_op_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMap1d) renderer_gl_map1d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMap1f) renderer_gl_map1f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMap2d) renderer_gl_map2d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMap2f) renderer_gl_map2f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMapGrid1d) renderer_gl_map_grid1d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMapGrid1f) renderer_gl_map_grid1f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMapGrid2d) renderer_gl_map_grid2d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMapGrid2f) renderer_gl_map_grid2f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMaterialf) renderer_gl_materialf_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMaterialfv) renderer_gl_materialfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMateriali) renderer_gl_materiali_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMaterialiv) renderer_gl_materialiv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMatrixMode) renderer_gl_matrix_mode_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMultMatrixd) renderer_gl_mult_matrixd_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glMultMatrixf) renderer_gl_mult_matrixf_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glNewList) renderer_gl_new_list_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glNormal3b) renderer_gl_normal3b_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glNormal3bv) renderer_gl_normal3bv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glNormal3d) renderer_gl_normal3d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glNormal3dv) renderer_gl_normal3dv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glNormal3f) renderer_gl_normal3f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glNormal3fv) renderer_gl_normal3fv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glNormal3i) renderer_gl_normal3i_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glNormal3iv) renderer_gl_normal3iv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glNormal3s) renderer_gl_normal3s_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glNormal3sv) renderer_gl_normal3sv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glOrtho) renderer_gl_ortho_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPassThrough) renderer_gl_pass_through_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPixelMapfv) renderer_gl_pixel_mapfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPixelMapuiv) renderer_gl_pixel_mapuiv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPixelMapusv) renderer_gl_pixel_mapusv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPixelStoref) renderer_gl_pixel_storef_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPixelStorei) renderer_gl_pixel_storei_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPixelTransferf) renderer_gl_pixel_transferf_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPixelTransferi) renderer_gl_pixel_transferi_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPixelZoom) renderer_gl_pixel_zoom_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPointSize) renderer_gl_point_size_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPolygonOffset) renderer_gl_polygon_offset_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPolygonStipple) renderer_gl_polygon_stipple_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPrioritizeTextures) renderer_gl_prioritize_textures_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPushAttrib) renderer_gl_attrib_mask_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glPushName) renderer_gl_push_name_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos2d) renderer_gl_raster_pos2d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos2dv) renderer_gl_raster_pos2dv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos2f) renderer_gl_raster_pos2f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos2fv) renderer_gl_raster_pos2fv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos2i) renderer_gl_raster_pos2i_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos2iv) renderer_gl_raster_pos2iv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos2s) renderer_gl_raster_pos2s_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos2sv) renderer_gl_raster_pos2sv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos3d) renderer_gl_raster_pos3d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos3dv) renderer_gl_raster_pos3dv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos3f) renderer_gl_raster_pos3f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos3fv) renderer_gl_raster_pos3fv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos3i) renderer_gl_raster_pos3i_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos3iv) renderer_gl_raster_pos3iv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos3s) renderer_gl_raster_pos3s_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos3sv) renderer_gl_raster_pos3sv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos4d) renderer_gl_raster_pos4d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos4dv) renderer_gl_raster_pos4dv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos4f) renderer_gl_raster_pos4f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos4fv) renderer_gl_raster_pos4fv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos4i) renderer_gl_raster_pos4i_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos4iv) renderer_gl_raster_pos4iv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos4s) renderer_gl_raster_pos4s_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRasterPos4sv) renderer_gl_raster_pos4sv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glReadBuffer) renderer_gl_read_buffer_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glReadPixels) renderer_gl_read_pixels_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRectd) renderer_gl_rectd_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRectdv) renderer_gl_rectdv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRectf) renderer_gl_rectf_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRectfv) renderer_gl_rectfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRecti) renderer_gl_recti_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRectiv) renderer_gl_rectiv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRects) renderer_gl_rects_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRectsv) renderer_gl_rectsv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRenderMode) renderer_gl_render_mode_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRotated) renderer_gl_rotated_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glRotatef) renderer_gl_rotatef_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glScaled) renderer_gl_scaled_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glScalef) renderer_gl_scalef_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glSelectBuffer) renderer_gl_select_buffer_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glShadeModel) renderer_gl_shade_model_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glStencilFunc) renderer_gl_stencil_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glStencilMask) renderer_gl_stencil_mask_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glStencilOp) renderer_gl_stencil_op_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord1d) renderer_gl_tex_coord1d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord1dv) renderer_gl_tex_coord1dv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord1f) renderer_gl_tex_coord1f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord1fv) renderer_gl_tex_coord1fv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord1i) renderer_gl_tex_coord1i_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord1iv) renderer_gl_tex_coord1iv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord1s) renderer_gl_tex_coord1s_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord1sv) renderer_gl_tex_coord1sv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord2d) renderer_gl_tex_coord2d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord2dv) renderer_gl_tex_coord2dv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord2f) renderer_gl_tex_coord2f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord2fv) renderer_gl_tex_coord2fv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord2i) renderer_gl_tex_coord2i_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord2iv) renderer_gl_tex_coord2iv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord2s) renderer_gl_tex_coord2s_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord2sv) renderer_gl_tex_coord2sv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord3d) renderer_gl_tex_coord3d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord3dv) renderer_gl_tex_coord3dv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord3f) renderer_gl_tex_coord3f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord3fv) renderer_gl_tex_coord3fv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord3i) renderer_gl_tex_coord3i_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord3iv) renderer_gl_tex_coord3iv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord3s) renderer_gl_tex_coord3s_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord3sv) renderer_gl_tex_coord3sv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord4d) renderer_gl_tex_coord4d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord4dv) renderer_gl_tex_coord4dv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord4f) renderer_gl_tex_coord4f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord4fv) renderer_gl_tex_coord4fv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord4i) renderer_gl_tex_coord4i_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord4iv) renderer_gl_tex_coord4iv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord4s) renderer_gl_tex_coord4s_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexCoord4sv) renderer_gl_tex_coord4sv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexEnvfv) renderer_gl_tex_envfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexEnviv) renderer_gl_tex_enviv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexGend) renderer_gl_tex_gend_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexGendv) renderer_gl_tex_gendv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexGenf) renderer_gl_tex_genf_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexGenfv) renderer_gl_tex_genfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexGeni) renderer_gl_tex_geni_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexGeniv) renderer_gl_tex_geniv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexImage1D) renderer_gl_tex_image_1d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexImage2D) renderer_gl_tex_image_2d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexParameterfv) renderer_gl_tex_parameterfv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexParameteriv) renderer_gl_tex_parameteriv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexSubImage1D) renderer_gl_tex_sub_image_1d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTexSubImage2D) renderer_gl_tex_sub_image_2d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTranslated) renderer_gl_translated_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glTranslatef) renderer_gl_translatef_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex2d) renderer_gl_vertex2d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex2dv) renderer_gl_vertex2dv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex2f) renderer_gl_vertex2f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex2fv) renderer_gl_vertex2fv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex2i) renderer_gl_vertex2i_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex2iv) renderer_gl_vertex2iv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex2s) renderer_gl_vertex2s_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex2sv) renderer_gl_vertex2sv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex3d) renderer_gl_vertex3d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex3dv) renderer_gl_vertex3dv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex3f) renderer_gl_vertex3f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex3fv) renderer_gl_vertex3fv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex3i) renderer_gl_vertex3i_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex3iv) renderer_gl_vertex3iv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex3s) renderer_gl_vertex3s_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex3sv) renderer_gl_vertex3sv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex4d) renderer_gl_vertex4d_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex4dv) renderer_gl_vertex4dv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex4f) renderer_gl_vertex4f_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex4fv) renderer_gl_vertex4fv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex4i) renderer_gl_vertex4i_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex4iv) renderer_gl_vertex4iv_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex4s) renderer_gl_vertex4s_func_t;
typedef CODUO_FUNCTION_POINTER_TYPE(glVertex4sv) renderer_gl_vertex4sv_func_t;

#endif
