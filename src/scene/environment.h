/*
VirtualEnvironment class
- the virtual world shown by the LED wall: objects placed behind the wall, at different depths
  (to make parallax visible), and a cubemap skybox
- Draw() renders the environment from the given camera: it is used for the wall FBO
  and for the direct (ground truth) view
*/

#pragma once

// Std. Includes
#include <string>

// Loader for OpenGL extensions
#include <glad/glad.h>

// Loader for GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// class to load models
#include <utils/model.h>

// scene shader, lighting parameters and skybox
#include <render/scene_shader.h>
#include <render/lighting.h>
#include <render/skybox.h>

/////////////////// VIRTUAL ENVIRONMENT class ///////////////////////
class VirtualEnvironment
{
public:

    // constructor: models loading, skybox and model matrices (the objects are static)
    // the wall is at z = -3: all the objects are placed behind it (z < -3)
    VirtualEnvironment(const std::string& skyboxFolder)
        : cube("assets/models/cube.obj"), sphere("assets/models/sphere.obj"), skybox(skyboxFolder)
    {
        // far pillar: little parallax
        this->farPillarModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.0f, -15.0f));
        this->farPillarModelMatrix = glm::scale(this->farPillarModelMatrix, glm::vec3(1.0f, 3.0f, 1.0f));

        // near pillar: close to the wall, strong parallax
        this->nearPillarModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 1.5f, -5.0f));
        this->nearPillarModelMatrix = glm::scale(this->nearPillarModelMatrix, glm::vec3(0.4f, 1.5f, 0.4f));

        // sphere: medium distance
        this->sphereModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 1.0f, -8.0f));
    }

    // rendering of the virtual environment from the given camera
    void Draw(SceneShader& shader, const glm::mat4& view, const glm::mat4& projection, const Lighting& lighting)
    {
        shader.Begin(view, projection, lighting);

        // we render the far pillar
        shader.DrawModel(this->cube, this->farPillarModelMatrix, this->farPillarColor);

        // we render the near pillar
        shader.DrawModel(this->cube, this->nearPillarModelMatrix, this->nearPillarColor);

        // we render the sphere
        shader.DrawModel(this->sphere, this->sphereModelMatrix, this->sphereColor);

        // the skybox is rendered last
        this->skybox.Draw(view, projection);
    }

private:
    Model cube;
    Model sphere;
    Skybox skybox;

    // model matrices
    glm::mat4 farPillarModelMatrix;
    glm::mat4 nearPillarModelMatrix;
    glm::mat4 sphereModelMatrix;

    // diffusive components
    glm::vec3 farPillarColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 nearPillarColor = glm::vec3(1.0f, 0.5f, 0.0f);
    glm::vec3 sphereColor = glm::vec3(1.0f, 1.0f, 0.0f);
};
