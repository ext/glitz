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

#include "glitz_glxint.h"

static glitz_extension_map glx_extensions[] = {
  { 0.0, "GLX_SGIX_fbconfig", GLITZ_GLX_FEATURE_GLX_FBCONFIG_MASK },
  { 0.0, "GLX_SGIX_pbuffer", GLITZ_GLX_FEATURE_GLX_PBUFFER_MASK },
  { 0.0, "GLX_SGI_make_current_read",
    GLITZ_GLX_FEATURE_GLX_MAKE_CURRENT_READ_MASK },
  { 0.0, "GLX_ARB_multisample", GLITZ_GLX_FEATURE_GLX_MULTISAMPLE_MASK },
  { 0.0, NULL, 0 }
}, gl_extensions[] = {
  { 0.0, "GL_ARB_texture_rectangle",
    GLITZ_GLX_FEATURE_TEXTURE_RECTANGLE_MASK },
  { 0.0, "GL_EXT_texture_rectangle",
    GLITZ_GLX_FEATURE_TEXTURE_RECTANGLE_MASK },
  { 0.0, "GL_NV_texture_rectangle", GLITZ_GLX_FEATURE_TEXTURE_RECTANGLE_MASK },
  { 0.0, "GL_ARB_texture_non_power_of_two",
    GLITZ_GLX_FEATURE_TEXTURE_NON_POWER_OF_TWO_MASK },
  { 0.0, "GL_ARB_texture_mirrored_repeat",
    GLITZ_GLX_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK },
  { 0.0, "GL_ARB_texture_border_clamp",
    GLITZ_GLX_FEATURE_TEXTURE_BORDER_CLAMP_MASK },
  { 0.0, "GL_ARB_texture_env_combine",
    GLITZ_GLX_FEATURE_TEXTURE_ENV_COMBINE_MASK },
  { 0.0, "GL_EXT_texture_env_combine",
    GLITZ_GLX_FEATURE_TEXTURE_ENV_COMBINE_MASK },
  { 0.0, "GL_ARB_texture_env_dot3", GLITZ_GLX_FEATURE_TEXTURE_ENV_DOT3_MASK },
  { 0.0, "GL_ARB_multisample", GLITZ_GLX_FEATURE_MULTISAMPLE_MASK },
  { 0.0, "GL_NV_multisample_filter_hint",
    GLITZ_GLX_FEATURE_MULTISAMPLE_FILTER_HINT_MASK },
  { 0.0, "GL_ARB_multitexture", GLITZ_GLX_FEATURE_MULTITEXTURE_MASK },
  { 0.0, "GL_ARB_fragment_program", GLITZ_GLX_FEATURE_FRAGMENT_PROGRAM_MASK },
  { 0.0, "GL_ARB_vertex_buffer_object",
    GLITZ_GLX_FEATURE_VERTEX_BUFFER_OBJECT_MASK },
  { 0.0, "GL_EXT_pixel_buffer_object",
    GLITZ_GLX_FEATURE_PIXEL_BUFFER_OBJECT_MASK },
  { 0.0, "GL_EXT_blend_color",
    GLITZ_GLX_FEATURE_BLEND_COLOR_MASK },
  { 0.0, "GL_ARB_imaging",
    GLITZ_GLX_FEATURE_BLEND_COLOR_MASK },
  { 0.0, NULL, 0 }
};

static unsigned long
_glitz_glx_extension_query_glx (Display *display,
                                int screen,
                                glitz_gl_float_t glx_version)
{
  const char *glx_extensions_string;
  
  glx_extensions_string = glXQueryExtensionsString (display, screen);
  
  return glitz_extensions_query (glx_version,
                                 glx_extensions_string,
                                 glx_extensions);
}

static unsigned long
_glitz_glx_extension_query_gl (glitz_gl_float_t gl_version)
{
  const char *gl_extensions_string;
  
  gl_extensions_string = (const char *) glGetString (GL_EXTENSIONS);

  return glitz_extensions_query (gl_version,
                                 gl_extensions_string,
                                 gl_extensions);
}

glitz_status_t
glitz_glx_query_extensions (glitz_glx_screen_info_t *screen_info)
{
  screen_info->gl_version = atof ((const char *) glGetString (GL_VERSION));
  if (screen_info->gl_version < 1.2f)
    return GLITZ_STATUS_NOT_SUPPORTED;
  
  screen_info->glx_feature_mask |=
    _glitz_glx_extension_query_glx (screen_info->display_info->display,
                                    screen_info->screen,
                                    screen_info->glx_version);
  
  screen_info->glx_feature_mask |=
    _glitz_glx_extension_query_gl (screen_info->gl_version);

  if ((screen_info->glx_feature_mask &
       GLITZ_GLX_FEATURE_GLX_MULTISAMPLE_MASK) &&
      (screen_info->glx_feature_mask & GLITZ_GLX_FEATURE_MULTISAMPLE_MASK)) {
    const glitz_gl_ubyte_t *renderer = glGetString (GL_RENDERER);
    
    screen_info->feature_mask |= GLITZ_FEATURE_MULTISAMPLE_MASK;
    
    if (screen_info->glx_feature_mask &
        GLITZ_GLX_FEATURE_MULTISAMPLE_FILTER_HINT_MASK)
      screen_info->feature_mask |= GLITZ_FEATURE_MULTISAMPLE_FILTER_HINT_MASK;
    
    if (renderer) {
      /* All geforce and quadro cards seems to support multisample with
         pbuffers */
      if (!strncmp ("GeForce", renderer, 7))
        screen_info->feature_mask |= GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK;
      else if (!strncmp ("Quadro", renderer, 6))
        screen_info->feature_mask |= GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK;
    }
  }

  if (screen_info->glx_feature_mask &
      GLITZ_GLX_FEATURE_TEXTURE_RECTANGLE_MASK)
    screen_info->feature_mask |= GLITZ_FEATURE_TEXTURE_RECTANGLE_MASK;
  
  if (screen_info->glx_feature_mask &
      GLITZ_GLX_FEATURE_TEXTURE_NON_POWER_OF_TWO_MASK)
    screen_info->feature_mask |= GLITZ_FEATURE_TEXTURE_NON_POWER_OF_TWO_MASK;

  if (screen_info->glx_feature_mask &
      GLITZ_GLX_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK)
    screen_info->feature_mask |= GLITZ_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK;

  if (screen_info->glx_feature_mask &
      GLITZ_GLX_FEATURE_TEXTURE_BORDER_CLAMP_MASK)
    screen_info->feature_mask |= GLITZ_FEATURE_TEXTURE_BORDER_CLAMP_MASK;

  if (screen_info->glx_feature_mask & GLITZ_GLX_FEATURE_MULTITEXTURE_MASK) {
    screen_info->feature_mask |= GLITZ_FEATURE_MULTITEXTURE_MASK;

    if (screen_info->glx_feature_mask &
        GLITZ_GLX_FEATURE_TEXTURE_ENV_COMBINE_MASK)
      screen_info->feature_mask |= GLITZ_FEATURE_TEXTURE_ENV_COMBINE_MASK;
    
    if (screen_info->glx_feature_mask &
        GLITZ_GLX_FEATURE_TEXTURE_ENV_DOT3_MASK)
      screen_info->feature_mask |= GLITZ_FEATURE_TEXTURE_ENV_DOT3_MASK;
      
    if ((screen_info->feature_mask & GLITZ_FEATURE_TEXTURE_ENV_COMBINE_MASK) &&
        (screen_info->feature_mask & GLITZ_FEATURE_TEXTURE_ENV_DOT3_MASK)) {
      glitz_gl_int_t max_texture_units;
      
      glGetIntegerv (GLITZ_GL_MAX_TEXTURE_UNITS, &max_texture_units);
      if (max_texture_units >= 3)
        screen_info->feature_mask |=
          GLITZ_FEATURE_PER_COMPONENT_RENDERING_MASK;
    }
        
    if (screen_info->glx_feature_mask &
        GLITZ_GLX_FEATURE_FRAGMENT_PROGRAM_MASK)
      screen_info->feature_mask |= GLITZ_FEATURE_FRAGMENT_PROGRAM_MASK;
  }

  if (screen_info->glx_feature_mask &
      GLITZ_GLX_FEATURE_VERTEX_BUFFER_OBJECT_MASK)
    screen_info->feature_mask |= GLITZ_FEATURE_VERTEX_BUFFER_OBJECT_MASK;

  if (screen_info->glx_feature_mask &
      GLITZ_GLX_FEATURE_PIXEL_BUFFER_OBJECT_MASK)
    screen_info->feature_mask |= GLITZ_FEATURE_PIXEL_BUFFER_OBJECT_MASK;

  if (screen_info->glx_feature_mask & GLITZ_GLX_FEATURE_BLEND_COLOR_MASK)
    screen_info->feature_mask |= GLITZ_FEATURE_BLEND_COLOR_MASK;

  return GLITZ_STATUS_SUCCESS;
}
