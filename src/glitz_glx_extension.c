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

extern glitz_glx_static_proc_address_list_t _glitz_glx_proc_address;

static glitz_extension_map client_glx_extensions[] = {
  { "GLX_ATI_render_texture", GLITZ_GLX_FEATURE_ATI_RENDER_TEXTURE_MASK },
  { "GLX_ARB_multisample", GLITZ_GLX_FEATURE_CLIENT_MULTISAMPLE_MASK },
  { NULL, 0 }
}, gl_extensions[] = {
  { "GL_EXT_texture_rectangle", GLITZ_GLX_FEATURE_TEXTURE_RECTANGLE_MASK },
  { "GL_NV_texture_rectangle", GLITZ_GLX_FEATURE_TEXTURE_RECTANGLE_MASK },
  { "GL_ARB_texture_non_power_of_two", GLITZ_GLX_FEATURE_TEXTURE_NPOT_MASK },
  { "GL_ARB_texture_mirrored_repeat",
    GLITZ_GLX_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK },
  { "GL_ARB_multisample", GLITZ_GLX_FEATURE_MULTISAMPLE_MASK },
  { "GL_NV_multisample_filter_hint",
    GLITZ_GLX_FEATURE_MULTISAMPLE_FILTER_MASK },
  { "GL_ARB_vertex_program", GLITZ_GLX_FEATURE_ARB_VERTEX_PROGRAM_MASK },
  { "GL_ARB_fragment_program", GLITZ_GLX_FEATURE_ARB_FRAGMENT_PROGRAM_MASK },
  { NULL, 0 }
};

static long int
_glitz_glx_extension_query_client_glx (Display *display)
{
  const char *client_glx_extensions_strings;
  
  client_glx_extensions_strings = glXGetClientString (display, GLX_EXTENSIONS);
  
  return glitz_extensions_query (client_glx_extensions_strings,
                                 client_glx_extensions);
}

static long int
_glitz_glx_extension_query_gl (void)
{
  const char *gl_extensions_strings;
  
  gl_extensions_strings = (const char *) glGetString (GL_EXTENSIONS);

  return glitz_extensions_query (gl_extensions_strings, gl_extensions);
}

void
glitz_glx_query_extensions (glitz_glx_screen_info_t *screen_info)
{
  screen_info->glx_feature_mask = 0;

  screen_info->glx_feature_mask |=
    _glitz_glx_extension_query_client_glx (screen_info->display_info->display);
  
  screen_info->glx_feature_mask |= _glitz_glx_extension_query_gl ();

  screen_info->feature_mask = 0;
  screen_info->texture_mask = GLITZ_TEXTURE_TARGET_2D_MASK;

  if (_glitz_glx_proc_address.get_fbconfigs &&
      _glitz_glx_proc_address.get_fbconfig_attrib &&
      _glitz_glx_proc_address.get_visual_from_fbconfig &&
      _glitz_glx_proc_address.create_pbuffer &&
      _glitz_glx_proc_address.destroy_pbuffer) {
    screen_info->feature_mask |= GLITZ_FEATURE_OFFSCREEN_DRAWING_MASK;
    screen_info->glx_feature_mask |= GLITZ_GLX_FEATURE_GLX13_MASK;
  }

  if (screen_info->glx_feature_mask & GLITZ_GLX_FEATURE_MULTISAMPLE_MASK &&
      screen_info->glx_feature_mask &
      GLITZ_GLX_FEATURE_CLIENT_MULTISAMPLE_MASK) {
    screen_info->feature_mask |= GLITZ_FEATURE_MULTISAMPLE_MASK;

    /* All geforce cards seems to support multisample with pbuffers */
    if (!strncmp ("GeForce", (char *) glGetString (GL_RENDERER), 7))
      screen_info->feature_mask |= GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK;
  }

  if (screen_info->glx_feature_mask & GLITZ_GLX_FEATURE_TEXTURE_NPOT_MASK) {
    screen_info->texture_mask |= GLITZ_TEXTURE_TARGET_NPOT_MASK;
    screen_info->feature_mask |= GLITZ_FEATURE_TEXTURE_NPOT_MASK;
  }

  if (screen_info->glx_feature_mask &
      GLITZ_GLX_FEATURE_TEXTURE_RECTANGLE_MASK) {
    screen_info->texture_mask |= GLITZ_TEXTURE_TARGET_RECTANGLE_MASK;
    screen_info->feature_mask |= GLITZ_FEATURE_TEXTURE_RECTANGLE_MASK;
  }

  if (screen_info->glx_feature_mask &
      GLITZ_GLX_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK)
    screen_info->feature_mask |= GLITZ_FEATURE_TEXTURE_MIRRORED_REPEAT_MASK;

  if (screen_info->glx_feature_mask &
      GLITZ_GLX_FEATURE_ATI_RENDER_TEXTURE_MASK) {
    screen_info->glx_feature_mask |= GLITZ_GLX_FEATURE_ATI_RENDER_TEXTURE_MASK;
    screen_info->feature_mask |= GLITZ_FEATURE_ATI_RENDER_TEXTURE_MASK;

    /* ATI render texture doesn't seem to support texture rectangle */
    screen_info->texture_mask &= ~GLITZ_TEXTURE_TARGET_RECTANGLE_MASK;
    screen_info->feature_mask &= ~GLITZ_FEATURE_TEXTURE_RECTANGLE_MASK;
  }

  if (screen_info->glx_feature_mask &
      GLITZ_GLX_FEATURE_ARB_VERTEX_PROGRAM_MASK) {
    screen_info->glx_feature_mask |= GLITZ_GLX_FEATURE_ARB_VERTEX_PROGRAM_MASK;
    screen_info->feature_mask |= GLITZ_FEATURE_ARB_VERTEX_PROGRAM_MASK;    
  }

  if (screen_info->glx_feature_mask &
      GLITZ_GLX_FEATURE_ARB_FRAGMENT_PROGRAM_MASK) {
    screen_info->glx_feature_mask |=
      GLITZ_GLX_FEATURE_ARB_FRAGMENT_PROGRAM_MASK;
    screen_info->feature_mask |= GLITZ_FEATURE_ARB_FRAGMENT_PROGRAM_MASK;
  }

  if ((screen_info->feature_mask & GLITZ_FEATURE_ARB_VERTEX_PROGRAM_MASK) &&
      (screen_info->feature_mask & GLITZ_FEATURE_ARB_FRAGMENT_PROGRAM_MASK))
    screen_info->feature_mask |= GLITZ_FEATURE_CONVOLUTION_FILTER_MASK;
}
