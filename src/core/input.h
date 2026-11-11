/*
Input class
- keyboard and mouse callbacks of the application
- state of the pressed keys, WASD + mouse movements applied to the camera
- application toggles (E: stage view / virtual environment view)

N.B.) GLFW accepts only free (or static) functions as callbacks: we store the pointer to the Input instance
inside the window (glfwSetWindowUserPointer), and the static callbacks retrieve it to call the member functions
*/

#pragma once

// Loader for OpenGL extensions (it must be included before GLFW)
#include <glad/glad.h>

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// class to manage the FPS camera
#include <utils/camera.h>

/////////////////// INPUT class ///////////////////////
class Input
{
public:

    // constructor: the instance is related to the window, and the callbacks are set
    Input(GLFWwindow* window, Camera& camera) : camera(camera)
    {
        // we store the pointer to this instance inside the window, to retrieve it in the static callbacks
        glfwSetWindowUserPointer(window, this);

        // relate window and the callbacks
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetCursorPosCallback(window, MouseCallback);

        // we disable the mouse cursor
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    // the window stores the address of this instance: copies are not allowed
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    // If one of the WASD keys is pressed, the camera is moved accordingly (the code is in utils/camera.h)
    void ApplyCameraMovements(GLfloat deltaTime)
    {
        // if a single WASD key is pressed, then we will apply the full value of velocity v in the corresponding direction.
        // However, if two keys are pressed together in order to move diagonally (W+D, W+A, S+D, S+A),
        // then the camera will apply a compensation factor to the velocities applied in the single directions,
        // in order to have the full v applied in the diagonal direction
        // the XOR on A and D is to avoid the application of a wrong attenuation in the case W+A+D or S+A+D are pressed together.
        GLboolean diagonal_movement = (this->keys[GLFW_KEY_W] ^ this->keys[GLFW_KEY_S]) && (this->keys[GLFW_KEY_A] ^ this->keys[GLFW_KEY_D]);
        this->camera.SetMovementCompensation(diagonal_movement);

        if(this->keys[GLFW_KEY_W])
            this->camera.ProcessKeyboard(FORWARD, deltaTime);
        if(this->keys[GLFW_KEY_S])
            this->camera.ProcessKeyboard(BACKWARD, deltaTime);
        if(this->keys[GLFW_KEY_A])
            this->camera.ProcessKeyboard(LEFT, deltaTime);
        if(this->keys[GLFW_KEY_D])
            this->camera.ProcessKeyboard(RIGHT, deltaTime);
    }

    // true if the virtual environment must be shown directly (ground truth view), false for the stage view
    bool ShowEnvironment() const { return this->showEnvironment; }

private:
    // camera moved by the keyboard and mouse events
    Camera& camera;

    // we keep trace of the pressed keys
    bool keys[1024] = {};

    // we need to store the previous mouse position to calculate the offset with the current frame
    GLfloat lastX = 0.0f, lastY = 0.0f;
    // when rendering the first frame, we do not have a "previous state" for the mouse, so we need to manage this situation
    bool firstMouse = true;

    // stage view / virtual environment view
    bool showEnvironment = false;

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

    // keyboard events
    void OnKey(GLFWwindow* window, int key, int action)
    {
        // if ESC is pressed, close the application
        if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

        // if E is pressed, we alternate stage view and virtual environment view
        if(key == GLFW_KEY_E && action == GLFW_PRESS)
            this->showEnvironment = !this->showEnvironment;

        // we keep trace of the pressed keys
        // with this method, we can manage 2 keys pressed at the same time:
        // many I/O managers often consider only 1 key pressed at the time (the first pressed, until it is released)
        // using a boolean array, we can then check and manage all the keys pressed at the same time
        if(key < 0 || key >= 1024)
            return;

        if(action == GLFW_PRESS)
            this->keys[key] = true;
        else if(action == GLFW_RELEASE)
            this->keys[key] = false;
    }

    // mouse events
    void OnMouseMove(double xpos, double ypos)
    {
        // we move the camera view following the mouse cursor
        // we calculate the offset of the mouse cursor from the position in the last frame
        // when rendering the first frame, we do not have a "previous state" for the mouse, so we set the previous state equal to the initial values (thus, the offset will be = 0)
        if(this->firstMouse)
        {
            this->lastX = xpos;
            this->lastY = ypos;
            this->firstMouse = false;
        }

        // offset of mouse cursor position
        GLfloat xoffset = xpos - this->lastX;
        GLfloat yoffset = this->lastY - ypos;

        // the new position will be the previous one for the next frame
        this->lastX = xpos;
        this->lastY = ypos;

        // we pass the offset to the Camera class instance in order to update the rendering
        this->camera.ProcessMouseMovement(xoffset, yoffset);
    }
};
