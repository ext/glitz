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

#include <stdlib.h>

static void
_glitz_glx_context_create_glx12 (glitz_glx_screen_info_t *screen_info,
                                 XID visualid,
                                 GLXContext share_list,
                                 glitz_glx_context_t *context)
{
  int vis_info_count, i;
  XVisualInfo *vis_infos;

  vis_infos = XGetVisualInfo (screen_info->display_info->display,
                              0, NULL, &vis_info_count);
  for (i = 0; i < vis_info_count; i++) {
    if (vis_infos[i].visual->visualid == visualid)
      break;
  }

  context->context = glXCreateContext (screen_info->display_info->display,
                                       &vis_infos[i], share_list, 1);
  context->id = visualid;  
  context->fbconfig = (XID) 0;

  XFree (vis_infos);
}

static void
_glitz_glx_context_create_glx13 (glitz_glx_screen_info_t *screen_info,
                                 XID fbconfigid,
                                 GLXContext share_list,
                                 glitz_glx_context_t *context)
{
  GLXFBConfig *fbconfigs;
  int i, n_fbconfigs;
  XVisualInfo *vinfo = NULL;
  glitz_glx_static_proc_address_list_t *glx =
    &screen_info->display_info->thread_info->glx;

  fbconfigs = glx->get_fbconfigs (screen_info->display_info->display,
                                  screen_info->screen, &n_fbconfigs);
  for (i = 0; i < n_fbconfigs; i++) {
    int value;
    
    glx->get_fbconfig_attrib (screen_info->display_info->display, fbconfigs[i],
                              GLX_FBCONFIG_ID, &value);
    if (value == (int) fbconfigid)
      break;
  }

  if (i < n_fbconfigs)
    vinfo = glx->get_visual_from_fbconfig (screen_info->display_info->display,
                                           fbconfigs[i]);
  
  if (vinfo) {
    context->context = glXCreateContext (screen_info->display_info->display,
                                         vinfo, share_list, 1);
    context->id = fbconfigid;
    context->fbconfig = fbconfigs[i];
    XFree (vinfo);
  } else {
    context->context = NULL;
    context->id = fbconfigid;
    context->fbconfig = NULL;
  }

  if (fbconfigs)
    XFree (fbconfigs);
}

int
glitz_glx_ensure_pbuffer_support (glitz_glx_screen_info_t *screen_info,
                                  XID fbconfigid)
{
  GLXFBConfig *fbconfigs;
  int i, n_fbconfigs;
  glitz_glx_static_proc_address_list_t *glx =
    &screen_info->display_info->thread_info->glx;
  int status = 1;
  
  fbconfigs = glx->get_fbconfigs (screen_info->display_info->display,
                                  screen_info->screen, &n_fbconfigs);
  for (i = 0; i < n_fbconfigs; i++) {
    int value;
    
    glx->get_fbconfig_attrib (screen_info->display_info->display, fbconfigs[i],
                              GLX_FBCONFIG_ID, &value);
    if (value == (int) fbconfigid)
      break;
  }
  
  if (i < n_fbconfigs) {
    GLXPbuffer pbuffer;
    glitz_texture_t texture;
    
    texture.width = texture.height = 1;
    pbuffer = glitz_glx_pbuffer_create (screen_info->display_info,
                                        fbconfigs[i],
                                        &texture);
    if (pbuffer) {
      glitz_glx_pbuffer_destroy (screen_info->display_info, pbuffer);
      
      status = 0;
    }
  }

  if (fbconfigs)
    XFree (fbconfigs);
  
  return status;
}

glitz_glx_context_t *
glitz_glx_context_get (glitz_glx_screen_info_t *screen_info,
                       glitz_format_t *format)
{
  glitz_glx_context_t *context;
  glitz_glx_context_t **contexts = screen_info->contexts;
  int index, n_contexts = screen_info->n_contexts;

  if (screen_info->format_ids[format->id] == (XID) 0)
    return &screen_info->root_context;

  for (; n_contexts; n_contexts--, contexts++)
    if ((*contexts)->id == screen_info->format_ids[format->id])
      return *contexts;

  index = screen_info->n_contexts++;

  screen_info->contexts =
    realloc (screen_info->contexts,
             sizeof (glitz_glx_context_t *) * screen_info->n_contexts);

  context = malloc (sizeof (glitz_glx_context_t));
  screen_info->contexts[index] = context;

  if (screen_info->glx_feature_mask & GLITZ_GLX_FEATURE_GLX13_MASK)
    _glitz_glx_context_create_glx13 (screen_info,
                                     screen_info->format_ids[format->id],
                                     screen_info->root_context.context,
                                     context);
  else
    _glitz_glx_context_create_glx12 (screen_info,
                                     screen_info->format_ids[format->id],
                                     screen_info->root_context.context,
                                     context);

  memcpy (&context->gl,
          &screen_info->root_context.gl,
          sizeof (glitz_gl_proc_address_list_t));
  memset (&context->glx, 0, sizeof (glitz_glx_proc_address_list_t));
  
  context->gl.need_lookup = context->glx.need_lookup = 1;
  
  return context;
}

void
glitz_glx_context_destroy (glitz_glx_screen_info_t *screen_info,
                           glitz_glx_context_t *context)
{
  glXDestroyContext (screen_info->display_info->display,
                     context->context);
  free (context);
}

void
glitz_glx_context_proc_address_lookup (glitz_glx_thread_info_t *thread_info,
                                       glitz_glx_context_t *context)
{
  context->glx.bind_tex_image_arb =
    (glitz_glx_bind_tex_image_arb_t)
    glitz_glx_get_proc_address (thread_info, "glXBindTexImageARB");
  context->glx.release_tex_image_arb =
    (glitz_glx_release_tex_image_arb_t)
    glitz_glx_get_proc_address (thread_info, "glXReleaseTexImageARB");

  context->gl.active_texture_arb =
    (glitz_gl_active_texture_arb_t)
    glitz_glx_get_proc_address (thread_info, "glActiveTextureARB");
  context->gl.multi_tex_coord_2d_arb =
    (glitz_gl_multi_tex_coord_2d_arb_t)
    glitz_glx_get_proc_address (thread_info, "glMultiTexCoord2dARB");

  context->gl.gen_programs_arb =
    (glitz_gl_gen_programs_arb_t)
    glitz_glx_get_proc_address (thread_info, "glGenProgramsARB");
  context->gl.delete_programs_arb =
    (glitz_gl_delete_programs_arb_t)
    glitz_glx_get_proc_address (thread_info, "glDeleteProgramsARB");
  context->gl.program_string_arb =
    (glitz_gl_program_string_arb_t)
    glitz_glx_get_proc_address (thread_info, "glProgramStringARB");
  context->gl.bind_program_arb =
    (glitz_gl_bind_program_arb_t)
    glitz_glx_get_proc_address (thread_info, "glBindProgramARB");
  context->gl.program_local_param_4d_arb =
    (glitz_gl_program_local_param_4d_arb_t)
    glitz_glx_get_proc_address (thread_info, "glProgramLocalParameter4dARB");
  context->gl.get_program_iv_arb =
    (glitz_gl_get_program_iv_arb_t)
    glitz_glx_get_proc_address (thread_info, "glGetProgramivARB");

  if (context->gl.get_program_iv_arb) {
    context->gl.get_program_iv_arb (GLITZ_GL_FRAGMENT_PROGRAM_ARB,
                                    GLITZ_GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB,
                                    &context->texture_indirections);
  }
  
  context->gl.need_lookup = 0;
  context->glx.need_lookup = 0;
}

static void
glitz_glx_context_set_surface_anti_aliasing (glitz_glx_surface_t *surface)
{
  if (surface->base.format->multisample.supported) {
    if (surface->base.polyedge == GLITZ_POLYEDGE_SMOOTH) {
      glEnable (GLITZ_GL_MULTISAMPLE_ARB);
      if (surface->screen_info->glx_feature_mask &
          GLITZ_GLX_FEATURE_MULTISAMPLE_FILTER_MASK)
        glHint (GLITZ_GL_MULTISAMPLE_FILTER_HINT_NV, GLITZ_GL_NICEST);
    } else
      glDisable (GLITZ_GL_MULTISAMPLE_ARB);
  }
}

void
glitz_glx_context_make_current (glitz_glx_surface_t *surface,
                                glitz_bool_t flush)
{
  GLXContext context;
  Drawable drawable;

  if (flush)
    glFlush ();
  
  if (!surface->drawable) {
    drawable = surface->screen_info->root_drawable;
    context = surface->screen_info->root_context.context;
  } else {
    context = surface->context->context;
    drawable = surface->drawable;
  }
  
  glXMakeCurrent (surface->screen_info->display_info->display,
                  drawable, context);

  if (surface->context->gl.need_lookup)
    glitz_glx_context_proc_address_lookup
      (surface->screen_info->display_info->thread_info, surface->context);
}

static void
glitz_glx_context_update (glitz_glx_surface_t *surface,
                          glitz_constraint_t constraint)
{
  GLXContext context = glXGetCurrentContext ();
  
  switch (constraint) {
  case GLITZ_CN_NONE:
    break;
  case GLITZ_CN_ANY_CONTEXT_CURRENT:
    if (context == NULL)
      glitz_glx_context_make_current (surface, 0);
    break;
  case GLITZ_CN_SURFACE_CONTEXT_CURRENT:
    if (context != surface->context->context)
      glitz_glx_context_make_current (surface, (context)? 1: 0);
    break;
  case GLITZ_CN_SURFACE_DRAWABLE_CURRENT:
    if ((context != surface->context->context) ||
        (glXGetCurrentDrawable () != surface->drawable))
      glitz_glx_context_make_current (surface, (context)? 1: 0);
    
    glitz_glx_context_set_surface_anti_aliasing (surface);
    break;
  }
}

glitz_glx_surface_t *
glitz_glx_context_push_current (glitz_glx_surface_t *surface,
                                glitz_constraint_t constraint)
{
  glitz_glx_screen_info_t *screen_info;
  glitz_glx_context_info_t *context_info;
  int index;

  screen_info = surface->screen_info;

  index = screen_info->context_stack_size++;

  screen_info->context_stack =
    realloc (screen_info->context_stack,
             sizeof (glitz_glx_context_info_t) *
             screen_info->context_stack_size);

  context_info = &screen_info->context_stack[index];
  context_info->surface = surface;
  context_info->constraint = constraint;
  
  glitz_glx_context_update (context_info->surface, constraint);

  if (context_info->constraint == GLITZ_CN_SURFACE_DRAWABLE_CURRENT)
    return context_info->surface;
  
  return NULL;
}

glitz_glx_surface_t *
glitz_glx_context_pop_current (glitz_glx_surface_t *surface)
{
  glitz_glx_screen_info_t *screen_info;
  glitz_glx_context_info_t *context_info = NULL;
  int index;

  screen_info = surface->screen_info;

  screen_info->context_stack_size--;
  index = screen_info->context_stack_size - 1;

  screen_info->context_stack =
    realloc (screen_info->context_stack,
             sizeof (glitz_glx_context_info_t) *
             screen_info->context_stack_size);
  
  context_info = &screen_info->context_stack[index];

  if (context_info->surface)
    glitz_glx_context_update (context_info->surface, context_info->constraint);
  
  if (context_info->constraint == GLITZ_CN_SURFACE_DRAWABLE_CURRENT)
    return context_info->surface;
  
  return NULL;
}
