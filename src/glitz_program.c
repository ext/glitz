/*
 * Copyright © 2004 David Reveman
 * 
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the names of
 * David Reveman not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * David Reveman makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DAVID REVEMAN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DAVID REVEMAN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, 
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <c99drn@cs.umu.se>
 */

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "glitzint.h"

#include <stdio.h>

#define EXPAND_NONE ""
#define EXPAND_2D "2D"
#define EXPAND_RECT "RECT"

#define EXPAND_NO_IN_OP \
  "MUL result.color, color, fragment.color.a;"

#define EXPAND_SRC_DECL "TEMP src_pos, src;"
#define EXPAND_SRC_2D_IN_OP \
  "DP4 src_pos.x, state.matrix.texture[1].row[0], fragment.texcoord[1];" \
  "DP4 src_pos.y, state.matrix.texture[1].row[1], fragment.texcoord[1];" \
  "DP4 src_pos.w, state.matrix.texture[1].row[3], fragment.texcoord[1];" \
  "TXP src, src_pos, texture[1], 2D;" \
  "DP4 color.a, color, fragment.color;" \
  "MUL result.color, src, color.a;"
#define EXPAND_SRC_RECT_IN_OP \
  "DP4 src_pos.x, state.matrix.texture[1].row[0], fragment.texcoord[1];" \
  "DP4 src_pos.y, state.matrix.texture[1].row[1], fragment.texcoord[1];" \
  "DP4 src_pos.w, state.matrix.texture[1].row[3], fragment.texcoord[1];" \
  "TEX src, fragment.texcoord[1], texture[1], RECT;" \
  "DP4 color.a, color, fragment.color;" \
  "MUL result.color, src, color.a;"

#define EXPAND_MASK_DECL "TEMP mask_pos, mask;"
#define EXPAND_MASK_2D_IN_OP \
  "DP4 mask_pos.x, state.matrix.texture[0].row[0], fragment.texcoord[0];" \
  "DP4 mask_pos.y, state.matrix.texture[0].row[1], fragment.texcoord[0];" \
  "DP4 mask_pos.w, state.matrix.texture[0].row[3], fragment.texcoord[0];" \
  "TXP mask, mask_pos, texture[0], 2D;" \
  "DP4 mask.a, mask, fragment.color;" \
  "MUL result.color, color, mask.a;"

#define EXPAND_MASK_RECT_IN_OP \
  "DP4 mask_pos.x, state.matrix.texture[0].row[0], fragment.texcoord[0];" \
  "DP4 mask_pos.y, state.matrix.texture[0].row[1], fragment.texcoord[0];" \
  "DP4 mask_pos.w, state.matrix.texture[0].row[3], fragment.texcoord[0];" \
  "TXP mask, fragment.texcoord[0], texture[0], RECT;" \
  "DP4 mask.a, mask, fragment.color;" \
  "MUL result.color, color, mask.a;"

typedef struct _glitz_program_expand_t glitz_program_expand_t;

static const struct _glitz_program_expand_t {
  char *texture;
  char *declarations;
  char *in;
} _program_expand_map[GLITZ_TEXTURE_LAST][GLITZ_TEXTURE_LAST] = {
  {
    /* [GLITZ_TEXTURE_NONE][GLITZ_TEXTURE_NONE] */
    { EXPAND_NONE, EXPAND_NONE, EXPAND_NO_IN_OP },
    
    /* [GLITZ_TEXTURE_NONE][GLITZ_TEXTURE_2D] */
    { EXPAND_NONE, EXPAND_MASK_DECL, EXPAND_MASK_2D_IN_OP },
    
    /* [GLITZ_TEXTURE_NONE][GLITZ_TEXTURE_RECT] */
    { EXPAND_NONE, EXPAND_MASK_DECL, EXPAND_MASK_RECT_IN_OP }
  }, {
    
    /* [GLITZ_TEXTURE_2D][GLITZ_TEXTURE_NONE] */
    { EXPAND_2D, EXPAND_NONE, EXPAND_NO_IN_OP },
    
    /* [GLITZ_TEXTURE_2D][GLITZ_TEXTURE_2D] */
    { EXPAND_2D, EXPAND_MASK_DECL, EXPAND_MASK_2D_IN_OP },
    
    /* [GLITZ_TEXTURE_2D][GLITZ_TEXTURE_RECT] */
    { EXPAND_2D, EXPAND_MASK_DECL, EXPAND_MASK_RECT_IN_OP }
  }, {
    
    /* [GLITZ_TEXTURE_RECT][GLITZ_TEXTURE_NONE] */
    { EXPAND_RECT, EXPAND_NONE, EXPAND_NO_IN_OP },
    
    /* [GLITZ_TEXTURE_RECT][GLITZ_TEXTURE_2D] */
    { EXPAND_RECT, EXPAND_MASK_DECL, EXPAND_MASK_2D_IN_OP },
    
    /* [GLITZ_TEXTURE_RECT][GLITZ_TEXTURE_RECT] */
    { EXPAND_RECT, EXPAND_MASK_DECL, EXPAND_MASK_RECT_IN_OP }
  }
};

/*
 * Default vertex program
 *
 * Author: David Reveman <c99drn@cs.umu.se>
 */   
static const char *_glitz_vertex_program[] = {
  "!!ARBvp1.0",
  "OPTION ARB_position_invariant;",
  "MOV result.texcoord[%s], vertex.texcoord[%s];",
  "MOV result.texcoord[%s], vertex.texcoord[%s];",
  "MOV result.color, vertex.color;",
  "END", NULL
};

/*
  TODO: the following convolution filter program could easily be
  dynamically created for any convolution kernel size.
*/

/*
 * 9 samples convolution filter. projective transformations might not
 * produce correct results.
 *
 * program.local[0..n]: Each parameter holds an offset and a weight.
 *
 * Author: David Reveman <c99drn@cs.umu.se>
 */
static const char *_glitz_fragment_program_convolution[] = {
  "!!ARBfp1.0",
  "ATTRIB center = fragment.texcoord[%s];",
  "PARAM p0 = program.local[0];",
  "PARAM p1 = program.local[1];",
  "PARAM p2 = program.local[2];",
  "PARAM p3 = program.local[3];",
  "PARAM p4 = program.local[4];",
  "PARAM p5 = program.local[5];",
  "PARAM p6 = program.local[6];",
  "PARAM p7 = program.local[7];",
  "PARAM p8 = program.local[8];",
  "PARAM m0 = state.matrix.texture[%s].row[0];",
  "PARAM m1 = state.matrix.texture[%s].row[1];",
  "PARAM m3 = state.matrix.texture[%s].row[3];",
  "TEMP color, in, coord, position;",

  /* extra declerations */
  "%s",

  /* projective transform */
  "DP4 position.x, m0, center;",
  "DP4 position.y, m1, center;",
  "DP4 position.w, m3, center;",
  "MOV coord, position;",

  /* 1 */
  "ADD coord.x, position.x, p0.x;",
  "ADD coord.y, position.y, p0.y;",
  "TXP in, coord, texture[%s], %s;",
  "MUL color, in, p0.z;",

  /* 2 */
  "ADD coord.x, position.x, p1.x;",
  "ADD coord.y, position.y, p1.y;",
  "TXP in, coord, texture[%s], %s;",
  "MAD color, in, p1.z, color;",

  /* 3 */
  "ADD coord.x, position.x, p2.x;",
  "ADD coord.y, position.y, p2.y;",
  "TXP in, coord, texture[%s], %s;",
  "MAD color, in, p2.z, color;",

  /* 4 */
  "ADD coord.x, position.x, p3.x;",
  "ADD coord.y, position.y, p3.y;",
  "TXP in, coord, texture[%s], %s;",
  "MAD color, in, p3.z, color;",

  /* 5 */
  "ADD coord.x, position.x, p4.x;",
  "ADD coord.y, position.y, p4.y;",
  "TXP in, coord, texture[%s], %s;",
  "MAD color, in, p4.z, color;",

  /* 6 */
  "ADD coord.x, position.x, p5.x;",
  "ADD coord.y, position.y, p5.y;",
  "TXP in, coord, texture[%s], %s;",
  "MAD color, in, p5.z, color;",

  /* 7 */
  "ADD coord.x, position.x, p6.x;",
  "ADD coord.y, position.y, p6.y;",
  "TXP in, coord, texture[%s], %s;",
  "MAD color, in, p6.z, color;",

  /* 8 */
  "ADD coord.x, position.x, p7.x;",
  "ADD coord.y, position.y, p7.y;",
  "TXP in, coord, texture[%s], %s;",
  "MAD color, in, p7.z, color;",

  /* 9 */
  "ADD coord.x, position.x, p8.x;",
  "ADD coord.y, position.y, p8.y;",
  "TXP in, coord, texture[%s], %s;",
  "MAD color, in, p8.z, color;",

  /* IN operation */
  "%s",
  
  "END", NULL
};

/*
 * Linear gradient filter.
 *
 * program.local[0].x = start offset
 * program.local[0].y = 1 / length
 * program.local[0].z = sin (angle)
 * program.local[0].w = cos (angle)
 *
 * Incoming texture coordinate is transformed using the affine
 * transform stored in the texture matrix.
 *
 * Author: David Reveman <c99drn@cs.umu.se>
 */
static const char *_glitz_fragment_program_linear[] = {
  "!!ARBfp1.0",
  "PARAM gradient = program.local[0];",
  "PARAM adjust = program.local[1];",
  "PARAM affine0 = state.matrix.texture[%s].row[0];",
  "PARAM affine1 = state.matrix.texture[%s].row[1];",
  "ATTRIB pos = fragment.texcoord[%s];",
  "TEMP color, distance, position;",

  /* extra declerations */
  "%s",

  /* affine transform */
  "DP4 position.x, affine0, pos;",
  "DP4 position.y, affine1, pos;",

  /* calculate gradient offset */
  "MUL position.x, gradient.z, position.x;",
  "MAD position.x, gradient.w, position.y, position.x;",
  
  "SUB distance.x, position.x, gradient.x;",
  "MUL distance.x, distance.x, gradient.y;",

  /* temporary fix (until the new gradient shaders are implemented) */
  "MOV distance.y, 0.5;",
  "MUL distance.x, distance.x, adjust.x;",
  
  "TEX color, distance, texture[%s], %s;",
  
  /* IN operation */
  "%s",
  
  "END", NULL
};

/*
 * Radial gradient filter.
 *
 * param.local[0].x = center point X coordinate
 * param.local[0].y = center point Y coordinate
 * param.local[0].z = 1 / (radius1 - radius0)
 * param.local[0].w = radius0
 *
 * Incoming texture coordinate is transformed using the affine
 * transform stored in the texture matrix.
 *
 * Author: David Reveman <c99drn@cs.umu.se>
 */
static const char *_glitz_fragment_program_radial[] = {
  "!!ARBfp1.0",
  "PARAM gradient = program.local[0];",
  "PARAM adjust = program.local[1];",
  "PARAM affine0 = state.matrix.texture[%s].row[0];",
  "PARAM affine1 = state.matrix.texture[%s].row[1];",
  "ATTRIB pos = fragment.texcoord[%s];",
  "TEMP color, distance, position;",

  /* extra declerations */
  "%s",

  /* affine transform */
  "DP4 position.x, affine0, pos;",
  "DP4 position.y, affine1, pos;",
  
  /* calculate gradient offset */
  "SUB distance, position, gradient;",

  "DP3 distance.x, distance, distance;",
  "RSQ distance.w, distance.x;",
  
  "RCP distance.x, distance.w;",
  "MUL distance.x, distance.x, distance.x;",
  "MUL distance.x, distance.x, distance.w;",
  
  "SUB distance.x, distance.x, gradient.w;",
  "MUL distance.x, distance.x, gradient.z;",

  /* temporary fix (until the new gradient shaders are implemented) */
  "MOV distance.y, 0.5;",
  "MUL distance.x, distance.x, adjust.x;",
  
  "TEX color, distance, texture[%s], %s;",
  
  /* IN operation */
  "%s",
  
  "END", NULL
};

static glitz_gl_uint_t
_glitz_compile_arb_vertex_program (glitz_gl_proc_address_list_t *gl,
                                   char *program_string)
{
  glitz_gl_int_t error;
  glitz_gl_uint_t program;
    
  gl->gen_programs (1, &program);
  gl->bind_program (GLITZ_GL_VERTEX_PROGRAM, program);
  gl->program_string (GLITZ_GL_VERTEX_PROGRAM,
                      GLITZ_GL_PROGRAM_FORMAT_ASCII,
                      strlen (program_string),
                      program_string);
  
  gl->get_integer_v (GLITZ_GL_PROGRAM_ERROR_POSITION, &error);
  if (error != -1) {
    gl->delete_programs (1, &program);
    program = 0;
  }
  
  return program;
}

static glitz_gl_uint_t
_glitz_compile_arb_fragment_program (glitz_gl_proc_address_list_t *gl,
                                     char *program_string)
{
  glitz_gl_int_t error;
  glitz_gl_uint_t program;

  gl->gen_programs (1, &program);
  gl->bind_program (GLITZ_GL_FRAGMENT_PROGRAM, program);
  gl->program_string (GLITZ_GL_FRAGMENT_PROGRAM,
                      GLITZ_GL_PROGRAM_FORMAT_ASCII,
                      strlen (program_string),
                      program_string);
  
  gl->get_integer_v (GLITZ_GL_PROGRAM_ERROR_POSITION, &error);
  if (error != -1) {
    gl->delete_programs (1, &program);
    program = 0;
  }
  
  return program;
}

static void
_string_array_to_char_array (char *dst, const char *src[])
{
  int i, n;
  
  for (i = 0; src[i]; i++) {
    n = strlen (src[i]);
    memcpy (dst, src[i], n);
    dst += n;
  }
  *dst = '\0';
}

static glitz_gl_uint_t
_glitz_create_vertex_program (glitz_composite_op_t *op,
                              glitz_filter_t filter,
                              const glitz_program_expand_t *expand)
{
  char program[512], program_buffer[512];

  _string_array_to_char_array (program, _glitz_vertex_program);
    
  switch (op->type) {
  case GLITZ_COMBINE_TYPE_ARGBF:
  case GLITZ_COMBINE_TYPE_ARGBF_SOLID:
  case GLITZ_COMBINE_TYPE_ARGBF_SOLIDC:
    sprintf (program_buffer, program,
             "0", "0", "1", "1");
    break;
  case GLITZ_COMBINE_TYPE_ARGBF_ARGB:
  case GLITZ_COMBINE_TYPE_ARGBF_ARGBC:
    sprintf (program_buffer, program,
             "1", "1", "0", "0");
    break;
  default:
    return 0;
  }
  
  return _glitz_compile_arb_vertex_program (op->gl, program_buffer);
}

static glitz_gl_uint_t
_glitz_create_fragment_program (glitz_composite_op_t *op,
                                glitz_filter_t filter,
                                const glitz_program_expand_t *expand)
{
  char program[4096], program_buffer[4096];

  switch (filter) {
  case GLITZ_FILTER_CONVOLUTION:
      _string_array_to_char_array (program,
                                   _glitz_fragment_program_convolution);
      
      switch (op->type) {
      case GLITZ_COMBINE_TYPE_ARGBF:
      case GLITZ_COMBINE_TYPE_ARGBF_SOLID:
      case GLITZ_COMBINE_TYPE_ARGBF_SOLIDC:
        sprintf (program_buffer, program,
                 "0", "0", "0", "0",
                 expand->declarations,
                 "0", expand->texture,
                 "0", expand->texture,
                 "0", expand->texture,
                 "0", expand->texture,
                 "0", expand->texture,
                 "0", expand->texture,
                 "0", expand->texture,
                 "0", expand->texture,
                 "0", expand->texture,
                 expand->in);
        break;
      case GLITZ_COMBINE_TYPE_ARGBF_ARGB:
      case GLITZ_COMBINE_TYPE_ARGBF_ARGBC:
        sprintf (program_buffer, program,
                 "1", "1", "1", "1",
                 expand->declarations,
                 "1", expand->texture,
                 "1", expand->texture,
                 "1", expand->texture,
                 "1", expand->texture,
                 "1", expand->texture,
                 "1", expand->texture,
                 "1", expand->texture,
                 "1", expand->texture,
                 "1", expand->texture,
                 expand->in);
        break;
      default:
        return 0;
      }
      break;
  case GLITZ_FILTER_LINEAR_GRADIENT:
    _string_array_to_char_array (program, _glitz_fragment_program_linear);
    
    switch (op->type) {
    case GLITZ_COMBINE_TYPE_ARGBF:
    case GLITZ_COMBINE_TYPE_ARGBF_SOLID:
    case GLITZ_COMBINE_TYPE_ARGBF_SOLIDC:
      sprintf (program_buffer, program,
               "0", "0", "0",
               expand->declarations,
               "0", expand->texture,
               expand->in);
      break;
    case GLITZ_COMBINE_TYPE_ARGBF_ARGB:
    case GLITZ_COMBINE_TYPE_ARGBF_ARGBC:
      sprintf (program_buffer, program,
               "1", "1", "1",
               expand->declarations,
               "1", expand->texture,
               expand->in);
      break;
    default:
      return 0;
    }
    break;
  case GLITZ_FILTER_RADIAL_GRADIENT:
    _string_array_to_char_array (program, _glitz_fragment_program_radial);
    
    switch (op->type) {
    case GLITZ_COMBINE_TYPE_ARGBF:
    case GLITZ_COMBINE_TYPE_ARGBF_SOLID:
    case GLITZ_COMBINE_TYPE_ARGBF_SOLIDC:
      sprintf (program_buffer, program,
               "0", "0", "0",
               expand->declarations,
               "0", expand->texture,
               expand->in);
      break;
    case GLITZ_COMBINE_TYPE_ARGBF_ARGB:
    case GLITZ_COMBINE_TYPE_ARGBF_ARGBC:
      sprintf (program_buffer, program,
               "1", "1", "1",
               expand->declarations,
               "1", expand->texture,
               expand->in);
      break;
    default:
      return 0;
    }
    break;
  default:
    return 0;
  }
  
  return _glitz_compile_arb_fragment_program (op->gl, program_buffer);
}

static int
_texture_index (glitz_surface_t *surface)
{
  if (surface) {
    if (surface->texture.target == GLITZ_GL_TEXTURE_2D)
      return GLITZ_TEXTURE_2D;
    else
      return GLITZ_TEXTURE_RECT;
  } else
    return GLITZ_TEXTURE_NONE;
}
                      
void
glitz_program_map_init (glitz_program_map_t *map)
{
  memset (map, 0, sizeof (glitz_program_map_t));
}

void
glitz_program_map_fini (glitz_gl_proc_address_list_t *gl,
                        glitz_program_map_t *map)
{
  int i, j, x, y;
    
  for (i = 0; i < GLITZ_COMBINE_TYPES; i++)
    for (j = 0; j < GLITZ_FRAGMENT_FILTER_TYPES; j++)
      for (x = 0; x < GLITZ_TEXTURE_LAST; x++)
        for (y = 0; y < GLITZ_TEXTURE_LAST; y++) {
          if (map->info[i][j].vertex[x][y])
            gl->delete_programs (1, &map->info[i][j].vertex[x][y]);
          if (map->info[i][j].fragment[x][y])
            gl->delete_programs (1, &map->info[i][j].vertex[x][y]);
        }
}

glitz_gl_uint_t
glitz_get_vertex_program (glitz_composite_op_t *op,
                          glitz_filter_t filter)
{
  glitz_gl_uint_t *program;
  int index1 = _texture_index (op->src);
  int index2 = _texture_index (op->mask);

  /* smae vertex program for all filters */
  program = &op->dst->program_map->info[op->type][0].vertex[index1][index2];
  
  if (*program == 0) {
    if (glitz_surface_push_current (op->dst,
                                    GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
      *program =
        _glitz_create_vertex_program (op, filter,
                                      &_program_expand_map[index1][index2]);
    }
    glitz_surface_pop_current (op->dst);
  }

  return *program;
}

glitz_gl_uint_t
glitz_get_fragment_program (glitz_composite_op_t *op,
                            glitz_filter_t filter)
{
  glitz_gl_uint_t *program;
  int index1 = _texture_index (op->src);
  int index2 = _texture_index (op->mask);
  int ftype;

  switch (filter) {
  case GLITZ_FILTER_CONVOLUTION:
    ftype = GLITZ_FRAGMENT_FILTER_CONVOLUTION;
    break;
  case GLITZ_FILTER_LINEAR_GRADIENT:
    ftype = GLITZ_FRAGMENT_FILTER_LINEAR_GRADIENT;
    break;
  case GLITZ_FILTER_RADIAL_GRADIENT:
    ftype = GLITZ_FRAGMENT_FILTER_RADIAL_GRADIENT;
    break;
  case GLITZ_FILTER_BILINEAR:
  case GLITZ_FILTER_NEAREST:
  default:
    return 0;
  }
  
  program =
    &op->dst->program_map->info[op->type][ftype].fragment[index1][index2];
  
  if (*program == 0) {
    if (glitz_surface_push_current (op->dst,
                                    GLITZ_CN_SURFACE_DRAWABLE_CURRENT)) {
      *program =
        _glitz_create_fragment_program (op, filter,
                                        &_program_expand_map[index1][index2]);
    }
    glitz_surface_pop_current (op->dst);
  }

  return *program;
}
