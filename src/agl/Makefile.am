if GLITZ_BUILD_AGL_BACKEND

INCLUDES =	      \
	$(GLITZ_INC)  \
	$(AGL_CFLAGS)

lib_LTLIBRARIES = libglitz-agl.la
include_HEADERS = glitz-agl.h

libglitz_agl_la_SOURCES =     \
	glitz-agl.h	      \
	glitz_agl_drawable.c  \
	glitz_agl_format.c    \
	glitz_agl_info.c      \
	glitz_agl_extension.c \
	glitz_agl_context.c   \
	glitz_agl_pbuffer.c   \
	glitz_aglint.h

libglitz_agl_la_LDFLAGS = -version-info @VERSION_INFO@
libglitz_agl_la_LIBADD = $(GLITZ_LIB) $(AGL_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = glitz-agl.pc

endif

EXTRA_DIST =		      \
	glitz-agl.h	      \
	glitz_agl_drawable.c  \
	glitz_agl_format.c    \
	glitz_agl_info.c      \
	glitz_agl_extension.c \
	glitz_agl_context.c   \
	glitz_agl_pbuffer.c   \
	glitz_aglint.h	      \
	glitz-agl.pc.in	      \
	glitz-agl.man
