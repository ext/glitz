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

#include "glitz_glxint.h"

GLXPbuffer
glitz_glx_pbuffer_create (glitz_glx_screen_info_t    *screen_info,
                          GLXFBConfig                fbconfig,
                          glitz_pbuffer_attributes_t *attributes,
                          unsigned long              mask,
                          unsigned int               *width,
                          unsigned int               *height)
{
  if (fbconfig) {
    GLXPbuffer pbuffer;
    int pbuffer_attr[13];
    int i = 0;

    pbuffer_attr[i++] = GLX_PBUFFER_WIDTH;
    if (mask & GLITZ_PBUFFER_WIDTH_MASK)
      pbuffer_attr[i++] = attributes->width;
    else
      pbuffer_attr[i++] = GLITZ_DEFAULT_PBUFFER_WIDTH;

    pbuffer_attr[i++] = GLX_PBUFFER_HEIGHT;
    if (mask & GLITZ_PBUFFER_HEIGHT_MASK)
      pbuffer_attr[i++] = attributes->height;
    else
      pbuffer_attr[i++] = GLITZ_DEFAULT_PBUFFER_HEIGHT;
    
    pbuffer_attr[i++] = GLX_LARGEST_PBUFFER;
    pbuffer_attr[i++] = 1;

    pbuffer_attr[i++] = GLX_PRESERVED_CONTENTS;
    pbuffer_attr[i++] = 1;
    pbuffer_attr[i++] = 0;

    pbuffer =
      screen_info->glx.create_pbuffer (screen_info->display_info->display,
                                       fbconfig, pbuffer_attr);
    if (!pbuffer)
      return (GLXPbuffer) 0;

    screen_info->glx.query_drawable (screen_info->display_info->display,
                                     pbuffer, GLX_WIDTH, width);

    screen_info->glx.query_drawable (screen_info->display_info->display,
                                     pbuffer, GLX_HEIGHT, height);
    return pbuffer;
  } else
    return (GLXPbuffer) 0;
}

void 
glitz_glx_pbuffer_destroy (glitz_glx_screen_info_t *screen_info,
                           GLXPbuffer              pbuffer)
{
  screen_info->glx.destroy_pbuffer (screen_info->display_info->display,
                                    pbuffer);
}
