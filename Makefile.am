SUBDIRS = . src

if GLITZ_BUILD_GLX_BACKEND
libglitz_glx_extra_dist = glitz-glx.pc.in
libglitz_glx_data = glitz-glx.pc
else
libglitz_glx_extra_dist =
libglitz_glx_data =
endif

if GLITZ_BUILD_AGL_BACKEND
libglitz_agl_extra_dist = glitz-agl.pc.in
libglitz_agl_data = glitz-agl.pc
else
libglitz_agl_extra_dist =
libglitz_agl_data =
endif

EXTRA_DIST = \
	COPYING \
	glitz.pc.in \
	$(libglitz_glx_extra_dist)
	$(libglitz_agl_extra_dist)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	glitz.pc \
	$(libglitz_glx_data) \
	$(libglitz_agl_data)
