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
// GUI for the runtime parameters
#include <core/gui.h>
// named constants of the application
#include <core/config.h>

// class to manage the FPS camera
#include <utils/camera.h>

// class to manage OpenGL failures
#include <render/gl_check.h>

// scene shader and lighting parameters
#include <render/scene_shader.h>
#include <render/lighting.h>
// off-axis projection for the LED wall
#include <render/offaxis.h>
#include <render/viewport.h>
// lines for the debug visualisation
#include <render/line_renderer.h>
// depth map of the set light, for the shadows
#include <render/shadow_map.h>

// objects of the scene: stage, virtual environment, LED wall
#include <scene/stage.h>
#include <scene/environment.h>
#include <scene/led_wall.h>
#include <scene/cine_camera.h>

// Loader for GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/////////////////// MAIN function ///////////////////////
int main()
{
    // the window (and the OpenGL context) is created first, so it is destroyed last,
    // after all the objects owning OpenGL resources
    Window window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);

    Camera camera(config::OBSERVER_START_POSITION, GL_TRUE);

    // cine camera on the stage, looking at the wall
    CineCamera cineCamera(config::CINE_START_POSITION, CINE_START_SENSOR);
    std::cout << "Cine camera: " << cineCamera.GetFocalLength() << "mm, vertical FOV: " << glm::degrees(cineCamera.GetVerticalFov()) << "°" << std::endl;

    // keyboard and mouse callbacks
    Input input(window.Get(), camera, cineCamera);

    // GUI, created after Input so that ImGui chains our callbacks
    Gui gui(window.Get());

    GL_CHECK(glEnable(GL_DEPTH_TEST));

    // ------- Shaders and Lighting -------
    SceneShader sceneShader(SCENE_VERT_PATH, SCENE_FRAG_PATH);
    DepthShader depthShader(DEPTH_VERT_PATH, DEPTH_FRAG_PATH);
    Lighting lighting;
    // shadow map of the set light
    ShadowMap shadowMap(SHADOW_MAP_SIZE);

    // ------- Scene -------
    Stage stage;
    VirtualEnvironment environment(SKYBOX_FOLDER);

    // LED wall 6x3 m standing on the floor at z = -3 (pa lower-left, pb lower-right, pc upper-left)
    // the off-screen target has the same 2:1 aspect ratio of the wall
    LedWall wall(config::WALL_LOWER_LEFT, config::WALL_LOWER_RIGHT, config::WALL_UPPER_LEFT, WALL_RT_WIDTH, WALL_RT_HEIGHT, WALL_OUTER_RT_WIDTH, WALL_OUTER_RT_HEIGHT);

    // groups of debug lines to draw (changed by the GUI panel)
    LineToggles lineToggles;

    // debug lines, drawn only in the observer views
    LineRenderer lines(LINE_MAX_VERTICES);

    // Projection matrix: recomputed every frame from the framebuffer size
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);

    int width, height;

    GLfloat deltaTime = 0.0f;
    GLfloat lastFrame = 0.0f;

    /////////////////// RENDERING LOOP ///////////////////////
    while(!window.ShouldClose())
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        window.PollEvents();
        // we apply the movements of the active control target (observer or cine camera)
        input.ApplyMovements(deltaTime);
        view = camera.GetViewMatrix();

        // the light position is a parameter that can change at runtime: we update its matrix every frame
        shadowMap.UpdateLight(lighting.lightPosition, config::SHADOW_TARGET);

        // ------- PASS 0: depth of the stage seen from the set light -------
        shadowMap.BeginCapture();
        stage.DrawDepth(depthShader, shadowMap.GetLightSpaceMatrix());
        shadowMap.EndCapture();

        // ------- PASS 1: virtual environment rendered into the wall FBO -------
        // off-axis projection from the cine camera position (it does not depend on its orientation and focal length)
        // near plane small and fixed: all the virtual objects are behind the wall
        wall.UpdateEye(cineCamera.GetPosition(), NEAR_PLANE, FAR_PLANE);

        wall.BeginCapture(WALL_CONTENT::INNER);
        environment.Draw(sceneShader, wall.GetEyeMatrices().view, wall.GetEyeMatrices().projection);
        wall.EndCapture();

        // ------- PASS 1b: virtual environment rendered into the outer frustum target, from the fixed eye -------
        wall.BeginCapture(WALL_CONTENT::OUTER);
        environment.Draw(sceneShader, wall.GetOuterEyeMatrices().view, wall.GetOuterEyeMatrices().projection);
        wall.EndCapture();

        // the wall lights the stage with the average colour of the outer frustum
        lighting.wallLightColor = wall.GetEmittedColor();
        lighting.wallLightDirection = wall.GetNormal();

        // ------- PASS 2: screen -------
        window.GetFramebufferSize(width, height);

        // window minimised: zero-sized framebuffer, skip the screen pass
        if(width == 0 || height == 0)
            continue;

        // the GUI uses the mouse only in UI mode (set before the new frame)
        gui.SetMouseEnabled(input.IsUIMode());

        // new GUI frame (after the minimised-window check: every NewFrame needs its Render)
        gui.BeginFrame();

        projection = glm::perspective(glm::radians(OBSERVER_FOV), (GLfloat)width / (GLfloat)height, NEAR_PLANE, FAR_PLANE);
        GL_CHECK(glViewport(0, 0, width, height));

        GL_CHECK(glClearColor(config::BACKGROUND_COLOR.r, config::BACKGROUND_COLOR.g, config::BACKGROUND_COLOR.b, 1.0f));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        // view and projection of the cine camera (what the film camera records)
        glm::mat4 cineView = cineCamera.GetViewMatrix();
        glm::mat4 cineProjection = cineCamera.GetProjectionMatrix(NEAR_PLANE, FAR_PLANE);

        // the wall composes inner and outer frustum from the cine camera of this frame
        wall.SetCameraViewProjection(cineProjection * cineView);

        // debug lines of the frame: world axes (X red, Y green, Z blue, 1 m long)
        if(lineToggles.worldAxes)
            lines.AddAxes(config::WORLD_ORIGIN, config::WORLD_X, config::WORLD_Y, config::WORLD_Z, WORLD_AXES_LENGTH);

        // outline of the LED wall (yellow)
        if(lineToggles.wallFrame)
        {
            lines.AddLine(wall.GetPa(), wall.GetPb(), config::WALL_FRAME_COLOR);
            lines.AddLine(wall.GetPb(), wall.GetPd(), config::WALL_FRAME_COLOR);
            lines.AddLine(wall.GetPd(), wall.GetPc(), config::WALL_FRAME_COLOR);
            lines.AddLine(wall.GetPc(), wall.GetPa(), config::WALL_FRAME_COLOR);
        }

        // frustum of the cine camera (cyan), with a short far plane used only for the visualisation
        if(lineToggles.cineFrustum)
        {
            std::array<glm::vec3, 8> frustumCorners = cineCamera.GetFrustumCorners(NEAR_PLANE, FRUSTUM_DRAW_FAR);
            lines.AddBox(frustumCorners, config::CINE_FRUSTUM_COLOR);
        }

        // gizmo of the cine camera: right (red), up (green), viewing direction (blue)
        if(lineToggles.cameraGizmo)
            lines.AddAxes(cineCamera.GetPosition(), cineCamera.GetRight(), cineCamera.GetUp(), cineCamera.GetFront(), GIZMO_LENGTH);

        // off-axis frustum of the wall (magenta): from the eye to the wall corners, it depends only on the camera position
        if(lineToggles.offAxisLines)
        {
            lines.AddLine(cineCamera.GetPosition(), wall.GetPa(), config::OFFAXIS_FRUSTUM_COLOR);
            lines.AddLine(cineCamera.GetPosition(), wall.GetPb(), config::OFFAXIS_FRUSTUM_COLOR);
            lines.AddLine(cineCamera.GetPosition(), wall.GetPc(), config::OFFAXIS_FRUSTUM_COLOR);
            lines.AddLine(cineCamera.GetPosition(), wall.GetPd(), config::OFFAXIS_FRUSTUM_COLOR);
        }

        // widgets of the panel: a change of view is applied in this same frame (the GUI is rendered after the views)
        gui.DrawPanel(cineCamera, input, lineToggles, lighting, environment.GetLighting(), wall);

        VIEW_MODE mode = input.GetViewMode();
        switch(mode)
        {
            case VIEW_MODE::SPLIT:
            {
                // two halves of the window: cine camera on the left, observer on the right
                ViewportRect leftArea = {0, 0, width / 2, height};
                ViewportRect rightArea = {width / 2, 0, width - width / 2, height};

                // the whole window is cleared to black (the letterbox bands)
                GL_CHECK(glClearColor(config::LETTERBOX_COLOR.r, config::LETTERBOX_COLOR.g, config::LETTERBOX_COLOR.b, 1.0f));
                GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

                // left: cine camera view, fitted to the sensor aspect ratio inside the half
                ViewportRect camRect = FitAspect(leftArea, cineCamera.GetAspect());
                ClearArea(camRect, config::BACKGROUND_COLOR);
                GL_CHECK(glViewport(camRect.x, camRect.y, camRect.width, camRect.height));

                stage.Draw(sceneShader, cineView, cineProjection, lighting, shadowMap);
                wall.Draw(cineView, cineProjection);

                // right: observer view from the free camera, looking at the stage from outside
                ClearArea(rightArea, config::BACKGROUND_COLOR);
                GL_CHECK(glViewport(rightArea.x, rightArea.y, rightArea.width, rightArea.height));

                // aspect ratio of the half, not of the window (otherwise the image is stretched)
                glm::mat4 observerProjection = glm::perspective(glm::radians(OBSERVER_FOV), (float)rightArea.width / (float)rightArea.height, NEAR_PLANE, FAR_PLANE);

                stage.Draw(sceneShader, view, observerProjection, lighting, shadowMap);
                wall.Draw(view, observerProjection);
                lines.Draw(view, observerProjection);
                break;
            }
            case VIEW_MODE::CINE_CAMERA:
            {
                // area of the window with the aspect ratio of the sensor (the image is not stretched)
                ViewportRect rect = FitAspect({0, 0, width, height}, cineCamera.GetAspect());

                // letterbox: the whole window is cleared to black (the bands)
                GL_CHECK(glClearColor(config::LETTERBOX_COLOR.r, config::LETTERBOX_COLOR.g, config::LETTERBOX_COLOR.b, 1.0f));
                GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

                // we clear only the camera area (glClear ignores the viewport)
                ClearArea(rect, config::BACKGROUND_COLOR);

                GL_CHECK(glViewport(rect.x, rect.y, rect.width, rect.height));

                stage.Draw(sceneShader, cineView, cineProjection, lighting, shadowMap);
                wall.Draw(cineView, cineProjection);
                break;
            }
            case VIEW_MODE::STAGE:
            {
                stage.Draw(sceneShader, view, projection, lighting, shadowMap);
                wall.Draw(view, projection);
                lines.Draw(view, projection);
                break;
            }
            case VIEW_MODE::GROUND_TRUTH:
            {
                // ground truth: the virtual environment seen directly from the cine camera
                environment.Draw(sceneShader, cineView, cineProjection);
                break;
            }
            case VIEW_MODE::DIFFERENCE:
            {
                // difference: ground truth first, then the wall subtracted from it (black if they match)
                environment.Draw(sceneShader, cineView, cineProjection);
                wall.DrawDifference(cineView, cineProjection);
                break;
            }
            default:
                break;
        }

        // the GUI is drawn last, on top of every view
        gui.Render();

        // the debug lines are added again at every frame
        lines.Clear();

        window.SwapBuffers();
    }

    // when I exit from the graphics loop, it is because the application is closing:
    // the OpenGL resources are released by the destructors, in reverse order of creation (the window is the last one)
    return 0;
}
