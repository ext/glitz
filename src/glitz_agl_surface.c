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

static glitz_surface_t *
_glitz_agl_surface_create_similar (void *abstract_templ,
                                 glitz_format_name_t format_name,
                                 glitz_bool_t drawable,
                                 int width,
                                 int height);

static void
_glitz_agl_surface_destroy (void *abstract_surface);

static glitz_texture_t *
_glitz_agl_surface_get_texture (void *abstract_surface);

static void
_glitz_agl_surface_update_size (void *abstract_surface);

static void
_glitz_agl_surface_flush (void *abstract_surface);

static glitz_bool_t
_glitz_agl_surface_push_current (void *abstract_surface,
                               glitz_constraint_t constraint)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;
  
  if (constraint == GLITZ_CN_SURFACE_DRAWABLE_CURRENT &&
      ((!surface->pbuffer) && (!surface->drawable)))
    constraint = GLITZ_CN_ANY_CONTEXT_CURRENT;
  
  surface = glitz_agl_context_push_current (surface, constraint);

  if (surface) {
    glitz_surface_setup_environment (&surface->base);
    return 1;
  }

  return 0;
}

static void
_glitz_agl_surface_pop_current (void *abstract_surface)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;

  surface = glitz_agl_context_pop_current (surface);
  
  if (surface)
    glitz_surface_setup_environment (&surface->base);
}

static const struct glitz_surface_backend glitz_agl_surface_backend = {
  _glitz_agl_surface_create_similar,
  _glitz_agl_surface_destroy,
  _glitz_agl_surface_push_current,
  _glitz_agl_surface_pop_current,
  _glitz_agl_surface_get_texture,
  _glitz_agl_surface_update_size,
  _glitz_agl_surface_flush
};

static void
_glitz_agl_surface_ensure_texture (glitz_agl_surface_t *surface)
{
   if (!(surface->base.hint_mask & GLITZ_INT_HINT_DIRTY_MASK))
    return;
    
  if (!surface->pbuffer)
    glitz_texture_copy_surface (surface->base.texture, &surface->base,
                              &surface->base.dirty_region);
  
  surface->base.hint_mask &= ~GLITZ_INT_HINT_DIRTY_MASK;
}

static glitz_texture_t *
_glitz_agl_surface_get_texture (void *abstract_surface) {
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;

  if (!surface->base.texture->allocated)
    glitz_texture_allocate (surface->base.gl, surface->base.texture);
  
  _glitz_agl_surface_ensure_texture (surface);
  
  return surface->base.texture;
}

static void
_glitz_agl_surface_update_size_for_window (WindowRef window,
                                         int *width,
                                         int *height)
{
  Rect window_bounds;

  GetWindowPortBounds (window, &window_bounds);
  
  *width = window_bounds.right - window_bounds.left;
  *height = window_bounds.bottom - window_bounds.top;
}

static void
_glitz_agl_set_features (glitz_agl_surface_t *surface)
{
  surface->base.feature_mask = surface->thread_info->feature_mask;

  surface->base.feature_mask &= ~GLITZ_FEATURE_CONVOLUTION_FILTER_MASK;
  surface->base.feature_mask &= ~GLITZ_FEATURE_MULTISAMPLE_MASK;
  surface->base.feature_mask &= ~GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK;

  if (surface->thread_info->feature_mask &
      GLITZ_FEATURE_CONVOLUTION_FILTER_MASK) {
    glitz_gl_uint_t texture_indirections;

    surface->base.gl->get_program_iv_arb
      (GLITZ_GL_FRAGMENT_PROGRAM_ARB,
       GLITZ_GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB,
       &texture_indirections);

    /* Convolution filter programs require support for at least nine
       texture indirections. */
    if (texture_indirections >= 9)
      surface->base.feature_mask |= GLITZ_FEATURE_CONVOLUTION_FILTER_MASK;
  }

  if (surface->base.format->multisample.supported) {
    surface->base.feature_mask |= GLITZ_FEATURE_MULTISAMPLE_MASK;
    if (surface->thread_info->feature_mask &
        GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK)
      surface->base.feature_mask |= GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK;
  }
}

static glitz_surface_t *
_glitz_agl_surface_create (glitz_agl_thread_info_t *thread_info,
                         glitz_format_t *format,
                         int width,
                         int height)
{
  glitz_agl_surface_t *surface;
  glitz_agl_context_t *context;
  unsigned int texture_format;
  long int texture_target_mask;

  context = glitz_agl_context_get (thread_info, format, 1);
  if (!context)
    return NULL;

  surface = (glitz_agl_surface_t *) calloc (1, sizeof (glitz_agl_surface_t));
  if (surface == NULL)
    return NULL;

  glitz_surface_init (&surface->base, &glitz_agl_surface_backend);
  
  surface->thread_info = thread_info;
  surface->context = context;

  surface->base.gl = &_glitz_agl_gl_proc_address;
  surface->base.programs = &thread_info->programs;
  surface->base.feature_mask = 0;
  surface->base.format = format;
  surface->base.width = width;
  surface->base.height = height;
  surface->base.hint_mask |= GLITZ_HINT_OFFSCREEN_MASK;

  texture_format = glitz_get_gl_format_from_bpp (format->bpp);

  glitz_surface_push_current (&surface->base, GLITZ_CN_ANY_CONTEXT_CURRENT);

  texture_target_mask = thread_info->texture_mask;

  /* Seems to be problem with binding a pbuffer to some power of two sized
     textures. This will try to avoid the problem. */
  if (((width > 1) && (width < 64)) ||
      ((height > 1) && (height < 64)))
    texture_target_mask &= ~GLITZ_TEXTURE_TARGET_2D_MASK;

  surface->base.texture =
    glitz_texture_generate (surface->base.gl,
                            width, height,
                            texture_format,
                            texture_target_mask);

  if (!surface->base.texture) {
    glitz_surface_pop_current (&surface->base);
    glitz_surface_destroy (&surface->base);
    return NULL;
  }
  
  if (thread_info->feature_mask & GLITZ_FEATURE_OFFSCREEN_DRAWING_MASK)
    surface->pbuffer = glitz_agl_pbuffer_create (surface->base.texture);
  
  _glitz_agl_set_features (surface);

  if (!surface->pbuffer) {
    glitz_texture_allocate (surface->base.gl, surface->base.texture);
  } else {
    glitz_surface_push_current (&surface->base,
                                GLITZ_CN_SURFACE_CONTEXT_CURRENT);
    glitz_agl_pbuffer_bind (surface->pbuffer,
                            surface->context->context,
                            surface->base.texture,
                            surface->base.format);
    glitz_surface_pop_current (&surface->base);
  }

  glitz_surface_pop_current (&surface->base);

  return &surface->base;
}

glitz_surface_t *
glitz_agl_surface_create (glitz_format_t *format,
                        int width,
                        int height)
{
  return _glitz_agl_surface_create (glitz_agl_thread_info_get (),
                                  format, width, height);
}
slim_hidden_def(glitz_agl_surface_create_offscreen);

glitz_surface_t *
glitz_agl_surface_create_for_window (glitz_format_t *format,
                                   WindowRef window)
{
  glitz_agl_surface_t *surface;
  glitz_agl_context_t *context;
  int width, height;
  glitz_agl_thread_info_t *thread_info = glitz_agl_thread_info_get ();

  context = glitz_agl_context_get (thread_info, format, 0);
  if (!context)
    return NULL;

  _glitz_agl_surface_update_size_for_window (window, &width, &height);

  surface = (glitz_agl_surface_t *) calloc (1, sizeof (glitz_agl_surface_t));
  if (surface == NULL)
    return NULL;

  glitz_surface_init (&surface->base, &glitz_agl_surface_backend);
  
  surface->thread_info = thread_info;
  surface->context = context;

  surface->base.gl = &_glitz_agl_gl_proc_address;
  surface->base.programs = &thread_info->programs;
  surface->base.format = format;
  surface->base.width = width;
  surface->base.height = height;

  _glitz_agl_set_features (surface);

  surface->window = window;
  surface->drawable = GetWindowPort (window);
  
  return &surface->base;
}
slim_hidden_def(glitz_agl_surface_create_for_window);

static glitz_surface_t *
_glitz_agl_surface_create_similar (void *abstract_templ,
                                   glitz_format_name_t format_name,
                                   glitz_bool_t drawable,
                                   int width,
                                   int height)
{
  glitz_agl_surface_t *templ = (glitz_agl_surface_t *) abstract_templ;
  
  if ((!drawable) ||
      (templ->thread_info->agl_feature_mask &
       GLITZ_AGL_FEATURE_PBUFFER_MASK)) {
    glitz_format_t *format;

    format = glitz_format_find_sufficient_standard
      (templ->base.format, 1, GLITZ_FORMAT_OPTION_OFFSCREEN_MASK, format_name);
    
    if (!format)
      format = glitz_format_find_standard (templ->thread_info->formats,
                                           templ->thread_info->n_formats,
                                           GLITZ_FORMAT_OPTION_OFFSCREEN_MASK,
                                           format_name);
    
    if (format)
      return _glitz_agl_surface_create (templ->thread_info, format,
                                        width, height);
  }
      
  return NULL;
}

static void
_glitz_agl_surface_destroy (void *abstract_surface)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;
  AGLContext context = aglGetCurrentContext ();
  
  if (context == surface->context->context) {
    if (surface->pbuffer) {
      AGLPbuffer pbuffer;
      GLuint unused;
      
      aglGetPBuffer (context, &pbuffer, &unused, &unused, &unused);
      
      if (pbuffer == surface->pbuffer)
        glitz_agl_context_make_current (surface);
    } else if (surface->drawable) {
      if (aglGetDrawable (context) == surface->drawable)
        glitz_agl_context_make_current (surface);
    }
  }
  
  if (surface->base.texture)
    glitz_texture_destroy (surface->base.gl, surface->base.texture);
  
  if (surface->pbuffer)
    glitz_agl_pbuffer_destroy (surface->pbuffer);
  
  glitz_surface_deinit (&surface->base);
  
  free (surface);
}

static void
_glitz_agl_surface_update_size (void *abstract_surface)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;
  
  if (surface->window) {
    _glitz_agl_surface_update_size_for_window (surface->window,
                                               &surface->base.width,
                                               &surface->base.height);
    
    glitz_agl_context_push_current (surface,
                                    GLITZ_CN_SURFACE_DRAWABLE_CURRENT);
    
    aglUpdateContext (surface->context->context);
    
    glitz_agl_context_pop_current (surface);
  }
}

static void
_glitz_agl_surface_flush (void *abstract_surface)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;
  
  if (!surface->window)
    return;

  glitz_agl_context_push_current (surface, GLITZ_CN_SURFACE_DRAWABLE_CURRENT);
  
  aglSwapBuffers (surface->context->context);
  
  glitz_agl_context_pop_current (surface);
}
