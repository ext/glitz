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

#include <stdlib.h>
#include <string.h>

void
glitz_surface_init (glitz_surface_t *surface,
                    glitz_surface_backend_t *backend,
                    glitz_format_t *format,
                    int width,
                    int height)
{
  surface->backend = backend;

  surface->ref_count = 1;

  surface->filter = GLITZ_FILTER_NEAREST;
  
  surface->format = format;
  surface->width = width;
  surface->height = height;
  surface->update_mask = GLITZ_UPDATE_ALL_MASK;

  if (format->doublebuffer)
    surface->draw_buffer = surface->read_buffer = GLITZ_GL_BACK;
  else
    surface->draw_buffer = surface->read_buffer = GLITZ_GL_FRONT;

  if (width == 1 && height == 1) {
    surface->flags |= GLITZ_SURFACE_FLAG_SOLID_MASK;
    surface->solid.red = surface->solid.green = surface->solid.blue = 0x0;
    surface->solid.alpha = 0xffff;
  }

  glitz_texture_init (&surface->texture,
                      width, height,
                      glitz_format_get_best_texture_format (backend->formats,
                                                            backend->n_formats,
                                                            format),
                      backend->feature_mask);
}

void
glitz_surface_fini (glitz_surface_t *surface)
{
  if (surface->geometry.buffer)
    glitz_buffer_destroy (surface->geometry.buffer);
   
  if (surface->texture.name) {
    glitz_surface_push_current (surface, GLITZ_CN_ANY_CONTEXT_CURRENT);
    glitz_texture_fini (&surface->backend->gl, &surface->texture);
    glitz_surface_pop_current (surface);
  }
  
  if (surface->transform)
    free (surface->transform);
  
  if (surface->filter_params)
    glitz_filter_params_destroy (surface->filter_params);
}

glitz_format_t *
glitz_surface_find_similar_standard_format (glitz_surface_t *surface,
                                            glitz_format_name_t format_name)
{
  return
    glitz_format_find_standard (surface->backend->formats,
                                surface->backend->n_formats,
                                format_name);
}
slim_hidden_def(glitz_surface_find_similar_standard_format);

glitz_format_t *
glitz_surface_find_similar_format (glitz_surface_t *surface,
                                   unsigned long mask,
                                   const glitz_format_t *templ,
                                   int count)
{
  return glitz_format_find (surface->backend->formats,
                            surface->backend->n_formats,
                            mask, templ, count);
}

glitz_surface_t *
glitz_surface_create_similar (glitz_surface_t *templ,
                              glitz_format_t *format,
                              int width,
                              int height)
{
  if (width < 1 || height < 1)
    return NULL;
  
  return templ->backend->create_similar (templ, format, width, height);
}

void
glitz_surface_reference (glitz_surface_t *surface)
{
  if (surface == NULL)
    return;

  surface->ref_count++;
}

void
glitz_surface_destroy (glitz_surface_t *surface)
{
  if (!surface)
    return;

  surface->ref_count--;
  if (surface->ref_count)
    return;
  
  surface->backend->destroy (surface);
}

static void
_glitz_surface_solid_store (glitz_surface_t *surface) {
  glitz_gl_proc_address_list_t *gl = &surface->backend->gl;

  if (SURFACE_DRAWABLE (surface)) {
    glitz_bounding_box_t box;

    box.x1 = box.y1 = 0;
    box.x2 = box.y2 = 1;
    
    gl->color_4us (surface->solid.red,
                   surface->solid.green,
                   surface->solid.blue,
                   surface->solid.alpha);
    
    glitz_set_operator (gl, GLITZ_OPERATOR_SRC);
    
    glitz_geometry_enable_default (gl, surface, &box);
    
    gl->draw_arrays (GLITZ_GL_QUADS, 0, 4);
    
    glitz_geometry_disable (gl, surface);
  } else {
    glitz_gl_float_t color[4];

    if (TEXTURE_ALLOCATED (&surface->texture)) {
      color[0] = surface->solid.red / 65535.0f;
      color[1] = surface->solid.green / 65535.0f;
      color[2] = surface->solid.blue / 65535.0f;
      color[3] = surface->solid.alpha / 65535.0f;
    
      glitz_texture_bind (gl, &surface->texture);
      gl->tex_sub_image_2d (surface->texture.target, 0,
                            surface->texture.box.x1, surface->texture.box.y1,
                            1, 1, GLITZ_GL_RGBA, GLITZ_GL_FLOAT, color);
      glitz_texture_unbind (gl, &surface->texture);
    }
  }
  surface->flags &= ~GLITZ_SURFACE_FLAG_DRAWABLE_DIRTY_MASK;
}

void
glitz_surface_ensure_solid (glitz_surface_t *surface)
{
  if (SURFACE_DIRTY (surface) || SURFACE_SOLID_DIRTY (surface)) {
    glitz_gl_proc_address_list_t *gl = &surface->backend->gl;
    glitz_gl_float_t *c, color[64];
    glitz_texture_t *texture;

    texture = glitz_surface_get_texture (surface, 0);

    c = &color[(texture->box.y1 * texture->width + texture->box.x1) * 4];
    if (texture) {
      glitz_texture_bind (gl, texture);
      gl->get_tex_image (texture->target, 0,
                         GLITZ_GL_RGBA, GLITZ_GL_FLOAT, color);
      glitz_texture_unbind (gl, texture);
    } else {
      c[0] = c[1] = c[2] = 0.0f;
      c[3] = 1.0f;
    }
  
    surface->solid.red = c[0] * 65535.0f;
    surface->solid.green = c[1] * 65535.0f;
    surface->solid.blue = c[2] * 65535.0f;
    surface->solid.alpha = c[3] * 65535.0f;

    surface->flags &= ~GLITZ_SURFACE_FLAG_SOLID_DIRTY_MASK;
  }
}

glitz_bool_t
glitz_surface_push_current (glitz_surface_t *surface,
                            glitz_constraint_t constraint)
{
  if (!surface->backend->push_current (surface, constraint))
    return 0;

  if (SURFACE_SOLID (surface) && SURFACE_DRAWABLE_DIRTY (surface))
    _glitz_surface_solid_store (surface);

  return 1;
}

void
glitz_surface_pop_current (glitz_surface_t *surface)
{
  surface->backend->pop_current (surface);
}

void
glitz_surface_set_transform (glitz_surface_t *surface,
                             glitz_transform_t *transform)
{
  static glitz_transform_t identity = {
    {
      { FIXED1, 0x00000, 0x00000 },
      { 0x00000, FIXED1, 0x00000 },
      { 0x00000, 0x00000, FIXED1 }
    }
  };

  if (transform &&
      memcmp (transform, &identity, sizeof (glitz_transform_t)) == 0)
    transform = NULL;
  
  if (transform) {
    glitz_gl_float_t height, *m, *t;
    
    if (!surface->transform) {
      surface->transform = malloc (sizeof (glitz_matrix_t));
      if (surface->transform == NULL) {
        glitz_surface_status_add (surface, GLITZ_STATUS_NO_MEMORY_MASK);
        return;
      }
    }

    m = surface->transform->m;
    t = surface->transform->t;

    m[0] = FIXED_TO_FLOAT (transform->matrix[0][0]);
    m[4] = FIXED_TO_FLOAT (transform->matrix[0][1]);
    m[8] = 0.0f;
    m[12] = FIXED_TO_FLOAT (transform->matrix[0][2]);

    m[1] = FIXED_TO_FLOAT (transform->matrix[1][0]);
    m[5] = FIXED_TO_FLOAT (transform->matrix[1][1]);
    m[9] = 0.0f;
    m[13] = FIXED_TO_FLOAT (transform->matrix[1][2]);

    m[2] = 0.0f;
    m[6] = 0.0f;
    m[10] = 1.0f;
    m[14] = 0.0f;
    
    m[3] = FIXED_TO_FLOAT (transform->matrix[2][0]);
    m[7] = FIXED_TO_FLOAT (transform->matrix[2][1]);
    m[11] = 0.0f;
    m[15] = FIXED_TO_FLOAT (transform->matrix[2][2]);
    
    /* Projective transformation matrix conversion to normalized
       texture coordinates seems to be working fine. However, it would be
       good if someone could verify that this is actually a correct way for
       doing this.
       
       We need to do we this:
       
       scale (IDENTITY, 0, -1)
       translate (IDENTITY, 0, -texture_height)
       multiply (TEXTURE_MATRIX, IDENTITY, MATRIX)
       scale (TEXTURE_MATRIX, 0, -1)
       translate (TEXTURE_MATRIX, 0, -texture_height)
       
       the following code does it pretty efficiently. */

    height = surface->texture.texcoord_height_unit *
      (surface->texture.box.y2 - surface->texture.box.y1);
      
    t[0] = m[0];
    t[4] = m[4];
    t[8] = 0.0f;
    t[12] = surface->texture.texcoord_width_unit * m[12];
    
    t[3] = m[3] / surface->texture.texcoord_width_unit;
    t[7] = m[7] / surface->texture.texcoord_height_unit;
    t[11] = 0.0f;
    t[15] = m[15];
    
    t[1] = height * t[3] - m[1];
    t[5] = height * t[7] - m[5];
    t[9] = 0.0f;
    t[13] = height * t[15] - surface->texture.texcoord_height_unit * m[13];

    t[2] = 0.0f;
    t[6] = 0.0f;
    t[10] = 1.0f;
    t[14] = 0.0f;

    /* scale y = -1 */
    t[4] = -t[4];
    t[5] = -t[5];
    t[7] = -t[7];

    /* translate y = -texture_height */
    t[12] -= t[4] * height;
    t[13] -= t[5] * height;
    t[15] -= t[7] * height;

    height = surface->texture.texcoord_height_unit * surface->texture.box.y1;
    
    /* Translate coordinates into texture. This only makes a difference when
       GL_ARB_texture_border_clamp is missing as box.x1 and box.y1 are
       otherwise always zero. This breaks projective transformations so
       those wont work without GL_ARB_texture_border_clamp. */
    t[12] += surface->texture.texcoord_width_unit * surface->texture.box.x1;
    t[13] += surface->texture.texcoord_height_unit * surface->texture.box.y1;

    surface->flags |= GLITZ_SURFACE_FLAG_TRANSFORM_MASK;
    if (m[3] != 0.0f || m[7] != 0.0f || (m[15] != 1.0f && m[15] != -1.0f))
      surface->flags |= GLITZ_SURFACE_FLAG_PROJECTIVE_TRANSFORM_MASK;
  } else {
    if (surface->transform)
      free (surface->transform);
    
    surface->transform = NULL;
    surface->flags &= ~GLITZ_SURFACE_FLAG_TRANSFORM_MASK;
    surface->flags &= ~GLITZ_SURFACE_FLAG_PROJECTIVE_TRANSFORM_MASK;
  }
}
slim_hidden_def(glitz_surface_set_transform);

void
glitz_surface_set_fill (glitz_surface_t *surface,
                        glitz_fill_t fill)
{
  switch (fill) {
  case GLITZ_FILL_TRANSPARENT:
    surface->flags &= ~GLITZ_SURFACE_FLAG_REPEAT_MASK;
    surface->flags &= ~GLITZ_SURFACE_FLAG_MIRRORED_MASK;
    surface->flags &= ~GLITZ_SURFACE_FLAG_PAD_MASK;
    break;
  case GLITZ_FILL_NEAREST:
    surface->flags &= ~GLITZ_SURFACE_FLAG_REPEAT_MASK;
    surface->flags &= ~GLITZ_SURFACE_FLAG_MIRRORED_MASK;
    surface->flags |= GLITZ_SURFACE_FLAG_PAD_MASK;
    break;
  case GLITZ_FILL_REPEAT:
    surface->flags |= GLITZ_SURFACE_FLAG_REPEAT_MASK;
    surface->flags &= ~GLITZ_SURFACE_FLAG_MIRRORED_MASK;
    surface->flags &= ~GLITZ_SURFACE_FLAG_PAD_MASK;
    break;
  case GLITZ_FILL_REFLECT:
    surface->flags |= GLITZ_SURFACE_FLAG_REPEAT_MASK;
    surface->flags |= GLITZ_SURFACE_FLAG_MIRRORED_MASK;
    surface->flags &= ~GLITZ_SURFACE_FLAG_PAD_MASK;
    break;
  }

  glitz_filter_set_type (surface, surface->filter);
}
slim_hidden_def(glitz_surface_set_fill);

void
glitz_surface_set_component_alpha (glitz_surface_t *surface,
                                   glitz_bool_t component_alpha)
{
  if (component_alpha && surface->format->red_size)
    surface->flags |= GLITZ_SURFACE_FLAG_COMPONENT_ALPHA_MASK;
  else
    surface->flags &= ~GLITZ_SURFACE_FLAG_COMPONENT_ALPHA_MASK;
}
slim_hidden_def(glitz_surface_set_component_alpha);

void
glitz_surface_set_filter (glitz_surface_t *surface,
                          glitz_filter_t filter,
                          glitz_fixed16_16_t *params,
                          int n_params)
{
  glitz_status_t status;
  
  status = glitz_filter_set_params (surface, filter, params, n_params);
  if (status) {
    glitz_surface_status_add (surface, glitz_status_to_status_mask (status));
  } else {
    switch (filter) {
    case GLITZ_FILTER_NEAREST:
      surface->flags &= ~GLITZ_SURFACE_FLAG_FRAGMENT_FILTER_MASK;
      surface->flags &= ~GLITZ_SURFACE_FLAG_LINEAR_TRANSFORM_FILTER_MASK;
      surface->flags &= ~GLITZ_SURFACE_FLAG_IGNORE_WRAP_MASK;
      surface->flags &= ~GLITZ_SURFACE_FLAG_EYE_COORDS_MASK;
      break;
    case GLITZ_FILTER_BILINEAR:
      surface->flags &= ~GLITZ_SURFACE_FLAG_FRAGMENT_FILTER_MASK;
      surface->flags |= GLITZ_SURFACE_FLAG_LINEAR_TRANSFORM_FILTER_MASK;
      surface->flags &= ~GLITZ_SURFACE_FLAG_IGNORE_WRAP_MASK;
      surface->flags &= ~GLITZ_SURFACE_FLAG_EYE_COORDS_MASK;
      break;
    case GLITZ_FILTER_CONVOLUTION:
    case GLITZ_FILTER_GAUSSIAN:
      surface->flags |= GLITZ_SURFACE_FLAG_FRAGMENT_FILTER_MASK;
      surface->flags |= GLITZ_SURFACE_FLAG_LINEAR_TRANSFORM_FILTER_MASK;
      surface->flags &= ~GLITZ_SURFACE_FLAG_IGNORE_WRAP_MASK;
      surface->flags &= ~GLITZ_SURFACE_FLAG_EYE_COORDS_MASK;
      break;
    case GLITZ_FILTER_LINEAR_GRADIENT:
    case GLITZ_FILTER_RADIAL_GRADIENT:
      surface->flags |= GLITZ_SURFACE_FLAG_FRAGMENT_FILTER_MASK;
      surface->flags &= ~GLITZ_SURFACE_FLAG_LINEAR_TRANSFORM_FILTER_MASK;
      surface->flags |= GLITZ_SURFACE_FLAG_IGNORE_WRAP_MASK;
      surface->flags |= GLITZ_SURFACE_FLAG_EYE_COORDS_MASK;
      break;
    }
    surface->filter = filter;
  }
}
slim_hidden_def(glitz_surface_set_filter);

int
glitz_surface_get_width (glitz_surface_t *surface)
{
  return surface->width;
}
slim_hidden_def(glitz_surface_get_width);

int
glitz_surface_get_height (glitz_surface_t *surface)
{
  return surface->height;
}
slim_hidden_def(glitz_surface_get_height);

glitz_texture_t *
glitz_surface_get_texture (glitz_surface_t *surface,
                           glitz_bool_t allocate)
{
  glitz_texture_t *texture;

  if (SURFACE_SOLID (surface) && SURFACE_DRAWABLE_DIRTY (surface)) {
    texture = surface->backend->get_texture (surface, 1);
    _glitz_surface_solid_store (surface);
  } else
    texture = surface->backend->get_texture (surface, allocate);

  return texture;
}

static glitz_gl_enum_t
_gl_color_buffer (glitz_color_buffer_t buffer)
{
  switch (buffer) {
  case GLITZ_COLOR_BUFFER_FRONT:
    return GLITZ_GL_FRONT;
  case GLITZ_COLOR_BUFFER_BACK:
    return GLITZ_GL_BACK;
  default:
    return GLITZ_GL_BACK;
  }
}

void
glitz_surface_set_read_color_buffer (glitz_surface_t *surface,
                                     glitz_color_buffer_t buffer)
{
  glitz_gl_enum_t mode;
  
  if (!surface->format->doublebuffer)
    return;
    
  mode = _gl_color_buffer (buffer);
  if (mode != surface->read_buffer) {
    surface->read_buffer = mode;
    glitz_surface_dirty (surface, NULL);
  }
}
slim_hidden_def(glitz_surface_set_read_color_buffer);

void
glitz_surface_set_draw_color_buffer (glitz_surface_t *surface,
                                     glitz_color_buffer_t buffer)
{
  if (!surface->format->doublebuffer)
    return;

  surface->draw_buffer = _gl_color_buffer (buffer);

  surface->update_mask |= GLITZ_UPDATE_DRAW_BUFFER_MASK;
}
slim_hidden_def(glitz_surface_set_draw_color_buffer);

void
glitz_surface_swap_buffers (glitz_surface_t *surface)
{
  if (!surface->format->doublebuffer)
    return;
  
  surface->backend->swap_buffers (surface);
}
slim_hidden_def(glitz_surface_swap_buffers);

void
glitz_surface_flush (glitz_surface_t *surface)
{
  if (glitz_surface_push_current (surface, GLITZ_CN_SURFACE_DRAWABLE_CURRENT))
    surface->backend->gl.flush ();
  
  glitz_surface_pop_current (surface);
}
slim_hidden_def(glitz_surface_flush);

void
glitz_surface_finish (glitz_surface_t *surface)
{
  if (glitz_surface_push_current (surface, GLITZ_CN_SURFACE_DRAWABLE_CURRENT))
    surface->backend->gl.finish ();
  
  glitz_surface_pop_current (surface);
}
slim_hidden_def(glitz_surface_finish);

glitz_bool_t
glitz_surface_make_current_read (glitz_surface_t *surface)
{
  return surface->backend->make_current_read (surface);
}

void
glitz_surface_dirty (glitz_surface_t *surface,
                     glitz_bounding_box_t *box)
{ 
  if (surface->draw_buffer != surface->read_buffer)
    return;
  
  if (!box) {
    surface->dirty_box.x1 = surface->dirty_box.y1 = 0;
    surface->dirty_box.x2 = surface->width;
    surface->dirty_box.y2 = surface->height;
  } else {
    if (!SURFACE_DIRTY (surface)) {
      surface->dirty_box = *box;
    } else
      glitz_union_bounding_box (box,
                                &surface->dirty_box,
                                &surface->dirty_box);
  }
  
  surface->flags |=
    (GLITZ_SURFACE_FLAG_DIRTY_MASK | GLITZ_SURFACE_FLAG_SOLID_DIRTY_MASK);
}

void
glitz_surface_status_add (glitz_surface_t *surface, int flags)
{
  surface->status_mask |= flags;
}

glitz_status_t
glitz_surface_get_status (glitz_surface_t *surface)
{
  return glitz_status_pop_from_mask (&surface->status_mask);
}
slim_hidden_def(glitz_surface_get_status);

void
glitz_surface_update_state (glitz_surface_t *surface)
{
  glitz_gl_proc_address_list_t *gl = &surface->backend->gl;
  
  if (surface->update_mask & GLITZ_UPDATE_VIEWPORT_MASK) {
    gl->viewport (0, 0, surface->width, surface->height);
    gl->matrix_mode (GLITZ_GL_PROJECTION);
    gl->load_identity ();
    gl->ortho (0.0, surface->width, 0.0, surface->height, -1.0, 1.0);
    gl->matrix_mode (GLITZ_GL_MODELVIEW);
    gl->load_identity ();
    gl->scale_f (1.0f, -1.0f, 1.0f);
    gl->translate_f (0.0f, -surface->height, 0.0f);
    gl->disable (GLITZ_GL_DEPTH_TEST); 
    gl->hint (GLITZ_GL_PERSPECTIVE_CORRECTION_HINT, GLITZ_GL_FASTEST);
    gl->disable (GLITZ_GL_CULL_FACE);
    gl->depth_mask (GLITZ_GL_FALSE);  
    gl->polygon_mode (GLITZ_GL_FRONT_AND_BACK, GLITZ_GL_FILL);
    gl->disable (GLITZ_GL_POLYGON_SMOOTH);
    gl->shade_model (GLITZ_GL_FLAT);
    gl->color_mask (GLITZ_GL_TRUE, GLITZ_GL_TRUE, GLITZ_GL_TRUE,
                    GLITZ_GL_TRUE);
    gl->disable (GLITZ_GL_DITHER);
    gl->enable (GLITZ_GL_SCISSOR_TEST);
    gl->scissor (0, 0, surface->width, surface->height);
    gl->disable (GLITZ_GL_STENCIL_TEST);
    gl->enable_client_state (GLITZ_GL_VERTEX_ARRAY);
    
    surface->update_mask &= ~GLITZ_UPDATE_VIEWPORT_MASK;
  }
  
  if (surface->update_mask & GLITZ_UPDATE_DRAW_BUFFER_MASK) {
    if (surface->format->doublebuffer)
      gl->draw_buffer (surface->draw_buffer);
    
    surface->update_mask &= ~GLITZ_UPDATE_DRAW_BUFFER_MASK;
  }

  if (surface->update_mask & GLITZ_UPDATE_MULTISAMPLE_MASK) {
    if (surface->format->multisample.samples) {
      if (SURFACE_MULTISAMPLE (surface)) {
        gl->enable (GLITZ_GL_MULTISAMPLE);
        
        if (surface->backend->feature_mask &
            GLITZ_FEATURE_MULTISAMPLE_FILTER_HINT_MASK) {
          if (SURFACE_NICEST_MULTISAMPLE (surface))
            gl->hint (GLITZ_GL_MULTISAMPLE_FILTER_HINT, GLITZ_GL_NICEST);
          else
            gl->hint (GLITZ_GL_MULTISAMPLE_FILTER_HINT, GLITZ_GL_FASTEST);
        }
      } else
        gl->disable (GLITZ_GL_MULTISAMPLE);
    }
    
    surface->update_mask &= ~GLITZ_UPDATE_MULTISAMPLE_MASK;
  }
}

unsigned long
glitz_surface_get_features (glitz_surface_t *surface)
{
  return surface->backend->feature_mask;
}
slim_hidden_def(glitz_surface_get_features);

glitz_format_t *
glitz_surface_get_format (glitz_surface_t *surface)
{
  return surface->format;
}
slim_hidden_def(glitz_surface_get_format);
