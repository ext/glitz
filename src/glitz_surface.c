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
                    const glitz_surface_backend_t *backend)
{
  surface->backend = backend;

  surface->filter = GLITZ_FILTER_NEAREST;
  surface->polyedge = GLITZ_POLYEDGE_SMOOTH;
}

void
glitz_surface_deinit (glitz_surface_t *surface)
{
  if (surface->transforms)
    free (surface->transforms);

  if (surface->convolution)
    free (surface->convolution);
}

glitz_surface_t *
glitz_int_surface_create_similar (glitz_surface_t *templ,
                                  glitz_format_name_t format_name,
                                  glitz_bool_t drawable,
                                  int width,
                                  int height)
{
  glitz_surface_t *surface;

  if (width < 1 || height < 1)
    return NULL;
  
  surface = templ->backend->create_similar (templ, format_name, drawable,
                                            width, height);
  
  if (surface)
    surface->polyedge = templ->polyedge;
  
  return surface;
}

glitz_surface_t *
glitz_surface_create_similar (glitz_surface_t *templ,
                              glitz_format_name_t format_name,
                              int width,
                              int height)
{
  return
    glitz_int_surface_create_similar (templ, format_name, 0, width, height);
}
slim_hidden_def(glitz_surface_create_similar);

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
                             glitz_distance_fixed_t *radius,
                             glitz_color_range_t *color_range)
{
  return
    glitz_programmatic_surface_create_radial (center, radius, color_range);
}
slim_hidden_def(glitz_surface_create_radial);

void
glitz_surface_destroy (glitz_surface_t *surface)
{
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

void
glitz_surface_pop_current (glitz_surface_t *surface)
{
  surface->backend->pop_current (surface);
}

void
glitz_surface_enable_program (glitz_program_type_t type,
                              glitz_surface_t *surface,
                              glitz_surface_t *src,
                              glitz_surface_t *mask,
                              glitz_texture_t *src_texture,
                              glitz_texture_t *mask_texture)
{
  glitz_program_enable (type, surface, src, mask, src_texture, mask_texture);
}

void
glitz_surface_disable_program (glitz_program_type_t type,
                               glitz_surface_t *surface)
{
  glitz_program_disable (type, surface);
}

void
glitz_surface_push_transform (glitz_surface_t *surface)
{
  static const glitz_matrix_t identity = {
    {
      { 1.0, 0.0, 0.0 },
      { 0.0, 1.0, 0.0 },
      { 0.0, 0.0, 1.0 }
    }
  };
  
  surface->n_transforms++;
  
  surface->transforms =
    realloc (surface->transforms,
             surface->n_transforms * sizeof (glitz_matrix_t));

  if (!surface->transforms)
    return;
  
  if (surface->n_transforms > 1) {
    surface->transforms[surface->n_transforms - 1] =
      surface->transforms[surface->n_transforms - 2];
  } else
    surface->transforms[surface->n_transforms - 1] = identity;
  
  surface->transform = &surface->transforms[surface->n_transforms - 1];
}

void
glitz_surface_pop_transform (glitz_surface_t *surface)
{
  if (surface->n_transforms < 1)
    return;
  
  surface->n_transforms--;
  
  surface->transforms =
    realloc (surface->transforms,
             surface->n_transforms * sizeof (glitz_matrix_t));

  if (surface->n_transforms)
    surface->transform = &surface->transforms[surface->n_transforms - 1];
  else
    surface->transform = NULL;
}

void
glitz_surface_bounds (glitz_surface_t *surface,
                      glitz_region_box_t *box)
{
  box->y1 = MINSHORT;
  box->y2 = MAXSHORT;
  box->x1 = MINSHORT;
  box->x2 = MAXSHORT;
  
  if (SURFACE_PROGRAMMATIC (surface) ||
      SURFACE_REPEAT (surface) ||
      surface->transform)
    return;
    
  box->x1 = 0;
  box->y1 = 0;
  box->x2 = surface->width;
  box->y2 = surface->height;
}

void
glitz_surface_set_transform (glitz_surface_t *surface,
                             glitz_transform_t *transform)
{
  static const glitz_transform_t identity = {
    {
      { FIXED1, 0x00000, 0x00000 },
      { 0x00000, FIXED1, 0x00000 },
      { 0x00000, 0x00000, FIXED1 }
    }
  };

  if (SURFACE_PROGRAMMATIC (surface))
    return;

  if (transform && memcmp (transform, &identity,
                           sizeof (glitz_transform_t)) == 0)
    transform = NULL;
  
  if (transform) {
    if (!surface->transform) {
      glitz_surface_push_transform (surface);
      if (!surface->transform)
		return;
	}

    surface->transform->m[0][0] = FIXED_TO_DOUBLE (transform->matrix[0][0]);
    surface->transform->m[1][0] = FIXED_TO_DOUBLE (transform->matrix[0][1]);
    surface->transform->m[2][0] = FIXED_TO_DOUBLE (transform->matrix[0][2]);

    surface->transform->m[0][1] = FIXED_TO_DOUBLE (transform->matrix[1][0]);
    surface->transform->m[1][1] = FIXED_TO_DOUBLE (transform->matrix[1][1]);
    surface->transform->m[2][1] = FIXED_TO_DOUBLE (transform->matrix[1][2]);

    if (glitz_matrix_invert (surface->transform)) {
      glitz_surface_pop_transform (surface);
      glitz_surface_status_add (surface, GLITZ_STATUS_INVALID_MATRIX_MASK);
    }
  } else
    glitz_surface_pop_transform (surface);
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

static void
_glitz_set_raster_pos (glitz_gl_proc_address_list_t *gl,
                       int x,
                       int y)
{
  gl->push_attrib (GLITZ_GL_TRANSFORM_BIT | GLITZ_GL_VIEWPORT_BIT);
  gl->matrix_mode (GLITZ_GL_PROJECTION);
  gl->push_matrix ();
  gl->load_identity ();
  gl->matrix_mode (GLITZ_GL_MODELVIEW);
  gl->push_matrix ();
  gl->load_identity ();
  gl->depth_range (0, 1);
  gl->viewport (-1, -1, 2, 2);
  
  gl->raster_pos_2d (0, 0);
  gl->bitmap (0, 0, 1, 1, x, y, NULL);
  
  gl->pop_matrix ();
  gl->matrix_mode (GLITZ_GL_PROJECTION);
  gl->pop_matrix ();
  gl->pop_attrib ();
}

void
glitz_surface_flush (glitz_surface_t *surface,
                     int x,
                     int y,
                     unsigned int width,
                     unsigned int height)
{
  if (SURFACE_PROGRAMMATIC (surface))
    return;
  
  if (surface->format->doublebuffer &&
      x <= 0 && y <= 0 &&
      (x + (int) width) >= surface->width &&
      (y + (int) height) >= surface->height) {
    surface->backend->flush (surface);
  } else if (width > 0 && height > 0) {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (glitz_surface_push_current (surface,
                                    GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
      if (surface->format->doublebuffer) {
        surface->gl->read_buffer (GLITZ_GL_BACK);
        surface->gl->draw_buffer (GLITZ_GL_FRONT);
        surface->gl->disable (GLITZ_GL_SCISSOR_TEST);
        surface->gl->disable (GLITZ_GL_DITHER);
        glitz_set_operator (surface->gl, GLITZ_OPERATOR_SRC);

        _glitz_set_raster_pos (surface->gl, x, surface->height - (y + height));
        surface->gl->copy_pixels (x, surface->height - (y + height),
                                  width, height, GLITZ_GL_COLOR);
        
        surface->gl->draw_buffer (GLITZ_GL_BACK);
      }
      surface->gl->flush ();
    }
    glitz_surface_pop_current (surface);
  }
}
slim_hidden_def(glitz_surface_flush);

void
glitz_surface_dirty (glitz_surface_t *surface,
                     glitz_region_box_t *region)
{
  if (!region) {
    surface->dirty_region.x1 = surface->dirty_region.y1 = 0;
    surface->dirty_region.x2 = surface->width;
    surface->dirty_region.y2 = surface->height;
  } else {
    if (!SURFACE_DIRTY (surface)) {
      surface->dirty_region = *region;
    } else
      glitz_union_region (region,
                          &surface->dirty_region,
                          &surface->dirty_region);
  }
  
  surface->hint_mask |= GLITZ_INT_HINT_DIRTY_MASK;
  
  surface->gl->flush ();
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
  
  gl->pixel_store_i (GLITZ_GL_PACK_ALIGNMENT, 4);
  gl->pixel_store_i (GLITZ_GL_UNPACK_ALIGNMENT, 4);
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

  if (surface->clip_mask)
    glitz_set_clip_operator (gl, GLITZ_INT_CLIP_OPERATOR_CLIP,
                             surface->clip_mask);
  else
    gl->disable (GLITZ_GL_STENCIL_TEST);
}

void
glitz_surface_read_pixels (glitz_surface_t *surface,
                           int x,
                           int y,
                           unsigned int width,
                           unsigned int height,
                           char *pixels)
{
  unsigned char *pixel_buf;
  int rowstride, area_rowstride, bytes_per_pixel;
  unsigned int i;
  glitz_gl_enum_t format, type;

  if (SURFACE_PROGRAMMATIC (surface))
    return;

  if (x < 0 || x > (int) surface->width - (int) width ||
      y < 0 || y > (int) surface->height - (int) height) {
    glitz_surface_status_add (surface, GLITZ_STATUS_BAD_COORDINATE_MASK);
    return;
  }

  bytes_per_pixel = (surface->format->bpp / 8);
  format = glitz_get_gl_format_from_bpp (surface->format->bpp);
  type = glitz_get_gl_data_type_from_bpp (surface->format->bpp);

  /* We currently read the whole image to a temporary buffer and then
     copy the part we want, not very efficient. We only want to read the
     area requested. I think it can be fixed with glPixelStore parameters. */

  if (glitz_surface_push_current (surface,
                                  GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    rowstride = surface->width * bytes_per_pixel;
    rowstride += (rowstride % 4)? (4 - (rowstride % 4)): 0;
    pixel_buf = (unsigned char *) malloc (surface->height * rowstride);
  
    surface->gl->read_pixels (0, 0, surface->width, surface->height,
                              format, type, pixel_buf);
  } else {
    glitz_texture_t *texture;

    texture = glitz_surface_get_texture (surface);
    glitz_texture_bind (surface->gl, texture);

    rowstride = texture->width * bytes_per_pixel;
    rowstride += (rowstride % 4)? (4 - (rowstride % 4)): 0;
    pixel_buf = (unsigned char *) malloc (texture->height * rowstride);

    surface->gl->pixel_store_i (GLITZ_GL_UNPACK_ROW_LENGTH, 0);
    surface->gl->pixel_store_i (GLITZ_GL_UNPACK_SKIP_ROWS, 0);
    surface->gl->pixel_store_i (GLITZ_GL_UNPACK_SKIP_PIXELS, 0);
    
    surface->gl->get_tex_image (texture->target, 0,
                                format,
                                type,
                                pixel_buf);

    glitz_texture_unbind (surface->gl, texture);
  }

  glitz_surface_pop_current (surface);

  area_rowstride = width * bytes_per_pixel;
  area_rowstride += (area_rowstride % 4)? (4 - (area_rowstride % 4)): 0;

  for (i = 0; i < height; i++) {
    memcpy (&pixels[(height - i - 1) * area_rowstride],
            &pixel_buf[(surface->height - y - height + i) * rowstride + x],
            area_rowstride);
  }
  
  free (pixel_buf);
}
slim_hidden_def(glitz_surface_read_pixels);

void
glitz_surface_draw_pixels (glitz_surface_t *surface,
                           int x,
                           int y,
                           unsigned int width,
                           unsigned int height,
                           char *pixels)
{
  unsigned char *pixel_buf;
  glitz_gl_enum_t format, type;
  int bytes_per_pixel;

  if (SURFACE_PROGRAMMATIC (surface))
    return;

  if (x < 0 || x > (surface->width - (int) width) ||
      y < 0 || y > (surface->height - (int) height)) {
    glitz_surface_status_add (surface, GLITZ_STATUS_BAD_COORDINATE_MASK);
    return;
  }

  bytes_per_pixel = (surface->format->bpp / 8);
  format = glitz_get_gl_format_from_bpp (surface->format->bpp);
  type = glitz_get_gl_data_type_from_bpp (surface->format->bpp);

  if (glitz_surface_push_current (surface,
                                  GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_region_box_t bounds;

    bounds.x1 = x;
    bounds.x2 = x + width;
    bounds.y1 = y;
    bounds.y2 = y + height;

    surface->gl->disable (GLITZ_GL_SCISSOR_TEST);
    surface->gl->disable (GLITZ_GL_DITHER);
    glitz_set_operator (surface->gl, GLITZ_OPERATOR_SRC);
    
    surface->gl->pixel_zoom (1.0, -1.0);
    
    _glitz_set_raster_pos (surface->gl, x, surface->height - y);
    surface->gl->draw_pixels (width, height, format, type, pixels);

    glitz_surface_dirty (surface, &bounds);
  } else {
    int i, rowstride;
    glitz_texture_t *texture;
    
    rowstride = width * bytes_per_pixel;
    rowstride += (rowstride % 4)? (4 - (rowstride % 4)): 0;
    pixel_buf = (unsigned char *) malloc (rowstride * height);
    
    /* TODO: This is very ugly and I like to remove it as soon as possible.
       Needs some changes to texture handling, as images will become upside
       down without it. */
    for (i = 0; i < (int) height; i++)
      memcpy (&pixel_buf[i * rowstride],
              &pixels[(height - i - 1) * rowstride], rowstride);

    texture = glitz_surface_get_texture (surface);
    glitz_texture_bind (surface->gl, texture);
    
    surface->gl->pixel_store_i (GLITZ_GL_PACK_ROW_LENGTH, 0);
    surface->gl->pixel_store_i (GLITZ_GL_PACK_SKIP_ROWS, 0);
    surface->gl->pixel_store_i (GLITZ_GL_PACK_SKIP_PIXELS, 0);
    
    surface->gl->tex_sub_image_2d (texture->target, 0,
                                   x, surface->height - y - height,
                                   width, height,
                                   format, type,
                                   pixel_buf);
    surface->gl->flush ();

    glitz_texture_unbind (surface->gl, texture);
    
    free (pixel_buf);
  }

  glitz_surface_pop_current (surface);
}
slim_hidden_def(glitz_surface_draw_pixels);

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
  if (!glitz_surface_push_current (surface, GLITZ_CN_SURFACE_DRAWABLE_CURRENT))
    glitz_surface_status_add (surface, GLITZ_STATUS_NOT_SUPPORTED_MASK);
}
slim_hidden_def(glitz_surface_gl_begin);

void
glitz_surface_gl_end (glitz_surface_t *surface)
{
  glitz_region_box_t region;

  region.x1 = region.y1 = 0;
  region.x2 = surface->width;
  region.y2 = surface->height;
  
  glitz_surface_dirty (surface, &region);
  
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
glitz_int_surface_clip_rectangles (glitz_surface_t *surface,
                                   glitz_int_clip_operator_t op,
                                   int mask,
                                   const glitz_rectangle_t *rects,
                                   int n_rects)
{
  static glitz_color_t color = { 0x0000, 0x0000, 0x0000, 0x0000 };

  if (n_rects == 0)
    return;

  if ((op == GLITZ_INT_CLIP_OPERATOR_SET ||
       op == GLITZ_INT_CLIP_OPERATOR_UNION) &&
      (n_rects == 1 &&
       rects->x <= 0 && rects->y <= 0 &&
       rects->width >= surface->width &&
       rects->height >= surface->height)) {
    surface->clip_mask = 0x0;
    return;
  }

  if (surface->format->stencil_size < 1)
    return;

  if (!glitz_surface_push_current (surface,
                                   GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_pop_current (surface);
    return;
  }

  glitz_set_clip_operator (surface->gl, op, mask);
  
  surface->gl->color_mask (GLITZ_GL_FALSE, GLITZ_GL_FALSE,
                           GLITZ_GL_FALSE, GLITZ_GL_FALSE);

  glitz_int_fill_rectangles ((glitz_operator_t)
                             ((op == GLITZ_INT_CLIP_OPERATOR_SET)?
                              GLITZ_INT_OPERATOR_STENCIL_RECT_SET:
                              GLITZ_INT_OPERATOR_STENCIL_RECT_SRC),
                             surface,
                             &color,
                             rects,
                             n_rects);
  
  surface->gl->color_mask (GLITZ_GL_TRUE, GLITZ_GL_TRUE,
                           GLITZ_GL_TRUE, GLITZ_GL_TRUE);
  
  surface->clip_mask = mask;

  glitz_surface_pop_current (surface);
}

void
glitz_surface_clip_rectangles (glitz_surface_t *surface,
                               glitz_clip_operator_t op,
                               const glitz_rectangle_t *rects,
                               int n_rects)
{
  glitz_int_surface_clip_rectangles (surface, (glitz_int_clip_operator_t) op,
                                     0x1, rects, n_rects);
}
slim_hidden_def(glitz_surface_clip_rectangles);

void
glitz_int_surface_clip_trapezoids (glitz_surface_t *surface,
                                   glitz_int_clip_operator_t op,
                                   int mask,
                                   const glitz_trapezoid_t *traps,
                                   int n_traps)
{
  static glitz_color_t color = { 0x0000, 0x0000, 0x0000, 0x0000 };

  if (n_traps == 0)
    return;

  if ((op == GLITZ_INT_CLIP_OPERATOR_SET ||
       op == GLITZ_INT_CLIP_OPERATOR_UNION) &&
      (n_traps == 1 &&
       FIXED_TO_INT (traps->top) <= 0 &&
       FIXED_TO_INT (traps->bottom) >= surface->height &&
       FIXED_TO_INT (traps->left.p1.x) <= 0 &&
       FIXED_TO_INT (traps->left.p2.x) <= 0 &&
       FIXED_TO_INT (traps->right.p1.x) >= surface->width &&
       FIXED_TO_INT (traps->right.p2.x) >= surface->width)) {
    surface->clip_mask = 0x0;
    return;
  }
  
  if (surface->format->stencil_size < 1)
    return;

  if (!glitz_surface_push_current (surface,
                                   GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_pop_current (surface);
    return;
  }

  glitz_set_clip_operator (surface->gl, op, mask);
  
  surface->gl->color_mask (GLITZ_GL_FALSE, GLITZ_GL_FALSE,
                           GLITZ_GL_FALSE, GLITZ_GL_FALSE);

  glitz_int_fill_trapezoids (GLITZ_OPERATOR_SRC,
                             surface,
                             0, 0,
                             &color,
                             traps,
                             n_traps);
    
  surface->gl->color_mask (GLITZ_GL_TRUE, GLITZ_GL_TRUE,
                           GLITZ_GL_TRUE, GLITZ_GL_TRUE);

  surface->clip_mask = mask;

  glitz_surface_pop_current (surface);
}

void
glitz_surface_clip_trapezoids (glitz_surface_t *surface,
                               glitz_clip_operator_t op,
                               const glitz_trapezoid_t *traps,
                               int n_traps)
{
  glitz_int_surface_clip_trapezoids (surface, (glitz_int_clip_operator_t) op,
                                     0x1, traps, n_traps);
}
slim_hidden_def(glitz_surface_clip_trapezoids);

void
glitz_int_surface_clip_triangles (glitz_surface_t *surface,
                                  glitz_int_clip_operator_t op,
                                  int mask,
                                  glitz_triangle_type_t type,
                                  const glitz_point_fixed_t *points,
                                  int n_points)                               
{
  static glitz_color_t color = { 0x0000, 0x0000, 0x0000, 0x0000 };

  if (n_points < 3)
    return;
  
  if (surface->format->stencil_size < 1)
    return;

  if (!glitz_surface_push_current (surface,
                                   GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
    glitz_surface_pop_current (surface);
    return;
  }
  
  glitz_set_clip_operator (surface->gl, op, mask);
  
  surface->gl->color_mask (GLITZ_GL_FALSE, GLITZ_GL_FALSE,
                           GLITZ_GL_FALSE, GLITZ_GL_FALSE);

  glitz_int_fill_triangles (GLITZ_OPERATOR_SRC,
                            surface,
                            type,
                            0, 0,
                            &color,                        
                            points,
                            n_points);
  
  surface->gl->color_mask (GLITZ_GL_TRUE, GLITZ_GL_TRUE,
                           GLITZ_GL_TRUE, GLITZ_GL_TRUE);

  surface->clip_mask = mask;

  glitz_surface_pop_current (surface);
}

void
glitz_surface_clip_triangles (glitz_surface_t *surface,
                              glitz_clip_operator_t op,
                              const glitz_triangle_t *tris,
                              int n_tris)
{
  glitz_int_surface_clip_triangles (surface,
                                    (glitz_int_clip_operator_t) op,
                                    0x1,
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

  if (surface->clip_mask)
    hint_mask |= GLITZ_HINT_CLIPPING_MASK;
  
  return hint_mask;
}
slim_hidden_def(glitz_surface_get_hints);
