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

#include <stdlib.h>

static glitz_format_t _texture_formats[] = {
  {
    GLITZ_GL_INTENSITY4, 0, 0, 0, 4, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_INTENSITY8, 0, 0, 0, 8, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_INTENSITY12, 0, 0, 0, 12, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_INTENSITY16, 0, 0, 0, 16, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_R3_G3_B2, 3, 3, 2, 0, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_RGB4, 4, 4, 4, 0, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_RGB5,5, 5, 5, 0, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_RGB8, 8, 8, 8, 0, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_RGB10, 10, 10, 10, 0, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_RGB12, 12, 12, 12, 0, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_RGB16, 16, 16, 16, 0, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_RGBA2, 2, 2, 2, 2, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_RGB5_A1, 5, 5, 5, 1, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_RGBA8, 8, 8, 8, 8, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_RGB10_A2, 10, 10, 10, 2, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_RGBA12, 12, 12, 12, 12, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }, {
    GLITZ_GL_RGBA16, 16, 16, 16, 16, 0, 0, 0, { 0, 1 }, { 0, 0 }, { 0, 0 }
  }
};

void
glitz_format_for_each_texture_format (glitz_format_call_back_t call_back,
				      glitz_gl_proc_address_list_t *gl,
                                      void *ptr)
{
  glitz_gl_uint_t name;
  glitz_gl_int_t value;
  int i, n_texture_formats =
    sizeof (_texture_formats) / sizeof (glitz_format_t);

  gl->gen_textures (1, &name);
  gl->enable (GLITZ_GL_PROXY_TEXTURE_2D);
  gl->bind_texture (GLITZ_GL_PROXY_TEXTURE_2D, name);
  
  for (i = 0; i < n_texture_formats; i++) {
    gl->tex_image_2d (GLITZ_GL_PROXY_TEXTURE_2D, 0,
		      _texture_formats[i].id, 1, 1,
		      0, GLITZ_GL_RGBA, GLITZ_GL_UNSIGNED_BYTE, NULL);
    gl->get_tex_level_parameter_iv (GLITZ_GL_PROXY_TEXTURE_2D, 0,
				    GLITZ_GL_TEXTURE_RED_SIZE, &value);
    if (value != _texture_formats[i].red_size)
      continue;

    gl->get_tex_level_parameter_iv (GLITZ_GL_PROXY_TEXTURE_2D, 0,
				    GLITZ_GL_TEXTURE_GREEN_SIZE, &value);
    if (value != _texture_formats[i].green_size)
      continue;

    gl->get_tex_level_parameter_iv (GLITZ_GL_PROXY_TEXTURE_2D, 0,
				    GLITZ_GL_TEXTURE_BLUE_SIZE, &value);
    if (value != _texture_formats[i].blue_size)
      continue;

    gl->get_tex_level_parameter_iv (GLITZ_GL_PROXY_TEXTURE_2D, 0,
				    GLITZ_GL_TEXTURE_ALPHA_SIZE, &value);
    if (value != _texture_formats[i].alpha_size) {
      gl->get_tex_level_parameter_iv (GLITZ_GL_PROXY_TEXTURE_2D, 0,
                                      GLITZ_GL_TEXTURE_INTENSITY_SIZE, &value);
      if (value != _texture_formats[i].alpha_size)
        continue;
    }

    call_back (&_texture_formats[i], ptr);
  }

  gl->bind_texture (GLITZ_GL_PROXY_TEXTURE_2D, 0);
  gl->disable (GLITZ_GL_PROXY_TEXTURE_2D);
  gl->delete_textures (1, &name);
}

glitz_format_t *
glitz_format_find (glitz_format_t *formats,
                   int n_formats,
                   unsigned long mask,
                   const glitz_format_t *templ,
                   int count)
{
  for (; n_formats; n_formats--, formats++) {
    if (mask & GLITZ_FORMAT_ID_MASK)
      if (templ->id != formats->id)
        continue;

    if (mask & GLITZ_FORMAT_RED_SIZE_MASK)
      if (templ->red_size != formats->red_size)
        continue;

    if (mask & GLITZ_FORMAT_GREEN_SIZE_MASK)
      if (templ->green_size != formats->green_size)
        continue;

    if (mask & GLITZ_FORMAT_BLUE_SIZE_MASK)
      if (templ->blue_size != formats->blue_size)
        continue;

    if (mask & GLITZ_FORMAT_ALPHA_SIZE_MASK)
      if (templ->alpha_size != formats->alpha_size)
        continue;

    if (mask & GLITZ_FORMAT_DEPTH_SIZE_MASK)
      if (templ->depth_size != formats->depth_size)
        continue;

    if (mask & GLITZ_FORMAT_STENCIL_SIZE_MASK)
      if (templ->stencil_size != formats->stencil_size)
        continue;

    if (mask & GLITZ_FORMAT_DOUBLEBUFFER_MASK)
      if (templ->doublebuffer != formats->doublebuffer)
        continue;

    if (mask & GLITZ_FORMAT_MULTISAMPLE_MASK)
      if (templ->multisample.supported != formats->multisample.supported)
        continue;

    if (mask & GLITZ_FORMAT_MULTISAMPLE_SAMPLES_MASK)
      if (templ->multisample.samples != formats->multisample.samples)
        continue;

    if (mask & GLITZ_FORMAT_READ_ONSCREEN_MASK)
      if (templ->read.onscreen != formats->read.onscreen)
        continue;
    
    if (mask & GLITZ_FORMAT_READ_OFFSCREEN_MASK)
      if (templ->read.offscreen != formats->read.offscreen)
        continue;

    if (mask & GLITZ_FORMAT_DRAW_ONSCREEN_MASK)
      if (templ->draw.onscreen != formats->draw.onscreen)
        continue;
    
    if (mask & GLITZ_FORMAT_DRAW_OFFSCREEN_MASK)
      if (templ->draw.offscreen != formats->draw.offscreen)
        continue;

    if (count-- == 0)
      return formats;    
  }

  return NULL;
}

static glitz_format_t *
glitz_format_find_best (glitz_format_t *formats,
                        int n_formats,
                        unsigned long mask,
                        const glitz_format_t *templ)
{
  glitz_format_t *format, *best_format;
  int best_diff, best_above;
  int i, diff, above, red_diff, green_diff, blue_diff, alpha_diff;
  unsigned long templ_mask;
  
  best_diff = MAXSHORT;
  best_above = 0;
  best_format = NULL;
  
  templ_mask = mask & ~(GLITZ_FORMAT_RED_SIZE_MASK |
                        GLITZ_FORMAT_GREEN_SIZE_MASK |
                        GLITZ_FORMAT_BLUE_SIZE_MASK |
                        GLITZ_FORMAT_ALPHA_SIZE_MASK);
  
  i = 0;
  do {
    format = glitz_format_find (formats, n_formats, templ_mask, templ, i++);
    if (format) {
      if (mask & GLITZ_FORMAT_RED_SIZE_MASK) {
        if (templ->red_size != 0 && format->red_size == 0)
          continue;
        
        red_diff = format->red_size - templ->red_size;
      } else
        red_diff = format->red_size;

      if (mask & GLITZ_FORMAT_GREEN_SIZE_MASK) {
        if (templ->green_size != 0 && format->green_size == 0)
          continue;

        green_diff = format->green_size - templ->green_size;
      } else
        green_diff = format->green_size;

      if (mask & GLITZ_FORMAT_BLUE_SIZE_MASK) {
        if (templ->blue_size != 0 && format->blue_size == 0)
          continue;

        blue_diff = format->blue_size - templ->blue_size;
      } else
        blue_diff = format->blue_size;

      if (mask & GLITZ_FORMAT_ALPHA_SIZE_MASK) {
        if (templ->alpha_size != 0 && format->alpha_size == 0)
          continue;

        alpha_diff = format->alpha_size - templ->alpha_size;
      } else
        alpha_diff = format->alpha_size;
      
      if (red_diff < 0 || green_diff < 0 || blue_diff < 0 || alpha_diff < 0)
        above = 0;
      else
        above = 1;
    
      diff = abs (red_diff) + abs (green_diff) + abs (blue_diff) +
        abs (alpha_diff);
      
      if (above > best_above || (above == best_above && diff < best_diff)) {
        best_diff = diff;
        best_above = above;
        best_format = format;
      }
    }
  } while (format);

  return best_format;
}

static void
_glitz_format_add_options (unsigned long options,
                           glitz_format_t *format,
                           unsigned long *mask)
{
  if (options & GLITZ_FORMAT_OPTION_DOUBLEBUFFER_MASK) {
    format->doublebuffer = 1;
    *mask |= GLITZ_FORMAT_DOUBLEBUFFER_MASK;
  }

  if (options & GLITZ_FORMAT_OPTION_SINGLEBUFFER_MASK) {
    format->doublebuffer = 0;
    *mask |= GLITZ_FORMAT_DOUBLEBUFFER_MASK;
  }

  if (options & GLITZ_FORMAT_OPTION_ONSCREEN_MASK) {
    format->draw.onscreen = 1;
    *mask |= GLITZ_FORMAT_DRAW_ONSCREEN_MASK;
  }

  if (options & GLITZ_FORMAT_OPTION_OFFSCREEN_MASK) {
    format->read.offscreen = 1;
    *mask |= GLITZ_FORMAT_READ_OFFSCREEN_MASK;
  }

  if (options & GLITZ_FORMAT_OPTION_MULTISAMPLE_MASK) {
    format->multisample.supported = 1;
    *mask |= GLITZ_FORMAT_MULTISAMPLE_MASK;
  }

  if (options & GLITZ_FORMAT_OPTION_NO_MULTISAMPLE_MASK) {
    format->multisample.supported = 0;
    *mask |= GLITZ_FORMAT_MULTISAMPLE_MASK;
  }

  if (options & GLITZ_FORMAT_OPTION_READDRAW_MASK) {
    if (*mask & GLITZ_FORMAT_READ_OFFSCREEN_MASK) {
      format->draw.offscreen = 1;
      *mask |= GLITZ_FORMAT_DRAW_OFFSCREEN_MASK;
    }
  }
  
  if (options & GLITZ_FORMAT_OPTION_READONLY_MASK) {
    format->draw.offscreen = 0;
    format->draw.onscreen = 0;
    *mask |= GLITZ_FORMAT_DRAW_OFFSCREEN_MASK;
    *mask |= GLITZ_FORMAT_DRAW_ONSCREEN_MASK;
  }
}

glitz_format_t *
glitz_format_find_standard (glitz_format_t *formats,
                            int n_formats,
                            unsigned long options,
                            glitz_format_name_t format_name)
{
  glitz_format_t templ;
  unsigned long mask = GLITZ_FORMAT_RED_SIZE_MASK |
    GLITZ_FORMAT_GREEN_SIZE_MASK | GLITZ_FORMAT_BLUE_SIZE_MASK |
    GLITZ_FORMAT_ALPHA_SIZE_MASK;

  switch (format_name) {
  case GLITZ_STANDARD_ARGB32:
    templ.red_size = 8;
    templ.green_size = 8;
    templ.blue_size = 8;
    templ.alpha_size = 8;
    break;
  case GLITZ_STANDARD_RGB24:
    templ.red_size = 8;
    templ.green_size = 8;
    templ.blue_size = 8;
    templ.alpha_size = 0;
    break;
  case GLITZ_STANDARD_A8:
    templ.red_size = 0;
    templ.green_size = 0;
    templ.blue_size = 0;
    templ.alpha_size = 8;
    break;
  case GLITZ_STANDARD_A1:
    templ.red_size = 0;
    templ.green_size = 0;
    templ.blue_size = 0;
    templ.alpha_size = 1;
    break;
  }

  _glitz_format_add_options (options, &templ, &mask);

  return glitz_format_find_best (formats, n_formats, mask, &templ);
}

glitz_gl_int_t
glitz_format_get_best_texture_format (glitz_format_t *formats,
				      int n_formats,
				      glitz_format_t *format)
{
  glitz_format_t templ;
  unsigned long mask;
  glitz_format_t *best_format, *texture_format;
  int n_texture_formats =
    sizeof (_texture_formats) / sizeof (glitz_format_t);
  
  if ((!format->read.offscreen) ||
      format->draw.offscreen || format->draw.onscreen) {
    templ = *format;
    templ.draw.offscreen = templ.draw.onscreen = 0;
    templ.read.offscreen = 1;
  
    mask = GLITZ_FORMAT_RED_SIZE_MASK | GLITZ_FORMAT_GREEN_SIZE_MASK | 
      GLITZ_FORMAT_BLUE_SIZE_MASK | GLITZ_FORMAT_ALPHA_SIZE_MASK |
      GLITZ_FORMAT_READ_OFFSCREEN_MASK | GLITZ_FORMAT_DRAW_ONSCREEN_MASK |
      GLITZ_FORMAT_DRAW_OFFSCREEN_MASK;
  
    best_format = glitz_format_find_best (formats, n_formats, mask, format);
    if (!best_format)
      return GLITZ_GL_RGBA;
  } else
    best_format = format;

  mask = GLITZ_FORMAT_RED_SIZE_MASK | GLITZ_FORMAT_GREEN_SIZE_MASK | 
    GLITZ_FORMAT_BLUE_SIZE_MASK | GLITZ_FORMAT_ALPHA_SIZE_MASK;
  
  texture_format = 
    glitz_format_find (_texture_formats, n_texture_formats,
		       mask, best_format, 0);
  if (!texture_format)
    return GLITZ_GL_RGBA;

  return (glitz_gl_int_t) texture_format->id;
}
