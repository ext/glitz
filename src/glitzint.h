/*
 * Copyright © 2004 David Reveman
 * 
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the names of
 * David Reveman not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * David Reveman makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DAVID REVEMAN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DAVID REVEMAN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, 
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <c99drn@cs.umu.se>
 */

#ifndef GLITZINT_H_INCLUDED
#define GLITZINT_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "glitz.h"

#if __GNUC__ >= 3 && defined(__ELF__)
# define slim_hidden_proto(name)	slim_hidden_proto1(name, INT_##name)
# define slim_hidden_def(name)		slim_hidden_def1(name, INT_##name)
# define slim_hidden_proto1(name, internal)				\
  extern __typeof (name) name						\
	__asm__ (slim_hidden_asmname (internal))			\
	__internal_linkage;
# define slim_hidden_def1(name, internal)				\
  extern __typeof (name) EXT_##name __asm__(slim_hidden_asmname(name))	\
	__attribute__((__alias__(slim_hidden_asmname(internal))))
# define slim_hidden_ulp		slim_hidden_ulp1(__USER_LABEL_PREFIX__)
# define slim_hidden_ulp1(x)		slim_hidden_ulp2(x)
# define slim_hidden_ulp2(x)		#x
# define slim_hidden_asmname(name)	slim_hidden_asmname1(name)
# define slim_hidden_asmname1(name)	slim_hidden_ulp #name
#else
# define slim_hidden_proto(name)
# define slim_hidden_def(name)
#endif

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3)) && defined(__ELF__)
#define __internal_linkage	__attribute__((__visibility__("hidden")))
#else
#define __internal_linkage
#endif

#ifndef __GNUC__
#define __attribute__(x)
#endif

#define GLITZ_STATUS_NO_MEMORY_MASK          (1L << 0)
#define GLITZ_STATUS_BAD_COORDINATE_MASK     (1L << 1)
#define GLITZ_STATUS_NOT_SUPPORTED_MASK      (1L << 2)
#define GLITZ_STATUS_CONTENT_DESTROYED_MASK  (1L << 3)

#define GLITZ_TEXTURE_TARGET_2D_MASK               (1L << 0)
#define GLITZ_TEXTURE_TARGET_RECTANGLE_MASK        (1L << 1)
#define GLITZ_TEXTURE_TARGET_NON_POWER_OF_TWO_MASK (1L << 2)

#define GLITZ_FORMAT_ALL_EXCEPT_ID_MASK ((1L << 19) - 2)

#include "glitz_gl.h"

#define GLITZ_CONTEXT_STACK_SIZE 16

typedef struct _glitz_gl_proc_address_list_t {
  glitz_gl_enable_t enable;
  glitz_gl_disable_t disable;
  glitz_gl_enable_client_state_t enable_client_state;
  glitz_gl_disable_client_state_t disable_client_state;
  glitz_gl_vertex_pointer_t vertex_pointer;
  glitz_gl_draw_arrays_t draw_arrays;
  glitz_gl_tex_env_f_t tex_env_f;
  glitz_gl_tex_env_fv_t tex_env_fv;
  glitz_gl_tex_gen_i_t tex_gen_i;
  glitz_gl_tex_gen_fv_t tex_gen_fv;
  glitz_gl_color_4us_t color_4us;
  glitz_gl_color_4f_t color_4f;
  glitz_gl_scissor_t scissor;
  glitz_gl_blend_func_t blend_func;
  glitz_gl_clear_t clear;
  glitz_gl_clear_color_t clear_color;
  glitz_gl_clear_stencil_t clear_stencil;
  glitz_gl_stencil_func_t stencil_func;
  glitz_gl_stencil_op_t stencil_op;
  glitz_gl_push_attrib_t push_attrib;
  glitz_gl_pop_attrib_t pop_attrib;
  glitz_gl_matrix_mode_t matrix_mode;
  glitz_gl_push_matrix_t push_matrix;
  glitz_gl_pop_matrix_t pop_matrix;
  glitz_gl_load_identity_t load_identity;
  glitz_gl_load_matrix_f_t load_matrix_f;
  glitz_gl_depth_range_t depth_range;
  glitz_gl_viewport_t viewport;
  glitz_gl_raster_pos_2f_t raster_pos_2f;
  glitz_gl_bitmap_t bitmap;
  glitz_gl_read_buffer_t read_buffer;
  glitz_gl_draw_buffer_t draw_buffer;
  glitz_gl_copy_pixels_t copy_pixels;
  glitz_gl_flush_t flush;
  glitz_gl_finish_t finish;
  glitz_gl_pixel_store_i_t pixel_store_i;
  glitz_gl_ortho_t ortho;
  glitz_gl_scale_f_t scale_f;
  glitz_gl_translate_f_t translate_f;
  glitz_gl_hint_t hint;
  glitz_gl_depth_mask_t depth_mask;
  glitz_gl_polygon_mode_t polygon_mode;
  glitz_gl_shade_model_t shade_model;
  glitz_gl_color_mask_t color_mask;
  glitz_gl_read_pixels_t read_pixels;
  glitz_gl_get_tex_image_t get_tex_image;
  glitz_gl_tex_sub_image_2d_t tex_sub_image_2d;
  glitz_gl_gen_textures_t gen_textures;
  glitz_gl_delete_textures_t delete_textures;
  glitz_gl_bind_texture_t bind_texture;
  glitz_gl_tex_image_2d_t tex_image_2d;
  glitz_gl_tex_parameter_i_t tex_parameter_i;
  glitz_gl_get_tex_level_parameter_iv_t get_tex_level_parameter_iv;
  glitz_gl_copy_tex_sub_image_2d_t copy_tex_sub_image_2d;
  glitz_gl_get_integer_v_t get_integer_v;
  
  glitz_gl_active_texture_t active_texture;
  glitz_gl_gen_programs_t gen_programs;
  glitz_gl_delete_programs_t delete_programs;
  glitz_gl_program_string_t program_string;
  glitz_gl_bind_program_t bind_program;
  glitz_gl_program_local_param_4fv_t program_local_param_4fv;
  glitz_gl_get_program_iv_t get_program_iv;
  glitz_gl_gen_buffers_t gen_buffers;
  glitz_gl_delete_buffers_t delete_buffers;
  glitz_gl_bind_buffer_t bind_buffer;
  glitz_gl_buffer_data_t buffer_data;
  glitz_gl_buffer_sub_data_t buffer_sub_data;
  glitz_gl_get_buffer_sub_data_t get_buffer_sub_data;
  glitz_gl_map_buffer_t map_buffer;
  glitz_gl_unmap_buffer_t unmap_buffer;
  
  glitz_bool_t need_lookup;
} glitz_gl_proc_address_list_t;

typedef int glitz_surface_type_t;

#define GLITZ_SURFACE_TYPE_NA    -1
#define GLITZ_SURFACE_TYPE_NULL   0
#define GLITZ_SURFACE_TYPE_ARGB   1
#define GLITZ_SURFACE_TYPE_ARGBC  2
#define GLITZ_SURFACE_TYPE_ARGBF  3
#define GLITZ_SURFACE_TYPE_SOLID  4
#define GLITZ_SURFACE_TYPE_SOLIDC 5
#define GLITZ_SURFACE_TYPES       6

typedef int glitz_combine_type_t;

#define GLITZ_COMBINE_TYPE_NA            -1
#define GLITZ_COMBINE_TYPE_INTERMEDIATE   0
#define GLITZ_COMBINE_TYPE_ARGB           1
#define GLITZ_COMBINE_TYPE_ARGB_ARGB      2
#define GLITZ_COMBINE_TYPE_ARGB_ARGBC     3
#define GLITZ_COMBINE_TYPE_ARGB_SOLID     4
#define GLITZ_COMBINE_TYPE_ARGB_SOLIDC    5
#define GLITZ_COMBINE_TYPE_ARGBF          6
#define GLITZ_COMBINE_TYPE_ARGBF_ARGB     7
#define GLITZ_COMBINE_TYPE_ARGBF_ARGBC    8
#define GLITZ_COMBINE_TYPE_ARGBF_SOLID    9
#define GLITZ_COMBINE_TYPE_ARGBF_SOLIDC  10
#define GLITZ_COMBINE_TYPE_SOLID         11
#define GLITZ_COMBINE_TYPE_SOLID_ARGB    12
#define GLITZ_COMBINE_TYPE_SOLID_ARGBC   13
#define GLITZ_COMBINE_TYPE_SOLID_SOLID   14
#define GLITZ_COMBINE_TYPE_SOLID_SOLIDC  15
#define GLITZ_COMBINE_TYPES              16

#define GLITZ_TEXTURE_NONE 0
#define GLITZ_TEXTURE_2D   1
#define GLITZ_TEXTURE_RECT 2
#define GLITZ_TEXTURE_LAST 3

#define GLITZ_FP_CONVOLUTION                 0
#define GLITZ_FP_LINEAR_GRADIENT_TRANSPARENT 1
#define GLITZ_FP_LINEAR_GRADIENT_NEAREST     2
#define GLITZ_FP_LINEAR_GRADIENT_REPEAT      3
#define GLITZ_FP_LINEAR_GRADIENT_REFLECT     4
#define GLITZ_FP_RADIAL_GRADIENT_TRANSPARENT 5
#define GLITZ_FP_RADIAL_GRADIENT_NEAREST     6
#define GLITZ_FP_RADIAL_GRADIENT_REPEAT      7
#define GLITZ_FP_RADIAL_GRADIENT_REFLECT     8
#define GLITZ_FP_TYPES                       9

typedef struct _glitz_program_t {
  glitz_gl_int_t *name;
  unsigned int size;
} glitz_program_t;

typedef struct _glitz_filter_map_t {
  glitz_program_t fp[GLITZ_TEXTURE_LAST][GLITZ_TEXTURE_LAST];
} glitz_filter_map_t;

typedef struct _glitz_program_map_t {
  glitz_filter_map_t filters[GLITZ_COMBINE_TYPES][GLITZ_FP_TYPES];
} glitz_program_map_t;

typedef enum {
  GLITZ_CN_NONE,
  GLITZ_CN_ANY_CONTEXT_CURRENT,
  GLITZ_CN_SURFACE_CONTEXT_CURRENT,
  GLITZ_CN_SURFACE_DRAWABLE_CURRENT
} glitz_constraint_t;

typedef struct _glitz_bounding_box_t {
  int x1, x2, y1, y2;
} glitz_bounding_box_t;

typedef struct _glitz_point_t {
  glitz_float_t x, y;
} glitz_point_t;

typedef struct _glitz_vec_t {
  glitz_float_t v[4];
} glitz_vec4_t;

typedef struct _glitz_texture {
  glitz_gl_uint_t name;
  glitz_gl_enum_t target;
  glitz_gl_enum_t format;
  glitz_bool_t allocated;
  
  glitz_gl_enum_t filter;
  glitz_gl_enum_t wrap;
  
  unsigned int width;
  unsigned int height;

  glitz_float_t texcoord_width;
  glitz_float_t texcoord_height;
  glitz_float_t texcoord_width_unit;
  glitz_float_t texcoord_height_unit;

  glitz_bool_t repeatable;
} glitz_texture_t;

struct _glitz_buffer {
  glitz_gl_sizei_t size;
  glitz_gl_uint_t name;
  glitz_gl_enum_t target;
  void *data;
  int owns_data;
  int ref_count;
  glitz_surface_t *surface;
};

typedef struct _glitz_geometry {
  glitz_gl_enum_t primitive;
  glitz_gl_enum_t type;
  glitz_gl_int_t first;
  glitz_gl_sizei_t count;
  glitz_buffer_t *buffer;
  glitz_float_t x_offset;
  glitz_float_t y_offset;

  glitz_gl_uint_t default_name;
  glitz_gl_float_t data[8];
} glitz_geometry_t;

typedef struct glitz_surface_backend {
  glitz_surface_t *
  (*create_similar) (void *surface,
                     glitz_format_t *format,
                     int width,
                     int height);
  
  void
  (*destroy) (void *surface);
  
  glitz_bool_t
  (*push_current) (void *surface,
                   glitz_constraint_t constraint);

  void
  (*pop_current) (void *surface);

  glitz_texture_t *
  (*get_texture) (void *surface,
                  glitz_bool_t allocate);
  
  void
  (*swap_buffers) (void *surface);

  glitz_bool_t
  (*make_current_read) (void *surface);

  glitz_gl_proc_address_list_t gl;
  glitz_format_t *formats;
  int n_formats;
  glitz_program_map_t *program_map;
  unsigned long feature_mask;
} glitz_surface_backend_t;

#define GLITZ_FLAG_SOLID_MASK                   (1L <<  0)
#define GLITZ_FLAG_OFFSCREEN_MASK               (1L <<  1)
#define GLITZ_FLAG_REPEAT_MASK                  (1L <<  2)
#define GLITZ_FLAG_MIRRORED_MASK                (1L <<  3)
#define GLITZ_FLAG_PAD_MASK                     (1L <<  4)
#define GLITZ_FLAG_DIRTY_MASK                   (1L <<  5)
#define GLITZ_FLAG_COMPONENT_ALPHA_MASK         (1L <<  6)
#define GLITZ_FLAG_MULTISAMPLE_MASK             (1L <<  7)
#define GLITZ_FLAG_NICEST_MULTISAMPLE_MASK      (1L <<  8)
#define GLITZ_FLAG_SOLID_DIRTY_MASK             (1L <<  9)
#define GLITZ_FLAG_DRAWABLE_DIRTY_MASK          (1L << 10)
#define GLITZ_FLAG_FRAGMENT_FILTER_MASK         (1L << 11)
#define GLITZ_FLAG_LINEAR_TRANSFORM_FILTER_MASK (1L << 12)
#define GLITZ_FLAG_DRAWABLE_MASK                (1L << 13)
#define GLITZ_FLAG_IGNORE_REPEAT_MASK           (1L << 14)
#define GLITZ_FLAG_TEXTURE_COORDS_MASK          (1L << 15)

#define SURFACE_OFFSCREEN(surface) \
  ((surface)->flags & GLITZ_FLAG_OFFSCREEN_MASK)

#define SURFACE_SOLID(surface) \
  ((surface)->flags & GLITZ_FLAG_SOLID_MASK)

#define SURFACE_REPEAT(surface) \
  (((surface)->flags & GLITZ_FLAG_REPEAT_MASK) && \
   (!((surface)->flags & GLITZ_FLAG_IGNORE_REPEAT_MASK)))

#define SURFACE_MIRRORED(surface) \
  ((surface)->flags & GLITZ_FLAG_MIRRORED_MASK)

#define SURFACE_PAD(surface) \
  ((surface)->flags & GLITZ_FLAG_PAD_MASK)

#define SURFACE_DIRTY(surface) \
  ((surface)->flags & GLITZ_FLAG_DIRTY_MASK)

#define SURFACE_COMPONENT_ALPHA(surface) \
  ((surface)->flags & GLITZ_FLAG_COMPONENT_ALPHA_MASK)

#define SURFACE_MULTISAMPLE(surface) \
  ((surface)->flags & GLITZ_FLAG_MULTISAMPLE_MASK)

#define SURFACE_NICEST_MULTISAMPLE(surface) \
  ((surface)->flags & GLITZ_FLAG_NICEST_MULTISAMPLE_MASK)

#define SURFACE_SOLID_DIRTY(surface) \
  ((surface)->flags & GLITZ_FLAG_SOLID_DIRTY_MASK)

#define SURFACE_DRAWABLE_DIRTY(surface) \
  ((surface)->flags & GLITZ_FLAG_DRAWABLE_DIRTY_MASK)

#define SURFACE_FRAGMENT_FILTER(surface) \
  ((surface)->flags & GLITZ_FLAG_FRAGMENT_FILTER_MASK)

#define SURFACE_LINEAR_TRANSFORM_FILTER(surface) \
  ((surface)->flags & GLITZ_FLAG_LINEAR_TRANSFORM_FILTER_MASK)

#define SURFACE_DRAWABLE(surface) \
  ((surface)->flags & GLITZ_FLAG_DRAWABLE_MASK)

#define SURFACE_TEXTURE_COORDS(surface) \
  ((surface)->flags & GLITZ_FLAG_TEXTURE_COORDS_MASK)

typedef struct _glitz_sample_offset {
  glitz_float_t x;
  glitz_float_t y;
} glitz_sample_offset_t;

typedef struct _glitz_multi_sample_info {
  glitz_sample_offset_t *offsets;
  unsigned short *weights;
  int n_samples;
} glitz_sample_info_t;

typedef struct _glitz_filter_params_t glitz_filter_params_t;

typedef struct _glitz_matrix {
  glitz_float_t t[16];
  glitz_float_t m[16];
} glitz_matrix_t;

#define GLITZ_UPDATE_VIEWPORT_MASK    (1L << 0)
#define GLITZ_UPDATE_DRAW_BUFFER_MASK (1L << 1)
#define GLITZ_UPDATE_MULTISAMPLE_MASK (1L << 2)
#define GLITZ_UPDATE_ALL_MASK         ((1L << 3) - 1)

struct _glitz_surface {
  glitz_surface_backend_t *backend;

  int ref_count;
  glitz_format_t *format;
  glitz_texture_t texture;
  unsigned long status_mask;
  unsigned long update_mask;
  glitz_filter_t filter;
  glitz_filter_params_t *filter_params;
  glitz_matrix_t *transform;
  int width, height;
  glitz_bounding_box_t dirty_box;
  unsigned long flags;
  glitz_gl_enum_t draw_buffer;
  glitz_gl_enum_t read_buffer;
  glitz_sample_info_t *indirect;
  glitz_color_t solid;
  glitz_geometry_t geometry;
};

typedef struct _glitz_composite_op_t glitz_composite_op_t;

typedef void (*glitz_combine_function_t) (glitz_composite_op_t *);

typedef struct _glitz_render_t {
  glitz_combine_type_t type;
  glitz_combine_function_t enable;
  int texture_units;
  int fragment_processing;
} glitz_combine_t;

typedef enum {
  GLITZ_COMPONENT_ALPHA_NONE = 0,
  GLITZ_COMPONENT_ALPHA_RGB = 3,
  GLITZ_COMPONENT_ALPHA_ARGB = 4
} glitz_component_alpha_type_t;

struct _glitz_composite_op_t {
  glitz_combine_type_t type;
  glitz_combine_t *combine;
  glitz_gl_proc_address_list_t *gl;
  glitz_surface_t *src;
  glitz_surface_t *mask;
  glitz_surface_t *dst;
  glitz_color_t *solid;
  glitz_color_t alpha_mask;
  glitz_component_alpha_type_t component_alpha;
  glitz_gl_uint_t fp;
  int count;
};

typedef struct _glitz_extension_map {
  char *name;
  int mask;
} glitz_extension_map;

extern void __internal_linkage
glitz_set_operator (glitz_gl_proc_address_list_t *gl, glitz_operator_t op);

void
glitz_intersect_bounding_box (glitz_bounding_box_t *box1,
                              glitz_bounding_box_t *box2,
                              glitz_bounding_box_t *return_box);

extern void __internal_linkage
glitz_union_bounding_box (glitz_bounding_box_t *box1,
                          glitz_bounding_box_t *box2,
                          glitz_bounding_box_t *return_box);

long int
glitz_extensions_query (const char *extensions_string,
                        glitz_extension_map *extensions_map);

extern unsigned int __internal_linkage
glitz_uint_to_power_of_two (unsigned int x);

extern void __internal_linkage
glitz_set_raster_pos (glitz_gl_proc_address_list_t *gl,
                      int x,
                      int y);

extern void __internal_linkage
glitz_clamp_value (glitz_float_t *value,
                   glitz_float_t min, glitz_float_t max);

void
glitz_texture_init (glitz_texture_t *texture,
                    unsigned int width,
                    unsigned int height,
                    unsigned int texture_format,
                    unsigned long target_mask);

void
glitz_texture_fini (glitz_gl_proc_address_list_t *gl,
                    glitz_texture_t *texture);

void
glitz_texture_allocate (glitz_gl_proc_address_list_t *gl,
                        glitz_texture_t *texture);

extern void __internal_linkage
glitz_texture_ensure_filter (glitz_gl_proc_address_list_t *gl,
                             glitz_texture_t *texture,
                             glitz_filter_t filter);

extern void __internal_linkage
glitz_texture_ensure_wrap (glitz_gl_proc_address_list_t *gl,
                           glitz_texture_t *texture,
                           glitz_gl_enum_t wrap);

void
glitz_texture_bind (glitz_gl_proc_address_list_t *gl,
                    glitz_texture_t *texture);

void
glitz_texture_unbind (glitz_gl_proc_address_list_t *gl,
                      glitz_texture_t *texture);

void
glitz_texture_copy_surface (glitz_texture_t *texture,
                            glitz_surface_t *surface,
                            int x_surface,
                            int y_surface,
                            int width,
                            int height,
                            int x_texture,
                            int y_texture);

void
glitz_texture_set_tex_gen (glitz_gl_proc_address_list_t *gl,
                           glitz_texture_t *texture,
                           int x_src,
                           int y_src,
                           int height,
                           unsigned long flags);

void
glitz_surface_init (glitz_surface_t *surface,
                    glitz_surface_backend_t *backend,
                    glitz_format_t *format,
                    int width,
                    int height,
                    unsigned long texture_mask);

void
glitz_surface_fini (glitz_surface_t *surface);

extern glitz_texture_t *__internal_linkage
glitz_surface_get_texture (glitz_surface_t *surface,
                           glitz_bool_t allocate);

extern void __internal_linkage
glitz_surface_ensure_solid (glitz_surface_t *surface);

glitz_bool_t
glitz_surface_push_current (glitz_surface_t *surface,
                            glitz_constraint_t constraint);

void
glitz_surface_pop_current (glitz_surface_t *surface);

extern glitz_bool_t __internal_linkage
glitz_surface_make_current_read (glitz_surface_t *surface);

extern void __internal_linkage
glitz_surface_dirty (glitz_surface_t *surface,
                     glitz_bounding_box_t *box);

extern void __internal_linkage
glitz_surface_status_add (glitz_surface_t *surface, int flags);

void
glitz_surface_update_state (glitz_surface_t *surface);

extern unsigned long __internal_linkage
glitz_status_to_status_mask (glitz_status_t status);
     
extern glitz_status_t __internal_linkage
glitz_status_pop_from_mask (unsigned long *mask);

typedef void (*glitz_format_call_back_t) (glitz_format_t *, void *ptr);

void
glitz_format_for_each_texture_format (glitz_format_call_back_t call_back,
                                      glitz_gl_proc_address_list_t *gl,
                                      void *ptr);

glitz_format_t *
glitz_format_find (glitz_format_t *formats,
                   int n_formats,
                   unsigned long mask,
                   const glitz_format_t *templ,
                   int count);

glitz_format_t *
glitz_format_find_standard (glitz_format_t *formats,
                            int n_formats,
                            glitz_format_name_t format_name);

extern glitz_gl_int_t __internal_linkage
glitz_format_get_best_texture_format (glitz_format_t *formats,
                                      int n_formats,
                                      glitz_format_t *format);

void
glitz_program_map_init (glitz_program_map_t *map);
     
void
glitz_program_map_fini (glitz_gl_proc_address_list_t *gl,
                        glitz_program_map_t *map);

extern glitz_gl_uint_t __internal_linkage
glitz_get_fragment_program (glitz_composite_op_t *op,
                            int fp_type,
                            int id);

extern void __internal_linkage
glitz_composite_op_init (glitz_composite_op_t *op,
                         glitz_surface_t *src,
                         glitz_surface_t *mask,
                         glitz_surface_t *dst);

extern void __internal_linkage
glitz_composite_enable (glitz_composite_op_t *op);

extern void __internal_linkage
glitz_composite_disable (glitz_composite_op_t *op);

extern void *__internal_linkage
glitz_buffer_bind (glitz_buffer_t *buffer,
                   glitz_gl_enum_t target);

extern void __internal_linkage
glitz_buffer_unbind (glitz_buffer_t *buffer);

extern glitz_status_t __internal_linkage
glitz_filter_set_params (glitz_surface_t *surface,
                         glitz_filter_t filter,
                         glitz_fixed16_16_t *params,
                         int n_params);

extern void __internal_linkage
glitz_filter_set_type (glitz_surface_t *surface,
                       glitz_filter_t filter);

extern void __internal_linkage
glitz_filter_params_destroy (glitz_filter_params_t *params);

extern glitz_gl_uint_t __internal_linkage
glitz_filter_get_vertex_program (glitz_surface_t *surface,
                                 glitz_composite_op_t *op);

extern glitz_gl_uint_t __internal_linkage
glitz_filter_get_fragment_program (glitz_surface_t *surface,
                                   glitz_composite_op_t *op);

extern void __internal_linkage
glitz_filter_enable (glitz_surface_t *surface,
                     glitz_composite_op_t *op);

extern void __internal_linkage
glitz_geometry_enable_default (glitz_gl_proc_address_list_t *gl,
                               glitz_surface_t *dst);

extern void __internal_linkage
glitz_geometry_enable (glitz_gl_proc_address_list_t *gl,
                       glitz_surface_t *dst,
                       glitz_gl_enum_t *primitive,
                       glitz_gl_int_t *first,
                       glitz_gl_sizei_t *count);

extern void __internal_linkage
glitz_geometry_disable (glitz_gl_proc_address_list_t *gl,
                        glitz_surface_t *dst);

#define MAXSHORT SHRT_MAX
#define MINSHORT SHRT_MIN

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define LSBFirst 0
#define MSBFirst 1

#ifdef WORDS_BIGENDIAN
#  define IMAGE_BYTE_ORDER MSBFirst
#  define BITMAP_BIT_ORDER MSBFirst
#else
#  define IMAGE_BYTE_ORDER LSBFirst
#  define BITMAP_BIT_ORDER LSBFirst
#endif

#define GLITZ_PI 3.14159265358979323846f

/* Fixed point updates from Carl Worth, USC, Information Sciences Institute */

#ifdef WIN32
typedef __int64 glitz_fixed_32_32;
#else
#  if defined(__alpha__) || defined(__alpha) || \
      defined(ia64) || defined(__ia64__) || \
      defined(__sparc64__) || \
      defined(__s390x__) || \
      defined(x86_64) || defined (__x86_64__)
typedef long glitz_fixed_32_32;
# else
#  if defined(__GNUC__) && \
    ((__GNUC__ > 2) || \
     ((__GNUC__ == 2) && defined(__GNUC_MINOR__) && (__GNUC_MINOR__ > 7)))
__extension__
#  endif
typedef long long int glitz_fixed_32_32;
# endif
#endif

typedef uint32_t glitz_fixed_1_31;
typedef uint32_t glitz_fixed_1_16;
typedef int32_t glitz_fixed_16_16;

/*
 * An unadorned "glitz_fixed" is the same as glitz_fixed_16_16, 
 * (since it's quite common in the code) 
 */
typedef glitz_fixed_16_16 glitz_fixed;

#define FIXED_BITS 16

#define FIXED_TO_INT(f) (int) ((f) >> FIXED_BITS)
#define INT_TO_FIXED(i) ((glitz_fixed) (i) << FIXED_BITS)
#define FIXED_E ((glitz_fixed) 1)
#define FIXED1 (INT_TO_FIXED (1))
#define FIXED1_MINUS_E (FIXED1 - FIXED_E)
#define FIXED_FRAC(f) ((f) & FIXED1_MINUS_E)
#define FIXED_FLOOR(f) ((f) & ~FIXED1_MINUS_E)
#define FIXED_CEIL(f) FIXED_FLOOR ((f) + FIXED1_MINUS_E)

#define FIXED_FRACTION(f) ((f) & FIXED1_MINUS_E)
#define FIXED_MOD2(f) ((f) & (FIXED1 | FIXED1_MINUS_E))

#define FIXED_TO_FLOAT(f) (((glitz_float_t) (f)) / 65536)
#define FLOAT_TO_FIXED(f) ((int) ((f) * 65536))

#define SHORT_MULT(s1, s2) \
  ((s1 == 0xffff)? s2: ((s2 == 0xffff)? s1: \
  ((unsigned short) (((unsigned int) s1 * s2) / 0xffff))))

#define POWER_OF_TWO(v) ((v & (v - 1)) == 0)

typedef void (*glitz_function_pointer_t) (void);


/* Avoid unnecessary PLT entries.  */

slim_hidden_proto(glitz_surface_find_similar_standard_format)
slim_hidden_proto(glitz_surface_set_transform)
slim_hidden_proto(glitz_surface_set_fill)
slim_hidden_proto(glitz_surface_set_component_alpha)
slim_hidden_proto(glitz_surface_set_filter)
slim_hidden_proto(glitz_surface_get_width)
slim_hidden_proto(glitz_surface_get_height)
slim_hidden_proto(glitz_surface_set_read_color_buffer)
slim_hidden_proto(glitz_surface_set_draw_color_buffer)
slim_hidden_proto(glitz_surface_flush)
slim_hidden_proto(glitz_surface_swap_buffers)
slim_hidden_proto(glitz_surface_finish)
slim_hidden_proto(glitz_surface_get_status)
slim_hidden_proto(glitz_surface_get_features)
slim_hidden_proto(glitz_surface_get_format)
slim_hidden_proto(glitz_set_rectangle)
slim_hidden_proto(glitz_set_rectangles)
slim_hidden_proto(glitz_set_geometry)
slim_hidden_proto(glitz_buffer_set_data)
slim_hidden_proto(glitz_buffer_get_data)

#endif /* GLITZINT_H_INCLUDED */
