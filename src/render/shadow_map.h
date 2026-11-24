/*
ShadowMap class
- depth-only FBO: the scene seen from the set light, used to compute the shadows
- light space matrix (orthographic projection, directional light)
*/

#pragma once

// Loader for OpenGL extensions
#include <glad/glad.h>

// Loader for GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

// named constants of the application
#include <core/config.h>
// class to manage OpenGL failures
#include <render/gl_check.h>

/////////////////// SHADOWMAP class ///////////////////////
class ShadowMap
{
public:

    // constructor: square depth texture of the given resolution, attached to a depth-only FBO
    ShadowMap(int size)
    {
        this->size = size;

        // depth texture: for each texel, the depth of the closest surface seen from the light
        GL_CHECK(glGenTextures(1, &this->depthTexture));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, this->depthTexture));
        // we allocate the memory only (nullptr): the texture is filled by the depth pass
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, this->size, this->size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));

        // we read exactly one texel, without interpolation between depths
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        // outside the map we return the border colour
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));

        // border depth = 1 (far plane): the points outside the light volume are never in shadow
        GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GL_CHECK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

        // FBO with the depth texture as depth attachment
        GL_CHECK(glGenFramebuffers(1, &this->FBO));
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, this->FBO));
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depthTexture, 0));
        // depth-only FBO: no colour buffer to write or read
        GL_CHECK(glDrawBuffer(GL_NONE));
        GL_CHECK(glReadBuffer(GL_NONE));

        GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "FrameBuffer incomplete! Status: " << framebufferStatus << std::endl;

        // we go back to the default framebuffer
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    // the instance owns OpenGL resources: copies are not allowed
    ShadowMap(const ShadowMap&) = delete;
    ShadowMap& operator=(const ShadowMap&) = delete;

    ~ShadowMap()
    {
        GL_CHECK(glDeleteFramebuffers(1, &this->FBO));
        GL_CHECK(glDeleteTextures(1, &this->depthTexture));
    }

    // light space matrix: from world coordinates to the clip space of the light
    void UpdateLight(const glm::vec3& lightPos, const glm::vec3& target)
    {
        // the light looks at the target. If the light is almost vertical, dir and WORLD_Y are parallel:
        // their cross product is zero and lookAt returns NaN, so we use WORLD_Z as up
        glm::vec3 up = config::WORLD_Y;
        glm::vec3 dir = glm::normalize(target - lightPos);
        if(glm::abs(glm::dot(dir, config::WORLD_Y)) > LIGHT_PARALLEL_UP_DOT)
            up = config::WORLD_Z;

        glm::mat4 lightView = glm::lookAt(lightPos, target, up);

        // orthographic projection: parallel rays, i.e. a directional light. The volume encloses the whole stage
        glm::mat4 lightProjection = glm::ortho(-SHADOW_ORTHO_HALF_SIZE, SHADOW_ORTHO_HALF_SIZE, -SHADOW_ORTHO_HALF_SIZE, SHADOW_ORTHO_HALF_SIZE, SHADOW_NEAR, SHADOW_FAR);

        this->lightSpaceMatrix = lightProjection * lightView;
    }

    void BeginCapture()
    {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, this->FBO));
        GL_CHECK(glViewport(0, 0, this->size, this->size));
        GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));
    }

    void EndCapture()
    {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    GLuint    GetDepthTexture()     const { return this->depthTexture; }
    glm::mat4 GetLightSpaceMatrix() const { return this->lightSpaceMatrix; }

private:

    GLuint FBO = 0, depthTexture = 0;
    // resolution of the (square) shadow map
    int size = 0;
    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
};
