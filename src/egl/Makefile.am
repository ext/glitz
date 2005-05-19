if GLITZ_BUILD_EGL_BACKEND

INCLUDES =	      \
	$(GLITZ_INC)  \
	$(EGL_CFLAGS)

lib_LTLIBRARIES = libglitz-egl.la
include_HEADERS = glitz-egl.h

libglitz_egl_la_SOURCES =     \
	glitz-egl.h	      \
	glitz_egl_surface.c  \
	glitz_egl_config.c    \
	glitz_egl_info.c      \
	glitz_egl_extension.c \
	glitz_egl_context.c   \
	glitz_egl_pbuffer.c   \
	glitz_eglext.h	      \
	glitz_eglint.h

libglitz_egl_la_LDFLAGS = -version-info @VERSION_INFO@ -no-undefined
libglitz_egl_la_LIBADD = $(GLITZ_LIB) $(EGL_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = glitz-egl.pc

endif

EXTRA_DIST =		      \
	glitz-egl.h	      \
	glitz_egl_surface.c  \
	glitz_egl_config.c    \
	glitz_egl_info.c      \
	glitz_egl_extension.c \
	glitz_egl_context.c   \
	glitz_egl_pbuffer.c   \
	glitz_eglext.h	      \
	glitz_eglint.h	      \
	glitz-egl.pc.in	      \
	glitz-egl.man
