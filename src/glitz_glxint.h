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

#ifndef GLITZ_GLXINT_H_INCLUDED
#define GLITZ_GLXINT_H_INCLUDED

#include "glitzint.h"

#include "glitz-glx.h"

#include <GL/gl.h>
#include <GL/glx.h>

#include "glitz_glxext.h"

#define GLITZ_GLX_FEATURE_TEXTURE_RECTANGLE_MASK       (1L <<  0)
#define GLITZ_GLX_FEATURE_TEXTURE_NPOT_MASK            (1L <<  1)
#define GLITZ_GLX_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK (1L <<  2)
#define GLITZ_GLX_FEATURE_MULTISAMPLE_MASK             (1L <<  3)
#define GLITZ_GLX_FEATURE_CLIENT_MULTISAMPLE_MASK      (1L <<  4)
#define GLITZ_GLX_FEATURE_MULTISAMPLE_FILTER_MASK      (1L <<  5)
#define GLITZ_GLX_FEATURE_ARB_MULTITEXTURE_MASK        (1L <<  6)
#define GLITZ_GLX_FEATURE_ARB_VERTEX_PROGRAM_MASK      (1L <<  7)
#define GLITZ_GLX_FEATURE_ARB_FRAGMENT_PROGRAM_MASK    (1L <<  8)
#define GLITZ_GLX_FEATURE_GLX13_MASK                   (1L <<  9)
#define GLITZ_GLX_FEATURE_ARB_RENDER_TEXTURE_MASK      (1L << 10)

typedef struct _glitz_glx_surface glitz_glx_surface_t;
typedef struct _glitz_glx_screen_info_t glitz_glx_screen_info_t;
typedef struct _glitz_glx_display_info_t glitz_glx_display_info_t;

typedef struct _glitz_glx_static_proc_address_list_t {
  glitz_glx_get_proc_address_arb_t get_proc_address_arb;
  glitz_glx_get_fbconfigs_t get_fbconfigs;
  glitz_glx_get_fbconfig_attrib_t get_fbconfig_attrib;
  glitz_glx_get_visual_from_fbconfig_t get_visual_from_fbconfig;
  glitz_glx_create_pbuffer_t create_pbuffer;
  glitz_glx_destroy_pbuffer_t destroy_pbuffer;
  glitz_glx_make_context_current_t make_context_current;
  glitz_bool_t need_lookup;
} glitz_glx_static_proc_address_list_t;

typedef struct _glitz_glx_proc_address_list_t {
  glitz_glx_bind_tex_image_arb_t bind_tex_image_arb;
  glitz_glx_release_tex_image_arb_t release_tex_image_arb;
  glitz_bool_t need_lookup;
} glitz_glx_proc_address_list_t;

typedef struct _glitz_glx_thread_info_t {
  glitz_glx_display_info_t **displays;
  int n_displays;
  glitz_glx_static_proc_address_list_t glx;
  char *gl_library;
  void *dlhand;
} glitz_glx_thread_info_t;

struct _glitz_glx_display_info_t {
  glitz_glx_thread_info_t *thread_info;
  Display *display;
  glitz_glx_screen_info_t **screens;
  int n_screens;
};

typedef struct _glitz_glx_context_info_t {
  glitz_glx_surface_t *surface;
  glitz_constraint_t constraint;
} glitz_glx_context_info_t;

typedef struct _glitz_glx_context_t {
  GLXContext context;
  XID id;
  glitz_gl_proc_address_list_t gl;
  glitz_glx_proc_address_list_t glx;
  GLXFBConfig fbconfig;
  glitz_gl_uint_t texture_indirections;
} glitz_glx_context_t;

struct _glitz_glx_screen_info_t {
  glitz_glx_display_info_t *display_info;
  int screen;

  glitz_format_t *formats;
  XID *format_ids;
  int n_formats;
  
  glitz_glx_context_t **contexts;
  int n_contexts;
  
  glitz_glx_context_info_t *context_stack;
  int context_stack_size;
  
  glitz_glx_context_t root_context;
  GLXDrawable root_drawable;

  long int feature_mask;
  long int glx_feature_mask;
  long int texture_mask;

  glitz_programs_t programs;
};

struct _glitz_glx_surface {
  glitz_surface_t base;
  
  glitz_glx_screen_info_t *screen_info;
  glitz_glx_context_t *context;
  GLXDrawable drawable;
  GLXDrawable pbuffer;
  glitz_bool_t render_texture;
};

extern void __internal_linkage
glitz_glx_query_extensions (glitz_glx_screen_info_t *screen_info);

extern glitz_glx_screen_info_t *__internal_linkage
glitz_glx_screen_info_get (Display *display,
                           int screen);

extern glitz_function_pointer_t __internal_linkage
glitz_glx_get_proc_address (glitz_glx_thread_info_t *info, const char *name);

extern glitz_glx_context_t *__internal_linkage
glitz_glx_context_get (glitz_glx_screen_info_t *screen_info,
                       glitz_format_t *format);

extern void __internal_linkage
glitz_glx_context_destroy (glitz_glx_screen_info_t *screen_info,
                           glitz_glx_context_t *context);

extern int __internal_linkage
glitz_glx_ensure_pbuffer_support (glitz_glx_screen_info_t *screen_info,
                                  XID fbconfigid);

extern glitz_glx_context_t *__internal_linkage
glitz_glx_context_get_default (glitz_glx_screen_info_t *screen_info);

extern void __internal_linkage
glitz_glx_context_make_current (glitz_glx_surface_t *surface,
                                glitz_bool_t flush);

extern glitz_glx_surface_t *__internal_linkage
glitz_glx_context_push_current (glitz_glx_surface_t *surface,
                                glitz_constraint_t constraint);

extern glitz_glx_surface_t *__internal_linkage
glitz_glx_context_pop_current (glitz_glx_surface_t *surface);

extern void __internal_linkage
glitz_glx_context_proc_address_lookup (glitz_glx_thread_info_t *thread_info,
                                       glitz_glx_context_t *context);

extern void __internal_linkage
glitz_glx_query_formats (glitz_glx_screen_info_t *screen_info);

extern GLXPbuffer __internal_linkage
glitz_glx_pbuffer_create (glitz_glx_display_info_t *display_info,
                          GLXFBConfig fbconfig,
                          glitz_texture_t *texture);

extern void __internal_linkage
glitz_glx_pbuffer_destroy (glitz_glx_display_info_t *display_info,
                           GLXPbuffer pbuffer);

/* Avoid unnecessary PLT entries.  */

slim_hidden_proto(glitz_glx_init)
slim_hidden_proto(glitz_glx_fini)
slim_hidden_proto(glitz_glx_find_format)
slim_hidden_proto(glitz_glx_find_standard_format)
slim_hidden_proto(glitz_glx_get_visual_info_from_format)
slim_hidden_proto(glitz_glx_surface_create)
slim_hidden_proto(glitz_glx_surface_create_for_window)

#endif /* GLITZ_GLXINT_H_INCLUDED */
