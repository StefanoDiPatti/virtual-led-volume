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

// we include the library for images loading (through the texture loading functions)
// the implementation of stb_image is compiled only here: the define must come before the first inclusion
#define STB_IMAGE_IMPLEMENTATION
#include <render/texture.h>

// window, OpenGL context and input management
#include <core/window.h>
#include <core/input.h>

// class to manage the FPS camera
#include <utils/camera.h>

// class to manage OpenGL failures
#include <render/gl_check.h>

// scene shader and lighting parameters
#include <render/scene_shader.h>
#include <render/lighting.h>
// off-axis projection for the LED wall
#include <render/offaxis.h>

// objects of the scene: stage, virtual environment, LED wall
#include <scene/stage.h>
#include <scene/environment.h>
#include <scene/led_wall.h>

// Loader for GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/////////////////// MAIN function ///////////////////////
int main()
{
    // the window (and the OpenGL context) is created first, so it is destroyed last,
    // after all the objects owning OpenGL resources
    Window window(1920, 1080, "virtual-led-volume");

    // we create a camera. We pass the initial position as a parameter to the constructor. The last boolean tells that we want a camera "anchored" to the ground
    Camera camera(glm::vec3(0.0f, 1.7f, 6.0f), GL_TRUE);
    // keyboard and mouse callbacks
    Input input(window.Get(), camera);

    // enable Z test
    GL_CHECK(glEnable(GL_DEPTH_TEST));

    // ------- Shaders and Lighting -------
    SceneShader sceneShader("shaders/scene.vert", "shaders/scene.frag");
    Lighting lighting;

    // ------- Scene -------
    Stage stage;
    VirtualEnvironment environment("assets/textures/skybox/");

    // LED wall 6x3 m standing on the floor at z = -3 (pa lower-left, pb lower-right, pc upper-left)
    // the off-screen target has the same 2:1 aspect ratio of the wall
    LedWall wall(glm::vec3(-3.0f, 0.0f, -3.0f), glm::vec3(3.0f, 0.0f, -3.0f), glm::vec3(-3.0f, 3.0f, -3.0f), 2048, 1024);

    // Projection matrix: recomputed every frame from the framebuffer size
    glm::mat4 projection = glm::mat4(1.0f);
    // View matrix: the camera moves, so we just set to identity now
    glm::mat4 view = glm::mat4(1.0f);

    // size of the framebuffer of the window
    int width, height;

    // parameters for time calculation
    GLfloat deltaTime = 0.0f;
    GLfloat lastFrame = 0.0f;

    /////////////////// RENDERING LOOP ///////////////////////
    // this code is executed at each frame
    while(!window.ShouldClose())
    {
        // we determine the time passed from the beginning
        // and we calculate time difference between current frame rendering and the previous one
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check if an I/O event is happening
        window.PollEvents();
        // we apply FPS camera movements
        input.ApplyCameraMovements(deltaTime);
        // View matrix (=camera): position, view direction, camera "up" vector
        view = camera.GetViewMatrix();

        // ------- PASS 1: virtual environment rendered into the wall FBO -------
        // off-axis projection from the camera position (it does not depend on the camera orientation)
        OffAxisMatrices wallCamera = wall.GetOffAxisMatrices(camera.Position, 0.1f, 100.0f);

        wall.BeginCapture();
        environment.Draw(sceneShader, wallCamera.view, wallCamera.projection, lighting);
        wall.EndCapture();

        // ------- PASS 2: screen -------
        window.GetFramebufferSize(width, height);

        // window minimised: zero-sized framebuffer, skip the screen pass
        if(width == 0 || height == 0)
            continue;

        projection = glm::perspective(glm::radians(45.0f), (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
        GL_CHECK(glViewport(0, 0, width, height));

        // "clear" the frame and z-buffer
        GL_CHECK(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        VIEW_MODE mode = input.GetViewMode();
        switch(mode)
        {
            case VIEW_MODE::STAGE:
            {
                // stage view: the physical set and the LED wall showing the content of the FBO
                stage.Draw(sceneShader, view, projection, lighting);
                wall.Draw(view, projection);
                break;
            }
            case VIEW_MODE::GROUND_TRUTH:
            {
                // ground truth: the virtual environment seen directly from the camera
                environment.Draw(sceneShader, view, projection, lighting);
                break;
            }
            case VIEW_MODE::DIFFERENCE:
            {
                // difference: ground truth first, then the wall subtracted from it (black if they match)
                environment.Draw(sceneShader, view, projection, lighting);
                wall.DrawDifference(view, projection);
                break;
            }
            default:
                break;
        }

        // double buffering: swapping back and front buffers
        window.SwapBuffers();
    }

    // when I exit from the graphics loop, it is because the application is closing:
    // the OpenGL resources are released by the destructors, in reverse order of creation (the window is the last one)
    return 0;
}
