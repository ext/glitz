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

#include <dlfcn.h>

glitz_gl_proc_address_list_t _glitz_gl_proc_address = {
  (glitz_gl_enable_t) glEnable,
  (glitz_gl_disable_t) glDisable,
  (glitz_gl_begin_t) glBegin,
  (glitz_gl_end_t) glEnd,
  (glitz_gl_vertex_2i_t) glVertex2i,
  (glitz_gl_vertex_2d_t) glVertex2d,
  (glitz_gl_tex_env_f_t) glTexEnvf,
  (glitz_gl_tex_coord_2d_t) glTexCoord2d,
  (glitz_gl_color_4us_t) glColor4us,
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
  (glitz_gl_depth_range_t) glDepthRange,
  (glitz_gl_viewport_t) glViewport,
  (glitz_gl_raster_pos_2d_t) glRasterPos2d,
  (glitz_gl_bitmap_t) glBitmap,
  (glitz_gl_read_buffer_t) glReadBuffer,
  (glitz_gl_draw_buffer_t) glDrawBuffer,
  (glitz_gl_copy_pixels_t) glCopyPixels,
  (glitz_gl_flush_t) glFlush,
  (glitz_gl_pixel_store_i_t) glPixelStorei,
  (glitz_gl_ortho_t) glOrtho,
  (glitz_gl_scale_d_t) glScaled,
  (glitz_gl_translate_d_t) glTranslated,
  (glitz_gl_hint_t) glHint,
  (glitz_gl_depth_mask_t) glDepthMask,
  (glitz_gl_polygon_mode_t) glPolygonMode,
  (glitz_gl_shade_model_t) glShadeModel,
  (glitz_gl_color_mask_t) glColorMask,
  (glitz_gl_read_pixels_t) glReadPixels,
  (glitz_gl_get_tex_image_t) glGetTexImage,
  (glitz_gl_pixel_zoom_t) glPixelZoom,
  (glitz_gl_draw_pixels_t) glDrawPixels,
  (glitz_gl_tex_sub_image_2d_t) glTexSubImage2D,
  (glitz_gl_gen_textures_t) glGenTextures,
  (glitz_gl_delete_textures_t) glDeleteTextures,
  (glitz_gl_bind_texture_t) glBindTexture,
  (glitz_gl_tex_image_1d_t) glTexImage1D,
  (glitz_gl_tex_image_2d_t) glTexImage2D,
  (glitz_gl_tex_parameter_i_t) glTexParameteri,
  (glitz_gl_copy_tex_sub_image_2d_t) glCopyTexSubImage2D,
  (glitz_gl_get_integer_v_t) glGetIntegerv,

  (glitz_gl_active_texture_arb_t) 0,
  (glitz_gl_multi_tex_coord_2d_arb_t) 0,
  (glitz_gl_gen_programs_arb_t) 0,
  (glitz_gl_delete_programs_arb_t) 0,
  (glitz_gl_program_string_arb_t) 0,
  (glitz_gl_bind_program_arb_t) 0,
  (glitz_gl_program_local_param_4d_arb_t) 0,
  (glitz_gl_get_program_iv_arb_t) 0,
  1
};

glitz_glx_static_proc_address_list_t _glitz_glx_proc_address = {
  (glitz_glx_get_fbconfigs_t) 0,
  (glitz_glx_get_fbconfig_attrib_t) 0,
  (glitz_glx_get_visual_from_fbconfig_t) 0,
  (glitz_glx_create_pbuffer_t) 0,
  (glitz_glx_destroy_pbuffer_t) 0,
  1
};

typedef void *(* glitz_glx_get_proc_address_arb_t)(glitz_gl_ubyte_t *);

glitz_glx_get_proc_address_arb_t glitz_glx_get_proc_address_arb = NULL;

void *
glitz_glx_get_proc_address (const char *name)
{
  void *address = NULL;
  
  if (glitz_glx_get_proc_address_arb)
    address = glitz_glx_get_proc_address_arb ((glitz_gl_ubyte_t *) name);
  
  if (!address) {
    void *dlhand;
    
    if ((dlhand = dlopen (NULL, RTLD_LAZY))) {
      address = dlsym (dlhand, name);
      dlclose (dlhand);
    }
  }
  
  return address;
}

static void
glitz_glx_proc_address_lookup (void)
{
  glitz_glx_get_proc_address_arb =
    (glitz_glx_get_proc_address_arb_t)
    glitz_glx_get_proc_address ("glXGetProcAddressARB");
  _glitz_glx_proc_address.get_fbconfigs =
    (glitz_glx_get_fbconfigs_t)
    glitz_glx_get_proc_address ("glXGetFBConfigs");
  _glitz_glx_proc_address.get_fbconfig_attrib =
    (glitz_glx_get_fbconfig_attrib_t)
    glitz_glx_get_proc_address ("glXGetFBConfigAttrib");
  _glitz_glx_proc_address.get_visual_from_fbconfig =
    (glitz_glx_get_visual_from_fbconfig_t)
    glitz_glx_get_proc_address ("glXGetVisualFromFBConfig");
  _glitz_glx_proc_address.create_pbuffer =
    (glitz_glx_create_pbuffer_t)
    glitz_glx_get_proc_address ("glXCreatePbuffer");
  _glitz_glx_proc_address.destroy_pbuffer =
    (glitz_glx_destroy_pbuffer_t)
    glitz_glx_get_proc_address ("glXDestroyPbuffer");
  _glitz_glx_proc_address.need_lookup = 0;
}

#ifdef XTHREADS

#include <X11/Xthreads.h>
#include <stdlib.h>

/* thread safe */
static int tsd_initialized = 0;
static xthread_key_t info_tsd;

glitz_glx_thread_info_t *
glitz_glx_thread_info_get (void)
{
  if (!tsd_initialized) {
    glitz_glx_thread_info_t *info = (glitz_glx_thread_info_t *)
      malloc (sizeof (glitz_glx_thread_info_t));
    info->displays = NULL;
    info->n_displays = 0;
    if (_glitz_glx_proc_address.need_lookup)
      glitz_glx_proc_address_lookup ();
    xthread_key_create (&info_tsd, NULL);
    xthread_set_specific (info_tsd, info);
    tsd_initialized = 1;
    return info;
  } else {
    void *p;

    xthread_get_specific (info_tsd, &p);
    return (glitz_glx_thread_info_t *) p;
  }
}

#else

/* not thread safe */
static glitz_glx_thread_info_t thread_info = {
  NULL,
  0
};

glitz_glx_thread_info_t *
glitz_glx_thread_info_get (void)
{
  if (!_glitz_glx_proc_address.supported)
    glitz_glx_proc_address_lookup ();
  
  return &thread_info;
}

#endif

glitz_glx_display_info_t *
glitz_glx_display_info_get (Display *display)
{
  glitz_glx_display_info_t *display_info;
  glitz_glx_thread_info_t *thread_info = glitz_glx_thread_info_get ();
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
    win_attrib.background_pixel = 0;
    win_attrib.border_pixel = 0;
    win_attrib.event_mask = StructureNotifyMask | ExposureMask;
    win_attrib.colormap = XCreateColormap (display,
                                           RootWindow (display, screen),
                                           vinfo->visual, AllocNone);
  
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
    
  memcpy (&screen_info->root_context.gl,
          &_glitz_gl_proc_address,
          sizeof (glitz_gl_proc_address_list_t));
  
  memset (&screen_info->root_context.glx, 0,
          sizeof (glitz_glx_proc_address_list_t));
  
  screen_info->root_context.gl.need_lookup =
    screen_info->root_context.glx.need_lookup = 1;
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

  memset (&screen_info->programs, 0, sizeof (glitz_programs_t));

  glitz_glx_create_root_context (screen_info);

  screen_info->glx_feature_mask = 0;
  screen_info->feature_mask = 0;
  screen_info->texture_mask = GLITZ_TEXTURE_TARGET_2D_MASK;

  if (screen_info->root_context.context &&
      glXMakeCurrent (screen_info->display_info->display,
                      screen_info->root_drawable,
                      screen_info->root_context.context)) {
    
    glPixelStorei (GL_PACK_ALIGNMENT, 4);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 4);

    glitz_glx_context_proc_address_lookup (&screen_info->root_context);

    glitz_glx_query_extensions (screen_info);
    glitz_glx_query_formats (screen_info);
  }
  
  screen_info->context_stack = malloc (sizeof (glitz_glx_context_info_t));
  screen_info->context_stack_size = 1;
  screen_info->context_stack->surface = NULL;
  screen_info->context_stack->constraint = GLITZ_CN_NONE;
  
  return screen_info;
}
