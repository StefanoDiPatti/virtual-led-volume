/*
DepthShader class
- Shader Program of the shadow pass (shaders/depth.vert and depth.frag)
- the objects are transformed into the clip space of the set light, only the depth is written
- Begin() sets the light space matrix, DrawModel() sets the model matrix and renders a model
*/

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utils/shader.h>
#include <utils/model.h>
#include <render/gl_check.h>

/////////////////// DEPTH SHADER class ///////////////////////
class DepthShader
{
public:

    DepthShader(const GLchar* vertexPath, const GLchar* fragmentPath) : shader(vertexPath, fragmentPath)
    {
        this->modelLocation = glCheckUniformLocation(this->shader.Program, "model");
        this->lightSpaceLocation = glCheckUniformLocation(this->shader.Program, "lightSpaceMatrix");
    }

    DepthShader(const DepthShader&) = delete;
    DepthShader& operator=(const DepthShader&) = delete;

    ~DepthShader()
    {
        this->shader.Delete();
    }

    // usage of the Shader Program, and uniform shared by all the objects of the shadow pass
    void Begin(const glm::mat4& lightSpaceMatrix)
    {
        this->shader.Use();

        GL_CHECK(glUniformMatrix4fv(this->lightSpaceLocation, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix)));
    }

    // per-object uniform and rendering of the model (Begin() must be called first)
    void DrawModel(Model& model, const glm::mat4& modelMatrix)
    {
        GL_CHECK(glUniformMatrix4fv(this->modelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix)));

        model.Draw();
    }

private:
    Shader shader;

    GLint modelLocation, lightSpaceLocation;
};
