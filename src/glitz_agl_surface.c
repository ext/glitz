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
                                   glitz_format_t *format,
                                   int width,
                                   int height);

static void
_glitz_agl_surface_destroy (void *abstract_surface);

static glitz_texture_t *
_glitz_agl_surface_get_texture (void *abstract_surface,
                                glitz_bool_t allocate);

static void
_glitz_agl_surface_update_size (void *abstract_surface);

static void
_glitz_agl_surface_swap_buffers (void *abstract_surface);

static glitz_bool_t
_glitz_agl_surface_push_current (void *abstract_surface,
                                 glitz_constraint_t constraint)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;
  glitz_bool_t success = 1;
  
  if (constraint == GLITZ_CN_SURFACE_DRAWABLE_CURRENT &&
      (!surface->pbuffer) && (!surface->drawable)) {
    if (surface->base.format->draw.offscreen) {
      surface->pbuffer = glitz_agl_pbuffer_create (&surface->base.texture);
    } else {
      constraint = GLITZ_CN_ANY_CONTEXT_CURRENT;
      success = 0;
    }
  }
  
  surface = glitz_agl_context_push_current (surface, constraint);

  if (surface) {
    glitz_surface_update_state (&surface->base);
    return 1;
  }

  return success;
}

static void
_glitz_agl_surface_pop_current (void *abstract_surface)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;

  surface = glitz_agl_context_pop_current (surface);
  
  if (surface)
    glitz_surface_update_state (&surface->base);
}

static glitz_bool_t
_glitz_agl_surface_make_current_read (void *abstract_surface)
{
  return 0;
}

static const struct glitz_surface_backend glitz_agl_surface_backend = {
  _glitz_agl_surface_create_similar,
  _glitz_agl_surface_destroy,
  _glitz_agl_surface_push_current,
  _glitz_agl_surface_pop_current,
  _glitz_agl_surface_get_texture,
  _glitz_agl_surface_update_size,
  _glitz_agl_surface_swap_buffers,
  _glitz_agl_surface_make_current_read
};

static glitz_texture_t *
_glitz_agl_surface_get_texture (void *abstract_surface,
                                glitz_bool_t allocate) {
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;

  if (surface->base.hint_mask & GLITZ_INT_HINT_DIRTY_MASK) {
    if (surface->pbuffer) {
      surface->base.hint_mask &= ~GLITZ_INT_HINT_DIRTY_MASK;

      if (surface->base.read_buffer != surface->bound_buffer) {
        glitz_surface_push_current (&surface->base,
                                    GLITZ_CN_SURFACE_DRAWABLE_CURRENT);
        glitz_agl_pbuffer_bind (surface->pbuffer,
                                surface->context->context,
                                &surface->base.texture,
                                surface->base.read_buffer);
        surface->bound_buffer = surface->base.read_buffer;
        glitz_surface_pop_current (&surface->base);
      }

      return &surface->base.texture;
    } else {
      glitz_bounding_box_t copy_box;

      copy_box.x1 = copy_box.y1 = 0;
      copy_box.x2 = surface->base.width;
      copy_box.y2 = surface->base.height;
      glitz_intersect_bounding_box (&surface->base.dirty_box,
                                    &copy_box, &copy_box);
      
      if (!surface->base.texture.allocated)
        glitz_texture_allocate (surface->base.gl, &surface->base.texture);
      
      glitz_texture_copy_surface (&surface->base.texture, &surface->base,
                                  copy_box.x1,
                                  copy_box.y1,
                                  copy_box.x2 - copy_box.x1,
                                  copy_box.y2 - copy_box.y1,
                                  copy_box.x1,
                                  copy_box.y1);
    }
    
    surface->base.hint_mask &= ~GLITZ_INT_HINT_DIRTY_MASK;
  }

  if (allocate) {
    if (!surface->base.texture.allocated) {
      if (SURFACE_RENDER_TEXTURE (&surface->base)) {
        if (!surface->pbuffer)
          surface->pbuffer = glitz_agl_pbuffer_create (&surface->base.texture);

        glitz_surface_push_current (&surface->base,
                                    GLITZ_CN_SURFACE_DRAWABLE_CURRENT);
        glitz_agl_pbuffer_bind (surface->pbuffer,
                                surface->context->context,
                                &surface->base.texture,
                                surface->base.read_buffer);
        surface->bound_buffer = surface->base.read_buffer;
        glitz_surface_pop_current (&surface->base);
      } else
        glitz_texture_allocate (surface->base.gl, &surface->base.texture);
    }
  }

  if (surface->base.texture.allocated)
    return &surface->base.texture;
  else
    return NULL;
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
}

static glitz_surface_t *
_glitz_agl_surface_create (glitz_agl_thread_info_t *thread_info,
                           glitz_format_t *format,
                           int width,
                           int height)
{
  glitz_agl_surface_t *surface;
  glitz_agl_context_t *context;
  unsigned long texture_mask;

  context = glitz_agl_context_get (thread_info, format, 1);
  if (!context)
    return NULL;

  surface = (glitz_agl_surface_t *) calloc (1, sizeof (glitz_agl_surface_t));
  if (surface == NULL)
    return NULL;

  texture_mask = thread_info->texture_mask;

  /* Seems to be a problem with binding a pbuffer to some power of two sized
     textures. This will try to avoid the problem. */
  if (((width > 1) && (width < 64)) ||
      ((height > 1) && (height < 64))) {
    if (texture_mask != GLITZ_TEXTURE_TARGET_2D_MASK)
      texture_mask &= ~GLITZ_TEXTURE_TARGET_2D_MASK;
  }

  glitz_surface_init (&surface->base,
                      &glitz_agl_surface_backend,
                      &_glitz_agl_gl_proc_address,
                      format,
                      thread_info->formats,
                      thread_info->n_formats,
                      width,
                      height,
                      &thread_info->program_map,
                      texture_mask);
  
  surface->thread_info = thread_info;
  surface->context = context;

  surface->base.hint_mask |= GLITZ_HINT_OFFSCREEN_MASK;

  if (format->draw.offscreen) {
    surface->base.hint_mask |= GLITZ_INT_HINT_DRAWABLE_MASK;
    surface->base.hint_mask |= GLITZ_INT_HINT_RENDER_TEXTURE_MASK;
  }

  _glitz_agl_set_features (surface);

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

  glitz_surface_init (&surface->base,
                      &glitz_agl_surface_backend,
                      &_glitz_agl_gl_proc_address,
                      format,
                      thread_info->formats,
                      thread_info->n_formats,
                      width,
                      height,
                      &thread_info->program_map,
                      thread_info->texture_mask);
  
  surface->thread_info = thread_info;
  surface->context = context;
  surface->window = window;
  surface->drawable = GetWindowPort (window);

  surface->base.hint_mask |= GLITZ_INT_HINT_DRAWABLE_MASK;

  _glitz_agl_set_features (surface);
  
  return &surface->base;
}
slim_hidden_def(glitz_agl_surface_create_for_window);

static glitz_surface_t *
_glitz_agl_surface_create_similar (void *abstract_templ,
                                   glitz_format_t *format,
                                   int width,
                                   int height)
{
  glitz_agl_surface_t *templ = (glitz_agl_surface_t *) abstract_templ;

  if (!format->read.offscreen)
    return NULL;
  
  return _glitz_agl_surface_create (templ->thread_info, format, width, height);
}

static void
_glitz_agl_surface_destroy (void *abstract_surface)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;

  glitz_surface_fini (&surface->base);
  
  if (surface->drawable || surface->pbuffer) {
    AGLContext context = aglGetCurrentContext ();
    
    if (context == surface->context->context) { 
      if (surface->pbuffer) {
        AGLPbuffer pbuffer;
        GLuint unused;
        
        aglGetPBuffer (context, &pbuffer, &unused, &unused, &unused);
        
        if (pbuffer == surface->pbuffer)
          glitz_agl_context_make_current (surface, 0);
      } else {
        if (aglGetDrawable (context) == surface->drawable)
          glitz_agl_context_make_current (surface, 0);
      }
    }
    
    if (surface->pbuffer)
      glitz_agl_pbuffer_destroy (surface->pbuffer);
  }
  
  free (surface);
}

static void
_glitz_agl_surface_update_size (void *abstract_surface)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;
  
  if (surface->window) {
    int width, height;
    
    _glitz_agl_surface_update_size_for_window (surface->window,
                                               &width, &height);

    if (width != surface->base.width || height != surface->base.height) {
      glitz_texture_t texture;

      glitz_texture_init (&texture,
                          width, height,
                          surface->base.texture.format,
                          surface->thread_info->texture_mask);
      
      if (texture.width != surface->base.texture.width ||
          texture.height != surface->base.texture.height ||
          texture.target != surface->base.texture.target) {
        texture.name = surface->base.texture.name;
        surface->base.texture = texture;
      }
      
      surface->base.width = width;
      surface->base.height = height;
    
      glitz_agl_context_push_current (surface,
                                      GLITZ_CN_SURFACE_DRAWABLE_CURRENT);
    
      aglUpdateContext (surface->context->context);
    
      glitz_agl_context_pop_current (surface);
    }
  }
}

static void
_glitz_agl_surface_swap_buffers (void *abstract_surface)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;
  
  glitz_agl_context_push_current (surface, GLITZ_CN_SURFACE_DRAWABLE_CURRENT);

  aglSwapBuffers (surface->context->context);
  
  glitz_agl_context_pop_current (surface);
}
