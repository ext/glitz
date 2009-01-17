#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "glitz_dummyint.h"

static glitz_dummy_drawable_t*
_glitz_dummy_create_drawable (glitz_drawable_format_t* format,
		int width, int height)
{
	glitz_dummy_drawable_t *drawable;

    drawable = (glitz_dummy_drawable_t *) malloc (sizeof (glitz_dummy_drawable_t));
    if (drawable == NULL)
	return NULL;

    drawable->width = width;
    drawable->height = height;

    _glitz_drawable_init (&drawable->base,
			  (glitz_int_drawable_format_t*)format,
			  NULL,
			  width, height);

    return drawable;
}

glitz_drawable_t *
glitz_dummy_create_drawable (glitz_drawable_format_t* format,
		int width, int height)
{
	glitz_dummy_drawable_t *drawable = _glitz_dummy_create_drawable(format, width, height);

	return &drawable->base;
}
