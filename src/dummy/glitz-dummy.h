#ifndef GLITZ_DUMMY_H_INCLUDED
#define GLITZ_DUMMY_H_INCLUDED

#include <glitz.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* glitz_dummy_drawable.c */

glitz_drawable_t *
glitz_dummy_create_drawable (glitz_drawable_format_t* format,
		int width, int height);


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* GLITZ_DUMMY_H_INCLUDED */
