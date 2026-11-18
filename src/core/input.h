/*
Input class
- keyboard and mouse callbacks of the application
- state of the pressed keys
- control target (Tab): the observer camera or the cine camera receives WASD + mouse (+ E/Q pedestal and scroll zoom for the cine camera)
- right mouse button: the active target looks again at the subject
- view mode of the application (keys 1..5: split / cine camera / stage / ground truth / difference)
- UI mode (G): free cursor, mouse, wheel and buttons are left to the GUI and do not move the cameras

N.B.) GLFW accepts only free (or static) functions as callbacks: we store the pointer to the Input instance
inside the window (glfwSetWindowUserPointer), and the static callbacks retrieve it to call the member functions
*/

#pragma once

// std::pair for the key bindings table of the cine camera
#include <utility>
// console output of the current state
#include <iostream>

// Loader for OpenGL extensions (it must be included before GLFW)
#include <glad/glad.h>

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// class to manage the FPS camera
#include <utils/camera.h>

// class to manage the cine camera
#include <scene/cine_camera.h>

// named constants of the application
#include <core/config.h>

// target of the WASD + mouse controls (Tab toggles it)
enum class CONTROL_TARGET
{
    OBSERVER,
    CAMERA
};

// view modes of the application
enum class VIEW_MODE
{
    SPLIT,
    CINE_CAMERA,
    STAGE,
    GROUND_TRUTH,
    DIFFERENCE
};

/////////////////// INPUT class ///////////////////////
class Input
{
public:

    // constructor: the instance is related to the window, and the callbacks are set
    Input(GLFWwindow* window, Camera& observer, CineCamera& camera) : observer(observer), camera(camera)
    {
        // we store the pointer to this instance inside the window, to retrieve it in the static callbacks
        glfwSetWindowUserPointer(window, this);

        // relate window and the callbacks
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetCursorPosCallback(window, MouseCallback);
        glfwSetScrollCallback(window, ScrollCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);

        // we disable the mouse cursor
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    // the window stores the address of this instance: copies are not allowed
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    // the pressed keys move the active control target
    void ApplyMovements(GLfloat deltaTime)
    {
        if(controlTarget == CONTROL_TARGET::OBSERVER)
            MoveObserver(deltaTime);
        else
            MoveCamera(deltaTime);
    }

    // current view mode
    VIEW_MODE GetViewMode() const { return this->viewMode; }

    // true if the UI mode is active (free cursor)
    bool IsUIMode() const { return this->uiMode; }

private:
    // observer camera: free camera walking in the stage
    Camera& observer;

    // cine camera: the tracked camera filming the wall
    CineCamera& camera;

    // we keep trace of the pressed keys
    bool keys[MAX_KEYS] = {};

    // we need to store the previous mouse position to calculate the offset with the current frame
    GLfloat lastX = 0.0f, lastY = 0.0f;
    // when rendering the first frame, we do not have a "previous state" for the mouse, so we need to manage this situation
    bool firstMouse = true;

    // current view mode
    VIEW_MODE viewMode = VIEW_MODE::SPLIT;

    // UI mode: free cursor, the mouse goes to the GUI and not to the cameras
    bool uiMode = false;

    // camera that receives WASD + mouse (the observer at start)
    CONTROL_TARGET controlTarget = CONTROL_TARGET::OBSERVER;

    // WASD moves the observer (the code is in utils/camera.h)
    void MoveObserver(GLfloat deltaTime)
    {
        // if a single WASD key is pressed, then we will apply the full value of velocity v in the corresponding direction.
        // However, if two keys are pressed together in order to move diagonally (W+D, W+A, S+D, S+A),
        // then the camera will apply a compensation factor to the velocities applied in the single directions,
        // in order to have the full v applied in the diagonal direction
        // the XOR on A and D is to avoid the application of a wrong attenuation in the case W+A+D or S+A+D are pressed together.
        GLboolean diagonal_movement = (keys[GLFW_KEY_W] ^ keys[GLFW_KEY_S]) && (keys[GLFW_KEY_A] ^ keys[GLFW_KEY_D]);
        observer.SetMovementCompensation(diagonal_movement);

        if(keys[GLFW_KEY_W])
            observer.ProcessKeyboard(FORWARD, deltaTime);
        if(keys[GLFW_KEY_S])
            observer.ProcessKeyboard(BACKWARD, deltaTime);
        if(keys[GLFW_KEY_A])
            observer.ProcessKeyboard(LEFT, deltaTime);
        if(keys[GLFW_KEY_D])
            observer.ProcessKeyboard(RIGHT, deltaTime);
    }

    // WASD + E/Q move the cine camera (the code is in scene/cine_camera.h)
    void MoveCamera(GLfloat deltaTime)
    {
        // table key -> move: to change a key, only one line has to be changed
        static const std::pair<int, CINE_MOVEMENT> bindings[] = { {GLFW_KEY_W, CINE_MOVEMENT::DOLLY_IN},    {GLFW_KEY_S, CINE_MOVEMENT::DOLLY_OUT},
                                                                  {GLFW_KEY_A, CINE_MOVEMENT::TRUCK_LEFT},  {GLFW_KEY_D, CINE_MOVEMENT::TRUCK_RIGHT},
                                                                  {GLFW_KEY_E, CINE_MOVEMENT::PEDESTAL_UP}, {GLFW_KEY_Q, CINE_MOVEMENT::PEDESTAL_DOWN} };
        for(const auto& [key, movement] : bindings)
            if(keys[key])
                camera.ProcessKeyboard(movement, deltaTime);
    }

    // static callbacks: they retrieve the Input instance stored in the window and call its member functions
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
    {
        Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
        input->OnKey(window, key, action);
    }

    static void MouseCallback(GLFWwindow* window, double xpos, double ypos)
    {
        Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
        input->OnMouseMove(xpos, ypos);
    }

    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
        input->OnScroll(yoffset);
    }

    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
        input->OnMouseButton(button, action);
    }

    // keyboard events
    void OnKey(GLFWwindow* window, int key, int action)
    {
        // if ESC is pressed, close the application
        if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

        // if Tab is pressed, WASD + mouse switch to the other camera
        if(key == GLFW_KEY_TAB && action == GLFW_PRESS)
        {
            controlTarget = (controlTarget == CONTROL_TARGET::OBSERVER) ? CONTROL_TARGET::CAMERA : CONTROL_TARGET::OBSERVER;
            PrintState();
        }

        // if G is pressed, we switch between UI mode (free cursor) and camera mode (captured cursor)
        if(key == GLFW_KEY_G && action == GLFW_PRESS)
        {
            uiMode = !uiMode;

            glfwSetInputMode(window, GLFW_CURSOR, uiMode ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

            // back to camera mode: the first offset starts again from zero (no jump of the view)
            if(!uiMode)
                firstMouse = true;

            PrintState();
        }

        // keys 1..5 select the view mode directly (split, cine camera, stage, ground truth, difference)
        if(key == GLFW_KEY_1 && action == GLFW_PRESS)
        {
            viewMode = VIEW_MODE::SPLIT;
            PrintState();
        }
        if(key == GLFW_KEY_2 && action == GLFW_PRESS)
        {
            viewMode = VIEW_MODE::CINE_CAMERA;
            PrintState();
        }
        if(key == GLFW_KEY_3 && action == GLFW_PRESS)
        {
            viewMode = VIEW_MODE::STAGE;
            PrintState();
        }
        if(key == GLFW_KEY_4 && action == GLFW_PRESS)
        {
            viewMode = VIEW_MODE::GROUND_TRUTH;
            PrintState();
        }
        if(key == GLFW_KEY_5 && action == GLFW_PRESS)
        {
            viewMode = VIEW_MODE::DIFFERENCE;
            PrintState();
        }

        // we keep trace of the pressed keys
        // with this method, we can manage 2 keys pressed at the same time:
        // many I/O managers often consider only 1 key pressed at the time (the first pressed, until it is released)
        // using a boolean array, we can then check and manage all the keys pressed at the same time
        if(key < 0 || key >= MAX_KEYS)
            return;

        if(action == GLFW_PRESS)
            keys[key] = true;
        else if(action == GLFW_RELEASE)
            keys[key] = false;
    }

    // mouse events
    void OnMouseMove(double xpos, double ypos)
    {
        // in UI mode the mouse is used by the GUI
        if(uiMode)
            return;

        // we move the camera view following the mouse cursor
        // we calculate the offset of the mouse cursor from the position in the last frame
        // when rendering the first frame, we do not have a "previous state" for the mouse, so we set the previous state equal to the initial values (thus, the offset will be = 0)
        if(firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        // offset of mouse cursor position
        GLfloat xoffset = xpos - lastX;
        GLfloat yoffset = lastY - ypos;

        // the new position will be the previous one for the next frame
        lastX = xpos;
        lastY = ypos;

        // we pass the offset to the active control target
        if(controlTarget == CONTROL_TARGET::OBSERVER)
            observer.ProcessMouseMovement(xoffset, yoffset);
        else
            camera.ProcessMouseMovement(xoffset, yoffset);
    }

    // mouse button events: the right button re-centres the active target on the subject
    void OnMouseButton(int button, int action)
    {
        if(uiMode)
            return;

        if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        {
            if(controlTarget == CONTROL_TARGET::CAMERA)
                camera.LookAt(config::SUBJECT_TARGET);
            else
            {
                glm::vec3 dir = glm::normalize(config::SUBJECT_TARGET - observer.Position);

                // we invert the front formula of the lab camera: front = (cos yaw * cos pitch, sin pitch, sin yaw * cos pitch)
                observer.Pitch = glm::degrees(glm::asin(dir.y));
                observer.Yaw = glm::degrees(glm::atan(dir.z, dir.x));

                // a zero offset only updates the camera vectors (UpdateCameraVectors is private)
                observer.ProcessMouseMovement(0.0f, 0.0f);
            }
        }
    }

    // scroll events: the wheel zooms the cine camera (the observer has a fixed field of view)
    void OnScroll(double yoffset)
    {
        if(uiMode)
            return;

        if(controlTarget == CONTROL_TARGET::CAMERA)
            camera.ProcessScroll(yoffset);
    }

    // it prints the current view mode and control target
    void PrintState() const
    {
        std::cout << "View: " << ToString(viewMode) << " | Controls: " << ToString(controlTarget) << " | UI: "<< (uiMode ? "ON" : "OFF") << std::endl;
    }

    // names of the enum values, for the console output
    static const char* ToString(VIEW_MODE mode)
    {
        switch(mode)
        {
            case VIEW_MODE::SPLIT:        return "SPLIT";
            case VIEW_MODE::CINE_CAMERA:  return "CINE_CAMERA";
            case VIEW_MODE::STAGE:        return "STAGE";
            case VIEW_MODE::GROUND_TRUTH: return "GROUND_TRUTH";
            case VIEW_MODE::DIFFERENCE:   return "DIFFERENCE";
        }
        return "UNKNOWN";
    }

    static const char* ToString(CONTROL_TARGET target)
    {
        switch(target)
        {
            case CONTROL_TARGET::OBSERVER: return "OBSERVER";
            case CONTROL_TARGET::CAMERA:   return "CAMERA";
        }
        return "UNKNOWN";
    }
};
