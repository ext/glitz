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

typedef struct _glitz_linear_gradient_filter_t {
  glitz_point_t start;
  glitz_point_t stop;
} glitz_linear_gradient_filter_t;

typedef struct _glitz_radial_gradient_filter_t {
  glitz_point_t center;
  double radius_base, radius_scale;
} glitz_radial_gradient_filter_t;

struct _glitz_filter_params_t {
  int fp_type;
  int id;
  
  union {
    glitz_linear_gradient_filter_t linear;
    glitz_radial_gradient_filter_t radial;
  } u;

  glitz_vec4_t *vectors;
  int n_vectors;
};

static int
_glitz_filter_params_ensure (glitz_filter_params_t **params, int vectors)
{
  if (*params == NULL) {
    *params = calloc (1, sizeof (glitz_filter_params_t));
    if (*params == NULL)      
      return 1;
  }

  if ((*params)->n_vectors != vectors) {
    (*params)->vectors =
      realloc ((*params)->vectors, vectors * sizeof (glitz_vec4_t));
    (*params)->n_vectors = vectors;
  }
  
  if (vectors > 0 && (*params)->vectors == NULL) {
    free (*params);
    return 1;
  }
  
  return 0;
}

static void
_glitz_filter_params_set (double *value,
                          const double default_value,
                          glitz_fixed16_16_t **params,
                          int *n_params)
{
  if (*n_params > 0) {
    *value = FIXED_TO_DOUBLE (**params);
    (*params)++;
    (*n_params)--;
  } else
    *value = default_value;
}

static int
_glitz_color_stop_compare (const void *elem1, const void *elem2)
{
  return
    (((glitz_vec4_t *) elem1)->v[2] == ((glitz_vec4_t *) elem2)->v[2]) ?
    /* equal offsets, sort on id */
    ((((glitz_vec4_t *) elem1)->v[3] <
      ((glitz_vec4_t *) elem2)->v[3]) ? -1 : 1) :
    /* sort on offset */
    ((((glitz_vec4_t *) elem1)->v[2] <
      ((glitz_vec4_t *) elem2)->v[2]) ? -1 : 1);
}

glitz_status_t
glitz_filter_set_params (glitz_surface_t *surface,
                         glitz_filter_t filter,
                         glitz_fixed16_16_t *params,
                         int n_params)
{
  glitz_vec4_t *vecs;
  int i, size = 0;
  
  switch (filter) {
  case GLITZ_FILTER_CONVOLUTION: {
    double dm, dn, sum, tx, ty;
    int cx, cy, m, n, j;
    
    _glitz_filter_params_set (&dm, 3.0, &params, &n_params);
    _glitz_filter_params_set (&dn, 3.0, &params, &n_params);
    m = dm;
    n = dn;

    size = m * n;
    if (_glitz_filter_params_ensure (&surface->filter_params, size))
      return GLITZ_STATUS_NO_MEMORY;

    vecs = surface->filter_params->vectors;
    
    surface->filter_params->id = 0;
    
    /* center point is rounded down in case dimensions are not even */
    cx = m / 2;
    cy = n / 2;

    tx = surface->texture.texcoord_width / surface->texture.width;
    ty = surface->texture.texcoord_height / surface->texture.height;

    sum = 0.0;
    for (i = 0; i < m; i++) {
      glitz_vec4_t *vec;
      double weight;
      
      for (j = 0; j < n; j++) {
        _glitz_filter_params_set (&weight, 0.0, &params, &n_params);
        if (weight != 0.0) {
          vec = &vecs[surface->filter_params->id++];
          vec->v[0] = (i - cx) * tx;
          vec->v[1] = (cy - j) * ty;
          vec->v[2] = weight;
          sum += weight;
        }
      }
    }

    /* normalize */
    if (sum != 0.0)
      sum = 1.0 / sum;
    
    for (i = 0; i < surface->filter_params->id; i++)
      vecs[i].v[2] *= sum;
    
  } break;
  case GLITZ_FILTER_GAUSSIAN: {
    double radius, sigma, alpha, scale, xy_scale, sum, tx, ty;
    int half_size, x, y;
    
    _glitz_filter_params_set (&radius, 1.0, &params, &n_params);
    glitz_clamp_value (&radius, 0.0, 1024.0);

    _glitz_filter_params_set (&sigma, radius / 2.0, &params, &n_params);
    glitz_clamp_value (&sigma, 0.0, 1024.0);
    
    _glitz_filter_params_set (&alpha, radius, &params, &n_params);
    glitz_clamp_value (&alpha, 0.0, 1024.0);

    scale = 1.0 / (2.0 * GLITZ_PI * sigma * sigma);
    half_size = (int) (alpha * radius) / 2;

    if (half_size == 0)
      half_size = 1;
    
    size = half_size * 2 + 1;
    xy_scale = 2.0 * radius / size;
    
    if (_glitz_filter_params_ensure (&surface->filter_params, size * size))
      return GLITZ_STATUS_NO_MEMORY;

    vecs = surface->filter_params->vectors;

    surface->filter_params->id = 0;

    tx = surface->texture.texcoord_width / surface->texture.width;
    ty = surface->texture.texcoord_height / surface->texture.height;

    sum = 0.0;
    for (x = 0; x < size; x++) {
      glitz_vec4_t *vec;
      double fx, fy;
      double amp;
      
      fx = xy_scale * (x - half_size);
      
      for (y = 0; y < size; y++) {
        fy = xy_scale * (y - half_size);

        amp = scale * exp ((-1.0 * (fx * fx + fy * fy)) /
                           (2.0 * sigma * sigma));

        if (amp > 0.0) {
          vec = &vecs[surface->filter_params->id++];
          vec->v[0] = fx * tx;
          vec->v[1] = fy * ty;
          vec->v[2] = amp;
          sum += amp;
        }
      }
    }

    /* normalize */
    if (sum != 0.0)
      sum = 1.0 / sum;
    
    for (i = 0; i < surface->filter_params->id; i++)
      vecs[i].v[2] *= sum;
  } break;
  case GLITZ_FILTER_LINEAR_GRADIENT:
  case GLITZ_FILTER_RADIAL_GRADIENT:
    if (n_params <= 4) {
      if (surface->width == 1)
        size = surface->height;
      else if (surface->height == 1)
        size = surface->width;
    } else
      size = (n_params - 1) / 3;
    
    if (size < 2)
      size = 2;

    if (_glitz_filter_params_ensure (&surface->filter_params, size))
      return GLITZ_STATUS_NO_MEMORY;
    
    if (filter == GLITZ_FILTER_LINEAR_GRADIENT) {
      _glitz_filter_params_set (&surface->filter_params->u.linear.start.x,
                                0.0, &params, &n_params);
      _glitz_filter_params_set (&surface->filter_params->u.linear.start.y,
                                0.0, &params, &n_params);
      _glitz_filter_params_set (&surface->filter_params->u.linear.stop.x,
                                1.0, &params, &n_params);
      _glitz_filter_params_set (&surface->filter_params->u.linear.stop.y,
                                0.0, &params, &n_params);
    } else {
      double r0, r1;
      
      if (_glitz_filter_params_ensure (&surface->filter_params, size))
        return GLITZ_STATUS_NO_MEMORY;

      _glitz_filter_params_set (&surface->filter_params->u.radial.center.x,
                                0.5, &params, &n_params);
      _glitz_filter_params_set (&surface->filter_params->u.radial.center.y,
                                0.5, &params, &n_params);
      _glitz_filter_params_set (&r0, 0.0, &params, &n_params);
      _glitz_filter_params_set (&r1, 0.5, &params, &n_params);
      glitz_clamp_value (&r0, 0.0, r1);
      surface->filter_params->u.radial.radius_base = r0;
      if (r1 != r0)
        surface->filter_params->u.radial.radius_scale = 1.0 / (r1 - r0);
      else
        surface->filter_params->u.radial.radius_scale = 2147483647.0;
    }

    vecs = surface->filter_params->vectors;
    surface->filter_params->id = size;
    
    for (i = 0; i < size; i++) {
      double x_default, y_default, o_default;
      
      o_default = i / (double) (size - 1);
      x_default = 0.5 + (surface->width * i) / (double) size;
      y_default = 0.5 + (surface->height * i) / (double) size;
      
      _glitz_filter_params_set (&vecs[i].v[2], o_default, &params, &n_params);
      _glitz_filter_params_set (&vecs[i].v[0], x_default, &params, &n_params);
      _glitz_filter_params_set (&vecs[i].v[1], y_default, &params, &n_params);

      glitz_clamp_value (&vecs[i].v[2], 0.0, 1.0);
      glitz_clamp_value (&vecs[i].v[0], 0.5, surface->width - 0.5);
      glitz_clamp_value (&vecs[i].v[1], 0.5, surface->height - 0.5);
      
      glitz_texture_tex_coord (&surface->texture,
                               vecs[i].v[0], vecs[i].v[1],
                               &vecs[i].v[0], &vecs[i].v[1]);
      
      vecs[i].v[1] = surface->texture.texcoord_height - vecs[i].v[1];
      vecs[i].v[3] = i;
    }
    
    /* sort color stops in ascending order */
    qsort (vecs, surface->filter_params->id, sizeof (glitz_vec4_t),
	   _glitz_color_stop_compare);
    
    for (i = 0; i < size; i++) {
      double diff;

      if ((i + 1) == size)
        diff = 1.0 - vecs[i].v[2];
      else
        diff = vecs[i + 1].v[2] - vecs[i].v[2];
      
      if (diff != 0.0)
        vecs[i].v[3] = 1.0 / diff;
      else
        vecs[i].v[3] = 2147483647.0; /* should be DBL_MAX, but this will do */
    }
    break;
  case GLITZ_FILTER_BILINEAR:
  case GLITZ_FILTER_NEAREST:
    break;
  }

  glitz_filter_set_type (surface, filter);
    
  return GLITZ_STATUS_SUCCESS;
}

void
glitz_filter_params_destroy (glitz_filter_params_t *params)
{
  if (params->vectors)
    free (params->vectors);

  free (params);
}

glitz_gl_uint_t
glitz_filter_get_vertex_program (glitz_surface_t *surface,
                                 glitz_composite_op_t *op)
{
  return glitz_get_vertex_program (op);
}

glitz_gl_uint_t
glitz_filter_get_fragment_program (glitz_surface_t *surface,
                                   glitz_composite_op_t *op)
{
  return glitz_get_fragment_program (op,
                                     surface->filter_params->fp_type,
                                     surface->filter_params->id);
}

void
glitz_filter_set_type (glitz_surface_t *surface,
                       glitz_filter_t filter)
{
  switch (filter) {
  case GLITZ_FILTER_CONVOLUTION:
  case GLITZ_FILTER_GAUSSIAN:
    surface->filter_params->fp_type = GLITZ_FP_CONVOLUTION;
    break;
  case GLITZ_FILTER_LINEAR_GRADIENT:
    if (surface->hint_mask & GLITZ_INT_HINT_REPEAT_MASK) {
      if (SURFACE_MIRRORED (surface))
        surface->filter_params->fp_type = GLITZ_FP_LINEAR_GRADIENT_REFLECT;
      else
        surface->filter_params->fp_type = GLITZ_FP_LINEAR_GRADIENT_REPEAT;
    } else if (SURFACE_PAD (surface)) {
      surface->filter_params->fp_type = GLITZ_FP_LINEAR_GRADIENT_NEAREST;
    } else
      surface->filter_params->fp_type = GLITZ_FP_LINEAR_GRADIENT_TRANSPARENT;
    break;
  case GLITZ_FILTER_RADIAL_GRADIENT:
    if (surface->hint_mask & GLITZ_INT_HINT_REPEAT_MASK) {
      if (SURFACE_MIRRORED (surface))
        surface->filter_params->fp_type = GLITZ_FP_RADIAL_GRADIENT_REFLECT;
      else
        surface->filter_params->fp_type = GLITZ_FP_RADIAL_GRADIENT_REPEAT;
    } else if (SURFACE_PAD (surface)) {
      surface->filter_params->fp_type = GLITZ_FP_RADIAL_GRADIENT_NEAREST;
    } else
      surface->filter_params->fp_type = GLITZ_FP_RADIAL_GRADIENT_TRANSPARENT;
    break;
  case GLITZ_FILTER_BILINEAR:
  case GLITZ_FILTER_NEAREST:
    break;
  }
}

void
glitz_filter_enable (glitz_surface_t *surface,
                     glitz_composite_op_t *op)
{
  glitz_gl_proc_address_list_t *gl = op->gl;
  int i;

  gl->enable (GLITZ_GL_VERTEX_PROGRAM);
  gl->bind_program (GLITZ_GL_VERTEX_PROGRAM, op->vp);
  
  gl->enable (GLITZ_GL_FRAGMENT_PROGRAM);
  gl->bind_program (GLITZ_GL_FRAGMENT_PROGRAM, op->fp);

  switch (surface->filter) {
  case GLITZ_FILTER_GAUSSIAN:
  case GLITZ_FILTER_CONVOLUTION:
    for (i = 0; i < surface->filter_params->id; i++) {
      glitz_vec4_t *vec;
      
      vec = &surface->filter_params->vectors[i];
      
      gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, i,
                                  vec->v[0], vec->v[1], vec->v[2], 0.0);
    }
    break;
  case GLITZ_FILTER_LINEAR_GRADIENT: {
    glitz_linear_gradient_filter_t *linear = &surface->filter_params->u.linear;
    double length, angle, start, cos_angle, sin_angle, dw, dh;
    int j, fp_type = surface->filter_params->fp_type;
    
    dw = linear->stop.x - linear->start.x;
    dh = linear->stop.y - linear->start.y;
      
    length = sqrt (dw * dw + dh * dh);
      
    angle = -atan2 (dh, dw);

    cos_angle = cos (angle);
    sin_angle = sin (angle);
      
    start = cos_angle * linear->start.x;
    start += -sin_angle * linear->start.y;

    gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 0, start,
                                (length)? 1.0 / length: INT_MAX,
                                cos_angle, -sin_angle);

    if (fp_type == GLITZ_FP_LINEAR_GRADIENT_TRANSPARENT) {
      glitz_vec4_t *vec = surface->filter_params->vectors;
      
      gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 1,
                                  -1.0, -1.0, 0.0,
                                  (vec->v[3])? 1.0 / vec->v[3]: 1.0);
      j = 2;
    } else
      j = 1;
    
    for (i = 0; i < surface->filter_params->id; i++) {
      glitz_vec4_t *vec;
      
      vec = &surface->filter_params->vectors[i];
      
      gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, i + j,
                                  vec->v[0], vec->v[1], vec->v[2], vec->v[3]);
    }

    if (fp_type == GLITZ_FP_LINEAR_GRADIENT_TRANSPARENT)
      gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, i + j,
                                  -1.0, -1.0, 1.0, 1.0);
      
  } break;
  case GLITZ_FILTER_RADIAL_GRADIENT: {
    glitz_radial_gradient_filter_t *radial = &surface->filter_params->u.radial;
    int j, fp_type = surface->filter_params->fp_type;
    
    gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 0,
                                radial->center.x, radial->center.y,
                                radial->radius_base, radial->radius_scale);

    if (fp_type == GLITZ_FP_RADIAL_GRADIENT_TRANSPARENT) {
      glitz_vec4_t *vec = surface->filter_params->vectors;
      
      gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 1,
                                  -1.0, -1.0, 0.0,
                                  (vec->v[3]) ? 1.0 / vec->v[3]: 1.0);
      j = 2;
    } else
      j = 1;
    
    for (i = 0; i < surface->filter_params->id; i++) {
      glitz_vec4_t *vec;
      
      vec = &surface->filter_params->vectors[i];
      
      gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, i + j,
                                  vec->v[0], vec->v[1], vec->v[2], vec->v[3]);
    }

    if (fp_type == GLITZ_FP_RADIAL_GRADIENT_TRANSPARENT)
      gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, i + j,
                                  -1.0, -1.0, 1.0, 1.0);
  } break;
  case GLITZ_FILTER_BILINEAR:
  case GLITZ_FILTER_NEAREST:
    break;
  }
}
