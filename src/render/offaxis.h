/*
Off-axis projection
- generalized perspective projection of a planar screen (the LED wall) from the eye position
- pa, pb, pc: lower-left, lower-right and upper-left corners of the screen (world coordinates)
- based on: R. Kooima, "Generalized Perspective Projection"
*/

#pragma once

// Loader for GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/////////////////// OFF-AXIS MATRICES struct ///////////////////////
struct OffAxisMatrices
{
    glm::mat4 view;
    glm::mat4 projection;
};

//////////////////////////////////////////
// it returns the view and projection matrices for the eye position pe
inline OffAxisMatrices OffAxisProjection(const glm::vec3& pa, const glm::vec3& pb, const glm::vec3& pc, const glm::vec3& pe, float nearPlane, float farPlane)
{
    OffAxisMatrices offAxisMatrices;

    // we calculate the orthonormal basis of the screen
    glm::vec3 vr = glm::normalize(pb - pa);   // right
    glm::vec3 vu = glm::normalize(pc - pa);   // up
    glm::vec3 vn = glm::normalize(glm::cross(vr, vu)); // normal (towards the viewer)

    // vectors from the eye to the corners (not normalized)
    glm::vec3 va = pa - pe;
    glm::vec3 vb = pb - pe;
    glm::vec3 vc = pc - pe;

    // distance from the eye to the screen plane
    float d = -glm::dot(va, vn);

    // frustum extents on the near plane (scaled by nearPlane/d)
    float l = glm::dot(vr, va) * nearPlane / d;
    float r = glm::dot(vr, vb) * nearPlane / d;
    float b = glm::dot(vu, va) * nearPlane / d;
    float t = glm::dot(vu, vc) * nearPlane / d;

    // asymmetric frustum
    offAxisMatrices.projection = glm::frustum(l, r, b, t, nearPlane, farPlane);

    // view matrix: we move the eye to the origin and we align the screen basis to the axes (M^T, M has the basis as columns)
    glm::mat4 M(glm::vec4(vr, 0.0f), glm::vec4(vu, 0.0f), glm::vec4(vn, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    offAxisMatrices.view = glm::transpose(M) * glm::translate(glm::mat4(1.0f), -pe);

    return offAxisMatrices;
}
