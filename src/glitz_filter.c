/*
 * Copyright � 2004 David Reveman
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

struct _glitz_filter_params_t {
  int fp_type;
  int id;  
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
_glitz_filter_params_set (glitz_float_t *value,
                          const glitz_float_t default_value,
                          glitz_fixed16_16_t **params,
                          int *n_params)
{
  if (*n_params > 0) {
    *value = FIXED_TO_FLOAT (**params);
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
    glitz_float_t dm, dn, sum;
    int cx, cy, m, n, j;
    
    _glitz_filter_params_set (&dm, 3.0f, &params, &n_params);
    _glitz_filter_params_set (&dn, 3.0f, &params, &n_params);
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

    sum = 0.0f;
    for (i = 0; i < m; i++) {
      glitz_vec4_t *vec;
      glitz_float_t weight;
      
      for (j = 0; j < n; j++) {
        _glitz_filter_params_set (&weight, 0.0f, &params, &n_params);
        if (weight != 0.0f) {
          vec = &vecs[surface->filter_params->id++];
          vec->v[0] = (i - cx) * surface->texture.texcoord_width_unit;
          vec->v[1] = (cy - j) * surface->texture.texcoord_height_unit;
          vec->v[2] = weight;
          vec->v[3] = 0.0f;
          sum += weight;
        }
      }
    }

    /* normalize */
    if (sum != 0.0f)
      sum = 1.0f / sum;
    
    for (i = 0; i < surface->filter_params->id; i++)
      vecs[i].v[2] *= sum;
    
  } break;
  case GLITZ_FILTER_GAUSSIAN: {
    glitz_float_t radius, sigma, alpha, scale, xy_scale, sum;
    int half_size, x, y;
    
    _glitz_filter_params_set (&radius, 1.0f, &params, &n_params);
    glitz_clamp_value (&radius, 0.0f, 1024.0f);

    _glitz_filter_params_set (&sigma, radius / 2.0f, &params, &n_params);
    glitz_clamp_value (&sigma, 0.0f, 1024.0f);
    
    _glitz_filter_params_set (&alpha, radius, &params, &n_params);
    glitz_clamp_value (&alpha, 0.0f, 1024.0f);

    scale = 1.0f / (2.0f * GLITZ_PI * sigma * sigma);
    half_size = alpha + 0.5f;

    if (half_size == 0)
      half_size = 1;
    
    size = half_size * 2 + 1;
    xy_scale = 2.0f * radius / size;

    if (_glitz_filter_params_ensure (&surface->filter_params, size * size))
      return GLITZ_STATUS_NO_MEMORY;

    vecs = surface->filter_params->vectors;

    surface->filter_params->id = 0;

    sum = 0.0f;
    for (x = 0; x < size; x++) {
      glitz_vec4_t *vec;
      glitz_float_t fx, fy, amp;
      
      fx = xy_scale * (x - half_size);
      
      for (y = 0; y < size; y++) {
        fy = xy_scale * (y - half_size);

        amp = scale * exp ((-1.0f * (fx * fx + fy * fy)) /
                           (2.0f * sigma * sigma));

        if (amp > 0.0f) {
          vec = &vecs[surface->filter_params->id++];
          vec->v[0] = fx * surface->texture.texcoord_width_unit;
          vec->v[1] = fy * surface->texture.texcoord_height_unit;
          vec->v[2] = amp;
          vec->v[3] = 0.0f;
          sum += amp;
        }
      }
    }

    /* normalize */
    if (sum != 0.0f)
      sum = 1.0f / sum;
    
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

    if (_glitz_filter_params_ensure (&surface->filter_params, size + 1))
      return GLITZ_STATUS_NO_MEMORY;

    vecs = surface->filter_params->vectors;
    
    if (filter == GLITZ_FILTER_LINEAR_GRADIENT) {
      glitz_float_t length, angle, dh, dv;
      glitz_point_t start, stop;

      _glitz_filter_params_set (&start.x, 0.0f, &params, &n_params);
      _glitz_filter_params_set (&start.y, 0.0f, &params, &n_params);
      _glitz_filter_params_set (&stop.x, 1.0f, &params, &n_params);
      _glitz_filter_params_set (&stop.y, 0.0f, &params, &n_params);

      dh = stop.x - start.x;
      dv = stop.y - start.y;

      length = sqrt (dh * dh + dv * dv);

      angle = -atan2 (dv, dh);

      vecs->v[2] = cos (angle);
      vecs->v[3] = -sin (angle);
      
      vecs->v[0] = vecs->v[2] * start.x;
      vecs->v[0] += vecs->v[3] * start.y;

      vecs->v[1] = (length)? 1.0f / length: 2147483647.0f;
    } else {
      glitz_float_t r0, r1;
      
      _glitz_filter_params_set (&vecs->v[0], 0.5f, &params, &n_params);
      _glitz_filter_params_set (&vecs->v[1], 0.5f, &params, &n_params);
      _glitz_filter_params_set (&r0, 0.0f, &params, &n_params);
      _glitz_filter_params_set (&r1, 0.5f, &params, &n_params);
      
      glitz_clamp_value (&r0, 0.0f, r1);
      
      vecs->v[2] = r0;
      if (r1 != r0)
        vecs->v[3] = 1.0f / (r1 - r0);
      else
        vecs->v[3] = 2147483647.0f;
    }

    vecs++;
    surface->filter_params->id = size;
    
    for (i = 0; i < size; i++) {
      glitz_float_t x_default, y_default, o_default;
      
      o_default = i / (glitz_float_t) (size - 1);
      x_default = 0.5f + (surface->width * i) / (glitz_float_t) size;
      y_default = 0.5f + (surface->height * i) / (glitz_float_t) size;
      
      _glitz_filter_params_set (&vecs[i].v[2], o_default, &params, &n_params);
      _glitz_filter_params_set (&vecs[i].v[0], x_default, &params, &n_params);
      _glitz_filter_params_set (&vecs[i].v[1], y_default, &params, &n_params);

      glitz_clamp_value (&vecs[i].v[2], 0.0f, 1.0f);
      glitz_clamp_value (&vecs[i].v[0], 0.5f, surface->width - 0.5f);
      glitz_clamp_value (&vecs[i].v[1], 0.5f, surface->height - 0.5f);

      vecs[i].v[0] *= surface->texture.texcoord_width_unit;
      vecs[i].v[1] *= surface->texture.texcoord_height_unit;
      
      vecs[i].v[1] = surface->texture.box.y2 - vecs[i].v[1];
      vecs[i].v[3] = i;
    }
    
    /* sort color stops in ascending order */
    qsort (vecs, surface->filter_params->id, sizeof (glitz_vec4_t),
           _glitz_color_stop_compare);
    
    for (i = 0; i < size; i++) {
      glitz_float_t diff;

      if ((i + 1) == size)
        diff = 1.0f - vecs[i].v[2];
      else
        diff = vecs[i + 1].v[2] - vecs[i].v[2];
      
      if (diff != 0.0f)
        vecs[i].v[3] = 1.0f / diff;
      else
        vecs[i].v[3] = 2147483647.0f; /* should be FLT_MAX, but this will do */
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
    if (surface->flags & GLITZ_SURFACE_FLAG_REPEAT_MASK) {
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
    if (surface->flags & GLITZ_SURFACE_FLAG_REPEAT_MASK) {
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

  gl->enable (GLITZ_GL_FRAGMENT_PROGRAM);
  gl->bind_program (GLITZ_GL_FRAGMENT_PROGRAM, op->fp);

  switch (surface->filter) {
  case GLITZ_FILTER_GAUSSIAN:
  case GLITZ_FILTER_CONVOLUTION:
    for (i = 0; i < surface->filter_params->id; i++)
      gl->program_local_param_4fv (GLITZ_GL_FRAGMENT_PROGRAM, i,
                                   surface->filter_params->vectors[i].v);
    break;
  case GLITZ_FILTER_LINEAR_GRADIENT:
  case GLITZ_FILTER_RADIAL_GRADIENT: {
    int j, fp_type = surface->filter_params->fp_type;
    glitz_vec4_t *vec;

    vec = surface->filter_params->vectors;
    
    gl->program_local_param_4fv (GLITZ_GL_FRAGMENT_PROGRAM, 0, vec->v);

    vec++;

    if (fp_type == GLITZ_FP_LINEAR_GRADIENT_TRANSPARENT ||
        fp_type == GLITZ_FP_RADIAL_GRADIENT_TRANSPARENT) {
      glitz_vec4_t v;

      v.v[0] = v.v[1] = -1.0f;
      v.v[2] = 0.0f;
      v.v[3] = (vec->v[3])? 1.0f / vec->v[3]: 1.0f;
      
      gl->program_local_param_4fv (GLITZ_GL_FRAGMENT_PROGRAM, 1, v.v);
      j = 2;
    } else
      j = 1;
    
    for (i = 0; i < surface->filter_params->id; i++, vec++)
      gl->program_local_param_4fv (GLITZ_GL_FRAGMENT_PROGRAM, i + j, vec->v);
    
    if (fp_type == GLITZ_FP_LINEAR_GRADIENT_TRANSPARENT ||
        fp_type == GLITZ_FP_RADIAL_GRADIENT_TRANSPARENT) {
      glitz_vec4_t v;

      v.v[0] = v.v[1] = -1.0f;
      v.v[2] = v.v[3] = 1.0f;
      
      gl->program_local_param_4fv (GLITZ_GL_FRAGMENT_PROGRAM, i + j, v.v);
    } 
  } break;
  case GLITZ_FILTER_BILINEAR:
  case GLITZ_FILTER_NEAREST:
    break;
  }
}