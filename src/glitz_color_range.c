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

#define GLITZ_COLOR_RANGE_UPDATE_TEXTURE_MASK (1L << 0)
#define GLITZ_COLOR_RANGE_UPDATE_FILTER_MASK  (1L << 1)
#define GLITZ_COLOR_RANGE_UPDATE_EXTEND_MASK  (1L << 2)
#define GLITZ_COLOR_RANGE_UPDATE_ALL_MASK     ((1L << 3) - 1)

struct _glitz_color_range {
  unsigned int size;
  glitz_gl_uint_t texture;
  glitz_filter_t filter;
  glitz_extend_t extend;
  unsigned long update_mask;
  unsigned int ref_count;
  glitz_pixel_buffer_t *buffer;
  glitz_surface_t *surface;
};

glitz_color_range_t *
glitz_color_range_create (glitz_surface_t *surface,
                          unsigned int size)
{
  glitz_color_range_t *color_range;
  
  color_range = malloc (sizeof (glitz_color_range_t));
  if (!color_range)
    return NULL;

  color_range->texture = 0;
  color_range->filter = GLITZ_FILTER_NEAREST;
  color_range->extend = GLITZ_EXTEND_PAD;
  color_range->ref_count = 1;

  /* If data is not POT and NPOT texture support is missing we force
     the size to POT */
  if ((!(surface->feature_mask & GLITZ_FEATURE_TEXTURE_NPOT_MASK)) &&
      (!glitz_uint_is_power_of_two (size)))
    glitz_uint_to_power_of_two (&size);
  
  color_range->size = size;
  color_range->buffer =
    glitz_pixel_buffer_create (surface, NULL, size * 4 /* ARGB32 */,
                               GLITZ_PIXEL_BUFFER_HINT_STATIC_DRAW);
  if (color_range->buffer == NULL) {
    free (color_range);
    return NULL;
  }
  
  color_range->update_mask = GLITZ_COLOR_RANGE_UPDATE_ALL_MASK;
  color_range->surface = surface;
  glitz_surface_reference (surface);

  return color_range;
}
slim_hidden_def(glitz_color_range_create);

void
glitz_color_range_reference (glitz_color_range_t *color_range)
{
  if (color_range == NULL)
    return;

  color_range->ref_count++;
}

void
glitz_color_range_destroy (glitz_color_range_t *color_range)
{
  if (!color_range)
    return;

  color_range->ref_count--;
  if (color_range->ref_count)
    return;

  glitz_pixel_buffer_destroy (color_range->buffer);
  
  if (color_range->texture) {
    glitz_surface_push_current (color_range->surface,
                                GLITZ_CN_ANY_CONTEXT_CURRENT);
    
    color_range->surface->gl->delete_textures (1, &color_range->texture);

    glitz_surface_pop_current (color_range->surface);
  }

  glitz_surface_destroy (color_range->surface);
  
  free (color_range);
}

void
glitz_color_range_bind (glitz_gl_proc_address_list_t *gl,
                        glitz_color_range_t *color_range,
                        unsigned long feature_mask)
{
  if (color_range->update_mask & GLITZ_COLOR_RANGE_UPDATE_TEXTURE_MASK) {
    char *data;
    
    if (!color_range->texture)
      gl->gen_textures (1, &color_range->texture);
    
    gl->enable (GLITZ_GL_TEXTURE_1D);
    gl->bind_texture (GLITZ_GL_TEXTURE_1D, color_range->texture);

    data = glitz_pixel_buffer_bind (color_range->buffer,
                                    GLITZ_GL_PIXEL_UNPACK_BUFFER);
    
    gl->tex_image_1d (GLITZ_GL_TEXTURE_1D, 0, GLITZ_GL_RGBA,
                      color_range->size, 0,
                      GLITZ_GL_BGRA,
                      
#if IMAGE_BYTE_ORDER == MSBFirst
                      GLITZ_GL_UNSIGNED_INT_8_8_8_8_REV,
#else
                      GLITZ_GL_UNSIGNED_BYTE,
#endif
                      
                      data);

    glitz_pixel_buffer_unbind (color_range->buffer);
    
    color_range->update_mask &= ~GLITZ_COLOR_RANGE_UPDATE_TEXTURE_MASK;
  } else {
    gl->enable (GLITZ_GL_TEXTURE_1D);
    gl->bind_texture (GLITZ_GL_TEXTURE_1D, color_range->texture);
  }
  
  gl->tex_env_f (GLITZ_GL_TEXTURE_ENV,
                 GLITZ_GL_TEXTURE_ENV_MODE,
                 GLITZ_GL_REPLACE);
  
  if (color_range->update_mask & GLITZ_COLOR_RANGE_UPDATE_FILTER_MASK) {
    switch (color_range->filter) {
    case GLITZ_FILTER_FAST:
    case GLITZ_FILTER_NEAREST:
      gl->tex_parameter_i (GLITZ_GL_TEXTURE_1D,
                           GLITZ_GL_TEXTURE_MAG_FILTER,
                           GLITZ_GL_NEAREST);
      gl->tex_parameter_i (GLITZ_GL_TEXTURE_1D,
                           GLITZ_GL_TEXTURE_MIN_FILTER,
                           GLITZ_GL_NEAREST);
      break;
    case GLITZ_FILTER_GOOD:
    case GLITZ_FILTER_BEST:
    case GLITZ_FILTER_BILINEAR:
      gl->tex_parameter_i (GLITZ_GL_TEXTURE_1D,
                           GLITZ_GL_TEXTURE_MAG_FILTER,
                           GLITZ_GL_LINEAR);
      gl->tex_parameter_i (GLITZ_GL_TEXTURE_1D,
                           GLITZ_GL_TEXTURE_MIN_FILTER,
                           GLITZ_GL_LINEAR);
      break;
    }
    color_range->update_mask &= ~GLITZ_COLOR_RANGE_UPDATE_FILTER_MASK;
  }
  
  if (color_range->update_mask & GLITZ_COLOR_RANGE_UPDATE_EXTEND_MASK) {
    switch (color_range->extend) {
    case GLITZ_EXTEND_PAD:
      gl->tex_parameter_i (GLITZ_GL_TEXTURE_1D,
                           GLITZ_GL_TEXTURE_WRAP_S,
                           GLITZ_GL_CLAMP_TO_EDGE);
      break;
    case GLITZ_EXTEND_REPEAT:
      gl->tex_parameter_i (GLITZ_GL_TEXTURE_1D,
                           GLITZ_GL_TEXTURE_WRAP_S,
                           GLITZ_GL_REPEAT);
      break;
    case GLITZ_EXTEND_REFLECT:
      if (feature_mask & GLITZ_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK)
        gl->tex_parameter_i (GLITZ_GL_TEXTURE_1D,
                             GLITZ_GL_TEXTURE_WRAP_S,
                             GLITZ_GL_MIRRORED_REPEAT);
      else
        gl->tex_parameter_i (GLITZ_GL_TEXTURE_1D,
                             GLITZ_GL_TEXTURE_WRAP_S,
                             GLITZ_GL_CLAMP_TO_EDGE);
      break;
    }
    color_range->update_mask &= ~GLITZ_COLOR_RANGE_UPDATE_EXTEND_MASK;
  }
}

char *
glitz_color_range_get_data (glitz_color_range_t *color_range)
{
  return glitz_pixel_buffer_get_data (color_range->buffer,
                                      GLITZ_PIXEL_BUFFER_ACCESS_WRITE_ONLY);
}
slim_hidden_def(glitz_color_range_get_data);

void
glitz_color_range_put_back_data (glitz_color_range_t *color_range)
{
  glitz_pixel_buffer_put_back_data (color_range->buffer);
  
  color_range->update_mask |= GLITZ_COLOR_RANGE_UPDATE_TEXTURE_MASK;
}
slim_hidden_def(glitz_color_range_put_back_data);

void
glitz_color_range_set_filter (glitz_color_range_t *color_range,
                              glitz_filter_t filter)
{
  color_range->filter = filter;
  color_range->update_mask |= GLITZ_COLOR_RANGE_UPDATE_TEXTURE_MASK;
}
slim_hidden_def(glitz_color_range_set_filter);

void
glitz_color_range_set_extend (glitz_color_range_t *color_range,
                              glitz_extend_t extend)
{
  color_range->extend = extend;
  color_range->update_mask |= GLITZ_COLOR_RANGE_UPDATE_TEXTURE_MASK;
}
slim_hidden_def(glitz_color_range_set_extend);
