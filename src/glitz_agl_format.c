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

#include <stdlib.h>

static const struct _glx_pixel_format_attrib {
  GLint attrib[20];
} pixel_format_attrib_map[] = {
  {
    {
      AGL_RGBA,
      AGL_DOUBLEBUFFER,
      AGL_RED_SIZE, 8,
      AGL_GREEN_SIZE, 8,
      AGL_BLUE_SIZE, 8,
      AGL_ALPHA_SIZE, 0,
      AGL_NO_RECOVERY,
      AGL_NONE, 0, 0, 0, 0, 0, 0, 0, 0
    }
  }, {
    {
      AGL_RGBA,
      AGL_DOUBLEBUFFER,
      AGL_RED_SIZE, 8,
      AGL_GREEN_SIZE, 8,
      AGL_BLUE_SIZE, 8,
      AGL_ALPHA_SIZE, 8,
      AGL_NO_RECOVERY,
      AGL_NONE, 0, 0, 0, 0, 0, 0, 0, 0
    }
  }, {
    {
      AGL_RGBA,
      AGL_DOUBLEBUFFER,
      AGL_RED_SIZE, 8,
      AGL_GREEN_SIZE, 8,
      AGL_BLUE_SIZE, 8,
      AGL_NO_RECOVERY,
      AGL_SAMPLE_BUFFERS_ARB, 1,
      AGL_SAMPLES_ARB, 2,
      AGL_NONE, 0, 0, 0, 0
    }
  }, {
    {
      AGL_RGBA,
      AGL_DOUBLEBUFFER,
      AGL_RED_SIZE, 8,
      AGL_GREEN_SIZE, 8,
      AGL_BLUE_SIZE, 8,
      AGL_NO_RECOVERY,
      AGL_SAMPLE_BUFFERS_ARB, 1,
      AGL_SAMPLES_ARB, 4,
      AGL_NONE, 0, 0, 0, 0
    }
  }, {
    {
      AGL_RGBA,
      AGL_DOUBLEBUFFER,
      AGL_RED_SIZE, 8,
      AGL_GREEN_SIZE, 8,
      AGL_BLUE_SIZE, 8,
      AGL_ALPHA_SIZE, 0,
      AGL_DEPTH_SIZE, 1,
      AGL_NO_RECOVERY,
      AGL_NONE, 0, 0, 0, 0, 0, 0
    }
  }, {
    {
      AGL_RGBA,
      AGL_DOUBLEBUFFER,
      AGL_RED_SIZE, 8,
      AGL_GREEN_SIZE, 8,
      AGL_BLUE_SIZE, 8,
      AGL_ALPHA_SIZE, 8,
      AGL_DEPTH_SIZE, 1,
      AGL_NO_RECOVERY,
      AGL_NONE, 0, 0, 0, 0, 0, 0
    }
  }, {
    {
      AGL_RGBA,
      AGL_DOUBLEBUFFER,
      AGL_RED_SIZE, 8,
      AGL_GREEN_SIZE, 8,
      AGL_BLUE_SIZE, 8,
      AGL_DEPTH_SIZE, 1,
      AGL_NO_RECOVERY,
      AGL_SAMPLE_BUFFERS_ARB, 1,
      AGL_SAMPLES_ARB, 2,
      AGL_NONE, 0, 0
    }
  }, {
    {
      AGL_RGBA,
      AGL_DOUBLEBUFFER,
      AGL_RED_SIZE, 8,
      AGL_GREEN_SIZE, 8,
      AGL_BLUE_SIZE, 8,
      AGL_DEPTH_SIZE, 1,
      AGL_NO_RECOVERY,
      AGL_SAMPLE_BUFFERS_ARB, 1,
      AGL_SAMPLES_ARB, 4,
      AGL_NONE, 0, 0
    }
  }, {
    {
      AGL_RGBA,
      AGL_DOUBLEBUFFER,
      AGL_RED_SIZE, 8,
      AGL_GREEN_SIZE, 8,
      AGL_BLUE_SIZE, 8,
      AGL_ALPHA_SIZE, 0,
      AGL_DEPTH_SIZE, 1,
      AGL_STENCIL_SIZE, 2,
      AGL_NO_RECOVERY,
      AGL_NONE, 0, 0, 0, 0
    }
  }, {
    {
      AGL_RGBA,
      AGL_DOUBLEBUFFER,
      AGL_RED_SIZE, 8,
      AGL_GREEN_SIZE, 8,
      AGL_BLUE_SIZE, 8,
      AGL_ALPHA_SIZE, 8,
      AGL_DEPTH_SIZE, 1,
      AGL_STENCIL_SIZE, 2,
      AGL_NO_RECOVERY,
      AGL_NONE, 0, 0, 0, 0
    }
  }, {
    {
      AGL_RGBA,
      AGL_DOUBLEBUFFER,
      AGL_RED_SIZE, 8,
      AGL_GREEN_SIZE, 8,
      AGL_BLUE_SIZE, 8,
      AGL_DEPTH_SIZE, 1,
      AGL_STENCIL_SIZE, 2,
      AGL_NO_RECOVERY,
      AGL_SAMPLE_BUFFERS_ARB, 1,
      AGL_SAMPLES_ARB, 2,
      AGL_NONE,
    }
  }, {
    {
      AGL_RGBA,
      AGL_DOUBLEBUFFER,
      AGL_RED_SIZE, 8,
      AGL_GREEN_SIZE, 8,
      AGL_BLUE_SIZE, 8,
      AGL_DEPTH_SIZE, 1,
      AGL_STENCIL_SIZE, 2,
      AGL_NO_RECOVERY,
      AGL_SAMPLE_BUFFERS_ARB, 1,
      AGL_SAMPLES_ARB, 4,
      AGL_NONE,
    }
  }, {
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
  }
};

static int
_glitz_agl_format_compare (const void *elem1,
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
    if (format[i]->depth_size)
      score[i] += 5;
    if (format[i]->stencil_size)
      score[i] += 10;
    if (format[i]->multisample.supported) 
      score[i] += (10 + format[i]->multisample.samples);
  }
  
  return score[1] - score[0];
}

static void
_glitz_add_format (glitz_agl_thread_info_t *thread_info,
                   glitz_format_t *format)
{
  glitz_format_calculate_pixel_transfer_info (format);
  
  if (!glitz_format_find (thread_info->formats, thread_info->n_formats,
                          GLITZ_FORMAT_ALL_EXCEPT_ID_MASK, format, 0)) {
    int index = thread_info->n_formats++;
    
    thread_info->formats =
      realloc (thread_info->formats,
               sizeof (glitz_format_t) * thread_info->n_formats);
  
    thread_info->formats[index] = *format;
  }
}

static void
_glitz_move_out_ids (glitz_agl_thread_info_t *thread_info)
{
  int i;
  glitz_format_t *formats = thread_info->formats;
  int n_formats = thread_info->n_formats;

  thread_info->format_ids = malloc (sizeof (AGLPixelFormat) * n_formats);
  
  for (i = 0; n_formats; n_formats--, formats++) {
    thread_info->format_ids[i] = (AGLPixelFormat) formats->id;
    formats->id = i++;
  }
}

void
glitz_agl_query_formats (glitz_agl_thread_info_t *thread_info)
{
  glitz_format_t format;
  AGLPixelFormat pixel_format;
  int i = 0;
  glitz_bool_t offscreen_argb32_format = 0;

  for (i = 0; *(pixel_format_attrib_map[i].attrib); i++) {
    GLint value;

    pixel_format = aglChoosePixelFormat (NULL, 0,
                                         pixel_format_attrib_map[i].attrib);

    /* Stereo is not supported yet */
    if (!(aglDescribePixelFormat (pixel_format, AGL_STEREO, &value)) ||
        value) {
      aglDestroyPixelFormat (pixel_format);
      continue;
    }

    aglDescribePixelFormat (pixel_format, AGL_DOUBLEBUFFER, &value);
    format.doublebuffer = (value) ? 1: 0;

    /* We don't support single buffering in MacOS X */
    if (!format.doublebuffer) {
      aglDestroyPixelFormat (pixel_format);
      continue;
    }

    format.id = (unsigned long int) pixel_format;

    format.drawable.onscreen = 1;
    if (thread_info->feature_mask & GLITZ_FEATURE_OFFSCREEN_DRAWING_MASK)
      format.drawable.offscreen = 1;
    else
      format.drawable.offscreen = 0;
      
    aglDescribePixelFormat (pixel_format, AGL_RED_SIZE, &value);
    format.red_size = (unsigned short) value;
    aglDescribePixelFormat (pixel_format, AGL_GREEN_SIZE, &value);
    format.green_size = (unsigned short) value;
    aglDescribePixelFormat (pixel_format, AGL_BLUE_SIZE, &value);
    format.blue_size = (unsigned short) value;
    aglDescribePixelFormat (pixel_format, AGL_ALPHA_SIZE, &value);
    format.alpha_size = (unsigned short) value;
    aglDescribePixelFormat (pixel_format, AGL_DEPTH_SIZE, &value);
    format.depth_size = (unsigned short) value;
    aglDescribePixelFormat (pixel_format, AGL_STENCIL_SIZE, &value);
    format.stencil_size = (unsigned short) value;
    
    if (thread_info->feature_mask & GLITZ_FEATURE_MULTISAMPLE_MASK) {
      aglDescribePixelFormat (pixel_format, AGL_SAMPLE_BUFFERS_ARB, &value);
      format.multisample.supported = (value) ? 1: 0;
      aglDescribePixelFormat (pixel_format, AGL_SAMPLES_ARB, &value);
      format.multisample.samples = (unsigned short) value;

      if (format.multisample.supported) {
        if (!(thread_info->feature_mask &
              GLITZ_FEATURE_OFFSCREEN_MULTISAMPLE_MASK))
          format.drawable.offscreen = 0;
      }
    } else {
      format.multisample.supported = 0;
      format.multisample.samples = 0;
    }

    if (format.drawable.offscreen &&
        format.alpha_size &&
        format.red_size && format.green_size && format.blue_size) 
      offscreen_argb32_format = 1;
    
    _glitz_add_format (thread_info, &format);

    if (format.alpha_size &&
        (format.red_size || format.green_size || format.blue_size)) {
      unsigned short tmp;
      
      tmp = format.alpha_size;
      format.alpha_size = 0;
      _glitz_add_format (thread_info, &format);
      format.alpha_size = tmp;
      format.red_size = format.green_size = format.blue_size = 0;
      _glitz_add_format (thread_info, &format);
    }
  }

  qsort (thread_info->formats, thread_info->n_formats,
         sizeof (glitz_format_t), _glitz_agl_format_compare);
  
  /* Adding fake offscreen formats if no real argb32 offscreen formats exist.
     Surfaces created with these formats can only be used with draw/read
     pixel functions and as source in composite functions. */
  if (!offscreen_argb32_format) {
    memset (&format, 0, sizeof (glitz_format_t));
    format.drawable.offscreen = 1;
    format.alpha_size = format.red_size = format.green_size =
      format.blue_size = 8;
    format.id = 0;
    _glitz_add_format (thread_info, &format);
    format.alpha_size = 0;
    _glitz_add_format (thread_info, &format);
    format.alpha_size  = 8;
    format.red_size = format.green_size = format.blue_size = 0;
    _glitz_add_format (thread_info, &format);
  }
  
  _glitz_move_out_ids (thread_info);
}

glitz_format_t *
glitz_agl_find_format (unsigned long mask,
                       const glitz_format_t *templ,
                       int count)
{
  glitz_agl_thread_info_t *thread_info = glitz_agl_thread_info_get ();

  return glitz_format_find (thread_info->formats, thread_info->n_formats,
                            mask, templ, count);
}
slim_hidden_def(glitz_agl_find_format);

glitz_format_t *
glitz_agl_find_standard_format (unsigned long options,
                                glitz_format_name_t format_name)
{
  glitz_agl_thread_info_t *thread_info = glitz_agl_thread_info_get ();
  
  return
    glitz_format_find_standard (thread_info->formats, thread_info->n_formats,
                                options, format_name);
}
slim_hidden_def(glitz_agl_find_standard_format);
