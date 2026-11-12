/*
CineCamera class
- tracked film camera on the stage: position and orientation (pan, tilt, roll in degrees, as on a fluid head)
- optics: focal length and sensor size in millimetres (sensor presets: Full Frame, Super 35)
- view matrix built with lookAt from the front and up vectors
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

    // it returns the current view matrix
    glm::mat4 GetViewMatrix() const { return glm::lookAt(this->position, this->position + this->front, this->up); }

    glm::vec3 GetPosition() const  { return this->position; }
    float GetFocalLength() const   { return this->focalLength; }
    float GetSensorWidth() const   { return this->sensorWidth; }
    float GetSensorHeight() const  { return this->sensorHeight; }

private:
    // position and orientation (degrees)
    glm::vec3 position = glm::vec3(0.0f);
    float pan = 0.0f, tilt = 0.0f, roll = 0.0f;

    // optics (mm)
    float focalLength = 35.0f;
    float sensorWidth = 36.0f, sensorHeight = 24.0f;
    SENSOR_PRESET sensorPreset = SENSOR_PRESET::FULL_FRAME;

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
