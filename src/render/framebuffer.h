#pragma once

#include <glad/glad.h>
#include "gl_check.h"

class RenderTarget
{
private:

    GLuint FBO = 0;                     //framebuffer ID
    GLuint colorTexture = 0;            // image texture
    GLuint depthRenderbuffer = 0;       // depth buffer (for FBO depth test)
    int width = 0, height = 0;          // resolution

    void Create(int width, int height)
    {
        this->width = width;
        this->height = height;

        GL_CHECK(glGenFramebuffers(1, &FBO));
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, FBO));

        GL_CHECK(glGenTextures(1, &colorTexture));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, colorTexture));
        
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
        
        // set how to consider UVs outside [0,1] range
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        // set the filtering for minification and magnification
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        // set the binding to 0 once we have finished
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0));
    
        GL_CHECK(glGenRenderbuffers(1, &depthRenderbuffer));
        GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer));
        GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height));
        GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer));

        GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "FrameBuffer incomplete! Status: " << framebufferStatus << std::endl;

        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    void Free()
    {
        if(FBO)
        {
            GL_CHECK(glDeleteFramebuffers(1, &FBO));
            GL_CHECK(glDeleteTextures(1, &colorTexture));
            GL_CHECK(glDeleteRenderbuffers(1, &depthRenderbuffer));
        }

        FBO = 0;
        colorTexture = 0;
        depthRenderbuffer = 0;
    }

public:

    // Copy and move are disabled: otherwise two copies would own and delete the same FBO
    RenderTarget(const RenderTarget& copy) = delete; //disallow copy
    RenderTarget& operator=(const RenderTarget&) = delete;

    RenderTarget(RenderTarget&& move) = delete; //disallow move
    RenderTarget& operator=(RenderTarget&&) = delete;

    RenderTarget() = default;

    RenderTarget(int width, int height)
    {
        Create(width, height);
    }

    ~RenderTarget()
    {
        Free();
    }

    void Bind()
    {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, FBO));
        GL_CHECK(glViewport(0, 0, width, height));
    }

    void Resize(int width, int height)
    {
        Free();
        Create(width, height);
    }

    // ---- GETTERS ----
    GLuint GetColorTexture() const  { return colorTexture; }
    int GetWidth() const            { return width; }
    int GetHeight() const           { return height; }
};