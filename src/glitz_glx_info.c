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

#include <string.h>
#include <dlfcn.h>

glitz_gl_proc_address_list_t _glitz_glx_gl_proc_address = {
  (glitz_gl_enable_t) glEnable,
  (glitz_gl_disable_t) glDisable,
  (glitz_gl_enable_client_state_t) glEnableClientState,
  (glitz_gl_disable_client_state_t) glDisableClientState,
  (glitz_gl_vertex_pointer_t) glVertexPointer,
  (glitz_gl_draw_arrays_t) glDrawArrays,
  (glitz_gl_tex_env_f_t) glTexEnvf,
  (glitz_gl_tex_env_fv_t) glTexEnvfv,
  (glitz_gl_tex_gen_i_t) glTexGeni,
  (glitz_gl_tex_gen_fv_t) glTexGenfv,
  (glitz_gl_color_4us_t) glColor4us,
  (glitz_gl_color_4f_t) glColor4f,
  (glitz_gl_scissor_t) glScissor,
  (glitz_gl_blend_func_t) glBlendFunc,
  (glitz_gl_clear_t) glClear,
  (glitz_gl_clear_color_t) glClearColor,
  (glitz_gl_clear_stencil_t) glClearStencil,
  (glitz_gl_stencil_func_t) glStencilFunc,
  (glitz_gl_stencil_op_t) glStencilOp,
  (glitz_gl_push_attrib_t) glPushAttrib,
  (glitz_gl_pop_attrib_t) glPopAttrib,
  (glitz_gl_matrix_mode_t) glMatrixMode,
  (glitz_gl_push_matrix_t) glPushMatrix,
  (glitz_gl_pop_matrix_t) glPopMatrix,
  (glitz_gl_load_identity_t) glLoadIdentity,
  (glitz_gl_load_matrix_f_t) glLoadMatrixf,
  (glitz_gl_depth_range_t) glDepthRange,
  (glitz_gl_viewport_t) glViewport,
  (glitz_gl_raster_pos_2f_t) glRasterPos2f,
  (glitz_gl_bitmap_t) glBitmap,
  (glitz_gl_read_buffer_t) glReadBuffer,
  (glitz_gl_draw_buffer_t) glDrawBuffer,
  (glitz_gl_copy_pixels_t) glCopyPixels,
  (glitz_gl_flush_t) glFlush,
  (glitz_gl_finish_t) glFinish,
  (glitz_gl_pixel_store_i_t) glPixelStorei,
  (glitz_gl_ortho_t) glOrtho,
  (glitz_gl_scale_f_t) glScalef,
  (glitz_gl_translate_f_t) glTranslatef,
  (glitz_gl_hint_t) glHint,
  (glitz_gl_depth_mask_t) glDepthMask,
  (glitz_gl_polygon_mode_t) glPolygonMode,
  (glitz_gl_shade_model_t) glShadeModel,
  (glitz_gl_color_mask_t) glColorMask,
  (glitz_gl_read_pixels_t) glReadPixels,
  (glitz_gl_get_tex_image_t) glGetTexImage,
  (glitz_gl_tex_sub_image_2d_t) glTexSubImage2D,
  (glitz_gl_gen_textures_t) glGenTextures,
  (glitz_gl_delete_textures_t) glDeleteTextures,
  (glitz_gl_bind_texture_t) glBindTexture,
  (glitz_gl_tex_image_2d_t) glTexImage2D,
  (glitz_gl_tex_parameter_i_t) glTexParameteri,
  (glitz_gl_get_tex_level_parameter_iv_t) glGetTexLevelParameteriv,
  (glitz_gl_copy_tex_sub_image_2d_t) glCopyTexSubImage2D,
  (glitz_gl_get_integer_v_t) glGetIntegerv,

  (glitz_gl_active_texture_t) 0,
  (glitz_gl_gen_programs_t) 0,
  (glitz_gl_delete_programs_t) 0,
  (glitz_gl_program_string_t) 0,
  (glitz_gl_bind_program_t) 0,
  (glitz_gl_program_local_param_4fv_t) 0,
  (glitz_gl_get_program_iv_t) 0,
  (glitz_gl_gen_buffers_t) 0,
  (glitz_gl_delete_buffers_t) 0,
  (glitz_gl_bind_buffer_t) 0,
  (glitz_gl_buffer_data_t) 0,
  (glitz_gl_buffer_sub_data_t) 0,
  (glitz_gl_get_buffer_sub_data_t) 0,
  (glitz_gl_map_buffer_t) 0,
  (glitz_gl_unmap_buffer_t) 0,
  
  1
};

glitz_function_pointer_t
glitz_glx_get_proc_address (glitz_glx_thread_info_t *info, const char *name)
{
  glitz_function_pointer_t address = NULL;
  
  if (info->glx.get_proc_address)
    address = info->glx.get_proc_address ((glitz_gl_ubyte_t *) name);
  
  if (!address) {
    if (!info->dlhand)
      info->dlhand = dlopen (info->gl_library, RTLD_LAZY);

    if (info->dlhand) {
      dlerror ();
      address = (glitz_function_pointer_t) dlsym (info->dlhand, name);
      if (dlerror () != NULL)
        address = NULL;
    }
  }
  
  return address;
}

static void
glitz_glx_proc_address_lookup (glitz_glx_thread_info_t *info)
{
  info->glx.get_fbconfigs = (glitz_glx_get_fbconfigs_t)
    glitz_glx_get_proc_address (info, "glXGetFBConfigs");
  info->glx.get_fbconfig_attrib = (glitz_glx_get_fbconfig_attrib_t)
    glitz_glx_get_proc_address (info, "glXGetFBConfigAttrib");
  info->glx.get_visual_from_fbconfig = (glitz_glx_get_visual_from_fbconfig_t)
    glitz_glx_get_proc_address (info, "glXGetVisualFromFBConfig");
  info->glx.create_pbuffer = (glitz_glx_create_pbuffer_t)
    glitz_glx_get_proc_address (info, "glXCreatePbuffer");
  info->glx.destroy_pbuffer = (glitz_glx_destroy_pbuffer_t)
    glitz_glx_get_proc_address (info, "glXDestroyPbuffer");
  info->glx.make_context_current = (glitz_glx_make_context_current_t)
    glitz_glx_get_proc_address (info, "glXMakeContextCurrent");
  info->glx.get_proc_address = (glitz_glx_get_proc_address_t)
    glitz_glx_get_proc_address (info, "glXGetProcAddressARB");
  
  info->glx.need_lookup = 0;
}

static void
glitz_glx_display_destroy (glitz_glx_display_info_t *display_info);

static void
glitz_glx_screen_destroy (glitz_glx_screen_info_t *screen_info);

static void
glitz_glx_thread_info_init (glitz_glx_thread_info_t *thread_info)
{
  thread_info->displays = NULL;
  thread_info->n_displays = 0;
  memset (&thread_info->glx, 0, sizeof (glitz_glx_static_proc_address_list_t));
  thread_info->glx.need_lookup = 1;
  thread_info->gl_library = NULL;
  thread_info->dlhand = NULL;
}

static void
glitz_glx_thread_info_fini (glitz_glx_thread_info_t *thread_info)
{
  int i;
  
  for (i = 0; i < thread_info->n_displays; i++)
    glitz_glx_display_destroy (thread_info->displays[i]);

  free (thread_info->displays);
  
  thread_info->displays = NULL;
  thread_info->n_displays = 0;

  if (thread_info->gl_library) {
    free (thread_info->gl_library);
    thread_info->gl_library = NULL;
  }

  if (thread_info->dlhand) {
    dlclose (thread_info->dlhand);
    thread_info->dlhand = NULL;
  }
}

#ifdef XTHREADS

#include <X11/Xthreads.h>
#include <stdlib.h>

/* thread safe */
static int tsd_initialized = 0;
static xthread_key_t info_tsd;

static void
glitz_glx_thread_info_destroy (glitz_glx_thread_info_t *thread_info)
{
  xthread_set_specific (info_tsd, NULL);
  
  if (thread_info) {
    glitz_glx_thread_info_fini (thread_info);
    free (thread_info);
  }
}

static void
_tsd_destroy (void *p)
{
  if (p) {
    glitz_glx_thread_info_fini ((glitz_glx_thread_info_t *) p);
    free (p);
  }
}

static glitz_glx_thread_info_t *
glitz_glx_thread_info_get (const char *gl_library)
{
  glitz_glx_thread_info_t *thread_info;
  void *p;
    
  if (!tsd_initialized) {
    xthread_key_create (&info_tsd, _tsd_destroy);
    tsd_initialized = 1;
  }

  xthread_get_specific (info_tsd, &p);
  
  if (p == NULL) {
    thread_info = malloc (sizeof (glitz_glx_thread_info_t));
    glitz_glx_thread_info_init (thread_info);
    
    xthread_set_specific (info_tsd, thread_info);
  } else
    thread_info = (glitz_glx_thread_info_t *) p;
  
  if (thread_info->glx.need_lookup) {
    if (gl_library) {
      int len = strlen (gl_library);
      
      thread_info->gl_library = malloc (len + 1);
      if (thread_info->gl_library) {
        memcpy (thread_info->gl_library, gl_library, len);
        thread_info->gl_library[len] = '\0';
      }
    }
    
    glitz_glx_proc_address_lookup (thread_info);
  }

  return thread_info;
}

#else

/* not thread safe */
static glitz_glx_thread_info_t thread_info = {
  NULL,
  0,
  { 0, 0, 0, 0, 0, 0, 1 },
  NULL,
  NULL
};

static void
glitz_glx_thread_info_destroy (glitz_glx_thread_info_t *thread_info)
{
  if (thread_info)
    glitz_glx_thread_info_fini (thread_info);
}

static glitz_glx_thread_info_t *
glitz_glx_thread_info_get (char *gl_library)
{
  if (!thread_info.glx.need_lookup) {
    if (gl_library) {
      int len = strlen (gl_library);
      
      thread_info->gl_library = malloc (len + 1);
      if (thread_info->gl_library) {
        memcpy (thread_info->gl_library, gl_library, len);
        thread_info->gl_library[len] = '\0';
      }
    }
    
    glitz_glx_proc_address_lookup (&thread_info);
  }
  
  return &thread_info;
}

#endif

static glitz_glx_display_info_t *
glitz_glx_display_info_get (Display *display)
{
  glitz_glx_display_info_t *display_info;
  glitz_glx_thread_info_t *thread_info = glitz_glx_thread_info_get (NULL);
  glitz_glx_display_info_t **displays = thread_info->displays;
  int index, n_displays = thread_info->n_displays;

  for (; n_displays; n_displays--, displays++)
    if ((*displays)->display == display)
      return *displays;

  index = thread_info->n_displays++;

  thread_info->displays =
    realloc (thread_info->displays,
             sizeof (glitz_glx_display_info_t *) * thread_info->n_displays);

  display_info = malloc (sizeof (glitz_glx_display_info_t));
  thread_info->displays[index] = display_info;
  
  display_info->thread_info = thread_info;
  display_info->display = display;
  display_info->screens = NULL;
  display_info->n_screens = 0;
  
  return display_info;
}

static void
glitz_glx_display_destroy (glitz_glx_display_info_t *display_info)
{
  int i;
  
  for (i = 0; i < display_info->n_screens; i++)
    glitz_glx_screen_destroy (display_info->screens[i]);

  if (display_info->screens)
    free (display_info->screens);
  
  free (display_info);
}

static void
glitz_glx_create_root_context (glitz_glx_screen_info_t *screen_info)
{
  XVisualInfo *vinfo;
  XSetWindowAttributes win_attrib;
  int attrib_single[] = {
      GLX_RGBA,
      GLX_RED_SIZE, 1,
      GLX_GREEN_SIZE, 1,
      GLX_BLUE_SIZE, 1,
      None
  };
   int attrib_double[] = {
      GLX_RGBA,
      GLX_RED_SIZE, 1,
      GLX_GREEN_SIZE, 1,
      GLX_BLUE_SIZE, 1,
      GLX_DOUBLEBUFFER,
      None
   };
  int screen = screen_info->screen;
  Display *display = screen_info->display_info->display;

  vinfo = glXChooseVisual (display, screen, attrib_single);
  if (!vinfo)
    vinfo = glXChooseVisual (display, screen, attrib_double);
  
  if (vinfo) {
    screen_info->root_colormap = XCreateColormap (display,
                                                  RootWindow (display, screen),
                                                  vinfo->visual, AllocNone);
    win_attrib.background_pixel = 0;
    win_attrib.border_pixel = 0;
    win_attrib.event_mask = StructureNotifyMask | ExposureMask;
    win_attrib.colormap = screen_info->root_colormap;

    screen_info->root_drawable =
      XCreateWindow (display, RootWindow (display, screen),
                     0, 0, 100, 100, 0, vinfo->depth, InputOutput,
                     vinfo->visual,
                     CWBackPixel | CWBorderPixel | CWColormap | CWEventMask,
                     &win_attrib);
    
    screen_info->root_context.context =
      glXCreateContext (display, vinfo, NULL, 1);
    
    screen_info->root_context.id = vinfo->visualid;
    
    XFree (vinfo);
  } else {
    screen_info->root_drawable = None;
    screen_info->root_context.context = NULL;
    screen_info->root_context.id = 0;
  }

  screen_info->root_context.fbconfig = (XID) 0;

  glitz_glx_surface_backend_init (&screen_info->root_context.backend);
  
  memcpy (&screen_info->root_context.backend.gl,
          &_glitz_glx_gl_proc_address,
          sizeof (glitz_gl_proc_address_list_t));

  screen_info->root_context.backend.formats = NULL;
  screen_info->root_context.backend.n_formats = 0;
  screen_info->root_context.backend.program_map = &screen_info->program_map;
  screen_info->root_context.backend.feature_mask = 0;
  
  screen_info->root_context.backend.gl.need_lookup = 1;
}

glitz_glx_screen_info_t *
glitz_glx_screen_info_get (Display *display,
                           int screen)
{
  glitz_glx_screen_info_t *screen_info;
  glitz_glx_display_info_t *display_info =
    glitz_glx_display_info_get (display);
  glitz_glx_screen_info_t **screens = display_info->screens;
  int index, n_screens = display_info->n_screens;

  for (; n_screens; n_screens--, screens++)
    if ((*screens)->screen == screen)
      return *screens;

  index = display_info->n_screens++;

  display_info->screens =
    realloc (display_info->screens,
             sizeof (glitz_glx_screen_info_t *) * display_info->n_screens);

  screen_info = malloc (sizeof (glitz_glx_screen_info_t));
  display_info->screens[index] = screen_info;

  screen_info->display_info = display_info;
  screen_info->screen = screen;
  screen_info->formats = NULL;
  screen_info->format_ids = NULL;
  screen_info->n_formats = 0;

  screen_info->contexts = NULL;
  screen_info->n_contexts = 0;

  glitz_program_map_init (&screen_info->program_map);

  glitz_glx_create_root_context (screen_info);

  screen_info->glx_feature_mask = 0;
  screen_info->feature_mask = 0;

  if (screen_info->root_context.context &&
      glXMakeCurrent (screen_info->display_info->display,
                      screen_info->root_drawable,
                      screen_info->root_context.context)) {
    if (glitz_glx_query_extensions (screen_info) == GLITZ_STATUS_SUCCESS) {
      glitz_glx_context_proc_address_lookup (screen_info,
                                             &screen_info->root_context);
      glitz_glx_query_formats (screen_info);
    }
  }

  screen_info->root_context.backend.formats = screen_info->formats;
  screen_info->root_context.backend.n_formats = screen_info->n_formats;
  
  screen_info->context_stack_size = 1;
  screen_info->context_stack->surface = NULL;
  screen_info->context_stack->constraint = GLITZ_CN_NONE;
  
  return screen_info;
}

static void
glitz_glx_screen_destroy (glitz_glx_screen_info_t *screen_info)
{
  int i;
  Display *display = screen_info->display_info->display;

  if (screen_info->root_context.context &&
      glXMakeCurrent (screen_info->display_info->display,
                      screen_info->root_drawable,
                      screen_info->root_context.context)) {
    glitz_program_map_fini (&screen_info->root_context.backend.gl,
                            &screen_info->program_map);
    glXMakeCurrent (display, None, NULL);
  }
  
  for (i = 0; i < screen_info->n_contexts; i++)
    glitz_glx_context_destroy (screen_info, screen_info->contexts[i]);

  if (screen_info->contexts)
    free (screen_info->contexts);
  
  if (screen_info->formats)
    free (screen_info->formats);

  if (screen_info->format_ids)
    free (screen_info->format_ids);
  
  if (screen_info->root_context.context)
    glXDestroyContext (display, screen_info->root_context.context);
  
  if (screen_info->root_drawable)
    XDestroyWindow (display, screen_info->root_drawable);

  if (screen_info->root_colormap)
    XFreeColormap (display, screen_info->root_colormap);

  free (screen_info);
}

void
glitz_glx_init (const char *gl_library)
{
  glitz_glx_thread_info_get (gl_library);
}
slim_hidden_def(glitz_glx_init);

void
glitz_glx_fini (void)
{
  glitz_glx_thread_info_t *info =
    glitz_glx_thread_info_get (NULL);

  glitz_glx_thread_info_destroy (info);
}
slim_hidden_def(glitz_glx_fini);
