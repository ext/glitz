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

    if (mask & GLITZ_FORMAT_BPP_MASK)
      if (templ->bpp != formats->bpp)
        continue;
        
    if (mask & GLITZ_FORMAT_RED_MASK_MASK)
      if (templ->red_mask != formats->red_mask)
        continue;

    if (mask & GLITZ_FORMAT_GREEN_MASK_MASK)
      if (templ->green_mask != formats->green_mask)
        continue;

    if (mask & GLITZ_FORMAT_BLUE_MASK_MASK)
      if (templ->blue_mask != formats->blue_mask)
        continue;

    if (mask & GLITZ_FORMAT_ALPHA_MASK_MASK)
      if (templ->alpha_mask != formats->alpha_mask)
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

  if (options & GLITZ_FORMAT_OPTION_READONLY_MASK) {
    format->draw.offscreen = format->draw.onscreen = 0;
    *mask |= GLITZ_FORMAT_DRAW_ONSCREEN_MASK;
    *mask |= GLITZ_FORMAT_DRAW_OFFSCREEN_MASK;
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

  return glitz_format_find (formats, n_formats, mask, &templ, 0);
}

void
glitz_format_calculate_pixel_transfer_info (glitz_format_t *format)
{
  long int mask = 0x000000ff;
  format->bpp = 0;

  if (format->red_size || format->green_size || format->blue_size) {
    format->blue_mask = mask;
    format->green_mask = mask << 8;
    format->red_mask = mask << 16;
    format->bpp += 24;
    mask = 0xff000000;
  } else
    format->blue_mask = format->green_mask = format->red_mask = 0x0;
  
  if (format->alpha_size) {
    format->alpha_mask = mask;
    format->bpp += 8;
  } else
    format->alpha_mask = 0x0;
}
