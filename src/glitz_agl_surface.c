/*
 * Copyright � 2004 David Reveman, Peter Nilsson
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

static glitz_texture_t *
_glitz_agl_surface_get_texture (void *abstract_surface,
                                glitz_bool_t allocate) {
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;

  if (surface->base.flags & GLITZ_SURFACE_FLAG_DIRTY_MASK) {
    glitz_bounding_box_t copy_box;

    copy_box.x1 = copy_box.y1 = 0;
    copy_box.x2 = surface->base.width;
    copy_box.y2 = surface->base.height;
    glitz_intersect_bounding_box (&surface->base.dirty_box,
                                  &copy_box, &copy_box);

    if (!(TEXTURE_ALLOCATED (&surface->base.texture)))
      glitz_texture_allocate (&surface->base.backend->gl,
                              &surface->base.texture);  
      
    glitz_texture_copy_surface (&surface->base.texture, &surface->base,
                                copy_box.x1,
                                copy_box.y1,
                                copy_box.x2 - copy_box.x1,
                                copy_box.y2 - copy_box.y1,
                                copy_box.x1,
                                copy_box.y1);
    
    surface->base.flags &= ~GLITZ_SURFACE_FLAG_DIRTY_MASK;
  }

  if (allocate) {
    if (!(TEXTURE_ALLOCATED (&surface->base.texture)))
      glitz_texture_allocate (&surface->base.backend->gl,
                              &surface->base.texture);
  }
  
  if (TEXTURE_ALLOCATED (&surface->base.texture))
    return &surface->base.texture;
  else
    return NULL;
}

static glitz_surface_t *
_glitz_agl_surface_create (glitz_agl_thread_info_t *thread_info,
                           glitz_format_t *format,
                           int width,
                           int height)
{
  glitz_agl_surface_t *surface;
  glitz_agl_context_t *context;

  if (width <= 0 || height <= 0)
    return NULL;

  context = glitz_agl_context_get (thread_info, format, 1);
  if (!context)
    return NULL;

  surface = (glitz_agl_surface_t *) calloc (1, sizeof (glitz_agl_surface_t));
  if (surface == NULL)
    return NULL;
  
  glitz_surface_init (&surface->base,
                      &context->backend,
                      format,
                      width,
                      height);
  
  surface->thread_info = thread_info;
  surface->context = context;

  surface->base.flags |= GLITZ_SURFACE_FLAG_OFFSCREEN_MASK;

  if (format->draw.offscreen)
    surface->base.flags |= GLITZ_SURFACE_FLAG_DRAWABLE_MASK;

  if (surface->context->backend.gl.need_lookup) {
    glitz_agl_context_push_current (surface, GLITZ_CN_SURFACE_CONTEXT_CURRENT);
    glitz_agl_context_pop_current (surface);
  }

  if (width > 64 || height > 64) {
    glitz_agl_context_push_current (surface, GLITZ_CN_ANY_CONTEXT_CURRENT);
    glitz_texture_size_check (&surface->base.backend->gl,
                              &surface->base.texture,
                              context->max_texture_2d_size,
                              context->max_texture_rect_size);
    glitz_agl_context_pop_current (surface);
    if (TEXTURE_INVALID_SIZE (&surface->base.texture) ||
        (format->draw.offscreen &&
         ((width > context->max_viewport_dims[0]) ||
          (height > context->max_viewport_dims[1])))) {
      glitz_surface_destroy (&surface->base);
      return NULL;
    }
  }

  return &surface->base;
}

glitz_surface_t *
glitz_agl_surface_create (glitz_format_t *format,
                          int width,
                          int height)
{
  glitz_agl_thread_info_t *thread_info;

  thread_info = glitz_agl_thread_info_get ();
  if (!thread_info)
    return NULL;
  
  return _glitz_agl_surface_create (thread_info, format, width, height);
}
slim_hidden_def(glitz_agl_surface_create_offscreen);

glitz_surface_t *
glitz_agl_surface_create_for_window (glitz_format_t *format,
                                     WindowRef window,
                                     int width,
                                     int height)
{
  glitz_agl_surface_t *surface;
  glitz_agl_context_t *context;
  glitz_agl_thread_info_t *thread_info;
  AGLDrawable drawable;

  if (width <= 0 || height <= 0)
    return NULL;

  drawable = GetWindowPort (window);
  if (!drawable)
    return NULL;

  thread_info = glitz_agl_thread_info_get ();
  if (!thread_info)
    return NULL;

  context = glitz_agl_context_get (thread_info, format, 0);
  if (!context)
    return NULL;

  surface = (glitz_agl_surface_t *) calloc (1, sizeof (glitz_agl_surface_t));
  if (surface == NULL)
    return NULL;

  glitz_surface_init (&surface->base,
                      &context->backend,
                      format,
                      width,
                      height);
  
  surface->thread_info = thread_info;
  surface->context = context;
  surface->window = window;
  surface->drawable = drawable;

  surface->base.flags |= GLITZ_SURFACE_FLAG_DRAWABLE_MASK;

  if (surface->context->backend.gl.need_lookup) {
    glitz_agl_context_push_current (surface, GLITZ_CN_SURFACE_CONTEXT_CURRENT);
    glitz_agl_context_pop_current (surface);
  }

  if (width > 64 || height > 64) {
    glitz_agl_context_push_current (surface, GLITZ_CN_ANY_CONTEXT_CURRENT);
    glitz_texture_size_check (&surface->base.backend->gl,
                              &surface->base.texture,
                              context->max_texture_2d_size,
                              context->max_texture_rect_size);
    glitz_agl_context_pop_current (surface);
    if (TEXTURE_INVALID_SIZE (&surface->base.texture) ||
        (width > context->max_viewport_dims[0]) ||
        (height > context->max_viewport_dims[1])) {
      glitz_surface_destroy (&surface->base);
      return NULL;
    }
  }
  
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
_glitz_agl_surface_swap_buffers (void *abstract_surface)
{
  glitz_agl_surface_t *surface = (glitz_agl_surface_t *) abstract_surface;
  
  glitz_agl_context_push_current (surface, GLITZ_CN_SURFACE_DRAWABLE_CURRENT);

  aglSwapBuffers (surface->context->context);
  
  glitz_agl_context_pop_current (surface);
}

void
glitz_agl_surface_backend_init (glitz_surface_backend_t *backend)
{
  backend->create_similar = _glitz_agl_surface_create_similar;
  backend->destroy = _glitz_agl_surface_destroy;
  backend->push_current = _glitz_agl_surface_push_current;
  backend->pop_current = _glitz_agl_surface_pop_current;
  backend->get_texture = _glitz_agl_surface_get_texture;
  backend->swap_buffers = _glitz_agl_surface_swap_buffers;
  backend->make_current_read = _glitz_agl_surface_make_current_read;
}
