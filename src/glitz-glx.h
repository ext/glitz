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

#ifndef GLITZ_GLX_H_INCLUDED
#define GLITZ_GLX_H_INCLUDED

#include <glitz.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>

/* glitz_glx_info.c */

void
glitz_glx_init (const char *gl_library);

void
glitz_glx_fini (void);
  
  
/* glitz_glx_format.c */

glitz_format_t *
glitz_glx_find_format (Display *display,
                       int screen,
                       unsigned long mask,
                       const glitz_format_t *templ,
                       int count);
  
glitz_format_t *
glitz_glx_find_standard_format (Display *display,
                                int screen,
                                unsigned long option_mask,
                                glitz_format_name_t format_name);

XVisualInfo *
glitz_glx_get_visual_info_from_format (Display *display,
                                       int screen,
                                       glitz_format_t *format);
  

/* glitz_glx_surface.c */

glitz_surface_t *
glitz_glx_surface_create (Display *display,
                          int screen,
                          glitz_format_t *format,
                          int width,
                          int height);

glitz_surface_t *
glitz_glx_surface_create_for_window (Display *display,
                                     int screen,
                                     glitz_format_t *format,
                                     Window window);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* GLITZ_GLX_H_INCLUDED */
