/*
SceneShader class
- Shader Program of the lit objects (Blinn-Phong, shaders/scene.vert and scene.frag)
- the uniform locations are determined once, in the constructor
- Begin() sets the uniforms shared by all the objects of a pass (camera and lighting),
  DrawModel() sets the per-object uniforms and renders a model
- with a ShadowMap, Begin() also applies the shadows of the set light
*/

#pragma once

// Loader for OpenGL extensions
#include <glad/glad.h>

// Loader for GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// classes to manage shaders and models
#include <utils/shader.h>
#include <utils/model.h>

// class to manage OpenGL failures
#include <render/gl_check.h>

// lighting parameters
#include <render/lighting.h>

// depth map of the set light, for the shadows
#include <render/shadow_map.h>

// named constants of the application
#include <core/config.h>

/////////////////// SCENE SHADER class ///////////////////////
class SceneShader
{
public:

    // constructor: creation of the Shader Program and of the uniform locations
    SceneShader(const GLchar* vertexPath, const GLchar* fragmentPath) : shader(vertexPath, fragmentPath)
    {
        // transformation matrices
        this->modelLocation = glCheckUniformLocation(this->shader.Program, "model");
        this->viewLocation = glCheckUniformLocation(this->shader.Program, "view");
        this->projectionLocation = glCheckUniformLocation(this->shader.Program, "projection");
        this->normalsLocation = glCheckUniformLocation(this->shader.Program, "normals");

        // lighting and material
        this->lightDirectionLocation = glCheckUniformLocation(this->shader.Program, "lightDirection");
        this->matDiffuseLocation = glCheckUniformLocation(this->shader.Program, "diffuseColor");
        this->matAmbientLocation = glCheckUniformLocation(this->shader.Program, "ambientColor");
        this->matSpecularLocation = glCheckUniformLocation(this->shader.Program, "specularColor");
        this->kaLocation = glCheckUniformLocation(this->shader.Program, "Ka");
        this->kdLocation = glCheckUniformLocation(this->shader.Program, "Kd");
        this->ksLocation = glCheckUniformLocation(this->shader.Program, "Ks");
        this->shineLocation = glCheckUniformLocation(this->shader.Program, "shininess");

        // shadow mapping
        this->lightSpaceLocation = glCheckUniformLocation(this->shader.Program, "lightSpaceMatrix");
        this->shadowMapLocation = glCheckUniformLocation(this->shader.Program, "shadowMap");
        this->shadowModeLocation = glCheckUniformLocation(this->shader.Program, "shadowMode");
    }

    // the instance owns the Shader Program: copies are not allowed
    SceneShader(const SceneShader&) = delete;
    SceneShader& operator=(const SceneShader&) = delete;

    // destructor: we delete the Shader Program
    ~SceneShader()
    {
        this->shader.Delete();
    }

    // usage of the Shader Program, and uniforms shared by all the objects rendered with the same camera
    void Begin(const glm::mat4& view, const glm::mat4& projection, const Lighting& lighting, const ShadowMap* shadowMap = nullptr)
    {
        // the view matrix is stored, because it is needed to compute the normal matrix of each object
        this->view = view;

        // usage of Shader Program as part of the current rendering process
        this->shader.Use();

        // shadow map of the set light: we bind it to its texture unit, and the sampler reads from that unit
        if(shadowMap != nullptr)
        {
            GL_CHECK(glActiveTexture(GL_TEXTURE0 + SHADOW_TEXTURE_UNIT));
            GL_CHECK(glBindTexture(GL_TEXTURE_2D, shadowMap->GetDepthTexture()));
            GL_CHECK(glUniform1i(this->shadowMapLocation, SHADOW_TEXTURE_UNIT));
            GL_CHECK(glUniformMatrix4fv(this->lightSpaceLocation, 1, GL_FALSE, glm::value_ptr(shadowMap->GetLightSpaceMatrix())));
            GL_CHECK(glUniform1i(this->shadowModeLocation, (int)lighting.shadowMode));
        }
        // no shadow map (e.g. virtual environment): shadows disabled. The uniform must be reset,
        // otherwise the value set by the previous Begin() is kept by the Shader Program
        else
            GL_CHECK(glUniform1i(this->shadowModeLocation, (int)SHADOW_MODE::OFF));

        // directional light: the same direction used by the shadow map (from the light to the target)
        glm::vec3 lightDirection = glm::normalize(config::SHADOW_TARGET - lighting.lightPosition);

        // we assign the value to the uniform variables
        GL_CHECK(glUniform3fv(this->lightDirectionLocation, 1, glm::value_ptr(lightDirection)));
        GL_CHECK(glUniform3fv(this->matAmbientLocation, 1, glm::value_ptr(config::AMBIENT_COLOR)));
        GL_CHECK(glUniform3fv(this->matSpecularLocation, 1, glm::value_ptr(config::SPECULAR_COLOR)));
        GL_CHECK(glUniform1f(this->kaLocation, lighting.Ka));
        GL_CHECK(glUniform1f(this->kdLocation, lighting.Kd));
        GL_CHECK(glUniform1f(this->ksLocation, lighting.Ks));
        GL_CHECK(glUniform1f(this->shineLocation, lighting.shininess));

        // we pass projection and view matrices to the Shader Program
        GL_CHECK(glUniformMatrix4fv(this->projectionLocation, 1, GL_FALSE, glm::value_ptr(projection)));
        GL_CHECK(glUniformMatrix4fv(this->viewLocation, 1, GL_FALSE, glm::value_ptr(view)));
    }

    // per-object uniforms and rendering of the model (Begin() must be called first)
    void DrawModel(Model& model, const glm::mat4& modelMatrix, const glm::vec3& diffuseColor)
    {
        // if we cast a mat4 to a mat3, we are automatically considering the upper left 3x3 submatrix
        glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(this->view * modelMatrix));
        GL_CHECK(glUniformMatrix4fv(this->modelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix)));
        GL_CHECK(glUniformMatrix3fv(this->normalsLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix)));

        GL_CHECK(glUniform3fv(this->matDiffuseLocation, 1, glm::value_ptr(diffuseColor)));

        // we render the model
        model.Draw();
    }

private:
    Shader shader;

    // view matrix of the current pass (set in Begin)
    glm::mat4 view = glm::mat4(1.0f);

    // variables used to store uniform location inside shaders
    GLint modelLocation, viewLocation, projectionLocation, normalsLocation;
    GLint lightDirectionLocation, matDiffuseLocation, matAmbientLocation, matSpecularLocation;
    GLint kdLocation, kaLocation, ksLocation, shineLocation;
    GLint lightSpaceLocation, shadowMapLocation, shadowModeLocation;
};
