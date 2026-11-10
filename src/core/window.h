/*
Window class
- initialization of GLFW and creation of the application's window
- creation of an OpenGL 4.1 Core Profile context, and loading of the OpenGL functions with GLAD

N.B.) the destructor terminates GLFW (and destroys the OpenGL context):
the Window instance must be created before, and destroyed after, every object owning OpenGL resources
(in main, it is the first local variable, so it is the last one to be destroyed)
*/

#pragma once

// Std. Includes
#include <iostream>
#include <cstdlib>

// Loader for OpenGL extensions (it must be included before GLFW)
#include <glad/glad.h>

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

/////////////////// WINDOW class ///////////////////////
class Window
{
public:

    //////////////////////////////////////////
    // constructor: GLFW initialization, window and OpenGL context creation
    Window(int width, int height, const char* title)
    {
        // initialization of OpenGL context using GLFW
        if(glfwInit() == GLFW_FALSE)
        {
            std::cout << "Failed to initialize GLFW" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        // OpenGL 4.1 Core
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        // we set if the window is resizable
        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

        // creation of the application's window
        this->window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if(!this->window)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            std::exit(EXIT_FAILURE);
        }
        glfwMakeContextCurrent(this->window);

        // GLAD tries to load the context set by GLFW
        if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
        {
            std::cout << "Failed to initialize OpenGL context" << std::endl;
            glfwTerminate();
            std::exit(EXIT_FAILURE);
        }

        std::cout << "OpenGL version: " << (const char*)glGetString(GL_VERSION) << std::endl;
        std::cout << "Renderer: " << (const char*)glGetString(GL_RENDERER) << std::endl;
        std::cout << "GLSL version: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    }

    // the window owns the OpenGL context: copies are not allowed
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    //////////////////////////////////////////
    // destructor: close and delete the context
    ~Window()
    {
        glfwTerminate();
    }

    //////////////////////////////////////////
    // GLFW handle of the window (used to set the callbacks)
    GLFWwindow* Get() const { return this->window; }

    // true when the user asked to close the application
    bool ShouldClose() const { return glfwWindowShouldClose(this->window); }

    // check if an I/O event is happening
    void PollEvents() const { glfwPollEvents(); }

    // double buffering: swapping back and front buffers
    void SwapBuffers() const { glfwSwapBuffers(this->window); }

    // size of the framebuffer in pixels (it can differ from the window size on HiDPI screens)
    void GetFramebufferSize(int& width, int& height) const { glfwGetFramebufferSize(this->window, &width, &height); }

private:
    GLFWwindow* window = nullptr;
};
