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

static void
glitz_point_fixed_bounds (int n_point,
                          const glitz_point_fixed_t *points,
                          glitz_bounding_box_t *box)
{
  box->x1 = FIXED_TO_INT (points->x);
  box->x2 = FIXED_TO_INT (FIXED_CEIL (points->x));
  box->y1 = FIXED_TO_INT (points->y);
  box->y2 = FIXED_TO_INT (FIXED_CEIL (points->y));
  points++;
  n_point--;
    
  while (n_point-- > 0) {
    int x1 = FIXED_TO_INT (points->x);
    int x2 = FIXED_TO_INT (FIXED_CEIL (points->x));
    int y1 = FIXED_TO_INT (points->y);
    int y2 = FIXED_TO_INT (FIXED_CEIL (points->y));
      
    if (x1 < box->x1)
      box->x1 = x1;
    else if (x2 > box->x2)
      box->x2 = x2;
    if (y1 < box->y1)
      box->y1 = y1;
    else if (y2 > box->y2)
      box->y2 = y2;
    
    points++;
  }
}

void
glitz_int_fill_triangles (glitz_operator_t op,
                          glitz_surface_t *dst,
                          glitz_triangle_type_t type,
                          int x_offset,
                          int y_offset,
                          const glitz_color_t *color,
                          const glitz_point_fixed_t *points,
                          int n_points)
{
  glitz_gl_vertex_2d_t vertex_2d;
  glitz_gl_uint_t list = 0;
  
  dst->gl->color_4us (color->red, color->green, color->blue, color->alpha);

  glitz_set_operator (dst->gl, op);

  if (dst->multi_sample) {
    list = dst->gl->gen_lists (1);
    dst->gl->new_list (list, GLITZ_GL_COMPILE);
  }

  switch (type) {
  case GLITZ_TRIANGLE_TYPE_NORMAL:
    dst->gl->begin (GLITZ_GL_TRIANGLES);
    break;
  case GLITZ_TRIANGLE_TYPE_STRIP:
    dst->gl->begin (GLITZ_GL_TRIANGLE_STRIP);
    break;
  case GLITZ_TRIANGLE_TYPE_FAN:
    dst->gl->begin (GLITZ_GL_TRIANGLE_FAN);
    break;
  }

  vertex_2d = dst->gl->vertex_2d;
  for (; n_points; n_points--, points++)
    vertex_2d (x_offset + FIXED_TO_DOUBLE (points->x),
               y_offset + FIXED_TO_DOUBLE (points->y));
  
  dst->gl->end ();

  if (list) {
    int i;
    
    dst->gl->end_list ();

    dst->gl->matrix_mode (GLITZ_GL_MODELVIEW);

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
glitz_fill_triangles (glitz_operator_t op,
                      glitz_surface_t *dst,
                      const glitz_color_t *color,
                      const glitz_triangle_t *tris,
                      int n_tris)
{
  glitz_bounding_box_t bounds;
  
  glitz_point_fixed_bounds (n_tris * 3, (glitz_point_fixed_t *) tris, &bounds);
  if (bounds.x1 > dst->width || bounds.y1 > dst->height ||
      bounds.x2 < 0 || bounds.y2 < 0)
    return;
    
  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_pop_current (dst);
    return;
  }

  glitz_int_fill_triangles (op,
                            dst,
                            GLITZ_TRIANGLE_TYPE_NORMAL,
                            0, 0,
                            color,
                            (glitz_point_fixed_t *) tris,
                            n_tris * 3);

  glitz_surface_dirty (dst, &bounds);  
  glitz_surface_pop_current (dst);
}
slim_hidden_def(glitz_fill_triangles);

static void
glitz_int_composite_triangles (glitz_operator_t op,
                               glitz_surface_t *src,
                               glitz_surface_t *dst,
                               int x_src,
                               int y_src,
                               glitz_triangle_type_t type,
                               const glitz_point_fixed_t *points,
                               int n_points)
{
  glitz_bounding_box_t tri_bounds;
  int x_dst, y_dst;
  glitz_rectangle_t rect;

  x_dst = points[0].x >> 16;
  y_dst = points[0].y >> 16;

  if (dst->format->stencil_size < ((*dst->stencil_mask)? 2: 1)) {
    glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    return;
  }

  glitz_point_fixed_bounds (n_points, points, &tri_bounds);

  if (tri_bounds.x1 > dst->width || tri_bounds.y1 > dst->height ||
      tri_bounds.x2 < 0 || tri_bounds.y2 < 0)
    return;

  rect.x = tri_bounds.x1;
  rect.y = tri_bounds.y1;
  rect.width = tri_bounds.x2 - tri_bounds.x1;
  rect.height = tri_bounds.y2 - tri_bounds.y1;
  
  glitz_surface_enable_anti_aliasing (dst);

  if (*dst->stencil_mask == 0x0)
    glitz_stencil_rectangles (dst,
                              GLITZ_STENCIL_OPERATOR_CLEAR,
                              &rect, 1);
  
  glitz_stencil_triangles (dst,
                           GLITZ_STENCIL_OPERATOR_INCR_EQUAL,
                           type,
                           points,
                           n_points);
    
  dst->hint_mask |= GLITZ_INT_HINT_POLYGON_OP_MASK;

  glitz_composite (op,
                   src,
                   NULL,
                   dst,
                   x_src + tri_bounds.x1 - x_dst,
                   y_src + tri_bounds.y1 - y_dst,
                   0, 0,
                   rect.x, rect.y,
                   rect.width, rect.height);

  dst->hint_mask &= ~GLITZ_INT_HINT_POLYGON_OP_MASK;

  glitz_surface_disable_anti_aliasing (dst);
    
  if (*dst->stencil_mask != 0x1)
    glitz_stencil_rectangles (dst,
                              GLITZ_STENCIL_OPERATOR_DECR_LESS,
                              &rect, 1);
  else
    *dst->stencil_mask = 0x0;

  dst->update_mask |= GLITZ_UPDATE_STENCIL_OP_MASK;
}

void
glitz_composite_triangles (glitz_operator_t op,
                           glitz_surface_t *src,
                           glitz_surface_t *dst,
                           int x_src,
                           int y_src,
                           const glitz_triangle_t *tris,
                           int n_tris)
{
  glitz_int_composite_triangles (op, src, dst, x_src, y_src,
                                 GLITZ_TRIANGLE_TYPE_NORMAL,
                                 (glitz_point_fixed_t *) tris,
                                 n_tris * 3);
}
slim_hidden_def(glitz_composite_triangles);
     
void
glitz_composite_tri_strip (glitz_operator_t op,
                           glitz_surface_t *src,
                           glitz_surface_t *dst,
                           int x_src,
                           int y_src,
                           const glitz_point_fixed_t *points,
                           int n_points)
{
  glitz_int_composite_triangles (op, src, dst, x_src, y_src,
                                 GLITZ_TRIANGLE_TYPE_STRIP,
                                 points, n_points);
}
slim_hidden_def(glitz_composite_tri_strip);

void
glitz_composite_tri_fan (glitz_operator_t op,
                         glitz_surface_t *src,
                         glitz_surface_t *dst,
                         int x_src,
                         int y_src,
                         const glitz_point_fixed_t *points,
                         int n_points)
{
  glitz_int_composite_triangles (op, src, dst, x_src, y_src,
                                 GLITZ_TRIANGLE_TYPE_FAN,
                                 points, n_points);
}
slim_hidden_def(glitz_composite_tri_fan);

static void
glitz_color_triangle_bounds (int n_color_tris,
                             const glitz_color_triangle_t *color_tris,
                             glitz_bounding_box_t *box)
{
  box->x1 = MIN (MIN (FIXED_TO_INT (color_tris->p1.point.x),
                      FIXED_TO_INT (color_tris->p2.point.x)),
                 FIXED_TO_INT (color_tris->p3.point.x));

  box->x2 = MAX (MAX (FIXED_TO_INT (FIXED_CEIL (color_tris->p1.point.x)),
                      FIXED_TO_INT (FIXED_CEIL (color_tris->p2.point.x))),
                 FIXED_TO_INT (FIXED_CEIL (color_tris->p3.point.x)));
  
  box->y1 = MIN (MIN (FIXED_TO_INT (color_tris->p1.point.y),
                      FIXED_TO_INT (color_tris->p2.point.y)),
                 FIXED_TO_INT (color_tris->p3.point.y));
  
  box->y2 = MAX (MAX (FIXED_TO_INT (FIXED_CEIL (color_tris->p1.point.y)),
                      FIXED_TO_INT (FIXED_CEIL (color_tris->p2.point.y))),
                 FIXED_TO_INT (FIXED_CEIL (color_tris->p3.point.y)));
  
  color_tris++;
  n_color_tris--;

  while (n_color_tris-- > 0) {
    int16_t x1, y1, x2, y2;

    x1 = MIN (MIN (FIXED_TO_INT (color_tris->p1.point.x),
                   FIXED_TO_INT (color_tris->p2.point.x)),
              FIXED_TO_INT (color_tris->p3.point.x));
    if (x1 < box->x1)
      box->x1 = x1;
    
    x2 = MAX (MAX (FIXED_TO_INT (FIXED_CEIL (color_tris->p1.point.x)),
                   FIXED_TO_INT (FIXED_CEIL (color_tris->p2.point.x))),
              FIXED_TO_INT (FIXED_CEIL (color_tris->p3.point.x)));
    if (x2 > box->x2)
      box->x2 = x2;
    
    y1 = MIN (MIN (FIXED_TO_INT (color_tris->p1.point.y),
                   FIXED_TO_INT (color_tris->p2.point.y)),
              FIXED_TO_INT (color_tris->p3.point.y));
    if (y1 < box->y1)
      box->y1 = y1;
    
    y2 = MAX (MAX (FIXED_TO_INT (FIXED_CEIL (color_tris->p1.point.y)),
                   FIXED_TO_INT (FIXED_CEIL (color_tris->p2.point.y))),
              FIXED_TO_INT (FIXED_CEIL (color_tris->p3.point.y)));
    if (y2 > box->y2)
      box->y2 = y2;
    
    color_tris++;
  }
}


void
glitz_color_triangles (glitz_operator_t op,
                       glitz_surface_t *dst,
                       const glitz_color_triangle_t *color_tris,
                       int n_color_tris)
{
  glitz_gl_vertex_2d_t vertex_2d;
  glitz_gl_color_4us_t color_4us;
  glitz_bounding_box_t bounds;
  int index;
  glitz_bool_t shade = 0;

  glitz_color_triangle_bounds (n_color_tris, color_tris, &bounds);
  if (bounds.y1 >= bounds.y2 || bounds.x1 >= bounds.x2 ||
      bounds.x1 > dst->width || bounds.y1 > dst->height ||
      bounds.x2 < 0 || bounds.y2 < 0)
    return;
  
  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_pop_current (dst);
    return;
  }

  glitz_set_operator (dst->gl, op);

  for (index = 0; index < n_color_tris; index++) {
    if (memcmp (&color_tris[index].p1.color,
                &color_tris[index].p2.color, sizeof (glitz_color_t)) ||
        memcmp (&color_tris[index].p2.color,
                &color_tris[index].p3.color, sizeof (glitz_color_t))) {
      dst->gl->shade_model (GLITZ_GL_SMOOTH);
      shade = 1;
      break;
    }
  }

  dst->gl->begin (GLITZ_GL_TRIANGLES);

  vertex_2d = dst->gl->vertex_2d;
  color_4us = dst->gl->color_4us;
  for (; n_color_tris; n_color_tris--, color_tris++) {    
    color_4us (color_tris->p1.color.red,
               color_tris->p1.color.green,
               color_tris->p1.color.blue,
               color_tris->p1.color.alpha);
    vertex_2d (FIXED_TO_DOUBLE (color_tris->p1.point.x),
               FIXED_TO_DOUBLE (color_tris->p1.point.y));
    
    if (shade)
      color_4us (color_tris->p2.color.red,
                 color_tris->p2.color.green,
                 color_tris->p2.color.blue,
                 color_tris->p2.color.alpha);
    vertex_2d (FIXED_TO_DOUBLE (color_tris->p2.point.x),
               FIXED_TO_DOUBLE (color_tris->p2.point.y));

    if (shade)
      color_4us (color_tris->p3.color.red,
                 color_tris->p3.color.green,
                 color_tris->p3.color.blue,
                 color_tris->p3.color.alpha);
    vertex_2d (FIXED_TO_DOUBLE (color_tris->p3.point.x),
               FIXED_TO_DOUBLE (color_tris->p3.point.y));
  }

  dst->gl->end ();

  glitz_surface_dirty (dst, &bounds);
  glitz_surface_pop_current (dst);
}
slim_hidden_def(glitz_color_triangles);
