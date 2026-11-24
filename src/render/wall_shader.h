/*
WallShader class
- Shader Program of the LED wall (shaders/wall.vert and wall.frag)
- unlit (emissive) textured surface: inner frustum (unit 0) and outer frustum (unit 3), composed per fragment
*/

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <core/config.h>
#include <utils/shader.h>
#include <render/gl_check.h>

// content shown on the wall: inner frustum only, outer frustum only, or both composed (inner where the cine camera looks)
enum class WALL_CONTENT
{
    INNER,
    OUTER,
    COMPOSITE
};

/////////////////// WALL SHADER class ///////////////////////
class WallShader
{
public:

    WallShader(const GLchar* vertexPath, const GLchar* fragmentPath) : shader(vertexPath, fragmentPath)
    {
        this->modelLocation = glCheckUniformLocation(this->shader.Program, "model");
        this->viewLocation = glCheckUniformLocation(this->shader.Program, "view");
        this->projectionLocation = glCheckUniformLocation(this->shader.Program, "projection");
        this->innerTextureLocation = glCheckUniformLocation(this->shader.Program, "innerTexture");
        this->outerTextureLocation = glCheckUniformLocation(this->shader.Program, "outerTexture");
        this->cameraViewProjectionLocation = glCheckUniformLocation(this->shader.Program, "cameraViewProjection");
        this->contentLocation = glCheckUniformLocation(this->shader.Program, "content");
        this->marginLocation = glCheckUniformLocation(this->shader.Program, "innerMargin");
        this->blendLocation = glCheckUniformLocation(this->shader.Program, "innerBlend");

        // samplers, overscan and soft edge never change: we set them once
        this->shader.Use();
        GL_CHECK(glUniform1i(this->innerTextureLocation, WALL_INNER_TEXTURE_UNIT));
        GL_CHECK(glUniform1i(this->outerTextureLocation, WALL_OUTER_TEXTURE_UNIT));
        GL_CHECK(glUniform1f(this->marginLocation, INNER_FRUSTUM_MARGIN));
        GL_CHECK(glUniform1f(this->blendLocation, INNER_FRUSTUM_BLEND));
    }

    WallShader(const WallShader&) = delete;
    WallShader& operator=(const WallShader&) = delete;

    ~WallShader()
    {
        this->shader.Delete();
    }

    // usage of the Shader Program, texture and transformation matrices
    void Begin(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, GLuint innerTexture, GLuint outerTexture, const glm::mat4& cameraViewProjection, WALL_CONTENT content)
    {
        this->shader.Use();

        // inner and outer frustum textures on their texture units
        GL_CHECK(glActiveTexture(GL_TEXTURE0 + WALL_INNER_TEXTURE_UNIT));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, innerTexture));
        GL_CHECK(glActiveTexture(GL_TEXTURE0 + WALL_OUTER_TEXTURE_UNIT));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, outerTexture));
        // we go back to unit 0: the skybox and the GUI expect it as the active unit
        GL_CHECK(glActiveTexture(GL_TEXTURE0));

        GL_CHECK(glUniformMatrix4fv(this->cameraViewProjectionLocation, 1, GL_FALSE, glm::value_ptr(cameraViewProjection)));
        GL_CHECK(glUniform1i(this->contentLocation, (int)content));

        GL_CHECK(glUniformMatrix4fv(this->modelLocation, 1, GL_FALSE, glm::value_ptr(model)));
        GL_CHECK(glUniformMatrix4fv(this->viewLocation, 1, GL_FALSE, glm::value_ptr(view)));
        GL_CHECK(glUniformMatrix4fv(this->projectionLocation, 1, GL_FALSE, glm::value_ptr(projection)));
    }

private:
    Shader shader;

    GLint modelLocation, viewLocation, projectionLocation, innerTextureLocation, outerTextureLocation, cameraViewProjectionLocation, contentLocation, marginLocation, blendLocation;
};
