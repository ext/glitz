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

static void
_glitz_texture_find_best_target (unsigned int width,
                                 unsigned int height,
                                 long int target_mask,
                                 unsigned int *target)
{
  *target = GLITZ_GL_TEXTURE_2D;

  if ((!(target_mask & GLITZ_TEXTURE_TARGET_2D_MASK)) ||
      (!glitz_uint_is_power_of_two (width)) ||
      (!glitz_uint_is_power_of_two (height))) {
    if (target_mask & GLITZ_TEXTURE_TARGET_RECTANGLE_MASK)
      *target = GLITZ_GL_TEXTURE_RECTANGLE_EXT;
  }
}

void
glitz_texture_init (glitz_gl_proc_address_list_t *gl,
                    glitz_texture_t *texture,
                    unsigned int width,
                    unsigned int height,
                    unsigned int texture_format,
                    unsigned long target_mask)
{
  texture->filter = -1;
  texture->repeat = -1;
  texture->width = width;
  texture->height = height;
  texture->format = texture_format;
  texture->allocated = 0;
  texture->name = 0;

  switch (texture->format) {
  case GLITZ_GL_LUMINANCE_ALPHA:
    texture->internal_format = GLITZ_GL_LUMINANCE_ALPHA;
    break;
  default:
    texture->internal_format = GLITZ_GL_RGBA;
    break;
  }

  if (!(target_mask & GLITZ_TEXTURE_TARGET_NPOT_MASK)) {
    _glitz_texture_find_best_target (width, height,
                                     target_mask, &texture->target);
    
    if (texture->target == GLITZ_GL_TEXTURE_2D) {
      glitz_uint_to_power_of_two (&texture->width);
      glitz_uint_to_power_of_two (&texture->height);
    }
  } else
    texture->target = GLITZ_GL_TEXTURE_2D;
  
  if (texture->target == GLITZ_GL_TEXTURE_2D &&
      texture->width == width && texture->height == height) {
    texture->repeatable = 1;
    texture->texcoord_width = texture->texcoord_height = 1.0;
  } else {
    texture->repeatable = 0;
    if (texture->target == GLITZ_GL_TEXTURE_2D) {
      texture->texcoord_width = (double) width / (double) texture->width;
      texture->texcoord_height = (double) height / (double) texture->height;
    } else {
      texture->texcoord_width = texture->width;
      texture->texcoord_height = texture->height;
    }
  }
}

static void
_glitz_texture_allocate (glitz_gl_proc_address_list_t *gl,
                         glitz_texture_t *texture)
{
  if (!texture->name)
    gl->gen_textures (1, &texture->name);

  texture->allocated = 1;

  glitz_texture_bind (gl, texture);

  gl->tex_image_2d (texture->target, 0,
                    texture->internal_format,
                    texture->width, texture->height,
                    0, texture->format, GLITZ_GL_UNSIGNED_BYTE, NULL);

  glitz_texture_unbind (gl, texture);
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
                             glitz_filter_t filter)
{
  if (!texture->target)
    return;

  if (!texture->allocated)
    _glitz_texture_allocate (gl, texture);
    
  if (texture->filter != filter) {
    switch (filter) {
    case GLITZ_FILTER_FAST:
    case GLITZ_FILTER_NEAREST:
      gl->tex_parameter_i (texture->target,
                           GLITZ_GL_TEXTURE_MAG_FILTER, GLITZ_GL_NEAREST);
      gl->tex_parameter_i (texture->target,
                           GLITZ_GL_TEXTURE_MIN_FILTER, GLITZ_GL_NEAREST);
      break;
    case GLITZ_FILTER_GOOD:
    case GLITZ_FILTER_BEST:
    case GLITZ_FILTER_BILINEAR:
      gl->tex_parameter_i (texture->target,
                           GLITZ_GL_TEXTURE_MAG_FILTER, GLITZ_GL_LINEAR);
      gl->tex_parameter_i (texture->target,
                           GLITZ_GL_TEXTURE_MIN_FILTER, GLITZ_GL_LINEAR);
      break;
    }
    texture->filter = filter;
  }
}

void
glitz_texture_ensure_repeat (glitz_gl_proc_address_list_t *gl,
                             glitz_texture_t *texture,
                             glitz_bool_t repeat)
{
  if (!texture->target)
    return;

  if (!texture->allocated)
    _glitz_texture_allocate (gl, texture);
  
  if (texture->repeat != repeat) {
    if (repeat) {
      gl->tex_parameter_i(texture->target,
                          GLITZ_GL_TEXTURE_WRAP_S, GLITZ_GL_REPEAT);
      gl->tex_parameter_i (texture->target,
                           GLITZ_GL_TEXTURE_WRAP_T, GLITZ_GL_REPEAT);
    } else {
      gl->tex_parameter_i (texture->target,
                           GLITZ_GL_TEXTURE_WRAP_S, GLITZ_GL_CLAMP_TO_EDGE);
      gl->tex_parameter_i (texture->target,
                           GLITZ_GL_TEXTURE_WRAP_T, GLITZ_GL_CLAMP_TO_EDGE);
    }
    texture->repeat = repeat;
  }
}

void
glitz_texture_bind (glitz_gl_proc_address_list_t *gl,
                    glitz_texture_t *texture)
{  
  gl->disable (GLITZ_GL_TEXTURE_RECTANGLE_EXT);
  gl->disable (GLITZ_GL_TEXTURE_2D);

  if (!texture->target)
    return;

  if (!texture->allocated)
    _glitz_texture_allocate (gl, texture);
  
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
                            glitz_region_box_t *region)
{
  glitz_surface_push_current (surface, GLITZ_CN_SURFACE_DRAWABLE_CURRENT);
  
  glitz_texture_bind (surface->gl, texture);

  if (!texture->allocated)
    _glitz_texture_allocate (surface->gl, texture);

  if (region->x1 < 0) region->x1 = 0;
  if (region->y1 < 0) region->y1 = 0;
  if (region->x2 > surface->width) region->x2 = surface->width;
  if (region->y2 > surface->height) region->y2 = surface->height;

  surface->gl->copy_tex_sub_image_2d (texture->target, 0,
                                      region->x1, region->y1,
                                      region->x1, region->y1,
                                      region->x2 - region->x1,
                                      region->y2 - region->y1);
  
  surface->gl->flush ();

  glitz_texture_unbind (surface->gl, texture);
  glitz_surface_pop_current (surface);
}
