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

#ifndef GLITZ_GLXEXT_H_INCLUDED
#define GLITZ_GLXEXT_H_INCLUDED

#ifndef GLX_VERSION_1_3
#define GLX_WINDOW_BIT                     0x00000001
#define GLX_PIXMAP_BIT                     0x00000002
#define GLX_PBUFFER_BIT                    0x00000004
#define GLX_RGBA_BIT                       0x00000001
#define GLX_COLOR_INDEX_BIT                0x00000002
#define GLX_PBUFFER_CLOBBER_MASK           0x08000000
#define GLX_FRONT_LEFT_BUFFER_BIT          0x00000001
#define GLX_FRONT_RIGHT_BUFFER_BIT         0x00000002
#define GLX_BACK_LEFT_BUFFER_BIT           0x00000004
#define GLX_BACK_RIGHT_BUFFER_BIT          0x00000008
#define GLX_AUX_BUFFERS_BIT                0x00000010
#define GLX_DEPTH_BUFFER_BIT               0x00000020
#define GLX_STENCIL_BUFFER_BIT             0x00000040
#define GLX_ACCUM_BUFFER_BIT               0x00000080
#define GLX_CONFIG_CAVEAT                  0x20
#define GLX_X_VISUAL_TYPE                  0x22
#define GLX_TRANSPARENT_TYPE               0x23
#define GLX_TRANSPARENT_INDEX_VALUE        0x24
#define GLX_TRANSPARENT_RED_VALUE          0x25
#define GLX_TRANSPARENT_GREEN_VALUE        0x26
#define GLX_TRANSPARENT_BLUE_VALUE         0x27
#define GLX_TRANSPARENT_ALPHA_VALUE        0x28
#define GLX_DONT_CARE                      0xFFFFFFFF
#define GLX_NONE                           0x8000
#define GLX_SLOW_CONFIG                    0x8001
#define GLX_TRUE_COLOR                     0x8002
#define GLX_DIRECT_COLOR                   0x8003
#define GLX_PSEUDO_COLOR                   0x8004
#define GLX_STATIC_COLOR                   0x8005
#define GLX_GRAY_SCALE                     0x8006
#define GLX_STATIC_GRAY                    0x8007
#define GLX_TRANSPARENT_RGB                0x8008
#define GLX_TRANSPARENT_INDEX              0x8009
#define GLX_VISUAL_ID                      0x800B
#define GLX_SCREEN                         0x800C
#define GLX_NON_CONFORMANT_CONFIG          0x800D
#define GLX_DRAWABLE_TYPE                  0x8010
#define GLX_RENDER_TYPE                    0x8011
#define GLX_X_RENDERABLE                   0x8012
#define GLX_FBCONFIG_ID                    0x8013
#define GLX_RGBA_TYPE                      0x8014
#define GLX_COLOR_INDEX_TYPE               0x8015
#define GLX_MAX_PBUFFER_WIDTH              0x8016
#define GLX_MAX_PBUFFER_HEIGHT             0x8017
#define GLX_MAX_PBUFFER_PIXELS             0x8018
#define GLX_PRESERVED_CONTENTS             0x801B
#define GLX_LARGEST_PBUFFER                0x801C
#define GLX_WIDTH                          0x801D
#define GLX_HEIGHT                         0x801E
#define GLX_EVENT_MASK                     0x801F
#define GLX_DAMAGED                        0x8020
#define GLX_SAVED                          0x8021
#define GLX_WINDOW                         0x8022
#define GLX_PBUFFER                        0x8023
#define GLX_PBUFFER_HEIGHT                 0x8040
#define GLX_PBUFFER_WIDTH                  0x8041

typedef struct __GLXFBConfigRec *GLXFBConfig;
typedef XID GLXFBConfigID;
typedef XID GLXContextID;
typedef XID GLXWindow;
typedef XID GLXPbuffer;

#endif

typedef GLXFBConfig *(* glitz_glx_get_fbconfigs_t)
     (Display *, int, int *);
typedef int (* glitz_glx_get_fbconfig_attrib_t)
     (Display *, GLXFBConfig, int, int *);
typedef XVisualInfo *(* glitz_glx_get_visual_from_fbconfig_t)
     (Display *, GLXFBConfig);
typedef GLXPbuffer (* glitz_glx_create_pbuffer_t)
     (Display *, GLXFBConfig, const int *);
typedef void (* glitz_glx_destroy_pbuffer_t)
     (Display *, GLXPbuffer);

#ifndef GLX_ARB_multisample
#define GLX_SAMPLE_BUFFERS_ARB              0x186a0
#define GLX_SAMPLES_ARB                     0x186a1
#endif

#ifndef GLX_ATI_render_texture
#define GLX_BIND_TO_TEXTURE_RGB_ATI         0x9800
#define GLX_BIND_TO_TEXTURE_RGBA_ATI        0x9801
#define GLX_TEXTURE_FORMAT_ATI              0x9802
#define GLX_TEXTURE_TARGET_ATI              0x9803
#define GLX_MIPMAP_TEXTURE_ATI              0x9804
#define GLX_TEXTURE_RGB_ATI                 0x9805
#define GLX_TEXTURE_RGBA_ATI                0x9806
#define GLX_NO_TEXTURE_ATI                  0x9807
#define GLX_TEXTURE_CUBE_MAP_ATI            0x9808
#define GLX_TEXTURE_1D_ATI                  0x9809
#define GLX_TEXTURE_2D_ATI                  0x980A
#define GLX_MIPMAP_LEVEL_ATI                0x980B
#define GLX_CUBE_MAP_FACE_ATI               0x980C
#define GLX_TEXTURE_CUBE_MAP_POSITIVE_X_ATI 0x980D
#define GLX_TEXTURE_CUBE_MAP_NEGATIVE_X_ATI 0x980E
#define GLX_TEXTURE_CUBE_MAP_POSITIVE_Y_ATI 0x980F
#define GLX_TEXTURE_CUBE_MAP_NEGATIVE_Y_ATI 0x9810
#define GLX_TEXTURE_CUBE_MAP_POSITIVE_Z_ATI 0x9811
#define GLX_TEXTURE_CUBE_MAP_NEGATIVE_Z_ATI 0x9812
#define GLX_FRONT_LEFT_ATI                  0x9813
#define GLX_FRONT_RIGHT_ATI                 0x9814
#define GLX_BACK_LEFT_ATI                   0x9815
#define GLX_BACK_RIGHT_ATI                  0x9816
#define GLX_AUX0_ATI                        0x9817
#define GLX_AUX1_ATI                        0x9818
#define GLX_AUX2_ATI                        0x9819
#define GLX_AUX3_ATI                        0x981A
#define GLX_AUX4_ATI                        0x981B
#define GLX_AUX5_ATI                        0x981C
#define GLX_AUX6_ATI                        0x981D
#define GLX_AUX7_ATI                        0x981E
#define GLX_AUX8_ATI                        0x981F
#define GLX_AUX9_ATI                        0x9820
#define GLX_BIND_TO_TEXTURE_LUMINANCE_ATI   0x9821
#define GLX_BIND_TO_TEXTURE_INTENSITY_ATI   0x9822
#endif

typedef void (* glitz_glx_bind_tex_image_ati_t)
     (Display *, GLXPbuffer, int);
typedef void (* glitz_glx_release_tex_image_ati_t)
     (Display *, GLXPbuffer, int);

#endif /* GLITZ_GLXEXT_H_INCLUDED */
