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

#define SURFACE_INFINITE_DATA(surface) \
  (SURFACE_REPEAT (surface) || SURFACE_PROGRAMMATIC (surface))

#define SURFACE_GLREPEAT(surface, texture) \
  (SURFACE_REPEAT (surface) && texture->repeatable)

#define SURFACE_MANUALREPEAT(surface, texture) \
  ((surface->hint_mask & GLITZ_INT_HINT_REPEAT_MASK) && (!texture->repeatable))

#define SURFACE_ROTATE(surface) \
  (surface->transform && \
   ((surface->transform->m[0][1] != 0.0) || \
   (surface->transform->m[1][0] != 0.0)))

/* This version of composite uses fragment programs for direct
   Porter-Duff compositing. It cannot handle rotating transformations
   and only one of source and mask can be a transformed the other
   surface must be repeating. Function will fall back to regular composite
   function if this isn't the case.
*/
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
  glitz_sub_pixel_region_box_t src_region, mask_region, dst_region;
  glitz_region_box_t dirty_region;
  glitz_point_t src_tl, src_br, mask_tl, mask_br, translate_src,
    translate_mask;
  double src_width, src_height, mask_width, mask_height;
  glitz_program_type_t type;

  gl = dst->gl;

  type = glitz_program_type (dst, src, mask);
  
  if (type == GLITZ_PROGRAM_TYPE_NOT_SUPPORTED)
    return 0;
  
  /* We cannot continue if we have a rotating transformation or
     if both surfaces have transformations or the surface not being
     transformed isn't repeating. */
  if (src->transform || mask->transform) {
    if (SURFACE_ROTATE (src) || SURFACE_ROTATE (mask))
      return 0;
    
    if ((src->transform &&
         (mask->transform || (!SURFACE_INFINITE_DATA (mask)))) ||
        (mask->transform &&
         (src->transform || (!SURFACE_INFINITE_DATA (src)))))
      return 0;
  }

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

  
  /* calculate source area */
  src_region.x1 = src_region.y1 = 0.0;
  src_region.x2 = src->width;
  src_region.y2 = src->height;
  
  if (src->transform)
    glitz_matrix_transform_sub_pixel_region (src->transform, &src_region);

  src_width = src_region.x2 - src_region.x1;
  src_height = src_region.y2 - src_region.y1;

  translate_src.x = (src_region.x1 < 0.0)? src_region.x1: 0.0;
  translate_src.y = (src_region.y1 < 0.0)? src_region.y1: 0.0;

  src_region.x1 += x_dst;
  src_region.y1 += y_dst;
  src_region.x2 += (x_dst - x_src);
  src_region.y2 += (y_dst - y_src);
  

  /* calculate mask area */
  mask_region.x1 = mask_region.y1 = 0.0;
  mask_region.x2 = mask->width;
  mask_region.y2 = mask->height;

  if (mask->transform)
    glitz_matrix_transform_sub_pixel_region (mask->transform, &mask_region);

  mask_width = mask_region.x2 - mask_region.x1;
  mask_height = mask_region.y2 - mask_region.y1;

  translate_mask.x = (mask_region.x1 < 0.0)? mask_region.x1: 0.0;
  translate_mask.y = (mask_region.y1 < 0.0)? mask_region.y1: 0.0;
  mask_region.x1 += x_dst;
  mask_region.y1 += y_dst;
  mask_region.x2 += (x_dst - x_mask);
  mask_region.y2 += (y_dst - y_mask);


  /* calculate destination area */
  dst_region.x1 = x_dst;
  dst_region.y1 = y_dst;
  dst_region.x2 = dst_region.x1 + width;
  dst_region.y2 = dst_region.y1 + height;
  
  if (!SURFACE_REPEAT (src))
    glitz_intersect_sub_pixel_region (&dst_region, &src_region, &dst_region);

  if (!SURFACE_REPEAT (mask))
    glitz_intersect_sub_pixel_region (&dst_region, &mask_region, &dst_region);
  

  /* re-calculate source area */
  if (SURFACE_REPEAT (src)) {
    src_region.y2 = src->height -
      (((y_src % src->height) + (int) (dst_region.y2 - dst_region.y1)) %
       src->height);
    src_region.y1 = (dst_region.y2 - dst_region.y1) + src_region.y2;    
    src_region.x1 = x_src % src->width;
    src_region.x2 = (dst_region.x2 - dst_region.x1) + src_region.x1;
  } else {
    glitz_intersect_sub_pixel_region (&src_region, &dst_region, &src_region);

    if (x_src < 0) x_src = 0;
    if (y_src < 0) y_src = 0;

    src_region.x2 = x_src + (src_region.x2 - src_region.x1) - translate_src.x;
    src_region.y2 = y_src + (src_region.y2 - src_region.y1) - translate_src.y;
    src_region.x1 = x_src - translate_src.x;
    src_region.y1 = y_src - translate_src.y;
  }

  /* re-calculate mask area */
  if (SURFACE_REPEAT (mask)) {
    mask_region.y2 = mask->height -
      (((y_mask % mask->height) + (int) (dst_region.y2 - dst_region.y1)) %
       mask->height);
    mask_region.y1 = (dst_region.y2 - dst_region.y1) + mask_region.y2;    
    mask_region.x1 = x_mask % mask->width;
    mask_region.x2 = (dst_region.x2 - dst_region.x1) + mask_region.x1;
  } else {
    glitz_intersect_sub_pixel_region (&mask_region, &dst_region, &mask_region);

    if (x_mask < 0) x_mask = 0;
    if (y_mask < 0) y_mask = 0;
    
    mask_region.x2 = x_mask + (mask_region.x2 - mask_region.x1) -
      translate_mask.x;
    mask_region.y2 = y_mask + (mask_region.y2 - mask_region.y1) -
      translate_mask.y;
    mask_region.x1 = x_mask - translate_mask.x;
    mask_region.y1 = y_mask - translate_mask.y;
  }

  /* normalize texture coordinates */
  src_tl.x = (src_region.x1 / src_width) * src_texture->texcoord_width;
  src_tl.y = (src_region.y1 / src_height) * src_texture->texcoord_height;

  src_br.x = (src_region.x2 / src_width) * src_texture->texcoord_width;
  src_br.y = (src_region.y2 / src_height) * src_texture->texcoord_height;

  mask_tl.x = (mask_region.x1 / mask_width) * mask_texture->texcoord_width;
  mask_tl.y = (mask_region.y1 / mask_height) * mask_texture->texcoord_height;

  mask_br.x = (mask_region.x2 / mask_width) * mask_texture->texcoord_width;
  mask_br.y = (mask_region.y2 / mask_height) * mask_texture->texcoord_height;

  if (!SURFACE_REPEAT(src)) {
    src_tl.y = src_texture->texcoord_height - src_tl.y;
    src_br.y = src_texture->texcoord_height - src_br.y;
  }
    
  if (!SURFACE_REPEAT(mask)) {
    mask_tl.y = mask_texture->texcoord_height - mask_tl.y;
    mask_br.y = mask_texture->texcoord_height - mask_br.y;
  }
  
  gl->begin (GLITZ_GL_QUADS);

  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE0_ARB, src_tl.x, src_tl.y);
  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE1_ARB, mask_tl.x, mask_tl.y);
  gl->vertex_2d (dst_region.x1, dst_region.y1);

  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE0_ARB, src_br.x, src_tl.y);
  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE1_ARB, mask_br.x, mask_tl.y);
  gl->vertex_2d (dst_region.x2, dst_region.y1);

  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE0_ARB, src_br.x, src_br.y);
  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE1_ARB, mask_br.x, mask_br.y);
  gl->vertex_2d (dst_region.x2, dst_region.y2);

  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE0_ARB, src_tl.x, src_br.y);
  gl->multi_tex_coord_2d_arb (GLITZ_GL_TEXTURE1_ARB, mask_tl.x, mask_br.y);
  gl->vertex_2d (dst_region.x1, dst_region.y2);

  gl->end ();

  gl->active_texture_arb (GLITZ_GL_TEXTURE1_ARB);
  glitz_texture_unbind (gl, mask_texture);
  
  gl->active_texture_arb (GLITZ_GL_TEXTURE0_ARB);
  glitz_texture_unbind (gl, src_texture);

  glitz_surface_disable_program (type, dst);

  dirty_region.x1 = floor (dst_region.x1);
  dirty_region.y1 = floor (dst_region.y1);
  dirty_region.x2 = ceil (dst_region.x2);
  dirty_region.y2 = ceil (dst_region.y2);
  glitz_surface_dirty (dst, &dirty_region);
    
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
                   glitz_region_box_t *bounds,
                   glitz_region_box_t *mbounds)
{
  glitz_region_box_t region;
  
  if (bounds->x1 <= 0)
    mbounds->x1 = 0;
  else
    mbounds->x1 = bounds->x1;

  if (bounds->y1 <= 0)
    mbounds->y1 = 0;
  else
    mbounds->y1 = bounds->y1;

  if (bounds->x2 >= dst->width)
    mbounds->x2 = dst->width;
  else
    mbounds->x2 = bounds->x2;

  if (bounds->y2 >= dst->height)
    mbounds->y2 = dst->height;
  else
    mbounds->y2 = bounds->y2;

  if (!SURFACE_REPEAT (src)) {
    region.x1 = x_dst;
    region.y1 = y_dst;
    if (x_src < 0) region.x1 -= x_src;
    if (y_src < 0) region.y1 -= y_src;
    region.x2 = region.x1 + src->width;
    region.y2 = region.y1 + src->height;
    if (x_src > 0) region.x2 -= x_src;
    if (y_src > 0) region.y2 -= y_src;

    if (src->transform)
      glitz_matrix_transform_region (src->transform, &region);
    
    if (mbounds->x1 < region.x1)
      mbounds->x1 = region.x1;
    
    if (mbounds->y1 < region.y1)
      mbounds->y1 = region.y1;
    
    if (mbounds->x2 > region.x2)
      mbounds->x2 = region.x2;
    
    if (mbounds->y2 > region.y2)
      mbounds->y2 = region.y2;
  }

  if (!SURFACE_REPEAT (mask)) {
    region.x1 = x_dst;
    region.y1 = y_dst;
    if (x_mask < 0) region.x1 -= x_mask;
    if (y_mask < 0) region.y1 -= y_mask;
    region.x2 = region.x1 + mask->width;
    region.y2 = region.y1 + mask->height;
    if (x_mask > 0) region.x2 -= x_mask;
    if (y_mask > 0) region.y2 -= y_mask;
    
    if (mask->transform)
      glitz_matrix_transform_region (mask->transform, &region);
    
    if (mbounds->x1 < region.x1)
      mbounds->x1 = region.x1;
    
    if (mbounds->y1 < region.y1)
      mbounds->y1 = region.y1;
    
    if (mbounds->x2 > region.x2)
      mbounds->x2 = region.x2;
    
    if (mbounds->y2 > region.y2)
      mbounds->y2 = region.y2;
  }
}

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
  glitz_region_box_t clip;
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
    glitz_region_box_t mask_bounds;
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
      glitz_region_box_t bounds;
      
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
    double save_tlx, save_trx, save_blx, save_brx;

    glitz_texture_ensure_repeat (gl, texture, 0);

    bl.x = tl.x = 0;
    tr.y = tl.y = 0;
    tr.x = br.x = src->width;
    bl.y = br.y = src->height;

    if (src->transform) {
      glitz_texture_ensure_filter (gl, texture, src->filter);
      glitz_matrix_transform_point (src->transform, &tl);
      glitz_matrix_transform_point (src->transform, &bl);
      glitz_matrix_transform_point (src->transform, &tr);
      glitz_matrix_transform_point (src->transform, &br);
    } else
      glitz_texture_ensure_filter (gl, texture, GLITZ_FILTER_NEAREST);
    
    /* Shift all coordinates with destination offset */
    if (x_dst) {
      tl.x += x_dst;
      bl.x += x_dst;
      tr.x += x_dst;
      br.x += x_dst;
    }
    if (y_dst) {
      tl.y += y_dst;
      bl.y += y_dst;
      tr.y += y_dst;
      br.y += y_dst;
    }

    /* Shift all coordinates with source offset */
    if (x_src) {
      x_src = abs (x_src);
      if (SURFACE_REPEAT (src))
        x_src = (x_src % src->width);
      tl.x -= x_src;
      bl.x -= x_src;
      tr.x -= x_src;
      br.x -= x_src;
    }
    if (y_src) {
      y_src = abs (y_src);
      if (SURFACE_REPEAT (src))
        y_src = (y_src % src->height);
      tl.y -= y_src;
      bl.y -= y_src;
      tr.y -= y_src;
      br.y -= y_src;
    }

    save_tlx = tl.x;
    save_blx = bl.x;
    save_trx = tr.x;
    save_brx = br.x;

    do {
      do {
        /* Clip to original source area if repeat and transform are both
           used. */
        if (src->transform && SURFACE_REPEAT (src)) {
          glitz_region_box_t src_clip, intersect_clip;
          
          src_clip.x1 = tl.x;
          src_clip.y1 = tl.y;
          src_clip.x2 = src_clip.x1 + src->width;
          src_clip.y2 = src_clip.y1 + src->height;

          glitz_intersect_region (&clip, &src_clip, &intersect_clip);

          gl->scissor (intersect_clip.x1,
                       dst->height - (intersect_clip.y1 +
                                      (intersect_clip.y2 - intersect_clip.y1)),
                       intersect_clip.x2 - intersect_clip.x1,
                       intersect_clip.y2 - intersect_clip.y1);
        }
        
        gl->begin (GLITZ_GL_QUADS);
        gl->tex_coord_2d (0.0, texture->texcoord_height);
        gl->vertex_2d (tl.x, tl.y);
        gl->tex_coord_2d (texture->texcoord_width, texture->texcoord_height);
        gl->vertex_2d (tr.x, tr.y);
        gl->tex_coord_2d (texture->texcoord_width, 0.0);
        gl->vertex_2d (br.x, br.y);
        gl->tex_coord_2d (0.0, 0.0);
        gl->vertex_2d (bl.x, bl.y);
        gl->end ();

        if (SURFACE_REPEAT (src)) {
          bl.x += src->width;
          tl.x += src->width;
          tr.x += src->width;
          br.x += src->width;
        }
        
      } while (SURFACE_REPEAT (src) && (tl.x < (x_dst + width)));

      if (SURFACE_REPEAT (src)) {
        bl.y += src->height;
        tl.y += src->height;
        tr.y += src->height;
        br.y += src->height;

        tl.x = save_tlx;
        bl.x = save_blx;
        tr.x = save_trx;
        br.x = save_brx;
      }
      
    } while (SURFACE_REPEAT (src) && (tl.y < (y_dst + height)));  
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
