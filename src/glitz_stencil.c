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
 * Author: David Reveman <c99drn@cs.umu.se>
 */

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "glitzint.h"

void
glitz_stencil_rectangles (glitz_stencil_operator_t op,
                          glitz_surface_t *dst,
                          int x_offset,
                          int y_offset,
                          const glitz_rectangle_t *rects,
                          int n_rects)
{
  static glitz_color_t color = { 0x0000, 0x0000, 0x0000, 0x0000 };
  glitz_operator_t rect_op;

  if (n_rects == 0)
    return;

  if (dst->format->stencil_size < 1)
    return;

  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    glitz_surface_pop_current (dst);
    return;
  }

  rect_op = (glitz_operator_t) GLITZ_INT_OPERATOR_STENCIL_RECT_SRC;

  switch (op) {
  case GLITZ_STENCIL_OPERATOR_CLEAR:
    rect_op = (glitz_operator_t) GLITZ_INT_OPERATOR_STENCIL_RECT_SET;
    break;
  case GLITZ_STENCIL_OPERATOR_DECR_LESS:
    *dst->stencil_mask &= (1 << (dst->format->stencil_size - 1));
    glitz_set_stencil_operator (dst->gl, op, *dst->stencil_mask);
    break;
  case GLITZ_STENCIL_OPERATOR_INTERSECT:
    glitz_set_stencil_operator (dst->gl, GLITZ_STENCIL_OPERATOR_INCR_EQUAL,
                                *dst->stencil_mask);
    break;
  case GLITZ_STENCIL_OPERATOR_SET:
  case GLITZ_STENCIL_OPERATOR_UNION:
    *dst->stencil_mask = 1 << (dst->format->stencil_size - 1);
    rect_op = (glitz_operator_t) GLITZ_INT_OPERATOR_STENCIL_RECT_SET;
    /* fall-through */
  default:
    glitz_set_stencil_operator (dst->gl, op, *dst->stencil_mask);
    break;
  }

  dst->gl->color_mask (0, 0, 0, 0);

  glitz_int_fill_rectangles (rect_op,
                             dst,
                             x_offset,
                             y_offset,
                             &color,
                             rects,
                             n_rects);

  if (op == GLITZ_STENCIL_OPERATOR_INTERSECT) {
    glitz_rectangle_t rect;

    rect.x = rect.y = 0;
    rect.width = dst->width;
    rect.height = dst->height;
    
    glitz_set_stencil_operator (dst->gl, GLITZ_STENCIL_OPERATOR_INTERSECT,
                                *dst->stencil_mask);
    glitz_int_fill_rectangles (GLITZ_INT_OPERATOR_STENCIL_RECT_SRC,
                               dst, 0, 0, &color, &rect, 1);
  } else if (op == GLITZ_STENCIL_OPERATOR_INCR_EQUAL)
    *dst->stencil_mask |= 0x1;

  dst->gl->color_mask (1, 1, 1, 1);
  
  dst->update_mask |= GLITZ_UPDATE_STENCIL_OP_MASK;

  glitz_surface_pop_current (dst);
}

void
glitz_stencil_trapezoids (glitz_stencil_operator_t op,
                          glitz_surface_t *dst,
                          int x_offset,
                          int y_offset,
                          const glitz_trapezoid_t *traps,
                          int n_traps)
{
  static glitz_color_t color = { 0x0000, 0x0000, 0x0000, 0x0000 };

  if (n_traps == 0)
    return;

  if (dst->format->stencil_size < 1)
    return;

  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    glitz_surface_pop_current (dst);
    return;
  }

  switch (op) {
  case GLITZ_STENCIL_OPERATOR_DECR_LESS:
    *dst->stencil_mask &= (1 << (dst->format->stencil_size - 1));
    glitz_set_stencil_operator (dst->gl, op, *dst->stencil_mask);
    break;
  case GLITZ_STENCIL_OPERATOR_INTERSECT:
    glitz_set_stencil_operator (dst->gl, GLITZ_STENCIL_OPERATOR_INCR_EQUAL,
                                *dst->stencil_mask);
    break;
  case GLITZ_STENCIL_OPERATOR_SET:
  case GLITZ_STENCIL_OPERATOR_UNION:
    *dst->stencil_mask = 1 << (dst->format->stencil_size - 1);
    /* fall-through */
  default:
    glitz_set_stencil_operator (dst->gl, op, *dst->stencil_mask);
    break;
  }
  
  dst->gl->color_mask (0, 0, 0, 0);

  glitz_int_fill_trapezoids (GLITZ_OPERATOR_SRC,
                             dst,
                             x_offset, y_offset,
                             &color,
                             traps,
                             n_traps);

  if (op == GLITZ_STENCIL_OPERATOR_INTERSECT) {
    glitz_rectangle_t rect;

    rect.x = rect.y = 0;
    rect.width = dst->width;
    rect.height = dst->height;
    
    glitz_set_stencil_operator (dst->gl, GLITZ_STENCIL_OPERATOR_INTERSECT,
                                *dst->stencil_mask);
    glitz_int_fill_rectangles (GLITZ_INT_OPERATOR_STENCIL_RECT_SRC,
                               dst, 0, 0, &color, &rect, 1);
  } else if (op == GLITZ_STENCIL_OPERATOR_INCR_EQUAL)
    *dst->stencil_mask |= 0x1;

  dst->gl->color_mask (1, 1, 1, 1);
  
  dst->update_mask |= GLITZ_UPDATE_STENCIL_OP_MASK;

  glitz_surface_pop_current (dst);
}

void
glitz_stencil_triangles (glitz_stencil_operator_t op,
                         glitz_surface_t *dst,
                         int x_offset,
                         int y_offset,
                         glitz_triangle_type_t type,
                         const glitz_point_fixed_t *points,
                         int n_points)                               
{
  static glitz_color_t color = { 0x0000, 0x0000, 0x0000, 0x0000 };

  if (n_points < 3)
    return;
  
  if (dst->format->stencil_size < 1)
    return;
  
  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    glitz_surface_pop_current (dst);
    return;
  }
  
  switch (op) {
  case GLITZ_STENCIL_OPERATOR_DECR_LESS:
    *dst->stencil_mask &= (1 << (dst->format->stencil_size - 1));
    glitz_set_stencil_operator (dst->gl, op, *dst->stencil_mask);
    break;
  case GLITZ_STENCIL_OPERATOR_INTERSECT:
    glitz_set_stencil_operator (dst->gl, GLITZ_STENCIL_OPERATOR_INCR_EQUAL,
                                *dst->stencil_mask);
    break;
  case GLITZ_STENCIL_OPERATOR_SET:
  case GLITZ_STENCIL_OPERATOR_UNION:
    *dst->stencil_mask = 1 << (dst->format->stencil_size - 1);
    /* fall-through */
  default:
    glitz_set_stencil_operator (dst->gl, op, *dst->stencil_mask);
    break;
  }

  dst->gl->color_mask (0, 0, 0, 0);

  glitz_int_fill_triangles (GLITZ_OPERATOR_SRC,
                            dst,
                            type,
                            x_offset, y_offset,
                            &color,                        
                            points,
                            n_points);

  if (op == GLITZ_STENCIL_OPERATOR_INTERSECT) {
    glitz_rectangle_t rect;

    rect.x = rect.y = 0;
    rect.width = dst->width;
    rect.height = dst->height;
    
    glitz_set_stencil_operator (dst->gl, GLITZ_STENCIL_OPERATOR_INTERSECT,
                                *dst->stencil_mask);
    glitz_int_fill_rectangles (GLITZ_INT_OPERATOR_STENCIL_RECT_SRC,
                               dst, 0, 0, &color, &rect, 1);
  } else if (op == GLITZ_STENCIL_OPERATOR_INCR_EQUAL)
    *dst->stencil_mask |= 0x1;

  dst->gl->color_mask (1, 1, 1, 1);

  dst->update_mask |= GLITZ_UPDATE_STENCIL_OP_MASK;

  glitz_surface_pop_current (dst);
}
