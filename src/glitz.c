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

glitz_gl_uint_t _texture_units[] = {
  0,
  GLITZ_GL_TEXTURE0,
  GLITZ_GL_TEXTURE1,
  GLITZ_GL_TEXTURE2
};

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
  glitz_gl_proc_address_list_t *gl = dst->gl;
  glitz_surface_t *intermediate = NULL;
  glitz_texture_t *texture, *mask_texture = NULL;
  glitz_point_t tl, bl, br, tr;
  glitz_bounding_box_t clip;
  glitz_render_op_t render_op;
  glitz_gl_uint_t list = 0;
  int texture_unit = 0;
  
  glitz_render_op_init (&render_op, &src, &mask, dst,
                        &x_src, &y_src, &x_mask, &y_mask);
  if (render_op.type == GLITZ_RENDER_TYPE_NA) {
    glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    return;
  }
  
  if (render_op.type == GLITZ_RENDER_TYPE_INTERMEDIATE) {
    glitz_bounding_box_t dst_bounds, mask_bounds;
    glitz_intermediate_t intermediate_type;
    static glitz_color_t clear_color = { 0x0000, 0x0000, 0x0000, 0x0000 };

    dst_bounds.x1 = x_dst;
    dst_bounds.x2 = x_dst + width;
    dst_bounds.y1 = y_dst;
    dst_bounds.y2 = y_dst + height;
      
    glitz_mask_bounds (src, mask, dst,
                       x_src, y_src, x_mask, y_mask, x_dst, y_dst,
                       &dst_bounds, &mask_bounds);
    if ((mask_bounds.x2 - x_dst) <= 0 || (mask_bounds.y2 - y_dst) <= 0)
      return;
    
    if (src->transform && (!SURFACE_REPEAT (src)))
      intermediate_type = GLITZ_INTERMEDIATE_RGBA_STENCIL;
    else
      intermediate_type = GLITZ_INTERMEDIATE_RGBA;
    
    intermediate =
      glitz_surface_create_intermediate (dst, intermediate_type,
                                         mask_bounds.x2 - x_dst,
                                         mask_bounds.y2 - y_dst);
    
    if (!intermediate) {
      glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
      return;
    }

    if (mask->transform)
      glitz_fill_rectangle (GLITZ_OPERATOR_SRC,
                            intermediate,
                            &clear_color,
                            0, 0,
                            intermediate->width,
                            intermediate->height);

    glitz_composite (GLITZ_OPERATOR_SRC,
                     mask, NULL, intermediate,
                     x_mask, y_mask,
                     0, 0,
                     0, 0,
                     intermediate->width,
                     intermediate->height);

    if (src->transform && (!SURFACE_REPEAT (src)))
      intermediate->hint_mask |= GLITZ_INT_HINT_CLEAR_EXTERIOR_MASK;
    
    glitz_composite (GLITZ_OPERATOR_IN,
                     src, NULL, intermediate,
                     x_src, y_src,
                     0, 0,
                     0, 0,
                     intermediate->width,
                     intermediate->height);
    
    width = intermediate->width;
    height = intermediate->height;
    src = intermediate;
    mask = NULL;
    x_src = y_src = 0;

    glitz_render_op_init (&render_op, &src, &mask, dst,
                          &x_src, &y_src, &x_mask, &y_mask);
    if (render_op.type == GLITZ_RENDER_TYPE_NA) {
      glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
      glitz_surface_destroy (intermediate);
      return;
    }
  }
  
  texture = glitz_surface_get_texture (src);
  
  /* Source texture has not been allocated, hence source and the result of this
     operation is undefined. So lets do nothing. */
  if (!texture)
    return;

  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_pop_current (dst);
    return;
  }
  
  if (mask) { /* MULTI-TEXTURE */
    glitz_bounding_box_double_t src_box, mask_box, dst_box;
    glitz_point_t src_tl, src_br, mask_tl, mask_br;
    glitz_gl_enum_t src_texture_unit = 0;

    mask_texture = glitz_surface_get_texture (mask);
    
    /* Mask texture has not been allocated, hence source and the result of this
       operation is undefined. So lets do nothing. */
    if (!mask_texture) {
      glitz_surface_pop_current (dst);
      return;
    }
    
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
      src_box.y1 += y_dst - y_src;
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
      mask_box.y1 += y_dst - y_mask;
      mask_box.y2 += y_dst - y_mask;

      if (!SURFACE_PROGRAMMATIC (mask))
        glitz_intersect_bounding_box_double (&dst_box, &mask_box, &dst_box);
    }

    if ((dst_box.x2 - dst_box.x1) <= 0 || (dst_box.y2 - dst_box.y1) <= 0) {
      glitz_surface_pop_current (dst);
      return;
    }

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

    src_tl.x = (src_tl.x / src->width) * texture->texcoord_width;
    src_tl.y = (src_tl.y / src->height) * texture->texcoord_height;
    src_br.x = (src_br.x / src->width) * texture->texcoord_width;
    src_br.y = (src_br.y / src->height) * texture->texcoord_height;

    if (!SURFACE_REPEAT (src)) {
      src_tl.y = texture->texcoord_height - src_tl.y;
      src_br.y = texture->texcoord_height - src_br.y;
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

    gl->disable (GLITZ_GL_SCISSOR_TEST);

    texture_unit++;
    glitz_texture_bind (gl, mask_texture);

    if (mask->transform)
      glitz_texture_ensure_filter (gl, mask_texture, mask->filter);
    else
      glitz_texture_ensure_filter (gl, mask_texture, GLITZ_FILTER_NEAREST);
    
    glitz_texture_ensure_repeat (gl, mask_texture, SURFACE_REPEAT (mask));

    while (texture_unit < render_op.render->texture_units) {
      src_texture_unit = _texture_units[++texture_unit];
      gl->active_texture (src_texture_unit);
      glitz_texture_bind (gl, texture);
    }

    if (src->transform)
      glitz_texture_ensure_filter (gl, texture, src->filter);
    else
      glitz_texture_ensure_filter (gl, texture, GLITZ_FILTER_NEAREST);

    glitz_texture_ensure_repeat (gl, texture, SURFACE_REPEAT (src));

    glitz_render_op_set_textures (&render_op, texture, mask_texture);
    glitz_set_operator (gl, op);

    if (render_op.component_alpha) {
      list = gl->gen_lists (1);
      gl->new_list (list, GLITZ_GL_COMPILE);
    } else
      glitz_render_enable (&render_op);

    gl->begin (GLITZ_GL_QUADS);

    gl->multi_tex_coord_2d (GLITZ_GL_TEXTURE0, mask_tl.x, mask_tl.y);
    gl->multi_tex_coord_2d (src_texture_unit, src_tl.x, src_tl.y);
    gl->vertex_2d (dst_box.x1, dst_box.y1);

    gl->multi_tex_coord_2d (GLITZ_GL_TEXTURE0, mask_br.x, mask_tl.y);
    gl->multi_tex_coord_2d (src_texture_unit, src_br.x, src_tl.y);
    gl->vertex_2d (dst_box.x2, dst_box.y1);

    gl->multi_tex_coord_2d (GLITZ_GL_TEXTURE0, mask_br.x, mask_br.y);
    gl->multi_tex_coord_2d (src_texture_unit, src_br.x, src_br.y);
    gl->vertex_2d (dst_box.x2, dst_box.y2);

    gl->multi_tex_coord_2d (GLITZ_GL_TEXTURE0, mask_tl.x, mask_br.y);
    gl->multi_tex_coord_2d (src_texture_unit, src_tl.x, src_br.y);
    gl->vertex_2d (dst_box.x1, dst_box.y2);

    gl->end ();

    clip.x1 = dst_box.x1;
    clip.y1 = dst_box.y1;
    clip.x2 = dst_box.x2 + 0.5;
    clip.y2 = dst_box.y2 + 0.5;
    
  } else { /* SINGLE-TEXTURE */

    while (texture_unit < render_op.render->texture_units) {
      if (texture_unit++)
        gl->active_texture (_texture_units[texture_unit]);
      glitz_texture_bind (gl, texture);
    }
    
    clip.x1 = x_dst;
    clip.y1 = y_dst;
    clip.x2 = clip.x1 + width;
    clip.y2 = clip.y1 + height;

    if (SURFACE_CLEAR_EXTERIOR (dst)) {
      gl->clear_stencil (0x0);
      gl->clear (GLITZ_GL_STENCIL_BUFFER_BIT);
      glitz_set_stencil_operator (gl, GLITZ_STENCIL_OPERATOR_SET, 0x1);
    }
  
    gl->scissor (clip.x1, dst->height - (clip.y1 + height), width, height);

    glitz_render_op_set_textures (&render_op, texture, NULL);
    glitz_set_operator (gl, op);
    
    if (dst->multi_sample || render_op.component_alpha)
      list = gl->gen_lists (1);
  
    if ((!src->transform) && SURFACE_REPEAT (src) && texture->repeatable) {
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

      if (list)
        gl->new_list (list, GLITZ_GL_COMPILE);
      else
        glitz_render_enable (&render_op);
      
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
      double save_base_x1, save_base_x2, min, max;
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

      if (list)
        gl->new_list (list, GLITZ_GL_COMPILE);
      else
        glitz_render_enable (&render_op);
    
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

            min = MIN (tl.x, MIN (bl.x, MIN (tr.x, br.x)));
            max = MAX (tl.x, MAX (bl.x, MAX (tr.x, br.x)));
          
            if ((min > (x_dst + width)) || (max < x_dst))
              x_is_ok = 0;
            else
              x_is_ok = 1;

            min = MIN (tl.y, MIN (bl.y, MIN (tr.y, br.y)));
            max = MAX (tl.y, MAX (bl.y, MAX (tr.y, br.y)));

            if ((min > (y_dst + height)) || (max < y_dst))
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
  }

  if (list) {
    gl->end_list ();
    
    if (dst->multi_sample) {
      unsigned int mask = *dst->stencil_mask & ~0x1;
      unsigned short alpha, opacity;
      int i;
    
      glitz_render_op_get_alpha_mask (&render_op, NULL, NULL, NULL, &opacity);
    
      for (i = 0; i < dst->multi_sample->n_samples; i++) {
        if ((i + 1) == dst->multi_sample->n_samples)
          glitz_set_stencil_operator (gl, GLITZ_STENCIL_OPERATOR_CLIP,
                                      mask | (i + 1));
        else
          glitz_set_stencil_operator (gl, GLITZ_STENCIL_OPERATOR_CLIP_EQUAL,
                                      mask | (i + 1));
        
        alpha = SHORT_MULT (dst->multi_sample->weights[i], opacity);
        glitz_render_op_set_alpha_mask (&render_op, 0x0, 0x0, 0x0, alpha);
        
        glitz_render_enable (&render_op);
        
        gl->call_list (list);
      }
    } else {
      glitz_render_op_set_alpha_mask (&render_op, 0xffff, 0x0, 0x0, 0x0);
      glitz_render_enable (&render_op);
      gl->color_mask (1, 0, 0, 0);
      gl->call_list (list);

      glitz_render_op_set_alpha_mask (&render_op, 0x0, 0x0, 0xffff, 0x0);
      glitz_render_enable (&render_op);
      gl->color_mask (0, 0, 1, 0);
      gl->call_list (list);
      
      if (render_op.component_alpha == GLITZ_COMPONENT_ALPHA_ARGB) {
        glitz_render_op_set_alpha_mask (&render_op, 0x0, 0xffff, 0x0, 0x0);
        glitz_render_enable (&render_op);
        gl->color_mask (0, 1, 0, 0);
        gl->call_list (list);

        glitz_render_op_set_alpha_mask (&render_op, 0x0, 0x0, 0x0, 0xffff);
        glitz_render_enable (&render_op);
        gl->color_mask (0, 0, 0, 1);
        gl->call_list (list);
      } else {
        glitz_render_op_set_alpha_mask (&render_op, 0x0, 0xffff, 0x0, 0x0);
        glitz_render_enable (&render_op);
        gl->color_mask (0, 1, 0, 1);
        gl->call_list (list);
      }
    }
    
    gl->delete_lists (list, 1);
  }

  glitz_render_disable (&render_op);

  for (; texture_unit; texture_unit--) {
    if (texture_unit == 1 && mask)
      glitz_texture_unbind (gl, mask_texture);
    else
      glitz_texture_unbind (gl, texture);

    if (texture_unit > 1)
      gl->active_texture (_texture_units[texture_unit - 1]);
  }
  
  if (SURFACE_CLEAR_EXTERIOR (dst)) {    
    glitz_set_operator (gl, GLITZ_OPERATOR_SRC);
    glitz_set_stencil_operator (gl, GLITZ_STENCIL_OPERATOR_CLIP_EQUAL, 0x0);

    gl->color_mask (1, 1, 1, 1);
    gl->color_4us (0x0, 0x0, 0x0, 0x0);
    
    gl->begin (GLITZ_GL_QUADS);
    gl->vertex_2d (0.0, 0.0);
    gl->vertex_2d (dst->width, 0.0);
    gl->vertex_2d (dst->width, dst->height);
    gl->vertex_2d (0.0, dst->height);
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
  int status;
  
  if (SURFACE_PROGRAMMATIC (dst) || SURFACE_PROGRAMMATIC (src))
    return;

  if (x_src < 0) {
    x_dst -= x_src;
    width += x_src;
    x_src = 0;
  }

  if (y_src < 0) {
    y_dst -= y_src;
    height += y_src;
    y_src = 0;
  }

  width = MIN (src->width - x_src, width);
  height = MIN (src->height - y_src, height);

  if (x_dst < 0) {
    x_src -= x_dst;
    width += x_dst;
    x_dst = 0;
  }

  if (y_dst < 0) {
    y_src -= y_dst;
    height += y_dst;
    y_dst = 0;
  }
  
  width = MIN (dst->width - x_dst, width);
  height = MIN (dst->height - y_dst, height);

  if (width <= 0 || height <= 0)
    return;

  gl = dst->gl;

  status = 0;
  if (glitz_surface_try_push_current (dst,
                                      GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_bounding_box_t box;
    
    if (src != dst)
      status = glitz_surface_make_current_read (src);
    else
      status = 1;

    if (status) {
      if (src->format->doublebuffer)
        gl->read_buffer (src->read_buffer);

      gl->disable (GLITZ_GL_SCISSOR_TEST);
      gl->disable (GLITZ_GL_DITHER);
      gl->disable (GLITZ_GL_STENCIL_TEST);
      glitz_set_operator (gl, GLITZ_OPERATOR_SRC);

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
        gl->color_4us (0x0, 0x0, 0x0, 0xffff);

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

    box.x1 = x_dst;
    box.y1 = y_dst;
    box.x2 = box.x1 + width;
    box.y2 = box.y1 + height;
    
    glitz_surface_dirty (dst, &box);
    glitz_surface_pop_current (dst);

    status = 1;
  }

  if (!status) {
    if (glitz_surface_try_push_current (src,
                                        GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
      glitz_texture_copy_surface (&dst->texture, src,
                                  x_src, y_src, width, height, x_dst, y_dst);
      status = 1;
    }
    glitz_surface_pop_current (src);
  }

  if (!status) {
    static glitz_pixel_format_t pf = {
      {
        32,
        0xff000000,
        0x00ff0000,
        0x0000ff00,
        0x000000ff
      },
      0, 0, 0,
      GLITZ_PIXEL_SCANLINE_ORDER_BOTTOM_UP
    };
    glitz_pixel_buffer_t *buffer =
      glitz_pixel_buffer_create (src,
                                 NULL,
                                 width * height * 4,
                                 GLITZ_PIXEL_BUFFER_HINT_STATIC_COPY);
    if (!buffer) {
      glitz_surface_status_add (dst, GLITZ_STATUS_NO_MEMORY_MASK);
      return;
    }

    glitz_pixel_buffer_set_format (buffer, &pf);
    
    glitz_get_pixels (src, x_src, y_src, width, height, buffer);
    glitz_put_pixels (dst, x_dst, y_dst, width, height, buffer);
    
    glitz_pixel_buffer_destroy (buffer);
  }
}
