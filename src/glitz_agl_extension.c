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

static glitz_extension_map gl_extensions[] = {
  { "GL_APPLE_pixel_buffer", GLITZ_AGL_FEATURE_PBUFFER_MASK },
  { "GL_EXT_texture_rectangle", GLITZ_AGL_FEATURE_TEXTURE_RECTANGLE_MASK },
  { "GL_NV_texture_rectangle", GLITZ_AGL_FEATURE_TEXTURE_RECTANGLE_MASK },
  { "GL_ARB_texture_non_power_of_two", GLITZ_AGL_FEATURE_TEXTURE_NPOT_MASK },
  { "GL_ARB_texture_mirrored_repeat",
    GLITZ_AGL_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK },
  { "GL_ARB_texture_env_combine",
    GLITZ_AGL_FEATURE_ARB_TEXTURE_ENV_COMBINE_MASK },
  { "GL_ARB_texture_env_dot3", GLITZ_AGL_FEATURE_ARB_TEXTURE_ENV_DOT3_MASK },
  { "GL_ARB_multisample", GLITZ_AGL_FEATURE_MULTISAMPLE_MASK },
  { "GL_NV_multisample_filter_hint",
    GLITZ_AGL_FEATURE_MULTISAMPLE_FILTER_MASK },
  { "GL_ARB_multitexture", GLITZ_AGL_FEATURE_ARB_MULTITEXTURE_MASK },
  { "GL_ARB_vertex_program", GLITZ_AGL_FEATURE_ARB_VERTEX_PROGRAM_MASK },
  { "GL_ARB_fragment_program", GLITZ_AGL_FEATURE_ARB_FRAGMENT_PROGRAM_MASK },
  { "GL_EXT_pixel_buffer_object", GLITZ_AGL_FEATURE_PIXEL_BUFFER_OBJECT_MASK },
  { NULL, 0 }
};

static long int
_glitz_agl_extension_query_gl (void)
{
  const char *gl_extensions_strings;
  
  gl_extensions_strings = (const char *) glGetString (GL_EXTENSIONS);

  return glitz_extensions_query (gl_extensions_strings, gl_extensions);
}

void
glitz_agl_query_extensions (glitz_agl_thread_info_t *thread_info)
{
  thread_info->agl_feature_mask = 0;
  
  thread_info->agl_feature_mask |= _glitz_agl_extension_query_gl ();

  thread_info->feature_mask = 0;
  thread_info->texture_mask = GLITZ_TEXTURE_TARGET_2D_MASK;

  if (thread_info->agl_feature_mask & GLITZ_AGL_FEATURE_PBUFFER_MASK)
    thread_info->feature_mask |= GLITZ_FEATURE_OFFSCREEN_DRAWING_MASK;

  if (thread_info->agl_feature_mask & GLITZ_AGL_FEATURE_MULTISAMPLE_MASK) {
    thread_info->feature_mask |= GLITZ_FEATURE_MULTISAMPLE_MASK;
    
    /*
      if (strcmp ("Card supporting pbuffer multisampling",
      glGetString (GL_RENDERER)))
      thread_info->feature_mask |= GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK;
    */
  }

  if (thread_info->agl_feature_mask & GLITZ_AGL_FEATURE_TEXTURE_NPOT_MASK) {
    thread_info->texture_mask |= GLITZ_TEXTURE_TARGET_NPOT_MASK;
    thread_info->feature_mask |= GLITZ_FEATURE_TEXTURE_NPOT_MASK;
  } 

  if (thread_info->agl_feature_mask &
      GLITZ_AGL_FEATURE_TEXTURE_RECTANGLE_MASK) {
    thread_info->texture_mask |= GLITZ_TEXTURE_TARGET_RECTANGLE_MASK;
    thread_info->feature_mask |= GLITZ_FEATURE_TEXTURE_RECTANGLE_MASK;
  }

  if (thread_info->agl_feature_mask &
      GLITZ_AGL_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK)
    thread_info->feature_mask |= GLITZ_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK;

  if (thread_info->agl_feature_mask &
      GLITZ_AGL_FEATURE_ARB_MULTITEXTURE_MASK) {
    thread_info->feature_mask |= GLITZ_FEATURE_ARB_MULTITEXTURE_MASK;

    if (thread_info->agl_feature_mask &
        GLITZ_AGL_FEATURE_ARB_TEXTURE_ENV_COMBINE_MASK)
      thread_info->feature_mask |= GLITZ_FEATURE_ARB_TEXTURE_ENV_COMBINE_MASK;
    
    if (thread_info->agl_feature_mask &
        GLITZ_AGL_FEATURE_ARB_TEXTURE_ENV_DOT3_MASK)
      thread_info->feature_mask |= GLITZ_FEATURE_ARB_TEXTURE_ENV_DOT3_MASK;
    
    if ((thread_info->feature_mask &
         GLITZ_FEATURE_ARB_TEXTURE_ENV_COMBINE_MASK) &&
        (thread_info->feature_mask &
         GLITZ_FEATURE_ARB_TEXTURE_ENV_DOT3_MASK)) {
      GLint max_texture_units;
      
      glGetIntegerv (GLITZ_GL_MAX_TEXTURE_UNITS, &max_texture_units);
      if (max_texture_units >= 3)
        thread_info->feature_mask |= GLITZ_FEATURE_COMPONENT_ALPHA_MASK;
    }
    
    if (thread_info->agl_feature_mask &
        GLITZ_AGL_FEATURE_ARB_VERTEX_PROGRAM_MASK)
      thread_info->feature_mask |= GLITZ_FEATURE_ARB_VERTEX_PROGRAM_MASK;
    
    if (thread_info->agl_feature_mask &
        GLITZ_AGL_FEATURE_ARB_FRAGMENT_PROGRAM_MASK)
      thread_info->feature_mask |= GLITZ_FEATURE_ARB_FRAGMENT_PROGRAM_MASK;
  }

    if (thread_info->agl_feature_mask &
      GLITZ_AGL_FEATURE_PIXEL_BUFFER_OBJECT_MASK)
    thread_info->feature_mask |= GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK;
}
