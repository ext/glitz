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

static glitz_surface_t *
_glitz_glx_surface_create_similar (void *abstract_templ,
                                   glitz_format_t *format,
                                   int width,
                                   int height);

static void
_glitz_glx_surface_destroy (void *abstract_surface);

static glitz_texture_t *
_glitz_glx_surface_get_texture (void *abstract_surface,
                                glitz_bool_t allocate);

static void
_glitz_glx_surface_update_size (void *abstract_surface);

static void
_glitz_glx_surface_swap_buffers (void *abstract_surface);

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

static const struct glitz_surface_backend glitz_glx_surface_backend = {
  _glitz_glx_surface_create_similar,
  _glitz_glx_surface_destroy,
  _glitz_glx_surface_push_current,
  _glitz_glx_surface_pop_current,
  _glitz_glx_surface_get_texture,
  _glitz_glx_surface_update_size,
  _glitz_glx_surface_swap_buffers,
  _glitz_glx_surface_make_current_read
};

static glitz_bool_t
_glitz_glx_surface_update_size_for_window (Display *display,
                                           Window drawable,
                                           int *width,
                                           int *height)
{
  unsigned int uwidth, uheight, bwidth_ignore, depth_ignore;
  Window root_ignore;
  int x_ignore, y_ignore;
  
  if (!XGetGeometry (display, drawable, &root_ignore, &x_ignore, &y_ignore,
                     &uwidth, &uheight, &bwidth_ignore, &depth_ignore))
    return 0;
  
  *width = (int) uwidth;
  *height = (int) uheight;
  
  return 1;
}

static glitz_texture_t *
_glitz_glx_surface_get_texture (void *abstract_surface,
                                glitz_bool_t allocate)
{
  glitz_glx_surface_t *surface = (glitz_glx_surface_t *) abstract_surface;
  
  if (surface->base.hint_mask & GLITZ_INT_HINT_DIRTY_MASK) {
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
    surface->base.hint_mask &= ~GLITZ_INT_HINT_DIRTY_MASK;
  }

  if (allocate) {
    if (!surface->base.texture.allocated)
      glitz_texture_allocate (surface->base.gl, &surface->base.texture);
  }
  
  if (surface->base.texture.allocated)
    return &surface->base.texture;
  else
    return NULL;
}

static void
_glitz_glx_set_features (glitz_glx_surface_t *surface)
{
  surface->base.feature_mask = surface->screen_info->feature_mask;

  surface->base.feature_mask &= ~GLITZ_FEATURE_MULTITEXTURE_MASK;
  surface->base.feature_mask &= ~GLITZ_FEATURE_COMPONENT_ALPHA_MASK;
  surface->base.feature_mask &= ~GLITZ_FEATURE_VERTEX_PROGRAM_MASK;
  surface->base.feature_mask &= ~GLITZ_FEATURE_FRAGMENT_PROGRAM_MASK;
  surface->base.feature_mask &= ~GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK;

  if (surface->context->gl.need_lookup) {
    glitz_glx_context_push_current (surface, GLITZ_CN_SURFACE_CONTEXT_CURRENT);
    glitz_glx_context_pop_current (surface);
  }

  if (surface->context->gl.active_texture &&
      surface->context->gl.multi_tex_coord_2d) {
    surface->base.feature_mask |= GLITZ_FEATURE_MULTITEXTURE_MASK;

    if (surface->screen_info->feature_mask &
        GLITZ_FEATURE_COMPONENT_ALPHA_MASK)
      surface->base.feature_mask |= GLITZ_FEATURE_COMPONENT_ALPHA_MASK;

    if (surface->context->gl.gen_programs &&
        surface->context->gl.delete_programs &&
        surface->context->gl.program_string &&
        surface->context->gl.bind_program &&
        surface->context->gl.program_local_param_4d) {
      if (surface->screen_info->feature_mask &
          GLITZ_FEATURE_VERTEX_PROGRAM_MASK) {
        surface->base.feature_mask |= GLITZ_FEATURE_VERTEX_PROGRAM_MASK;
        
        if (surface->screen_info->feature_mask &
            GLITZ_FEATURE_FRAGMENT_PROGRAM_MASK)
          surface->base.feature_mask |= GLITZ_FEATURE_FRAGMENT_PROGRAM_MASK;
      }
    }
  }

  if (surface->context->gl.gen_buffers &&
      surface->context->gl.delete_buffers &&
      surface->context->gl.bind_buffer &&
      surface->context->gl.buffer_data &&
      surface->context->gl.map_buffer &&
      surface->context->gl.unmap_buffer)
    if (surface->screen_info->feature_mask &
        GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK)
      surface->base.feature_mask |= GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK;
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
                      &glitz_glx_surface_backend,
                      &context->gl,
                      format,
                      screen_info->formats,
                      screen_info->n_formats,
                      width,
                      height,
                      &screen_info->program_map,
                      screen_info->texture_mask);
  
  surface->screen_info = screen_info;
  surface->context = context;
  
  surface->base.hint_mask |= GLITZ_HINT_OFFSCREEN_MASK;

  if (format->draw.offscreen)
    surface->base.hint_mask |= GLITZ_INT_HINT_DRAWABLE_MASK;

  _glitz_glx_set_features (surface);

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
    _glitz_glx_surface_create (glitz_glx_screen_info_get
                               (display, screen),
                               format, width, height);
}
slim_hidden_def(glitz_glx_surface_create);

glitz_surface_t *
glitz_glx_surface_create_for_window (Display *display,
                                     int screen,
                                     glitz_format_t *format,
                                     Window window)
{
  glitz_glx_surface_t *surface;
  glitz_glx_context_t *context;
  int width, height;
  glitz_glx_screen_info_t *screen_info =
    glitz_glx_screen_info_get (display, screen);
  
  context = glitz_glx_context_get (screen_info, format);
  if (!context)
    return NULL;

  if (!_glitz_glx_surface_update_size_for_window (display, window,
                                                  &width, &height))
    return NULL;

  surface = (glitz_glx_surface_t *) calloc (1, sizeof (glitz_glx_surface_t));
  if (surface == NULL)
    return NULL;

  glitz_surface_init (&surface->base,
                      &glitz_glx_surface_backend,
                      &context->gl,
                      format,
                      screen_info->formats,
                      screen_info->n_formats,
                      width,
                      height,
                      &screen_info->program_map,
                      screen_info->texture_mask);
  
  surface->screen_info = screen_info;
  surface->context = context;
  surface->drawable = window;

  surface->base.hint_mask |= GLITZ_INT_HINT_DRAWABLE_MASK;

  _glitz_glx_set_features (surface);
  
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
_glitz_glx_surface_update_size (void *abstract_surface)
{
  glitz_glx_surface_t *surface = (glitz_glx_surface_t *) abstract_surface;
  
  if ((!surface->pbuffer) && surface->drawable) {
    int width, height;
    
    _glitz_glx_surface_update_size_for_window
      (surface->screen_info->display_info->display, surface->drawable,
       &width, &height);

    if (width != surface->base.width || height != surface->base.height) {
      glitz_texture_t texture;

      glitz_texture_init (&texture,
                          width, height,
                          surface->base.texture.format,
                          surface->screen_info->texture_mask);
      
      if (texture.width != surface->base.texture.width ||
          texture.height != surface->base.texture.height ||
          texture.target != surface->base.texture.target) {
        texture.name = surface->base.texture.name;
        surface->base.texture = texture;
      }
      
      surface->base.width = width;
      surface->base.height = height;
    }
  }
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
