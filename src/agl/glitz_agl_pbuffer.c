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
#  include "../../config.h"
#endif

#include "glitz_aglint.h"

AGLPbuffer
glitz_agl_pbuffer_create (glitz_agl_thread_info_t    *thread_info,
                          glitz_pbuffer_attributes_t *attributes,
                          unsigned long              mask,
                          int                        *width,
                          int                        *height)
{
  AGLPbuffer pbuffer;
  glitz_gl_enum_t target;
  int w, h;

  if (mask & GLITZ_PBUFFER_WIDTH_MASK)
    w = attributes->width;
  else
    w = GLITZ_DEFAULT_PBUFFER_WIDTH;

  if (mask & GLITZ_PBUFFER_HEIGHT_MASK)
    h = attributes->height;
  else
    h = GLITZ_DEFAULT_PBUFFER_HEIGHT;

  if (!POWER_OF_TWO (w) || !POWER_OF_TWO (h)) {
    if (thread_info->agl_feature_mask &
        GLITZ_AGL_FEATURE_TEXTURE_RECTANGLE_MASK)
      target = GLITZ_GL_TEXTURE_RECTANGLE;
    else
      return (AGLPbuffer) 0;
  } else
    target = GLITZ_GL_TEXTURE_2D;

  aglCreatePBuffer (w, h, target, GLITZ_GL_RGBA, 0, &pbuffer);

  *width = w;
  *height = h;

  return pbuffer;
}

void 
glitz_agl_pbuffer_destroy (AGLPbuffer pbuffer)
{
  aglDestroyPBuffer (pbuffer);
}
