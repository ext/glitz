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

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "glitzint.h"

#include <string.h>

typedef struct _glitz_gl_pixel_format glitz_gl_pixel_format_t;
  
static struct _glitz_gl_pixel_format {
  glitz_pixel_format_t pixel;
  glitz_gl_enum_t format;
  glitz_gl_enum_t type;
}  _gl_format_map[] = {
  {
    {
      {
        8,
        0x000000ff,
        0x00000000,
        0x00000000,
        0x00000000
      },
      0, 0, 0,
      GLITZ_PIXEL_SCANLINE_ORDER_BOTTOM_UP
    },
    GLITZ_GL_ALPHA,
    GLITZ_GL_UNSIGNED_BYTE
  }, {
    {
      {
        24,
        0x00000000,
        0x00ff0000,
        0x0000ff00,
        0x000000ff
      },
      0, 0, 0,
      GLITZ_PIXEL_SCANLINE_ORDER_BOTTOM_UP
    },
    
#if IMAGE_BYTE_ORDER == MSBFirst
    GLITZ_GL_RGB,
#else
    GLITZ_GL_BGR,
#endif
    
    GLITZ_GL_UNSIGNED_BYTE
  }, {
    {
      {
        32,
        0xff000000,
        0x00ff0000,
        0x0000ff00,
        0x000000ff
      },
      0, 0, 0,
      GLITZ_PIXEL_SCANLINE_ORDER_BOTTOM_UP
    },
    GLITZ_GL_BGRA,
    
#if IMAGE_BYTE_ORDER == MSBFirst  
    GLITZ_GL_UNSIGNED_INT_8_8_8_8_REV
#else
    GLITZ_GL_UNSIGNED_BYTE
#endif
    
  }
};

#define GLITZ_GL_FORMAT_A    0
#define GLITZ_GL_FORMAT_RGB  1
#define GLITZ_GL_FORMAT_ARGB 2

typedef struct _glitz_pixel_color {
  uint32_t r, g, b, a;
} glitz_pixel_color_t;

typedef struct _glitz_pixel_transform_op {
  char *line;
  int offset;
  glitz_pixel_format_t *format;
  glitz_pixel_color_t *color;
} glitz_pixel_transform_op_t;

#define FETCH(p, mask) ((mask)? \
  ((uint32_t) ((((uint64_t) (((uint32_t) (p)) & (mask))) * 0xffffffff) / \
  ((uint64_t) (mask)))): 0x0)

#define FETCH_A(p, mask) ((mask)? \
  ((uint32_t) ((((uint64_t) (((uint32_t) (p)) & (mask))) * 0xffffffff) / \
  ((uint64_t) (mask)))): 0xffffffff)

typedef void (*glitz_pixel_fetch_function_t) (glitz_pixel_transform_op_t *op);

static void
_fetch_1 (glitz_pixel_transform_op_t *op)
{
  uint8_t p = (uint8_t) op->line[op->offset / 8];

#if BITMAP_BIT_ORDER == MSBFirst
  p = (p >> (7 - (op->offset % 8))) & 0x1;
#else
  p = (p >> (op->offset % 8)) & 0x1;
#endif
  
  op->color->a = FETCH_A (p, op->format->masks.alpha_mask);
  op->color->r = FETCH (p, op->format->masks.red_mask);
  op->color->g = FETCH (p, op->format->masks.green_mask);
  op->color->b = FETCH (p, op->format->masks.blue_mask);
}

static void
_fetch_8 (glitz_pixel_transform_op_t *op)
{
  uint8_t p = op->line[op->offset];
  
  op->color->a = FETCH_A (p, op->format->masks.alpha_mask);
  op->color->r = FETCH (p, op->format->masks.red_mask);
  op->color->g = FETCH (p, op->format->masks.green_mask);
  op->color->b = FETCH (p, op->format->masks.blue_mask);
}

static void
_fetch_16 (glitz_pixel_transform_op_t *op)
{
  uint16_t p = ((uint16_t *) op->line)[op->offset];
  
  op->color->a = FETCH_A (p, op->format->masks.alpha_mask);
  op->color->r = FETCH (p, op->format->masks.red_mask);
  op->color->g = FETCH (p, op->format->masks.green_mask);
  op->color->b = FETCH (p, op->format->masks.blue_mask);
}

static void
_fetch_24 (glitz_pixel_transform_op_t *op)
{
  uint8_t *l = (uint8_t *) &op->line[op->offset * 3];
  uint32_t p;
  
#if IMAGE_BYTE_ORDER == MSBFirst
  p = 0xff000000 | (l[2] << 16) | (l[1] << 8) | (l[0]);
#else
  p = 0xff000000 | (l[0] << 16) | (l[1] << 8) | (l[2]);
#endif

  op->color->a = FETCH_A (p, op->format->masks.alpha_mask);
  op->color->r = FETCH (p, op->format->masks.red_mask);
  op->color->g = FETCH (p, op->format->masks.green_mask);
  op->color->b = FETCH (p, op->format->masks.blue_mask);
}

static void
_fetch_32 (glitz_pixel_transform_op_t *op)
{
  uint32_t p = ((uint32_t *) op->line)[op->offset];
  
  op->color->a = FETCH_A (p, op->format->masks.alpha_mask);
  op->color->r = FETCH (p, op->format->masks.red_mask);
  op->color->g = FETCH (p, op->format->masks.green_mask);
  op->color->b = FETCH (p, op->format->masks.blue_mask);
}

typedef void (*glitz_pixel_store_function_t) (glitz_pixel_transform_op_t *op);

#define STORE(v, mask) \
  (((uint32_t) (((v) * (uint64_t) (mask)) / 0xffffffff)) & (mask))

static void
_store_1 (glitz_pixel_transform_op_t *op)
{
  uint8_t *p = (uint8_t *) &op->line[op->offset / 8];
  uint32_t offset;

#if BITMAP_BIT_ORDER == MSBFirst
    offset = 7 - (op->offset % 8);
#else
    offset = op->offset % 8;
#endif

  *p |=
    ((STORE (op->color->a, op->format->masks.alpha_mask) |
      STORE (op->color->r, op->format->masks.red_mask) |
      STORE (op->color->g, op->format->masks.green_mask) |
      STORE (op->color->b, op->format->masks.blue_mask)) & 0x1) << offset;
}

static void
_store_8 (glitz_pixel_transform_op_t *op)
{
  uint8_t *p = (uint8_t *) &op->line[op->offset];
  
  *p = (uint8_t)
    STORE (op->color->a, op->format->masks.alpha_mask) |
    STORE (op->color->r, op->format->masks.red_mask) |
    STORE (op->color->g, op->format->masks.green_mask) |
    STORE (op->color->b, op->format->masks.blue_mask);
}

static void
_store_16 (glitz_pixel_transform_op_t *op)
{
  uint16_t *p = &((uint16_t *) op->line)[op->offset];

  *p = (uint16_t)
    STORE (op->color->a, op->format->masks.alpha_mask) |
    STORE (op->color->r, op->format->masks.red_mask) |
    STORE (op->color->g, op->format->masks.green_mask) |
    STORE (op->color->b, op->format->masks.blue_mask);
}

static void
_store_24 (glitz_pixel_transform_op_t *op)
{
  uint8_t *l = (uint8_t *) &op->line[op->offset * 3];
  
  uint32_t p =
    STORE (op->color->a, op->format->masks.alpha_mask) |
    STORE (op->color->r, op->format->masks.red_mask) |
    STORE (op->color->g, op->format->masks.green_mask) |
    STORE (op->color->b, op->format->masks.blue_mask);
  
#if IMAGE_BYTE_ORDER == MSBFirst
  l[2] = p >> 16;
  l[1] = p >> 8;
  l[0] = p;
#else
  l[0] = p >> 16;
  l[1] = p >> 8;
  l[2] = p;
#endif
  
}

static void
_store_32 (glitz_pixel_transform_op_t *op)
{
  uint32_t *p = &((uint32_t *) op->line)[op->offset];
  
  *p =
    STORE (op->color->a, op->format->masks.alpha_mask) |
    STORE (op->color->r, op->format->masks.red_mask) |
    STORE (op->color->g, op->format->masks.green_mask) |
    STORE (op->color->b, op->format->masks.blue_mask);
}

#define GLITZ_TRANSFORM_PIXELS_MASK         (1L << 0)
#define GLITZ_TRANSFORM_SCANLINE_ORDER_MASK (1L << 1)
#define GLITZ_TRANSFORM_COPY_REGION_MASK    (1L << 2)

typedef struct _glitz_image {
  char *data;
  glitz_pixel_format_t *format;
  int width;
  int height;
} glitz_image_t;

static void
_glitz_pixel_transform (unsigned long transform,
                        glitz_image_t *src,
                        glitz_image_t *dst,
                        int x_src,
                        int x_dst,
                        int width,
                        int height)
{
  int src_stride, dst_stride;
  int x, y, bytes_per_pixel = 0;
  glitz_pixel_fetch_function_t fetch;
  glitz_pixel_store_function_t store;
  glitz_pixel_color_t color;
  glitz_pixel_transform_op_t src_op, dst_op;

  switch (src->format->masks.bpp) {
  case 1:
    fetch = _fetch_1;
    break;
  case 8:
    fetch = _fetch_8;
    break;
  case 16:
    fetch = _fetch_16;
    break;
  case 24:
    fetch = _fetch_24;
    break;
  case 32:
  default:
    fetch = _fetch_32;
    break;
  }

  switch (dst->format->masks.bpp) {
  case 1:
    store = _store_1;
    break;
  case 8:
    store = _store_8;
    break;
  case 16:
    store = _store_16;
    break;
  case 24:
    store = _store_24;
    break;
  case 32:
  default:
    store = _store_32;
    break;
  }

  src_stride = (src->format->bytes_per_line)? src->format->bytes_per_line:
    (((src->width * src->format->masks.bpp) / 8) + 3) & -4;
  if (src_stride == 0)
    src_stride = 1;
  src_op.format = src->format;
  src_op.color = &color;

  dst_stride = (dst->format->bytes_per_line)? dst->format->bytes_per_line:
    (((dst->width * dst->format->masks.bpp) / 8) + 3) & -4;
  if (dst_stride == 0)
    dst_stride = 1;
  dst_op.format = dst->format;
  dst_op.color = &color;

  for (y = 0; y < height; y++) {
    if (src->format->scanline_order != dst->format->scanline_order)
      src_op.line = &src->data[(src->height - y - 1) * src_stride];
    else
      src_op.line = &src->data[y * src_stride];

    dst_op.line = &dst->data[y * dst_stride];
    
    if (transform & GLITZ_TRANSFORM_PIXELS_MASK) {
      for (x = 0; x < width; x++) {
        src_op.offset = x_src + x;
        dst_op.offset = x_dst + x;
        
        fetch (&src_op);
        store (&dst_op);
      }
    } else {
      /* This only works for bpp >= 8, but it shouldn't be a problem as
         it will never be used for bitmaps */
      if (bytes_per_pixel == 0)
        bytes_per_pixel = src->format->masks.bpp / 8;
      
      memcpy (&dst_op.line[x_dst * bytes_per_pixel],
              &src_op.line[x_src * bytes_per_pixel],
              width * bytes_per_pixel);
    }
  }
}

static glitz_gl_pixel_format_t *
_glitz_find_gl_pixel_format (glitz_pixel_format_t *format)
{
  int i, n_formats;

  n_formats = sizeof (_gl_format_map) / sizeof (glitz_gl_pixel_format_t);
  for (i = 0; i < n_formats; i++) {
    if (memcmp (&_gl_format_map[i].pixel.masks, &format->masks,
                sizeof (glitz_pixel_masks_t)) == 0)
      return &_gl_format_map[i];
  }
  
  return NULL;
}

static glitz_gl_pixel_format_t *
_glitz_best_gl_pixel_format (glitz_format_t *format)
{
  if (format->red_size) {
    if (format->alpha_size)
      return &_gl_format_map[GLITZ_GL_FORMAT_ARGB];
    else
      return &_gl_format_map[GLITZ_GL_FORMAT_RGB];
  } else
    return &_gl_format_map[GLITZ_GL_FORMAT_A];
}

void
glitz_set_pixels (glitz_surface_t *dst,
                  int x_dst,
                  int y_dst,
                  int width,
                  int height,
                  glitz_pixel_format_t *format,
                  glitz_buffer_t *buffer)
{
  glitz_gl_proc_address_list_t *gl;
  glitz_bool_t to_drawable;
  glitz_texture_t *texture;
  char *pixels, *data = NULL;
  glitz_gl_pixel_format_t *gl_format = NULL;
  unsigned long transform = 0;
  int xoffset, bytes_per_line;

  if (x_dst < 0 || x_dst > (dst->width - width) ||
      y_dst < 0 || y_dst > (dst->height - height)) {
    glitz_surface_status_add (dst, GLITZ_STATUS_BAD_COORDINATE_MASK);
    return;
  }

  gl = &dst->backend->gl;

  if (format->scanline_order == GLITZ_PIXEL_SCANLINE_ORDER_TOP_DOWN)
    transform |= GLITZ_TRANSFORM_SCANLINE_ORDER_MASK;

  /* find direct format */
  gl_format = _glitz_find_gl_pixel_format (format);
  if (gl_format == NULL) {
    transform |= GLITZ_TRANSFORM_PIXELS_MASK;
    gl_format = _glitz_best_gl_pixel_format (dst->format);
  }

  if (SURFACE_DRAWABLE (dst))
    to_drawable = 1;
  else
    to_drawable = 0;

  if (to_drawable) {
    if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT))
      to_drawable = 0;
  } else
    glitz_surface_push_current (dst, GLITZ_CN_ANY_CONTEXT_CURRENT);
  
  if (transform) {
    glitz_image_t src_image, dst_image;
    int stride;
    
    stride = (((width * gl_format->pixel.masks.bpp) / 8) + 3) & -4;
    
    data = malloc (stride * height);
    if (!data) {
      glitz_surface_pop_current (dst);
      glitz_surface_status_add (dst, GLITZ_STATUS_NO_MEMORY_MASK);
      return;
    }

    dst_image.data = data;
    dst_image.format = &gl_format->pixel;
    dst_image.width = width;
    dst_image.height = height;

    src_image.data = glitz_buffer_map (buffer, GLITZ_BUFFER_ACCESS_READ_ONLY);
    src_image.data += format->skip_lines * format->bytes_per_line;
    src_image.format = format;
    src_image.width = width;
    src_image.height = height;

    _glitz_pixel_transform (transform,
                            &src_image,
                            &dst_image,
                            format->xoffset,
                            0,
                            width, height);

    glitz_buffer_unmap (buffer);
                                 
    pixels = data;
    xoffset = 0;
    bytes_per_line = stride;
  } else {
    xoffset = format->xoffset;
    bytes_per_line = format->bytes_per_line;
    pixels = glitz_buffer_bind (buffer, GLITZ_GL_PIXEL_UNPACK_BUFFER);
    pixels += format->skip_lines * bytes_per_line;
  }

  texture = glitz_surface_get_texture (dst, 1);
  if (!texture) {
    glitz_surface_pop_current (dst);
    return;
  }
  
  glitz_texture_bind (gl, texture);

  gl->pixel_store_i (GLITZ_GL_UNPACK_ROW_LENGTH, 0);
  gl->pixel_store_i (GLITZ_GL_UNPACK_SKIP_ROWS, 0);
  gl->pixel_store_i (GLITZ_GL_UNPACK_SKIP_PIXELS, xoffset);

  if (bytes_per_line) {
    if ((bytes_per_line % 4) == 0)
      gl->pixel_store_i (GLITZ_GL_UNPACK_ALIGNMENT, 4);
    else if ((bytes_per_line % 3) == 0)
      gl->pixel_store_i (GLITZ_GL_UNPACK_ALIGNMENT, 3);
    else if ((bytes_per_line % 2) == 0)
      gl->pixel_store_i (GLITZ_GL_UNPACK_ALIGNMENT, 2);
    else
      gl->pixel_store_i (GLITZ_GL_UNPACK_ALIGNMENT, 2);
  } else
    gl->pixel_store_i (GLITZ_GL_UNPACK_ALIGNMENT, 1);    

  gl->tex_sub_image_2d (texture->target, 0,
                        x_dst, dst->height - y_dst - height,
                        width, height,
                        gl_format->format, gl_format->type,
                        pixels);

  if (to_drawable) {
    glitz_texture_set_tex_gen (gl, texture, x_dst, y_dst, height, ~0); 

    gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
                   GLITZ_GL_REPLACE);
    gl->color_4us (0x0, 0x0, 0x0, 0xffff);
    
    glitz_texture_ensure_wrap (gl, texture, GLITZ_GL_CLAMP_TO_EDGE);
    glitz_texture_ensure_filter (gl, texture, GLITZ_GL_NEAREST);

    glitz_set_operator (gl, GLITZ_OPERATOR_SRC);

    gl->scissor (x_dst,
                 dst->height - (y_dst + height),
                 width, height);

    glitz_geometry_enable_default (gl, dst);

    gl->draw_arrays (GLITZ_GL_QUADS, 0, 4);

    glitz_geometry_disable (gl, dst);

    if (x_dst == 0 && y_dst == 0 &&
        width == dst->width && height == dst->height)
      dst->flags &= ~GLITZ_FLAG_DIRTY_MASK;
  }
  
  glitz_texture_unbind (gl, texture);

  dst->flags |= GLITZ_FLAG_SOLID_DIRTY_MASK;

  if (transform == 0)
    glitz_buffer_unbind (buffer);
  
  glitz_surface_pop_current (dst);

  if (data)
    free (data);
}

void
glitz_get_pixels (glitz_surface_t *src,
                  int x_src,
                  int y_src,
                  int width,
                  int height,
                  glitz_pixel_format_t *format,
                  glitz_buffer_t *buffer)
{
  glitz_gl_proc_address_list_t *gl;
  glitz_bool_t from_drawable;
  glitz_texture_t *texture = NULL;
  char *pixels, *data = NULL;
  glitz_gl_pixel_format_t *gl_format = NULL;
  unsigned long transform = 0;
  int src_x = 0, src_y = 0, src_w = width, src_h = height;
  int xoffset, bytes_per_line;
  
  if (x_src < 0 || x_src > (src->width - width) ||
      y_src < 0 || y_src > (src->height - height)) {
    glitz_surface_status_add (src, GLITZ_STATUS_BAD_COORDINATE_MASK);
    return;
  }

  gl = &src->backend->gl;

  if (glitz_surface_push_current (src, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    from_drawable = 1;
  } else {
    from_drawable = 0;
    
    texture = glitz_surface_get_texture (src, 0);
    if (!texture) {
      glitz_surface_pop_current (src);
      return;
    }

    transform |= GLITZ_TRANSFORM_COPY_REGION_MASK;
  }
  
  if (format->scanline_order == GLITZ_PIXEL_SCANLINE_ORDER_TOP_DOWN)
    transform |= GLITZ_TRANSFORM_SCANLINE_ORDER_MASK;
  
  /* find direct format */
  gl_format = _glitz_find_gl_pixel_format (format);
  if (gl_format == NULL) {
    transform |= GLITZ_TRANSFORM_PIXELS_MASK;
    gl_format = _glitz_best_gl_pixel_format (src->format);
  }
  
  if (transform) {
    int stride;
    
    if (transform & GLITZ_TRANSFORM_COPY_REGION_MASK) {
      src_w = texture->width;
      src_h = texture->height;
      src_x = x_src;
      src_y = y_src;
    }

    stride = (((src_w * gl_format->pixel.masks.bpp) / 8) + 3) & -4;

    data = malloc (stride * src_h);
    if (!data) {
      glitz_surface_status_add (src, GLITZ_STATUS_NO_MEMORY_MASK);
      return;
    }
    pixels = data;
    xoffset = 0;
    bytes_per_line = stride;
  } else {
    xoffset = format->xoffset;
    bytes_per_line = format->bytes_per_line;
    pixels = glitz_buffer_bind (buffer, GLITZ_GL_PIXEL_PACK_BUFFER);
    pixels += format->skip_lines * bytes_per_line;
  }
  
  gl->pixel_store_i (GLITZ_GL_PACK_ROW_LENGTH, 0);
  gl->pixel_store_i (GLITZ_GL_PACK_SKIP_ROWS, 0);
  gl->pixel_store_i (GLITZ_GL_PACK_SKIP_PIXELS, xoffset);

  if (bytes_per_line) {
    if ((bytes_per_line % 4) == 0)
      gl->pixel_store_i (GLITZ_GL_UNPACK_ALIGNMENT, 4);
    else if ((bytes_per_line % 3) == 0)
      gl->pixel_store_i (GLITZ_GL_UNPACK_ALIGNMENT, 3);
    else if ((bytes_per_line % 2) == 0)
      gl->pixel_store_i (GLITZ_GL_UNPACK_ALIGNMENT, 2);
    else
      gl->pixel_store_i (GLITZ_GL_UNPACK_ALIGNMENT, 2);
  } else
    gl->pixel_store_i (GLITZ_GL_UNPACK_ALIGNMENT, 1);

  if (from_drawable) {
    gl->read_pixels (x_src, src->height - y_src - height,
                     width, height,
                     gl_format->format, gl_format->type,
                     pixels);
  } else {
    glitz_texture_bind (gl, texture);
    gl->get_tex_image (texture->target, 0,
                       gl_format->format, gl_format->type,
                       pixels);
    glitz_texture_unbind (gl, texture);
  }

  if (transform) {
    glitz_image_t src_image, dst_image;

    src_image.data = data + src_y * gl_format->pixel.bytes_per_line;
    src_image.format = &gl_format->pixel;
    src_image.width = src_w;
    src_image.height = src_h;

    dst_image.data = glitz_buffer_map (buffer, GLITZ_BUFFER_ACCESS_WRITE_ONLY);
    dst_image.data += format->skip_lines * format->bytes_per_line;
    dst_image.format = format;
    dst_image.width = width;
    dst_image.height = height;

    _glitz_pixel_transform (transform,
                            &src_image,
                            &dst_image,
                            src_x,
                            format->xoffset,
                            width, height);

    glitz_buffer_unmap (buffer);
  } else
    glitz_buffer_unbind (buffer);

  glitz_surface_pop_current (src);

  if (data)
    free (data);
}
