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

static glitz_surface_t *
_glitz_programmatic_surface_create_similar (void *abstract_templ,
                                            glitz_format_name_t format_name,
                                            glitz_bool_t drawable,
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
  case GLITZ_PROGRAMMATIC_SURFACE_LINEAR_TYPE:
    glitz_color_range_destroy (surface->u.linear.color_range);
    break;
  case GLITZ_PROGRAMMATIC_SURFACE_RADIAL_TYPE:
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
_glitz_programmatic_surface_realize (void *abstract_surface) {}

static void
_glitz_programmatic_surface_show (void *abstract_surface) {}

static const struct glitz_surface_backend
glitz_programmatic_surface_backend = {
  _glitz_programmatic_surface_create_similar,
  _glitz_programmatic_surface_destroy,
  _glitz_programmatic_surface_push_current,
  _glitz_programmatic_surface_pop_current,
  _glitz_programmatic_surface_get_texture,
  _glitz_programmatic_surface_realize,
  _glitz_programmatic_surface_show
};

glitz_programmatic_surface_t *
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
                      NULL, NULL, 1, 1, NULL, 0);

  surface->base.hint_mask |= GLITZ_HINT_PROGRAMMATIC_MASK;
  
  surface->base.texture.target = GLITZ_GL_TEXTURE_2D;
  surface->base.texture.format = GLITZ_GL_RGBA;
  surface->base.texture.filter = surface->base.filter;
  surface->base.texture.texcoord_width =
    surface->base.texture.texcoord_height = 1.0;
  surface->base.texture.repeatable = surface->base.texture.repeat = 1;
  surface->transform = identity;
  
  return surface;
}

void
glitz_programmatic_surface_setup (glitz_surface_t *abstract_surface,
                                  int width,
                                  int height)
{
  glitz_programmatic_surface_t *surface =
    (glitz_programmatic_surface_t *) abstract_surface;
  
  surface->base.texture.texcoord_width = surface->base.width =
    surface->base.texture.width = width;
  surface->base.texture.texcoord_height = surface->base.height =
    surface->base.texture.height = height;
}

glitz_surface_t *
glitz_programmatic_surface_create_solid (glitz_color_t *color)
{
  glitz_programmatic_surface_t *surface;
  
  surface = _glitz_programmatic_surface_create ();
  if (!surface)
    return NULL;

  surface->type = GLITZ_PROGRAMMATIC_SURFACE_SOLID_TYPE;
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

  surface->type = GLITZ_PROGRAMMATIC_SURFACE_LINEAR_TYPE;
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

  surface->type = GLITZ_PROGRAMMATIC_SURFACE_RADIAL_TYPE;
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
  
  surface->transform.m[0][0] = FIXED_TO_DOUBLE (transform->matrix[0][0]);
  surface->transform.m[1][0] = FIXED_TO_DOUBLE (transform->matrix[0][1]);
  surface->transform.m[2][0] = FIXED_TO_DOUBLE (transform->matrix[0][2]);
  
  surface->transform.m[0][1] = FIXED_TO_DOUBLE (transform->matrix[1][0]);
  surface->transform.m[1][1] = FIXED_TO_DOUBLE (transform->matrix[1][1]);
  surface->transform.m[2][1] = FIXED_TO_DOUBLE (transform->matrix[1][2]);
}

void
glitz_programmatic_surface_bind (glitz_gl_proc_address_list_t *gl,
                                 glitz_programmatic_surface_t *surface,
                                 unsigned long feature_mask)
{
  switch (surface->type) {
  case GLITZ_PROGRAMMATIC_SURFACE_SOLID_TYPE: {
    gl->color_4us (surface->u.solid.color.red,
                   surface->u.solid.color.green,
                   surface->u.solid.color.blue,
                   surface->u.solid.color.alpha);
  } break;
  case GLITZ_PROGRAMMATIC_SURFACE_LINEAR_TYPE: {
    glitz_point_t p1, p2;
    double length, angle, start;
    
    p1.x = FIXED_TO_DOUBLE (surface->u.linear.start.x);
    p1.y = FIXED_TO_DOUBLE (surface->u.linear.start.y);
    p2.x = FIXED_TO_DOUBLE (surface->u.linear.stop.x);
    p2.y = FIXED_TO_DOUBLE (surface->u.linear.stop.y);
    
    length = sqrt ((p2.x - p1.x) * (p2.x - p1.x) +
                   (p2.y - p1.y) * (p2.y - p1.y));
  
    angle = -atan2 (p2.y - p1.y, p2.x - p1.x);

    start = cos (angle) * p1.x;
    start += -sin (angle) * p1.y;

    gl->program_local_param_4d_arb (GLITZ_GL_FRAGMENT_PROGRAM_ARB, 0,
                                    start,
                                    (length)? 1.0 / length: INT_MAX,
                                    cos (angle),
                                    -sin (angle));
    gl->program_local_param_4d_arb (GLITZ_GL_FRAGMENT_PROGRAM_ARB, 1,
                                    surface->transform.m[0][0],
                                    surface->transform.m[0][1],
                                    surface->transform.m[1][0],
                                    surface->transform.m[1][1]);
    gl->program_local_param_4d_arb (GLITZ_GL_FRAGMENT_PROGRAM_ARB, 2,
                                    surface->transform.m[2][0],
                                    surface->transform.m[2][1],
                                    surface->base.height, 0.0);

    gl->active_texture_arb (GLITZ_GL_TEXTURE2_ARB);
    glitz_color_range_bind (gl, surface->u.linear.color_range, feature_mask);
    gl->active_texture_arb (GLITZ_GL_TEXTURE0_ARB);
  } break;
  case GLITZ_PROGRAMMATIC_SURFACE_RADIAL_TYPE:
    gl->program_local_param_4d_arb
      (GLITZ_GL_FRAGMENT_PROGRAM_ARB, 0,
       FIXED_TO_DOUBLE (surface->u.radial.center.x),
       FIXED_TO_DOUBLE (surface->u.radial.center.y),
       1.0 / (FIXED_TO_DOUBLE (surface->u.radial.radius1) -
              FIXED_TO_DOUBLE (surface->u.radial.radius0)),
       FIXED_TO_DOUBLE (surface->u.radial.radius0));
    gl->program_local_param_4d_arb (GLITZ_GL_FRAGMENT_PROGRAM_ARB, 1,
                                    surface->transform.m[0][0],
                                    surface->transform.m[0][1],
                                    surface->transform.m[1][0],
                                    surface->transform.m[1][1]);
    gl->program_local_param_4d_arb (GLITZ_GL_FRAGMENT_PROGRAM_ARB, 2,
                                    surface->transform.m[2][0],
                                    surface->transform.m[2][1],
                                    surface->base.height, 0.0);

    gl->active_texture_arb (GLITZ_GL_TEXTURE2_ARB);
    glitz_color_range_bind (gl, surface->u.radial.color_range, feature_mask);
    gl->active_texture_arb (GLITZ_GL_TEXTURE0_ARB);
  default:
    break;
  }
}
