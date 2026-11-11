/*
WallShader class
- Shader Program of the LED wall (shaders/wall.vert and wall.frag)
- unlit (emissive) textured surface: the texture is sampled from the texture unit 0
*/

#pragma once

// Loader for OpenGL extensions
#include <glad/glad.h>

// Loader for GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// class to manage shaders
#include <utils/shader.h>

// class to manage OpenGL failures
#include <render/gl_check.h>

/////////////////// WALL SHADER class ///////////////////////
class WallShader
{
public:

    // constructor: creation of the Shader Program and of the uniform locations
    WallShader(const GLchar* vertexPath, const GLchar* fragmentPath) : shader(vertexPath, fragmentPath)
    {
        this->modelLocation = glCheckUniformLocation(this->shader.Program, "model");
        this->viewLocation = glCheckUniformLocation(this->shader.Program, "view");
        this->projectionLocation = glCheckUniformLocation(this->shader.Program, "projection");
        this->textureLocation = glCheckUniformLocation(this->shader.Program, "wallTexture");
    }

    // the instance owns the Shader Program: copies are not allowed
    WallShader(const WallShader&) = delete;
    WallShader& operator=(const WallShader&) = delete;

    // destructor: we delete the Shader Program
    ~WallShader()
    {
        this->shader.Delete();
    }

    // usage of the Shader Program, texture and transformation matrices
    void Begin(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, GLuint texture)
    {
        this->shader.Use();

        // we activate the texture unit 0, and we bind the texture to it
        GL_CHECK(glActiveTexture(GL_TEXTURE0));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
        // the sampler reads from the texture unit 0
        GL_CHECK(glUniform1i(this->textureLocation, 0));

        // we pass model, view and projection matrices to the Shader Program
        GL_CHECK(glUniformMatrix4fv(this->modelLocation, 1, GL_FALSE, glm::value_ptr(model)));
        GL_CHECK(glUniformMatrix4fv(this->viewLocation, 1, GL_FALSE, glm::value_ptr(view)));
        GL_CHECK(glUniformMatrix4fv(this->projectionLocation, 1, GL_FALSE, glm::value_ptr(projection)));
    }

private:
    Shader shader;

    // variables used to store uniform location inside shaders
    GLint modelLocation, viewLocation, projectionLocation, textureLocation;
};
