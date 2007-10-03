SUBDIRS = . glx agl egl wgl

if OS_WIN32
glitz_def = $(srcdir)/glitz.def
libglitz_export_symbols = -export-symbols $(glitz_def)
else
glitz_def =
libglitz_export_symbols =
endif

lib_LTLIBRARIES = libglitz.la
include_HEADERS = glitz.h glitz_gl.h glitzint.h

libglitz_la_SOURCES =	    \
	glitz.h		    \
	glitz.c		    \
	glitz_operator.c    \
	glitz_drawable.c    \
	glitz_surface.c	    \
	glitz_texture.c	    \
	glitz_rect.c	    \
	glitz_status.c	    \
	glitz_util.c	    \
	glitz_region.c	    \
	glitz_format.c	    \
	glitz_program.c	    \
	glitz_compose.c	    \
	glitz_filter.c	    \
	glitz_buffer.c	    \
	glitz_geometry.c    \
	glitz_pixel.c	    \
	glitz_trap.c	    \
	glitz_framebuffer.c \
	glitz_context.c	    \
	glitz_trapimp.h	    \
	glitz_gl.h	    \
	glitzint.h

libglitz_la_LDFLAGS = -version-info @VERSION_INFO@ -no-undefined $(libglitz_export_symbols)
libglitz_la_LIBADD = $(LIBM)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = glitz.pc

EXTRA_DIST =	     \
	glitz.def    \
	glitz.pc.in  \
	glitz.man
