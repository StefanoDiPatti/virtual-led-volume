/*
Viewport utilities
- ViewportRect: rectangle of pixels (lower-left corner, size), as used by glViewport and glScissor
- FitAspect: largest rectangle with a given aspect ratio, centred inside an area (letterbox / pillarbox)
*/

#pragma once

/////////////////// VIEWPORT RECT struct ///////////////////////
struct ViewportRect
{
    int x, y, width, height;
};

// it returns the largest rectangle with the given aspect ratio (width / height), centred in the area
inline ViewportRect FitAspect(int areaWidth, int areaHeight, float aspect)
{
    ViewportRect viewportRect;

    // area wider than the aspect: full height, bands on the left and right (pillarbox)
    if(((float)areaWidth / (float)areaHeight) > aspect)
    {
        viewportRect.height = areaHeight;
        viewportRect.width = areaHeight * aspect;
    }
    // area taller than the aspect: full width, bands on the top and bottom (letterbox)
    else
    {
        viewportRect.width = areaWidth;
        viewportRect.height = areaWidth / aspect;
    }

    // we centre the rectangle in the area
    viewportRect.x = (areaWidth - viewportRect.width) / 2.0f;
    viewportRect.y = (areaHeight - viewportRect.height) / 2.0f;

    return viewportRect;
}
