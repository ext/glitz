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

#include <math.h>

static void
_glitz_matrix_transform_distance (glitz_matrix_t *matrix,
                                  double *dx,
                                  double *dy)
{
  double new_x, new_y;

  new_x = (matrix->m[0][0] * *dx + matrix->m[1][0] * *dy);
  new_y = (matrix->m[0][1] * *dx + matrix->m[1][1] * *dy);
    
  *dx = new_x;
  *dy = new_y;
}

void
glitz_matrix_transform_point (glitz_matrix_t *matrix,
                              glitz_point_t *point)
{
  _glitz_matrix_transform_distance (matrix, &point->x, &point->y);

  point->x += matrix->m[2][0];
  point->y += matrix->m[2][1];
}

void
glitz_matrix_transform_region (glitz_matrix_t *matrix,
                               glitz_region_box_t *region)
{
  glitz_point_t point;

  point.x = region->x1;
  point.y = region->y1;
  _glitz_matrix_transform_distance (matrix, &point.x, &point.y);
  region->x1 = floor (point.x);
  region->y1 = floor (point.y);

  point.x = region->x2;
  point.y = region->y2;
  _glitz_matrix_transform_distance (matrix, &point.x, &point.y);
  region->x2 = ceil (point.x);
  region->y2 = ceil (point.y);
  
  region->x1 += (int) floor (matrix->m[2][0]);
  region->y1 += (int) floor (matrix->m[2][1]);
  region->x2 += (int) ceil (matrix->m[2][0]);
  region->y2 += (int) ceil (matrix->m[2][1]);
}

void
glitz_matrix_transform_sub_pixel_region (glitz_matrix_t *matrix,
                                         glitz_sub_pixel_region_box_t *region)
{
  _glitz_matrix_transform_distance (matrix, &region->x1, &region->y1);
  _glitz_matrix_transform_distance (matrix, &region->x2, &region->y2);
  
  region->x1 += matrix->m[2][0];
  region->y1 += matrix->m[2][1];
  region->x2 += matrix->m[2][0];
  region->y2 += matrix->m[2][1];
}

static void
_glitz_matrix_set_affine (glitz_matrix_t *matrix,
                          double a, double b,
                          double c, double d,
                          double tx, double ty)
{  
  matrix->m[0][0] =  a; matrix->m[0][1] =  b;
  matrix->m[1][0] =  c; matrix->m[1][1] =  d;
  matrix->m[2][0] = tx; matrix->m[2][1] = ty;
}

static void
_glitz_matrix_scalar_multiply (glitz_matrix_t *matrix,
                               double scalar)
{
  int row, col;

  for (row = 0; row < 3; row++)
    for (col = 0; col < 2; col++)
      matrix->m[row][col] *= scalar;
}

/* This function isn't a correct adjoint in that the implicit 1 in the
   homogeneous result should actually be ad-bc instead. But, since this
   adjoint is only used in the computation of the inverse, which
   divides by det (A)=ad-bc anyway, everything works out in the end. */
static void
_glitz_matrix_compute_adjoint (glitz_matrix_t *matrix)
{
  /* adj (A) = transpose (C:cofactor (A,i,j)) */
  double a, b, c, d, tx, ty;

  a  = matrix->m[0][0]; b  = matrix->m[0][1];
  c  = matrix->m[1][0]; d  = matrix->m[1][1];
  tx = matrix->m[2][0]; ty = matrix->m[2][1];
  
  _glitz_matrix_set_affine (matrix,
                            d, -b,
                            -c, a,
                            c * ty - d * tx, b * tx - a * ty);
}

static void
_glitz_matrix_compute_determinant (glitz_matrix_t *matrix,
                                   double *det)
{
  double a, b, c, d;

  a = matrix->m[0][0]; b = matrix->m[0][1];
  c = matrix->m[1][0]; d = matrix->m[1][1];

  *det = a * d - b * c;
}

glitz_status_t
glitz_matrix_invert (glitz_matrix_t *matrix)
{
  /* inv (A) = 1/det (A) * adj (A) */
  double det;

  _glitz_matrix_compute_determinant (matrix, &det);
    
  if (det == 0)
    return GLITZ_STATUS_INVALID_MATRIX;

  _glitz_matrix_compute_adjoint (matrix);
  _glitz_matrix_scalar_multiply (matrix, 1 / det);

  return GLITZ_STATUS_SUCCESS;
}

void
glitz_matrix_translate (glitz_matrix_t *matrix,
                        double tx,
                        double ty)
{  
  matrix->m[2][0] += tx;
  matrix->m[2][1] += ty;
}

/* This function is only used for convolution kernel normalization.
   I'm not sure that it does the right thing when kernel contains negative
   values or when the sum equals zero. */
glitz_status_t
glitz_matrix_normalize (glitz_matrix_t *matrix)
{
  int row, col;
  double sum = 0.0;

  for (row = 0; row < 3; row++)
    for (col = 0; col < 3; col++)
      sum += matrix->m[row][col];

  if (sum != 0.0) {
    sum = 1.0 / sum;

    for (row = 0; row < 3; row++)
      for (col = 0; col < 3; col++)
        matrix->m[row][col] *= sum;
  }
  
  return GLITZ_STATUS_SUCCESS;
}
