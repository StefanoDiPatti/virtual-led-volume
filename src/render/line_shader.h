/*
LineShader class
- Shader Program of the debug lines (shaders/line.vert and line.frag)
- unlit: every vertex has its own colour, the positions are already in world coordinates
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

/////////////////// LINE SHADER class ///////////////////////
class LineShader
{
public:

    // constructor: creation of the Shader Program and of the uniform locations
    LineShader(const GLchar* vertexPath, const GLchar* fragmentPath) : shader(vertexPath, fragmentPath)
    {
        this->viewLocation = glCheckUniformLocation(this->shader.Program, "view");
        this->projectionLocation = glCheckUniformLocation(this->shader.Program, "projection");
    }

    // the instance owns the Shader Program: copies are not allowed
    LineShader(const LineShader&) = delete;
    LineShader& operator=(const LineShader&) = delete;

    // destructor: we delete the Shader Program
    ~LineShader()
    {
        this->shader.Delete();
    }

    // usage of the Shader Program and transformation matrices
    void Begin(const glm::mat4& view, const glm::mat4& projection)
    {
        this->shader.Use();

        // we pass view and projection matrices to the Shader Program
        GL_CHECK(glUniformMatrix4fv(this->viewLocation, 1, GL_FALSE, glm::value_ptr(view)));
        GL_CHECK(glUniformMatrix4fv(this->projectionLocation, 1, GL_FALSE, glm::value_ptr(projection)));
    }

private:
    Shader shader;

    // variables used to store uniform location inside shaders
    GLint viewLocation, projectionLocation;
};
