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

static void
_glitz_render_argb_solid (glitz_render_op_t *op)
{
  if (op->alpha_mask.alpha != 0xffff) {
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
                       GLITZ_GL_MODULATE);
    op->gl->color_4us (op->alpha_mask.alpha,
                       op->alpha_mask.alpha,
                       op->alpha_mask.alpha,
                       op->alpha_mask.alpha);
  } else {
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
                       GLITZ_GL_REPLACE);
    op->gl->color_4us (0x0, 0x0, 0x0, 0xffff);
  }
}

static void
_glitz_render_argb_argb (glitz_render_op_t *op)
{
  op->gl->active_texture (GLITZ_GL_TEXTURE0);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
                     GLITZ_GL_REPLACE);  
  op->gl->color_4us (0x0, 0x0, 0x0, 0xffff);
  
  op->gl->active_texture (GLITZ_GL_TEXTURE1);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
                     GLITZ_GL_COMBINE);
  
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_COMBINE_RGB,
                     GLITZ_GL_MODULATE);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE0_RGB,
                     GLITZ_GL_TEXTURE);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE1_RGB,
                     GLITZ_GL_PREVIOUS);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND0_RGB,
                     GLITZ_GL_SRC_COLOR);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND1_RGB,
                     GLITZ_GL_SRC_ALPHA);
  
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_COMBINE_ALPHA,
                     GLITZ_GL_MODULATE);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE0_ALPHA,
                     GLITZ_GL_TEXTURE);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE1_ALPHA,
                     GLITZ_GL_PREVIOUS);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND0_ALPHA,
                     GLITZ_GL_SRC_ALPHA);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND1_ALPHA,
                     GLITZ_GL_SRC_ALPHA);
}

static void
_setup_x_argbc (glitz_render_op_t *op)
{
  op->gl->active_texture (GLITZ_GL_TEXTURE0);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
                     GLITZ_GL_COMBINE);
    
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_COMBINE_RGB,
                     GLITZ_GL_INTERPOLATE);

  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE0_RGB,
                     GLITZ_GL_TEXTURE);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE1_RGB,
                     GLITZ_GL_PRIMARY_COLOR);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE2_RGB,
                     GLITZ_GL_PRIMARY_COLOR);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND0_RGB,
                     GLITZ_GL_SRC_COLOR);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND1_RGB,
                     GLITZ_GL_SRC_COLOR);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND2_RGB,
                     GLITZ_GL_SRC_ALPHA);

  /* we don't care about the alpha channel, so lets do something (simple?) */
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_COMBINE_ALPHA,
                     GLITZ_GL_REPLACE);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE0_ALPHA,
                     GLITZ_GL_PRIMARY_COLOR);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND0_ALPHA,
                     GLITZ_GL_SRC_ALPHA);

    
  op->gl->active_texture (GLITZ_GL_TEXTURE1);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
                     GLITZ_GL_COMBINE);
  
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_COMBINE_RGB,
                     GLITZ_GL_DOT3_RGBA);
    
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE0_RGB,
                     GLITZ_GL_PREVIOUS);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE1_RGB,
                     GLITZ_GL_PRIMARY_COLOR);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND0_RGB,
                     GLITZ_GL_SRC_COLOR);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND1_RGB,
                     GLITZ_GL_SRC_COLOR);
}

static void
_glitz_render_argb_argbc (glitz_render_op_t *op)
{
  if (op->count == 0) {
    _setup_x_argbc (op);
    
    op->gl->active_texture (GLITZ_GL_TEXTURE2);
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
                       GLITZ_GL_MODULATE);
  }
    
  if (op->alpha_mask.red) {
    op->gl->color_4d (1.0, 0.5, 0.5, 0.5);
  } else if (op->alpha_mask.green) {
    op->gl->color_4d (0.5, 1.0, 0.5, 0.5);
  } else if (op->alpha_mask.blue) {
    op->gl->color_4d (0.5, 0.5, 1.0, 0.5);
  } else {
    op->gl->active_texture (GLITZ_GL_TEXTURE0);
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
                       GLITZ_GL_REPLACE);  
    op->gl->color_4us (0x0, 0x0, 0x0, 0xffff);
    
    op->gl->active_texture (GLITZ_GL_TEXTURE1);
    glitz_texture_unbind (op->gl, op->src_texture);
    
    op->gl->active_texture (GLITZ_GL_TEXTURE2);
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
                       GLITZ_GL_MODULATE);
  }
}


static void
_glitz_render_solid_solid (glitz_render_op_t *op)
{
  op->gl->color_4us (SHORT_MULT (op->solid->red, op->alpha_mask.alpha),
                     SHORT_MULT (op->solid->green, op->alpha_mask.alpha),
                     SHORT_MULT (op->solid->blue, op->alpha_mask.alpha),
                     SHORT_MULT (op->solid->alpha, op->alpha_mask.alpha));
}

static void
_glitz_render_solid_argb (glitz_render_op_t *op)
{ 
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
                     GLITZ_GL_COMBINE);
  
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_COMBINE_RGB,
                     GLITZ_GL_MODULATE);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE0_RGB,
                     GLITZ_GL_TEXTURE);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE1_RGB,
                     GLITZ_GL_PRIMARY_COLOR);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND0_RGB,
                     GLITZ_GL_SRC_ALPHA);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND1_RGB,
                     GLITZ_GL_SRC_COLOR);

  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_COMBINE_ALPHA,
                     GLITZ_GL_MODULATE);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE0_ALPHA,
                     GLITZ_GL_TEXTURE);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE1_ALPHA,
                     GLITZ_GL_PRIMARY_COLOR);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND0_ALPHA,
                     GLITZ_GL_SRC_ALPHA);
  op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND1_ALPHA,
                     GLITZ_GL_SRC_ALPHA);
                       
  op->gl->color_4us (op->solid->red,
                     op->solid->green,
                     op->solid->blue,
                     op->solid->alpha);
}

static void
_glitz_render_solid_argbc (glitz_render_op_t *op)
{
  if (op->count == 0) {
    glitz_gl_float_t color[4];
    
    _setup_x_argbc (op);
    
    op->gl->active_texture (GLITZ_GL_TEXTURE2);
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
                       GLITZ_GL_COMBINE);
  
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_COMBINE_RGB,
                       GLITZ_GL_MODULATE);
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE0_RGB,
                       GLITZ_GL_PREVIOUS);
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE1_RGB,
                       GLITZ_GL_CONSTANT);
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND0_RGB,
                       GLITZ_GL_SRC_COLOR);
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND1_RGB,
                       GLITZ_GL_SRC_COLOR);

    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_COMBINE_ALPHA,
                       GLITZ_GL_MODULATE);
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE0_ALPHA,
                       GLITZ_GL_PREVIOUS);
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_SOURCE1_ALPHA,
                       GLITZ_GL_CONSTANT);
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND0_ALPHA,
                       GLITZ_GL_SRC_ALPHA);
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_OPERAND1_ALPHA,
                       GLITZ_GL_SRC_ALPHA);

    color[0] = (double) op->solid->red / 65536.0;
    color[1] = (double) op->solid->green / 65536.0;
    color[2] = (double) op->solid->blue / 65536.0;
    color[3] = (double) op->solid->alpha / 65536.0;
    
    op->gl->tex_env_fv (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_COLOR,
                        color);
  }
    
  if (op->alpha_mask.red) {
    op->gl->color_4d (1.0, 0.5, 0.5, 0.5);
  } else if (op->alpha_mask.green) {
    op->gl->color_4d (0.5, 1.0, 0.5, 0.5);
  } else if (op->alpha_mask.blue) {
    op->gl->color_4d (0.5, 0.5, 1.0, 0.5);
  } else {
    op->gl->active_texture (GLITZ_GL_TEXTURE0);
    op->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
                       GLITZ_GL_MODULATE);
    op->gl->color_4us (op->solid->red,
                       op->solid->green,
                       op->solid->blue,
                       op->solid->alpha);
    
    op->gl->active_texture (GLITZ_GL_TEXTURE1);
    glitz_texture_unbind (op->gl, op->src_texture);
    
    op->gl->active_texture (GLITZ_GL_TEXTURE2);
    glitz_texture_unbind (op->gl, op->src_texture);
  }
}

static void
_glitz_render_argbf (glitz_render_op_t *op)
{
  if (op->count == 0) {
    glitz_gl_uint_t vertex_program, fragment_program;

    vertex_program = glitz_get_vertex_program (op);
    fragment_program = glitz_get_fragment_program (op);
  
    if (vertex_program && fragment_program) {
      glitz_texture_t *texture = op->src_texture;
      glitz_surface_t *surface = op->src;

      op->gl->enable (GLITZ_GL_VERTEX_PROGRAM);
      op->gl->bind_program (GLITZ_GL_VERTEX_PROGRAM, vertex_program);
      op->gl->program_local_param_4d (GLITZ_GL_VERTEX_PROGRAM, 0,
                                      texture->texcoord_width /
                                      (double) texture->width,
                                      0.000, 0.0, 0.0);
      op->gl->program_local_param_4d (GLITZ_GL_VERTEX_PROGRAM, 1,
                                      0.000,
                                      texture->texcoord_height /
                                      (double) texture->height,
                                      0.0, 0.0);
    
      op->gl->enable (GLITZ_GL_FRAGMENT_PROGRAM);
      op->gl->bind_program (GLITZ_GL_FRAGMENT_PROGRAM, fragment_program);
      op->gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 0,
                                      surface->convolution->m[0][0],
                                      surface->convolution->m[0][1],
                                      surface->convolution->m[0][2], 0.0);
      op->gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 1,
                                      surface->convolution->m[1][0],
                                      surface->convolution->m[1][1],
                                      surface->convolution->m[1][2], 0.0);
      op->gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 2,
                                      surface->convolution->m[2][0],
                                      surface->convolution->m[2][1],
                                      surface->convolution->m[2][2], 0.0);
    }

    op->gl->color_4us (op->alpha_mask.red,
                       op->alpha_mask.green,
                       op->alpha_mask.blue,
                       op->alpha_mask.alpha);
  }
}

static void
_glitz_end_f (glitz_render_op_t *op)
{
  op->gl->bind_program (GLITZ_GL_FRAGMENT_PROGRAM, 0);
  op->gl->disable (GLITZ_GL_FRAGMENT_PROGRAM);
  op->gl->bind_program (GLITZ_GL_VERTEX_PROGRAM, 0);
  op->gl->disable (GLITZ_GL_VERTEX_PROGRAM);
}

static void
_glitz_render_lgrad (glitz_render_op_t *op)
{
  if (op->count == 0) {
    glitz_gl_uint_t fragment_program;

    fragment_program = glitz_get_fragment_program (op);
  
    if (fragment_program) {
      glitz_programmatic_surface_t *surface =
        (glitz_programmatic_surface_t *) op->src;
      glitz_point_t p1, p2;
      double length, angle, start;
    
      op->gl->enable (GLITZ_GL_FRAGMENT_PROGRAM);
      op->gl->bind_program (GLITZ_GL_FRAGMENT_PROGRAM, fragment_program);

      p1.x = FIXED_TO_DOUBLE (surface->u.linear.start.x);
      p1.y = FIXED_TO_DOUBLE (surface->u.linear.start.y);
      p2.x = FIXED_TO_DOUBLE (surface->u.linear.stop.x);
      p2.y = FIXED_TO_DOUBLE (surface->u.linear.stop.y);
    
      length = sqrt ((p2.x - p1.x) * (p2.x - p1.x) +
                     (p2.y - p1.y) * (p2.y - p1.y));
      
      angle = -atan2 (p2.y - p1.y, p2.x - p1.x);
      
      start = cos (angle) * p1.x;
      start += -sin (angle) * p1.y;
      
      op->gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 0, start,
                                      (length)? 1.0 / length: INT_MAX,
                                      cos (angle), -sin (angle));
      op->gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 1,
                                      surface->matrix.m[0][0],
                                      surface->matrix.m[0][1],
                                      surface->matrix.m[1][0],
                                      surface->matrix.m[1][1]);
      op->gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 2,
                                      surface->matrix.m[2][0],
                                      surface->matrix.m[2][1],
                                      surface->base.height, 0.0);
      
      op->gl->active_texture (GLITZ_GL_TEXTURE2);
      glitz_color_range_bind (op->gl, surface->u.linear.color_range,
                              op->dst->feature_mask);
      op->gl->active_texture (GLITZ_GL_TEXTURE0);
    }
  }

  op->gl->color_4us (op->alpha_mask.red,
                     op->alpha_mask.green,
                     op->alpha_mask.blue,
                     op->alpha_mask.alpha);
}

static void
_glitz_render_rgrad (glitz_render_op_t *op)
{
  if (op->count == 0) {
    glitz_gl_uint_t fragment_program;

    fragment_program = glitz_get_fragment_program (op);
  
    if (fragment_program) {
      glitz_programmatic_surface_t *surface =
        (glitz_programmatic_surface_t *) op->src;
      
      op->gl->enable (GLITZ_GL_FRAGMENT_PROGRAM);
      op->gl->bind_program (GLITZ_GL_FRAGMENT_PROGRAM, fragment_program);

      op->gl->program_local_param_4d
        (GLITZ_GL_FRAGMENT_PROGRAM, 0,
         FIXED_TO_DOUBLE (surface->u.radial.center.x),
         FIXED_TO_DOUBLE (surface->u.radial.center.y),
         1.0 / (FIXED_TO_DOUBLE (surface->u.radial.radius1) -
                FIXED_TO_DOUBLE (surface->u.radial.radius0)),
         FIXED_TO_DOUBLE (surface->u.radial.radius0));
      op->gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 1,
                                      surface->matrix.m[0][0],
                                      surface->matrix.m[0][1],
                                      surface->matrix.m[1][0],
                                      surface->matrix.m[1][1]);
      op->gl->program_local_param_4d (GLITZ_GL_FRAGMENT_PROGRAM, 2,
                                      surface->matrix.m[2][0],
                                      surface->matrix.m[2][1],
                                      surface->base.height, 0.0);
      
      op->gl->active_texture (GLITZ_GL_TEXTURE2);
      glitz_color_range_bind (op->gl, surface->u.radial.color_range,
                              op->dst->feature_mask);
      op->gl->active_texture (GLITZ_GL_TEXTURE0);
    }
  }

  op->gl->color_4us (op->alpha_mask.red,
                     op->alpha_mask.green,
                     op->alpha_mask.blue,
                     op->alpha_mask.alpha);
}

static void
_glitz_end_grad (glitz_render_op_t *op)
{
  op->gl->active_texture (GLITZ_GL_TEXTURE2);
  op->gl->bind_texture (GLITZ_GL_TEXTURE_1D, 0);
  op->gl->disable (GLITZ_GL_TEXTURE_1D);
  op->gl->active_texture (GLITZ_GL_TEXTURE0);
  op->gl->bind_program (GLITZ_GL_FRAGMENT_PROGRAM, 0);
  op->gl->disable (GLITZ_GL_FRAGMENT_PROGRAM);
}

static glitz_render_t
_glitz_render_map[GLITZ_SURFACE_TYPES][GLITZ_SURFACE_TYPES] = {
  {
    { GLITZ_RENDER_TYPE_NA, NULL, NULL, 0 },
    { GLITZ_RENDER_TYPE_NA, NULL, NULL, 0 },
    { GLITZ_RENDER_TYPE_NA, NULL, NULL, 0 },
    { GLITZ_RENDER_TYPE_NA, NULL, NULL, 0 },
    { GLITZ_RENDER_TYPE_NA, NULL, NULL, 0 },
    { GLITZ_RENDER_TYPE_NA, NULL, NULL, 0 },
    { GLITZ_RENDER_TYPE_NA, NULL, NULL, 0 }
  }, {
    { GLITZ_RENDER_TYPE_ARGB,       _glitz_render_argb_solid, NULL, 1 },
    { GLITZ_RENDER_TYPE_ARGB_ARGB,  _glitz_render_argb_argb,  NULL, 2 },
    { GLITZ_RENDER_TYPE_ARGB_ARGBC, _glitz_render_argb_argbc, NULL, 3 },
    { GLITZ_RENDER_TYPE_NA,         NULL,                     NULL, 0 },
    { GLITZ_RENDER_TYPE_ARGB_SOLID, _glitz_render_argb_solid, NULL, 1 },
    { GLITZ_RENDER_TYPE_NA,         NULL,                     NULL, 0 },
    { GLITZ_RENDER_TYPE_NA,         NULL,                     NULL, 0 }
  }, {
    { GLITZ_RENDER_TYPE_ARGB,       _glitz_render_argb_solid, NULL, 1 },
    { GLITZ_RENDER_TYPE_ARGB_ARGB,  _glitz_render_argb_argb,  NULL, 2 },
    { GLITZ_RENDER_TYPE_ARGB_ARGBC, _glitz_render_argb_argbc, NULL, 3 },
    { GLITZ_RENDER_TYPE_NA,         NULL,                     NULL, 0 },
    { GLITZ_RENDER_TYPE_ARGB_SOLID, _glitz_render_argb_solid, NULL, 1 },
    { GLITZ_RENDER_TYPE_NA,         NULL,                     NULL, 0 },
    { GLITZ_RENDER_TYPE_NA,         NULL,                     NULL, 0 }
  }, {
    { GLITZ_RENDER_TYPE_ARGBF,       _glitz_render_argbf, _glitz_end_f, 1 },
    { GLITZ_RENDER_TYPE_ARGBF_ARGB,  _glitz_render_argbf, _glitz_end_f, 2 },
    { GLITZ_RENDER_TYPE_ARGBF_ARGBC, _glitz_render_argbf, _glitz_end_f, 2 },
    { GLITZ_RENDER_TYPE_NA,          NULL,                NULL,         0 },
    { GLITZ_RENDER_TYPE_ARGBF_SOLID, _glitz_render_argbf, _glitz_end_f, 1 },
    { GLITZ_RENDER_TYPE_NA,          NULL,                NULL,         0 },
    { GLITZ_RENDER_TYPE_NA,          NULL,                NULL,         0 }
  }, {
    { GLITZ_RENDER_TYPE_SOLID,       _glitz_render_solid_solid, NULL, 0 },
    { GLITZ_RENDER_TYPE_SOLID_ARGB,  _glitz_render_solid_argb,  NULL, 1 },
    { GLITZ_RENDER_TYPE_SOLID_ARGBC, _glitz_render_solid_argbc, NULL, 3 },
    { GLITZ_RENDER_TYPE_NA,          NULL,                      NULL, 0 },
    { GLITZ_RENDER_TYPE_SOLID_SOLID, _glitz_render_solid_solid, NULL, 0 },
    { GLITZ_RENDER_TYPE_NA,          NULL,                      NULL, 0 },
    { GLITZ_RENDER_TYPE_NA,          NULL,                      NULL, 0 }
  }, {
    { GLITZ_RENDER_TYPE_LGRAD,       _glitz_render_lgrad, _glitz_end_grad, 0 },
    { GLITZ_RENDER_TYPE_LGRAD_ARGB,  _glitz_render_lgrad, _glitz_end_grad, 2 },
    { GLITZ_RENDER_TYPE_LGRAD_ARGBC, _glitz_render_lgrad, _glitz_end_grad, 2 },
    { GLITZ_RENDER_TYPE_NA,          NULL,                NULL,            0 },
    { GLITZ_RENDER_TYPE_LGRAD_SOLID, _glitz_render_lgrad, _glitz_end_grad, 1 },
    { GLITZ_RENDER_TYPE_NA,          NULL,                NULL,            0 },
    { GLITZ_RENDER_TYPE_NA,          NULL,                NULL,            0 }
  }, {
    { GLITZ_RENDER_TYPE_RGRAD,       _glitz_render_rgrad, _glitz_end_grad, 0 },
    { GLITZ_RENDER_TYPE_RGRAD_ARGB,  _glitz_render_rgrad, _glitz_end_grad, 2 },
    { GLITZ_RENDER_TYPE_RGRAD_ARGBC, _glitz_render_rgrad, _glitz_end_grad, 2 },
    { GLITZ_RENDER_TYPE_NA,          NULL,                NULL,            0 },
    { GLITZ_RENDER_TYPE_RGRAD_SOLID, _glitz_render_rgrad, _glitz_end_grad, 1 },
    { GLITZ_RENDER_TYPE_NA,          NULL,                NULL,            0 },
    { GLITZ_RENDER_TYPE_NA,          NULL,                NULL,            0 }
  }
};
  
#define MANUAL_REPEAT(surface) \
  (((surface)->hint_mask & GLITZ_INT_HINT_REPEAT_MASK) && \
   (!(surface)->texture.repeatable))

#define ROTATING_TRANSFORM(surface) \
  ((surface)->transform && \
   (((surface)->transform->m[0][1] != 0.0) || \
   ((surface)->transform->m[1][0] != 0.0)))

#define SIMPLE_SURFACE(surface) \
  ((!MANUAL_REPEAT (surface)) && (!ROTATING_TRANSFORM (surface)))

#define MULTI_TEXTURE(feature_mask, src, mask) \
  (((feature_mask) & GLITZ_FEATURE_ARB_MULTITEXTURE_MASK) && \
   (SIMPLE_SURFACE (src) && SIMPLE_SURFACE (mask)))

static glitz_surface_type_t
_glitz_get_surface_type (unsigned long feature_mask,
                         glitz_surface_t *surface)
{
  if (surface == NULL)
    return GLITZ_SURFACE_TYPE_NULL;
  
  if (SURFACE_PROGRAMMATIC (surface)) {
    if (SURFACE_SOLID (surface))
      return GLITZ_SURFACE_TYPE_SOLID;
    else if (feature_mask & GLITZ_FEATURE_ARB_FRAGMENT_PROGRAM_MASK) {
      if (SURFACE_LINEAR_GRADIENT (surface))
        return GLITZ_SURFACE_TYPE_LGRAD;
      else
        return GLITZ_SURFACE_TYPE_RGRAD;
    }
  } else {
    if (!surface->convolution) {
      if (SURFACE_COMPONENT_ALPHA (surface)) {
        if (feature_mask & GLITZ_FEATURE_COMPONENT_ALPHA_MASK)
          return GLITZ_SURFACE_TYPE_ARGBC;
      } else
        return GLITZ_SURFACE_TYPE_ARGB;
      
    } else if (feature_mask & GLITZ_FEATURE_CONVOLUTION_FILTER_MASK)
      return GLITZ_SURFACE_TYPE_ARGBF;
  }
  
  return GLITZ_SURFACE_TYPE_NA;
}

static glitz_color_t _default_alpha_mask = {
  0x0000, 0x0000, 0x0000, 0xffff
};

void
glitz_render_op_init (glitz_render_op_t *op,
                      glitz_surface_t **src,
                      glitz_surface_t **mask,
                      glitz_surface_t *dst,
                      int *x_src,
                      int *y_src,
                      int *x_mask,
                      int *y_mask)
{
  glitz_surface_type_t src_type;
  glitz_surface_type_t mask_type;
  glitz_render_t *render;

  op->type = GLITZ_RENDER_TYPE_NA;
  op->render = NULL;
  op->alpha_mask = _default_alpha_mask;
  op->gl = dst->gl;
  op->dst = dst;
  op->src_texture = op->mask_texture = NULL;
  op->count = 0;
  op->src = *src;
  op->mask = *mask;
  op->solid = NULL;
  op->component_alpha = GLITZ_COMPONENT_ALPHA_NONE;
  
  src_type = _glitz_get_surface_type (dst->feature_mask, *src);
  if (src_type < 1)
    return;

  mask_type = _glitz_get_surface_type (dst->feature_mask, *mask);
  if (mask_type < 0)
    return;

  render = &_glitz_render_map[src_type][mask_type];
  if (render->type == GLITZ_RENDER_TYPE_NA) {
    if (dst->feature_mask & GLITZ_FEATURE_OFFSCREEN_DRAWING_MASK)
      op->type = GLITZ_RENDER_TYPE_INTERMEDIATE;
    
    return;
  }

  if (mask_type == GLITZ_SURFACE_TYPE_SOLID) {
    op->alpha_mask.alpha =
      ((glitz_programmatic_surface_t *) (op->mask))->u.solid.color.alpha;
    *mask = NULL;
  }

  if (src_type == GLITZ_SURFACE_TYPE_SOLID) {
    op->solid = &((glitz_programmatic_surface_t *) (op->src))->u.solid.color;

    /* mask becomes source */
    if (*mask) {
      *src = *mask;
      *mask = NULL;
      *x_src = *x_mask;
      *y_src = *y_mask;
    }
  }
  
  if (mask_type != GLITZ_SURFACE_TYPE_NULL &&
      mask_type != GLITZ_SURFACE_TYPE_SOLID) {
    if (dst->feature_mask & GLITZ_FEATURE_ARB_TEXTURE_ENV_COMBINE_MASK) {
      if (mask_type == GLITZ_SURFACE_TYPE_ARGBC) {
        if (op->mask->format->alpha_size)
          op->component_alpha = GLITZ_COMPONENT_ALPHA_ARGB;
        else
          op->component_alpha = GLITZ_COMPONENT_ALPHA_RGB;
      }
      
      if (src_type != GLITZ_SURFACE_TYPE_SOLID) {
        if (MULTI_TEXTURE (dst->feature_mask, *src, *mask))
          op->render = render;
        else if ((dst->feature_mask & GLITZ_FEATURE_OFFSCREEN_DRAWING_MASK) &&
                 (!SURFACE_COMPONENT_ALPHA (*mask)))
          op->type = GLITZ_RENDER_TYPE_INTERMEDIATE;
        
      } else
        op->render = render;
    }
  } else
    op->render = render;

  /* update source and mask */
  op->src = *src;
  op->mask = *mask;

  if (op->render == render)
    op->type = render->type;
}

void
glitz_render_op_set_textures (glitz_render_op_t *op,
                              glitz_texture_t *src,
                              glitz_texture_t *mask)
{
  op->src_texture = src;
  op->mask_texture = mask;
}

void
glitz_render_op_set_alpha_mask (glitz_render_op_t *op,
                                unsigned short red,
                                unsigned short green,
                                unsigned short blue,
                                unsigned short alpha)
{
  op->alpha_mask.red = red;
  op->alpha_mask.green = green;
  op->alpha_mask.blue = blue;
  op->alpha_mask.alpha = alpha;
}

void
glitz_render_op_get_alpha_mask (glitz_render_op_t *op,
                                unsigned short *red,
                                unsigned short *green,
                                unsigned short *blue,
                                unsigned short *alpha)
{
  if (red) *red = op->alpha_mask.red;
  if (green) *green = op->alpha_mask.green;
  if (blue) *blue = op->alpha_mask.blue;
  if (alpha) *alpha = op->alpha_mask.alpha;
}

void
glitz_render_enable (glitz_render_op_t *op)
{
  op->render->enable (op);
  op->count++;
}

void
glitz_render_disable (glitz_render_op_t *op)
{
  if (op->render->disable)
    op->render->disable (op);
}
