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
#define GLITZ_STATUS_NULL_POINTER_MASK       (1L << 1)
#define GLITZ_STATUS_BAD_DRAWABLE_MASK       (1L << 2)
#define GLITZ_STATUS_UNRELATED_SURFACES_MASK (1L << 3)
#define GLITZ_STATUS_BAD_COORDINATE_MASK     (1L << 4)
#define GLITZ_STATUS_NOT_SUPPORTED_MASK      (1L << 5)
#define GLITZ_STATUS_INVALID_MATRIX_MASK     (1L << 6)

#define GLITZ_TEXTURE_TARGET_2D_MASK        (1L << 0)
#define GLITZ_TEXTURE_TARGET_RECTANGLE_MASK (1L << 1)
#define GLITZ_TEXTURE_TARGET_NPOT_MASK      (1L << 2)

#define GLITZ_FORMAT_ALL_EXCEPT_ID_MASK ((1L << 17) - 2)

#include "glitz_gl.h"

typedef struct _glitz_gl_proc_address_list_t {
  glitz_gl_enable_t enable;
  glitz_gl_disable_t disable;
  glitz_gl_begin_t begin;
  glitz_gl_end_t end;
  glitz_gl_vertex_2i_t vertex_2i;
  glitz_gl_vertex_2d_t vertex_2d;
  glitz_gl_tex_env_f_t tex_env_f;
  glitz_gl_tex_coord_2d_t tex_coord_2d;
  glitz_gl_color_4us_t color_4us;
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
  glitz_gl_depth_range_t depth_range;
  glitz_gl_viewport_t viewport;
  glitz_gl_raster_pos_2d_t raster_pos_2d;
  glitz_gl_bitmap_t bitmap;
  glitz_gl_read_buffer_t read_buffer;
  glitz_gl_draw_buffer_t draw_buffer;
  glitz_gl_copy_pixels_t copy_pixels;
  glitz_gl_flush_t flush;
  glitz_gl_pixel_store_i_t pixel_store_i;
  glitz_gl_ortho_t ortho;
  glitz_gl_scale_d_t scale_d;
  glitz_gl_translate_d_t translate_d;
  glitz_gl_hint_t hint;
  glitz_gl_depth_mask_t depth_mask;
  glitz_gl_polygon_mode_t polygon_mode;
  glitz_gl_shade_model_t shade_model;
  glitz_gl_color_mask_t color_mask;
  glitz_gl_read_pixels_t read_pixels;
  glitz_gl_get_tex_image_t get_tex_image;
  glitz_gl_pixel_zoom_t pixel_zoom;
  glitz_gl_draw_pixels_t draw_pixels;
  glitz_gl_tex_sub_image_2d_t tex_sub_image_2d;
  glitz_gl_gen_textures_t gen_textures;
  glitz_gl_delete_textures_t delete_textures;
  glitz_gl_bind_texture_t bind_texture;
  glitz_gl_tex_image_1d_t tex_image_1d;
  glitz_gl_tex_image_2d_t tex_image_2d;
  glitz_gl_tex_parameter_i_t tex_parameter_i;
  glitz_gl_copy_tex_sub_image_2d_t copy_tex_sub_image_2d;
  glitz_gl_get_integer_v_t get_integer_v;

  glitz_gl_active_texture_arb_t active_texture_arb;
  glitz_gl_multi_tex_coord_2d_arb_t multi_tex_coord_2d_arb;
  glitz_gl_gen_programs_arb_t gen_programs_arb;
  glitz_gl_delete_programs_arb_t delete_programs_arb;
  glitz_gl_program_string_arb_t program_string_arb;
  glitz_gl_bind_program_arb_t bind_program_arb;
  glitz_gl_program_local_param_4d_arb_t program_local_param_4d_arb;
  glitz_gl_get_program_iv_arb_t get_program_iv_arb;
  glitz_bool_t supported;
} glitz_gl_proc_address_list_t;

typedef enum {
  GLITZ_PROGRAMMATIC_SURFACE_SOLID_TYPE = 0,
  GLITZ_PROGRAMMATIC_SURFACE_LINEAR_TYPE,
  GLITZ_PROGRAMMATIC_SURFACE_RADIAL_TYPE
} glitz_programmatic_surface_type_t;

typedef enum {
  GLITZ_PROGRAM_TYPE_NONE,
  GLITZ_PROGRAM_TYPE_NOT_SUPPORTED,
  GLITZ_PROGRAM_TYPE_SRC_CONVOLUTION,
  GLITZ_PROGRAM_TYPE_SRC_CONVOLUTION_AND_SOLID_MASK,
  GLITZ_PROGRAM_TYPE_MASK_CONVOLUTION,
  GLITZ_PROGRAM_TYPE_MASK_CONVOLUTION_AND_SOLID_SRC,
  GLITZ_PROGRAM_TYPE_SRC_PROGRAMMATIC,
  GLITZ_PROGRAM_TYPE_MASK_PROGRAMMATIC,
  GLITZ_PROGRAM_TYPE_SIMPLE
} glitz_program_type_t;

#define GLITZ_PROGRAMMATIC_SURFACE_NUM \
  (GLITZ_PROGRAMMATIC_SURFACE_RADIAL_TYPE + 1)

#define GLITZ_PROGRAM_2DSRC_2DMASK_OFFSET     0
#define GLITZ_PROGRAM_RECTSRC_2DMASK_OFFSET   1
#define GLITZ_PROGRAM_2DSRC_RECTMASK_OFFSET   2
#define GLITZ_PROGRAM_RECTSRC_RECTMASK_OFFSET 3
#define GLITZ_PROGRAM_NOSRC_2DMASK_OFFSET     4
#define GLITZ_PROGRAM_NOSRC_RECTMASK_OFFSET   5
#define GLITZ_PROGRAM_2DSRC_NOMASK_OFFSET     6
#define GLITZ_PROGRAM_RECTSRC_NOMASK_OFFSET   7
#define GLITZ_PROGRAM_NOSRC_NOMASK_OFFSET     8

#define GLITZ_PROGRAM_SRC_OPERATION_OFFSET  0
#define GLITZ_PROGRAM_MASK_OPERATION_OFFSET 9

#define GLITZ_VERTEX_PROGRAM_TYPES 2
#define GLITZ_FRAGMENT_PROGRAM_TYPES 18
#define GLITZ_FRAGMENT_PROGRAMMATIC_PROGRAM_TYPES \
  (GLITZ_FRAGMENT_PROGRAM_TYPES * GLITZ_PROGRAMMATIC_SURFACE_NUM)

typedef struct _glitz_programs_t {
  unsigned long vertex_convolution[GLITZ_VERTEX_PROGRAM_TYPES];
  unsigned long fragment_simple[GLITZ_FRAGMENT_PROGRAM_TYPES];
  unsigned long fragment_convolution[GLITZ_FRAGMENT_PROGRAM_TYPES * 3];
  unsigned long fragment_programmatic[GLITZ_FRAGMENT_PROGRAMMATIC_PROGRAM_TYPES];
} glitz_programs_t;

typedef enum {
  GLITZ_CN_NONE,
  GLITZ_CN_ANY_CONTEXT_CURRENT,
  GLITZ_CN_SURFACE_CONTEXT_CURRENT,
  GLITZ_CN_SURFACE_DRAWABLE_CURRENT
} glitz_constraint_t;

typedef enum {
  GLITZ_TRIANGLE_TYPE_NORMAL,
  GLITZ_TRIANGLE_TYPE_STRIP,
  GLITZ_TRIANGLE_TYPE_FAN
} glitz_triangle_type_t;

typedef struct _glitz_region_box_t {
  int x1, x2, y1, y2;
} glitz_region_box_t;

typedef struct _glitz_sub_pixel_region_box_t {
  double x1, x2, y1, y2;
} glitz_sub_pixel_region_box_t;

typedef struct _glitz_point_t {
  double x, y;
} glitz_point_t;

typedef struct _glitz_matrix_t {
  double m[3][3];
} glitz_matrix_t;

typedef struct _glitz_texture {
  glitz_gl_uint_t name;
  glitz_gl_enum_t target;
  glitz_gl_enum_t format;
  glitz_gl_enum_t internal_format;
  glitz_bool_t allocated;
  
  glitz_filter_t filter;
  glitz_bool_t repeat;
  
  unsigned int width;
  unsigned int height;

  double texcoord_width;
  double texcoord_height;

  glitz_bool_t repeatable;
} glitz_texture_t;

typedef struct glitz_surface_backend {
  glitz_surface_t *
  (*create_similar) (void *surface,
                     glitz_format_name_t format_name,
                     glitz_bool_t drawable,
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
  (*get_texture) (void *surface);
  
  void
  (*update_size) (void *surface);

  void
  (*flush) (void *surface);
} glitz_surface_backend_t;

#define GLITZ_INT_HINT_REPEAT_MASK               (1L << 4)
#define GLITZ_INT_HINT_IMPLICIT_MASK_MASK        (1L << 5)
#define GLITZ_INT_HINT_DIRTY_MASK                (1L << 6)
#define GLITZ_INT_HINT_CLEAR_EXTERIOR_MASK       (1L << 7)

#define SURFACE_PROGRAMMATIC(surface) \
  (surface->hint_mask & GLITZ_HINT_PROGRAMMATIC_MASK)

#define SURFACE_SOLID(surface) \
  ((surface->hint_mask & GLITZ_HINT_PROGRAMMATIC_MASK) && \
   ((glitz_programmatic_surface_t *) surface)->type == \
   GLITZ_PROGRAMMATIC_SURFACE_SOLID_TYPE)

#define SURFACE_REPEAT(surface) \
  (surface->hint_mask & GLITZ_INT_HINT_REPEAT_MASK)

#define SURFACE_IMPLICIT_MASK(surface) \
  (surface->hint_mask & GLITZ_INT_HINT_IMPLICIT_MASK_MASK)

#define SURFACE_DIRTY(surface) \
  (surface->hint_mask & GLITZ_INT_HINT_DIRTY_MASK)

#define SURFACE_CLEAR_EXTERIOR(surface) \
  (surface->hint_mask & GLITZ_INT_HINT_CLEAR_EXTERIOR_MASK)

struct _glitz_surface {
  const glitz_surface_backend_t *backend;

  glitz_format_t *format;
  glitz_texture_t *texture;
  unsigned long status_mask;
  unsigned long feature_mask;
  glitz_filter_t filter;
  glitz_polyedge_t polyedge;
  glitz_matrix_t *transform;
  glitz_matrix_t *transforms;
  unsigned int n_transforms;
  int width, height;
  glitz_region_box_t dirty_region;
  glitz_gl_proc_address_list_t *gl;
  glitz_programs_t *programs;
  glitz_matrix_t *convolution;
  unsigned int clip_mask;
  unsigned long hint_mask;
};

#define GLITZ_COLOR_RANGE_UPDATE_TEXTURE_MASK (1L << 0)
#define GLITZ_COLOR_RANGE_UPDATE_FILTER_MASK  (1L << 1)
#define GLITZ_COLOR_RANGE_UPDATE_EXTEND_MASK  (1L << 2)
#define GLITZ_COLOR_RANGE_UPDATE_ALL_MASK     ((1L << 3) - 1)

struct _glitz_color_range {
  unsigned char *data;
  unsigned int size;
  glitz_gl_uint_t texture;
  glitz_filter_t filter;
  glitz_extend_t extend;
  unsigned long update_mask;
  unsigned int ref_count;
  glitz_gl_delete_textures_t delete_textures;
};

typedef struct _glitz_programmatic_surface_t {
  glitz_surface_t base;
  
  glitz_texture_t texture;
  glitz_matrix_t transform;
  
  glitz_programmatic_surface_type_t type;
  union {
    struct {
      glitz_color_t color;
    } solid;
    struct {
      glitz_point_fixed_t start;
      glitz_point_fixed_t stop;
      glitz_color_range_t *color_range;
    } linear;
    struct {
      glitz_point_fixed_t center;
      glitz_fixed16_16_t radius0;
      glitz_fixed16_16_t radius1;
      glitz_color_range_t *color_range;
    } radial;
  } u;
} glitz_programmatic_surface_t;

typedef struct _glitz_extension_map {
  char *name;
  int mask;
} glitz_extension_map;

extern void __internal_linkage
glitz_matrix_transform_point (glitz_matrix_t *matrix,
                              glitz_point_t *point);

extern void __internal_linkage
glitz_matrix_transform_region (glitz_matrix_t *matrix,
                               glitz_region_box_t *region);
     
extern void __internal_linkage
glitz_matrix_transform_sub_pixel_region (glitz_matrix_t *matrix,
                                         glitz_sub_pixel_region_box_t *region);

extern glitz_status_t __internal_linkage
glitz_matrix_invert (glitz_matrix_t *matrix);

extern void __internal_linkage
glitz_matrix_translate (glitz_matrix_t *matrix,
                        double tx,
                        double ty);

extern glitz_status_t __internal_linkage
glitz_matrix_normalize (glitz_matrix_t *matrix);

typedef enum glitz_int_operator {
  GLITZ_INT_OPERATOR_STENCIL_RECT_SET = 1000,
  GLITZ_INT_OPERATOR_STENCIL_RECT_SRC
} glitz_int_operator_t;

extern void __internal_linkage
glitz_set_operator (glitz_gl_proc_address_list_t *gl, glitz_operator_t op);

typedef enum glitz_int_clip_operator {
  GLITZ_INT_CLIP_OPERATOR_SET = GLITZ_CLIP_OPERATOR_SET,
  GLITZ_INT_CLIP_OPERATOR_UNION = GLITZ_CLIP_OPERATOR_UNION,
  GLITZ_INT_CLIP_OPERATOR_INTERSECT = GLITZ_CLIP_OPERATOR_INTERSECT,
  GLITZ_INT_CLIP_OPERATOR_INCR_INTERSECT,
  GLITZ_INT_CLIP_OPERATOR_DECR_INTERSECT,
  GLITZ_INT_CLIP_OPERATOR_CLIP
} glitz_int_clip_operator_t;

extern void __internal_linkage
glitz_set_clip_operator (glitz_gl_proc_address_list_t *gl,
                         glitz_int_clip_operator_t op,
                         int mask);

extern void __internal_linkage
glitz_intersect_region (glitz_region_box_t *box1,
                        glitz_region_box_t *box2,
                        glitz_region_box_t *return_box);

extern void __internal_linkage
glitz_union_region (glitz_region_box_t *box1,
                    glitz_region_box_t *box2,
                    glitz_region_box_t *return_box);

extern void __internal_linkage
glitz_intersect_sub_pixel_region (glitz_sub_pixel_region_box_t *box1,
                                  glitz_sub_pixel_region_box_t *box2,
                                  glitz_sub_pixel_region_box_t *return_box);

extern void __internal_linkage
glitz_union_sub_pixel_region (glitz_sub_pixel_region_box_t *box1,
                              glitz_sub_pixel_region_box_t *box2,
                              glitz_sub_pixel_region_box_t *return_box);

glitz_gl_enum_t
glitz_get_gl_format_from_bpp (unsigned short bpp);

extern glitz_gl_enum_t __internal_linkage
glitz_get_gl_data_type_from_bpp (unsigned short bpp);

long int
glitz_extensions_query (const char *extensions_string,
                        glitz_extension_map *extensions_map);

extern glitz_bool_t __internal_linkage
glitz_uint_is_power_of_two (unsigned int value);

extern void __internal_linkage
glitz_uint_to_power_of_two (unsigned int *value);

glitz_texture_t *
glitz_texture_generate (glitz_gl_proc_address_list_t *gl,
                        unsigned int width,
                        unsigned int height,
                        unsigned int texture_format,
                        long int target_mask);

void
glitz_texture_allocate (glitz_gl_proc_address_list_t *gl,
                        glitz_texture_t *texture);

void
glitz_texture_destroy (glitz_gl_proc_address_list_t *gl,
                       glitz_texture_t *texture);

extern void __internal_linkage
glitz_texture_ensure_filter (glitz_gl_proc_address_list_t *gl,
                             glitz_texture_t *texture,
                             glitz_filter_t filter);

extern void __internal_linkage
glitz_texture_ensure_repeat (glitz_gl_proc_address_list_t *gl,
                             glitz_texture_t *texture,
                             glitz_bool_t repeat);

void
glitz_texture_bind (glitz_gl_proc_address_list_t *gl,
                    glitz_texture_t *texture);

void
glitz_texture_unbind (glitz_gl_proc_address_list_t *gl,
                      glitz_texture_t *texture);

void
glitz_texture_copy_surface (glitz_texture_t *texture,
                            glitz_surface_t *surface,
                            glitz_region_box_t *region);

void
glitz_surface_init (glitz_surface_t *surface,
                    const glitz_surface_backend_t *backend);

extern void __internal_linkage
glitz_surface_push_transform (glitz_surface_t *surface);

extern void __internal_linkage
glitz_surface_pop_transform (glitz_surface_t *surface);

void
glitz_surface_deinit (glitz_surface_t *surface);

extern glitz_texture_t *__internal_linkage
glitz_surface_get_texture (glitz_surface_t *surface);

extern void __internal_linkage
glitz_surface_destory (glitz_surface_t *surface);

glitz_bool_t
glitz_surface_push_current (glitz_surface_t *surface,
                            glitz_constraint_t constraint);

void
glitz_surface_pop_current (glitz_surface_t *surface);

extern void __internal_linkage
glitz_surface_bounds (glitz_surface_t *surface,
                      glitz_region_box_t *box);

extern void __internal_linkage
glitz_surface_enable_program (glitz_program_type_t type,
                              glitz_surface_t *surface,
                              glitz_surface_t *src,
                              glitz_surface_t *mask,
                              glitz_texture_t *src_texture,
                              glitz_texture_t *mask_texture);

extern void __internal_linkage
glitz_surface_disable_program (glitz_program_type_t type,
                               glitz_surface_t *surface);

extern void __internal_linkage
glitz_surface_dirty (glitz_surface_t *surface,
                     glitz_region_box_t *region);

extern void __internal_linkage
glitz_surface_status_add (glitz_surface_t *surface, int flags);

void
glitz_surface_setup_environment (glitz_surface_t *surface);

extern glitz_status_t __internal_linkage
glitz_status_pop_from_mask (unsigned long *mask);

extern glitz_surface_t *__internal_linkage
glitz_int_surface_create_similar (glitz_surface_t *templ,
                                  glitz_format_name_t format_name,
                                  glitz_bool_t drawable,
                                  int width,
                                  int height);

extern void __internal_linkage
glitz_int_surface_clip_rectangles (glitz_surface_t *surface,
                                   glitz_int_clip_operator_t op,
                                   int mask,
                                   const glitz_rectangle_t *rects,
                                   int n_rects);

extern void __internal_linkage
glitz_int_surface_clip_trapezoids (glitz_surface_t *surface,
                                   glitz_int_clip_operator_t op,
                                   int mask,
                                   const glitz_trapezoid_t *traps,
                                   int n_traps);

extern void __internal_linkage
glitz_int_surface_clip_triangles (glitz_surface_t *surface,
                                  glitz_int_clip_operator_t op,
                                  int mask,
                                  glitz_triangle_type_t type,
                                  const glitz_point_fixed_t *points,
                                  int n_points);

glitz_format_t *
glitz_format_find (glitz_format_t *formats,
                   int n_formats,
                   unsigned long mask,
                   const glitz_format_t *templ,
                   int count);

glitz_format_t *
glitz_format_find_standard (glitz_format_t *formats,
                            int n_formats,
                            unsigned long options,
                            glitz_format_name_t format_name);

void
glitz_format_calculate_pixel_transfer_info (glitz_format_t *format);

extern glitz_program_type_t __internal_linkage
glitz_program_type (glitz_surface_t *dst,
                    glitz_surface_t *src,
                    glitz_surface_t *mask);

extern void __internal_linkage
glitz_program_enable (glitz_program_type_t type,
                      glitz_surface_t *dst,
                      glitz_surface_t *src,
                      glitz_surface_t *mask,
                      glitz_texture_t *src_texture,
                      glitz_texture_t *mask_texture);

extern void __internal_linkage
glitz_program_disable (glitz_program_type_t type,
                       glitz_surface_t *dst);

extern void __internal_linkage
glitz_programmatic_surface_setup (glitz_surface_t *abstract_surface,
                                  int width,
                                  int height);

extern void __internal_linkage
glitz_programmatic_surface_bind (glitz_gl_proc_address_list_t *proc_address,
                                 glitz_programmatic_surface_t *surface,
                                 unsigned long feature_mask);

extern glitz_surface_t *__internal_linkage
glitz_programmatic_surface_create_solid (glitz_color_t *color);

extern glitz_surface_t *__internal_linkage
glitz_programmatic_surface_create_linear (glitz_point_fixed_t *start,
                                          glitz_point_fixed_t *stop,
                                          glitz_color_range_t *color_range);

extern glitz_surface_t *__internal_linkage
glitz_programmatic_surface_create_radial (glitz_point_fixed_t *start,
                                          glitz_fixed16_16_t radius0,
                                          glitz_fixed16_16_t radius1,
                                          glitz_color_range_t *color_range);

extern void __internal_linkage
glitz_programmatic_surface_set_transform (glitz_surface_t *surface,
                                          glitz_transform_t *transform);

extern void __internal_linkage
glitz_color_range_bind (glitz_gl_proc_address_list_t *gl,
                        glitz_color_range_t *color_range,
                        unsigned long feature_mask);

extern void __internal_linkage
glitz_color_range_reference (glitz_color_range_t *color_range);

extern void __internal_linkage
glitz_int_fill_rectangles (glitz_operator_t op,
                           glitz_surface_t *dst,
                           const glitz_color_t *color,
                           const glitz_rectangle_t *rects,
                           int n_rects);

extern void __internal_linkage
glitz_int_fill_trapezoids (glitz_operator_t op,
                           glitz_surface_t *dst,
                           int x_offset,
                           int y_offset,
                           const glitz_color_t *color,
                           const glitz_trapezoid_t *traps,
                           int n_traps);

extern void __internal_linkage
glitz_int_fill_triangles (glitz_operator_t op,
                          glitz_surface_t *dst,
                          glitz_triangle_type_t type,
                          int x_offset,
                          int y_offset,
                          const glitz_color_t *color,
                          const glitz_point_fixed_t *points,
                          int n_points);

#define MAXSHORT SHRT_MAX
#define MINSHORT SHRT_MIN

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

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
#define FIXED1 (INT_TO_FIXED(1))
#define FIXED1_MINUS_E (FIXED1 - FIXED_E)
#define FIXED_FRAC(f) ((f) & FIXED1_MINUS_E)
#define FIXED_FLOOR(f) ((f) & ~FIXED1_MINUS_E)
#define FIXED_CEIL(f) FIXED_FLOOR((f) + FIXED1_MINUS_E)

#define FIXED_FRACTION(f) ((f) & FIXED1_MINUS_E)
#define FIXED_MOD2(f) ((f) & (FIXED1 | FIXED1_MINUS_E))

#define FIXED_TO_DOUBLE(f) (((double) (f)) / 65536)
#define DOUBLE_TO_FIXED(f) ((int) ((f) * 65536))


/* Avoid unnecessary PLT entries.  */

slim_hidden_proto(glitz_surface_create_similar)
slim_hidden_proto(glitz_surface_create_solid)
slim_hidden_proto(glitz_surface_create_linear)
slim_hidden_proto(glitz_surface_create_radial)
slim_hidden_proto(glitz_surface_set_transform)
slim_hidden_proto(glitz_surface_set_convolution)
slim_hidden_proto(glitz_surface_set_repeat)
slim_hidden_proto(glitz_surface_set_filter)
slim_hidden_proto(glitz_surface_set_polyedge)
slim_hidden_proto(glitz_surface_get_width)
slim_hidden_proto(glitz_surface_get_height)
slim_hidden_proto(glitz_surface_update_size)
slim_hidden_proto(glitz_surface_flush)
slim_hidden_proto(glitz_surface_read_pixels)
slim_hidden_proto(glitz_surface_draw_pixels)
slim_hidden_proto(glitz_surface_get_status)
slim_hidden_proto(glitz_surface_get_gl_texture)
slim_hidden_proto(glitz_surface_gl_begin)
slim_hidden_proto(glitz_surface_gl_end)
slim_hidden_proto(glitz_surface_get_features)
slim_hidden_proto(glitz_surface_clip_rectangles)
slim_hidden_proto(glitz_surface_clip_trapezoids)
slim_hidden_proto(glitz_surface_clip_triangles)
slim_hidden_proto(glitz_surface_get_format)
slim_hidden_proto(glitz_surface_get_hints)
slim_hidden_proto(glitz_fill_rectangle)
slim_hidden_proto(glitz_fill_rectangles)
slim_hidden_proto(glitz_fill_trapezoids)
slim_hidden_proto(glitz_composite_trapezoids)
slim_hidden_proto(glitz_color_trapezoids)
slim_hidden_proto(glitz_fill_triangles)
slim_hidden_proto(glitz_composite_triangles)
slim_hidden_proto(glitz_composite_tri_strip)
slim_hidden_proto(glitz_composite_tri_fan)
slim_hidden_proto(glitz_color_triangles)
slim_hidden_proto(glitz_color_range_create)
slim_hidden_proto(glitz_color_range_get_data)
slim_hidden_proto(glitz_color_range_put_back_data)
slim_hidden_proto(glitz_color_range_set_filter)
slim_hidden_proto(glitz_color_range_set_extend)     

#endif /* GLITZINT_H_INCLUDED */
