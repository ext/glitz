/*
 * Copyright © 2004 David Sveningsson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * David Sveningsson not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * David Sveningsson makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DAVID SVENINSSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DAVID SVENINSSON BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Sveningsson <ext@sidvind.com>
 */

#ifndef GLITZ_DUMMYINT_H_INCLUDED
#define GLITZ_DUMMYINT_H_INCLUDED

#include "glitz.h"
#include "glitzint.h"

#include "glitz-dummy.h"

typedef struct _glitz_dummy_drawable glitz_dummy_drawable_t;

struct _glitz_dummy_drawable {
    glitz_drawable_t        base;

    int width;
    int height;
};

#endif /* GLITZ_DUMMYINT_H_INCLUDED */
