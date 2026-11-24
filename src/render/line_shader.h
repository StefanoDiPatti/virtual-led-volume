/*
LineShader class
- Shader Program of the debug lines (shaders/line.vert and line.frag)
- unlit: every vertex has its own colour, the positions are already in world coordinates
*/

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utils/shader.h>
#include <render/gl_check.h>

/////////////////// LINE SHADER class ///////////////////////
class LineShader
{
public:

    LineShader(const GLchar* vertexPath, const GLchar* fragmentPath) : shader(vertexPath, fragmentPath)
    {
        this->viewLocation = glCheckUniformLocation(this->shader.Program, "view");
        this->projectionLocation = glCheckUniformLocation(this->shader.Program, "projection");
    }

    LineShader(const LineShader&) = delete;
    LineShader& operator=(const LineShader&) = delete;

    ~LineShader()
    {
        this->shader.Delete();
    }

    // usage of the Shader Program and transformation matrices
    void Begin(const glm::mat4& view, const glm::mat4& projection)
    {
        this->shader.Use();

        GL_CHECK(glUniformMatrix4fv(this->viewLocation, 1, GL_FALSE, glm::value_ptr(view)));
        GL_CHECK(glUniformMatrix4fv(this->projectionLocation, 1, GL_FALSE, glm::value_ptr(projection)));
    }

private:
    Shader shader;

    GLint viewLocation, projectionLocation;
};
