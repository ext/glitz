if GLITZ_BUILD_WGL_BACKEND

INCLUDES =	      \
	$(GLITZ_INC)  \
	$(WGL_CFLAGS)

if OS_WIN32
glitz_wgl_def = $(srcdir)/glitz-wgl.def
libglitz_wgl_export_symbols = -export-symbols $(glitz_wgl_def)
else
glitz_wgl_def =
libglitz_wgl_export_symbols =
endif

lib_LTLIBRARIES = libglitz-wgl.la
include_HEADERS = glitz-wgl.h

libglitz_wgl_la_SOURCES =     \
	glitz-wgl.h	      \
	glitz_wgl_drawable.c  \
	glitz_wgl_format.c    \
	glitz_wgl_info.c      \
	glitz_wgl_extension.c \
	glitz_wgl_context.c   \
	glitz_wgl_pbuffer.c   \
	glitz_wglext.h	      \
	glitz_wglint.h

libglitz_wgl_la_LDFLAGS = -version-info @VERSION_INFO@ -no-undefined $(libglitz_wgl_export_symbols)
libglitz_wgl_la_LIBADD = $(GLITZ_LIB) $(WGL_LIBS)
libglitz_wgl_la_DEPENDENCIES = $(glitz_wgl_def)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = glitz-wgl.pc

endif

EXTRA_DIST =		      \
	glitz-wgl.def	      \
	glitz-wgl.h	      \
	glitz_wgl_drawable.c  \
	glitz_wgl_format.c    \
	glitz_wgl_info.c      \
	glitz_wgl_extension.c \
	glitz_wgl_context.c   \
	glitz_wgl_pbuffer.c   \
	glitz_wglext.h	      \
	glitz_wglint.h	      \
	glitz-wgl.pc.in
