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
#include <string.h>

static int
_glitz_glx_format_compare (const void *elem1,
                           const void *elem2)
{
  int i, score[2];
  glitz_format_t *format[2];
  
  format[0] = (glitz_format_t *) elem1;
  format[1] = (glitz_format_t *) elem2;
  i = score[0] = score[1] = 0;

  for (; i < 2; i++) {
    if (format[i]->red_size)
      score[i] += 10;
    if (format[i]->alpha_size)
      score[i] += 10;
    if (format[i]->stencil_size)
      score[i] += (10 + format[i]->stencil_size);
    if (format[i]->doublebuffer)
      score[i] -= 10;
    if (format[i]->draw.onscreen)
      score[i] += 5;
    if (format[i]->draw.offscreen)
      score[i] += 5;
    if (format[i]->draw.offscreen && format[i]->draw.onscreen)
      score[i] += 5;
    if (format[i]->multisample.supported) 
      score[i] -= (10 - format[i]->multisample.samples);
  }
  
  return score[1] - score[0];
}

static void
_glitz_add_format (glitz_glx_screen_info_t *screen_info,
                   glitz_format_t *format)
{
  if (!glitz_format_find (screen_info->formats, screen_info->n_formats,
                          GLITZ_FORMAT_ALL_EXCEPT_ID_MASK, format, 0)) {
    int index = screen_info->n_formats++;

    screen_info->formats =
      realloc (screen_info->formats,
               sizeof (glitz_format_t) * screen_info->n_formats);

    screen_info->formats[index] = *format;
  }
}

static void
_glitz_move_out_ids (glitz_glx_screen_info_t *screen_info)
{
  int i;
  glitz_format_t *formats = screen_info->formats;
  int n_formats = screen_info->n_formats;

  screen_info->format_ids = malloc (sizeof (XID) * n_formats);
  
  for (i = 0; n_formats; n_formats--, formats++) {
    screen_info->format_ids[i] = formats->id;
    formats->id = i++;
  }
}

static void
glitz_glx_query_formats_glx12 (glitz_glx_screen_info_t *screen_info)
{
  Display *display;
  glitz_format_t format;
  XVisualInfo visual_templ;
  XVisualInfo *visuals;
  int i, num_visuals;
  
  display = screen_info->display_info->display;

  visual_templ.screen = screen_info->screen;
  visuals = XGetVisualInfo (display, VisualScreenMask,
                            &visual_templ, &num_visuals);

  /* Offscreen drawing never supported if GLX is older than 1.3 */
  format.draw.offscreen = format.read.offscreen = 0;
  format.draw.onscreen = format.read.onscreen = 1;

  for (i = 0; i < num_visuals; i++) {
    int value;
    
    if ((glXGetConfig (display, &visuals[i], GLX_USE_GL, &value) != 0) ||
        (value == 0))
      continue;
    
    glXGetConfig (display, &visuals[i], GLX_RGBA, &value);
    if (value == 0)
      continue;

    /* Stereo is not supported yet */
    glXGetConfig (display, &visuals[i], GLX_STEREO, &value);
    if (value != 0)
      continue;
    
    format.id = visuals[i].visualid;
    glXGetConfig (display, &visuals[i], GLX_RED_SIZE, &value);
    format.red_size = (unsigned short) value;
    glXGetConfig (display, &visuals[i], GLX_GREEN_SIZE, &value);
    format.green_size = (unsigned short) value;
    glXGetConfig (display, &visuals[i], GLX_BLUE_SIZE, &value);
    format.blue_size = (unsigned short) value;
    glXGetConfig (display, &visuals[i], GLX_ALPHA_SIZE, &value);
    format.alpha_size = (unsigned short) value;
    glXGetConfig (display, &visuals[i], GLX_DEPTH_SIZE, &value);
    format.depth_size = (unsigned short) value;
    glXGetConfig (display, &visuals[i], GLX_STENCIL_SIZE, &value);
    format.stencil_size = (unsigned short) value;
    glXGetConfig (display, &visuals[i], GLX_DOUBLEBUFFER, &value);
    format.doublebuffer = (value) ? 1: 0;

    if (screen_info->feature_mask & GLITZ_FEATURE_MULTISAMPLE_MASK) {
      glXGetConfig (display, &visuals[i], GLX_SAMPLE_BUFFERS_ARB, &value);
      format.multisample.supported = (value) ? 1: 0;
      glXGetConfig (display, &visuals[i], GLX_SAMPLES_ARB, &value);
      format.multisample.samples = (unsigned short) value;
    } else {
      format.multisample.supported = 0;
      format.multisample.samples = 0;
    }

    if (format.red_size || format.green_size || format.blue_size ||
        format.alpha_size)
      _glitz_add_format (screen_info, &format);
  }
  
  if (visuals)
    XFree (visuals);
}

static glitz_bool_t
glitz_glx_query_formats_glx13 (glitz_glx_screen_info_t *screen_info)
{
  Display *display;
  glitz_format_t format;
  GLXFBConfig *fbconfigs;
  int i, num_configs, pbuffer_check = 1, pbuffer_support = 1;
  glitz_glx_static_proc_address_list_t *glx =
    &screen_info->display_info->thread_info->glx;
  
  display = screen_info->display_info->display;

  fbconfigs = glx->get_fbconfigs (display, screen_info->screen, &num_configs);
  if (!fbconfigs) {
    /* GLX 1.3 is not support, falling back to GLX 1.2 */
    screen_info->glx_feature_mask &= ~GLITZ_GLX_FEATURE_GLX13_MASK;
    return 1;
  }
  
  for (i = 0; i < num_configs; i++) {
    int value;
    
    if ((glx->get_fbconfig_attrib (display, fbconfigs[i],
                                   GLX_RENDER_TYPE, &value) != 0) ||
        (!(value & GLX_RGBA_BIT)))
      continue;

    /* Stereo is not supported yet */
    glx->get_fbconfig_attrib (display, fbconfigs[i], GLX_STEREO, &value);
    if (value != 0)
      continue;

    glx->get_fbconfig_attrib (display, fbconfigs[i],
                              GLX_DRAWABLE_TYPE, &value);
    if (!((value & GLX_WINDOW_BIT) || (value & GLX_PBUFFER_BIT)))
      continue;
    
    format.read.onscreen = format.draw.onscreen =
      (value & GLX_WINDOW_BIT)? 1: 0;
    format.read.offscreen = format.draw.offscreen =
      (pbuffer_support && (value & GLX_PBUFFER_BIT))? 1: 0;
    
    glx->get_fbconfig_attrib (display, fbconfigs[i], GLX_FBCONFIG_ID, &value);
    format.id = (XID) value;

    if (pbuffer_check && format.draw.offscreen) {
      if (glitz_glx_ensure_pbuffer_support (screen_info, format.id)) {
	pbuffer_support = 0;
	format.draw.offscreen = 0;
      }
      pbuffer_check = 0;
    }
    
    glx->get_fbconfig_attrib (display, fbconfigs[i], GLX_RED_SIZE, &value);
    format.red_size = (unsigned short) value;
    glx->get_fbconfig_attrib (display, fbconfigs[i], GLX_GREEN_SIZE, &value);
    format.green_size = (unsigned short) value;
    glx->get_fbconfig_attrib (display, fbconfigs[i], GLX_BLUE_SIZE, &value);
    format.blue_size = (unsigned short) value;
    glx->get_fbconfig_attrib (display, fbconfigs[i], GLX_ALPHA_SIZE, &value);
    format.alpha_size = (unsigned short) value;
    glx->get_fbconfig_attrib (display, fbconfigs[i], GLX_DEPTH_SIZE, &value);
    format.depth_size = (unsigned short) value;
    glx->get_fbconfig_attrib (display, fbconfigs[i], GLX_STENCIL_SIZE, &value);
    format.stencil_size = (unsigned short) value;
    glx->get_fbconfig_attrib (display, fbconfigs[i], GLX_DOUBLEBUFFER, &value);
    format.doublebuffer = (value)? 1: 0;
    
    if (screen_info->feature_mask & GLITZ_FEATURE_MULTISAMPLE_MASK) {
      glx->get_fbconfig_attrib (display, fbconfigs[i],
                                GLX_SAMPLE_BUFFERS_ARB, &value);
      format.multisample.supported = (value)? 1: 0;
      glx->get_fbconfig_attrib (display, fbconfigs[i],
                                GLX_SAMPLES_ARB, &value);
      format.multisample.samples = (unsigned short) value;
    } else {
      format.multisample.supported = 0;
      format.multisample.samples = 0;
    }

    if (format.red_size || format.green_size || format.blue_size ||
        format.alpha_size)
      _glitz_add_format (screen_info, &format);
  }
  
  if (fbconfigs)
    XFree (fbconfigs);

  return 0;
}

static void
_glitz_glx_add_texture_format (glitz_format_t *texture_format, void *ptr)
{
  glitz_glx_screen_info_t *screen_info = (glitz_glx_screen_info_t *) ptr;
  glitz_format_t format;

  format = *texture_format;
  format.id = 0;

  _glitz_add_format (screen_info, &format);
}

void
glitz_glx_query_formats (glitz_glx_screen_info_t *screen_info)
{
  glitz_bool_t status = 1;

  if (screen_info->glx_feature_mask & GLITZ_GLX_FEATURE_GLX13_MASK)
    status = glitz_glx_query_formats_glx13 (screen_info);

  if (status)
    glitz_glx_query_formats_glx12 (screen_info);

  qsort (screen_info->formats, screen_info->n_formats, 
         sizeof (glitz_format_t), _glitz_glx_format_compare);
 
  glitz_format_for_each_texture_format (&_glitz_glx_add_texture_format,
					&screen_info->root_context.backend.gl,
					(void *) screen_info);
  
  _glitz_move_out_ids (screen_info);
}

glitz_format_t *
glitz_glx_find_format (Display *display,
                       int screen,
                       unsigned long mask,
                       const glitz_format_t *templ,
                       int count)
{
  glitz_glx_screen_info_t *screen_info =
    glitz_glx_screen_info_get (display, screen);

  return glitz_format_find (screen_info->formats, screen_info->n_formats,
                            mask, templ, count);
}
slim_hidden_def(glitz_glx_find_format);

glitz_format_t *
glitz_glx_find_standard_format (Display *display,
                                int screen,
                                glitz_format_name_t format_name)
{
  glitz_glx_screen_info_t *screen_info =
    glitz_glx_screen_info_get (display, screen);

  return
    glitz_format_find_standard (screen_info->formats, screen_info->n_formats,
                                format_name);
}
slim_hidden_def(glitz_glx_find_standard_format);

XVisualInfo *
glitz_glx_get_visual_info_from_format (Display *display,
                                       int screen,
                                       glitz_format_t *format)
{
  XVisualInfo *vinfo = NULL;
  glitz_glx_screen_info_t *screen_info =
    glitz_glx_screen_info_get (display, screen);
  glitz_glx_static_proc_address_list_t *glx =
    &screen_info->display_info->thread_info->glx;

  if (screen_info->glx_feature_mask & GLITZ_GLX_FEATURE_GLX13_MASK) {
    GLXFBConfig *fbconfigs;
    int i, n_fbconfigs;
    int fbconfigid = screen_info->format_ids[format->id];

    fbconfigs = glx->get_fbconfigs (display, screen, &n_fbconfigs);
    for (i = 0; i < n_fbconfigs; i++) {
      int value;
      
      glx->get_fbconfig_attrib (display, fbconfigs[i],
                                GLX_FBCONFIG_ID, &value);
      if (value == fbconfigid)
        break;
    }
    
    if (i < n_fbconfigs)
      vinfo = glx->get_visual_from_fbconfig (display, fbconfigs[i]);
    
    if (fbconfigs)
      XFree (fbconfigs);
    
  } else {
    XVisualInfo templ;
    int n_items;
    
    templ.visualid = screen_info->format_ids[format->id];
    
    vinfo = XGetVisualInfo (display, VisualIDMask, &templ, &n_items);
  }
  
  return vinfo;
}
slim_hidden_def(glitz_glx_get_visual_info_from_format);
