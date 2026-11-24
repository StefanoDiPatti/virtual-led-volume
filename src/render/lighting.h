/*
Lighting struct
- position of the directional light, weights and shininess of the Blinn-Phong illumination model,
  passed as uniforms to the scene shader (they can change at runtime)
- colours of the light are named constants (config.h)
- technique used for the shadows of the set light
*/

#pragma once

// Loader for OpenGL extensions
#include <glad/glad.h>

// Loader for GLM
#include <glm/glm.hpp>

// shadow technique: same order of the shadowMode values in shaders/scene.frag (0..4)
enum class SHADOW_MODE
{
    OFF,
    NO_BIAS,
    BIAS,
    PCF,
    PCF_HARDWARE
};

/////////////////// LIGHTING struct ///////////////////////
struct Lighting
{
    // directional light: it goes from this position (world coordinates, metres) towards the target in config.h.
    // 5 m to the right of the subject, 10 m high, 10 m towards the cameras (the wall is at z = -3):
    // the front of the subject is lit and its shadow falls behind it, towards the wall
    glm::vec3 lightPosition = glm::vec3(5.0f, 10.0f, 10.0f);

    // weights for the diffusive, specular and ambient components
    GLfloat Kd = 0.5f;
    GLfloat Ks = 0.4f;
    GLfloat Ka = 0.1f;

    // shininess coefficient for Blinn-Phong shaders
    GLfloat shininess = 25.0f;

    // active shadow technique (it can change at runtime)
    SHADOW_MODE shadowMode = SHADOW_MODE::PCF;
};
