/*
Stage class
- the physical set in front of the LED wall: floor and subject
- the objects are rendered with the scene shader (Blinn-Phong)
*/

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utils/model.h>
#include <render/scene_shader.h>
#include <render/lighting.h>
#include <render/depth_shader.h>
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

    // rendering of the stage from the given camera, with the shadows of the set light
    void Draw(SceneShader& shader, const glm::mat4& view, const glm::mat4& projection, const Lighting& lighting, const ShadowMap& shadowMap)
    {
        shader.Begin(view, projection, lighting, &shadowMap);

        shader.DrawModel(this->plane, this->planeModelMatrix, config::PLANE_COLOR);

        shader.DrawModel(this->subject, this->subjectModelMatrix, config::SUBJECT_COLOR);
    }

    // rendering of the stage from the set light, depth only (shadow pass): same model matrices of Draw()
    void DrawDepth(DepthShader& depthShader, const glm::mat4& lightSpaceMatrix)
    {
        depthShader.Begin(lightSpaceMatrix);

        depthShader.DrawModel(this->plane, this->planeModelMatrix);

        depthShader.DrawModel(this->subject, this->subjectModelMatrix);
    }

private:
    Model plane;
    Model subject;

    glm::mat4 planeModelMatrix;
    glm::mat4 subjectModelMatrix;
};
