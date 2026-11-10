/*
Skybox class
- cubemap environment map rendered on a cube surrounding the camera (shaders/skybox.vert and skybox.frag)
- the skybox is rendered last: its depth is forced to 1.0 in the vertex shader (xyww),
  so GL_LEQUAL is needed to let it pass the depth test only where nothing else was drawn
*/

#pragma once

// Std. Includes
#include <string>

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

// texture loading functions
#include <render/texture.h>

/////////////////// SKYBOX class ///////////////////////
class Skybox
{
public:

    //////////////////////////////////////////
    // constructor: Shader Program, cube model, cubemap texture and uniform locations
    // cubemapFolder: folder with the 6 images of the cubemap (with the final '/')
    Skybox(const std::string& cubemapFolder)
        : shader("shaders/skybox.vert", "shaders/skybox.frag"), cube("assets/models/cube.obj")
    {
        this->texture = LoadTextureCube(cubemapFolder);

        this->samplerLocation = glCheckUniformLocation(this->shader.Program, "tCube");
        this->viewLocation = glCheckUniformLocation(this->shader.Program, "view");
        this->projectionLocation = glCheckUniformLocation(this->shader.Program, "projection");
    }

    // the instance owns OpenGL resources: copies are not allowed
    Skybox(const Skybox&) = delete;
    Skybox& operator=(const Skybox&) = delete;

    //////////////////////////////////////////
    // destructor: we delete the Shader Program and the cubemap
    ~Skybox()
    {
        this->shader.Delete();
        GL_CHECK(glDeleteTextures(1, &this->texture));
    }

    //////////////////////////////////////////
    // rendering of the skybox (it must be called after all the other objects of the pass)
    void Draw(const glm::mat4& view, const glm::mat4& projection)
    {
        // the skybox has maximum depth, so it passes the depth test only with GL_LEQUAL
        GL_CHECK(glDepthFunc(GL_LEQUAL));

        this->shader.Use();

        // we activate the cube map on the texture unit 0
        GL_CHECK(glActiveTexture(GL_TEXTURE0));
        GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, this->texture));
        GL_CHECK(glUniform1i(this->samplerLocation, 0));

        // rotation-only view matrix: removing the translation keeps the sky infinitely far away
        glm::mat4 skyView = glm::mat4(glm::mat3(view));

        // we pass projection and view matrices to the Shader Program
        GL_CHECK(glUniformMatrix4fv(this->projectionLocation, 1, GL_FALSE, glm::value_ptr(projection)));
        GL_CHECK(glUniformMatrix4fv(this->viewLocation, 1, GL_FALSE, glm::value_ptr(skyView)));

        // we render the skybox
        this->cube.Draw();

        // restore the default depth test for the next draws
        GL_CHECK(glDepthFunc(GL_LESS));
    }

private:
    Shader shader;
    Model cube;
    GLuint texture = 0;

    // variables used to store uniform location inside shaders
    GLint samplerLocation, viewLocation, projectionLocation;
};
