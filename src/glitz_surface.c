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

#include "glitzint.h"

#include <stdlib.h>
#include <string.h>

void
glitz_surface_init (glitz_surface_t *surface,
                    const glitz_surface_backend_t *backend,
                    glitz_gl_proc_address_list_t *gl,
                    glitz_format_t *format,
                    glitz_format_t *formats,
                    int n_formats,
                    int width,
                    int height,
                    glitz_program_map_t *program_map,
                    unsigned long texture_mask)
{
  surface->backend = backend;

  surface->ref_count = 1;

  surface->filter = GLITZ_FILTER_NEAREST;
  surface->polyedge = GLITZ_POLYEDGE_SMOOTH;
  surface->polyedge_smooth_hint = GLITZ_POLYEDGE_SMOOTH_HINT_GOOD;
  surface->polyopacity = 0xffff;
  
  surface->program_map = program_map;
  surface->format = format;
  surface->formats = formats;
  surface->n_formats = n_formats;
  surface->width = width;
  surface->height = height;
  surface->gl = gl;
  surface->draw_buffer = surface->read_buffer = GLITZ_GL_FRONT;
  surface->stencil_mask = surface->stencil_masks;

  if (surface->gl) {
    if (format->doublebuffer)
      surface->draw_buffer = surface->read_buffer = GLITZ_GL_BACK;
                     
    glitz_texture_init (&surface->texture,
                        width, height,
                        glitz_surface_texture_format (surface),
                        texture_mask);
  }
}

void
glitz_surface_fini (glitz_surface_t *surface)
{
  if (surface->gl)
    glitz_texture_fini (surface->gl, &surface->texture);
  
  if (surface->transform)
    free (surface->transform);

  if (surface->inverse_transform)
    free (surface->inverse_transform);

  if (surface->convolution)
    free (surface->convolution);
}

glitz_gl_int_t
glitz_surface_texture_format (glitz_surface_t *surface)
{
  return
    glitz_format_get_best_texture_format (surface->formats,
					  surface->n_formats,
					  surface->format);
}

glitz_format_t *
glitz_surface_find_similar_standard_format (glitz_surface_t *surface,
                                            unsigned long option_mask,
                                            glitz_format_name_t format_name)
{
  return
    glitz_format_find_standard (surface->formats, surface->n_formats,
                                option_mask, format_name);
}
slim_hidden_def(glitz_surface_find_similar_standard_format);

glitz_format_t *
glitz_surface_find_similar_format (glitz_surface_t *surface,
                                   unsigned long mask,
                                   const glitz_format_t *templ,
                                   int count)
{
  return glitz_format_find (surface->formats, surface->n_formats,
                            mask, templ, count);
}

glitz_surface_t *
glitz_surface_create_similar (glitz_surface_t *templ,
                              glitz_format_t *format,
                              int width,
                              int height)
{
  glitz_surface_t *surface;
  
  if (width < 1 || height < 1)
    return NULL;
  
  surface = templ->backend->create_similar (templ, format, width, height);
  if (surface) {
    surface->filter = templ->filter;
    surface->polyedge = templ->polyedge;
    surface->polyedge_smooth_hint = templ->polyedge_smooth_hint;
    surface->polyopacity = templ->polyopacity;
  }
  
  return surface;
}

glitz_surface_t *
glitz_surface_create_intermediate (glitz_surface_t *surface,
                                   glitz_intermediate_t type,
                                   int width,
                                   int height)
{
  glitz_format_t templ;
  glitz_format_t *format;
  glitz_surface_t *intermediate;
  unsigned long mask;
  
  templ.draw.offscreen = 1;
  mask = GLITZ_FORMAT_DRAW_OFFSCREEN_MASK;

  /* component size 8 should instead be 1 but first need to add a
     GLITZ_FORMAT_MINIMUM_MASK. */
  switch (type) {
  case GLITZ_INTERMEDIATE_RGBA_STENCIL:
    templ.stencil_size = 8;
    mask |= GLITZ_FORMAT_STENCIL_SIZE_MASK;
    /* fall-through */
  case GLITZ_INTERMEDIATE_RGBA:
    templ.red_size = 8;
    templ.green_size = 8;
    templ.blue_size = 8;
    mask |= (GLITZ_FORMAT_RED_SIZE_MASK |
             GLITZ_FORMAT_GREEN_SIZE_MASK |
             GLITZ_FORMAT_BLUE_SIZE_MASK);
    /* fall-through */
  case GLITZ_INTERMEDIATE_ALPHA:
    templ.alpha_size = 8;
    mask |= GLITZ_FORMAT_ALPHA_SIZE_MASK;
    break;
  }

  format = glitz_surface_find_similar_format (surface, mask, &templ, 0);
  if (!format) {
    glitz_surface_status_add (surface, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    return NULL;
  }
    
  intermediate = glitz_surface_create_similar (surface, format, width, height);
  
  if (!intermediate)
    glitz_surface_status_add (surface, GLITZ_STATUS_NO_MEMORY_MASK);

  return intermediate;
}

glitz_surface_t *
glitz_surface_create_solid (glitz_color_t *color)
{
  return glitz_programmatic_surface_create_solid (color);
}
slim_hidden_def(glitz_surface_create_solid);
  
glitz_surface_t *
glitz_surface_create_linear (glitz_point_fixed_t *start,
                             glitz_point_fixed_t *end,
                             glitz_color_range_t *color_range)
{
  return glitz_programmatic_surface_create_linear (start, end, color_range);
}
slim_hidden_def(glitz_surface_create_linear);

glitz_surface_t *
glitz_surface_create_radial (glitz_point_fixed_t *center,
                             glitz_fixed16_16_t radius0,
                             glitz_fixed16_16_t radius1,
                             glitz_color_range_t *color_range)
{
  return
    glitz_programmatic_surface_create_radial (center,
                                              radius0, radius1,
                                              color_range);
}
slim_hidden_def(glitz_surface_create_radial);

void
glitz_surface_reference (glitz_surface_t *surface)
{
  if (surface == NULL)
    return;

  surface->ref_count++;
}

void
glitz_surface_destroy (glitz_surface_t *surface)
{
  if (!surface)
    return;

  surface->ref_count--;
  if (surface->ref_count)
    return;
  
  surface->backend->destroy (surface);
}

glitz_bool_t
glitz_surface_push_current (glitz_surface_t *surface,
                            glitz_constraint_t constraint)
{
  if (!surface->backend->push_current (surface, constraint)) {
    glitz_surface_status_add (surface, GLITZ_STATUS_NOT_SUPPORTED_MASK);
    return 0;
  }

  return 1;
}

glitz_bool_t
glitz_surface_try_push_current (glitz_surface_t *surface,
                                glitz_constraint_t constraint)
{
  if (!surface->backend->push_current (surface, constraint))
    return 0;

  return 1;
}

void
glitz_surface_pop_current (glitz_surface_t *surface)
{
  surface->backend->pop_current (surface);
}

void
glitz_surface_set_transform (glitz_surface_t *surface,
                             glitz_transform_t *transform)
{
  static glitz_transform_t identity = {
    {
      { FIXED1, 0x00000, 0x00000 },
      { 0x00000, FIXED1, 0x00000 },
      { 0x00000, 0x00000, FIXED1 }
    }
  };

  if (SURFACE_PROGRAMMATIC (surface)) {
    if (transform == NULL)
      transform = &identity;
    
    glitz_programmatic_surface_set_transform (surface, transform);
    return;
  }

  if (transform && memcmp (transform, &identity,
                           sizeof (glitz_transform_t)) == 0)
    transform = NULL;
  
  if (transform) {
    if (!surface->transform) {
      surface->transform = malloc (sizeof (glitz_matrix_t));
      surface->inverse_transform = malloc (sizeof (glitz_matrix_t));
      if ((!surface->transform) || (!surface->inverse_transform)) {
        if (surface->transform)
          free (surface->transform);
        
        if (surface->inverse_transform)
          free (surface->inverse_transform);
        
        surface->transform = surface->inverse_transform = NULL;
        glitz_surface_status_add (surface, GLITZ_STATUS_NO_MEMORY_MASK);
        return;
      }
    }

    surface->transform->m[0][0] = FIXED_TO_DOUBLE (transform->matrix[0][0]);
    surface->transform->m[1][0] = FIXED_TO_DOUBLE (transform->matrix[0][1]);
    surface->transform->m[2][0] = FIXED_TO_DOUBLE (transform->matrix[0][2]);

    surface->transform->m[0][1] = FIXED_TO_DOUBLE (transform->matrix[1][0]);
    surface->transform->m[1][1] = FIXED_TO_DOUBLE (transform->matrix[1][1]);
    surface->transform->m[2][1] = FIXED_TO_DOUBLE (transform->matrix[1][2]);

    *surface->inverse_transform = *surface->transform;

    if (glitz_matrix_invert (surface->transform)) {
      free (surface->transform);
      free (surface->inverse_transform);
      surface->transform = surface->inverse_transform = NULL;
      glitz_surface_status_add (surface, GLITZ_STATUS_INVALID_MATRIX_MASK);
    }
  } else {
    if (surface->transform)
      free (surface->transform);
    
    if (surface->inverse_transform)
      free (surface->inverse_transform);
    
    surface->transform = surface->inverse_transform = NULL;
  }
}
slim_hidden_def(glitz_surface_set_transform);

void
glitz_surface_set_convolution (glitz_surface_t *surface,
                               glitz_convolution_t *convolution)
{
  if (SURFACE_PROGRAMMATIC (surface))
    return;
  
  if (convolution &&
      convolution->matrix[0][0] == 0x00000 &&
      convolution->matrix[0][1] == 0x00000 &&
      convolution->matrix[0][2] == 0x00000 &&
      convolution->matrix[1][0] == 0x00000 &&
      convolution->matrix[1][2] == 0x00000 &&
      convolution->matrix[2][0] == 0x00000 &&
      convolution->matrix[2][1] == 0x00000 &&
      convolution->matrix[2][2] == 0x00000)
    convolution = NULL;
  
  if (convolution) {
    int row, col;
    
    if (!surface->convolution) {
      surface->convolution = malloc (sizeof (glitz_matrix_t));
      if (!surface->convolution)
        return;
    }

    for (row = 0; row < 3; row++)
      for (col = 0; col < 3; col++)
        surface->convolution->m[row][col] =
          FIXED_TO_DOUBLE (convolution->matrix[row][col]);
    
    if (glitz_matrix_normalize (surface->convolution)) {
      free (surface->convolution);
      surface->convolution = NULL;
      glitz_surface_status_add (surface, GLITZ_STATUS_INVALID_MATRIX_MASK);
    }
  } else {
    if (surface->convolution) {
      free (surface->convolution);
      surface->convolution = NULL;
    }
  }
}
slim_hidden_def(glitz_surface_set_convolution);

void
glitz_surface_set_repeat (glitz_surface_t *surface,
                          glitz_bool_t repeat)
{
  if (SURFACE_PROGRAMMATIC (surface))
    return;

  if (repeat)
    surface->hint_mask |= GLITZ_INT_HINT_REPEAT_MASK;
  else
    surface->hint_mask &= ~GLITZ_INT_HINT_REPEAT_MASK;
    
}
slim_hidden_def(glitz_surface_set_repeat);

void
glitz_surface_set_component_alpha (glitz_surface_t *surface,
                                   glitz_bool_t component_alpha)
{
  if (SURFACE_PROGRAMMATIC (surface))
    return;

  if (component_alpha && surface->format->red_size)
    surface->hint_mask |= GLITZ_INT_HINT_COMPONENT_ALPHA_MASK;
  else
    surface->hint_mask &= ~GLITZ_INT_HINT_COMPONENT_ALPHA_MASK;
    
}
slim_hidden_def(glitz_surface_set_component_alpha);

void
glitz_surface_set_filter (glitz_surface_t *surface,
                          glitz_filter_t filter)
{
  if (SURFACE_PROGRAMMATIC (surface))
    return;
  
  surface->filter = filter;
}
slim_hidden_def(glitz_surface_set_filter);

void
glitz_surface_set_polyedge (glitz_surface_t *surface,
                            glitz_polyedge_t polyedge)
{
  surface->polyedge = polyedge;
}
slim_hidden_def(glitz_surface_set_polyedge);

void
glitz_surface_set_polyopacity (glitz_surface_t *surface,
                               unsigned short polyopacity)
{
  surface->polyopacity = polyopacity;
}
slim_hidden_def(glitz_surface_set_polyopacity);

int
glitz_surface_get_width (glitz_surface_t *surface)
{
  if (SURFACE_PROGRAMMATIC (surface))
    return INT_MAX;
  
  return surface->width;
}
slim_hidden_def(glitz_surface_get_width);

int
glitz_surface_get_height (glitz_surface_t *surface)
{
  if (SURFACE_PROGRAMMATIC (surface))
    return INT_MAX;
  
  return surface->height;
}
slim_hidden_def(glitz_surface_get_height);

glitz_texture_t *
glitz_surface_get_texture (glitz_surface_t *surface)
{
  return surface->backend->get_texture (surface);
}

void
glitz_surface_update_size (glitz_surface_t *surface)
{
  surface->backend->update_size (surface);
}
slim_hidden_def(glitz_surface_update_size);

static glitz_gl_enum_t
_gl_buffer (glitz_buffer_t buffer)
{
  switch (buffer) {
  case GLITZ_BUFFER_FRONT:
    return GLITZ_GL_FRONT;
  case GLITZ_BUFFER_BACK:
    return GLITZ_GL_BACK;
  default:
    return GLITZ_GL_BACK;
  }
}

void
glitz_surface_set_read_buffer (glitz_surface_t *surface,
                               glitz_buffer_t buffer)
{
  glitz_gl_enum_t mode;
  
  if (SURFACE_PROGRAMMATIC (surface) || (!surface->format->doublebuffer))
    return;
    
  mode = _gl_buffer (buffer);
  if (mode != surface->read_buffer) {
    surface->read_buffer = mode;
    glitz_surface_dirty (surface, NULL);
  }
}
slim_hidden_def(glitz_surface_set_read_buffer);

void
glitz_surface_set_draw_buffer (glitz_surface_t *surface,
                               glitz_buffer_t buffer)
{
  if (SURFACE_PROGRAMMATIC (surface) || (!surface->format->doublebuffer))
    return;

  surface->draw_buffer = _gl_buffer (buffer);
  surface->stencil_mask =
    &surface->stencil_masks[(buffer == GLITZ_BUFFER_FRONT)? 1: 0];
}
slim_hidden_def(glitz_surface_set_draw_buffer);

void
glitz_surface_flush (glitz_surface_t *surface)
{
  if (glitz_surface_try_push_current (surface,
                                      GLITZ_CN_SURFACE_DRAWABLE_CURRENT))
    surface->gl->flush ();
  
  glitz_surface_pop_current (surface);
}
slim_hidden_def(glitz_surface_flush);

void
glitz_surface_swap_buffers (glitz_surface_t *surface)
{
  if (SURFACE_PROGRAMMATIC (surface) || (!surface->format->doublebuffer))
    return;
  
  surface->backend->swap_buffers (surface);

  memset (surface->stencil_masks, 0x0,
          sizeof (unsigned int) * GLITZ_N_STENCIL_MASKS);
}
slim_hidden_def(glitz_surface_swap_buffers);

glitz_bool_t
glitz_surface_make_current_read (glitz_surface_t *surface)
{
  return surface->backend->make_current_read (surface);
}

void
glitz_surface_dirty (glitz_surface_t *surface,
                     glitz_bounding_box_t *box)
{ 
  if (surface->draw_buffer != surface->read_buffer)
    return;
  
  if (!box) {
    surface->dirty_box.x1 = surface->dirty_box.y1 = 0;
    surface->dirty_box.x2 = surface->width;
    surface->dirty_box.y2 = surface->height;
  } else {
    if (!SURFACE_DIRTY (surface)) {
      surface->dirty_box = *box;
    } else
      glitz_union_bounding_box (box,
                                &surface->dirty_box,
                                &surface->dirty_box);
  }
  
  surface->hint_mask |= GLITZ_INT_HINT_DIRTY_MASK;
}

void
glitz_surface_status_add (glitz_surface_t *surface, int flags)
{
  surface->status_mask |= flags;
}

glitz_status_t
glitz_surface_get_status (glitz_surface_t *surface)
{
  return glitz_status_pop_from_mask (&surface->status_mask);
}
slim_hidden_def(glitz_surface_get_status);

void
glitz_surface_setup_environment (glitz_surface_t *surface)
{
  glitz_gl_proc_address_list_t *gl = surface->gl;
  
  gl->viewport (0, 0, surface->width, surface->height);
  gl->matrix_mode (GLITZ_GL_PROJECTION);
  gl->load_identity ();
  gl->ortho (0.0, surface->width, 0.0, surface->height, -1.0, 1.0);
  gl->matrix_mode (GLITZ_GL_MODELVIEW);
  gl->load_identity ();
  gl->scale_d (1.0, -1.0, 1.0);
  gl->translate_d (0.0, -surface->height, 0.0);
  gl->disable (GLITZ_GL_DEPTH_TEST); 
  gl->hint (GLITZ_GL_PERSPECTIVE_CORRECTION_HINT, GLITZ_GL_FASTEST);
  gl->disable (GLITZ_GL_CULL_FACE);
  gl->depth_mask (GLITZ_GL_FALSE);
  gl->enable (GLITZ_GL_SCISSOR_TEST);
  gl->scissor (0, 0, surface->width, surface->height);
  gl->polygon_mode (GLITZ_GL_FRONT_AND_BACK, GLITZ_GL_FILL);
  gl->disable (GLITZ_GL_POLYGON_SMOOTH);
  gl->shade_model (GLITZ_GL_FLAT);
  gl->color_mask (GLITZ_GL_TRUE, GLITZ_GL_TRUE, GLITZ_GL_TRUE, GLITZ_GL_TRUE);
  
  if (surface->format->doublebuffer)
    gl->draw_buffer (surface->draw_buffer);

  if (*surface->stencil_mask)
    glitz_set_stencil_operator (gl, GLITZ_STENCIL_OPERATOR_CLIP,
                                *surface->stencil_mask);
  else
    gl->disable (GLITZ_GL_STENCIL_TEST);
}

/* This is supposed to be 8x, 4x and 2x rotated grid multi-sample
   patterns. I'm not sure they're actually correct. */
static glitz_sample_offset_t _8x_multi_sample_offsets[] = {
  { -0.375, -0.375 },
  { -0.125, -0.125 },
  {  0.375, -0.375 },
  {  0.125, -0.125 },
  {  0.375,  0.375 },
  {  0.125,  0.125 },
  { -0.375,  0.375 },
  { -0.125,  0.125 }
};

static unsigned short _8x_multi_sample_weights[] = {
  0x2000, 0x4000, 0x6000, 0x8000, 0xa000, 0xbfff, 0xdfff, 0xffff
};

static glitz_multi_sample_info_t _8x_multi_sample = {
  _8x_multi_sample_offsets,
  _8x_multi_sample_weights,
  8
};

static glitz_sample_offset_t _4x_multi_sample_offsets[] = {
  { -0.125, -0.375 },
  {  0.375, -0.125 },
  {  0.125,  0.375 },
  { -0.375,  0.125 }
};

static unsigned short _4x_multi_sample_weights[] = {
  0x4000, 0x8000, 0xbfff, 0xffff
};

static glitz_multi_sample_info_t _4x_multi_sample = {
  _4x_multi_sample_offsets,
  _4x_multi_sample_weights,
  4
};

static glitz_sample_offset_t _2x_multi_sample_offsets[] = {
  { -0.24, -0.24 },
  {  0.24,  0.24 }
};

static unsigned short _2x_multi_sample_weights[] = {
  0x8000, 0xffff
};

static glitz_multi_sample_info_t _2x_multi_sample = {
  _2x_multi_sample_offsets,
  _2x_multi_sample_weights,
  2
};

void
glitz_surface_enable_anti_aliasing (glitz_surface_t *surface)
{
  if (surface->format->multisample.samples > 1)
    return;

  if (surface->polyedge == GLITZ_POLYEDGE_SMOOTH &&
      surface->format->stencil_size >= 4) {
    switch (surface->polyedge_smooth_hint) {
    case GLITZ_POLYEDGE_SMOOTH_HINT_BEST:
      if (surface->format->stencil_size >= 8) {
        surface->multi_sample = &_8x_multi_sample;
        break;
      }
      /* fall-through */
    case GLITZ_POLYEDGE_SMOOTH_HINT_GOOD:
      surface->multi_sample = &_4x_multi_sample;
      break;
    case GLITZ_POLYEDGE_SMOOTH_HINT_FAST:
      surface->multi_sample = &_2x_multi_sample;
      break;
    }
  }
}

void
glitz_surface_disable_anti_aliasing (glitz_surface_t *surface)
{
  surface->multi_sample = NULL;
}

void
glitz_surface_set_polyedge_smooth_hint (glitz_surface_t *surface,
                                        glitz_polyedge_smooth_hint_t hint)
{
  surface->polyedge_smooth_hint = hint;
}

void
glitz_surface_get_gl_texture (glitz_surface_t *surface,
                              unsigned int *name,
                              unsigned int *target,
                              double *texcoord_width,
                              double *texcoord_height,
                              glitz_bool_t *repeatable)
{
  glitz_texture_t *texture = glitz_surface_get_texture (surface);
  
  if (!texture)
    return;

  if (!SURFACE_PROGRAMMATIC (surface)) {
    glitz_texture_bind (surface->gl, texture);
    glitz_texture_ensure_filter (surface->gl, texture, surface->filter);
    glitz_texture_ensure_repeat (surface->gl, texture,
                                 (SURFACE_REPEAT (surface) &&
                                  texture->repeatable));
    glitz_texture_unbind (surface->gl, texture);
  }

  if (name)
    *name = texture->name;

  if (target)
    *target = texture->target;

  if (texcoord_width)
    *texcoord_width = texture->texcoord_width;

  if (texcoord_height)
    *texcoord_height = texture->texcoord_height;

  if (repeatable)
    *repeatable = texture->repeatable;
}
slim_hidden_def(glitz_surface_get_gl_texture);

void
glitz_surface_gl_begin (glitz_surface_t *surface)
{
  glitz_surface_push_current (surface, GLITZ_CN_SURFACE_DRAWABLE_CURRENT);
}
slim_hidden_def(glitz_surface_gl_begin);

void
glitz_surface_gl_end (glitz_surface_t *surface)
{
  glitz_bounding_box_t box;

  box.x1 = box.y1 = 0;
  box.x2 = surface->width;
  box.y2 = surface->height;
  
  glitz_surface_dirty (surface, &box);
  
  glitz_surface_pop_current (surface);
}
slim_hidden_def(glitz_surface_gl_end);

unsigned long
glitz_surface_get_features (glitz_surface_t *surface)
{
  return surface->feature_mask;
}
slim_hidden_def(glitz_surface_get_features);

void
glitz_surface_clip_rectangles (glitz_surface_t *surface,
                               glitz_clip_operator_t op,
                               const glitz_rectangle_t *rects,
                               int n_rects)
{
  glitz_stencil_rectangles (surface, (glitz_stencil_operator_t) op,
                            rects, n_rects);
}
slim_hidden_def(glitz_surface_clip_rectangles);

void
glitz_surface_clip_trapezoids (glitz_surface_t *surface,
                               glitz_clip_operator_t op,
                               const glitz_trapezoid_t *traps,
                               int n_traps)
{
  glitz_stencil_trapezoids (surface, (glitz_stencil_operator_t) op,
                            traps, n_traps);
}
slim_hidden_def(glitz_surface_clip_trapezoids);

void
glitz_surface_clip_triangles (glitz_surface_t *surface,
                              glitz_clip_operator_t op,
                              const glitz_triangle_t *tris,
                              int n_tris)
{
  glitz_stencil_triangles (surface, (glitz_stencil_operator_t) op,
                           GLITZ_TRIANGLE_TYPE_NORMAL,
                           (glitz_point_fixed_t *) tris, n_tris * 3);
}
slim_hidden_def(glitz_surface_clip_triangles);

glitz_format_t *
glitz_surface_get_format (glitz_surface_t *surface)
{
  return surface->format;
}
slim_hidden_def(glitz_surface_get_format);

unsigned long
glitz_surface_get_hints (glitz_surface_t *surface)
{
  unsigned hint_mask;

  hint_mask = surface->hint_mask &
    (GLITZ_HINT_OFFSCREEN_MASK | GLITZ_HINT_PROGRAMMATIC_MASK);

  if (*surface->stencil_mask)
    hint_mask |= GLITZ_HINT_CLIPPING_MASK;
  
  if ((!SURFACE_PROGRAMMATIC (surface)) &&
      surface->polyedge == GLITZ_POLYEDGE_SMOOTH &&
      surface->format->multisample.samples < 2 &&
      surface->format->stencil_size >= 4)
    hint_mask |= GLITZ_HINT_MULTISAMPLE_MASK;
  
  return hint_mask;
}
slim_hidden_def(glitz_surface_get_hints);
