/*
LedWall class
- planar rectangular LED wall, defined by 3 corners in world coordinates:
  pa (lower-left), pb (lower-right), pc (upper-left); the 4th corner is pd = pb + pc - pa
- quad geometry (VAO/VBO/EBO with positions and UVs)
- off-screen render target (FBO) where the virtual environment is rendered,
  then applied as texture on the quad (unlit/emissive)
*/

#pragma once

// Loader for OpenGL extensions
#include <glad/glad.h>

// Loader for GLM
#include <glm/glm.hpp>

// class to manage OpenGL failures
#include <render/gl_check.h>

// render-to-texture and wall shader
#include <render/framebuffer.h>
#include <render/wall_shader.h>

/////////////////// LED WALL class ///////////////////////
class LedWall
{
public:

    //////////////////////////////////////////
    // constructor: corners of the wall, resolution of the render target
    LedWall(const glm::vec3& pa, const glm::vec3& pb, const glm::vec3& pc, int targetWidth, int targetHeight)
        : pa(pa), pb(pb), pc(pc), pd(pb + pc - pa),
          target(targetWidth, targetHeight),
          shader("shaders/wall.vert", "shaders/wall.frag")
    {
        this->SetupQuad();
    }

    // the instance owns OpenGL resources: copies are not allowed
    LedWall(const LedWall&) = delete;
    LedWall& operator=(const LedWall&) = delete;

    //////////////////////////////////////////
    // destructor: we delete the buffers of the quad
    ~LedWall()
    {
        GL_CHECK(glDeleteVertexArrays(1, &this->VAO));
        GL_CHECK(glDeleteBuffers(1, &this->VBO));
        GL_CHECK(glDeleteBuffers(1, &this->EBO));
    }

    //////////////////////////////////////////
    // the following rendering calls are directed to the wall render target
    void BeginCapture()
    {
        this->target.Bind();
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }

    // back to the default framebuffer (the window): the viewport must be set again by the caller
    void EndCapture()
    {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    //////////////////////////////////////////
    // rendering of the wall quad, textured with the content of the render target
    void Draw(const glm::mat4& view, const glm::mat4& projection)
    {
        // the vertices are already in world coordinates
        this->shader.Begin(glm::mat4(1.0f), view, projection, this->target.GetColorTexture());

        GL_CHECK(glBindVertexArray(this->VAO));
        GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
        GL_CHECK(glBindVertexArray(0));
    }

    //////////////////////////////////////////
    // aspect ratio of the render target (same as the wall: 6x3 m -> 2:1)
    GLfloat GetAspect() const { return (GLfloat)this->target.GetWidth() / (GLfloat)this->target.GetHeight(); }

    // corners of the wall in world coordinates
    const glm::vec3& GetPa() const { return this->pa; }
    const glm::vec3& GetPb() const { return this->pb; }
    const glm::vec3& GetPc() const { return this->pc; }
    const glm::vec3& GetPd() const { return this->pd; }

private:
    // corners of the wall
    glm::vec3 pa, pb, pc, pd;

    // off-screen target for the virtual environment
    RenderTarget target;
    WallShader shader;

    // buffer objects\arrays of the quad
    GLuint VAO = 0, VBO = 0, EBO = 0;

    //////////////////////////////////////////
    // creation of the quad: interleaved positions (location 0) and UVs (location 2)
    void SetupQuad()
    {
        GLfloat vertices[] =
        {
            this->pa.x, this->pa.y, this->pa.z,  0.0f, 0.0f,    // pa: lower-left
            this->pb.x, this->pb.y, this->pb.z,  1.0f, 0.0f,    // pb: lower-right
            this->pc.x, this->pc.y, this->pc.z,  0.0f, 1.0f,    // pc: upper-left
            this->pd.x, this->pd.y, this->pd.z,  1.0f, 1.0f     // pd: upper-right
        };
        GLuint indices[] =
        {
            0, 1, 3,  // First Triangle (pa, pb, pd)
            0, 3, 2   // Second Triangle (pa, pd, pc)
        };

        GL_CHECK(glGenVertexArrays(1, &this->VAO));
        GL_CHECK(glGenBuffers(1, &this->VBO));
        GL_CHECK(glGenBuffers(1, &this->EBO));

        // VAO is made "active"
        GL_CHECK(glBindVertexArray(this->VAO));
        // we copy data in the VBO - we must set the data dimension, and the pointer to the structure containing the data
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->VBO));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
        // we copy data in the EBO - we must set the data dimension, and the pointer to the structure containing the data
        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO));
        GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

        // vertex positions (location 0) and UVs (location 2)
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0));

        GL_CHECK(glEnableVertexAttribArray(2));
        GL_CHECK(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))));

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GL_CHECK(glBindVertexArray(0));
    }
};
