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

#include <stdio.h>

#define EXPAND_NONE ""
#define EXPAND_2D_TEX "2D"
#define EXPAND_RECT_TEX "RECT"
#define EXPAND_NO_PD_OP \
  "MOV result.color, color;\n"

#define EXPAND_SRC_TEMP "TEMP src;\n"
#define EXPAND_SRC_2D_PD_OP \
  "TEX src, fragment.texcoord[0], texture[0], 2D;\n" \
  "MUL result.color, src, color.a;\n"
#define EXPAND_SRC_RECT_PD_OP \
  "TEX src, fragment.texcoord[0], texture[0], RECT;\n" \
  "MUL result.color, src, color.a;\n"

#define EXPAND_MASK_TEMP "TEMP mask;\n"
#define EXPAND_MASK_2D_PD_OP \
  "TEX mask, fragment.texcoord[1], texture[1], 2D;\n" \
  "MUL result.color, color, mask.a;\n"
#define EXPAND_MASK_RECT_PD_OP \
  "TEX mask, fragment.texcoord[1], texture[1], RECT;\n" \
  "MUL result.color, color, mask.a;\n"

typedef struct _glitz_program_expand_t glitz_program_expand_t;

static const struct _glitz_program_expand_t {
  int index;
  char *tex;
  char *temporary;
  char *operation;
} _program_expand_map[] = {

  /* src is operation surface */

  /* GLITZ_PROGRAM_2DSRC_2DMASK_OFFSET */
  { 0, EXPAND_2D_TEX, EXPAND_MASK_TEMP, EXPAND_MASK_2D_PD_OP },

  /* GLITZ_PROGRAM_RECTSRC_2DMASK_OFFSET */
  { 0, EXPAND_RECT_TEX, EXPAND_MASK_TEMP, EXPAND_MASK_2D_PD_OP },

  /* GLITZ_PROGRAM_2DSRC_RECTMASK_OFFSET */
  { 0, EXPAND_2D_TEX, EXPAND_MASK_TEMP, EXPAND_MASK_RECT_PD_OP },

  /* GLITZ_PROGRAM_RECTSRC_RECTMASK_OFFSET */
  { 0, EXPAND_RECT_TEX, EXPAND_MASK_TEMP, EXPAND_MASK_RECT_PD_OP },

  /* GLITZ_PROGRAM_NOSRC_2DMASK_OFFSET */
  { 0, EXPAND_NONE, EXPAND_MASK_TEMP, EXPAND_MASK_2D_PD_OP },

  /* GLITZ_PROGRAM_NOSRC_RECTMASK_OFFSET */
  { 0, EXPAND_NONE, EXPAND_MASK_TEMP, EXPAND_MASK_RECT_PD_OP },

  /* GLITZ_PROGRAM_2DSRC_NOMASK_OFFSET */
  { 0, EXPAND_2D_TEX, EXPAND_NONE, EXPAND_NO_PD_OP },

  /* GLITZ_PROGRAM_RECTSRC_NOMASK_OFFSET */
  { 0, EXPAND_RECT_TEX, EXPAND_NONE, EXPAND_NO_PD_OP },

  /* GLITZ_PROGRAM_NOSRC_NOMASK_OFFSET */
  { 0, EXPAND_NONE, EXPAND_NONE, EXPAND_NO_PD_OP },

  
  /* mask is operation surface */

  /* GLITZ_PROGRAM_2DSRC_2DMASK_OFFSET */
  { 1, EXPAND_2D_TEX, EXPAND_SRC_TEMP, EXPAND_SRC_2D_PD_OP },

  /* GLITZ_PROGRAM_RECTSRC_2DMASK_OFFSET */
  { 1, EXPAND_2D_TEX, EXPAND_SRC_TEMP, EXPAND_SRC_RECT_PD_OP },

  /* GLITZ_PROGRAM_2DSRC_RECTMASK_OFFSET */
  { 1, EXPAND_RECT_TEX, EXPAND_SRC_TEMP, EXPAND_SRC_2D_PD_OP },

  /* GLITZ_PROGRAM_RECTSRC_RECTMASK_OFFSET */
  { 1, EXPAND_RECT_TEX, EXPAND_SRC_TEMP, EXPAND_SRC_RECT_PD_OP },

  /* GLITZ_PROGRAM_NOSRC_2DMASK_OFFSET */
  { 1, EXPAND_2D_TEX, EXPAND_NONE, EXPAND_NO_PD_OP },

  /* GLITZ_PROGRAM_NOSRC_RECTMASK_OFFSET */
  { 1, EXPAND_RECT_TEX, EXPAND_NONE, EXPAND_NO_PD_OP },

  /* GLITZ_PROGRAM_2DSRC_NOMASK_OFFSET */
  { 1, EXPAND_NONE, EXPAND_SRC_TEMP, EXPAND_SRC_2D_PD_OP },

  /* GLITZ_PROGRAM_RECTSRC_NOMASK_OFFSET */
  { 1, EXPAND_NONE, EXPAND_SRC_TEMP, EXPAND_SRC_RECT_PD_OP },

  /* GLITZ_PROGRAM_NOSRC_NOMASK_OFFSET */
  { 1, EXPAND_NONE, EXPAND_NONE, EXPAND_NO_PD_OP }
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
static char *_glitz_vertex_program_convolution =
"!!ARBvp1.0\n"
"OPTION ARB_position_invariant;\n"
"ATTRIB coord = vertex.texcoord[%d];\n"
"PARAM vertical_offset   = program.local[0];\n"
"PARAM horizontal_offset = program.local[1];\n"
"MOV result.texcoord[%d], coord;\n"
"ADD result.texcoord[2], coord, vertical_offset;\n"
"SUB result.texcoord[3], coord, vertical_offset;\n"
"ADD result.texcoord[4], coord, horizontal_offset;\n"
"SUB result.texcoord[5], coord, horizontal_offset;\n"
"MOV result.texcoord[%d], vertex.texcoord[%d];\n"
"END";

/*
 * Porter-Duff compositing (SRC in MASK).
 * Texture unit 0 is SRC.
 * Texture unit 1 is MASK.
 *
 * Author: David Reveman <c99drn@cs.umu.se>
 */
static char *_glitz_fragment_program_simple =
"!!ARBfp1.0\n"
"TEMP color;\n"

/* temporary */
"%s"

/* src texture */
"TEX color, fragment.texcoord[0], texture[0], %s;\n"

/* pd operation */
"%s"

"END";

typedef enum {
  GLITZ_CONVOLUTION_3X3_GENERAL = 1,
  GLITZ_CONVOLUTION_3X3_SIMPLE = 2
} glitz_convolution_type_t;

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
static char *_glitz_fragment_program_convolution =
"!!ARBfp1.0\n"
"ATTRIB east = fragment.texcoord[2];\n"
"ATTRIB west = fragment.texcoord[3];\n"
"ATTRIB south = fragment.texcoord[4];\n"
"ATTRIB north = fragment.texcoord[5];\n"
"PARAM k0 = program.local[0];\n"
"PARAM k1 = program.local[1];\n"
"PARAM k2 = program.local[2];\n"
"TEMP color, in, coord;\n"

/* temporary */
"%s"

/* center */
"TEX in, fragment.texcoord[%d], texture[%d], %s;\n"
"MUL color, in, k1.y;\n"

/* north west */
"MOV coord.x, west.x;\n"
"MOV coord.y, north.y;\n"
"TEX in, coord, texture[%d], %s;\n"
"MAD color, in, k0.x, color;\n"

/* north */
"TEX in, north, texture[%d], %s;\n"
"MAD color, in, k0.y, color;\n"

/* north east */
"MOV coord.x, east.x;\n"
"TEX in, coord, texture[%d], %s;\n"
"MAD color, in, k0.z, color;\n"

/* east */
"TEX in, east, texture[%d], %s;\n"
"MAD color, in, k1.x, color;\n"

/* south east */
"MOV coord.y, south.y;\n"
"TEX in, coord, texture[%d], %s;\n"
"MAD color, in, k2.z, color;\n"

/* south */
"TEX in, south, texture[%d], %s;\n"
"MAD color, in, k2.y, color;\n"

/* south west */
"MOV coord.x, west.x;\n"
"TEX in, coord, texture[%d], %s;\n"
"MAD color, in, k2.x, color;\n"

/* west */
"TEX in, west, texture[%d], %s;\n"
"MAD color, in, k1.x, color;\n"

/* pd operation */
"%s"

"END";

/*
 * Simple 3x3 convolution filter (corners are zero).
 * Convolution kernel must be normalized.
 *
 * program.local[0]: Top convolution kernel row
 * program.local[1]: Middle convolution kernel row
 * program.local[2]: Bottom convolution kernel row
 *
 * Author: David Reveman <c99drn@cs.umu.se>
 */
static char *_glitz_fragment_program_convolution_simple =
"!!ARBfp1.0\n"
"ATTRIB east = fragment.texcoord[2];\n"
"ATTRIB west = fragment.texcoord[3];\n"
"ATTRIB south = fragment.texcoord[4];\n"
"ATTRIB north = fragment.texcoord[5];\n"
"PARAM k0 = program.local[0];\n"
"PARAM k1 = program.local[1];\n"
"PARAM k2 = program.local[2];\n"
"TEMP color, in, coord;\n"

/* temporary */
"%s"

/* center */
"TEX in, fragment.texcoord[%d], texture[%d], %s;\n"
"MUL color, in, k1.y;\n"

/* north */
"TEX in, north, texture[%d], %s;\n"
"MAD color, in, k0.y, color;\n"

/* east */
"TEX in, east, texture[%d], %s;\n"
"MAD color, in, k1.x, color;\n"

/* south */
"TEX in, south, texture[%d], %s;\n"
"MAD color, in, k2.y, color;\n"

/* west */
"TEX in, west, texture[%d], %s;\n"
"MAD color, in, k1.x, color;\n"

/* pd operation */
"%s"

"END";

char *_glitz_fragment_program_programmatic[] = {
  /*
   * Solid.
   *
   * fragment.color: color
   *
   * Author: David Reveman <c99drn@cs.umu.se>
   */
  "!!ARBfp1.0\n"
  "ATTRIB color = fragment.color;\n"

  /* temporary */
  "%s"

  /* pd operation */
  "%s"

  "END",
  
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

  "!!ARBfp1.0\n"
  "PARAM gradient = program.local[0];\n"
  "PARAM transform = program.local[1];\n"
  "PARAM translate = program.local[2];\n"
  "ATTRIB pos = fragment.texcoord[%d];\n"
  "TEMP color, distance, position;\n"

  /* temporary */
  "%s"

  /* flip Y position */
  "SUB position.y, translate.z, pos.y;\n"

  /* transform X position */
  "MUL position.x, transform.x, pos.x;\n"
  "MAD position.x, transform.z, position.y, position.x;\n"
  "ADD position.x, position.x, translate.x;\n"

  /* transform Y position */
  "MUL position.y, transform.w, position.y;\n"
  "MAD position.y, transform.y, pos.x, position.y;\n"
  "ADD position.y, position.y, translate.y;\n"

  /* calculate gradient offset */
  "MUL position.x, gradient.z, position.x;\n"
  "MAD position.x, gradient.w, position.y, position.x;\n"
  
  "SUB distance.x, position.x, gradient.x;\n"
  "MUL distance.x, distance.x, gradient.y;\n"
  
  "TEX color, distance, texture[2], 1D;\n"

  "MUL color, color, fragment.color.a;\n"
  
  /* pd operation */
  "%s"
  
  "END",

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

  "!!ARBfp1.0\n"
  "PARAM gradient = program.local[0];\n"
  "PARAM transform = program.local[1];\n"
  "PARAM translate = program.local[2];\n"
  "ATTRIB pos = fragment.texcoord[%d];\n"
  "TEMP color, distance, position;\n"

  /* temporary */
  "%s"

  /* flip Y position */
  "SUB position.y, translate.z, pos.y;\n"

  /* transform X position */
  "MUL position.x, transform.x, pos.x;\n"
  "MAD position.x, transform.z, position.y, position.x;\n"
  "ADD position.x, position.x, translate.x;\n"

  /* transform Y position */
  "MUL position.y, transform.w, position.y;\n"
  "MAD position.y, transform.y, pos.x, position.y;\n"
  "ADD position.y, position.y, translate.y;\n"

  /* calculate gradient offset */
  "SUB distance, position, gradient;\n"

  "DP3 distance.x, distance, distance;\n"
  "RSQ distance.w, distance.x;\n"
  
  "RCP distance.x, distance.w;\n"
  "MUL distance.x, distance.x, distance.x;\n"
  "MUL distance.x, distance.x, distance.w;\n"

  "SUB distance.x, distance.x, gradient.w;\n"
  "MUL distance.x, distance.x, gradient.z;\n"
  
  "TEX color, distance, texture[2], 1D;\n"

  "MUL color, color, fragment.color;\n"

  /* pd operation */
  "%s"
  
  "END"
};

static glitz_gl_uint_t
glitz_program_compile_vertex_arb (glitz_gl_proc_address_list_t *gl,
                                  char *program_string)
{
  glitz_gl_int_t error;
  glitz_gl_uint_t program;
    
  gl->gen_programs_arb (1, &program);
  gl->bind_program_arb (GLITZ_GL_VERTEX_PROGRAM_ARB, program);
  gl->program_string_arb (GLITZ_GL_VERTEX_PROGRAM_ARB,
                          GLITZ_GL_PROGRAM_FORMAT_ASCII_ARB,
                          strlen (program_string),
                          program_string);
  
  gl->get_integer_v (GLITZ_GL_PROGRAM_ERROR_POSITION_ARB, &error);
  if (error != -1) {
    gl->delete_programs_arb (1, &program);
    program = 0;
  }
  
  return program;
}

static glitz_gl_uint_t
glitz_program_compile_fragment_arb (glitz_gl_proc_address_list_t *gl,
                                    char *program_string)
{
  glitz_gl_int_t error;
  glitz_gl_uint_t program;
    
  gl->gen_programs_arb (1, &program);
  gl->bind_program_arb (GLITZ_GL_FRAGMENT_PROGRAM_ARB, program);
  gl->program_string_arb (GLITZ_GL_FRAGMENT_PROGRAM_ARB,
                          GLITZ_GL_PROGRAM_FORMAT_ASCII_ARB,
                          strlen (program_string),
                          program_string);
  
  gl->get_integer_v (GLITZ_GL_PROGRAM_ERROR_POSITION_ARB, &error);
  if (error != -1) {
    gl->delete_programs_arb (1, &program);
    program = 0;
  }
  
  return program;
}

static int
_glitz_program_offset (glitz_texture_t *src_texture,
                       glitz_texture_t *mask_texture)
{
  int offset;

  if (src_texture) {
    if (src_texture->target == GLITZ_GL_TEXTURE_2D) {
      offset = GLITZ_PROGRAM_2DSRC_NOMASK_OFFSET;
      
      if (mask_texture)
        offset = (mask_texture->target == GLITZ_GL_TEXTURE_2D)?
          GLITZ_PROGRAM_2DSRC_2DMASK_OFFSET:
      GLITZ_PROGRAM_2DSRC_RECTMASK_OFFSET;
    } else {
      offset = GLITZ_PROGRAM_RECTSRC_NOMASK_OFFSET;
      
      if (mask_texture)
        offset = (mask_texture->target == GLITZ_GL_TEXTURE_2D)?
          GLITZ_PROGRAM_RECTSRC_2DMASK_OFFSET:
      GLITZ_PROGRAM_RECTSRC_RECTMASK_OFFSET;
    }
  } else {
    offset = GLITZ_PROGRAM_NOSRC_NOMASK_OFFSET;
    
    if (mask_texture)
      offset = (mask_texture->target == GLITZ_GL_TEXTURE_2D)?
        GLITZ_PROGRAM_NOSRC_2DMASK_OFFSET: GLITZ_PROGRAM_NOSRC_RECTMASK_OFFSET;
  }
  
  return offset;
}

static glitz_gl_uint_t
glitz_program_compile_simple (glitz_gl_proc_address_list_t *gl,
                              int offset)
{
  char program_buffer[256];
  const glitz_program_expand_t *expand = &_program_expand_map[offset];
  
  sprintf (program_buffer,
           _glitz_fragment_program_simple,
           expand->temporary,
           expand->tex,
           expand->operation);

  return glitz_program_compile_fragment_arb (gl, program_buffer);
}

static glitz_gl_uint_t
glitz_program_compile_vertex_convolution (glitz_gl_proc_address_list_t *gl,
                                          int offset)
{
  char program_buffer[512];
  int conv_index = (offset)? 1: 0;
  int other_index = (offset)? 0: 1;
  
  sprintf (program_buffer,
           _glitz_vertex_program_convolution,
           conv_index, conv_index,
           other_index, other_index);

  return glitz_program_compile_vertex_arb (gl, program_buffer);
}

static glitz_gl_uint_t
glitz_program_compile_convolution (glitz_gl_proc_address_list_t *gl,
                                   int offset,
                                   int solid_offset,
                                   glitz_convolution_type_t type)
{
  static char *solid_op_table[] = {
    "MUL result.color, color, fragment.color.a;\n",
    "MUL result.color, fragment.color, color.a;\n",
  };
  char program_buffer[1280];
  const glitz_program_expand_t *expand = &_program_expand_map[offset];
  char *temporary, *operation;

  if (solid_offset) {
    temporary = "";
    operation = solid_op_table[solid_offset - 1];
  } else {
    temporary = expand->temporary;
    operation = expand->operation;
  }

  switch (type) {
  case GLITZ_CONVOLUTION_3X3_GENERAL:
    sprintf (program_buffer,
             _glitz_fragment_program_convolution,
             temporary,
             expand->index, expand->index, expand->tex,
             expand->index, expand->tex,
             expand->index, expand->tex,
             expand->index, expand->tex,
             expand->index, expand->tex,
             expand->index, expand->tex,
             expand->index, expand->tex,
             expand->index, expand->tex,
             expand->index, expand->tex,
             operation);
    break;
  case GLITZ_CONVOLUTION_3X3_SIMPLE:
    sprintf (program_buffer,
             _glitz_fragment_program_convolution_simple,
             temporary,
             expand->index, expand->index, expand->tex,
             expand->index, expand->tex,
             expand->index, expand->tex,
             expand->index, expand->tex,
             expand->index, expand->tex,
             operation);
    break;
  }

  return glitz_program_compile_fragment_arb (gl, program_buffer);
}

static glitz_gl_uint_t
glitz_program_compile_programmatic (glitz_programmatic_surface_type_t type,
                                    glitz_gl_proc_address_list_t *gl,
                                    int offset)
{
  char program_buffer[1024];
  const glitz_program_expand_t *expand = &_program_expand_map[offset];

  switch (type) {
  case GLITZ_PROGRAMMATIC_SURFACE_SOLID_TYPE:
    sprintf (program_buffer,
             _glitz_fragment_program_programmatic[type],
             expand->temporary,
             expand->operation);
    break;
  case GLITZ_PROGRAMMATIC_SURFACE_LINEAR_TYPE:
  case GLITZ_PROGRAMMATIC_SURFACE_RADIAL_TYPE:
    sprintf (program_buffer,
             _glitz_fragment_program_programmatic[type],
             expand->index,
             expand->temporary,
             expand->operation);
    break;
  }

  return glitz_program_compile_fragment_arb (gl, program_buffer);
}

void
glitz_program_enable_argb_argb (glitz_gl_proc_address_list_t *gl,
                                glitz_programs_t *programs,
                                glitz_texture_t *src_texture,
                                glitz_texture_t *mask_texture)
{
  int offset;

  offset = _glitz_program_offset (src_texture, mask_texture);
  
  if (!programs->fragment_simple[offset])
    programs->fragment_simple[offset] =
      glitz_program_compile_simple (gl, offset);
  
  if (programs->fragment_simple[offset]) {
    gl->enable (GLITZ_GL_FRAGMENT_PROGRAM_ARB);
    gl->bind_program_arb (GLITZ_GL_FRAGMENT_PROGRAM_ARB,
                          programs->fragment_simple[offset]);
  }
}

void
glitz_program_enable_convolution (glitz_gl_proc_address_list_t *gl,
                                  glitz_programs_t *programs,
                                  glitz_surface_t *src,
                                  glitz_surface_t *mask,
                                  glitz_texture_t *src_texture,
                                  glitz_texture_t *mask_texture,
                                  int offset,
                                  int solid_offset,
                                  unsigned short opacity)
{
  glitz_texture_t *texture;
  glitz_surface_t *surface;
  int fragment_offset, vertex_offset = (offset)? 1: 0;
  glitz_convolution_type_t type;

  if (offset) {
    texture = mask_texture;
    surface = mask;
  } else {
    texture = src_texture;
    surface = src;
  }

  offset += _glitz_program_offset (src_texture, mask_texture);

  if (surface->convolution->m[0][0] == 0.0 &&
      surface->convolution->m[0][2] == 0.0 &&
      surface->convolution->m[2][2] == 0.0 &&
      surface->convolution->m[2][0] == 0.0)
    type = GLITZ_CONVOLUTION_3X3_SIMPLE;
  else
    type = GLITZ_CONVOLUTION_3X3_GENERAL;

  fragment_offset = GLITZ_FRAGMENT_PROGRAM_TYPES * solid_offset * type +
    offset;

  if (!programs->vertex_convolution[vertex_offset])
    programs->vertex_convolution[vertex_offset] =
      glitz_program_compile_vertex_convolution (gl, vertex_offset);
  
  if (!programs->fragment_convolution[fragment_offset])
    programs->fragment_convolution[fragment_offset] =
      glitz_program_compile_convolution (gl, offset, solid_offset, type);
  
  if (programs->fragment_convolution[fragment_offset] &&
      programs->vertex_convolution[vertex_offset]) {

    gl->enable (GLITZ_GL_VERTEX_PROGRAM_ARB);
    gl->bind_program_arb (GLITZ_GL_VERTEX_PROGRAM_ARB,
                          programs->vertex_convolution[vertex_offset]);
    gl->program_local_param_4d_arb (GLITZ_GL_VERTEX_PROGRAM_ARB, 0,
                                    texture->texcoord_width /
                                    (double) texture->width,
                                    0.000, 0.0, 0.0);
    gl->program_local_param_4d_arb (GLITZ_GL_VERTEX_PROGRAM_ARB, 1,
                                    0.000,
                                    texture->texcoord_height /
                                    (double) texture->height,
                                    0.0, 0.0);
    
    gl->enable (GLITZ_GL_FRAGMENT_PROGRAM_ARB);
    gl->bind_program_arb (GLITZ_GL_FRAGMENT_PROGRAM_ARB,
                          programs->fragment_convolution[fragment_offset]);
    gl->program_local_param_4d_arb (GLITZ_GL_FRAGMENT_PROGRAM_ARB, 0,
                                    surface->convolution->m[0][0],
                                    surface->convolution->m[0][1],
                                    surface->convolution->m[0][2],
                                    0.0);
    gl->program_local_param_4d_arb (GLITZ_GL_FRAGMENT_PROGRAM_ARB, 1,
                                    surface->convolution->m[1][0],
                                    surface->convolution->m[1][1],
                                    surface->convolution->m[1][2],
                                    0.0);
    gl->program_local_param_4d_arb (GLITZ_GL_FRAGMENT_PROGRAM_ARB, 2,
                                    surface->convolution->m[2][0],
                                    surface->convolution->m[2][1],
                                    surface->convolution->m[2][2],
                                    0.0);

    if (solid_offset && mask) {
      glitz_color_t *color;
      
      if (solid_offset == 1)
        color = &((glitz_programmatic_surface_t *) mask)->u.solid.color;
      else
        color = &((glitz_programmatic_surface_t *) src)->u.solid.color;

      gl->color_4us (color->red, color->green, color->blue, color->alpha);
    } else
      gl->color_4us (opacity, opacity, opacity, opacity);
  }
}

void
glitz_program_enable_programmatic (glitz_surface_t *dst,
                                   glitz_programmatic_surface_t *surface,
                                   glitz_texture_t *src_texture,
                                   glitz_texture_t *mask_texture,
                                   int offset,
                                   unsigned short opacity)
{
  int type_offset, add_offset;
  glitz_programs_t *programs = dst->programs;

  if (offset)
    add_offset = _glitz_program_offset (src_texture, NULL);
  else
    add_offset = _glitz_program_offset (NULL, mask_texture);

  offset += add_offset;

  type_offset = offset + GLITZ_FRAGMENT_PROGRAM_TYPES * surface->type;

  if (dst->feature_mask & GLITZ_FEATURE_ARB_FRAGMENT_PROGRAM_MASK) {
    if (!programs->fragment_programmatic[type_offset])
      programs->fragment_programmatic[type_offset] =
        glitz_program_compile_programmatic (surface->type, dst->gl, offset);
    
    if (programs->fragment_programmatic[type_offset]) {
      dst->gl->enable (GLITZ_GL_FRAGMENT_PROGRAM_ARB);
      dst->gl->bind_program_arb (GLITZ_GL_FRAGMENT_PROGRAM_ARB,
                                 programs->fragment_programmatic[type_offset]);
    
      glitz_programmatic_surface_bind (dst->gl, surface, dst->feature_mask,
                                       0xffff);

      dst->gl->color_4us (opacity, opacity, opacity, opacity);
    }
  }
}

void
glitz_programs_fini (glitz_gl_proc_address_list_t *gl,
                     glitz_programs_t *programs)
{
  int i;
  
  for (i = 0; i < GLITZ_VERTEX_PROGRAM_TYPES; i++) {
    if (programs->vertex_convolution[i])
      gl->delete_programs_arb (1, &programs->vertex_convolution[i]);
  }

  for (i = 0; i < GLITZ_FRAGMENT_PROGRAM_TYPES; i++) {
    if (programs->fragment_simple[i])
      gl->delete_programs_arb (1, &programs->fragment_simple[i]);
  }

  for (i = 0; i < (GLITZ_FRAGMENT_PROGRAM_TYPES * 3); i++) {
    if (programs->fragment_convolution[i])
      gl->delete_programs_arb (1, &programs->fragment_convolution[i]);
  }

  for (i = 0; i < GLITZ_FRAGMENT_PROGRAMMATIC_PROGRAM_TYPES; i++) {
    if (programs->fragment_programmatic[i])
      gl->delete_programs_arb (1, &programs->fragment_programmatic[i]);
  }
}
