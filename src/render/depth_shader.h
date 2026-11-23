/*
DepthShader class
- Shader Program of the shadow pass (shaders/depth.vert and depth.frag)
- the objects are transformed into the clip space of the set light, only the depth is written
- Begin() sets the light space matrix, DrawModel() sets the model matrix and renders a model
*/

#pragma once

// Loader for OpenGL extensions
#include <glad/glad.h>

// Loader for GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// classes to manage shaders and models
#include <utils/shader.h>
#include <utils/model.h>

// class to manage OpenGL failures
#include <render/gl_check.h>

/////////////////// DEPTH SHADER class ///////////////////////
class DepthShader
{
public:

    // constructor: creation of the Shader Program and of the uniform locations
    DepthShader(const GLchar* vertexPath, const GLchar* fragmentPath) : shader(vertexPath, fragmentPath)
    {
        // transformation matrices
        this->modelLocation = glCheckUniformLocation(this->shader.Program, "model");
        this->lightSpaceLocation = glCheckUniformLocation(this->shader.Program, "lightSpaceMatrix");
    }

    // the instance owns the Shader Program: copies are not allowed
    DepthShader(const DepthShader&) = delete;
    DepthShader& operator=(const DepthShader&) = delete;

    // destructor: we delete the Shader Program
    ~DepthShader()
    {
        this->shader.Delete();
    }

    // usage of the Shader Program, and uniform shared by all the objects of the shadow pass
    void Begin(const glm::mat4& lightSpaceMatrix)
    {
        // usage of Shader Program as part of the current rendering process
        this->shader.Use();

        // we pass the light space matrix (projection * view of the light) to the Shader Program
        GL_CHECK(glUniformMatrix4fv(this->lightSpaceLocation, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix)));
    }

    // per-object uniform and rendering of the model (Begin() must be called first)
    void DrawModel(Model& model, const glm::mat4& modelMatrix)
    {
        GL_CHECK(glUniformMatrix4fv(this->modelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix)));

        // we render the model
        model.Draw();
    }

private:
    Shader shader;

    // variables used to store uniform location inside shaders
    GLint modelLocation, lightSpaceLocation;
};
