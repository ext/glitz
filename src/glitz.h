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
#define GLITZ_MINOR 2
#define GLITZ_REVISION 2

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef int glitz_bool_t;
typedef short glitz_short_t;
typedef int glitz_int_t;
typedef float glitz_float_t;
typedef double glitz_double_t;
typedef int glitz_fixed16_16_t;

typedef struct _glitz_rectangle_t {
  short x, y;
  unsigned short width, height;
} glitz_rectangle_t;

typedef struct _glitz_transform_t {
  glitz_fixed16_16_t matrix[3][3];
} glitz_transform_t;

typedef struct {
  unsigned short red;
  unsigned short green;
  unsigned short blue;
  unsigned short alpha;
} glitz_color_t;
  
typedef enum {
  GLITZ_FILTER_NEAREST,
  GLITZ_FILTER_BILINEAR,
  GLITZ_FILTER_CONVOLUTION,
  GLITZ_FILTER_GAUSSIAN,
  GLITZ_FILTER_LINEAR_GRADIENT,
  GLITZ_FILTER_RADIAL_GRADIENT
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

#define GLITZ_FEATURE_TEXTURE_RECTANGLE_MASK        (1L <<  0)
#define GLITZ_FEATURE_TEXTURE_NON_POWER_OF_TWO_MASK (1L <<  1)
#define GLITZ_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK  (1L <<  2)
#define GLITZ_FEATURE_TEXTURE_BORDER_CLAMP_MASK     (1L <<  3)
#define GLITZ_FEATURE_MULTISAMPLE_MASK              (1L <<  4)
#define GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK    (1L <<  5)
#define GLITZ_FEATURE_MULTISAMPLE_FILTER_HINT_MASK  (1L <<  6)
#define GLITZ_FEATURE_MULTITEXTURE_MASK             (1L <<  7)
#define GLITZ_FEATURE_TEXTURE_ENV_COMBINE_MASK      (1L <<  8)
#define GLITZ_FEATURE_TEXTURE_ENV_DOT3_MASK         (1L <<  9)
#define GLITZ_FEATURE_FRAGMENT_PROGRAM_MASK         (1L << 10)
#define GLITZ_FEATURE_VERTEX_BUFFER_OBJECT_MASK     (1L << 11)
#define GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK      (1L << 12)
#define GLITZ_FEATURE_PER_COMPONENT_RENDERING_MASK  (1L << 13)
#define GLITZ_FEATURE_BLEND_COLOR_MASK              (1L << 14)

typedef enum {
  GLITZ_STANDARD_ARGB32,
  GLITZ_STANDARD_RGB24,
  GLITZ_STANDARD_A8,
  GLITZ_STANDARD_A1
} glitz_format_name_t;
  
#define GLITZ_FORMAT_ID_MASK                  (1L <<  0)
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

typedef struct _glitz_surface glitz_surface_t;
typedef struct _glitz_buffer glitz_buffer_t;

  
/* glitz_status.c */
  
typedef enum {
    GLITZ_STATUS_SUCCESS = 0,
    GLITZ_STATUS_NO_MEMORY,
    GLITZ_STATUS_BAD_COORDINATE,
    GLITZ_STATUS_NOT_SUPPORTED,
    GLITZ_STATUS_CONTENT_DESTROYED
} glitz_status_t;

const char *
glitz_status_string (glitz_status_t status);

  
/* glitz_surface.c */

void
glitz_surface_destroy (glitz_surface_t *surface);

void
glitz_surface_reference (glitz_surface_t *surface);

void
glitz_surface_set_transform (glitz_surface_t *surface,
                             glitz_transform_t *transform);

typedef enum {
  GLITZ_FILL_TRANSPARENT,
  GLITZ_FILL_NEAREST,
  GLITZ_FILL_REPEAT,
  GLITZ_FILL_REFLECT
} glitz_fill_t;

void
glitz_surface_set_fill (glitz_surface_t *surface,
                        glitz_fill_t fill);

void
glitz_surface_set_component_alpha (glitz_surface_t *surface,
                                   glitz_bool_t component_alpha);

void
glitz_surface_set_filter (glitz_surface_t *surface,
                          glitz_filter_t filter,
                          glitz_fixed16_16_t *params,
                          int n_params);  

typedef enum {
  GLITZ_COLOR_BUFFER_FRONT,
  GLITZ_COLOR_BUFFER_BACK
} glitz_color_buffer_t;

void
glitz_surface_set_read_color_buffer (glitz_surface_t *surface,
                                     glitz_color_buffer_t buffer);

void
glitz_surface_set_draw_color_buffer (glitz_surface_t *surface,
                                     glitz_color_buffer_t buffer);
  
void
glitz_surface_swap_buffers (glitz_surface_t *surface);

void
glitz_surface_flush (glitz_surface_t *surface);

void
glitz_surface_finish (glitz_surface_t *surface);

int
glitz_surface_get_width (glitz_surface_t *surface);

int
glitz_surface_get_height (glitz_surface_t *surface);

glitz_status_t
glitz_surface_get_status (glitz_surface_t *surface);

unsigned long
glitz_surface_get_features (glitz_surface_t *surface);

glitz_format_t *
glitz_surface_get_format (glitz_surface_t *surface);

glitz_format_t *
glitz_surface_find_similar_standard_format (glitz_surface_t *surface,
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

  
/* glitz_rect.c */

void
glitz_set_rectangle (glitz_surface_t *dst,
                     const glitz_color_t *color,
                     int x,
                     int y,
                     unsigned int width,
                     unsigned int height);
  
void
glitz_set_rectangles (glitz_surface_t *dst,
                      const glitz_color_t *color,
                      const glitz_rectangle_t *rects,
                      int n_rects);


/* glitz_buffer.c */
  
typedef enum {
  GLITZ_BUFFER_HINT_STREAM_DRAW,
  GLITZ_BUFFER_HINT_STREAM_READ,
  GLITZ_BUFFER_HINT_STREAM_COPY,
  GLITZ_BUFFER_HINT_STATIC_DRAW,
  GLITZ_BUFFER_HINT_STATIC_READ,
  GLITZ_BUFFER_HINT_STATIC_COPY,
  GLITZ_BUFFER_HINT_DYNAMIC_DRAW,
  GLITZ_BUFFER_HINT_DYNAMIC_READ,
  GLITZ_BUFFER_HINT_DYNAMIC_COPY
} glitz_buffer_hint_t;

typedef enum {
  GLITZ_BUFFER_ACCESS_READ_ONLY,
  GLITZ_BUFFER_ACCESS_WRITE_ONLY,
  GLITZ_BUFFER_ACCESS_READ_WRITE
} glitz_buffer_access_t;

glitz_buffer_t *
glitz_geometry_buffer_create (glitz_surface_t *surface,
                              void *data,
                              unsigned int size,
                              glitz_buffer_hint_t hint);

glitz_buffer_t *
glitz_pixel_buffer_create (glitz_surface_t *surface,
                           void *data,
                           unsigned int size,
                           glitz_buffer_hint_t hint);

glitz_buffer_t *
glitz_buffer_create_for_data (void *data);
  
void
glitz_buffer_destroy (glitz_buffer_t *buffer);

void
glitz_buffer_reference (glitz_buffer_t *buffer);

void
glitz_buffer_set_data (glitz_buffer_t *buffer,
                       int offset,
                       unsigned int size,
                       const void *data);

void
glitz_buffer_get_data (glitz_buffer_t *buffer,
                       int offset,
                       unsigned int size,
                       void *data);
       
void *
glitz_buffer_map (glitz_buffer_t *buffer,
                  glitz_buffer_access_t access);
  
glitz_status_t
glitz_buffer_unmap (glitz_buffer_t *buffer);

  
/* glitz_pixel.c */
  
typedef enum {
  GLITZ_PIXEL_SCANLINE_ORDER_TOP_DOWN,
  GLITZ_PIXEL_SCANLINE_ORDER_BOTTOM_UP
} glitz_pixel_scanline_order_t;

typedef struct _glitz_pixel_masks {
  int bpp;
  unsigned long alpha_mask;
  unsigned long red_mask;
  unsigned long green_mask;
  unsigned long blue_mask;
} glitz_pixel_masks_t;

typedef struct _glitz_pixel_format {
  glitz_pixel_masks_t masks;
  int xoffset;
  int skip_lines;
  int bytes_per_line;
  glitz_pixel_scanline_order_t scanline_order;
} glitz_pixel_format_t;

void
glitz_set_pixels (glitz_surface_t *dst,
                  int x_dst,
                  int y_dst,
                  int width,
                  int height,
                  glitz_pixel_format_t *format,
                  glitz_buffer_t *buffer);                

void
glitz_get_pixels (glitz_surface_t *src,
                  int x_src,
                  int y_src,
                  int width,
                  int height,
                  glitz_pixel_format_t *format,
                  glitz_buffer_t *buffer);
  
  
/* glitz_geometry.c */

typedef enum {
  GLITZ_GEOMETRY_MODE_DIRECT,
  GLITZ_GEOMETRY_MODE_INDIRECT
} glitz_geometry_mode_t;

typedef enum {
  GLITZ_GEOMETRY_EDGE_HINT_SHARP,
  GLITZ_GEOMETRY_EDGE_HINT_FAST_SMOOTH,
  GLITZ_GEOMETRY_EDGE_HINT_GOOD_SMOOTH,
  GLITZ_GEOMETRY_EDGE_HINT_BEST_SMOOTH
} glitz_geometry_edge_hint_t;

typedef enum {
  GLITZ_GEOMETRY_PRIMITIVE_TRIANGLES,
  GLITZ_GEOMETRY_PRIMITIVE_TRIANGLE_STRIP,
  GLITZ_GEOMETRY_PRIMITIVE_TRIANGLE_FAN,
  GLITZ_GEOMETRY_PRIMITIVE_QUADS,
  GLITZ_GEOMETRY_PRIMITIVE_QUAD_STRIP,
  GLITZ_GEOMETRY_PRIMITIVE_POLYGON
} glitz_geometry_primitive_t;
  
typedef enum {
  GLITZ_DATA_TYPE_SHORT,
  GLITZ_DATA_TYPE_INT,
  GLITZ_DATA_TYPE_FLOAT,
  GLITZ_DATA_TYPE_DOUBLE
} glitz_data_type_t;

typedef struct _glitz_geometry_format {
  glitz_geometry_mode_t mode;
  glitz_geometry_edge_hint_t edge_hint;
  glitz_geometry_primitive_t primitive;
  glitz_data_type_t type;
  int first;
  unsigned int count;
} glitz_geometry_format_t;

void
glitz_set_geometry (glitz_surface_t *dst,
                    int x_dst,
                    int y_dst,
                    glitz_geometry_format_t *format,
                    glitz_buffer_t *buffer);
  

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
