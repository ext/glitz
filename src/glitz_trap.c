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
#define TRAPEZOID_VALID(t) ((t)->left.p1.y != (t)->left.p2.y && \
			     (t)->right.p1.y != (t)->right.p2.y && \
			     (int) ((t)->bottom - (t)->top) > 0)

/* whether 't' is a well defined not obviously empty color trapezoid */
#define COLORTRAPEZOID_VALID(t) ((t)->top.left <= (t)->top.right && \
			     (t)->bottom.left <= (t)->bottom.right && \
			     (int) ((t)->bottom.y - (t)->top.y) >= 0)

static glitz_fixed16_16_t
glitz_line_fixed_x (const glitz_line_fixed_t *l,
                    glitz_fixed16_16_t y,
                    int ceil)
{
  glitz_fixed_32_32 ex = (glitz_fixed_32_32) (y - l->p1.y) *
    (l->p2.x - l->p1.x);
  glitz_fixed16_16_t dy = l->p2.y - l->p1.y;
  
  if (ceil)
    ex += (dy - 1);
  
  return l->p1.x + (glitz_fixed16_16_t) (ex / dy);
}

static void
glitz_trapezoid_bounds (int n_traps,
                        const glitz_trapezoid_t *traps,
                        glitz_bounding_box_t *box)
{
  box->y1 = MAXSHORT;
  box->y2 = MINSHORT;
  box->x1 = MAXSHORT;
  box->x2 = MINSHORT;
  
  for (; n_traps; n_traps--, traps++) {
    int16_t x1, y1, x2, y2;
    
    if (!TRAPEZOID_VALID (traps))
      continue;
    
    y1 = FIXED_TO_INT (traps->top);
    if (y1 < box->y1)
      box->y1 = y1;

    y2 = FIXED_TO_INT (FIXED_CEIL (traps->bottom));
    if (y2 > box->y2)
      box->y2 = y2;

    x1 = FIXED_TO_INT (MIN (glitz_line_fixed_x (&traps->left, traps->top, 0),
                            glitz_line_fixed_x (&traps->left,
                                                traps->bottom, 0)));
    if (x1 < box->x1)
      box->x1 = x1;

    x2 = FIXED_TO_INT (FIXED_CEIL
                       (MAX (glitz_line_fixed_x (&traps->right, traps->top, 1),
                             glitz_line_fixed_x (&traps->right,
                                                 traps->bottom, 1))));
    if (x2 > box->x2)
      box->x2 = x2;
  }
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
  glitz_gl_uint_t list = 0;

  dst->gl->color_4us (color->red, color->green, color->blue, color->alpha);

  glitz_set_operator (dst->gl, op);

  if (dst->multi_sample) {
    list = dst->gl->gen_lists (1);
    dst->gl->new_list (list, GLITZ_GL_COMPILE);
  }
    
  dst->gl->begin (GLITZ_GL_QUADS);

  vertex_2d = dst->gl->vertex_2d;
  for (; n_traps; n_traps--, traps++) {
    double top, bottom;
    
    if (!TRAPEZOID_VALID (traps))
      continue;
    
    top = y_offset + FIXED_TO_DOUBLE (traps->top);
    bottom = y_offset + FIXED_TO_DOUBLE (traps->bottom);

    vertex_2d (x_offset +
               FIXED_TO_DOUBLE (glitz_line_fixed_x
                                (&traps->left, traps->top, 0)), top);
    vertex_2d (x_offset +
               FIXED_TO_DOUBLE (glitz_line_fixed_x
                                (&traps->right, traps->top, 1)), top);
    vertex_2d (x_offset +
               FIXED_TO_DOUBLE (glitz_line_fixed_x
                                (&traps->right, traps->bottom, 1)), bottom);
    vertex_2d (x_offset +
               FIXED_TO_DOUBLE (glitz_line_fixed_x
                                (&traps->left, traps->bottom, 0)), bottom);
  }

  dst->gl->end ();

  if (list) {
    int i;
    
    dst->gl->end_list ();

    for (i = 0; i < dst->multi_sample->n_samples; i++) {
      dst->gl->translate_d (dst->multi_sample->offsets[i].x,
                            -dst->multi_sample->offsets[i].y, 0.0);
      dst->gl->call_list (list);
      dst->gl->translate_d (-dst->multi_sample->offsets[i].x,
                            dst->multi_sample->offsets[i].y, 0.0);
    }

    dst->gl->delete_lists (list, 1);
  }
}

void
glitz_fill_trapezoids (glitz_operator_t op,
                       glitz_surface_t *dst,
                       const glitz_color_t *color,
                       const glitz_trapezoid_t *traps,
                       int n_traps)
{
  glitz_bounding_box_t bounds;
  
  glitz_trapezoid_bounds (n_traps, traps, &bounds);
  if (bounds.x1 > dst->width || bounds.y1 > dst->height ||
      bounds.x2 < 0 || bounds.y2 < 0)
    return;

  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_pop_current (dst);
    return;
  }
  
  glitz_int_fill_trapezoids (op, dst, 0, 0, color, traps, n_traps);

  glitz_surface_dirty (dst, &bounds);
  glitz_surface_pop_current (dst);
}
slim_hidden_def(glitz_fill_trapezoids);

void
glitz_composite_trapezoids (glitz_operator_t op,
                            glitz_surface_t *src,
                            glitz_surface_t *dst,
                            int x_src,
                            int y_src,
                            const glitz_trapezoid_t *traps,
                            int n_traps)
{
  glitz_surface_t *mask;
  glitz_bounding_box_t trap_bounds;
  int x_dst, y_dst;
  glitz_rectangle_t rect;

  if (n_traps == 0)
    return;

  x_dst = traps[0].left.p1.x >> 16;
  y_dst = traps[0].left.p1.y >> 16;

  if (dst->format->stencil_size < ((*dst->stencil_mask)? 2: 1)) {
    glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    return;
  }

  glitz_trapezoid_bounds (n_traps, traps, &trap_bounds);
  if (trap_bounds.x1 > dst->width || trap_bounds.y1 > dst->height ||
      trap_bounds.x2 < 0 || trap_bounds.y2 < 0)
    return;

  glitz_surface_enable_anti_aliasing (dst);
  
  glitz_stencil_trapezoids (dst,
                            GLITZ_STENCIL_OPERATOR_INCR_EQUAL,
                            traps, n_traps);
  
  rect.x = trap_bounds.x1;
  rect.y = trap_bounds.y1;
  rect.width = trap_bounds.x2 - trap_bounds.x1;
  rect.height = trap_bounds.y2 - trap_bounds.y1;
  
  if (dst->polyopacity != 0xffff) {
    glitz_color_t color;
    
    color.red = color.green = color.blue = color.alpha = dst->polyopacity;
    mask = glitz_surface_create_solid (&color);
  } else
    mask = NULL;

  glitz_composite (op,
                   src,
                   mask,
                   dst,
                   x_src + trap_bounds.x1 - x_dst,
                   y_src + trap_bounds.y1 - y_dst,
                   0, 0,
                   rect.x, rect.y,
                   rect.width, rect.height);

  if (mask)
    glitz_surface_destroy (mask);

  glitz_surface_disable_anti_aliasing (dst);

  glitz_stencil_rectangles (dst,
                            GLITZ_STENCIL_OPERATOR_DECR_LESS,
                            &rect, 1);
}
slim_hidden_def(glitz_composite_trapezoids);

static void
glitz_color_trapezoid_bounds (int n_color_traps,
                              const glitz_color_trapezoid_t *color_traps,
                              glitz_bounding_box_t *box)
{
  box->y1 = MAXSHORT;
  box->y2 = MINSHORT;
  box->x1 = MAXSHORT;
  box->x2 = MINSHORT;
  
  for (; n_color_traps; n_color_traps--, color_traps++) {
    int16_t x1, y1, x2, y2;

    if (!COLORTRAPEZOID_VALID (color_traps))
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
}

void
glitz_color_trapezoids (glitz_operator_t op,
                        glitz_surface_t *dst,
                        const glitz_color_trapezoid_t *color_traps,
                        int n_color_traps)
{
  glitz_gl_vertex_2d_t vertex_2d;
  glitz_gl_color_4us_t color_4us;
  glitz_bounding_box_t bounds;
  int index;
  glitz_bool_t shade = 0;
  
  glitz_color_trapezoid_bounds (n_color_traps, color_traps, &bounds);
  if (bounds.y1 >= bounds.y2 || bounds.x1 >= bounds.x2 ||
      bounds.x1 > dst->width || bounds.y1 > dst->height ||
      bounds.x2 < 0 || bounds.y2 < 0)
    return;
  
  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_pop_current (dst);
    return;
  }

  glitz_set_operator (dst->gl, op);

  for (index = 0; index < n_color_traps; index++) {
    
    if (!COLORTRAPEZOID_VALID (&color_traps[index]))
      continue;
    
    if (memcmp (&color_traps[index].top.right_color,
                &color_traps[index].top.left_color,
                sizeof (glitz_color_t)) ||
        memcmp (&color_traps[index].bottom.right_color,
                &color_traps[index].bottom.left_color,
                sizeof (glitz_color_t)) ||
        memcmp (&color_traps[index].top.left_color,
                &color_traps[index].bottom.left_color,
                sizeof (glitz_color_t))) {
      dst->gl->shade_model (GLITZ_GL_SMOOTH);
      shade = 1;
      break;
    }
  }
  
  dst->gl->begin (GLITZ_GL_QUADS);

  vertex_2d = dst->gl->vertex_2d;
  color_4us = dst->gl->color_4us;
  for (; n_color_traps; n_color_traps--, color_traps++) {
    
    if (!COLORTRAPEZOID_VALID (color_traps))
      continue;

    color_4us (color_traps->bottom.left_color.red,
               color_traps->bottom.left_color.green,
               color_traps->bottom.left_color.blue,
               color_traps->bottom.left_color.alpha);
    vertex_2d (FIXED_TO_DOUBLE (color_traps->bottom.left),
               FIXED_TO_DOUBLE (color_traps->bottom.y));

    if (shade)
      color_4us (color_traps->top.left_color.red,
                 color_traps->top.left_color.green,
                 color_traps->top.left_color.blue,
                 color_traps->top.left_color.alpha);
    vertex_2d (FIXED_TO_DOUBLE (color_traps->top.left),
               FIXED_TO_DOUBLE (color_traps->top.y));

    if (shade)
      color_4us (color_traps->top.right_color.red,
                 color_traps->top.right_color.green,
                 color_traps->top.right_color.blue,
                 color_traps->top.right_color.alpha);
    vertex_2d (FIXED_TO_DOUBLE (color_traps->top.right),
               FIXED_TO_DOUBLE (color_traps->top.y));

    if (shade)
      color_4us (color_traps->bottom.right_color.red,
                 color_traps->bottom.right_color.green,
                 color_traps->bottom.right_color.blue,
                 color_traps->bottom.right_color.alpha);
    vertex_2d (FIXED_TO_DOUBLE (color_traps->bottom.right),
               FIXED_TO_DOUBLE (color_traps->bottom.y));
  }
  
  dst->gl->end ();

  glitz_surface_dirty (dst, &bounds);
  glitz_surface_pop_current (dst);
}
slim_hidden_def(glitz_color_trapezoids);
