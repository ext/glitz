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

#include <math.h>

#define SURFACE_GLREPEAT(surface, texture) \
  (SURFACE_REPEAT (surface) && texture->repeatable)

#define SURFACE_MANUALREPEAT(surface, texture) \
  ((surface->hint_mask & GLITZ_INT_HINT_REPEAT_MASK) && (!texture->repeatable))

#define SURFACE_ROTATE(surface) \
  (surface->transform && \
   ((surface->transform->m[0][1] != 0.0) || \
   (surface->transform->m[1][0] != 0.0)))

/* This version of composite uses multi-texturing for direct
   Porter-Duff compositing. It cannot handle rotating transformations
   and will fall back to regular composite function if this is the case. */
static glitz_bool_t
_glitz_composite_direct (glitz_operator_t op,
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
                         int height)
{
  glitz_gl_proc_address_list_t *gl;
  glitz_texture_t *src_texture;
  glitz_texture_t *mask_texture;
  glitz_bounding_box_double_t src_box, mask_box, dst_box;
  glitz_bounding_box_t dirty_box;
  glitz_point_t src_tl, src_br, mask_tl, mask_br;
  glitz_program_type_t type;

  gl = dst->gl;

  type = glitz_program_type (dst, src, mask);
  
  if (type == GLITZ_PROGRAM_TYPE_NOT_SUPPORTED)
    return 0;
  
  /* We cannot continue if we have a rotating transformation. */
  if (SURFACE_ROTATE (src) || SURFACE_ROTATE (mask))
    return 0;

  src_texture = glitz_surface_get_texture (src);
  mask_texture = glitz_surface_get_texture (mask);

  /* Texture has not been allocated, hence source and the result of this
     operation is undefined. So lets do nothing. */
  if ((!src_texture) || (!mask_texture))
    return 1;

  if (SURFACE_MANUALREPEAT (src, src_texture) ||
      SURFACE_MANUALREPEAT (mask, mask_texture))
    return 0;

  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_pop_current (dst);
    return 0;
  }

  gl->disable (GLITZ_GL_SCISSOR_TEST);

  glitz_surface_enable_program (type, dst, src, mask,
                                src_texture, mask_texture);

  glitz_set_operator (gl, op);
  
  gl->active_texture_arb (GLITZ_GL_TEXTURE0_ARB);
  glitz_texture_bind (gl, src_texture);
  
  gl->tex_env_f (GLITZ_GL_TEXTURE_ENV,
                 GLITZ_GL_TEXTURE_ENV_MODE,
                 GLITZ_GL_REPLACE);

  if (src->transform)
    glitz_texture_ensure_filter (gl, src_texture, src->filter);
  else
    glitz_texture_ensure_filter (gl, src_texture, GLITZ_FILTER_NEAREST);
    
  glitz_texture_ensure_repeat (gl,
                               src_texture,
                               src->hint_mask & GLITZ_INT_HINT_REPEAT_MASK);

  dst->gl->active_texture_arb (GLITZ_GL_TEXTURE1_ARB);
  glitz_texture_bind (gl, mask_texture);

  gl->tex_env_f (GLITZ_GL_TEXTURE_ENV,
                 GLITZ_GL_TEXTURE_ENV_MODE,
                 GLITZ_GL_MODULATE);
  
  if (mask->transform)
    glitz_texture_ensure_filter (gl, mask_texture, mask->filter);
  else
    glitz_texture_ensure_filter (gl, mask_texture, GLITZ_FILTER_NEAREST);

  dst_box.x1 = x_dst;
  dst_box.y1 = y_dst;
  dst_box.x2 = dst_box.x1 + width;
  dst_box.y2 = dst_box.y1 + height;
  
  if (!SURFACE_REPEAT (src)) {
    src_box.x1 = src_box.y1 = 0.0;
    src_box.x2 = src->width;
    src_box.y2 = src->height;
  
    if (src->transform)
      glitz_matrix_transform_bounding_box (src->transform,
                                           &src_box.x1, &src_box.y1,
                                           &src_box.x2, &src_box.y2);
    
    src_box.x1 += x_dst - x_src;
    src_box.x2 += x_dst - x_src;
    src_box.y2 += y_dst - y_src;
    src_box.y2 += y_dst - y_src;

    if (!SURFACE_PROGRAMMATIC (src))
      glitz_intersect_bounding_box_double (&dst_box, &src_box, &dst_box);
  }

  if (!SURFACE_REPEAT (mask)) {
    mask_box.x1 = mask_box.y1 = 0.0;
    mask_box.x2 = mask->width;
    mask_box.y2 = mask->height;
    
    if (mask->transform)
      glitz_matrix_transform_bounding_box (mask->transform,
                                           &mask_box.x1, &mask_box.y1,
                                           &mask_box.x2, &mask_box.y2);
  
    mask_box.x1 += x_dst - x_mask;
    mask_box.x2 += x_dst - x_mask;
    mask_box.y2 += y_dst - y_mask;
    mask_box.y2 += y_dst - y_mask;

    if (!SURFACE_PROGRAMMATIC (mask))
      glitz_intersect_bounding_box_double (&dst_box, &mask_box, &dst_box);
  }

  if ((dst_box.x2 - dst_box.x1) <= 0 || (dst_box.y2 - dst_box.y1) <= 0)
    return 1;

  if (SURFACE_REPEAT (src)) {
    src_br.y = src->height -
      (((y_src % src->height) + (int) (dst_box.y2 - dst_box.y1)) %
       src->height);
    src_tl.y = (dst_box.y2 - dst_box.y1) + src_br.y;
    src_tl.x = x_src % src->width;
    src_br.x = (dst_box.x2 - dst_box.x1) + src_tl.x;
  } else {
    glitz_intersect_bounding_box_double (&src_box, &dst_box, &src_box);

    src_tl.x = src_box.x1 - x_dst + x_src;
    src_br.x = src_box.x2 - x_dst + x_src;
    src_tl.y = src_box.y1 - y_dst + y_src;
    src_br.y = src_box.y2 - y_dst + y_src;

    if (src->transform) {
      glitz_matrix_transform_point (src->inverse_transform,
                                    &src_tl.x, &src_tl.y);
      glitz_matrix_transform_point (src->inverse_transform,
                                    &src_br.x, &src_br.y);
    }
  }

  src_tl.x = (src_tl.x / src->width) * src_texture->texcoord_width;
  src_tl.y = (src_tl.y / src->height) * src_texture->texcoord_height;
  src_br.x = (src_br.x / src->width) * src_texture->texcoord_width;
  src_br.y = (src_br.y / src->height) * src_texture->texcoord_height;

  if (!SURFACE_REPEAT (src)) {
    src_tl.y = src_texture->texcoord_height - src_tl.y;
    src_br.y = src_texture->texcoord_height - src_br.y;
  }

  if (SURFACE_REPEAT (mask)) {
    mask_br.y = mask->height -
      (((y_mask % mask->height) + (int) (dst_box.y2 - dst_box.y1)) %
       mask->height);
    mask_tl.y = (dst_box.y2 - dst_box.y1) + mask_br.y;
    mask_tl.x = x_mask % mask->width;
    mask_br.x = (dst_box.x2 - dst_box.x1) + mask_tl.x;    
  } else {
    glitz_intersect_bounding_box_double (&mask_box, &dst_box, &mask_box);

    mask_tl.x = mask_box.x1 - x_dst + x_mask;
    mask_br.x = mask_box.x2 - x_dst + x_mask;
    mask_tl.y = mask_box.y1 - y_dst + y_mask;
    mask_br.y = mask_box.y2 - y_dst + y_mask;

    if (mask->transform) {
      glitz_matrix_transform_point (mask->inverse_transform,
                                    &mask_tl.x, &mask_tl.y);
      glitz_matrix_transform_point (mask->inverse_transform,
                                    &mask_br.x, &mask_br.y);
    }
  }

  mask_tl.x = (mask_tl.x / mask->width) * mask_texture->texcoord_width;
  mask_tl.y = (mask_tl.y / mask->height) * mask_texture->texcoord_height;
  mask_br.x = (mask_br.x / mask->width) * mask_texture->texcoord_width;
  mask_br.y = (mask_br.y / mask->height) * mask_texture->texcoord_height;

  if (!SURFACE_REPEAT (mask)) {
    mask_tl.y = mask_texture->texcoord_height - mask_tl.y;
    mask_br.y = mask_texture->texcoord_height - mask_br.y;
  }

  gl->begin (GLITZ_GL_QUADS);

  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE0_ARB, src_tl.x, src_tl.y);
  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE1_ARB, mask_tl.x, mask_tl.y);
  gl->vertex_2d (dst_box.x1, dst_box.y1);

  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE0_ARB, src_br.x, src_tl.y);
  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE1_ARB, mask_br.x, mask_tl.y);
  gl->vertex_2d (dst_box.x2, dst_box.y1);

  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE0_ARB, src_br.x, src_br.y);
  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE1_ARB, mask_br.x, mask_br.y);
  gl->vertex_2d (dst_box.x2, dst_box.y2);

  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE0_ARB, src_tl.x, src_br.y);
  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE1_ARB, mask_tl.x, mask_br.y);
  gl->vertex_2d (dst_box.x1, dst_box.y2);

  gl->end ();

  gl->active_texture_arb (GLITZ_GL_TEXTURE1_ARB);
  glitz_texture_unbind (gl, mask_texture);
  
  gl->active_texture_arb (GLITZ_GL_TEXTURE0_ARB);
  glitz_texture_unbind (gl, src_texture);

  glitz_surface_disable_program (type, dst);

  dirty_box.x1 = floor (dst_box.x1);
  dirty_box.y1 = floor (dst_box.y1);
  dirty_box.x2 = ceil (dst_box.x2);
  dirty_box.y2 = ceil (dst_box.y2);
  glitz_surface_dirty (dst, &dirty_box);
    
  glitz_surface_pop_current (dst);

  return 1;
}

static void
glitz_mask_bounds (glitz_surface_t *src,
                   glitz_surface_t *mask,
                   glitz_surface_t *dst,
                   int x_src,
                   int y_src,
                   int x_mask,
                   int y_mask,
                   int x_dst,
                   int y_dst,
                   glitz_bounding_box_t *bounds,
                   glitz_bounding_box_t *mbounds)
{
  double x1, y1, x2, y2;
  int ix1, iy1, ix2, iy2;
  
  if (bounds->x1 < 0)
    mbounds->x1 = 0;
  else
    mbounds->x1 = bounds->x1;

  if (bounds->y1 < 0)
    mbounds->y1 = 0;
  else
    mbounds->y1 = bounds->y1;

  if (bounds->x2 > dst->width)
    mbounds->x2 = dst->width;
  else
    mbounds->x2 = bounds->x2;

  if (bounds->y2 > dst->height)
    mbounds->y2 = dst->height;
  else
    mbounds->y2 = bounds->y2;

  if (!SURFACE_REPEAT (src)) {
    x1 = y1 = 0;
    x2 = src->width;
    y2 = src->height;

    if (src->transform)
      glitz_matrix_transform_bounding_box (src->transform,
                                           &x1, &y1, &x2, &y2);
    
    ix1 = (int) x1 + x_dst - x_src;
    iy1 = (int) y1 + y_dst - y_src;
    ix2 = (int) x2 + x_dst - x_src;
    iy2 = (int) y2 + y_dst - y_src;
    
    if (mbounds->x1 < ix1)
      mbounds->x1 = ix1;
    
    if (mbounds->y1 < iy1)
      mbounds->y1 = iy1;
    
    if (mbounds->x2 > ix2)
      mbounds->x2 = ix2;
    
    if (mbounds->y2 > iy2)
      mbounds->y2 = iy2;
  }

  if (!SURFACE_REPEAT (mask)) {
    x1 = y1 = 0;
    x2 = mask->width;
    y2 = mask->height;

    if (mask->transform)
      glitz_matrix_transform_bounding_box (mask->transform,
                                           &x1, &y1, &x2, &y2);
    
    ix1 = (int) x1 + x_dst - x_mask;
    iy1 = (int) y1 + y_dst - y_mask;
    ix2 = (int) x2 + x_dst - x_mask;
    iy2 = (int) y2 + y_dst - y_mask;
    
    if (mbounds->x1 < ix1)
      mbounds->x1 = ix1;
    
    if (mbounds->y1 < iy1)
      mbounds->y1 = iy1;
    
    if (mbounds->x2 > ix2)
      mbounds->x2 = ix2;
    
    if (mbounds->y2 > iy2)
      mbounds->y2 = iy2;
  }
}

typedef enum {
  GLITZ_REPEAT_NONE = 0,
  GLITZ_REPEAT_SOUTHEAST,
  GLITZ_REPEAT_SOUTHWEST,
  GLITZ_REPEAT_NORTHEAST,
  GLITZ_REPEAT_NORTHWEST
} glitz_repeat_direction_t;

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
                 int height)
{
  glitz_gl_proc_address_list_t *gl;
  glitz_surface_t *intermediate = NULL, *mask_surface;
  glitz_texture_t *texture;
  glitz_point_t tl, bl, br, tr;
  glitz_bounding_box_t clip;
  glitz_program_type_t type = 0;
  glitz_bool_t simple_modulate = 0;

  gl = dst->gl;
  
  if (SURFACE_PROGRAMMATIC (src))
    glitz_programmatic_surface_setup (src,
                                      width + abs (x_src),
                                      height + abs (y_src));

  if (mask) {
    if (SURFACE_PROGRAMMATIC (mask))
      glitz_programmatic_surface_setup (mask,
                                        width + abs (x_mask),
                                        height + abs (y_mask));

    if (SURFACE_SOLID (mask) && (!SURFACE_PROGRAMMATIC (src))) {
      simple_modulate = 1;
      if ((dst->feature_mask & GLITZ_FEATURE_CONVOLUTION_FILTER_MASK) &&
          src->convolution)
        simple_modulate = 0;
    }
  }

  if (mask && (!simple_modulate)) {
    glitz_bounding_box_t mask_bounds;
    static glitz_color_t clear_color = { 0x0000, 0x0000, 0x0000, 0x0000 };
    glitz_bool_t intermediate_translate;

    if ((dst->feature_mask & GLITZ_FEATURE_ARB_MULTITEXTURE_MASK) &&
        _glitz_composite_direct (op,
                                 src, mask, dst,
                                 x_src, y_src,
                                 x_mask, y_mask,
                                 x_dst, y_dst,
                                 width, height))
      return;

    mask_bounds.x1 = x_dst;
    mask_bounds.y1 = y_dst;
    intermediate_translate = 0;
    
    if (!SURFACE_IMPLICIT_MASK (mask)) {
      glitz_bounding_box_t bounds;
      
      bounds.x1 = x_dst;
      bounds.x2 = x_dst + width;
      bounds.y1 = y_dst;
      bounds.y2 = y_dst + height;
      
      glitz_mask_bounds (src, mask, dst,
                         x_src, y_src, x_mask, y_mask, x_dst, y_dst,
                         &bounds, &mask_bounds);

      if (mask_bounds.x2 - mask_bounds.x1 <= 0 ||
          mask_bounds.y2 - mask_bounds.y1 <= 0)
        return;

      if ((x_dst != mask_bounds.x1) || (y_dst != mask_bounds.y1))
        intermediate_translate = 1;
      
      mask_surface = intermediate =
        glitz_int_surface_create_similar (dst,
                                          GLITZ_STANDARD_ARGB32,
                                          1,
                                          mask_bounds.x2 - mask_bounds.x1,
                                          mask_bounds.y2 - mask_bounds.y1);
      
      if (!mask_surface) {
        glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
        return;
      }

      if (mask->transform)
        glitz_fill_rectangle (GLITZ_OPERATOR_SRC,
                              mask_surface,
                              &clear_color,
                              0, 0,
                              mask_surface->width,
                              mask_surface->height);

      if ((!SURFACE_REPEAT (mask)) && intermediate_translate) {
        glitz_surface_push_transform (mask);
        glitz_matrix_translate (mask->transform,
                                x_dst - mask_bounds.x1,
                                y_dst - mask_bounds.y1);
      }
      
      glitz_composite (GLITZ_OPERATOR_SRC,
                       mask, NULL, mask_surface,
                       x_mask, y_mask,
                       0, 0,
                       0, 0,
                       mask_surface->width - (x_dst - mask_bounds.x1),
                       mask_surface->height - (y_dst - mask_bounds.y1));

      if (intermediate_translate)
        glitz_surface_pop_transform (mask);
      
    } else
      mask_surface = mask;

    if ((!SURFACE_REPEAT (src)) && intermediate_translate) {
      glitz_surface_push_transform (src);
      glitz_matrix_translate (src->transform,
                              x_dst - mask_bounds.x1,
                              y_dst - mask_bounds.y1);
    }

    if (src->transform)
      mask_surface->hint_mask |= GLITZ_INT_HINT_CLEAR_EXTERIOR_MASK;
    
    glitz_composite (GLITZ_OPERATOR_IN,
                     src, NULL, mask_surface,
                     x_src, y_src,
                     0, 0,
                     0, 0,
                     mask_surface->width,
                     mask_surface->height);

    if (intermediate_translate)
      glitz_surface_pop_transform (src);

    x_dst = mask_bounds.x1;
    y_dst = mask_bounds.y1;
    width = mask_surface->width;
    height = mask_surface->height;
    src = mask_surface;
    x_src = y_src = 0;
  }

  texture = glitz_surface_get_texture (src);
  
  /* Texture has not been allocated, hence source and the result of this
     operation is undefined. So lets do nothing. */
  if (!texture)
    return;

  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_pop_current (dst);
    return;
  }

  glitz_texture_bind (gl, texture);
  
  gl->tex_env_f (GLITZ_GL_TEXTURE_ENV,
                 GLITZ_GL_TEXTURE_ENV_MODE,
                 (simple_modulate)? GLITZ_GL_MODULATE: GLITZ_GL_REPLACE);
  
  if (simple_modulate) {
    glitz_programmatic_surface_t *m = (glitz_programmatic_surface_t *) mask;

    gl->tex_env_f (GLITZ_GL_TEXTURE_ENV,
                   GLITZ_GL_TEXTURE_ENV_MODE,
                   GLITZ_GL_MODULATE);
    gl->color_4us (m->u.solid.color.alpha,
                   m->u.solid.color.alpha,
                   m->u.solid.color.alpha,
                   m->u.solid.color.alpha);
  } else {
    gl->tex_env_f (GLITZ_GL_TEXTURE_ENV,
                   GLITZ_GL_TEXTURE_ENV_MODE,
                   GLITZ_GL_REPLACE);
  }

  clip.x1 = x_dst;
  clip.y1 = y_dst;
  clip.x2 = clip.x1 + width;
  clip.y2 = clip.y1 + height;
  
  gl->scissor (clip.x1, dst->height - (clip.y1 + height), width, height);
  
  glitz_set_operator (gl, op);

  if (src->convolution || SURFACE_PROGRAMMATIC (src)) {
    type = glitz_program_type (dst, src, NULL);
    glitz_surface_enable_program (type, dst, src, NULL, texture, NULL);
  }
  
  if ((!src->transform) && SURFACE_GLREPEAT (src, texture)) {
    /* CASE 1: Repeat, no transformation and power of two sized texture,
       GL can do repeat for us. */
    double repeat_factor_x, repeat_factor_y;

    glitz_texture_ensure_repeat (gl, texture, 1);
    glitz_texture_ensure_filter (gl, texture, GLITZ_FILTER_NEAREST);
    
    bl.x = tl.x = x_dst;
    tr.y = tl.y = y_dst;
    tr.x = br.x = x_dst + width;
    bl.y = br.y = y_dst + height;
    
    /* Shift coordinates with source offset */
    if (x_src) {
      x_src = (x_src % src->width);
      tl.x -= x_src;
      bl.x -= x_src;
    }
    if (y_src) {
      y_src = (y_src % src->height);
      tl.y -= y_src;
      tr.y -= y_src;
    }

    /* Align with top left edge */
    bl.y += texture->height - (((int) (br.y - tl.y)) % texture->height);
    br.y += texture->height - (((int) (br.y - tl.y)) % texture->height);
      
    repeat_factor_x = (br.x - tl.x) / (double) texture->width;
    repeat_factor_y = (br.y - tl.y) / (double) texture->height;
      
    gl->begin (GLITZ_GL_QUADS);
    gl->tex_coord_2d (0.0, repeat_factor_y);
    gl->vertex_2d (tl.x, tl.y);
    gl->tex_coord_2d (repeat_factor_x, repeat_factor_y);
    gl->vertex_2d (tr.x, tr.y);
    gl->tex_coord_2d (repeat_factor_x, 0.0);
    gl->vertex_2d (br.x, br.y);
    gl->tex_coord_2d (0.0, 0.0);
    gl->vertex_2d (bl.x, bl.y);
    gl->end ();
    
  } else {
    /* CASE 2: Either none power of two sized texture or
       transformation is set. */
    glitz_point_t base_tl, base_bl, base_br, base_tr;
    int x_dir, y_dir, continue_y, continue_x, x_is_ok, y_is_ok,
      in_destination_area;
    double save_base_x1, save_base_x2;
    glitz_repeat_direction_t repeat_direction;

    glitz_texture_ensure_repeat (gl, texture, 0);

    base_bl.x = base_tl.x = 0;
    base_tr.y = base_tl.y = 0;
    base_tr.x = base_br.x = src->width;
    base_bl.y = base_br.y = src->height;

    /* Start repeating in southeast direction */
    repeat_direction = GLITZ_REPEAT_SOUTHEAST;
    x_dir = y_dir = 1;

    if (src->transform)
      glitz_texture_ensure_filter (gl, texture, src->filter);
    else
      glitz_texture_ensure_filter (gl, texture, GLITZ_FILTER_NEAREST);

    while (repeat_direction) {
      save_base_x1 = base_tl.x;
      save_base_x2 = base_tr.x;
      
      do {
        continue_y = continue_x = in_destination_area = 0;
        do {
          bl = base_bl;
          br = base_br;
          tl = base_tl;
          tr = base_tr;
        
          if (src->transform) {
            glitz_matrix_transform_point (src->transform, &tl.x, &tl.y);
            glitz_matrix_transform_point (src->transform, &bl.x, &bl.y);
            glitz_matrix_transform_point (src->transform, &tr.x, &tr.y);
            glitz_matrix_transform_point (src->transform, &br.x, &br.y);
          }
    
          tl.x += x_dst - x_src;
          bl.x += x_dst - x_src;
          tr.x += x_dst - x_src;
          br.x += x_dst - x_src;
          
          tl.y += y_dst - y_src;
          bl.y += y_dst - y_src;
          tr.y += y_dst - y_src;
          br.y += y_dst - y_src;
          
          if ((tl.x > (x_dst + width) && bl.x > (x_dst + width)) ||
              (tr.x < x_dst && br.x < x_dst))
            x_is_ok = 0;
          else
            x_is_ok = 1;

          if ((tl.y > (y_dst + height) && tr.y > (y_dst + height)) ||
              (bl.y < y_dst && br.y < y_dst))
            y_is_ok = 0;
          else
            y_is_ok = 1;
        
          if (x_is_ok && y_is_ok) {
            gl->begin (GLITZ_GL_QUADS);
            gl->tex_coord_2d (0.0, texture->texcoord_height);
            gl->vertex_2d (tl.x, tl.y);
            gl->tex_coord_2d (texture->texcoord_width,
                              texture->texcoord_height);
            gl->vertex_2d (tr.x, tr.y);
            gl->tex_coord_2d (texture->texcoord_width, 0.0);
            gl->vertex_2d (br.x, br.y);
            gl->tex_coord_2d (0.0, 0.0);
            gl->vertex_2d (bl.x, bl.y);
            gl->end ();
            in_destination_area = 1;
          }

          if (SURFACE_REPEAT (src)) {
            base_bl.x += src->width * x_dir;
            base_tl.x += src->width * x_dir;
            base_tr.x += src->width * x_dir;
            base_br.x += src->width * x_dir;

            if (y_is_ok)
              continue_y = 1;

            if (in_destination_area)
              continue_x = (x_is_ok && y_is_ok);
            else
              continue_x = (x_is_ok || y_is_ok);
          }
        } while (SURFACE_REPEAT (src) && continue_x);

        if (SURFACE_REPEAT (src)) {
          base_bl.y += src->height * y_dir;
          base_tl.y += src->height * y_dir;
          base_tr.y += src->height * y_dir;
          base_br.y += src->height * y_dir;

          base_tl.x = base_bl.x = save_base_x1;
          base_tr.x = base_br.x = save_base_x2;
        }
      } while (SURFACE_REPEAT (src) && continue_y);
    
      if (src->transform && SURFACE_REPEAT (src)) {
        switch (repeat_direction) {
        case GLITZ_REPEAT_SOUTHEAST:
          y_dir = -1;
          base_tl.y = base_tr.y = -src->height;
          base_bl.y = base_br.y = 0.0;
          repeat_direction = GLITZ_REPEAT_SOUTHWEST;
          break;
        case GLITZ_REPEAT_SOUTHWEST:
          x_dir = -1;
          base_tl.y = base_tr.y = -src->height;
          base_bl.y = base_br.y = 0.0;
          base_tl.x = base_bl.x = -src->width;
          base_tr.x = base_br.x = 0.0;
          repeat_direction = GLITZ_REPEAT_NORTHEAST;
          break;
        case GLITZ_REPEAT_NORTHEAST:
          y_dir = 1;
          base_tl.y = base_tr.y = 0.0;
          base_bl.y = base_br.y = src->height;
          base_tl.x = base_bl.x = -src->width;
          base_tr.x = base_br.x = 0.0;
          repeat_direction = GLITZ_REPEAT_NORTHWEST;
          break;
        case GLITZ_REPEAT_NORTHWEST:
          repeat_direction = GLITZ_REPEAT_NONE;
          break;
        case GLITZ_REPEAT_NONE:
          break;
        }
      } else
        repeat_direction = GLITZ_REPEAT_NONE;
    }
  }

  if (src->convolution || SURFACE_PROGRAMMATIC (src))
    glitz_surface_disable_program (type, dst);
  
  glitz_texture_unbind (gl, texture);

  /* Clear intermediate mask exterior. This is only done if
     destination surfaces is an intermediate mask surface
     and source surface is transformed and not repeating. */
  if (SURFACE_CLEAR_EXTERIOR (dst) && (!SURFACE_REPEAT (src))) {
    glitz_set_operator (gl, GLITZ_OPERATOR_SRC);
    
    gl->color_4us (0x0000, 0x0000, 0x0000, 0x0000);
    
    gl->begin (GLITZ_GL_QUADS);

    gl->vertex_2d (tl.x, tl.y);
    gl->vertex_2d (0.0, tl.y);
    gl->vertex_2d (0.0, 0.0);
    gl->vertex_2d (tl.x, 0.0);   

    gl->vertex_2d (tl.x, tl.y);
    gl->vertex_2d (tl.x, 0.0);
    gl->vertex_2d (tr.x, 0.0);
    gl->vertex_2d (tr.x, tr.y);
    
    gl->vertex_2d (tr.x, tr.y);
    gl->vertex_2d (tr.x, 0.0);
    gl->vertex_2d (dst->width, 0.0);
    gl->vertex_2d (dst->width, tr.y);

    gl->vertex_2d (tr.x, tr.y);
    gl->vertex_2d (dst->width, tr.y);
    gl->vertex_2d (dst->width, br.y);
    gl->vertex_2d (br.x, br.y);
    
    gl->vertex_2d (br.x, br.y);
    gl->vertex_2d (dst->width, br.y);
    gl->vertex_2d (dst->width, dst->height);
    gl->vertex_2d (br.x, dst->height);

    gl->vertex_2d (br.x, br.y);
    gl->vertex_2d (br.x, dst->height);
    gl->vertex_2d (bl.x, dst->height);
    gl->vertex_2d (bl.x, bl.y);

    gl->vertex_2d (bl.x, bl.y);
    gl->vertex_2d (bl.x, dst->height);
    gl->vertex_2d (0.0, dst->height);
    gl->vertex_2d (0.0, bl.y);

    gl->vertex_2d (bl.x, bl.y);
    gl->vertex_2d (0.0, bl.y);
    gl->vertex_2d (0.0, tl.y);
    gl->vertex_2d (tl.x, tl.y);

    gl->end ();
  }

  glitz_surface_dirty (dst, &clip);

  glitz_surface_pop_current (dst);

  if (intermediate)
    glitz_surface_destroy (intermediate);
}

void
glitz_copy_area (glitz_surface_t *src,
                 glitz_surface_t *dst,
                 int x_src,
                 int y_src,
                 int width,
                 int height,
                 int x_dst,
                 int y_dst)
{
  glitz_gl_proc_address_list_t *gl;
  glitz_bounding_box_t box, src_box, dst_box;
  int status;
  
  if (SURFACE_PROGRAMMATIC (dst) || SURFACE_PROGRAMMATIC (src))
    return;

  gl = dst->gl;

  box.x1 = x_src;
  box.y1 = y_src;
  box.x2 = box.x1 + width;
  box.y2 = box.y1 + height;

  src_box.x1 = src_box.y1 = 0;
  src_box.x2 = src->width;
  src_box.y2 = src->height;

  glitz_intersect_bounding_box (&box, &src_box, &src_box);

  box.x1 = x_dst;
  box.y1 = y_dst;
  box.x2 = box.x1 + (src_box.x2 - src_box.x1);
  box.y2 = box.y1 + (src_box.y2 - src_box.y1);

  dst_box.x1 = dst_box.y1 = 0;
  dst_box.x2 = dst->width;
  dst_box.y2 = dst->height;

  glitz_intersect_bounding_box (&box, &dst_box, &dst_box);

  x_src = src_box.x1;
  y_src = src_box.y1;
  width = dst_box.x2 - dst_box.x1;
  height = dst_box.y2 - dst_box.y1;
  x_dst = dst_box.x1;
  y_dst = dst_box.y1;

  if (width <= 0 || height <= 0) {
    glitz_surface_status_add (dst, GLITZ_STATUS_BAD_COORDINATE_MASK);
    return;
  }

  status = 0;
  if (glitz_surface_try_push_current (dst,
                                      GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    if (src != dst)
      status = glitz_surface_make_current_read (src);
    else
      status = 1;

    if (status) {
      if (src->format->doublebuffer)
        gl->read_buffer (src->read_buffer);

      gl->disable (GLITZ_GL_SCISSOR_TEST);
      gl->disable (GLITZ_GL_DITHER);    
      glitz_set_operator (gl, GLITZ_OPERATOR_SRC);
      
      gl->pixel_zoom (1.0, 1.0);
      glitz_set_raster_pos (gl, x_dst, dst->height - (y_dst + height));
      gl->copy_pixels (x_src, src->height - (y_src + height),
                       width, height, GLITZ_GL_COLOR);
    } else {
      glitz_texture_t *texture = glitz_surface_get_texture (src);
      if (texture) {
        glitz_point_t tl, br;
          
        gl->disable (GLITZ_GL_SCISSOR_TEST);
        gl->disable (GLITZ_GL_DITHER);
        
        glitz_texture_bind (gl, texture);
        
        gl->tex_env_f (GLITZ_GL_TEXTURE_ENV,
                       GLITZ_GL_TEXTURE_ENV_MODE,
                       GLITZ_GL_REPLACE);

        glitz_set_operator (gl, GLITZ_OPERATOR_SRC);
        
        glitz_texture_ensure_repeat (gl, texture, 0);
        glitz_texture_ensure_filter (gl, texture, GLITZ_FILTER_NEAREST);
        
        tl.x = (x_src / (double) src->width) * texture->texcoord_width;
        tl.y = (y_src / (double) src->height) * texture->texcoord_height;

        br.x = ((x_src + width) / (double) src->width) *
          texture->texcoord_width;
        br.y = ((y_src + height) / (double) src->height) *
          texture->texcoord_height;

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

        glitz_texture_unbind (gl, texture);
      }
    }
    status = 1;
    glitz_surface_dirty (dst, &dst_box);
    glitz_surface_pop_current (dst);
  }

  if (!status) {
    if (glitz_surface_try_push_current (src,
                                        GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
      glitz_texture_copy_surface (&dst->texture, src,
                                  &dst_box, x_src, y_src);
      status = 1;
    }
    glitz_surface_pop_current (src);
  }

  if (!status) {
    int rowstride, bytes_per_pixel;
    char *pixel_buf;
    
    bytes_per_pixel = MAX (dst->format->bpp, src->format->bpp) / 8;
    
    rowstride = width * bytes_per_pixel;
    rowstride = (rowstride + 3) & -4;
    pixel_buf = malloc (height * rowstride);
    if (!pixel_buf) {
      glitz_surface_status_add (dst, GLITZ_STATUS_NO_MEMORY_MASK);
      return;
    }
    glitz_surface_read_pixels (src, x_src, y_src, width, height, pixel_buf);
    glitz_surface_draw_pixels (dst, x_dst, y_dst, width, height, pixel_buf);
    free (pixel_buf);
  }
}
