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
 * Author: David Reveman <c99drn@cs.umu.se>
 */

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "glitzint.h"

glitz_render_type_t
glitz_render_type (glitz_surface_t *src,
                   glitz_surface_t *mask,
                   glitz_surface_t *dst)
{
  int src_conv, mask_conv;

  if (dst->feature_mask & GLITZ_FEATURE_CONVOLUTION_FILTER_MASK) {
    src_conv = (src->convolution)? 1: 0;
    mask_conv = (mask && mask->convolution)? 1: 0;
  } else
    src_conv = mask_conv = 0;
  
  if (!mask) {
    if (SURFACE_SOLID (src))
      return GLITZ_RENDER_TYPE_SOLID;
    else if ((!src_conv) && (!SURFACE_PROGRAMMATIC (src)))
      return GLITZ_RENDER_TYPE_ARGB;
    else if (SURFACE_PROGRAMMATIC (src) &&
             (dst->feature_mask & GLITZ_FEATURE_ARB_FRAGMENT_PROGRAM_MASK))
      return GLITZ_RENDER_TYPE_SRC_PROGRAMMATIC;
  }

  if ((!src_conv) &&
      mask && (!SURFACE_PROGRAMMATIC (mask)) && (!mask_conv)) {
    if ((mask->texture.internal_format == GLITZ_GL_LUMINANCE_ALPHA) &&
        (dst->feature_mask & GLITZ_FEATURE_ARB_MULTITEXTURE_MASK)) {
      if (SURFACE_SOLID (src))
        return GLITZ_RENDER_TYPE_SOLID_A;
      else if (!SURFACE_PROGRAMMATIC (src))
        return GLITZ_RENDER_TYPE_ARGB_A;
      else if (dst->feature_mask & GLITZ_FEATURE_ARB_FRAGMENT_PROGRAM_MASK)
        return GLITZ_RENDER_TYPE_SRC_PROGRAMMATIC;
    } else if (dst->feature_mask & GLITZ_FEATURE_ARB_FRAGMENT_PROGRAM_MASK) {
      if (!SURFACE_PROGRAMMATIC (src))
        return GLITZ_RENDER_TYPE_ARGB_ARGB;
      else
        return GLITZ_RENDER_TYPE_SRC_PROGRAMMATIC;
    }
  }
  
  if (src_conv) {
    if (mask && SURFACE_SOLID (mask)) {
      return GLITZ_RENDER_TYPE_SRC_CONVOLUTION_AND_SOLID_MASK;
    } else if ((!mask) || (!SURFACE_PROGRAMMATIC (mask)))
      return GLITZ_RENDER_TYPE_SRC_CONVOLUTION;
  }
    
  if (mask_conv) {
    if (SURFACE_SOLID (src)) {
      return GLITZ_RENDER_TYPE_MASK_CONVOLUTION_AND_SOLID_SRC;
    } else if (!SURFACE_PROGRAMMATIC (src))
      return GLITZ_RENDER_TYPE_MASK_CONVOLUTION;
  }
  
  return GLITZ_RENDER_TYPE_NOT_SUPPORTED;
}

void
glitz_render_enable (glitz_render_type_t type,
                     glitz_surface_t *src,
                     glitz_surface_t *mask,
                     glitz_surface_t *dst,
                     glitz_texture_t *src_texture,
                     glitz_texture_t *mask_texture,
                     unsigned short opacity)
{
  switch (type) {
  case GLITZ_RENDER_TYPE_ARGB:
    if (opacity != 0xffff) {
      dst->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV,
                          GLITZ_GL_TEXTURE_ENV_MODE,
                          GLITZ_GL_MODULATE);
      dst->gl->color_4us (opacity, opacity, opacity, opacity);
    } else
      dst->gl->tex_env_f (GLITZ_GL_TEXTURE_ENV,
                          GLITZ_GL_TEXTURE_ENV_MODE,
                          GLITZ_GL_REPLACE);
    break;
  case GLITZ_RENDER_TYPE_ARGB_ARGB:
    glitz_program_enable_argb_argb (dst->gl, dst->programs,
                                    src_texture, mask_texture);
    break;
  case GLITZ_RENDER_TYPE_SOLID_A:
    glitz_programmatic_surface_bind (dst->gl,
                                     (glitz_programmatic_surface_t *) src,
                                     dst->feature_mask,
                                     0xffff);
    break;
  case GLITZ_RENDER_TYPE_ARGB_A:
    break;
  case GLITZ_RENDER_TYPE_SOLID:
    glitz_programmatic_surface_bind (dst->gl,
                                     (glitz_programmatic_surface_t *) src,
                                     dst->feature_mask,
                                     opacity);
    break;
  case GLITZ_RENDER_TYPE_SRC_CONVOLUTION:
  case GLITZ_RENDER_TYPE_SRC_CONVOLUTION_AND_SOLID_MASK:
    glitz_program_enable_convolution (dst->gl, dst->programs,
                                      src, mask, src_texture, mask_texture,
                                      GLITZ_PROGRAM_SRC_OPERATION_OFFSET, 1,
                                      opacity);
    break;
  case GLITZ_RENDER_TYPE_MASK_CONVOLUTION:
    glitz_program_enable_convolution (dst->gl, dst->programs,
                                      src, mask, src_texture, mask_texture,
                                      GLITZ_PROGRAM_MASK_OPERATION_OFFSET, 0,
                                      0xffff);
    break;
  case GLITZ_RENDER_TYPE_MASK_CONVOLUTION_AND_SOLID_SRC:
    glitz_program_enable_convolution (dst->gl, dst->programs,
                                      src, mask, src_texture, mask_texture,
                                      GLITZ_PROGRAM_MASK_OPERATION_OFFSET, 2,
                                      0xffff);
    break;
  case GLITZ_RENDER_TYPE_SRC_PROGRAMMATIC:
    glitz_program_enable_programmatic (dst,
                                       (glitz_programmatic_surface_t *) src,
                                       src_texture, mask_texture,
                                       GLITZ_PROGRAM_SRC_OPERATION_OFFSET,
                                       opacity);
    break;
  case GLITZ_RENDER_TYPE_MASK_PROGRAMMATIC:
    glitz_program_enable_programmatic (dst,
                                       (glitz_programmatic_surface_t *) mask,
                                       src_texture, mask_texture,
                                       GLITZ_PROGRAM_MASK_OPERATION_OFFSET,
                                       0xffff);
    break;
  case GLITZ_RENDER_TYPE_NOT_SUPPORTED:
    break;
  }
}

void
glitz_render_disable (glitz_render_type_t type,
                      glitz_surface_t *dst)
{
  switch (type) {
  case GLITZ_RENDER_TYPE_SRC_PROGRAMMATIC:
  case GLITZ_RENDER_TYPE_MASK_PROGRAMMATIC:
    dst->gl->active_texture_arb (GLITZ_GL_TEXTURE2_ARB);
    dst->gl->bind_texture (GLITZ_GL_TEXTURE_1D, 0);
    dst->gl->disable (GLITZ_GL_TEXTURE_1D);
    dst->gl->active_texture_arb (GLITZ_GL_TEXTURE0_ARB);
    /* fall-through */
  case GLITZ_RENDER_TYPE_SRC_CONVOLUTION:
  case GLITZ_RENDER_TYPE_SRC_CONVOLUTION_AND_SOLID_MASK:
  case GLITZ_RENDER_TYPE_MASK_CONVOLUTION:
  case GLITZ_RENDER_TYPE_MASK_CONVOLUTION_AND_SOLID_SRC:
    dst->gl->bind_program_arb (GLITZ_GL_FRAGMENT_PROGRAM_ARB, 0);
    dst->gl->disable (GLITZ_GL_FRAGMENT_PROGRAM_ARB);
    dst->gl->bind_program_arb (GLITZ_GL_VERTEX_PROGRAM_ARB, 0);
    dst->gl->disable (GLITZ_GL_VERTEX_PROGRAM_ARB);
    break;
  default:
    break;
  }
}
