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
  double a;
  double r;
  double g;
  double b;
} glitz_pixel_color_t;

typedef struct _glitz_pixel_transform_op {
  char *line;
  int offset;
  glitz_pixel_format_t *format;
  glitz_pixel_color_t *color;
} glitz_pixel_transform_op_t;

#define FETCH(p, mask) ((mask)? \
  (((double) (((uint32_t) (p)) & (mask))) / ((double) mask)): 0.0)
#define FETCH_A(p, mask) ((mask)? \
  (((double) (((uint32_t) (p)) & (mask))) / ((double) mask)): 1.0)

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
  (((uint32_t) (v * (double) mask)) & mask)

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

struct _glitz_pixel_buffer {
  glitz_gl_sizei_t size;
  glitz_gl_uint_t name;
  glitz_gl_enum_t target;
  char *data;
  int owns_data;
  glitz_surface_t *surface;
  glitz_pixel_format_t format;
};

glitz_pixel_buffer_t *
glitz_pixel_buffer_create (glitz_surface_t *surface,
                           char *data,
                           unsigned int size,
                           glitz_buffer_hint_t hint)
{
  glitz_pixel_buffer_t *buffer;
  glitz_gl_enum_t usage;

  buffer = malloc (sizeof (glitz_pixel_buffer_t));
  if (buffer == NULL)
    return NULL;

  buffer->size = size;
  buffer->name = 0;
  
  switch (hint) {
  case GLITZ_PIXEL_BUFFER_HINT_STREAM_DRAW:
    usage = GLITZ_GL_STREAM_DRAW;
    buffer->target = GLITZ_GL_PIXEL_UNPACK_BUFFER;
    break;
  case GLITZ_PIXEL_BUFFER_HINT_STREAM_READ:
    usage = GLITZ_GL_STREAM_READ;
    buffer->target = GLITZ_GL_PIXEL_PACK_BUFFER;
    break;
  case GLITZ_PIXEL_BUFFER_HINT_STREAM_COPY:
    usage = GLITZ_GL_STREAM_COPY;
    buffer->target = GLITZ_GL_PIXEL_UNPACK_BUFFER;
    break;
  case GLITZ_PIXEL_BUFFER_HINT_STATIC_DRAW:
    usage = GLITZ_GL_STATIC_DRAW;
    buffer->target = GLITZ_GL_PIXEL_UNPACK_BUFFER;
    break;
  case GLITZ_PIXEL_BUFFER_HINT_STATIC_READ:
    usage = GLITZ_GL_STATIC_READ;
    buffer->target = GLITZ_GL_PIXEL_PACK_BUFFER;
    break;
  case GLITZ_PIXEL_BUFFER_HINT_STATIC_COPY:
    usage = GLITZ_GL_STATIC_COPY;
    buffer->target = GLITZ_GL_PIXEL_UNPACK_BUFFER;
    break;
  case GLITZ_PIXEL_BUFFER_HINT_DYNAMIC_DRAW:
    usage = GLITZ_GL_DYNAMIC_DRAW;
    buffer->target = GLITZ_GL_PIXEL_UNPACK_BUFFER;
    break;
  case GLITZ_PIXEL_BUFFER_HINT_DYNAMIC_READ:
    usage = GLITZ_GL_DYNAMIC_READ;
    buffer->target = GLITZ_GL_PIXEL_PACK_BUFFER;
    break;
  default:
    usage = GLITZ_GL_DYNAMIC_COPY;
    buffer->target = GLITZ_GL_PIXEL_UNPACK_BUFFER;
    break;
  }

  if (surface->feature_mask & GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK) {
    buffer->surface = surface;
    glitz_surface_reference (surface);
    
    glitz_surface_push_current (surface, GLITZ_CN_ANY_CONTEXT_CURRENT);
    
    surface->gl->gen_buffers (1, &buffer->name);
    if (buffer->name) {
      surface->gl->bind_buffer (buffer->target, buffer->name);
      surface->gl->buffer_data (buffer->target, size, data, usage);
    }
    
    glitz_surface_pop_current (surface); 
  } else
    buffer->surface = NULL;
  
  if (buffer->name == 0) {
    buffer->data = malloc (size);
    if (buffer->data == NULL) {
      free (buffer);
      return NULL;
    }

    if (data)
      memcpy (buffer->data, data, size);
    
    buffer->owns_data = 1;
  }
  
  buffer->format = _glitz_best_gl_pixel_format (surface->format)->pixel;
  
  return buffer;
}

glitz_pixel_buffer_t *
glitz_pixel_buffer_create_for_data (char *data,
                                    glitz_pixel_format_t *format)
{
  glitz_pixel_buffer_t *buffer;

  buffer = malloc (sizeof (glitz_pixel_buffer_t));
  if (buffer == NULL)
    return NULL;

  buffer->name = 0;
  buffer->size = 0;
  buffer->data = data;
  buffer->owns_data = 0;
  buffer->surface = NULL;
  buffer->target = 0;
  buffer->format = *format;

  return buffer;
}

void
glitz_pixel_buffer_destroy (glitz_pixel_buffer_t *buffer)
{
  glitz_surface_t *surface = buffer->surface;
  
  if (surface &&
      surface->feature_mask & GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK) {
    glitz_surface_push_current (surface, GLITZ_CN_ANY_CONTEXT_CURRENT);
    
    surface->gl->delete_buffers (1, &buffer->name);
    
    glitz_surface_pop_current (surface);
    
    glitz_surface_destroy (surface);
  } else if (buffer->owns_data)
    free (buffer->data);
  
  free (buffer);
}

void
glitz_pixel_buffer_set_format (glitz_pixel_buffer_t *buffer,
                               glitz_pixel_format_t *format)
{
  buffer->format = *format;
}

void
glitz_pixel_buffer_get_format (glitz_pixel_buffer_t *buffer,
                               glitz_pixel_format_t *format)
{
  *format = buffer->format;
}

char *
glitz_pixel_buffer_get_data (glitz_pixel_buffer_t *buffer,
                             glitz_pixel_buffer_access_t access)
{
  char *pointer = NULL;
  glitz_surface_t *surface = buffer->surface;
  
  if (surface &&
      surface->feature_mask & GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK) {
    glitz_gl_enum_t buffer_access;
    
    glitz_surface_push_current (surface, GLITZ_CN_ANY_CONTEXT_CURRENT);

    switch (access) {
    case GLITZ_PIXEL_BUFFER_ACCESS_READ_ONLY:
      buffer_access = GLITZ_GL_READ_ONLY;
      break;
    case GLITZ_PIXEL_BUFFER_ACCESS_WRITE_ONLY:
      buffer_access = GLITZ_GL_WRITE_ONLY;
      break;
    default:
      buffer_access = GLITZ_GL_READ_WRITE;
      break;
    }

    surface->gl->bind_buffer (buffer->target, buffer->name);
    pointer = (char *) surface->gl->map_buffer (buffer->target,
                                                buffer_access);
    
    glitz_surface_pop_current (surface);
  }
  
  if (pointer == NULL)
    pointer = buffer->data;
  
  return pointer;
}

void
glitz_pixel_buffer_put_back_data (glitz_pixel_buffer_t *buffer)
{
  glitz_surface_t *surface = buffer->surface;
  
  if (surface &&
      surface->feature_mask & GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK) {
    glitz_surface_push_current (surface, GLITZ_CN_ANY_CONTEXT_CURRENT);
    
    if (surface->gl->unmap_buffer (buffer->target)) {
      /* don't know what to do here, maybe glitz_pixel_buffer_put_back_data
         should return a status value */
    }
    surface->gl->bind_buffer (buffer->target, 0);
    
    glitz_surface_pop_current (surface);
  }
}

char *
glitz_pixel_buffer_bind (glitz_pixel_buffer_t *buffer,
                          glitz_gl_enum_t target)
{
  glitz_surface_t *surface = buffer->surface;
  
  if (surface &&
      surface->feature_mask & GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK) {
    surface->gl->bind_buffer (target, buffer->name);
    buffer->target = target;
    
    return NULL;
  }
  
  return buffer->data;
}

void
glitz_pixel_buffer_unbind (glitz_pixel_buffer_t *buffer)
{
  glitz_surface_t *surface = buffer->surface;
  
  if (surface &&
      surface->feature_mask & GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK)
    surface->gl->bind_buffer (buffer->target, 0);
}

void
glitz_put_pixels (glitz_surface_t *dst,
                  int x_dst,
                  int y_dst,
                  int width,
                  int height,
                  glitz_pixel_buffer_t *buffer)
{
  glitz_gl_proc_address_list_t *gl;
  glitz_bool_t to_drawable;
  glitz_texture_t *texture;
  char *pixels, *data = NULL;
  glitz_gl_pixel_format_t *format = NULL;
  unsigned long transform = 0;
  int xoffset, bytes_per_line;

  if (x_dst < 0 || x_dst > (dst->width - width) ||
      y_dst < 0 || y_dst > (dst->height - height)) {
    glitz_surface_status_add (dst, GLITZ_STATUS_BAD_COORDINATE_MASK);
    return;
  }

  gl = dst->gl;

  if (buffer->format.scanline_order == GLITZ_PIXEL_SCANLINE_ORDER_TOP_DOWN)
    transform |= GLITZ_TRANSFORM_SCANLINE_ORDER_MASK;

  /* find direct format */
  format = _glitz_find_gl_pixel_format (&buffer->format);
  if (format == NULL) {
    transform |= GLITZ_TRANSFORM_PIXELS_MASK;
    format = _glitz_best_gl_pixel_format (dst->format);
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
    
    stride = (((width * format->pixel.masks.bpp) / 8) + 3) & -4;
    
    data = malloc (stride * height);
    if (!data) {
      glitz_surface_pop_current (dst);
      glitz_surface_status_add (dst, GLITZ_STATUS_NO_MEMORY_MASK);
      return;
    }

    dst_image.data = data;
    dst_image.format = &format->pixel;
    dst_image.width = width;
    dst_image.height = height;

    src_image.data =
      glitz_pixel_buffer_get_data (buffer,
                                   GLITZ_PIXEL_BUFFER_ACCESS_READ_ONLY);
    src_image.data += buffer->format.skip_lines *
      buffer->format.bytes_per_line;
    src_image.format = &buffer->format;
    src_image.width = width;
    src_image.height = height;

    _glitz_pixel_transform (transform,
                            &src_image,
                            &dst_image,
                            buffer->format.xoffset,
                            0,
                            width, height);

    glitz_pixel_buffer_put_back_data (buffer);
                                 
    pixels = data;
    xoffset = 0;
    bytes_per_line = stride;
  } else {
    xoffset = buffer->format.xoffset;
    bytes_per_line = buffer->format.bytes_per_line;
    pixels = glitz_pixel_buffer_bind (buffer, GLITZ_GL_PIXEL_UNPACK_BUFFER);
    pixels += buffer->format.skip_lines * bytes_per_line;
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
                        format->format, format->type,
                        pixels);

  if (to_drawable) {
    glitz_point_t tl, br;

    gl->matrix_mode (GLITZ_GL_TEXTURE);
    gl->load_identity ();
    
    gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
                   GLITZ_GL_REPLACE);
    gl->color_4us (0x0, 0x0, 0x0, 0xffff);
    
    glitz_set_operator (gl, GLITZ_OPERATOR_SRC);
    
    glitz_texture_ensure_wrap (gl, texture, GLITZ_GL_CLAMP_TO_EDGE);
    glitz_texture_ensure_filter (gl, texture, GLITZ_GL_NEAREST);

    glitz_texture_tex_coord (texture, x_dst, y_dst, &tl.x, &tl.y);
    glitz_texture_tex_coord (texture,
                             x_dst + width, y_dst + height, &br.x, &br.y);

    tl.y = texture->texcoord_height - tl.y;
    br.y = texture->texcoord_height - br.y;
    
    gl->begin (GLITZ_GL_QUADS);
    gl->tex_coord_2d (tl.x, tl.y);
    gl->vertex_2d (x_dst, y_dst);
    gl->tex_coord_2d (br.x, tl.y);
    gl->vertex_2d (x_dst + width, y_dst);
    gl->tex_coord_2d (br.x, br.y);
    gl->vertex_2d (x_dst + width, y_dst + height);
    gl->tex_coord_2d (tl.x, br.y);
    gl->vertex_2d (x_dst, y_dst + height);
    gl->end ();

    if (x_dst == 0 && y_dst == 0 &&
        width == dst->width && height == dst->height)
      dst->hint_mask &= ~GLITZ_INT_HINT_DIRTY_MASK;
  }
  
  glitz_texture_unbind (gl, texture);

  dst->hint_mask |= GLITZ_INT_HINT_SOLID_DIRTY_MASK;

  if (transform == 0)
    glitz_pixel_buffer_unbind (buffer);
  
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
                  glitz_pixel_buffer_t *buffer)
{
  glitz_gl_proc_address_list_t *gl;
  glitz_bool_t from_drawable;
  glitz_texture_t *texture = NULL;
  char *pixels, *data = NULL;
  glitz_gl_pixel_format_t *format = NULL;
  unsigned long transform = 0;
  int src_x = 0, src_y = 0, src_w = width, src_h = height;
  int xoffset, bytes_per_line;
  
  if (x_src < 0 || x_src > (src->width - width) ||
      y_src < 0 || y_src > (src->height - height)) {
    glitz_surface_status_add (src, GLITZ_STATUS_BAD_COORDINATE_MASK);
    return;
  }

  gl = src->gl;

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
  
  if (buffer->format.scanline_order == GLITZ_PIXEL_SCANLINE_ORDER_TOP_DOWN)
    transform |= GLITZ_TRANSFORM_SCANLINE_ORDER_MASK;
  
  /* find direct format */
  format = _glitz_find_gl_pixel_format (&buffer->format);
  if (format == NULL) {
    transform |= GLITZ_TRANSFORM_PIXELS_MASK;
    format = _glitz_best_gl_pixel_format (src->format);
  }
  
  if (transform) {
    int stride;
    
    if (transform & GLITZ_TRANSFORM_COPY_REGION_MASK) {
      src_w = texture->width;
      src_h = texture->height;
      src_x = x_src;
      src_y = y_src;
    }

    stride = (((src_w * format->pixel.masks.bpp) / 8) + 3) & -4;

    data = malloc (stride * src_h);
    if (!data) {
      glitz_surface_status_add (src, GLITZ_STATUS_NO_MEMORY_MASK);
      return;
    }
    pixels = data;
    xoffset = 0;
    bytes_per_line = stride;
  } else {
    xoffset = buffer->format.xoffset;
    bytes_per_line = buffer->format.bytes_per_line;
    pixels = glitz_pixel_buffer_bind (buffer, GLITZ_GL_PIXEL_PACK_BUFFER);
    pixels += buffer->format.skip_lines * bytes_per_line;
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
                     format->format, format->type,
                     pixels);
  } else {
    glitz_texture_bind (gl, texture);
    gl->get_tex_image (texture->target, 0,
                       format->format, format->type,
                       pixels);
    glitz_texture_unbind (gl, texture);
  }

  if (transform) {
    glitz_image_t src_image, dst_image;

    src_image.data = data + src_y * format->pixel.bytes_per_line;
    src_image.format = &format->pixel;
    src_image.width = src_w;
    src_image.height = src_h;

    dst_image.data =
      glitz_pixel_buffer_get_data (buffer,
                                   GLITZ_PIXEL_BUFFER_ACCESS_WRITE_ONLY);
    dst_image.data += buffer->format.skip_lines *
      buffer->format.bytes_per_line;
    dst_image.format = &buffer->format;
    dst_image.width = width;
    dst_image.height = height;

    _glitz_pixel_transform (transform,
                            &src_image,
                            &dst_image,
                            src_x,
                            buffer->format.xoffset,
                            width, height);

    glitz_pixel_buffer_put_back_data (buffer);
  } else
    glitz_pixel_buffer_unbind (buffer);

  glitz_surface_pop_current (src);

  if (data)
    free (data);
}
