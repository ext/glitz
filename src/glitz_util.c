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
glitz_intersect_bounding_box (glitz_bounding_box_t *box1,
                              glitz_bounding_box_t *box2,
                              glitz_bounding_box_t *return_box)
{
  return_box->x1 = (box1->x1 >= box2->x1)? box1->x1: box2->x1;
  return_box->x2 = (box1->x2 <= box2->x2)? box1->x2: box2->x2;
  return_box->y1 = (box1->y1 >= box2->y1)? box1->y1: box2->y1;
  return_box->y2 = (box1->y2 <= box2->y2)? box1->y2: box2->y2;

  if (return_box->x1 >= return_box->x2)
    return_box->x1 = return_box->x2 = 0;
  
  if (return_box->y1 >= return_box->y2)
    return_box->y1 = return_box->y2 = 0;
}

void
glitz_union_bounding_box (glitz_bounding_box_t *box1,
                          glitz_bounding_box_t *box2,
                          glitz_bounding_box_t *return_box)
{
  return_box->x1 = (box1->x1 <= box2->x1)? box1->x1: box2->x1;
  return_box->x2 = (box1->x2 >= box2->x2)? box1->x2: box2->x2;
  return_box->y1 = (box1->y1 <= box2->y1)? box1->y1: box2->y1;
  return_box->y2 = (box1->y2 >= box2->y2)? box1->y2: box2->y2;
}

static glitz_bool_t
_glitz_extension_check (const char *extensions,
                        const char *ext_name)
{
  char *end;
  char *p = (char *) extensions;
  int ext_name_len = strlen (ext_name);

  if (! p)
    return 0;

  end = p + strlen (p);

  while (p < end) {
    int n = strcspn (p, " ");

    if ((ext_name_len == n) && (strncmp (ext_name, p, n) == 0)) {
      return 1;
    }
    p += (n + 1);
  }
  return 0;
}

long int
glitz_extensions_query (const char *extensions_string,
                        glitz_extension_map *extensions_map)
{
  long int mask = 0;
  int i;

  for (i = 0; extensions_map[i].name; i++)
    if (_glitz_extension_check (extensions_string, extensions_map[i].name))
      mask |= extensions_map[i].mask;

  return mask;
}

unsigned int
glitz_uint_to_power_of_two (unsigned int x)
{
  x |= (x >> 1);
  x |= (x >> 2);
  x |= (x >> 4);
  x |= (x >> 8);
  x |= (x >> 16);
  
  return (x + 1);
}

void
glitz_set_raster_pos (glitz_gl_proc_address_list_t *gl,
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
glitz_clamp_value (double *value, double min, double max)
{
  if (*value < min)
    *value = min;
  else if (*value > max)
    *value = max;
}

