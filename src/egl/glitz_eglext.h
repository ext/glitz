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
 * Authors: David Reveman <davidr@novell.com>
 *          Peter Nilsson <c99pnn@cs.umu.se>
 */

#ifndef GLITZ_EGLEXT_H_INCLUDED
#define GLITZ_EGLEXT_H_INCLUDED


typedef glitz_function_pointer_t (* glitz_egl_get_proc_address_t)
     (const glitz_gl_ubyte_t *);
typedef EGLConfig *(* glitz_egl_get_configs_t)
     (EGLDisplay egl_display, EGLScreenMESA egl_screen, int *n_elements);
typedef int (* glitz_egl_get_config_attrib_t)
     (EGLDisplay egl_display, EGLConfig egl_config, int attribute, int *value);
typedef EGLSurface (* glitz_egl_create_pbuffer_t)
     (EGLDisplay egl_display, EGLConfig egl_config, const int *attrib_list);
typedef void (* glitz_egl_destroy_pbuffer_t)
     (EGLDisplay egl_display, EGLSurface egl_pbuffer);
typedef void (* glitz_egl_query_drawable_t)
     (EGLDisplay egl_display, EGLSurface egl_surface,
      int attribute, unsigned int *value);
typedef EGLBoolean (* glitz_egl_make_context_current_t)
     (EGLDisplay egl_display, EGLSurface egl_draw, EGLSurface egl_read, EGLContext egl_ctx);
typedef EGLContext (* glitz_egl_create_new_context_t)
     (EGLDisplay egl_display, EGLConfig egl_config, int render_type,
      EGLContext egl_share_list, EGLBoolean direct);

typedef EGLBoolean *(* glitz_egl_bind_tex_image_t)
     (EGLDisplay egl_display, EGLSurface egl_pbuffer, int buffer);
typedef EGLBoolean (* glitz_egl_release_tex_image_t)
     (EGLDisplay egl_display, EGLSurface egl_pbuffer, int buffer);

#endif /* GLITZ_EGLEXT_H_INCLUDED */
