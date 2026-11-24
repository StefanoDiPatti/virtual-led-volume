/*
VirtualEnvironment class
- the virtual world shown by the LED wall: objects placed behind the wall, at different depths
  (to make parallax visible), and a cubemap skybox
- Draw() renders the environment from the given camera: it is used for the wall FBO
  and for the direct (ground truth) view
- own directional light, independent of the set light
*/

#pragma once

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utils/model.h>
#include <render/scene_shader.h>
#include <render/lighting.h>
#include <render/skybox.h>
#include <core/config.h>

/////////////////// VIRTUAL ENVIRONMENT class ///////////////////////
class VirtualEnvironment
{
public:

    // constructor: models loading, skybox and model matrices (the objects are static)
    // the wall is at z = -3: all the objects are placed behind it (z < -3)
    VirtualEnvironment(const std::string& skyboxFolder)
        : cube(CUBE_MODEL_PATH), sphere(SPHERE_MODEL_PATH), skybox(skyboxFolder)
    {
        // far pillar: little parallax
        this->farPillarModelMatrix = glm::translate(glm::mat4(1.0f), config::FAR_PILLAR_POSITION);
        this->farPillarModelMatrix = glm::scale(this->farPillarModelMatrix, config::FAR_PILLAR_SCALE);

        // near pillar: close to the wall, strong parallax
        this->nearPillarModelMatrix = glm::translate(glm::mat4(1.0f), config::NEAR_PILLAR_POSITION);
        this->nearPillarModelMatrix = glm::scale(this->nearPillarModelMatrix, config::NEAR_PILLAR_SCALE);

        // sphere: medium distance
        this->sphereModelMatrix = glm::translate(glm::mat4(1.0f), config::SPHERE_POSITION);

        // own light of the virtual environment: its sun, fixed in the virtual world
        this->lighting.lightPosition = config::ENVIRONMENT_LIGHT_POSITION;
    }

    void Draw(SceneShader& shader, const glm::mat4& view, const glm::mat4& projection)
    {
        shader.Begin(view, projection, this->lighting);

        shader.DrawModel(this->cube, this->farPillarModelMatrix, config::FAR_PILLAR_COLOR);

        shader.DrawModel(this->cube, this->nearPillarModelMatrix, config::NEAR_PILLAR_COLOR);

        shader.DrawModel(this->sphere, this->sphereModelMatrix, config::SPHERE_COLOR);

        // the skybox is rendered last
        this->skybox.Draw(view, projection);
    }

    // the GUI changes the environment light through this reference
    Lighting& GetLighting() { return this->lighting; }

private:
    Model cube;
    Model sphere;
    Skybox skybox;

    Lighting lighting;

    glm::mat4 farPillarModelMatrix;
    glm::mat4 nearPillarModelMatrix;
    glm::mat4 sphereModelMatrix;
};
