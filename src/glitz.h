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

#ifndef GLITZ_H_INCLUDED
#define GLITZ_H_INCLUDED

#if defined(__SVR4) && defined(__sun)
#  include <sys/int_types.h>
#else
#  if defined(__OpenBSD__)
#    include <inttypes.h>
#  else
#    include <stdint.h>
#  endif
#endif

/* NOTE: Must be manually synchronized with GLITZ_VERSION in configure.in */
#define GLITZ_MAJOR 0
#define GLITZ_MINOR 1
#define GLITZ_REVISION 3

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef int glitz_bool_t;
typedef int glitz_fixed16_16_t;

typedef struct _glitz_point_fixed_t {
  glitz_fixed16_16_t x, y;
} glitz_point_fixed_t;

typedef struct _glitz_line_fixed_t {
  glitz_point_fixed_t p1, p2;
} glitz_line_fixed_t;

typedef struct _glitz_rectangle_t {
  short x, y;
  unsigned short width, height;
} glitz_rectangle_t;

typedef struct _glitz_triangle_t {
  glitz_point_fixed_t p1, p2, p3;
} glitz_triangle_t;

typedef struct _glitz_trapezoid_t {
  glitz_fixed16_16_t top, bottom;
  glitz_line_fixed_t left, right;
} glitz_trapezoid_t;

typedef struct _glitz_transform_t {
  glitz_fixed16_16_t matrix[3][3];
} glitz_transform_t;

typedef struct _glitz_convolution_t {
  glitz_fixed16_16_t matrix[3][3];
} glitz_convolution_t;

typedef struct {
  unsigned short red;
  unsigned short green;
  unsigned short blue;
  unsigned short alpha;
} glitz_color_t;
  
typedef struct _glitz_colorspan_t {
  glitz_fixed16_16_t left, right, y;
  glitz_color_t left_color;
  glitz_color_t right_color;
} glitz_colorspan_t;

typedef struct _glitz_colorpoint_t {
  glitz_point_fixed_t point;
  glitz_color_t color;
} glitz_colorpoint_t;

typedef struct _glitz_color_trapezoid_t {
  glitz_colorspan_t top, bottom;
} glitz_color_trapezoid_t;

typedef struct _glitz_color_triangle_t {
  glitz_colorpoint_t p1, p2, p3;
} glitz_color_triangle_t;

typedef enum {
  GLITZ_FILTER_FAST,
  GLITZ_FILTER_GOOD,
  GLITZ_FILTER_BEST,
  GLITZ_FILTER_NEAREST,
  GLITZ_FILTER_BILINEAR
} glitz_filter_t;

typedef enum {
  GLITZ_OPERATOR_CLEAR,
  GLITZ_OPERATOR_SRC,
  GLITZ_OPERATOR_DST,
  GLITZ_OPERATOR_OVER,
  GLITZ_OPERATOR_OVER_REVERSE,
  GLITZ_OPERATOR_IN,
  GLITZ_OPERATOR_IN_REVERSE,
  GLITZ_OPERATOR_OUT,
  GLITZ_OPERATOR_OUT_REVERSE,
  GLITZ_OPERATOR_ATOP,
  GLITZ_OPERATOR_ATOP_REVERSE,
  GLITZ_OPERATOR_XOR,
  GLITZ_OPERATOR_ADD,
  GLITZ_OPERATOR_SATURATE
} glitz_operator_t;

#define GLITZ_FEATURE_OFFSCREEN_DRAWING_MASK       (1L << 0)
#define GLITZ_FEATURE_CONVOLUTION_FILTER_MASK      (1L << 1)
#define GLITZ_FEATURE_TEXTURE_RECTANGLE_MASK       (1L << 2)
#define GLITZ_FEATURE_TEXTURE_NPOT_MASK            (1L << 3)
#define GLITZ_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK (1L << 4)
#define GLITZ_FEATURE_MULTISAMPLE_MASK             (1L << 5)
#define GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK   (1L << 6)
#define GLITZ_FEATURE_ARB_MULTITEXTURE_MASK        (1L << 7)
#define GLITZ_FEATURE_ARB_VERTEX_PROGRAM_MASK      (1L << 8)
#define GLITZ_FEATURE_ARB_FRAGMENT_PROGRAM_MASK    (1L << 9)

typedef enum {  
  GLITZ_STANDARD_ARGB32,
  GLITZ_STANDARD_RGB24,
  GLITZ_STANDARD_A8,
  GLITZ_STANDARD_A1
} glitz_format_name_t;
  
#define GLITZ_FORMAT_ID_MASK                  (1L <<  0)
#define GLITZ_FORMAT_BPP_MASK                 (1L <<  1)
#define GLITZ_FORMAT_RED_MASK_MASK            (1L <<  2)
#define GLITZ_FORMAT_GREEN_MASK_MASK          (1L <<  3)
#define GLITZ_FORMAT_BLUE_MASK_MASK           (1L <<  4)
#define GLITZ_FORMAT_ALPHA_MASK_MASK          (1L <<  5)
#define GLITZ_FORMAT_RED_SIZE_MASK            (1L <<  6)
#define GLITZ_FORMAT_GREEN_SIZE_MASK          (1L <<  7)
#define GLITZ_FORMAT_BLUE_SIZE_MASK           (1L <<  8)
#define GLITZ_FORMAT_ALPHA_SIZE_MASK          (1L <<  9)
#define GLITZ_FORMAT_DEPTH_SIZE_MASK          (1L << 10)
#define GLITZ_FORMAT_STENCIL_SIZE_MASK        (1L << 11)
#define GLITZ_FORMAT_DOUBLEBUFFER_MASK        (1L << 12)
#define GLITZ_FORMAT_READ_ONSCREEN_MASK       (1L << 13)
#define GLITZ_FORMAT_READ_OFFSCREEN_MASK      (1L << 14)
#define GLITZ_FORMAT_DRAW_ONSCREEN_MASK       (1L << 15)
#define GLITZ_FORMAT_DRAW_OFFSCREEN_MASK      (1L << 16)
#define GLITZ_FORMAT_MULTISAMPLE_MASK         (1L << 17)
#define GLITZ_FORMAT_MULTISAMPLE_SAMPLES_MASK (1L << 18)

typedef unsigned long int glitz_format_id_t;

typedef struct _glitz_drawable_type_t {
  glitz_bool_t onscreen;
  glitz_bool_t offscreen;
} glitz_drawable_type_t;

typedef struct _glitz_multisample_format_t {
  glitz_bool_t supported;
  unsigned short samples;
} glitz_multisample_format_t;

typedef struct _glitz_format_t {
  glitz_format_id_t id;

  /* bpp and mask values specifies the pixel format for read/draw pixels */
  int bpp;
  unsigned long red_mask;
  unsigned long green_mask;
  unsigned long blue_mask;
  unsigned long alpha_mask;
  
  unsigned short red_size;
  unsigned short green_size;
  unsigned short blue_size;
  unsigned short alpha_size;
  unsigned short depth_size;
  unsigned short stencil_size;
  
  glitz_bool_t doublebuffer;
  glitz_drawable_type_t read;
  glitz_drawable_type_t draw;
  glitz_multisample_format_t multisample;
} glitz_format_t;

#define GLITZ_FORMAT_OPTION_DOUBLEBUFFER_MASK   (1L << 0)
#define GLITZ_FORMAT_OPTION_SINGLEBUFFER_MASK   (1L << 1)
#define GLITZ_FORMAT_OPTION_ONSCREEN_MASK       (1L << 2)
#define GLITZ_FORMAT_OPTION_OFFSCREEN_MASK      (1L << 3)
#define GLITZ_FORMAT_OPTION_MULTISAMPLE_MASK    (1L << 4)
#define GLITZ_FORMAT_OPTION_NO_MULTISAMPLE_MASK (1L << 5)
#define GLITZ_FORMAT_OPTION_READONLY_MASK       (1L << 6)

/* glitz_status.c */
  
typedef enum {
    GLITZ_STATUS_SUCCESS = 0,
    GLITZ_STATUS_NO_MEMORY,
    GLITZ_STATUS_NULL_POINTER,
    GLITZ_STATUS_BAD_COORDINATE,
    GLITZ_STATUS_NOT_SUPPORTED,
    GLITZ_STATUS_INVALID_MATRIX
} glitz_status_t;

const char *
glitz_status_string (glitz_status_t status);

/* glitz_color_range.c */

typedef struct _glitz_color_range glitz_color_range_t;

typedef enum {
  GLITZ_EXTEND_PAD,
  GLITZ_EXTEND_REPEAT,
  GLITZ_EXTEND_REFLECT
} glitz_extend_t;
  
glitz_color_range_t *
glitz_color_range_create (unsigned int size);

void
glitz_color_range_destroy (glitz_color_range_t *color_range);

unsigned char *
glitz_color_range_get_data (glitz_color_range_t *color_range);

void
glitz_color_range_put_back_data (glitz_color_range_t *color_range);

void
glitz_color_range_set_filter (glitz_color_range_t *color_range,
                              glitz_filter_t filter);
  
void
glitz_color_range_set_extend (glitz_color_range_t *color_range,
                              glitz_extend_t extend);

  
/* glitz_surface.c */

typedef struct _glitz_surface glitz_surface_t;

typedef enum {
  GLITZ_POLYEDGE_SHARP,
  GLITZ_POLYEDGE_SMOOTH
} glitz_polyedge_t;

glitz_surface_t *
glitz_surface_create_solid (glitz_color_t *color);
  
glitz_surface_t *
glitz_surface_create_linear (glitz_point_fixed_t *start,
                             glitz_point_fixed_t *end,
                             glitz_color_range_t *color_range);

glitz_surface_t *
glitz_surface_create_radial (glitz_point_fixed_t *center,
                             glitz_fixed16_16_t radius0,
                             glitz_fixed16_16_t radius1,
                             glitz_color_range_t *color_range);
  
void
glitz_surface_destroy (glitz_surface_t *surface);

void
glitz_surface_set_transform (glitz_surface_t *surface,
                             glitz_transform_t *transform);

void
glitz_surface_set_convolution (glitz_surface_t *surface,
                               glitz_convolution_t *convolution);

void
glitz_surface_set_repeat (glitz_surface_t *surface,
                          glitz_bool_t repeat);

void
glitz_surface_set_filter (glitz_surface_t *surface,
                          glitz_filter_t filter);
  
void
glitz_surface_set_polyedge (glitz_surface_t *surface,
                            glitz_polyedge_t polyedge);

void
glitz_surface_set_polyopacity (glitz_surface_t *surface,
                               unsigned short polyopacity);

typedef enum {
  GLITZ_CLIP_OPERATOR_SET,
  GLITZ_CLIP_OPERATOR_UNION,
  GLITZ_CLIP_OPERATOR_INTERSECT
} glitz_clip_operator_t;
  
void
glitz_surface_clip_rectangles (glitz_surface_t *surface,
                               glitz_clip_operator_t op,
                               const glitz_rectangle_t *rects,
                               int n_rects);
  
void
glitz_surface_clip_trapezoids (glitz_surface_t *surface,
                               glitz_clip_operator_t op,
                               const glitz_trapezoid_t *traps,
                               int n_traps);

void
glitz_surface_clip_triangles (glitz_surface_t *surface,
                              glitz_clip_operator_t op,
                              const glitz_triangle_t *tris,
                              int n_tris);
  
int
glitz_surface_get_width (glitz_surface_t *surface);

int
glitz_surface_get_height (glitz_surface_t *surface);

void
glitz_surface_update_size (glitz_surface_t *surface);

typedef enum {
  GLITZ_BUFFER_FRONT,
  GLITZ_BUFFER_BACK
} glitz_buffer_t;

void
glitz_surface_set_read_buffer (glitz_surface_t *surface,
                               glitz_buffer_t buffer);

void
glitz_surface_set_draw_buffer (glitz_surface_t *surface,
                               glitz_buffer_t buffer);

void
glitz_surface_flush (glitz_surface_t *surface);
  
void
glitz_surface_swap_buffers (glitz_surface_t *surface);

void
glitz_surface_read_pixels (glitz_surface_t *surface,
                           int x,
                           int y,
                           unsigned int width,
                           unsigned int height,
                           char *pixels);

void
glitz_surface_draw_pixels (glitz_surface_t *surface,
                           int x,
                           int y,
                           unsigned int width,
                           unsigned int height,
                           char *pixels);

void
glitz_surface_get_gl_texture (glitz_surface_t *surface,
                              unsigned int *name,
                              unsigned int *target,
                              double *texcoord_width,
                              double *texcoord_height,
                              glitz_bool_t *repeatable);  

void
glitz_surface_gl_begin (glitz_surface_t *surface);

void
glitz_surface_gl_end (glitz_surface_t *surface);

glitz_status_t
glitz_surface_get_status (glitz_surface_t *surface);

unsigned long
glitz_surface_get_features (glitz_surface_t *surface);

glitz_format_t *
glitz_surface_get_format (glitz_surface_t *surface);

glitz_format_t *
glitz_surface_find_similar_standard_format (glitz_surface_t *surface,
                                            unsigned long option_mask,
                                            glitz_format_name_t format_name);

glitz_format_t *
glitz_surface_find_similar_format (glitz_surface_t *surface,
                                   unsigned long mask,
                                   const glitz_format_t *templ,
                                   int count);

glitz_surface_t *
glitz_surface_create_similar (glitz_surface_t *templ,
                              glitz_format_t *format,
                              int width,
                              int height);

#define GLITZ_HINT_CLIPPING_MASK     (1L << 0)
#define GLITZ_HINT_OFFSCREEN_MASK    (1L << 1)
#define GLITZ_HINT_PROGRAMMATIC_MASK (1L << 2)

unsigned long
glitz_surface_get_hints (glitz_surface_t *surface);
  
  
/* glitz_rect.c */

void
glitz_fill_rectangle (glitz_operator_t op,
                      glitz_surface_t *dst,
                      const glitz_color_t *color,
                      int x,
                      int y,
                      unsigned int width,
                      unsigned int height);
  
void
glitz_fill_rectangles (glitz_operator_t op,
                       glitz_surface_t *dst,
                       const glitz_color_t *color,
                       const glitz_rectangle_t *rects,
                       int n_rects);


/* glitz_trap.c */

void
glitz_fill_trapezoids (glitz_operator_t op,
                       glitz_surface_t *dst,
                       const glitz_color_t *color,
                       const glitz_trapezoid_t *traps,
                       int n_traps);

void
glitz_composite_trapezoids (glitz_operator_t op,
                            glitz_surface_t *src,
                            glitz_surface_t *dst,
                            int x_src,
                            int y_src,
                            const glitz_trapezoid_t *traps,
                            int n_traps);

void
glitz_color_trapezoids (glitz_operator_t op,
                        glitz_surface_t *dst,
                        const glitz_color_trapezoid_t *color_traps,
                        int n_color_traps);


/* glitz_tri.c */

void
glitz_fill_triangles (glitz_operator_t op,
                      glitz_surface_t *dst,
                      const glitz_color_t *color,
                      const glitz_triangle_t *tris,
                      int n_tris);
  
void
glitz_composite_triangles (glitz_operator_t op,
                           glitz_surface_t *src,
                           glitz_surface_t *dst,
                           int x_src,
                           int y_src,
                           const glitz_triangle_t *tris,
                           int n_tris);

void
glitz_composite_tri_strip (glitz_operator_t op,
                           glitz_surface_t *src,
                           glitz_surface_t *dst,
                           int x_src,
                           int y_src,
                           const glitz_point_fixed_t *points,
                           int n_points);

  void
glitz_composite_tri_fan (glitz_operator_t op,
                         glitz_surface_t *src,
                         glitz_surface_t *dst,
                         int x_src,
                         int y_src,
                         const glitz_point_fixed_t *points,
                         int n_points);
  
void
glitz_color_triangles (glitz_operator_t op,
                       glitz_surface_t *dst,
                       const glitz_color_triangle_t *color_tris,
                       int n_color_tris);
  

/* glitz.c */

void
glitz_composite (glitz_operator_t op,
                 glitz_surface_t *src,
                 glitz_surface_t *mask,
                 glitz_surface_t *dst,
                 int x_src,
                 int y_src,
                 int x_mask,
                 int y_mask,
                 int x_dst,
                 int y_dst,
                 int width,
                 int height);

void
glitz_copy_area (glitz_surface_t *src,
                 glitz_surface_t *dst,
                 int x_src,
                 int y_src,
                 int width,
                 int height,
                 int x_dst,
                 int y_dst);
  
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* GLITZ_H_INCLUDED */
