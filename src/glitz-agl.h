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

#ifndef GLITZ_AGL_H_INCLUDED
#define GLITZ_AGL_H_INCLUDED

#include <glitz.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <Carbon/Carbon.h>

/* glitz_agl_info.c */

void
glitz_agl_init (void);

void
glitz_agl_fini (void);
  
  
/* glitz_agl_format.c */

glitz_format_t *
glitz_agl_find_format (unsigned long mask,
                       const glitz_format_t *templ,
                       int count);
  
glitz_format_t *
glitz_agl_find_standard_format (unsigned long options,
                                glitz_format_name_t format_name);

  
/* glitz_agl_surface.c */

glitz_surface_t *
glitz_agl_surface_create (glitz_format_t *format,
                          int width,
                          int height);

glitz_surface_t *
glitz_agl_surface_create_for_window (glitz_format_t *format,
                                     WindowRef window);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* GLITZ_AGL_H_INCLUDED */
