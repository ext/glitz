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

void
glitz_texture_init (glitz_texture_t *texture,
                    unsigned int width,
                    unsigned int height,
                    unsigned int texture_format,
                    unsigned long feature_mask)
{
  texture->filter = 0;
  texture->wrap = 0;
  texture->format = texture_format;
  texture->name = 0;
  texture->flags = GLITZ_TEXTURE_FLAG_REPEATABLE_MASK |
    GLITZ_TEXTURE_FLAG_PADABLE_MASK;

  if (feature_mask & GLITZ_FEATURE_TEXTURE_BORDER_CLAMP_MASK) {
    texture->box.x1 = texture->box.y1 = 0;
    texture->box.x2 = texture->width = width;
    texture->box.y2 = texture->height = height;
  } else {
    texture->box.x1 = texture->box.y1 = 1;
    texture->box.x2 = width + 1;
    texture->box.y2 = height + 1;
    texture->width = width + 2;
    texture->height = height + 2;
    texture->flags &= ~(GLITZ_TEXTURE_FLAG_REPEATABLE_MASK |
                        GLITZ_TEXTURE_FLAG_PADABLE_MASK);
  }

  if ((feature_mask & GLITZ_FEATURE_TEXTURE_NON_POWER_OF_TWO_MASK) ||
      (POWER_OF_TWO (texture->width) && POWER_OF_TWO (texture->height))) {
    texture->target = GLITZ_GL_TEXTURE_2D;
  } else {
    texture->flags &= ~GLITZ_TEXTURE_FLAG_REPEATABLE_MASK;
      
    if (feature_mask & GLITZ_FEATURE_TEXTURE_RECTANGLE_MASK) {
      texture->target = GLITZ_GL_TEXTURE_RECTANGLE;
    } else {
      texture->target = GLITZ_GL_TEXTURE_2D;
      texture->flags &= ~GLITZ_TEXTURE_FLAG_PADABLE_MASK;
        
      if (!POWER_OF_TWO (texture->width))
        texture->width = glitz_uint_to_power_of_two (texture->width);
      
      if (!POWER_OF_TWO (texture->height))
        texture->height = glitz_uint_to_power_of_two (texture->height);
    }
  }

  if (texture->target == GLITZ_GL_TEXTURE_2D) {
    texture->texcoord_width_unit = 1.0f / texture->width;
    texture->texcoord_height_unit = 1.0f / texture->height;
  } else {
    texture->texcoord_width_unit = 1.0f;
    texture->texcoord_height_unit = 1.0f;
  }
}

void
glitz_texture_allocate (glitz_gl_proc_address_list_t *gl,
                        glitz_texture_t *texture)
{
  char *data = NULL;
  
  if (!texture->name)
    gl->gen_textures (1, &texture->name);

  texture->flags |= GLITZ_TEXTURE_FLAG_ALLOCATED_MASK;
  
  glitz_texture_bind (gl, texture);

  if (texture->box.x2 != texture->width ||
      texture->box.y2 != texture->height) {
    data = malloc (texture->width * texture->height * 4);
    if (data)
      memset (data, 0, texture->width * texture->height * 4);
  }
  
  gl->tex_image_2d (texture->target, 0, texture->format,
                    texture->width, texture->height,
                    0, GLITZ_GL_RGBA, GLITZ_GL_UNSIGNED_BYTE, data);
  
  glitz_texture_unbind (gl, texture);

  if (data)
    free (data);
}

void 
glitz_texture_fini (glitz_gl_proc_address_list_t *gl,
                    glitz_texture_t *texture)
{
  if (texture->name)
    gl->delete_textures (1, &texture->name);
}

void
glitz_texture_ensure_filter (glitz_gl_proc_address_list_t *gl,
                             glitz_texture_t *texture,
                             glitz_gl_enum_t filter)
{
  if (!texture->name)
    return;
    
  if (texture->filter != filter) {
    gl->tex_parameter_i (texture->target, GLITZ_GL_TEXTURE_MAG_FILTER, filter);
    gl->tex_parameter_i (texture->target, GLITZ_GL_TEXTURE_MIN_FILTER, filter);
    texture->filter = filter;
  }
}

void
glitz_texture_ensure_wrap (glitz_gl_proc_address_list_t *gl,
                           glitz_texture_t *texture,
                           glitz_gl_enum_t wrap)
{
  if (!texture->name)
    return;
  
  if (texture->wrap != wrap) {
    gl->tex_parameter_i (texture->target, GLITZ_GL_TEXTURE_WRAP_S, wrap);
    gl->tex_parameter_i (texture->target, GLITZ_GL_TEXTURE_WRAP_T, wrap);
    texture->wrap = wrap;
  }
}

void
glitz_texture_bind (glitz_gl_proc_address_list_t *gl,
                    glitz_texture_t *texture)
{  
  gl->disable (GLITZ_GL_TEXTURE_RECTANGLE);
  gl->disable (GLITZ_GL_TEXTURE_2D);

  if (!texture->name)
    return;

  gl->enable (texture->target);
  gl->bind_texture (texture->target, texture->name);
}

void
glitz_texture_unbind (glitz_gl_proc_address_list_t *gl,
                      glitz_texture_t *texture)
{
  gl->bind_texture (texture->target, 0);
  gl->disable (texture->target);
}

void
glitz_texture_copy_surface (glitz_texture_t *texture,
                            glitz_surface_t *surface,
                            int x_surface,
                            int y_surface,
                            int width,
                            int height,
                            int x_texture,
                            int y_texture)
{
  glitz_gl_proc_address_list_t *gl = &surface->backend->gl;
  
  glitz_surface_push_current (surface, GLITZ_CN_SURFACE_DRAWABLE_CURRENT);
  
  glitz_texture_bind (gl, texture);

  if (surface->format->doublebuffer)
    gl->read_buffer (surface->read_buffer);

  gl->copy_tex_sub_image_2d (texture->target, 0,
                             texture->box.x1 + x_texture,
                             texture->box.y2 - y_texture - height,
                             x_surface,
                             surface->height - y_surface - height,
                             width, height);

  glitz_texture_unbind (gl, texture);
  glitz_surface_pop_current (surface);
}

void
glitz_texture_set_tex_gen (glitz_gl_proc_address_list_t *gl,
                           glitz_texture_t *texture,
                           int x_src,
                           int y_src,
                           unsigned long flags)
{
  glitz_vec4_t plane;

  if (flags & GLITZ_SURFACE_FLAG_TEXTURE_COORDS_MASK) {
    plane.v[1] = plane.v[2] = 0.0f;
    plane.v[0] = texture->texcoord_width_unit;
    plane.v[3] = -(x_src - texture->box.x1) * texture->texcoord_width_unit;
  } else {
    plane.v[1] = plane.v[2] = 0.0f;
    plane.v[0] = 1.0;
    plane.v[3] = -x_src;
  }   
  
  gl->tex_gen_i (GLITZ_GL_S, GLITZ_GL_TEXTURE_GEN_MODE,
                 GLITZ_GL_EYE_LINEAR);
  gl->tex_gen_fv (GLITZ_GL_S, GLITZ_GL_EYE_PLANE, plane.v);
  gl->enable (GLITZ_GL_TEXTURE_GEN_S);

  if (flags & GLITZ_SURFACE_FLAG_TEXTURE_COORDS_MASK) {
    plane.v[0] = 0.0f;
    plane.v[1] = -texture->texcoord_height_unit;
    plane.v[3] = (y_src + texture->box.y2) * texture->texcoord_height_unit;
  } else {
    plane.v[0] = 0.0f;
    plane.v[1] = 1.0;
    plane.v[3] = y_src;
  }
  
  gl->tex_gen_i (GLITZ_GL_T, GLITZ_GL_TEXTURE_GEN_MODE,
                 GLITZ_GL_EYE_LINEAR);
  gl->tex_gen_fv (GLITZ_GL_T, GLITZ_GL_EYE_PLANE, plane.v);
  gl->enable (GLITZ_GL_TEXTURE_GEN_T);
}
