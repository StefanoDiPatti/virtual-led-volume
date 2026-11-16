/*
Viewport utilities
- ViewportRect: rectangle of pixels (lower-left corner, size), as used by glViewport and glScissor
- FitAspect: largest rectangle with a given aspect ratio, centred inside an area (letterbox / pillarbox)
- ClearArea: clear of colour and depth limited to a rectangle, through the scissor test
*/

#pragma once

#include <glm/glm.hpp>
#include <render/gl_check.h>

/////////////////// VIEWPORT RECT struct ///////////////////////
struct ViewportRect
{
    int x, y, width, height;
};

// it returns the largest rectangle with the given aspect ratio (width / height), centred in the area
inline ViewportRect FitAspect(ViewportRect area, float aspect)
{
    ViewportRect viewportRect;

    // area wider than the aspect: full height, bands on the left and right (pillarbox)
    if(((float)area.width / (float)area.height) > aspect)
    {
        viewportRect.height = area.height;
        viewportRect.width = area.height * aspect;
    }
    // area taller than the aspect: full width, bands on the top and bottom (letterbox)
    else
    {
        viewportRect.width = area.width;
        viewportRect.height = area.width / aspect;
    }

    // we centre the rectangle in the area
    viewportRect.x = area.x + (area.width - viewportRect.width) / 2.0f;
    viewportRect.y = area.y + (area.height - viewportRect.height) / 2.0f;

    return viewportRect;
}

// we clear colour and depth only inside the rectangle: glClear ignores the viewport, so we use the scissor test
// the scissor test is disabled again at the end, otherwise the next clears are limited to this area
inline void ClearArea(ViewportRect rect, glm::vec3 color)
{
    GL_CHECK(glEnable(GL_SCISSOR_TEST));
    GL_CHECK(glScissor(rect.x, rect.y, rect.width, rect.height));
    GL_CHECK(glClearColor(color.r, color.g, color.b, 1.0f));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GL_CHECK(glDisable(GL_SCISSOR_TEST));
}
