/*
Stage class
- the physical set in front of the LED wall: floor and subject
- the objects are rendered with the scene shader (Blinn-Phong)
*/

#pragma once

// Loader for OpenGL extensions
#include <glad/glad.h>

// Loader for GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// class to load models
#include <utils/model.h>

// scene shader and lighting parameters
#include <render/scene_shader.h>
#include <render/lighting.h>

// named constants of the application
#include <core/config.h>

/////////////////// STAGE class ///////////////////////
class Stage
{
public:

    // constructor: models loading and model matrices (the objects are static)
    Stage() : plane(PLANE_MODEL_PATH), subject(SUBJECT_MODEL_PATH)
    {
        // floor: 10x10 m at y = 0
        this->planeModelMatrix = glm::mat4(1.0f);

        // subject: the model is too big, we scale it down (~1.45 m)
        this->subjectModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(SUBJECT_SCALE));
    }

    // rendering of the stage from the given camera
    void Draw(SceneShader& shader, const glm::mat4& view, const glm::mat4& projection, const Lighting& lighting)
    {
        shader.Begin(view, projection, lighting);

        // we render the plane
        shader.DrawModel(this->plane, this->planeModelMatrix, config::PLANE_COLOR);

        // we render the subject
        shader.DrawModel(this->subject, this->subjectModelMatrix, config::SUBJECT_COLOR);
    }

private:
    Model plane;
    Model subject;

    // model matrices
    glm::mat4 planeModelMatrix;
    glm::mat4 subjectModelMatrix;
};
