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

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "glitzint.h"

#define STORE_16(dst, size, src) \
  dst = ((size) ? \
         ((((((1L << (size)) - 1L) * (src)) / 0xffff) * 0xffff) / \
          ((1L << (size)) - 1L)) : \
         dst)

static void
glitz_rectangle_bounds (int x_offset,
                        int y_offset,
                        int n_rects,
                        const glitz_rectangle_t *rects,
                        glitz_bounding_box_t *box)
{
  box->x1 = MAXSHORT;
  box->x2 = MINSHORT;
  box->y1 = MAXSHORT;
  box->y2 = MINSHORT;
  
  for (; n_rects; n_rects--, rects++) {
    if (rects->x < box->x1)
      box->x1 = rects->x;
    else if ((rects->x + rects->width) > box->x2)
      box->x2 = rects->x + rects->width;
    if (rects->y < box->y1)
      box->y1 = rects->y;
    else if ((rects->y + rects->height) > box->y2)
      box->y2 = rects->y + rects->height;
  }

  box->x1 += x_offset;
  box->x2 += x_offset;
  box->y1 += y_offset;
  box->y2 += y_offset;
}

void
glitz_int_fill_rectangles (glitz_operator_t op,
                           glitz_surface_t *dst,
                           int x_offset,
                           int y_offset,
                           const glitz_color_t *color,
                           const glitz_rectangle_t *rects,
                           int n_rects)
{
  glitz_gl_proc_address_list_t *gl = dst->gl;
  glitz_gl_bitfield_t clear_mask;
  
  if (op == GLITZ_OPERATOR_SRC && (!*dst->stencil_mask)) {
    clear_mask = GLITZ_GL_COLOR_BUFFER_BIT;
    gl->clear_color (color->red / (glitz_gl_clampf_t) 0xffff,
                     color->green / (glitz_gl_clampf_t) 0xffff,
                     color->blue / (glitz_gl_clampf_t) 0xffff,
                     color->alpha / (glitz_gl_clampf_t) 0xffff);
  } else if (op == (glitz_operator_t) GLITZ_INT_OPERATOR_STENCIL_RECT_SET) {
    clear_mask = GLITZ_GL_STENCIL_BUFFER_BIT;
    gl->clear_stencil (*dst->stencil_mask);
  } else {
    if (op == (glitz_operator_t) GLITZ_INT_OPERATOR_STENCIL_RECT_SRC)
      op = GLITZ_OPERATOR_SRC;
    
    clear_mask = 0x0;
  }
  
  if (clear_mask) {
    for (; n_rects; n_rects--, rects++) {
      gl->enable (GLITZ_GL_SCISSOR_TEST);
      gl->scissor (x_offset + rects->x,
                   dst->height - (y_offset + rects->y + rects->height),
                   rects->width,
                   rects->height);
      gl->clear (clear_mask);
    }
    dst->update_mask |= GLITZ_UPDATE_SCISSOR_MASK;
  } else {
    glitz_gl_vertex_2i_t vertex_2i = gl->vertex_2i;
    
    gl->color_4us (color->red, color->green, color->blue, color->alpha);
      
    glitz_set_operator (gl, op);
      
    gl->begin (GLITZ_GL_QUADS);
      
    for (; n_rects; n_rects--, rects++) {
      vertex_2i (x_offset + rects->x, y_offset + rects->y);
      vertex_2i (x_offset + rects->x + rects->width, y_offset + rects->y);
      vertex_2i (x_offset + rects->x + rects->width,
                 y_offset + rects->y + rects->height);
      vertex_2i (x_offset + rects->x, y_offset + rects->y + rects->height);
    }
  
    gl->end ();
  }
}

/* This is a hack that allows drawing with GLITZ_OPERATOR_SRC
   to all solid surfaces (1x1 surfaces) just by scaling the color
   to the appropriate format and storing it in the surface's solid
   color. */
static void
_glitz_fill_solid (glitz_surface_t *dst,
                   const glitz_color_t *color)
{  
  STORE_16 (dst->solid.red, dst->format->red_size, color->red);
  STORE_16 (dst->solid.green, dst->format->green_size, color->green);
  STORE_16 (dst->solid.blue, dst->format->blue_size, color->blue);
  STORE_16 (dst->solid.alpha, dst->format->alpha_size, color->alpha);
  
  dst->hint_mask |= GLITZ_INT_HINT_DRAWABLE_DIRTY_MASK;
}

void
glitz_fill_rectangle (glitz_operator_t op,
                      glitz_surface_t *dst,
                      const glitz_color_t *color,
                      int x,
                      int y,
                      unsigned int width,
                      unsigned int height)
{
  glitz_bounding_box_t bounds;
  glitz_rectangle_t rect;

  bounds.x1 = x;
  bounds.x2 = x + width;
  bounds.y1 = y;
  bounds.y2 = y + height;
  if (bounds.x1 > dst->width || bounds.y1 > dst->height ||
      bounds.x2 < 0 || bounds.y2 < 0)
    return;

  if (SURFACE_SOLID (dst) && SURFACE_OFFSCREEN (dst) &&
      op == GLITZ_OPERATOR_SRC && width > 0 && height > 0) {
    _glitz_fill_solid (dst, color);
    return;
  }

  rect.x = x;
  rect.y = y;
  rect.width = width;
  rect.height = height;

  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    glitz_surface_pop_current (dst);
    return;
  }

  glitz_int_fill_rectangles (op, dst, 0, 0, color, &rect, 1);

  glitz_surface_dirty (dst, &bounds);
  glitz_surface_pop_current (dst);
}
slim_hidden_def(glitz_fill_rectangle);

void
glitz_fill_rectangles (glitz_operator_t op,
                       glitz_surface_t *dst,
                       int x_offset,
                       int y_offset,
                       const glitz_color_t *color,
                       const glitz_rectangle_t *rects,
                       int n_rects)
{
  glitz_bounding_box_t bounds;
  
  glitz_rectangle_bounds (x_offset, y_offset, n_rects, rects, &bounds);
  if (bounds.x1 > dst->width || bounds.y1 > dst->height ||
      bounds.x2 < 0 || bounds.y2 < 0)
    return;

  if (SURFACE_SOLID (dst) && SURFACE_OFFSCREEN (dst) &&
      op == GLITZ_OPERATOR_SRC &&
      (bounds.x2 - bounds.x1) > 0 && (bounds.y2 - bounds.y1) > 0) {
    _glitz_fill_solid (dst, color);
    return;
  }

  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    glitz_surface_pop_current (dst);
    return;
  }

  glitz_int_fill_rectangles (op, dst, x_offset, y_offset,
                             color, rects, n_rects);

  glitz_surface_dirty (dst, &bounds);
  glitz_surface_pop_current (dst);
}
slim_hidden_def(glitz_fill_rectangles);
