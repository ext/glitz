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

#include <math.h>

typedef struct _glitz_convolution_filter_t {
  glitz_matrix_t matrix;
} glitz_convolution_filter_t;

typedef struct _glitz_linear_gradient_filter_t {
  glitz_point_fixed_t start;
  glitz_point_fixed_t stop;
} glitz_linear_gradient_filter_t;

typedef struct _glitz_radial_gradient_filter_t {
  glitz_point_fixed_t center;
  glitz_fixed16_16_t radius0;
  glitz_fixed16_16_t radius1;
} glitz_radial_gradient_filter_t;

struct _glitz_filter_params_t {
  union {
    glitz_convolution_filter_t convolution;
    glitz_linear_gradient_filter_t linear;
    glitz_radial_gradient_filter_t radial;
  } u;
};

static int
_glitz_filter_params_ensure (glitz_filter_params_t **params)
{
  if (*params == NULL) {
    *params = malloc (sizeof (glitz_filter_params_t));
    if (*params == NULL)      
      return 1;
  }
  
  return 0;
}

static void
_glitz_filter_params_set_fixed (glitz_fixed16_16_t *fixed,
                                const glitz_fixed16_16_t default_fixed,
                                glitz_fixed16_16_t **params,
                                int *n_params)
{
  if (*n_params > 0) {
    *fixed = **params;
    (*params)++;
    n_params--;
  } else
    *fixed = default_fixed;
}

static void
_glitz_filter_params_set_double (double *value,
                                 const double default_value,
                                 glitz_fixed16_16_t **params,
                                 int *n_params)
{
  if (*n_params > 0) {
    *value = FIXED_TO_DOUBLE (**params);
    (*params)++;
    n_params--;
  } else
    *value = default_value;
}

static void
_glitz_filter_params_set_point (glitz_point_fixed_t *point,
                                const glitz_point_fixed_t *default_point,
                                glitz_fixed16_16_t **params,
                                int *n_params)
{
  _glitz_filter_params_set_fixed (&point->x, default_point->x,
                                  params, n_params);
  _glitz_filter_params_set_fixed (&point->y, default_point->y,
                                  params, n_params);
}

static void
_glitz_filter_params_set_matrix (glitz_matrix_t *matrix,
                                 const glitz_matrix_t *default_matrix,
                                 glitz_fixed16_16_t **params,
                                 int *n_params)
{
  int row, col;
  
  for (row = 0; row < 3; row++)
    for (col = 0; col < 3; col++)
      _glitz_filter_params_set_double (&matrix->m[row][col],
                                       default_matrix->m[row][col],
                                       params, n_params);
}


static const glitz_matrix_t _default_convolution_kernel = {
  {
    { 1.0, 1.0, 1.0 },
    { 1.0, 1.0, 1.0 },
    { 1.0, 1.0, 1.0 }
  }
};

static const glitz_point_fixed_t _point_zero = {
  0, 0
};

static const glitz_point_fixed_t _point_one = {
  65536, 65536
};

glitz_status_t
glitz_filter_set_params (glitz_filter_params_t **filter_params,
                         glitz_filter_t filter,
                         glitz_fixed16_16_t *params,
                         int n_params)
{
  switch (filter) {
  case GLITZ_FILTER_CONVOLUTION: {
    glitz_fixed16_16_t w, h;
    
    if (_glitz_filter_params_ensure (filter_params))
      return GLITZ_STATUS_NO_MEMORY;
    
    _glitz_filter_params_set_fixed (&w, INT_TO_FIXED (3), &params, &n_params);
    _glitz_filter_params_set_fixed (&h, INT_TO_FIXED (3), &params, &n_params);

    /* TODO: support any convolution filter size */
    if (w != INT_TO_FIXED (3) || h != INT_TO_FIXED (3))
      return GLITZ_STATUS_NOT_SUPPORTED;
    
    _glitz_filter_params_set_matrix (&(*filter_params)->u.convolution.matrix,
                                     &_default_convolution_kernel,
                                     &params, &n_params);
    
    if (glitz_matrix_normalize (&(*filter_params)->u.convolution.matrix))
      return GLITZ_STATUS_INVALID_MATRIX_MASK;
    
  } break;
  case GLITZ_FILTER_LINEAR_GRADIENT:
    if (_glitz_filter_params_ensure (filter_params))
      return GLITZ_STATUS_NO_MEMORY;

    _glitz_filter_params_set_point (&(*filter_params)->u.linear.start,
                                    &_point_zero,
                                    &params, &n_params);
    _glitz_filter_params_set_point (&(*filter_params)->u.linear.stop,
                                    &_point_one,
                                    &params, &n_params);
    break;
  case GLITZ_FILTER_RADIAL_GRADIENT:
    if (_glitz_filter_params_ensure (filter_params))
      return GLITZ_STATUS_NO_MEMORY;

    _glitz_filter_params_set_point (&(*filter_params)->u.radial.center,
                                    &_point_zero, &params, &n_params);
    _glitz_filter_params_set_fixed (&(*filter_params)->u.radial.radius0,
                                    0, &params, &n_params);
    _glitz_filter_params_set_fixed (&(*filter_params)->u.radial.radius1,
                                    65536, &params, &n_params);
    break;
  case GLITZ_FILTER_BILINEAR:
  case GLITZ_FILTER_NEAREST:
    break;
  }

  return GLITZ_STATUS_SUCCESS;
}

void
glitz_filter_enable (glitz_surface_t *surface,
                     glitz_composite_op_t *op)
{
  glitz_gl_proc_address_list_t *gl = op->gl;

  gl->enable (GLITZ_GL_VERTEX_PROGRAM);
  gl->bind_program (GLITZ_GL_VERTEX_PROGRAM, op->vp);
  
  gl->enable (GLITZ_GL_FRAGMENT_PROGRAM);
  gl->bind_program (GLITZ_GL_FRAGMENT_PROGRAM, op->fp);

  switch (surface->filter) {
  case GLITZ_FILTER_CONVOLUTION: {
    glitz_convolution_filter_t *c = &surface->filter_params->u.convolution;
    double dw, dh;

    dw = surface->texture.texcoord_width / (double) surface->texture.width;
    dh = surface->texture.texcoord_height / (double) surface->texture.height;

    gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 0,
                                0.0, 0.0, c->matrix.m[1][1], 0.0);
    gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 1,
                                -dw, 0.0, c->matrix.m[0][1], 0.0);
    gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 2,
                                dw, 0.0, c->matrix.m[2][1], 0.0);
    gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 3,
                                0.0, -dh, c->matrix.m[1][0], 0.0);
    gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 4,
                                0.0, dh, c->matrix.m[1][2], 0.0);
    gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 5,
                                -dw, -dh, c->matrix.m[0][0], 0.0);
    gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 6,
                                dw, -dh, c->matrix.m[2][0], 0.0);
    gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 7,
                                -dw, dh, c->matrix.m[0][2], 0.0);
    gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 8,
                                dw, dh, c->matrix.m[2][2], 0.0);
  } break;
  case GLITZ_FILTER_LINEAR_GRADIENT: {
    glitz_linear_gradient_filter_t *linear = &surface->filter_params->u.linear;
    glitz_point_t p1, p2;
    double length, angle, start, cos_angle, sin_angle;
    
    p1.x = FIXED_TO_DOUBLE (linear->start.x);
    p1.y = FIXED_TO_DOUBLE (linear->start.y);
    p2.x = FIXED_TO_DOUBLE (linear->stop.x);
    p2.y = FIXED_TO_DOUBLE (linear->stop.y);
      
    length = sqrt ((p2.x - p1.x) * (p2.x - p1.x) +
                   (p2.y - p1.y) * (p2.y - p1.y));
      
    angle = -atan2 (p2.y - p1.y, p2.x - p1.x);

    cos_angle = cos (angle);
    sin_angle = sin (angle);
      
    start = cos_angle * p1.x;
    start += -sin_angle * p1.y;

    gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 0, start,
                                (length)? 1.0 / length: INT_MAX,
                                cos_angle, -sin_angle);
    gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 1,
                                surface->texture.texcoord_width,
                                0.0, 0.0, 0.0);
  } break;
  case GLITZ_FILTER_RADIAL_GRADIENT: {
    glitz_radial_gradient_filter_t *radial = &surface->filter_params->u.radial;
    
    gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 0,
                                FIXED_TO_DOUBLE (radial->center.x),
                                FIXED_TO_DOUBLE (radial->center.y),
                                1.0 / (FIXED_TO_DOUBLE (radial->radius1) -
                                       FIXED_TO_DOUBLE (radial->radius0)),
                                FIXED_TO_DOUBLE (radial->radius0));
    gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 1,
                                surface->texture.texcoord_width,
                                0.0, 0.0, 0.0);
  } break;
  case GLITZ_FILTER_BILINEAR:
  case GLITZ_FILTER_NEAREST:
    break;
  }
}
  
