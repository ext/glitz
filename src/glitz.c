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

#define SET_COLOR(color, set_red, set_green, set_blue, set_alpha) \
  (color).red = (set_red); \
  (color).green = (set_green); \
  (color).blue = (set_blue); \
  (color).alpha = (set_alpha)

glitz_gl_uint_t _texture_units[] = {
  GLITZ_GL_TEXTURE0,
  GLITZ_GL_TEXTURE1,
  GLITZ_GL_TEXTURE2
};

typedef struct _glitz_texture_unit_t {
  glitz_texture_t *texture;
  glitz_gl_uint_t unit;
  glitz_bool_t transform;
} glitz_texture_unit_t;

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
  glitz_gl_proc_address_list_t *gl = &dst->backend->gl;
  glitz_surface_t *intermediate = NULL;
  glitz_bounding_box_t rect;
  glitz_composite_op_t comp_op;
  int i, passes, texture_nr = -1;
  glitz_texture_t *stexture, *mtexture;
  glitz_texture_unit_t textures[3];
  glitz_color_t alpha_mask;
  glitz_gl_enum_t primitive;
  glitz_gl_int_t first;
  glitz_gl_sizei_t count;

  if (width <= 0 || height <= 0)
    return;
  
  glitz_composite_op_init (&comp_op, src, mask, dst);
  if (comp_op.type == GLITZ_COMBINE_TYPE_NA) {
    glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    return;
  }

  src = comp_op.src;
  mask = comp_op.mask;
  
  if (comp_op.type == GLITZ_COMBINE_TYPE_INTERMEDIATE) {
    glitz_format_t templ;
    glitz_format_t *format;
    unsigned long templ_mask;
    
    templ.red_size = src->format->red_size;
    templ.green_size = src->format->green_size;
    templ.blue_size = src->format->blue_size;
    templ.alpha_size = MAX (src->format->alpha_size, mask->format->alpha_size);
    templ.draw.offscreen = 1;
    
    templ_mask = GLITZ_FORMAT_RED_SIZE_MASK | GLITZ_FORMAT_GREEN_SIZE_MASK |
      GLITZ_FORMAT_BLUE_SIZE_MASK | GLITZ_FORMAT_ALPHA_SIZE_MASK |
      GLITZ_FORMAT_DRAW_OFFSCREEN_MASK;
    
    format = glitz_surface_find_similar_format (dst, templ_mask, &templ, 0);
    if (!format) {
      glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
      return;
    }
    
    intermediate = glitz_surface_create_similar (dst, format, width, height);  
    if (!intermediate) {
      glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
      return;
    }
    
    glitz_composite (GLITZ_OPERATOR_SRC,
                     mask, NULL, intermediate,
                     x_mask, y_mask,
                     0, 0,
                     0, 0,
                     width,
                     height);

    glitz_composite (GLITZ_OPERATOR_IN,
                     src, NULL, intermediate,
                     x_src, y_src,
                     0, 0,
                     0, 0,
                     width,
                     height);
    
    src = intermediate;
    mask = NULL;
    x_src = y_src = 0;

    glitz_composite_op_init (&comp_op, src, mask, dst);
    if (comp_op.type == GLITZ_COMBINE_TYPE_NA) {
      glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
      glitz_surface_destroy (intermediate);
      return;
    }

    src = comp_op.src;
    mask = comp_op.mask;
  }

  if (src) {
    stexture = glitz_surface_get_texture (src, 0);
    if (!stexture)
      return;
  } else
    stexture = NULL;

  if (mask) {
    mtexture = glitz_surface_get_texture (mask, 0);
    if (!mtexture)
      return;
  } else
    mtexture = NULL;

  if (!glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_status_add (dst, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    glitz_surface_pop_current (dst);
    return;
  }

  rect.x1 = x_dst;
  rect.y1 = y_dst;
  rect.x2 = rect.x1 + width;
  rect.y2 = rect.y1 + height;

  if (mtexture) {
    textures[0].texture = mtexture;
    textures[0].unit = _texture_units[0];
    textures[0].transform = 0;
    texture_nr = 0;
    
    glitz_texture_bind (gl, mtexture);

    glitz_texture_set_tex_gen (gl,
                               mtexture,
                               rect.x1 - x_mask,
                               rect.y1 - y_mask,
                               mask->height,
                               mask->flags);
    
    if (mask->transform) {
      textures[0].transform = 1;
      gl->matrix_mode (GLITZ_GL_TEXTURE);
      gl->load_matrix_f (SURFACE_TEXTURE_COORDS (mask)?
                         mask->transform->t: mask->transform->m);
      gl->matrix_mode (GLITZ_GL_MODELVIEW);
      
      if (SURFACE_LINEAR_TRANSFORM_FILTER (mask))
        glitz_texture_ensure_filter (gl, mtexture, GLITZ_GL_LINEAR);
      else
        glitz_texture_ensure_filter (gl, mtexture, GLITZ_GL_NEAREST);
    } else
      glitz_texture_ensure_filter (gl, mtexture, GLITZ_GL_NEAREST);
    
    if (SURFACE_REPEAT (mask)) {
      if (SURFACE_MIRRORED (mask))
        glitz_texture_ensure_wrap (gl, mtexture, GLITZ_GL_MIRRORED_REPEAT);
      else
        glitz_texture_ensure_wrap (gl, mtexture, GLITZ_GL_REPEAT);
    } else {
      if (SURFACE_PAD (mask))
        glitz_texture_ensure_wrap (gl, mtexture, GLITZ_GL_CLAMP_TO_EDGE);
      else
        glitz_texture_ensure_wrap (gl, mtexture, GLITZ_GL_CLAMP_TO_BORDER);
    }
  }
    
  if (stexture) {
    int last_texture_nr = comp_op.combine->texture_units - 1;
    
    while (texture_nr < last_texture_nr) {
      textures[++texture_nr].texture = stexture;
      textures[texture_nr].unit = _texture_units[texture_nr];
      textures[texture_nr].transform = 0;
      if (texture_nr > 0)
        gl->active_texture (textures[texture_nr].unit);
      glitz_texture_bind (gl, stexture);
    }

    glitz_texture_set_tex_gen (gl,
                               stexture,
                               rect.x1 - y_src,
                               rect.y1 - x_src,
                               src->height,
                               src->flags);

    if (src->transform) {
      textures[texture_nr].transform = 1;
      gl->matrix_mode (GLITZ_GL_TEXTURE);
      gl->load_matrix_f (SURFACE_TEXTURE_COORDS (src)?
                         src->transform->t: src->transform->m);
      gl->matrix_mode (GLITZ_GL_MODELVIEW);
      
      if (SURFACE_LINEAR_TRANSFORM_FILTER (src))
        glitz_texture_ensure_filter (gl, stexture, GLITZ_GL_LINEAR);
      else
        glitz_texture_ensure_filter (gl, stexture, GLITZ_GL_NEAREST);
    } else
      glitz_texture_ensure_filter (gl, stexture, GLITZ_GL_NEAREST);
      
    if (SURFACE_REPEAT (src)) {
      if (SURFACE_MIRRORED (src))
        glitz_texture_ensure_wrap (gl, stexture, GLITZ_GL_MIRRORED_REPEAT);
      else
        glitz_texture_ensure_wrap (gl, stexture, GLITZ_GL_REPEAT);
    } else {
      if (SURFACE_PAD (src))
        glitz_texture_ensure_wrap (gl, stexture, GLITZ_GL_CLAMP_TO_EDGE);
      else
        glitz_texture_ensure_wrap (gl, stexture, GLITZ_GL_CLAMP_TO_BORDER);
    }
  }

  glitz_set_operator (gl, op);

  gl->scissor (rect.x1,
               dst->height - rect.y2,
               rect.x2 - rect.x1,
               rect.y2 - rect.y1);

  gl->push_matrix ();
  
  glitz_geometry_enable (gl, dst, &primitive, &first, &count);

  if (dst->indirect) {
    glitz_sample_offset_t *offsets;

    passes = dst->indirect->n_samples;
    offsets = dst->indirect->offsets;

    gl->enable (GLITZ_GL_STENCIL_TEST);
    gl->color_mask (GLITZ_GL_FALSE, GLITZ_GL_FALSE, GLITZ_GL_FALSE,
                    GLITZ_GL_FALSE);
    gl->clear_stencil (0);
    gl->clear (GLITZ_GL_STENCIL_BUFFER_BIT);
    gl->stencil_func (GLITZ_GL_ALWAYS, ~0, ~0);
    gl->stencil_op (GLITZ_GL_KEEP, GLITZ_GL_INCR, GLITZ_GL_INCR);
    
    for (i = 0; i < passes; i++) {
      gl->push_matrix ();
      gl->translate_f (offsets[i].x, offsets[i].y, 0.0f);
      gl->draw_arrays (primitive, first, count);
      gl->pop_matrix ();
    }

    gl->pop_matrix ();
    gl->push_matrix ();
    
    gl->color_mask (GLITZ_GL_TRUE, GLITZ_GL_TRUE, GLITZ_GL_TRUE,
                    GLITZ_GL_TRUE);
    
    glitz_geometry_enable_default (gl, dst);
    primitive = GLITZ_GL_QUADS;
    first = 0;
    count = 4;
  } else
    passes = 1;

  if (comp_op.component_alpha)
    passes *= comp_op.component_alpha;

  alpha_mask = comp_op.alpha_mask;

  for (i = 0; i < passes; i++) {
    unsigned short weight, sample;
    
    if (dst->indirect) {
      if (comp_op.component_alpha)
        sample = i / comp_op.component_alpha;
      else
        sample = i;
      
      weight = dst->indirect->weights[sample];
      
      gl->stencil_func (GLITZ_GL_EQUAL, sample + 1, ~0);
      gl->stencil_op (GLITZ_GL_KEEP, GLITZ_GL_KEEP, GLITZ_GL_KEEP);
    } else
      weight = 0xffff;
    
    if (comp_op.component_alpha) {
      switch (i % comp_op.component_alpha) {
      case 0:
        SET_COLOR (comp_op.alpha_mask,
                   SHORT_MULT (weight, alpha_mask.red), 0, 0, 0);
        gl->color_mask (GLITZ_GL_TRUE, GLITZ_GL_FALSE, GLITZ_GL_FALSE,
                        GLITZ_GL_FALSE);
        break;
      case 1:
        SET_COLOR (comp_op.alpha_mask,
                   0, 0, SHORT_MULT (weight, alpha_mask.blue), 0);
        gl->color_mask (GLITZ_GL_FALSE, GLITZ_GL_FALSE, GLITZ_GL_TRUE,
                        GLITZ_GL_FALSE);
        break;
      case 2:
        SET_COLOR (comp_op.alpha_mask,
                   0, SHORT_MULT (weight, alpha_mask.green), 0, 0);
        gl->color_mask (GLITZ_GL_FALSE, GLITZ_GL_TRUE, GLITZ_GL_FALSE,
                        (comp_op.component_alpha == 4)? GLITZ_GL_FALSE:
                        GLITZ_GL_TRUE);
        break;
      case 3:
        SET_COLOR (comp_op.alpha_mask,
                   0, 0, 0, SHORT_MULT (weight, alpha_mask.alpha));
        gl->color_mask (GLITZ_GL_FALSE, GLITZ_GL_FALSE, GLITZ_GL_FALSE,
                        GLITZ_GL_TRUE);
        break;
      }
    } else {
      SET_COLOR (comp_op.alpha_mask, 0, 0, 0,
                 SHORT_MULT (weight, alpha_mask.alpha));
    }
    
    glitz_composite_enable (&comp_op);
    
    gl->draw_arrays (primitive, first, count);
  }

  glitz_geometry_disable (gl, dst);

  gl->pop_matrix ();

  if (dst->indirect)
    gl->disable (GLITZ_GL_STENCIL_TEST);
  
  if (comp_op.component_alpha)
    gl->color_mask (GLITZ_GL_TRUE, GLITZ_GL_TRUE, GLITZ_GL_TRUE,
                    GLITZ_GL_TRUE);
  
  glitz_composite_disable (&comp_op);
  
  for (i = texture_nr; i >= 0; i--) {
    glitz_texture_unbind (gl, textures[i].texture);
    if (textures[i].transform) {
      gl->matrix_mode (GLITZ_GL_TEXTURE);
      gl->load_identity ();
      gl->matrix_mode (GLITZ_GL_MODELVIEW);
    }

    if (i > 0)
      gl->active_texture (textures[i - 1].unit);
  }
  
  glitz_surface_dirty (dst, &rect);
  
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

  gl = &dst->backend->gl;

  status = 0;
  if (glitz_surface_push_current (dst, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_bounding_box_t box;
    
    if (src != dst)
      status = glitz_surface_make_current_read (src);
    else
      status = 1;

    box.x1 = x_dst;
    box.y1 = y_dst;
    box.x2 = box.x1 + width;
    box.y2 = box.y1 + height;

    if (status) {
      if (src->format->doublebuffer)
        gl->read_buffer (src->read_buffer);
      
      glitz_set_operator (gl, GLITZ_OPERATOR_SRC);

      glitz_set_raster_pos (gl, x_dst, dst->height - (y_dst + height));
      gl->copy_pixels (x_src, src->height - (y_src + height),
                       width, height, GLITZ_GL_COLOR);
    } else {
      glitz_texture_t *texture = glitz_surface_get_texture (src, 0);
      if (texture) {
        glitz_texture_bind (gl, texture);

        glitz_texture_set_tex_gen (gl, texture, x_dst, y_dst, height, ~0); 

        gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
                       GLITZ_GL_REPLACE);
        gl->color_4us (0x0, 0x0, 0x0, 0xffff);
        
        glitz_texture_ensure_wrap (gl, texture, GLITZ_GL_CLAMP_TO_EDGE);
        glitz_texture_ensure_filter (gl, texture, GLITZ_GL_NEAREST);

        glitz_set_operator (gl, GLITZ_OPERATOR_SRC);

        gl->scissor (box.x1,
                     dst->height - box.y2,
                     width, height);
        
        glitz_geometry_enable_default (gl, dst);
        
        gl->draw_arrays (GLITZ_GL_QUADS, 0, 4);
        
        glitz_geometry_disable (gl, dst);

        glitz_texture_unbind (gl, texture);
      }
    }
    
    glitz_surface_dirty (dst, &box);
    glitz_surface_pop_current (dst);

    status = 1;
  }

  if (!status) {
    if (glitz_surface_push_current (src, GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
      glitz_texture_t *texture;

      texture = glitz_surface_get_texture (dst, 1);
      if (texture) {
        glitz_texture_copy_surface (texture, src,
                                    x_src, y_src, width, height, x_dst, y_dst);
        status = 1;
      }
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
    glitz_buffer_t *buffer =
      glitz_pixel_buffer_create (src,
                                 NULL,
                                 width * height * 4,
                                 GLITZ_BUFFER_HINT_STATIC_COPY);
    if (!buffer) {
      glitz_surface_status_add (dst, GLITZ_STATUS_NO_MEMORY_MASK);
      return;
    }

    glitz_get_pixels (src, x_src, y_src, width, height, &pf, buffer);
    glitz_set_pixels (dst, x_dst, y_dst, width, height, &pf, buffer);
    
    glitz_buffer_destroy (buffer);
  }
}
