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

#include "glitz_glxint.h"

static glitz_bool_t
_glitz_glx_surface_push_current (void *abstract_surface,
                                 glitz_constraint_t constraint)
{
  glitz_glx_surface_t *surface = (glitz_glx_surface_t *) abstract_surface;
  glitz_bool_t success = 1;

  if (constraint == GLITZ_CN_SURFACE_DRAWABLE_CURRENT &&
      (!surface->drawable)) {
    if (surface->base.format->draw.offscreen) {
      surface->drawable = surface->pbuffer =
        glitz_glx_pbuffer_create (surface->screen_info->display_info,
                                  surface->context->fbconfig,
                                  &surface->base.texture);
    } else {
      constraint = GLITZ_CN_ANY_CONTEXT_CURRENT;
      success = 0;
    }
  }
  
  surface = glitz_glx_context_push_current (surface, constraint);

  if (surface) {
    glitz_surface_update_state (&surface->base);
    return 1;
  }

  return success;
}

static void
_glitz_glx_surface_pop_current (void *abstract_surface)
{
  glitz_glx_surface_t *surface = (glitz_glx_surface_t *) abstract_surface;

  surface = glitz_glx_context_pop_current (surface);
  
  if (surface)
    glitz_surface_update_state (&surface->base);
}

static glitz_bool_t
_glitz_glx_surface_make_current_read (void *abstract_surface)
{
  /* This doesn't seem to work. 
  glitz_glx_surface_t *surface = (glitz_glx_surface_t *) abstract_surface;
  glitz_glx_static_proc_address_list_t *glx =
    &surface->screen_info->display_info->thread_info->glx;
    
  if (glx->make_context_current && surface->drawable) {
    GLXContext context = glXGetCurrentContext ();

    if (context == surface->context->context)
      return
        glx->make_context_current (surface->screen_info->display_info->display,
                                   glXGetCurrentDrawable (),
                                   surface->drawable,
                                   context);
  }
  */

  return 0;
}

static glitz_texture_t *
_glitz_glx_surface_get_texture (void *abstract_surface,
                                glitz_bool_t allocate)
{
  glitz_glx_surface_t *surface = (glitz_glx_surface_t *) abstract_surface;
  
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
_glitz_glx_surface_create (glitz_glx_screen_info_t *screen_info,
                           glitz_format_t *format,
                           int width,
                           int height)
{
  glitz_glx_surface_t *surface;
  glitz_glx_context_t *context;

  context = glitz_glx_context_get (screen_info, format);
  if (!context)
    return NULL;
  
  surface = (glitz_glx_surface_t *) calloc (1, sizeof (glitz_glx_surface_t));
  if (surface == NULL)
    return NULL;

  glitz_surface_init (&surface->base,
                      &context->backend,
                      format,
                      width,
                      height);
  
  surface->screen_info = screen_info;
  surface->context = context;
  
  surface->base.flags |= GLITZ_SURFACE_FLAG_OFFSCREEN_MASK;

  if (format->draw.offscreen)
    surface->base.flags |= GLITZ_SURFACE_FLAG_DRAWABLE_MASK;

  if (surface->context->backend.gl.need_lookup) {
    glitz_glx_context_push_current (surface, GLITZ_CN_SURFACE_CONTEXT_CURRENT);
    glitz_glx_context_pop_current (surface);
  }

  return &surface->base;
}

glitz_surface_t *
glitz_glx_surface_create (Display *display,
                          int screen,
                          glitz_format_t *format,
                          int width,
                          int height)
{

  return
    _glitz_glx_surface_create (glitz_glx_screen_info_get (display, screen),
                               format, width, height);
}
slim_hidden_def(glitz_glx_surface_create);

glitz_surface_t *
glitz_glx_surface_create_for_window (Display *display,
                                     int screen,
                                     glitz_format_t *format,
                                     Window window,
                                     int width,
                                     int height)
{
  glitz_glx_surface_t *surface;
  glitz_glx_context_t *context;
  glitz_glx_screen_info_t *screen_info =
    glitz_glx_screen_info_get (display, screen);
  
  context = glitz_glx_context_get (screen_info, format);
  if (!context)
    return NULL;

  surface = (glitz_glx_surface_t *) calloc (1, sizeof (glitz_glx_surface_t));
  if (surface == NULL)
    return NULL;

  glitz_surface_init (&surface->base,
                      &context->backend,
                      format,
                      width,
                      height);
  
  surface->screen_info = screen_info;
  surface->context = context;
  surface->drawable = window;

  surface->base.flags |= GLITZ_SURFACE_FLAG_DRAWABLE_MASK;

  if (surface->context->backend.gl.need_lookup) {
    glitz_glx_context_push_current (surface, GLITZ_CN_SURFACE_CONTEXT_CURRENT);
    glitz_glx_context_pop_current (surface);
  }
  
  return &surface->base;
}
slim_hidden_def(glitz_glx_surface_create_for_window);

static glitz_surface_t *
_glitz_glx_surface_create_similar (void *abstract_templ,
                                   glitz_format_t *format,
                                   int width,
                                   int height)
{
  glitz_glx_surface_t *templ = (glitz_glx_surface_t *) abstract_templ;

  if (!format->read.offscreen)
    return NULL;
  
  return _glitz_glx_surface_create (templ->screen_info, format, width, height);
}

static void
_glitz_glx_surface_destroy (void *abstract_surface)
{
  glitz_glx_surface_t *surface = (glitz_glx_surface_t *) abstract_surface;

  glitz_surface_fini (&surface->base);
  
  if (surface->drawable &&
      (glXGetCurrentDrawable () == surface->drawable)) {
    surface->drawable = None;
    glitz_glx_context_make_current (surface, 0);
  }

  if (surface->pbuffer)
    glitz_glx_pbuffer_destroy (surface->screen_info->display_info,
                               surface->pbuffer);
  
  free (surface);
}

static void
_glitz_glx_surface_swap_buffers (void *abstract_surface)
{
  glitz_glx_surface_t *surface = (glitz_glx_surface_t *) abstract_surface;

  if (surface->pbuffer || (!surface->drawable))
    return;

  glitz_glx_context_push_current (surface, GLITZ_CN_SURFACE_DRAWABLE_CURRENT);
  
  glXSwapBuffers (surface->screen_info->display_info->display,
                  surface->drawable);
  
  glitz_glx_context_pop_current (surface);
}

void
glitz_glx_surface_backend_init (glitz_surface_backend_t *backend)
{
  backend->create_similar = _glitz_glx_surface_create_similar;
  backend->destroy = _glitz_glx_surface_destroy;
  backend->push_current = _glitz_glx_surface_push_current;
  backend->pop_current = _glitz_glx_surface_pop_current;
  backend->get_texture = _glitz_glx_surface_get_texture;
  backend->swap_buffers = _glitz_glx_surface_swap_buffers;
  backend->make_current_read = _glitz_glx_surface_make_current_read;
}
