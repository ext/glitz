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
glitz_color_range_init (glitz_color_range_t *color_range,
                        unsigned int size)
{
  color_range->texture = 0;
  color_range->filter = GLITZ_FILTER_NEAREST;
  color_range->extend = GLITZ_EXTEND_PAD;
  color_range->ref_count = 1;
  color_range->data = malloc (size * 4);
  color_range->size = size;
  color_range->update_mask = GLITZ_COLOR_RANGE_UPDATE_ALL_MASK;
  color_range->delete_textures = NULL;
}

static void
glitz_color_range_fini (glitz_color_range_t *color_range)
{
  if (color_range->texture)
    color_range->delete_textures (1, &color_range->texture);

  if (color_range->data)
    free (color_range->data);
}

glitz_color_range_t *
glitz_color_range_create (unsigned int size)
{
  glitz_color_range_t *color_range;
  
  color_range = malloc (sizeof (glitz_color_range_t));
  if (!color_range)
    return NULL;

  glitz_color_range_init (color_range, size);

  if (!color_range->data) {
    free (color_range);
    return NULL;
  }
  
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

  glitz_color_range_fini (color_range);
  free (color_range);
}

void
glitz_color_range_bind (glitz_gl_proc_address_list_t *gl,
                        glitz_color_range_t *color_range,
                        unsigned long feature_mask)
{
  if (color_range->update_mask & GLITZ_COLOR_RANGE_UPDATE_TEXTURE_MASK) {
    
    if (!color_range->texture) {
      gl->gen_textures (1, &color_range->texture);
      color_range->delete_textures = gl->delete_textures;
    }
    
    gl->enable (GLITZ_GL_TEXTURE_1D);
    gl->bind_texture (GLITZ_GL_TEXTURE_1D, color_range->texture);
    
    /* If data is not POT and NPOT texture support is missing we reallocate
       a POT sized memory block for glTexImage1D */
    if ((!(feature_mask & GLITZ_FEATURE_TEXTURE_NPOT_MASK)) &&
        (!glitz_uint_is_power_of_two (color_range->size))) {
      glitz_uint_to_power_of_two (&color_range->size);
      
      color_range->data = realloc (color_range->data, color_range->size * 4);
    }
    
    gl->tex_image_1d (GLITZ_GL_TEXTURE_1D, 0, GLITZ_GL_RGBA,
                      color_range->size, 0,
                      glitz_get_gl_format_from_bpp (32),
                      glitz_get_gl_data_type_from_bpp (32),
                      color_range->data);
    
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
                             GLITZ_GL_MIRRORED_REPEAT_ARB);
      else
        gl->tex_parameter_i (GLITZ_GL_TEXTURE_1D,
                             GLITZ_GL_TEXTURE_WRAP_S,
                             GLITZ_GL_CLAMP_TO_EDGE);
      break;
    }
    color_range->update_mask &= ~GLITZ_COLOR_RANGE_UPDATE_EXTEND_MASK;
  }
}

unsigned char *
glitz_color_range_get_data (glitz_color_range_t *color_range)
{
  return color_range->data;
}
slim_hidden_def(glitz_color_range_get_data);

void
glitz_color_range_put_back_data (glitz_color_range_t *color_range)
{
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
