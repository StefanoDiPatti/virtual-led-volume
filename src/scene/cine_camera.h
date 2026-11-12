/*
CineCamera class
- tracked film camera on the stage: position and orientation (pan, tilt, roll in degrees, as on a fluid head)
- optics: focal length and sensor size in millimetres (sensor presets: Full Frame, Super 35)
- view matrix built with lookAt from the front and up vectors
- projection matrix from focal length and sensor size
- keyboard moves as on a film set: dolly, truck, pedestal, pan, tilt, zoom
*/

#pragma once

// Loader for GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// sensor formats of the camera
enum class SENSOR_PRESET
{
    FULL_FRAME,     // 36 x 24 mm
    SUPER_35        // 24.89 x 18.66 mm
};

// moves of the cine camera
// dolly: forward/backward, truck: left/right, pedestal: up/down, pan/tilt: rotations, zoom: focal length
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

    // constructor: position, orientation (degrees), focal length (mm) and sensor format
    CineCamera(glm::vec3 position, float pan, float tilt, float roll, float focalLength, SENSOR_PRESET sensorPreset)
    {
        this->position = position;
        this->pan = pan;
        this->tilt = tilt;
        this->roll = roll;
        this->focalLength = focalLength;
        this->sensorPreset = sensorPreset;

        // we set the sensor size (mm) from the chosen format
        switch(sensorPreset)
        {
            case SENSOR_PRESET::FULL_FRAME:
                this->sensorWidth = 36.0f;
                this->sensorHeight = 24.0f;
                break;
            case SENSOR_PRESET::SUPER_35:
                this->sensorWidth = 24.89f;
                this->sensorHeight = 18.66f;
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
                focalLength = glm::clamp(focalLength, 18.0f, 50.0f);
                break;
            case CINE_MOVEMENT::ZOOM_OUT:
                focalLength -= zoomSpeed * deltaTime;
                focalLength = glm::clamp(focalLength, 18.0f, 50.0f);
                break;
        }
    }

    // it returns the current view matrix
    glm::mat4 GetViewMatrix() const { return glm::lookAt(this->position, this->position + this->front, this->up); }

    glm::vec3 GetPosition() const  { return this->position; }
    float GetFocalLength() const   { return this->focalLength; }
    float GetSensorWidth() const   { return this->sensorWidth; }
    float GetSensorHeight() const  { return this->sensorHeight; }

    // vertical angle of view (radians): FOV = 2 * atan(h / 2f), sensor height and focal length in mm
    float GetVerticalFov() const   { return 2 * glm::atan(sensorHeight / (2 * focalLength)); }
    // aspect ratio of the sensor
    float GetAspect() const        { return sensorWidth / sensorHeight; }

    // projection matrix of the cine camera, from its optics
    glm::mat4 GetProjectionMatrix(float nearPlane, float farPlane) { return glm::perspective(this->GetVerticalFov(), this->GetAspect(), nearPlane, farPlane); }

private:
    // position and orientation (degrees)
    glm::vec3 position = glm::vec3(0.0f);
    float pan = 0.0f, tilt = 0.0f, roll = 0.0f;

    // optics (mm)
    float focalLength = 35.0f;
    float sensorWidth = 36.0f, sensorHeight = 24.0f;
    SENSOR_PRESET sensorPreset = SENSOR_PRESET::FULL_FRAME;

    // speeds of the moves
    float moveSpeed = 1.0f;         // m/s
    float rotationSpeed = 30.0f;    // degrees/s
    float zoomSpeed = 10.0f;        // mm/s

    // camera reference system (by default looking towards -Z, at the wall)
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    // it updates the camera reference system from pan, tilt and roll
    void UpdateVectors()
    {
        glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

        // we limit the tilt: at +-90 degrees front would be parallel to worldUp and the cross product would be zero
        if(tilt > 89.0f)
            tilt = 89.0f;
        if(tilt < -89.0f)
            tilt = -89.0f;

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
