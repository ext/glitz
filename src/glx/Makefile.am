if GLITZ_BUILD_GLX_BACKEND

INCLUDES =	      \
	$(GLITZ_INC)  \
	$(GLX_CFLAGS)

lib_LTLIBRARIES = libglitz-glx.la
include_HEADERS = glitz-glx.h

libglitz_glx_la_SOURCES =     \
	glitz-glx.h	      \
	glitz_glx_drawable.c  \
	glitz_glx_format.c    \
	glitz_glx_info.c      \
	glitz_glx_extension.c \
	glitz_glx_context.c   \
	glitz_glx_pbuffer.c   \
	glitz_glxext.h	      \
	glitz_glxint.h

libglitz_glx_la_LDFLAGS = -version-info @VERSION_INFO@ -no-undefined
libglitz_glx_la_LIBADD = $(GLITZ_LIB) $(GLX_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = glitz-glx.pc

endif

EXTRA_DIST =		      \
	glitz-glx.h	      \
	glitz_glx_drawable.c  \
	glitz_glx_format.c    \
	glitz_glx_info.c      \
	glitz_glx_extension.c \
	glitz_glx_context.c   \
	glitz_glx_pbuffer.c   \
	glitz_glxext.h	      \
	glitz_glxint.h	      \
	glitz-glx.pc.in	      \
	glitz-glx.man
