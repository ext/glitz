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

static glitz_surface_t *
_glitz_programmatic_surface_create_similar (void *abstract_templ,
                                            glitz_format_t *format,
                                            int width,
                                            int height)
{
  return NULL;
}

static void
_glitz_programmatic_surface_destroy (void *abstract_surface)
{
  glitz_programmatic_surface_t *surface =
    (glitz_programmatic_surface_t *) abstract_surface;

  switch (surface->type) {
  case GLITZ_PROGRAMMATIC_SURFACE_TYPE_LINEAR:
    glitz_color_range_destroy (surface->u.linear.color_range);
    break;
  case GLITZ_PROGRAMMATIC_SURFACE_TYPE_RADIAL:
    glitz_color_range_destroy (surface->u.radial.color_range);
  default:
    break;
  }
  
  glitz_surface_fini (&surface->base);
  
  free (surface);
}

static glitz_bool_t
_glitz_programmatic_surface_push_current (void *abstract_surface,
                                          glitz_constraint_t constraint)
{
  glitz_programmatic_surface_t *surface =
    (glitz_programmatic_surface_t *) abstract_surface;
  
  glitz_surface_status_add (&surface->base, GLITZ_STATUS_NOT_SUPPORTED_MASK);
  
  return 0;
}

static void
_glitz_programmatic_surface_pop_current (void *abstract_surface) {}

static glitz_texture_t *
_glitz_programmatic_surface_get_texture (void *abstract_surface)
{
  glitz_programmatic_surface_t *surface =
    (glitz_programmatic_surface_t *) abstract_surface;
  
  return &surface->base.texture;
}

static void
_glitz_programmatic_surface_update_size (void *abstract_surface) {}

static void
_glitz_programmatic_surface_swap_buffers (void *abstract_surface) {}

static glitz_bool_t
_glitz_programmatic_surface_make_current_read (void *abstract_surface)
{
  return 0;
}

static const struct glitz_surface_backend
glitz_programmatic_surface_backend = {
  _glitz_programmatic_surface_create_similar,
  _glitz_programmatic_surface_destroy,
  _glitz_programmatic_surface_push_current,
  _glitz_programmatic_surface_pop_current,
  _glitz_programmatic_surface_get_texture,
  _glitz_programmatic_surface_update_size,
  _glitz_programmatic_surface_swap_buffers,
  _glitz_programmatic_surface_make_current_read
};

static glitz_programmatic_surface_t *
_glitz_programmatic_surface_create (void)
{
  static const glitz_matrix_t identity = {
    {
      { 1.0, 0.0, 0.0 },
      { 0.0, 1.0, 0.0 },
      { 0.0, 0.0, 1.0 }
    }
  };
  glitz_programmatic_surface_t *surface;

  surface = (glitz_programmatic_surface_t *)
    calloc (1, sizeof (glitz_programmatic_surface_t));
  if (surface == NULL)
    return NULL;
  
  glitz_surface_init (&surface->base,
                      &glitz_programmatic_surface_backend,
                      NULL, NULL, NULL, 0, MAXSHORT, MAXSHORT, NULL, 0);

  surface->base.hint_mask |= GLITZ_HINT_PROGRAMMATIC_MASK;
  
  surface->base.texture.target = GLITZ_GL_TEXTURE_2D;
  surface->base.texture.format = GLITZ_GL_RGBA;
  surface->base.texture.filter = surface->base.filter;
  surface->base.texture.texcoord_width = surface->base.width =
    surface->base.texture.width = MAXSHORT;
  surface->base.texture.texcoord_height = surface->base.height =
    surface->base.texture.height = MAXSHORT;
  surface->base.texture.repeatable = surface->base.texture.repeat = 1;
  surface->matrix = identity;
  
  return surface;
}

glitz_surface_t *
glitz_programmatic_surface_create_solid (glitz_color_t *color)
{
  glitz_programmatic_surface_t *surface;
  
  surface = _glitz_programmatic_surface_create ();
  if (!surface)
    return NULL;

  surface->type = GLITZ_PROGRAMMATIC_SURFACE_TYPE_SOLID;
  surface->u.solid.color = *color;
  
  return &surface->base;
}

glitz_surface_t *
glitz_programmatic_surface_create_linear (glitz_point_fixed_t *start,
                                          glitz_point_fixed_t *stop,
                                          glitz_color_range_t *color_range)
{
  glitz_programmatic_surface_t *surface;

  surface = _glitz_programmatic_surface_create ();
  if (!surface)
    return NULL;

  surface->type = GLITZ_PROGRAMMATIC_SURFACE_TYPE_LINEAR;
  surface->u.linear.start = *start;
  surface->u.linear.stop = *stop;
  surface->u.linear.color_range = color_range;
  glitz_color_range_reference (color_range);
  
  return &surface->base;
}

glitz_surface_t *
glitz_programmatic_surface_create_radial (glitz_point_fixed_t *start,
                                          glitz_fixed16_16_t radius0,
                                          glitz_fixed16_16_t radius1,
                                          glitz_color_range_t *color_range)
{
  glitz_programmatic_surface_t *surface;

  surface = _glitz_programmatic_surface_create ();
  if (!surface)
    return NULL;

  surface->type = GLITZ_PROGRAMMATIC_SURFACE_TYPE_RADIAL;
  surface->u.radial.center = *start;
  surface->u.radial.radius0 = radius0;
  surface->u.radial.radius1 = radius1;
  surface->u.radial.color_range = color_range;
  glitz_color_range_reference (color_range);
  
  return &surface->base;
}

void
glitz_programmatic_surface_set_transform (glitz_surface_t *abstract_surface,
                                          glitz_transform_t *transform)
{
  glitz_programmatic_surface_t *surface =
    (glitz_programmatic_surface_t *) abstract_surface;
  
  surface->matrix.m[0][0] = FIXED_TO_DOUBLE (transform->matrix[0][0]);
  surface->matrix.m[1][0] = FIXED_TO_DOUBLE (transform->matrix[0][1]);
  surface->matrix.m[2][0] = FIXED_TO_DOUBLE (transform->matrix[0][2]);
  
  surface->matrix.m[0][1] = FIXED_TO_DOUBLE (transform->matrix[1][0]);
  surface->matrix.m[1][1] = FIXED_TO_DOUBLE (transform->matrix[1][1]);
  surface->matrix.m[2][1] = FIXED_TO_DOUBLE (transform->matrix[1][2]);
}
