/*
LedWall class
- planar rectangular LED wall, defined by 3 corners in world coordinates:
  pa (lower-left), pb (lower-right), pc (upper-left); the 4th corner is pd = pb + pc - pa
- quad geometry (VAO/VBO/EBO with positions and UVs)
- off-screen render target (FBO) where the virtual environment is rendered,
  then applied as texture on the quad (unlit/emissive)
- off-axis projection of the wall from the camera position (the last valid one is kept)
- outer frustum: second, low resolution render target, rendered from a fixed eye at the centre of the stage
*/

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <render/gl_check.h>
#include <render/framebuffer.h>
#include <render/wall_shader.h>
#include <render/offaxis.h>
#include <core/config.h>

/////////////////// LED WALL class ///////////////////////
class LedWall
{
public:

    // constructor: corners of the wall, resolutions of the inner and outer render targets
    LedWall(const glm::vec3& pa, const glm::vec3& pb, const glm::vec3& pc, int targetWidth, int targetHeight, int outerWidth, int outerHeight)
        : pa(pa), pb(pb), pc(pc), pd(pb + pc - pa),
          target(targetWidth, targetHeight), outerTarget(outerWidth, outerHeight),
          shader(WALL_VERT_PATH, WALL_FRAG_PATH)
    {
        this->SetupQuad();

        // the outer eye does not move: its off-axis projection is computed once
        this->outerEyeMatrices = OffAxisProjection(this->pa, this->pb, this->pc, config::OUTER_EYE_POSITION, NEAR_PLANE, FAR_PLANE);
    }

    LedWall(const LedWall&) = delete;
    LedWall& operator=(const LedWall&) = delete;

    ~LedWall()
    {
        GL_CHECK(glDeleteVertexArrays(1, &this->VAO));
        GL_CHECK(glDeleteBuffers(1, &this->VBO));
        GL_CHECK(glDeleteBuffers(1, &this->EBO));
    }

    // the following rendering calls are directed to the render target of the given frustum (inner or outer)
    void BeginCapture(WALL_CONTENT content)
    {
        if(content == WALL_CONTENT::OUTER)
            this->outerTarget.Bind();
        else
            this->target.Bind();

        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }

    // back to the default framebuffer (the window): the viewport must be set again by the caller
    void EndCapture()
    {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    // rendering of the wall quad, textured with the inner or the outer render target
    void Draw(const glm::mat4& view, const glm::mat4& projection)
    {
        // the vertices are already in world coordinates; the shader chooses inner or outer per fragment
        this->shader.Begin(glm::mat4(1.0f), view, projection, this->target.GetColorTexture(), this->outerTarget.GetColorTexture(), this->cameraViewProjection, this->content, this->ledPanel);

        GL_CHECK(glBindVertexArray(this->VAO));
        GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
        GL_CHECK(glBindVertexArray(0));
    }

    // rendering of the wall subtracted from the image already in the framebuffer (ground truth - wall):
    // where the off-axis projection is correct, the result is black
    void DrawDifference(const glm::mat4& view, const glm::mat4& projection)
    {
        GL_CHECK(glEnable(GL_BLEND));
        // full colours, no alpha weights
        GL_CHECK(glBlendFunc(GL_ONE, GL_ONE));
        // result = destination - source
        GL_CHECK(glBlendEquation(GL_FUNC_REVERSE_SUBTRACT));
        this->Draw(view, projection);

        // we restore the default blending state for the next draws
        GL_CHECK(glBlendEquation(GL_FUNC_ADD));
        GL_CHECK(glDisable(GL_BLEND));
    }

    // aspect ratio of the render target (same as the wall: 6x3 m -> 2:1)
    GLfloat GetAspect() const { return (GLfloat)this->target.GetWidth() / (GLfloat)this->target.GetHeight(); }

    // update of the off-axis projection from the eye position:
    // if the eye is not in front of the wall, we keep the last valid projection
    void UpdateEye(const glm::vec3& eye, float nearPlane, float farPlane)
    {
        OffAxisMatrices updatedEyeMatrices = OffAxisProjection(this->pa, this->pb, this->pc, eye, nearPlane, farPlane);

        if(updatedEyeMatrices.isValid)
            eyeMatrices = updatedEyeMatrices;
    }

    OffAxisMatrices GetEyeMatrices() const { return this->eyeMatrices; }

    OffAxisMatrices GetOuterEyeMatrices() const { return this->outerEyeMatrices; }

    // light emitted by the wall: average colour of the outer frustum, travelling along the wall normal
    glm::vec3 GetEmittedColor() { return this->outerTarget.GetAverageColor(); }
    glm::vec3 GetNormal() const { return glm::normalize(glm::cross(this->pb - this->pa, this->pc - this->pa)); }

    WALL_CONTENT GetContent() const        { return this->content; }
    bool GetLedPanel()        const        { return this->ledPanel; }

    const glm::vec3& GetPa() const { return this->pa; }
    const glm::vec3& GetPb() const { return this->pb; }
    const glm::vec3& GetPc() const { return this->pc; }
    const glm::vec3& GetPd() const { return this->pd; }

    void SetContent(WALL_CONTENT content)  { this->content = content; }
    void SetLedPanel(bool ledPanel)        { this->ledPanel = ledPanel; }
    void SetCameraViewProjection(const glm::mat4& viewProjection) { this->cameraViewProjection = viewProjection; }

private:
    glm::vec3 pa, pb, pc, pd;

    // last valid off-axis view and projection (inner frustum, from the cine camera)
    OffAxisMatrices eyeMatrices;
    // off-axis view and projection from the fixed outer eye
    OffAxisMatrices outerEyeMatrices;
    // projection * view of the cine camera: it decides which part of the wall is in the inner frustum
    glm::mat4 cameraViewProjection = glm::mat4(1.0f);

    WALL_CONTENT content = WALL_CONTENT::COMPOSITE;

    bool ledPanel = false;

    // off-screen target for the virtual environment (inner frustum)
    RenderTarget target;
    // low resolution target for the outer frustum
    RenderTarget outerTarget;
    WallShader shader;

    GLuint VAO = 0, VBO = 0, EBO = 0;

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

        GL_CHECK(glBindVertexArray(this->VAO));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->VBO));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
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
