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

/* This is supposed to be 8x, 4x and 2x rotated grid multi-sample
   patterns. I'm not sure they're actually correct. */
static glitz_sample_offset_t _8x_multi_sample_offsets[] = {
  { -0.375f, -0.375f },
  { -0.125f, -0.125f },
  {  0.375f, -0.375f },
  {  0.125f, -0.125f },
  {  0.375f,  0.375f },
  {  0.125f,  0.125f },
  { -0.375f,  0.375f },
  { -0.125f,  0.125f }
};

static unsigned short _8x_multi_sample_weights[] = {
  0x2000, 0x4000, 0x6000, 0x8000, 0xa000, 0xbfff, 0xdfff, 0xffff
};

static glitz_sample_info_t _8x_multi_sample = {
  _8x_multi_sample_offsets,
  _8x_multi_sample_weights,
  8
};

static glitz_sample_offset_t _4x_multi_sample_offsets[] = {
  { -0.125f, -0.375f },
  {  0.375f, -0.125f },
  {  0.125f,  0.375f },
  { -0.375f,  0.125f }
};

static unsigned short _4x_multi_sample_weights[] = {
  0x4000, 0x8000, 0xbfff, 0xffff
};

static glitz_sample_info_t _4x_multi_sample = {
  _4x_multi_sample_offsets,
  _4x_multi_sample_weights,
  4
};

static glitz_sample_offset_t _2x_multi_sample_offsets[] = {
  { -0.24f, -0.24f },
  {  0.24f,  0.24f }
};

static unsigned short _2x_multi_sample_weights[] = {
  0x8000, 0xffff
};

static glitz_sample_info_t _2x_multi_sample = {
  _2x_multi_sample_offsets,
  _2x_multi_sample_weights,
  2
};

static glitz_sample_offset_t _1x_sample_offsets[] = {
  {  0.0f,  0.0f }
};

static unsigned short _1x_sample_weights[] = {
  0xffff
};

static glitz_sample_info_t _1x_sample = {
  _1x_sample_offsets,
  _1x_sample_weights,
  1
};

void
glitz_set_geometry (glitz_surface_t *dst,
                    int x_dst,
                    int y_dst,
                    glitz_geometry_format_t *format,
                    glitz_buffer_t *buffer)
{
  if (dst->geometry.buffer) {
    glitz_buffer_destroy (dst->geometry.buffer);
    dst->geometry.buffer = NULL;
  }

  if (buffer) {
    dst->geometry.buffer = buffer;
    glitz_buffer_reference (buffer);
    
    dst->geometry.x_offset = x_dst;
    dst->geometry.y_offset = y_dst;

    switch (format->primitive) {
    case GLITZ_GEOMETRY_PRIMITIVE_TRIANGLES:
      dst->geometry.primitive = GLITZ_GL_TRIANGLES;
      break;
    case GLITZ_GEOMETRY_PRIMITIVE_TRIANGLE_STRIP:
      dst->geometry.primitive = GLITZ_GL_TRIANGLE_STRIP;
      break;
    case GLITZ_GEOMETRY_PRIMITIVE_TRIANGLE_FAN:
      dst->geometry.primitive = GLITZ_GL_TRIANGLE_FAN;
      break;
    case GLITZ_GEOMETRY_PRIMITIVE_QUADS:
      dst->geometry.primitive = GLITZ_GL_QUADS;
      break;
    case GLITZ_GEOMETRY_PRIMITIVE_QUAD_STRIP:
      dst->geometry.primitive = GLITZ_GL_QUAD_STRIP;
      break;
    default:
      dst->geometry.primitive = GLITZ_GL_POLYGON;
      break;
    }

    switch (format->type) {
    case GLITZ_DATA_TYPE_SHORT:
      dst->geometry.type = GLITZ_GL_SHORT;
      break;
    case GLITZ_DATA_TYPE_INT:
      dst->geometry.type = GLITZ_GL_INT;
      break;
    case GLITZ_DATA_TYPE_DOUBLE:
      dst->geometry.type = GLITZ_GL_DOUBLE;
      break;
    default:
      dst->geometry.type = GLITZ_GL_FLOAT;
      break;
    }

    dst->geometry.first = format->first;
    dst->geometry.count = format->count;

    if (dst->format->multisample.samples > 1) {
      if (format->edge_hint != GLITZ_GEOMETRY_EDGE_HINT_SHARP) {
        dst->flags |= GLITZ_SURFACE_FLAG_MULTISAMPLE_MASK;

        if (format->edge_hint != GLITZ_GEOMETRY_EDGE_HINT_FAST_SMOOTH)
          dst->flags |= GLITZ_SURFACE_FLAG_NICEST_MULTISAMPLE_MASK;
        else
          dst->flags &= ~GLITZ_SURFACE_FLAG_NICEST_MULTISAMPLE_MASK;
        
        dst->update_mask |= GLITZ_UPDATE_MULTISAMPLE_MASK;
      } else {
        dst->flags &= ~GLITZ_SURFACE_FLAG_MULTISAMPLE_MASK;
        dst->update_mask |= GLITZ_UPDATE_MULTISAMPLE_MASK;
      }
    } else {
      if (format->mode == GLITZ_GEOMETRY_MODE_INDIRECT) {
        switch (format->edge_hint) {
        case GLITZ_GEOMETRY_EDGE_HINT_BEST_SMOOTH:
          if (dst->format->stencil_size >= 4) {
            dst->indirect = &_8x_multi_sample;
            break;
          }
          /* fall-through */
        case GLITZ_GEOMETRY_EDGE_HINT_GOOD_SMOOTH:
          if (dst->format->stencil_size >= 3) {
            dst->indirect = &_4x_multi_sample;
            break;
          }
          /* fall-through */
        case GLITZ_GEOMETRY_EDGE_HINT_FAST_SMOOTH:
          if (dst->format->stencil_size >= 2) {
            dst->indirect = &_2x_multi_sample;
            break;
          }
          /* fall-through */
        case GLITZ_GEOMETRY_EDGE_HINT_SHARP:
        default:
          dst->indirect = &_1x_sample;
          break;
        }
      } else
        dst->indirect = NULL;
    }
  } else {
    if (dst->format->multisample.samples > 1) {
      dst->flags &= ~GLITZ_SURFACE_FLAG_MULTISAMPLE_MASK;
      dst->update_mask |= GLITZ_UPDATE_MULTISAMPLE_MASK;
    } else
      dst->indirect = NULL;
  }
}
slim_hidden_def(glitz_set_geometry);

void
glitz_geometry_enable_default (glitz_gl_proc_address_list_t *gl,
                               glitz_surface_t *dst,
                               glitz_bounding_box_t *box)
{
  dst->geometry.data[0] = box->x1;
  dst->geometry.data[1] = box->y1;
  dst->geometry.data[2] = box->x2;
  dst->geometry.data[3] = box->y1;
  dst->geometry.data[4] = box->x2;
  dst->geometry.data[5] = box->y2;
  dst->geometry.data[6] = box->x1;
  dst->geometry.data[7] = box->y2;
  
  gl->vertex_pointer (2, GLITZ_GL_FLOAT, 0, dst->geometry.data);
}

void
glitz_geometry_enable (glitz_gl_proc_address_list_t *gl,
                       glitz_surface_t *dst,
                       glitz_gl_enum_t *primitive,
                       glitz_gl_int_t *first,
                       glitz_gl_sizei_t *count,
                       glitz_bounding_box_t *box)
{
  if (dst->geometry.buffer) {
    void *ptr;

    ptr = glitz_buffer_bind (dst->geometry.buffer, GLITZ_GL_ARRAY_BUFFER);

    gl->vertex_pointer (2, dst->geometry.type, 0, ptr);
    
    if (dst->geometry.x_offset || dst->geometry.y_offset)
      gl->translate_f (dst->geometry.x_offset, dst->geometry.y_offset, 0.0f);
    
    *primitive = dst->geometry.primitive;
    *first = dst->geometry.first;
    *count = dst->geometry.count;
  } else {
    glitz_geometry_enable_default (gl, dst, box);
    *primitive = GLITZ_GL_QUADS;
    *first = 0;
    *count = 4;
  }
}

void
glitz_geometry_disable (glitz_gl_proc_address_list_t *gl,
                        glitz_surface_t *dst)
{
  if (dst->geometry.buffer &&
      (dst->backend->feature_mask & GLITZ_FEATURE_VERTEX_BUFFER_OBJECT_MASK))
    gl->bind_buffer (GLITZ_GL_ARRAY_BUFFER, 0);
}
