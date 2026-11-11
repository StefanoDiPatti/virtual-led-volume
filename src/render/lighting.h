/*
Lighting struct
- parameters of the point light and of the Blinn-Phong illumination model,
  passed as uniforms to the scene shader
*/

#pragma once

// Loader for OpenGL extensions
#include <glad/glad.h>

// Loader for GLM
#include <glm/glm.hpp>

/////////////////// LIGHTING struct ///////////////////////
struct Lighting
{
    // position of a pointlight (world coordinates)
    glm::vec3 lightPosition = glm::vec3(5.0f, 10.0f, 10.0f);

    // ambient and specular components
    glm::vec3 ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::vec3 specularColor = glm::vec3(1.0f, 1.0f, 1.0f);

    // weights for the diffusive, specular and ambient components
    GLfloat Kd = 0.5f;
    GLfloat Ks = 0.4f;
    GLfloat Ka = 0.1f;

    // shininess coefficient for Blinn-Phong shaders
    GLfloat shininess = 25.0f;
};
