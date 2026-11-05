/*
OpenGL coordinate system (right-handed)
positive X axis points right
positive Y axis points up
positive Z axis points "outside" the screen


                              Y
                              |
                              |
                              |________X
                             /
                            /
                           /
                          Z
*/

// Std. Includes
#include <string>

// Loader for OpenGL extensions
#include <glad/glad.h>

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// class to manage shaders 
//#include <utils/shader.h>

// class to manage OpenGL failures
#include "gl_check.h"

// dimensions of application's window
GLuint screenWidth = 1920, screenHeight = 1080;

// callback function for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void fb_size_callback(GLFWwindow* window, int width, int height);

/////////////////// MAIN function ///////////////////////
int main()
{
    // initialization of OpenGL context using GLFW
    int glfwStatus = glfwInit();
    if(glfwStatus == GLFW_FALSE)
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // OpenGL 4.1 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // we set if the window is resizable
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // creation of the application's window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "virtual-led-volume", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // relate window and the callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, fb_size_callback);

    // GLAD tries to load the context set by GLFW
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL version: " << (const char*)glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer: " << (const char*)glGetString(GL_RENDERER) << std::endl;
    std::cout << "GLSL version: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    GL_CHECK(glViewport(0, 0, width, height));

    // enable Z test
    GL_CHECK(glEnable(GL_DEPTH_TEST));

    /////////////////// RENDERING LOOP ///////////////////////
    // this code is executed at each frame
    while(!glfwWindowShouldClose(window))
    {
        // Check is an I/O event is happening
        glfwPollEvents();

        // "clear" the frame and z-buffer
        GL_CHECK(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        // double buffering: swapping back and front buffers
        glfwSwapBuffers(window);
    }

    // close and delete context
    glfwTerminate();
    return 0;
}

/////////////////// CALLBACK FOR KEYBOARD EVENTS ///////////////////////
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // if ESC is pressed, we close the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void fb_size_callback(GLFWwindow* window, int width, int height)
{
    GL_CHECK(glViewport(0, 0, width, height));
}
