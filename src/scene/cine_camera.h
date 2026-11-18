/*
CineCamera class
- tracked film camera on the stage: position and orientation (pan, tilt, roll in degrees, as on a fluid head)
- optics: focal length and sensor size in millimetres (sensor presets: Full Frame, Super 35)
- view matrix built with lookAt from the front and up vectors
- projection matrix from focal length and sensor size
- moves as on a film set: dolly, truck, pedestal (keyboard), pan and tilt (mouse), zoom (scroll wheel)
- re-centre: the camera can be turned to look at a point of the stage
*/

#pragma once

#include <array>

// Loader for GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// named constants of the application
#include <core/config.h>

// sensor formats of the camera
enum class SENSOR_PRESET
{
    FULL_FRAME,     // 36 x 24 mm
    SUPER_35        // 24.89 x 18.66 mm
};

// moves of the cine camera
// dolly: forward/backward, truck: left/right, pedestal: up/down, pan/tilt: rotations, zoom: focal length
// N.B.) pan, tilt and zoom are now driven by mouse and scroll wheel: their keyboard moves are kept as reference
enum class CINE_MOVEMENT
{
    DOLLY_IN,
    DOLLY_OUT,
    TRUCK_LEFT,
    TRUCK_RIGHT,
    PEDESTAL_UP,
    PEDESTAL_DOWN,
    PAN_LEFT,
    PAN_RIGHT,
    TILT_UP,
    TILT_DOWN,
    ZOOM_IN,
    ZOOM_OUT
};

/////////////////// CINE CAMERA class ///////////////////////
class CineCamera
{
public:

    // constructor: position and sensor format (the camera starts level, with the initial focal length below)
    CineCamera(glm::vec3 position, SENSOR_PRESET sensorPreset)
    {
        this->position = position;
        this->sensorPreset = sensorPreset;

        // we set the sensor size (mm) from the chosen format
        switch(sensorPreset)
        {
            case SENSOR_PRESET::FULL_FRAME:
                this->sensorWidth = FULL_FRAME_SENSOR_WIDTH;
                this->sensorHeight = FULL_FRAME_SENSOR_HEIGHT;
                break;
            case SENSOR_PRESET::SUPER_35:
                this->sensorWidth = SUPER_35_SENSOR_WIDTH;
                this->sensorHeight = SUPER_35_SENSOR_HEIGHT;
                break;
        }

        // initialization of the camera reference system
        UpdateVectors();
    }

    // it applies a move of the cine camera, scaled by deltaTime (frame-rate independent)
    void ProcessKeyboard(CINE_MOVEMENT movement, float deltaTime)
    {
        // horizontal front and right directions (from pan only): the dolly and the truck move on the floor,
        // even if the camera is tilted or rolled
        glm::vec3 frontH = glm::vec3(glm::sin(glm::radians(pan)), 0.0f, -glm::cos(glm::radians(pan)));
        glm::vec3 rightH = glm::vec3(glm::cos(glm::radians(pan)), 0.0f, glm::sin(glm::radians(pan)));

        switch(movement)
        {
            case CINE_MOVEMENT::DOLLY_IN:
                position += frontH * moveSpeed * deltaTime;
                break;
            case CINE_MOVEMENT::DOLLY_OUT:
                position -= frontH * moveSpeed * deltaTime;
                break;
            case CINE_MOVEMENT::TRUCK_RIGHT:
                position += rightH * moveSpeed * deltaTime;
                break;
            case CINE_MOVEMENT::TRUCK_LEFT:
                position -= rightH * moveSpeed * deltaTime;
                break;
            case CINE_MOVEMENT::PEDESTAL_UP:
                position.y += moveSpeed * deltaTime;
                break;
            case CINE_MOVEMENT::PEDESTAL_DOWN:
                position.y -= moveSpeed * deltaTime;
                break;
            // rotations: the reference system must be updated
            case CINE_MOVEMENT::PAN_RIGHT:
                pan += rotationSpeed * deltaTime;
                UpdateVectors();
                break;
            case CINE_MOVEMENT::PAN_LEFT:
                pan -= rotationSpeed * deltaTime;
                UpdateVectors();
                break;
            case CINE_MOVEMENT::TILT_UP:
                tilt += rotationSpeed * deltaTime;
                UpdateVectors();
                break;
            case CINE_MOVEMENT::TILT_DOWN:
                tilt -= rotationSpeed * deltaTime;
                UpdateVectors();
                break;
            // zoom: only the focal length changes (it is read by the projection), limited to the zoom lens range
            case CINE_MOVEMENT::ZOOM_IN:
                focalLength += zoomSpeed * deltaTime;
                focalLength = glm::clamp(focalLength, MIN_FOCAL_LENGTH, MAX_FOCAL_LENGTH);
                break;
            case CINE_MOVEMENT::ZOOM_OUT:
                focalLength -= zoomSpeed * deltaTime;
                focalLength = glm::clamp(focalLength, MIN_FOCAL_LENGTH, MAX_FOCAL_LENGTH);
                break;
        }
    }

    // mouse movement: pan and tilt follow the cursor offset (in pixels)
    // no deltaTime: the offset is already an amount of movement, not a state held in time
    void ProcessMouseMovement(float xoffset, float yoffset)
    {
        pan += xoffset * mouseSensitivity;
        tilt += yoffset * mouseSensitivity;

        // the tilt is clamped inside UpdateVectors
        UpdateVectors();
    }

    // scroll wheel: each notch changes the focal length (zoom), limited to the zoom lens range
    // no deltaTime: the scroll arrives as discrete events, not as a key held down
    void ProcessScroll(float yoffset)
    {
        focalLength += yoffset * zoomStep;
        focalLength = glm::clamp(focalLength, MIN_FOCAL_LENGTH, MAX_FOCAL_LENGTH);
    }

    // it turns the camera to look at a point of the stage (roll is not changed)
    // we invert the front formula of UpdateVectors: front.y = sin(tilt), front.x / -front.z = tan(pan)
    void LookAt(glm::vec3 target)
    {
        glm::vec3 offset = target - position;

        // target on the camera position: the direction is not defined (normalize would give NaN)
        if(glm::length(offset) < LOOKAT_MIN_DISTANCE)
            return;

        glm::vec3 dir = glm::normalize(offset);

        // atan2 keeps the right quadrant also when the camera looks backwards
        tilt = glm::degrees(glm::asin(dir.y));
        pan = glm::degrees((glm::atan(dir.x, -dir.z)));

        UpdateVectors();
    }

    // it returns the current view matrix
    glm::mat4 GetViewMatrix() const { return glm::lookAt(this->position, this->position + this->front, this->up); }

    // position and optics of the cine camera
    glm::vec3 GetPosition() const  { return this->position; }
    float GetFocalLength() const   { return this->focalLength; }
    float GetSensorWidth() const   { return this->sensorWidth; }
    float GetSensorHeight() const  { return this->sensorHeight; }

    // vertical angle of view (radians): FOV = 2 * atan(h / 2f), sensor height and focal length in mm
    float GetVerticalFov() const   { return 2 * glm::atan(sensorHeight / (2 * focalLength)); }
    // aspect ratio of the sensor
    float GetAspect() const        { return sensorWidth / sensorHeight; }

    // projection matrix of the cine camera, from its optics
    glm::mat4 GetProjectionMatrix(float nearPlane, float farPlane) const { return glm::perspective(this->GetVerticalFov(), this->GetAspect(), nearPlane, farPlane); }

    // corners of the frustum in world coordinates (0-3 near, 4-7 far, same counter-clockwise order):
    // we unproject the 8 corners of the NDC cube with the inverse of projection * view
    std::array<glm::vec3, 8> GetFrustumCorners(float nearPlane, float farPlane) const
    {
        std::array<glm::vec3, 8> corners;
        glm::mat4 inv = glm::inverse(this->GetProjectionMatrix(nearPlane, farPlane) * this->GetViewMatrix());
        std::array<glm::vec3, 8> ndcCorners = {glm::vec3(-1.0f, -1.0f, -1.0f),  glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(1.0f, 1.0f, -1.0f),  glm::vec3(-1.0f, 1.0f, -1.0f), // near (z = -1)
                                               glm::vec3(-1.0f, -1.0f,  1.0f),  glm::vec3(1.0f, -1.0f, 1.0f),  glm::vec3(1.0f, 1.0f,  1.0f),  glm::vec3(-1.0f, 1.0f, 1.0f)}; // far (z = +1)

        for(int i = 0; i < 8; i++)
        {
            glm::vec4 h = inv * glm::vec4(ndcCorners[i], 1.0f);
            // perspective divide: after the inverse projection w is not 1
            corners[i] = glm::vec3(h) / h.w;
        }

        return corners;
    }

    // orientation of the cine camera (unit vectors, roll included)
    glm::vec3 GetFront() const { return this->front; }
    glm::vec3 GetRight() const { return this->right; }
    glm::vec3 GetUp()    const { return this->up; }

private:
    // position and orientation (degrees)
    glm::vec3 position;
    float pan = 0.0f, tilt = 0.0f, roll = 0.0f;

    // optics (mm)
    float focalLength = 35.0f;
    float sensorWidth, sensorHeight;
    SENSOR_PRESET sensorPreset;

    // speeds of the moves (rotation and zoom speeds are used only by the keyboard moves kept as reference)
    float moveSpeed = 1.0f;         // m/s
    float rotationSpeed = 30.0f;    // degrees/s
    float zoomSpeed = 10.0f;        // mm/s
    float mouseSensitivity = 0.1f;  // degrees/pixel
    float zoomStep = 2.0f;          // mm/notch

    // camera reference system (by default looking towards -Z, at the wall)
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    // it updates the camera reference system from pan, tilt and roll
    void UpdateVectors()
    {
        glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

        // we limit the tilt: at +-90 degrees front would be parallel to worldUp and the cross product would be zero
        if(tilt > MAX_TILT)
            tilt = MAX_TILT;
        if(tilt < -MAX_TILT)
            tilt = -MAX_TILT;

        float panRad = glm::radians(pan);
        float tiltRad = glm::radians(tilt);
        float rollRad = glm::radians(roll);

        // view direction: pan = 0 and tilt = 0 look towards -Z, positive pan turns right, positive tilt looks up
        // (same formula of the lab camera with yaw = pan - 90)
        front = glm::vec3(glm::sin(panRad) * glm::cos(tiltRad), glm::sin(tiltRad), -glm::cos(panRad) * glm::cos(tiltRad));

        // right and up vectors without roll
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));

        // roll: we rotate right and up around the view direction (w = 0, they are directions)
        // positive roll = clockwise as seen from behind the camera
        glm::mat4 R = glm::rotate(glm::mat4(1.0f), rollRad, front);
        up = glm::vec3(R * glm::vec4(up, 0.0f));
        right = glm::vec3(R * glm::vec4(right, 0.0f));
    }
};
