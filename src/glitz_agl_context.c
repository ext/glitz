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
  
  return context;
}

void
glitz_agl_context_destroy (glitz_agl_thread_info_t *thread_info,
                           glitz_agl_context_t *context)
{
  aglDestroyContext (context->context);
  free (context);
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
