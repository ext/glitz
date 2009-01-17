if GLITZ_BUILD_DUMMY_BACKEND

INCLUDES =	      \
	$(GLITZ_INC)

lib_LTLIBRARIES = libglitz-dummy.la
include_HEADERS = glitz-dummy.h

libglitz_dummy_la_SOURCES =     \
	glitz-dummy.h          \
	glitz_dummy_drawable.c \
	glitz_dummyint.h

libglitz_dummy_la_LDFLAGS = -version-info @VERSION_INFO@ -no-undefined
libglitz_dummy_la_LIBADD = $(GLITZ_LIB)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = glitz-dummy.pc

endif

EXTRA_DIST =		       \
	glitz-dummy.h          \
	glitz_dummy_drawable.c \
	glitz_dummyint.h
