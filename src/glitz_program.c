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

#define EXPAND_SRC_TEMP "TEMP src;"
#define EXPAND_SRC_2D_IN_OP \
  "TEX src, fragment.texcoord[1], texture[1], 2D;" \
  "DP4 color.a, color, fragment.color;" \
  "MUL result.color, src, color.a;"
#define EXPAND_SRC_RECT_IN_OP \
  "TEX src, fragment.texcoord[1], texture[1], RECT;" \
  "DP4 color.a, color, fragment.color;" \
  "MUL result.color, src, color.a;"

#define EXPAND_MASK_TEMP "TEMP mask;"
#define EXPAND_MASK_2D_IN_OP \
  "TEX mask, fragment.texcoord[0], texture[0], 2D;" \
  "DP4 mask.a, mask, fragment.color;" \
  "MUL result.color, color, mask.a;"
#define EXPAND_MASK_RECT_IN_OP \
  "TEX mask, fragment.texcoord[0], texture[0], RECT;" \
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
    { EXPAND_NONE, EXPAND_MASK_TEMP, EXPAND_MASK_2D_IN_OP },
    
    /* [GLITZ_TEXTURE_NONE][GLITZ_TEXTURE_RECT] */
    { EXPAND_NONE, EXPAND_MASK_TEMP, EXPAND_MASK_RECT_IN_OP }
  }, {
    
    /* [GLITZ_TEXTURE_2D][GLITZ_TEXTURE_NONE] */
    { EXPAND_2D, EXPAND_NONE, EXPAND_NO_IN_OP },
    
    /* [GLITZ_TEXTURE_2D][GLITZ_TEXTURE_2D] */
    { EXPAND_2D, EXPAND_MASK_TEMP, EXPAND_MASK_2D_IN_OP },
    
    /* [GLITZ_TEXTURE_2D][GLITZ_TEXTURE_RECT] */
    { EXPAND_2D, EXPAND_MASK_TEMP, EXPAND_MASK_RECT_IN_OP }
  }, {
    
    /* [GLITZ_TEXTURE_RECT][GLITZ_TEXTURE_NONE] */
    { EXPAND_RECT, EXPAND_NONE, EXPAND_NO_IN_OP },
    
    /* [GLITZ_TEXTURE_RECT][GLITZ_TEXTURE_2D] */
    { EXPAND_RECT, EXPAND_MASK_TEMP, EXPAND_MASK_2D_IN_OP },
    
    /* [GLITZ_TEXTURE_RECT][GLITZ_TEXTURE_RECT] */
    { EXPAND_RECT, EXPAND_MASK_TEMP, EXPAND_MASK_RECT_IN_OP }
  }
};

/*
 * Passes texture coordinates to convolution filter
 * fragment programs.
 *
 * program.local[0]: Vertical pixel offset in texture coordinates
 * program.local[1]: Horizontal pixel offset in texture coordinates
 *
 * Author: David Reveman <c99drn@cs.umu.se>
 */   
static const char *_glitz_vertex_program_convolution[] = {
  "!!ARBvp1.0",
  "OPTION ARB_position_invariant;",
  "ATTRIB coord = vertex.texcoord[%s];",
  "PARAM vertical_offset   = program.local[0];",
  "PARAM horizontal_offset = program.local[1];",
  "MOV result.texcoord[%s], coord;",
  "ADD result.texcoord[2], coord, vertical_offset;",
  "SUB result.texcoord[3], coord, vertical_offset;",
  "ADD result.texcoord[4], coord, horizontal_offset;",
  "SUB result.texcoord[5], coord, horizontal_offset;",
  "MOV result.texcoord[%s], vertex.texcoord[%s];",
  "END", NULL
};

/*
 * General 3x3 convolution filter.
 * Convolution kernel must be normalized.
 *
 * program.local[0]: Top convolution kernel row
 * program.local[1]: Middle convolution kernel row
 * program.local[2]: Bottom convolution kernel row
 *
 * Author: David Reveman <c99drn@cs.umu.se>
 */
static const char *_glitz_fragment_program_convolution[] = {
  "!!ARBfp1.0",
  "ATTRIB east = fragment.texcoord[2];",
  "ATTRIB west = fragment.texcoord[3];",
  "ATTRIB south = fragment.texcoord[4];",
  "ATTRIB north = fragment.texcoord[5];",
  "PARAM k0 = program.local[0];",
  "PARAM k1 = program.local[1];",
  "PARAM k2 = program.local[2];",
  "TEMP color, in, coord;",

  /* extra declerations */
  "%s",

  /* center */
  "TEX in, fragment.texcoord[%s], texture[%s], %s;",
  "MUL color, in, k1.y;",
  
  /* north west */
  "MOV coord.x, west.x;",
  "MOV coord.y, north.y;",
  "TEX in, coord, texture[%s], %s;",
  "MAD color, in, k0.x, color;",

  /* north */
  "TEX in, north, texture[%s], %s;",
  "MAD color, in, k0.y, color;",

  /* north east */
  "MOV coord.x, east.x;",
  "TEX in, coord, texture[%s], %s;",
  "MAD color, in, k0.z, color;",

  /* east */
  "TEX in, east, texture[%s], %s;",
  "MAD color, in, k1.x, color;",

  /* south east */
  "MOV coord.y, south.y;",
  "TEX in, coord, texture[%s], %s;",
  "MAD color, in, k2.z, color;",

  /* south */
  "TEX in, south, texture[%s], %s;",
  "MAD color, in, k2.y, color;",

  /* south west */
  "MOV coord.x, west.x;",
  "TEX in, coord, texture[%s], %s;",
  "MAD color, in, k2.x, color;",

  /* west */
  "TEX in, west, texture[%s], %s;",
  "MAD color, in, k1.x, color;",

  /* IN operation */
  "%s",
  
  "END", NULL
};

/*
 * Linear gradient using 1D texture as color range.
 * Color range in texture unit 2.
 *
 * program.local[0].x = start offset
 * program.local[0].y = 1 / length
 * program.local[0].z = sin (angle)
 * program.local[0].w = cos (angle)
 *
 * transform:
 * [ a | c | tx ]
 * [ b | d | ty ]
 * [ 0 | 0 |  1 ]
 *
 * program.local[1].x = a
 * program.local[1].y = b
 * program.local[1].z = c
 * program.local[1].w = d
 * program.local[2].x = tx
 * program.local[2].y = ty
 * program.local[2].z = height
 *
 * Author: David Reveman <c99drn@cs.umu.se>
 */
static const char *_glitz_fragment_program_linear[] = {
  "!!ARBfp1.0",
  "PARAM gradient = program.local[0];",
  "PARAM transform = program.local[1];",
  "PARAM translate = program.local[2];",
  "ATTRIB pos = fragment.texcoord[%s];",
  "TEMP color, distance, position;",

  /* extra declerations */
  "%s",

  /* flip Y position (this should be done in advance) */
  "SUB position.y, translate.z, pos.y;",

  /* transform X position */
  "MUL position.x, transform.x, pos.x;",
  "MAD position.x, transform.z, position.y, position.x;",
  "ADD position.x, position.x, translate.x;",

  /* transform Y position */
  "MUL position.y, transform.w, position.y;",
  "MAD position.y, transform.y, pos.x, position.y;",
  "ADD position.y, position.y, translate.y;",

  /* calculate gradient offset */
  "MUL position.x, gradient.z, position.x;",
  "MAD position.x, gradient.w, position.y, position.x;",
  
  "SUB distance.x, position.x, gradient.x;",
  "MUL distance.x, distance.x, gradient.y;",
  
  "TEX color, distance, texture[2], 1D;",
  
  /* IN operation */
  "%s",
  
  "END", NULL
};

/*
 * Radial gradient using 1D texture as color range.
 * Color range in texture unit 2.
 *
 * param.local[0].x = center point X coordinate
 * param.local[0].y = center point Y coordinate
 * param.local[0].z = 1 / (radius1 - radius0)
 * param.local[0].w = radius0
 *
 * transform:
 * [ a | c | tx ]
 * [ b | d | ty ]
 * [ 0 | 0 |  1 ]
 *
 * program.local[1].x = a
 * program.local[1].y = b
 * program.local[1].z = c
 * program.local[1].w = d
 * program.local[2].x = tx
 * program.local[2].y = ty
 * program.local[2].z = height
 *
 * Author: David Reveman <c99drn@cs.umu.se>
 */
static const char *_glitz_fragment_program_radial[] = {
  "!!ARBfp1.0",
  "PARAM gradient = program.local[0];",
  "PARAM transform = program.local[1];",
  "PARAM translate = program.local[2];",
  "ATTRIB pos = fragment.texcoord[%s];",
  "TEMP color, distance, position;",

  /* extra declerations */
  "%s",

  /* flip Y position (this should be done in advance) */
  "SUB position.y, translate.z, pos.y;",
  
  /* transform X position */
  "MUL position.x, transform.x, pos.x;",
  "MAD position.x, transform.z, position.y, position.x;",
  "ADD position.x, position.x, translate.x;",

  /* transform Y position */
  "MUL position.y, transform.w, position.y;",
  "MAD position.y, transform.y, pos.x, position.y;",
  "ADD position.y, position.y, translate.y;",
  
  /* calculate gradient offset */
  "SUB distance, position, gradient;",

  "DP3 distance.x, distance, distance;",
  "RSQ distance.w, distance.x;",
  
  "RCP distance.x, distance.w;",
  "MUL distance.x, distance.x, distance.x;",
  "MUL distance.x, distance.x, distance.w;",
  
  "SUB distance.x, distance.x, gradient.w;",
  "MUL distance.x, distance.x, gradient.z;",
  
  "TEX color, distance, texture[2], 1D;",
  
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
}

static glitz_gl_uint_t
_glitz_create_vertex_program (glitz_render_op_t *op,
                              const glitz_program_expand_t *expand)
{
  char program[512], program_buffer[512];
  
  switch (op->type) {
  case GLITZ_RENDER_TYPE_ARGBF:
  case GLITZ_RENDER_TYPE_ARGBF_SOLID:
    _string_array_to_char_array (program, _glitz_vertex_program_convolution);
    sprintf (program_buffer, program,
             "0", "0", "1", "1");
    break;
  case GLITZ_RENDER_TYPE_ARGBF_ARGB:
  case GLITZ_RENDER_TYPE_ARGBF_ARGBC:
    _string_array_to_char_array (program, _glitz_vertex_program_convolution);
    sprintf (program_buffer, program,
             "1", "1", "0", "0");
    break;
  default:
    return 0;
  }
  
  return _glitz_compile_arb_vertex_program (op->gl, program_buffer);
}

static glitz_gl_uint_t
_glitz_create_fragment_program (glitz_render_op_t *op,
                                const glitz_program_expand_t *expand)
{
  char program[2048], program_buffer[2048];
  
  switch (op->type) {
  case GLITZ_RENDER_TYPE_ARGBF:
  case GLITZ_RENDER_TYPE_ARGBF_SOLID:
    _string_array_to_char_array (program, _glitz_fragment_program_convolution);
    sprintf (program_buffer, program,
             expand->declarations,
             "0", "0", expand->texture,
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
  case GLITZ_RENDER_TYPE_ARGBF_ARGB:
  case GLITZ_RENDER_TYPE_ARGBF_ARGBC:
    _string_array_to_char_array (program, _glitz_fragment_program_convolution);
    sprintf (program_buffer, program,
             expand->declarations,
             "1", "1", expand->texture,
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
  case GLITZ_RENDER_TYPE_LGRAD:
  case GLITZ_RENDER_TYPE_LGRAD_SOLID:
    _string_array_to_char_array (program, _glitz_fragment_program_linear);
    sprintf (program_buffer, program,
             "0",
             expand->declarations,
             expand->in);
    break;
  case GLITZ_RENDER_TYPE_LGRAD_ARGB:
  case GLITZ_RENDER_TYPE_LGRAD_ARGBC:
    _string_array_to_char_array (program, _glitz_fragment_program_linear);
    sprintf (program_buffer, program,
             "1",
             expand->declarations,
             expand->in);
    break;
  case GLITZ_RENDER_TYPE_RGRAD:
  case GLITZ_RENDER_TYPE_RGRAD_SOLID:
    _string_array_to_char_array (program, _glitz_fragment_program_radial);
    sprintf (program_buffer, program,
             "0",
             expand->declarations,
             expand->in);
    break;
  case GLITZ_RENDER_TYPE_RGRAD_ARGB:
  case GLITZ_RENDER_TYPE_RGRAD_ARGBC:
    _string_array_to_char_array (program, _glitz_fragment_program_radial);
    sprintf (program_buffer, program,
             "1",
             expand->declarations,
             expand->in);
    break;
  default:
    return 0;
  }
  
  return _glitz_compile_arb_fragment_program (op->gl, program_buffer);
}

static int
_texture_index (glitz_texture_t *texture)
{
  if (texture) {
    if (texture->target == GLITZ_GL_TEXTURE_2D)
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
  int i, x, y;
    
  for (i = 0; i < GLITZ_RENDER_TYPES; i++) {
    for (x = 0; x < GLITZ_TEXTURE_LAST; x++)
      for (y = 0; y < GLITZ_TEXTURE_LAST; y++) {
        if (map->info[i].vertex[x][y])
          gl->delete_programs (1, &map->info[i].vertex[x][y]);
        if (map->info[i].fragment[x][y])
          gl->delete_programs (1, &map->info[i].vertex[x][y]);
      }
  }
}

glitz_gl_uint_t
glitz_get_vertex_program (glitz_render_op_t *op)
{
  glitz_gl_uint_t *program;
  int index1 = _texture_index (op->src_texture);
  int index2 = _texture_index (op->mask_texture);
    
  program = &op->dst->program_map->info[op->type].vertex[index1][index2];
  
  if (*program == 0)
    *program =
      _glitz_create_vertex_program (op, &_program_expand_map[index1][index2]);

  return *program;
}

glitz_gl_uint_t
glitz_get_fragment_program (glitz_render_op_t *op)
{
  glitz_gl_uint_t *program;
  int index1 = _texture_index (op->src_texture);
  int index2 = _texture_index (op->mask_texture);
    
  program = &op->dst->program_map->info[op->type].fragment[index1][index2];
  
  if (*program == 0)
    *program =
      _glitz_create_fragment_program (op,
                                      &_program_expand_map[index1][index2]);

  return *program;
}
