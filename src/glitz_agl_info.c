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

#include <OpenGL/glext.h>

glitz_gl_proc_address_list_t _glitz_agl_gl_proc_address = {
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
  
  (glitz_gl_active_texture_arb_t) glActiveTextureARB,
  (glitz_gl_multi_tex_coord_2d_arb_t) glMultiTexCoord2dARB,
  (glitz_gl_gen_programs_arb_t) glGenProgramsARB,
  (glitz_gl_delete_programs_arb_t) glDeleteProgramsARB,
  (glitz_gl_program_string_arb_t) glProgramStringARB,
  (glitz_gl_bind_program_arb_t) glBindProgramARB,
  (glitz_gl_program_local_param_4d_arb_t) glProgramLocalParameter4dARB,
  (glitz_gl_get_program_iv_arb_t) glGetProgramivARB,
  0
};

static void
_glitz_agl_thread_info_init (glitz_agl_thread_info_t *thread_info);

#ifdef PTHREADS

#include <pthread.h>
#include <stdlib.h>

/* thread safe */
static int tsd_initialized = 0;
static pthread_key_t info_tsd;

glitz_agl_thread_info_t *
glitz_agl_thread_info_get (void)
{
  if (!tsd_initialized) {
    glitz_agl_thread_info_t *info = (glitz_agl_thread_info_t *)
      malloc (sizeof (glitz_agl_thread_info_t));
    pthread_key_create (&info_tsd, NULL);
    pthread_setspecific (info_tsd, info);
    tsd_initialized = 1;
    _glitz_agl_thread_info_init (info);
    
    return info;
  } else
    return (glitz_agl_thread_info_t *) pthread_getspecific (info_tsd);
}

#else

/* not thread safe */
static glitz_agl_thread_info_t _thread_info = {
  NULL,
  NULL,
  0,
  NULL,
  0,
  NULL,
  0,
  { (AGLITZontext) 0, (AGLPixelFormat) 0, 0 },
  0,
  0,
  0,
  0,
  { 0, 0, 0, 0 }
};

glitz_agl_thread_info_t *
glitz_agl_thread_info_get (void)
{
  if (_thread_info.context_stack == NULL)
    _glitz_agl_thread_info_init (&_thread_info);
      
  return &_thread_info;
}

#endif

static void
_glitz_agl_thread_info_init (glitz_agl_thread_info_t *thread_info)
{
  GLint attrib[] = {
    AGL_RGBA,
    AGL_NO_RECOVERY,
    AGL_NONE
  };

  thread_info->formats = NULL;
  thread_info->format_ids = NULL;
  thread_info->n_formats = 0;
  thread_info->contexts = NULL;
  thread_info->n_contexts = 0;

  memset (&thread_info->programs, 0, sizeof (glitz_programs_t));
  
  thread_info->root_context.pixel_format =
    aglChoosePixelFormat (NULL, 0, attrib);
  thread_info->root_context.context =
    aglCreateContext (thread_info->root_context.pixel_format, NULL);

  aglSetCurrentContext (thread_info->root_context.context);
  glPixelStorei (GL_PACK_ALIGNMENT, 4);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
  
  glitz_agl_query_extensions (thread_info);
  glitz_agl_query_formats (thread_info);

  thread_info->context_stack = malloc (sizeof (glitz_agl_context_info_t));
  thread_info->context_stack_size = 1;
  thread_info->context_stack->surface = NULL;
  thread_info->context_stack->constraint = GLITZ_CN_NONE;
}
