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

#include "glitz_aglint.h"

extern glitz_gl_proc_address_list_t _glitz_agl_gl_proc_address;

AGLPbuffer
glitz_agl_pbuffer_create (glitz_texture_t *texture)
{
  AGLPbuffer pbuffer;

  aglCreatePBuffer (texture->width, texture->height,
                    texture->target, GL_RGBA, 0, &pbuffer);

  return pbuffer;
}

void 
glitz_agl_pbuffer_bind (AGLPbuffer pbuffer,
                        AGLContext context,
                        glitz_texture_t *texture,
                        glitz_format_t *format)
{
  glitz_texture_bind (&_glitz_agl_gl_proc_address, texture);

  if (format->doublebuffer)
    aglTexImagePBuffer (context, pbuffer, GL_BACK);
  else
    aglTexImagePBuffer (context, pbuffer, GL_FRONT);
  
  glitz_texture_unbind (&_glitz_agl_gl_proc_address, texture);
}

void 
glitz_agl_pbuffer_destroy (AGLPbuffer pbuffer)
{
  aglDestroyPBuffer (pbuffer);
}
