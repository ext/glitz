/*
 * Copyright © 2005 Novell, Inc.
 * 
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, 
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "glitzint.h"

void
_glitz_context_init (glitz_context_t  *context,
                     glitz_drawable_t *drawable)
{
    glitz_drawable_reference (drawable);
    
    context->ref_count    = 1;
    context->drawable     = drawable;
    context->surface      = NULL;
    context->closure      = NULL;
    context->lose_current = NULL;
}

void
_glitz_context_fini (glitz_context_t *context)
{
    glitz_drawable_destroy (context->drawable);
    if (context->surface)
        glitz_surface_destroy (context->surface);
}

glitz_context_t *
glitz_context_create (glitz_drawable_t        *drawable,
                      glitz_drawable_format_t *format)
{
    return drawable->backend->create_context (drawable, format);
}
slim_hidden_def(glitz_context_create);

void
glitz_context_destroy (glitz_context_t *context)
{
    if (!context)
        return;

    context->ref_count--;
    if (context->ref_count)
        return;

    context->drawable->backend->destroy_context (context);
}
slim_hidden_def(glitz_context_destroy);

void
glitz_context_reference (glitz_context_t *context)
{
    if (!context)
        return;
    
    context->ref_count++;
}
slim_hidden_def(glitz_context_reference);

void
glitz_context_copy (glitz_context_t *src,
                    glitz_context_t *dst,
                    unsigned long   mask)
{
    src->drawable->backend->copy_context (src, dst, mask);
}
slim_hidden_def(glitz_context_copy);

void
glitz_context_set_user_data (glitz_context_t               *context,
                             void                          *closure,
                             glitz_lose_current_function_t lose_current)
{
    context->closure = closure;
    context->lose_current = lose_current;
}
slim_hidden_def(glitz_context_set_user_data);

glitz_function_pointer_t
glitz_context_get_proc_address (glitz_context_t *context,
                                const char      *name)
{
    return context->drawable->backend->get_proc_address (context, name);
}
slim_hidden_def(glitz_context_get_proc_address);

void
glitz_context_make_current (glitz_context_t *context)
{
    glitz_surface_t *surface = context->surface;

    if (surface->attached)
    {
        glitz_box_t *scissor  = &context->scissor;
        glitz_box_t *viewport = &context->viewport;
        
        GLITZ_GL_SURFACE (surface);

        surface->attached->backend->make_current (context, surface->attached);

        glitz_surface_sync_drawable (surface);

        REGION_EMPTY (&surface->texture_damage);
        REGION_INIT (&surface->texture_damage, &context->scissor);

        gl->enable (GLITZ_GL_SCISSOR_TEST);

        gl->scissor (surface->x + scissor->x1,
                     surface->attached->height - surface->y - scissor->y2,
                     scissor->x2 - scissor->x1,
                     scissor->y2 - scissor->y1);

        gl->viewport (surface->x + viewport->x1,
                      surface->attached->height - surface->y - viewport->y2,
                      viewport->x2 - viewport->x1,
                      viewport->y2 - viewport->y1);

        gl->draw_buffer (surface->buffer);
    }
    else if (surface->drawable->backend->feature_mask &
             GLITZ_FEATURE_FRAMEBUFFER_OBJECT_MASK)
    {
        surface->drawable->backend->make_current (context, surface->drawable);

        if (!glitz_framebuffer_complete (&surface->drawable->backend->gl,
                                         &surface->framebuffer,
                                         &surface->texture))
            glitz_surface_status_add (surface,
                                      GLITZ_STATUS_NOT_SUPPORTED_MASK);
    }
}
slim_hidden_def(glitz_context_make_current);

void
glitz_context_set_surface (glitz_context_t *context,
                           glitz_surface_t *surface)
{
    glitz_surface_reference (surface);

    if (context->surface)
        glitz_surface_destroy (context->surface);

    context->surface  = surface;
    context->scissor  = surface->box;
    context->viewport = surface->box;
}
slim_hidden_def(glitz_context_set_surface);

void
glitz_context_set_scissor (glitz_context_t *context,
                           int             x,
                           int             y,
                           int             width,
                           int             height)
{
    glitz_surface_t *surface = context->surface;
    
    GLITZ_GL_SURFACE (surface);

    if (surface->attached)
    {
        glitz_box_t *scissor = &context->scissor;
        
        *scissor = surface->box;
        if (x > 0)
            scissor->x1 = x;
        if (y > 0)
            scissor->y2 -= y;
        if (x + width < scissor->x2)
            scissor->x2 = x + width;
        if (y + height < scissor->y2)
            scissor->y1 = scissor->y2 - y - height;

        gl->scissor (surface->x + scissor->x1,
                     surface->attached->height - surface->y - scissor->y2,
                     scissor->x2 - scissor->x1,
                     scissor->y2 - scissor->y1);
    } else
        gl->scissor (x, y, width, height);
}
slim_hidden_def(glitz_context_set_scissor);

void
glitz_context_set_viewport (glitz_context_t *context,
                            int             x,
                            int             y,
                            int             width,
                            int             height)
{
    glitz_surface_t *surface = context->surface;
    
    GLITZ_GL_SURFACE (surface);

    if (surface->attached)
    {
        glitz_box_t *viewport = &context->viewport;
        
        viewport->x1 = x;
        viewport->y1 = surface->box.y2 - y - height;
        viewport->x2 = x + width;
        viewport->y2 = surface->box.y2 - y;
        
        gl->viewport (surface->x + viewport->x1,
                      surface->attached->height - surface->y - viewport->y2,
                      viewport->x2 - viewport->x1,
                      viewport->y2 - viewport->y1);
    } else
        gl->viewport (x, y, width, height);
}
slim_hidden_def(glitz_context_set_viewport);
