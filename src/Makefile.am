SUBDIRS = . glx agl

lib_LTLIBRARIES = libglitz.la
include_HEADERS = glitz.h

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
	glitz_trapimp.h	    \
	glitz_gl.h	    \
	glitzint.h

libglitz_la_LDFLAGS = -version-info @VERSION_INFO@ -no-undefined
libglitz_la_LIBADD = -lm

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = glitz.pc

EXTRA_DIST =	     \
	glitz.pc.in  \
	glitz.man
