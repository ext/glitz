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

/* whether 't' is a well defined not obviously empty trapezoid */
#define TRAPEZOID_VALID(t) ((t)->top.left <= (t)->top.right && \
			     (t)->bottom.left <= (t)->bottom.right && \
			     (int) ((t)->bottom.y - (t)->top.y) >= 0)

static void
glitz_trapezoid_bounds (int x_offset,
                        int y_offset,
                        int n_traps,
                        const glitz_trapezoid_t *traps,
                        glitz_bounding_box_t *box)
{
  box->x1 = MAXSHORT;
  box->x2 = MINSHORT;
  box->y1 = MAXSHORT;
  box->y2 = MINSHORT;
  
  for (; n_traps; n_traps--, traps++) {
    int x1, y1, x2, y2;

    if (!TRAPEZOID_VALID (traps))
      continue;
    
    x1 = MIN (FIXED_TO_INT (traps->top.left),
              FIXED_TO_INT (traps->bottom.left));
    if (x1 < box->x1)
      box->x1 = x1;
    
    x2 = MAX (FIXED_TO_INT (FIXED_CEIL (traps->top.right)),
              FIXED_TO_INT (FIXED_CEIL (traps->bottom.right)));
    if (x2 > box->x2)
      box->x2 = x2;
    
    y1 = FIXED_TO_INT (traps->top.y);
    if (y1 < box->y1)
      box->y1 = y1;
    
    y2 = FIXED_TO_INT (FIXED_CEIL (traps->bottom.y));
    if (y2 > box->y2)
      box->y2 = y2;
  }

  box->x1 += x_offset;
  box->x2 += x_offset;
  box->y1 += y_offset;
  box->y2 += y_offset;
}

void
glitz_int_fill_trapezoids (glitz_operator_t op,
                           glitz_surface_t *dst,
                           int x_offset,
                           int y_offset,
                           const glitz_color_t *color,
                           const glitz_trapezoid_t *traps,
                           int n_traps)
{
  glitz_gl_vertex_2d_t vertex_2d;
  static glitz_sample_offset_t zero_offset = { 0.0, 0.0 };
  glitz_sample_offset_t *offset;
  int i, passes;

  dst->gl->color_4us (color->red, color->green, color->blue, color->alpha);

  glitz_set_operator (dst->gl, op);

  if (dst->multi_sample) {
    passes = dst->multi_sample->n_samples;
    offset = dst->multi_sample->offsets;
  } else {
    passes = 1;
    offset = &zero_offset;
  }

  vertex_2d = dst->gl->vertex_2d;
  
  dst->gl->begin (GLITZ_GL_QUADS);

  for (; n_traps; n_traps--, traps++) {
    double top, topleft, topright;
    double bottom, bottomleft, bottomright;
    
      if (!TRAPEZOID_VALID (traps))
        continue;

      top = y_offset + FIXED_TO_DOUBLE (traps->top.y);
      bottom = y_offset + FIXED_TO_DOUBLE (traps->bottom.y);
      bottomleft = x_offset + FIXED_TO_DOUBLE (traps->bottom.left);
      topleft = x_offset + FIXED_TO_DOUBLE (traps->top.left);
      topright = x_offset + FIXED_TO_DOUBLE (traps->top.right);
      bottomright = x_offset + FIXED_TO_DOUBLE (traps->bottom.right);
      
      for (i = 0; i < passes; i++) {
        vertex_2d (offset[i].x + bottomleft, offset[i].y + bottom);
        vertex_2d (offset[i].x + topleft, offset[i].y + top);
        vertex_2d (offset[i].x + topright, offset[i].y + top);
        vertex_2d (offset[i].x + bottomright, offset[i].y + bottom);
    }
  }

  dst->gl->end ();
}

void
glitz_fill_trapezoids (glitz_operator_t op,
                       glitz_surface_t *dst,
                       int x_offset,
                       int y_offset,
                       const glitz_color_t *color,
                       const glitz_trapezoid_t *traps,
                       int n_traps)
{
  glitz_bounding_box_t bounds;
  
  glitz_trapezoid_bounds (x_offset, y_offset, n_traps, traps, &bounds);
  if (bounds.x1 > dst->width || bounds.y1 > dst->height ||
      bounds.x2 < 0 || bounds.y2 < 0)
    return;

  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    glitz_surface_pop_current (dst);
    return;
  }
  
  glitz_int_fill_trapezoids (op, dst,
                             x_offset, y_offset,
                             color, traps, n_traps);

  glitz_surface_dirty (dst, &bounds);
  glitz_surface_pop_current (dst);
}
slim_hidden_def(glitz_fill_trapezoids);

void
glitz_add_trapezoids (glitz_surface_t *dst,
                      int x_offset,
                      int y_offset,
                      const glitz_trapezoid_t *traps,
                      int n_traps)
{
  glitz_bounding_box_t bounds;
  glitz_color_t color;
  
  glitz_trapezoid_bounds (x_offset, y_offset, n_traps, traps, &bounds);
  if (bounds.x1 > dst->width || bounds.y1 > dst->height ||
      bounds.x2 < 0 || bounds.y2 < 0)
    return;

  glitz_surface_enable_anti_aliasing (dst);

  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    glitz_surface_pop_current (dst);
    return;
  }

  if (dst->multi_sample)
    color.red = color.green = color.blue = color.alpha =
      0xffff / dst->multi_sample->n_samples;
  else
    color.red = color.green = color.blue = color.alpha = 0xffff;
  
  glitz_int_fill_trapezoids (GLITZ_OPERATOR_ADD, dst,
                             x_offset, y_offset,
                             &color, traps, n_traps);

  glitz_surface_disable_anti_aliasing (dst);

  glitz_surface_dirty (dst, &bounds);
  glitz_surface_pop_current (dst);
}
slim_hidden_def(glitz_add_trapezoids);

void
glitz_composite_trapezoids (glitz_operator_t op,
                            glitz_surface_t *src,
                            glitz_surface_t *dst,
                            int x_src,
                            int y_src,
                            int x_offset,
                            int y_offset,
                            unsigned short opacity,
                            const glitz_trapezoid_t *traps,
                            int n_traps)
{
  glitz_bounding_box_t bounds;
  glitz_rectangle_t rect;

  if (n_traps == 0)
    return;

  if (dst->format->stencil_size < ((*dst->stencil_mask)? 2: 1)) {
    glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    return;
  }

  glitz_trapezoid_bounds (x_offset, y_offset, n_traps, traps, &bounds);
  if (bounds.x1 > dst->width || bounds.y1 > dst->height ||
      bounds.x2 < 0 || bounds.y2 < 0)
    return;

  rect.x = bounds.x1;
  rect.y = bounds.y1;
  rect.width = bounds.x2 - bounds.x1;
  rect.height = bounds.y2 - bounds.y1;

  glitz_surface_enable_anti_aliasing (dst);

  if (*dst->stencil_mask == 0x0)
    glitz_stencil_rectangles (GLITZ_STENCIL_OPERATOR_CLEAR,
                              dst, 0, 0, &rect, 1);
  
  glitz_stencil_trapezoids (GLITZ_STENCIL_OPERATOR_INCR_EQUAL,
                            dst,
                            x_offset, y_offset,
                            traps, n_traps);

  dst->polyopacity = opacity;
  dst->hint_mask |= GLITZ_INT_HINT_POLYGON_OP_MASK;

  glitz_composite (op,
                   src,
                   NULL,
                   dst,
                   x_src + bounds.x1,
                   y_src + bounds.y1,
                   0, 0,
                   rect.x, rect.y,
                   rect.width, rect.height);

  dst->hint_mask &= ~GLITZ_INT_HINT_POLYGON_OP_MASK;

  glitz_surface_disable_anti_aliasing (dst);
  
  if (*dst->stencil_mask != 0x1)
    glitz_stencil_rectangles (GLITZ_STENCIL_OPERATOR_DECR_LESS,
                              dst, 0, 0, &rect, 1);
  else
    *dst->stencil_mask = 0x0;

  dst->update_mask |= GLITZ_UPDATE_STENCIL_OP_MASK;
}
slim_hidden_def(glitz_composite_trapezoids);

static void
glitz_color_trapezoid_bounds (int x_offset,
                              int y_offset,
                              int n_color_traps,
                              const glitz_color_trapezoid_t *color_traps,
                              glitz_bounding_box_t *box)
{
  box->x1 = MAXSHORT;
  box->x2 = MINSHORT;
  box->y1 = MAXSHORT;
  box->y2 = MINSHORT;
  
  for (; n_color_traps; n_color_traps--, color_traps++) {
    int x1, y1, x2, y2;

    if (!TRAPEZOID_VALID (color_traps))
      continue;
    
    x1 = MIN (FIXED_TO_INT (color_traps->top.left),
              FIXED_TO_INT (color_traps->bottom.left));
    if (x1 < box->x1)
      box->x1 = x1;
    
    x2 = MAX (FIXED_TO_INT (FIXED_CEIL (color_traps->top.right)),
              FIXED_TO_INT (FIXED_CEIL (color_traps->bottom.right)));
    if (x2 > box->x2)
      box->x2 = x2;
    
    y1 = FIXED_TO_INT (color_traps->top.y);
    if (y1 < box->y1)
      box->y1 = y1;
    
    y2 = FIXED_TO_INT (FIXED_CEIL (color_traps->bottom.y));
    if (y2 > box->y2)
      box->y2 = y2;
  }

  box->x1 += x_offset;
  box->x2 += x_offset;
  box->y1 += y_offset;
  box->y2 += y_offset;
}

void
glitz_color_trapezoids (glitz_operator_t op,
                        glitz_surface_t *dst,
                        int x_offset,
                        int y_offset,
                        const glitz_color_trapezoid_t *color_traps,
                        int n_color_traps)
{
  glitz_gl_vertex_2d_t vertex_2d;
  glitz_gl_color_4us_t color_4us;
  glitz_bounding_box_t bounds;
  
  glitz_color_trapezoid_bounds (x_offset, y_offset,
                                n_color_traps, color_traps, &bounds);
  if (bounds.x1 > dst->width || bounds.y1 > dst->height ||
      bounds.x2 < 0 || bounds.y2 < 0)
    return;
  
  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    glitz_surface_pop_current (dst);
    return;
  }

  glitz_set_operator (dst->gl, op);

  dst->gl->shade_model (GLITZ_GL_SMOOTH);
  
  dst->gl->begin (GLITZ_GL_QUADS);

  vertex_2d = dst->gl->vertex_2d;
  color_4us = dst->gl->color_4us;
  for (; n_color_traps; n_color_traps--, color_traps++) {
    
    if (!TRAPEZOID_VALID (color_traps))
      continue;

    color_4us (color_traps->bottom.left_color.red,
               color_traps->bottom.left_color.green,
               color_traps->bottom.left_color.blue,
               color_traps->bottom.left_color.alpha);
    vertex_2d (x_offset + FIXED_TO_DOUBLE (color_traps->bottom.left),
               y_offset + FIXED_TO_DOUBLE (color_traps->bottom.y));

    color_4us (color_traps->top.left_color.red,
               color_traps->top.left_color.green,
               color_traps->top.left_color.blue,
               color_traps->top.left_color.alpha);
    vertex_2d (x_offset + FIXED_TO_DOUBLE (color_traps->top.left),
               y_offset + FIXED_TO_DOUBLE (color_traps->top.y));

    color_4us (color_traps->top.right_color.red,
               color_traps->top.right_color.green,
               color_traps->top.right_color.blue,
               color_traps->top.right_color.alpha);
    vertex_2d (x_offset + FIXED_TO_DOUBLE (color_traps->top.right),
               y_offset + FIXED_TO_DOUBLE (color_traps->top.y));

    color_4us (color_traps->bottom.right_color.red,
               color_traps->bottom.right_color.green,
               color_traps->bottom.right_color.blue,
               color_traps->bottom.right_color.alpha);
    vertex_2d (x_offset + FIXED_TO_DOUBLE (color_traps->bottom.right),
               y_offset + FIXED_TO_DOUBLE (color_traps->bottom.y));
  }
  
  dst->gl->end ();

  dst->gl->shade_model (GLITZ_GL_FLAT);

  glitz_surface_dirty (dst, &bounds);
  glitz_surface_pop_current (dst);
}
slim_hidden_def(glitz_color_trapezoids);
