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

static void
_glitz_agl_context_create (glitz_agl_thread_info_t *thread_info,
                           AGLPixelFormat pixel_format,
                           glitz_agl_context_t *context)
{
  context->context =
    aglCreateContext (pixel_format, thread_info->root_context.context);
  context->pixel_format = pixel_format;
}

glitz_agl_context_t *
glitz_agl_context_get (glitz_agl_thread_info_t *thread_info,
                       glitz_format_t *format,
                       glitz_bool_t offscreen)
{
  glitz_agl_context_t *context;
  glitz_agl_context_t **contexts = thread_info->contexts;
  int index, n_contexts = thread_info->n_contexts;

  if (thread_info->format_ids[format->id] == (AGLPixelFormat) 0)
    return &thread_info->root_context;

  for (; n_contexts; n_contexts--, contexts++)
    if ((*contexts)->pixel_format == thread_info->format_ids[format->id] &&
        (*contexts)->offscreen == offscreen)
      return *contexts;
  
  index = thread_info->n_contexts++;

  thread_info->contexts =
    realloc (thread_info->contexts,
             sizeof (glitz_agl_context_t *) * thread_info->n_contexts);

  context = malloc (sizeof (glitz_agl_context_t));
  thread_info->contexts[index] = context;

  _glitz_agl_context_create (thread_info,
                             thread_info->format_ids[format->id],
                             context);
  context->offscreen = offscreen;

  glitz_agl_surface_backend_init (&context->backend);

  memcpy (&context->backend.gl,
          &_glitz_agl_gl_proc_address,
          sizeof (glitz_gl_proc_address_list_t));
  
  context->backend.formats = thread_info->formats;
  context->backend.n_formats = thread_info->n_formats;
  context->backend.program_map = &thread_info->program_map;
  context->backend.feature_mask = thread_info->feature_mask;

  context->backend.gl.need_lookup = 1;
  
  return context;
}

void
glitz_agl_context_destroy (glitz_agl_thread_info_t *thread_info,
                           glitz_agl_context_t *context)
{
  aglDestroyContext (context->context);
  free (context);
}

/* These function addresses are never context specific but we retrive them
   for each context anyway. */
void
glitz_agl_context_proc_address_lookup (glitz_agl_thread_info_t *thread_info,
                                       glitz_agl_context_t *context)
{
  CFBundleRef bundle;
  
  bundle = glitz_agl_get_bundle ("OpenGL.framework");

  if (thread_info->gl_version >= 1.3) {
    context->backend.gl.active_texture =
      (glitz_gl_active_texture_t)
      glitz_agl_get_proc_address (bundle, "glActiveTexture");
  } else {
    context->backend.gl.active_texture =
      (glitz_gl_active_texture_t)
      glitz_agl_get_proc_address (bundle, "glActiveTextureARB");
  }
  
  context->backend.gl.gen_programs =
    (glitz_gl_gen_programs_t)
    glitz_agl_get_proc_address (bundle, "glGenProgramsARB");
  context->backend.gl.delete_programs =
    (glitz_gl_delete_programs_t)
    glitz_agl_get_proc_address (bundle, "glDeleteProgramsARB");
  context->backend.gl.program_string =
    (glitz_gl_program_string_t)
    glitz_agl_get_proc_address (bundle, "glProgramStringARB");
  context->backend.gl.bind_program =
    (glitz_gl_bind_program_t)
    glitz_agl_get_proc_address (bundle, "glBindProgramARB");
  context->backend.gl.program_local_param_4fv =
    (glitz_gl_program_local_param_4fv_t)
    glitz_agl_get_proc_address (bundle, "glProgramLocalParameter4fvARB");
  context->backend.gl.get_program_iv =
    (glitz_gl_get_program_iv_t)
    glitz_agl_get_proc_address (bundle, "glGetProgramivARB");

  if (thread_info->gl_version >= 1.5) {
    context->backend.gl.gen_buffers =
      (glitz_gl_gen_buffers_t)
      glitz_agl_get_proc_address (bundle, "glGenBuffers");
    context->backend.gl.delete_buffers =
      (glitz_gl_delete_buffers_t)
      glitz_agl_get_proc_address (bundle, "glDeleteBuffers");
    context->backend.gl.bind_buffer =
      (glitz_gl_bind_buffer_t)
      glitz_agl_get_proc_address (bundle, "glBindBuffer");
    context->backend.gl.buffer_data =
      (glitz_gl_buffer_data_t)
      glitz_agl_get_proc_address (bundle, "glBufferData");
    context->backend.gl.buffer_sub_data =
      (glitz_gl_buffer_sub_data_t)
      glitz_agl_get_proc_address (bundle, "glBufferSubData");
    context->backend.gl.get_buffer_sub_data =
      (glitz_gl_get_buffer_sub_data_t)
      glitz_agl_get_proc_address (bundle, "glGetBufferSubData");
    context->backend.gl.map_buffer =
      (glitz_gl_map_buffer_t)
      glitz_agl_get_proc_address (bundle, "glMapBuffer");
    context->backend.gl.unmap_buffer =
      (glitz_gl_unmap_buffer_t)
      glitz_agl_get_proc_address (bundle, "glUnmapBuffer");
  } else {
    context->backend.gl.gen_buffers =
      (glitz_gl_gen_buffers_t)
      glitz_agl_get_proc_address (bundle, "glGenBuffersARB");
    context->backend.gl.delete_buffers =
      (glitz_gl_delete_buffers_t)
      glitz_agl_get_proc_address (bundle, "glDeleteBuffersARB");
    context->backend.gl.bind_buffer =
      (glitz_gl_bind_buffer_t)
      glitz_agl_get_proc_address (bundle, "glBindBufferARB");
    context->backend.gl.buffer_data =
      (glitz_gl_buffer_data_t)
      glitz_agl_get_proc_address (bundle, "glBufferDataARB");
    context->backend.gl.buffer_sub_data =
      (glitz_gl_buffer_sub_data_t)
      glitz_agl_get_proc_address (bundle, "glBufferSubDataARB");
    context->backend.gl.get_buffer_sub_data =
      (glitz_gl_get_buffer_sub_data_t)
      glitz_agl_get_proc_address (bundle, "glGetBufferSubDataARB");
    context->backend.gl.map_buffer =
      (glitz_gl_map_buffer_t)
      glitz_agl_get_proc_address (bundle, "glMapBufferARB");
    context->backend.gl.unmap_buffer =
      (glitz_gl_unmap_buffer_t)
      glitz_agl_get_proc_address (bundle, "glUnmapBufferARB");
  }

  glitz_agl_release_bundle (bundle);

  context->backend.feature_mask &= ~GLITZ_FEATURE_MULTITEXTURE_MASK;
  context->backend.feature_mask &= ~GLITZ_FEATURE_PER_COMPONENT_RENDERING_MASK;
  context->backend.feature_mask &= ~GLITZ_FEATURE_FRAGMENT_PROGRAM_MASK;
  context->backend.feature_mask &= ~GLITZ_FEATURE_VERTEX_BUFFER_OBJECT_MASK;
  context->backend.feature_mask &= ~GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK;

  if (context->backend.gl.active_texture) {
    context->backend.feature_mask |= GLITZ_FEATURE_MULTITEXTURE_MASK;

    if (thread_info->feature_mask & GLITZ_FEATURE_PER_COMPONENT_RENDERING_MASK)
      context->backend.feature_mask |=
        GLITZ_FEATURE_PER_COMPONENT_RENDERING_MASK;
    
    if (context->backend.gl.gen_programs &&
        context->backend.gl.delete_programs &&
        context->backend.gl.program_string &&
        context->backend.gl.bind_program &&
        context->backend.gl.program_local_param_4fv) {
      if (thread_info->feature_mask & GLITZ_FEATURE_FRAGMENT_PROGRAM_MASK)
        context->backend.feature_mask |= GLITZ_FEATURE_FRAGMENT_PROGRAM_MASK;
    }
  }

  if (context->backend.gl.gen_buffers &&
      context->backend.gl.delete_buffers &&
      context->backend.gl.bind_buffer &&
      context->backend.gl.buffer_data &&
      context->backend.gl.buffer_sub_data &&
      context->backend.gl.get_buffer_sub_data &&
      context->backend.gl.map_buffer &&
      context->backend.gl.unmap_buffer) {
    if (thread_info->feature_mask & GLITZ_FEATURE_VERTEX_BUFFER_OBJECT_MASK)
      context->backend.feature_mask |= GLITZ_FEATURE_VERTEX_BUFFER_OBJECT_MASK;
    
    if (thread_info->feature_mask & GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK)
      context->backend.feature_mask |= GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK;
  }
  
  context->backend.gl.need_lookup = 0;
}

void
glitz_agl_context_make_current (glitz_agl_surface_t *surface,
                                glitz_bool_t flush)
{
  AGLContext context;
  AGLDrawable drawable = (AGLDrawable) 0;
  AGLPbuffer pbuffer = (AGLPbuffer) 0;

  if (flush)
    glFlush ();
  
  if ((!surface->drawable) && (!surface->pbuffer)) {
    context = surface->thread_info->root_context.context;
  } else {
    context = surface->context->context;
    pbuffer = surface->pbuffer;
    drawable = surface->drawable;
    surface->base.update_mask |= GLITZ_UPDATE_ALL_MASK;
  }

  if (pbuffer)
    aglSetPBuffer (context, pbuffer, 0, 0, aglGetVirtualScreen (context));
  else
    aglSetDrawable (context, drawable);

  aglSetCurrentContext (context);

  if (surface->context->backend.gl.need_lookup)
    glitz_agl_context_proc_address_lookup (surface->thread_info,
                                           surface->context);
}

static void
glitz_agl_context_update (glitz_agl_surface_t *surface,
                          glitz_constraint_t constraint)
{
  AGLContext context = aglGetCurrentContext ();

  switch (constraint) {
  case GLITZ_CN_NONE:
    break;
  case GLITZ_CN_ANY_CONTEXT_CURRENT:
    if (context == NULL)
      glitz_agl_context_make_current (surface, 0);
    break;
  case GLITZ_CN_SURFACE_CONTEXT_CURRENT:
    if (context != surface->context->context)
      glitz_agl_context_make_current (surface, (context)? 1: 0);
    break;
  case GLITZ_CN_SURFACE_DRAWABLE_CURRENT:
    if (context != surface->context->context) {
      glitz_agl_context_make_current (surface, (context)? 1: 0);
    } else {
      if (surface->pbuffer) {
        AGLPbuffer pbuffer;
        GLuint unused;

        aglGetPBuffer (surface->context->context, &pbuffer,
                       &unused, &unused, &unused);
        
        if (pbuffer != surface->pbuffer)
          glitz_agl_context_make_current (surface, (context)? 1: 0);
        
      } else if (surface->drawable) {
        if (aglGetDrawable (surface->context->context) != surface->drawable)
          glitz_agl_context_make_current (surface, (context)? 1: 0);
      }
    }
    break;
  }
}

glitz_agl_surface_t *
glitz_agl_context_push_current (glitz_agl_surface_t *surface,
                                glitz_constraint_t constraint)
{
  glitz_agl_thread_info_t *thread_info;
  glitz_agl_context_info_t *context_info;
  int index;

  thread_info = surface->thread_info;

  index = thread_info->context_stack_size++;
  
  context_info = &thread_info->context_stack[index];
  context_info->surface = surface;
  context_info->constraint = constraint;
  
  glitz_agl_context_update (context_info->surface, constraint);

  if (context_info->constraint == GLITZ_CN_SURFACE_DRAWABLE_CURRENT)
    return context_info->surface;
  
  return NULL;
}

glitz_agl_surface_t *
glitz_agl_context_pop_current (glitz_agl_surface_t *surface)
{
  glitz_agl_thread_info_t *thread_info;
  glitz_agl_context_info_t *context_info = NULL;
  int index;

  thread_info = surface->thread_info;

  thread_info->context_stack_size--;
  index = thread_info->context_stack_size - 1;

  context_info = &thread_info->context_stack[index];

  if (context_info->surface)
    glitz_agl_context_update (context_info->surface, context_info->constraint);
  
  if (context_info->constraint == GLITZ_CN_SURFACE_DRAWABLE_CURRENT)
    return context_info->surface;
  
  return NULL;
}
