if GLITZ_BUILD_GLX_BACKEND
libglitz_glx_library = libglitz-glx.la
libglitz_glx_header = glitz-glx.h
else
libglitz_glx_library =
libglitz_glx_header =
endif

if GLITZ_BUILD_AGL_BACKEND
libglitz_agl_library = libglitz-agl.la
libglitz_agl_header = glitz-agl.h
else
libglitz_agl_library =
libglitz_agl_header =
endif

lib_LTLIBRARIES = \
	libglitz.la \
	$(libglitz_glx_library) \
	$(libglitz_agl_library)

include_HEADERS = \
	glitz.h \
	$(libglitz_glx_header) \
	$(libglitz_agl_header)

libglitz_la_SOURCES = \
	glitz.c \
	glitz.h \
	glitz_operator.c \
	glitz_surface.c \
	glitz_texture.c \
	glitz_rect.c \
	glitz_trap.c \
	glitz_tri.c \
	glitz_status.c \
	glitz_matrix.c \
	glitz_util.c \
	glitz_format.c \
	glitz_program.c \
	glitz_programmatic.c \
	glitz_color_range.c \
	glitz_gl.h
	glitzint.h

libglitz_la_LDFLAGS = -version-info @VERSION_INFO@ -no-undefined
libglitz_la_CFLAGS =
libglitz_la_LIBADD = -lm

if GLITZ_BUILD_GLX_BACKEND
libglitz_glx_la_SOURCES = \
	glitz.h \
	glitz_gl.h \
	glitzint.h \
	glitz-glx.h \
	glitz_glx_surface.c \
	glitz_glx_format.c \
	glitz_glx_info.c \
	glitz_glx_extension.c \
	glitz_glx_context.c \
	glitz_glx_pbuffer.c \
	glitz_glxext.h \
	glitz_glxint.h
libglitz_glx_la_LDFLAGS = -version-info @VERSION_INFO@ -no-undefined
libglitz_glx_la_CFLAGS = $(GLX_CFLAGS)
libglitz_glx_la_LIBADD = -lglitz $(GLX_LIBS)
endif

if GLITZ_BUILD_AGL_BACKEND
libglitz_agl_la_SOURCES = \
	glitz.h \
	glitz_gl.h \
	glitzint.h \
	glitz-agl.h \
	glitz_agl_surface.c \
	glitz_agl_format.c \
	glitz_agl_info.c \
	glitz_agl_extension.c \
	glitz_agl_context.c \
	glitz_agl_pbuffer.c \
	glitz_aglext.h
	glitz_aglint.h
libglitz_agl_la_LDFLAGS = -version-info @VERSION_INFO@
libglitz_agl_la_CFLAGS = $(AGL_CFLAGS)
libglitz_agl_la_LIBADD = -lglitz $(AGL_LIBS)
endif
