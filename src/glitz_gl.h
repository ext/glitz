/*
 * Copyright © 2004 David Reveman, Peter Nilsson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the names of
 * David Reveman and Peter Nilsson not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission. David Reveman and Peter Nilsson
 * makes no representations about the suitability of this software for
 * any purpose. It is provided "as is" without express or implied warranty.
 *
 * DAVID REVEMAN AND PETER NILSSON DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL DAVID REVEMAN AND
 * PETER NILSSON BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: David Reveman <c99drn@cs.umu.se>
 *          Peter Nilsson <c99pnn@cs.umu.se>
 */

#ifndef GLITZ_GL_H_INCLUDED
#define GLITZ_GL_H_INCLUDED

typedef unsigned int glitz_gl_enum_t;
typedef unsigned char glitz_gl_boolean_t;
typedef void glitz_gl_void_t;
typedef int glitz_gl_int_t;
typedef unsigned int glitz_gl_uint_t;
typedef int glitz_gl_sizei_t;
typedef double glitz_gl_double_t;
typedef float glitz_gl_float_t;
typedef unsigned short glitz_gl_ushort_t;
typedef unsigned int glitz_gl_bitfield_t;
typedef double glitz_gl_clampd_t;
typedef float glitz_gl_clampf_t;
typedef unsigned char glitz_gl_ubyte_t;

#define GLITZ_GL_FALSE 0x0
#define GLITZ_GL_TRUE  0x1

#define GLITZ_GL_UNSIGNED_BYTE            0x1401
#define GLITZ_GL_UNSIGNED_INT_8_8_8_8_REV 0x8367

#define GLITZ_GL_MODELVIEW  0x1700
#define GLITZ_GL_PROJECTION 0x1701

#define GLITZ_GL_TRIANGLES      0x0004
#define GLITZ_GL_TRIANGLE_STRIP 0x0005
#define GLITZ_GL_TRIANGLE_FAN   0x0006
#define GLITZ_GL_QUADS          0x0007

#define GLITZ_GL_FILL           0x1B02
#define GLITZ_GL_FRONT          0x0404
#define GLITZ_GL_BACK           0x0405
#define GLITZ_GL_CULL_FACE      0x0B44
#define GLITZ_GL_POLYGON_SMOOTH 0x0B41

#define GLITZ_GL_SCISSOR_TEST 0x0C11

#define GLITZ_GL_TEXTURE_ENV        0x2300
#define GLITZ_GL_TEXTURE_ENV_MODE   0x2200
#define GLITZ_GL_TEXTURE_1D         0x0DE0
#define GLITZ_GL_TEXTURE_2D         0x0DE1
#define GLITZ_GL_TEXTURE_WRAP_S     0x2802
#define GLITZ_GL_TEXTURE_WRAP_T     0x2803
#define GLITZ_GL_TEXTURE_MAG_FILTER 0x2800
#define GLITZ_GL_TEXTURE_MIN_FILTER 0x2801
#define GLITZ_GL_NEAREST            0x2600
#define GLITZ_GL_LINEAR             0x2601
#define GLITZ_GL_REPEAT             0x2901
#define GLITZ_GL_CLAMP_TO_EDGE      0x812F

#define GLITZ_GL_STENCIL_TEST 0x0B90
#define GLITZ_GL_KEEP         0x1E00
#define GLITZ_GL_REPLACE      0x1E01
#define GLITZ_GL_INCR         0x1E02
#define GLITZ_GL_DECR         0x1E03

#define GLITZ_GL_EQUAL      0x0202
#define GLITZ_GL_ALWAYS     0x0207
#define GLITZ_GL_DEPTH_TEST 0x0B71

#define GLITZ_GL_STENCIL_BUFFER_BIT 0x00000400
#define GLITZ_GL_VIEWPORT_BIT       0x00000800
#define GLITZ_GL_TRANSFORM_BIT      0x00001000
#define GLITZ_GL_COLOR_BUFFER_BIT   0x00004000

#define GLITZ_GL_ALPHA  0x1906
#define GLITZ_GL_RGB    0x1907
#define GLITZ_GL_COLOR  0x1800
#define GLITZ_GL_DITHER 0x0BD0
#define GLITZ_GL_RGBA   0x1908
#define GLITZ_GL_BGR    0x80E0
#define GLITZ_GL_BGRA   0x80E1

#define GLITZ_GL_FRONT_AND_BACK 0x0408
#define GLITZ_GL_FLAT           0x1D00
#define GLITZ_GL_SMOOTH         0x1D01

#define GLITZ_GL_BLEND               0x0BE2
#define GLITZ_GL_ZERO                0x0000
#define GLITZ_GL_ONE                 0x0001
#define GLITZ_GL_SRC_ALPHA           0x0302
#define GLITZ_GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GLITZ_GL_DST_ALPHA           0x0304
#define GLITZ_GL_ONE_MINUS_DST_ALPHA 0x0305
#define GLITZ_GL_SRC_ALPHA_SATURATE  0x0308

#define GLITZ_GL_PACK_ALIGNMENT     0x0D05
#define GLITZ_GL_PACK_ROW_LENGTH    0x0D02
#define GLITZ_GL_PACK_SKIP_PIXELS   0x0D04
#define GLITZ_GL_PACK_SKIP_ROWS     0x0D03
#define GLITZ_GL_UNPACK_ALIGNMENT   0x0CF5
#define GLITZ_GL_UNPACK_ROW_LENGTH  0x0CF2
#define GLITZ_GL_UNPACK_SKIP_PIXELS 0x0CF4
#define GLITZ_GL_UNPACK_SKIP_ROWS   0x0CF3

#define GLITZ_GL_PERSPECTIVE_CORRECTION_HINT 0x0C50
#define GLITZ_GL_FASTEST                     0x1101
#define GLITZ_GL_NICEST                      0x1102

#define GLITZ_GL_TEXTURE_RECTANGLE_EXT 0x84F5

#define GLITZ_GL_MIRRORED_REPEAT_ARB 0x8370

#define GLITZ_GL_TEXTURE0_ARB       0x84C0
#define GLITZ_GL_TEXTURE1_ARB       0x84C1
#define GLITZ_GL_TEXTURE2_ARB       0x84C2
#define GLITZ_GL_ACTIVE_TEXTURE_ARB 0x84E0

#define GLITZ_GL_MULTISAMPLE_ARB 0x809D

#define GLITZ_GL_MULTISAMPLE_FILTER_HINT_NV 0x8534

#define GLITZ_GL_VERTEX_PROGRAM_ARB         0x8620
#define GLITZ_GL_PROGRAM_STRING_ARB         0x8628
#define GLITZ_GL_PROGRAM_FORMAT_ASCII_ARB   0x8875
#define GLITZ_GL_PROGRAM_ERROR_POSITION_ARB 0x864B

#define GLITZ_GL_FRAGMENT_PROGRAM_ARB             0x8804
#define GLITZ_GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB 0x880D

typedef glitz_gl_void_t (* glitz_gl_enable_t)
     (glitz_gl_enum_t cap);
typedef glitz_gl_void_t (* glitz_gl_disable_t)
     (glitz_gl_enum_t cap);
typedef glitz_gl_void_t (* glitz_gl_begin_t)
     (glitz_gl_enum_t mode);
typedef glitz_gl_void_t (* glitz_gl_end_t)
     (glitz_gl_void_t);
typedef glitz_gl_void_t (* glitz_gl_vertex_2i_t)
     (glitz_gl_int_t x, glitz_gl_int_t y);
typedef glitz_gl_void_t (* glitz_gl_vertex_2d_t)
     (glitz_gl_double_t x, glitz_gl_double_t y);
typedef glitz_gl_void_t (* glitz_gl_tex_env_f_t)
     (glitz_gl_enum_t target, glitz_gl_enum_t pname, glitz_gl_float_t param);
typedef glitz_gl_void_t (* glitz_gl_tex_coord_2d_t)
     (glitz_gl_double_t s, glitz_gl_double_t t);
typedef glitz_gl_void_t (* glitz_gl_scissor_t)
     (glitz_gl_int_t x, glitz_gl_int_t y,
      glitz_gl_sizei_t width, glitz_gl_sizei_t height);
typedef glitz_gl_void_t (* glitz_gl_color_4us_t)
     (glitz_gl_ushort_t red, glitz_gl_ushort_t green, glitz_gl_ushort_t blue,
      glitz_gl_ushort_t alpha);
typedef glitz_gl_void_t (* glitz_gl_blend_func_t)
     (glitz_gl_enum_t sfactor, glitz_gl_enum_t dfactor);
typedef glitz_gl_void_t (* glitz_gl_clear_t)
     (glitz_gl_bitfield_t mask);
typedef glitz_gl_void_t (* glitz_gl_clear_color_t)
     (glitz_gl_clampf_t red, glitz_gl_clampf_t green,
      glitz_gl_clampf_t blue, glitz_gl_clampf_t alpha);
typedef glitz_gl_void_t (* glitz_gl_clear_stencil_t)
     (glitz_gl_int_t s);
typedef glitz_gl_void_t (* glitz_gl_stencil_func_t)
     (glitz_gl_enum_t func, glitz_gl_int_t ref, glitz_gl_uint_t mask);
typedef glitz_gl_void_t (* glitz_gl_stencil_op_t)
     (glitz_gl_enum_t fail, glitz_gl_enum_t zfail, glitz_gl_enum_t zpass);
typedef glitz_gl_void_t (* glitz_gl_push_attrib_t)
     (glitz_gl_bitfield_t mask);
typedef glitz_gl_void_t (* glitz_gl_pop_attrib_t)
     (glitz_gl_void_t);
typedef glitz_gl_void_t (* glitz_gl_matrix_mode_t)
     (glitz_gl_enum_t mode);
typedef glitz_gl_void_t (* glitz_gl_push_matrix_t)
     (glitz_gl_void_t);
typedef glitz_gl_void_t (* glitz_gl_pop_matrix_t)
     (glitz_gl_void_t);
typedef glitz_gl_void_t (* glitz_gl_load_identity_t)
     (glitz_gl_void_t);
typedef glitz_gl_void_t (* glitz_gl_depth_range_t)
     (glitz_gl_clampd_t near_val, glitz_gl_clampd_t far_val);
typedef glitz_gl_void_t (* glitz_gl_viewport_t)
     (glitz_gl_int_t x, glitz_gl_int_t y,
      glitz_gl_sizei_t width, glitz_gl_sizei_t height);
typedef glitz_gl_void_t (* glitz_gl_raster_pos_2d_t)
     (glitz_gl_double_t x, glitz_gl_double_t y);
typedef glitz_gl_void_t (* glitz_gl_bitmap_t)
     (glitz_gl_sizei_t width, glitz_gl_sizei_t height,
      glitz_gl_float_t xorig, glitz_gl_float_t yorig,
      glitz_gl_float_t xmove, glitz_gl_float_t ymove,
      const glitz_gl_ubyte_t *bitmap);
typedef glitz_gl_void_t (* glitz_gl_read_buffer_t)
     (glitz_gl_enum_t mode);
typedef glitz_gl_void_t (* glitz_gl_draw_buffer_t)
     (glitz_gl_enum_t mode);
typedef glitz_gl_void_t (* glitz_gl_copy_pixels_t)
     (glitz_gl_int_t x, glitz_gl_int_t y,
      glitz_gl_sizei_t width, glitz_gl_sizei_t height,
      glitz_gl_enum_t type);
typedef glitz_gl_void_t (* glitz_gl_flush_t)
     (glitz_gl_void_t);
typedef glitz_gl_void_t (* glitz_gl_pixel_store_i_t)
     (glitz_gl_enum_t pname, glitz_gl_int_t param);
typedef glitz_gl_void_t (* glitz_gl_ortho_t)
     (glitz_gl_double_t left, glitz_gl_double_t right,
      glitz_gl_double_t bottom, glitz_gl_double_t top,
      glitz_gl_double_t near_val, glitz_gl_double_t far_val);
typedef glitz_gl_void_t (* glitz_gl_scale_d_t)
     (glitz_gl_double_t x, glitz_gl_double_t y, glitz_gl_double_t z);
typedef glitz_gl_void_t (* glitz_gl_translate_d_t)
     (glitz_gl_double_t x, glitz_gl_double_t y, glitz_gl_double_t z);
typedef glitz_gl_void_t (* glitz_gl_hint_t)
     (glitz_gl_enum_t target, glitz_gl_enum_t mode);
typedef glitz_gl_void_t (* glitz_gl_depth_mask_t)
     (glitz_gl_boolean_t flag);
typedef glitz_gl_void_t (* glitz_gl_polygon_mode_t)
     (glitz_gl_enum_t face, glitz_gl_enum_t mode);
typedef glitz_gl_void_t (* glitz_gl_shade_model_t)
     (glitz_gl_enum_t mode);
typedef glitz_gl_void_t (* glitz_gl_color_mask_t)
     (glitz_gl_boolean_t red,
      glitz_gl_boolean_t green,
      glitz_gl_boolean_t blue,
      glitz_gl_boolean_t alpha);
typedef glitz_gl_void_t (* glitz_gl_read_pixels_t)
     (glitz_gl_int_t x, glitz_gl_int_t y,
      glitz_gl_sizei_t width, glitz_gl_sizei_t height,
      glitz_gl_enum_t format, glitz_gl_enum_t type,
      glitz_gl_void_t *pixels);
typedef glitz_gl_void_t (* glitz_gl_get_tex_image_t)
     (glitz_gl_enum_t target, glitz_gl_int_t level,
      glitz_gl_enum_t format, glitz_gl_enum_t type,
      glitz_gl_void_t *pixels);
typedef glitz_gl_void_t (* glitz_gl_pixel_zoom_t)
     (glitz_gl_float_t xfactor, glitz_gl_float_t yfactor);
typedef glitz_gl_void_t (* glitz_gl_draw_pixels_t)
     (glitz_gl_sizei_t width, glitz_gl_sizei_t height,
      glitz_gl_enum_t format, glitz_gl_enum_t type,
      const glitz_gl_void_t *pixels);
typedef glitz_gl_void_t (* glitz_gl_tex_sub_image_2d_t)
     (glitz_gl_enum_t target, glitz_gl_int_t level,
      glitz_gl_int_t xoffset, glitz_gl_int_t yoffset,
      glitz_gl_sizei_t width, glitz_gl_sizei_t height,
      glitz_gl_enum_t format, glitz_gl_enum_t type,
      const glitz_gl_void_t *pixels);
typedef glitz_gl_void_t (* glitz_gl_gen_textures_t)
     (glitz_gl_sizei_t n, glitz_gl_uint_t *textures);
typedef glitz_gl_void_t (* glitz_gl_delete_textures_t)
     (glitz_gl_sizei_t n, const glitz_gl_uint_t *textures);
typedef glitz_gl_void_t (* glitz_gl_bind_texture_t)
     (glitz_gl_enum_t target, glitz_gl_uint_t texture);
typedef glitz_gl_void_t (* glitz_gl_tex_image_1d_t)
     (glitz_gl_enum_t target, glitz_gl_int_t level,
      glitz_gl_int_t internal_format,
      glitz_gl_sizei_t width, glitz_gl_int_t border,
      glitz_gl_enum_t format, glitz_gl_enum_t type,
      const glitz_gl_void_t *pixels);
typedef glitz_gl_void_t (* glitz_gl_tex_image_2d_t)
     (glitz_gl_enum_t target, glitz_gl_int_t level,
      glitz_gl_int_t internal_format,
      glitz_gl_sizei_t width, glitz_gl_sizei_t height,
      glitz_gl_int_t border, glitz_gl_enum_t format, glitz_gl_enum_t type,
      const glitz_gl_void_t *pixels);
typedef glitz_gl_void_t (* glitz_gl_tex_parameter_i_t)
     (glitz_gl_enum_t target, glitz_gl_enum_t pname, glitz_gl_int_t param);
typedef glitz_gl_void_t (* glitz_gl_copy_tex_sub_image_2d_t)
     (glitz_gl_enum_t target, glitz_gl_int_t level,
      glitz_gl_int_t xoffset, glitz_gl_int_t yoffset,
      glitz_gl_int_t x, glitz_gl_int_t y,
      glitz_gl_sizei_t width, glitz_gl_sizei_t height);
typedef glitz_gl_void_t (* glitz_gl_get_integer_v_t)
     (glitz_gl_enum_t pname, glitz_gl_int_t *params);
typedef glitz_gl_void_t (* glitz_gl_active_texture_arb_t)
     (glitz_gl_enum_t);
typedef glitz_gl_void_t (* glitz_gl_multi_tex_coord_2d_arb_t)
     (glitz_gl_enum_t, glitz_gl_double_t, glitz_gl_double_t);
typedef glitz_gl_void_t (* glitz_gl_gen_programs_arb_t)
     (glitz_gl_sizei_t, glitz_gl_uint_t *);
typedef glitz_gl_void_t (* glitz_gl_delete_programs_arb_t)
     (glitz_gl_sizei_t, const glitz_gl_uint_t *);
typedef glitz_gl_void_t (* glitz_gl_program_string_arb_t)
     (glitz_gl_enum_t, glitz_gl_enum_t, glitz_gl_sizei_t,
      const glitz_gl_void_t *);
typedef glitz_gl_void_t (* glitz_gl_bind_program_arb_t)
     (glitz_gl_enum_t, glitz_gl_uint_t);
typedef glitz_gl_void_t (* glitz_gl_program_local_param_4d_arb_t)
     (glitz_gl_enum_t, glitz_gl_uint_t,
      glitz_gl_double_t, glitz_gl_double_t,
      glitz_gl_double_t, glitz_gl_double_t);
typedef glitz_gl_void_t (* glitz_gl_get_program_iv_arb_t)
     (glitz_gl_enum_t, glitz_gl_enum_t, glitz_gl_uint_t *);

#endif /* GLITZ_GL_H_INCLUDED */
