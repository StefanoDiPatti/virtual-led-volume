/*
config
- named constants of the application, collected in a single place
- scalars, strings and enum values are preprocessor macros (#define)
- vectors and colours are typed constants (const glm::vec3) inside the config namespace
*/

#pragma once

#include <glm/glm.hpp>

/////////////////// window ///////////////////////
#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080
#define WINDOW_TITLE  "virtual-led-volume"

/////////////////// GUI  //////////////////////////
// top left corner of the GUI panel (pixels)
#define GUI_POSITION_X 10.0f
#define GUI_POSITION_Y 10.0f

/////////////////// glsl ///////////////////////
// GLSL version of the shaders of the ImGui OpenGL 3 backend
#define GLSL_VERSION "#version 410"

/////////////////// paths ///////////////////////
// paths are relative to the folder where the executable is launched
#define SCENE_VERT_PATH  "shaders/scene.vert"
#define SCENE_FRAG_PATH  "shaders/scene.frag"
#define WALL_VERT_PATH   "shaders/wall.vert"
#define WALL_FRAG_PATH   "shaders/wall.frag"
#define LINE_VERT_PATH   "shaders/line.vert"
#define LINE_FRAG_PATH   "shaders/line.frag"
#define SKYBOX_VERT_PATH "shaders/skybox.vert"
#define SKYBOX_FRAG_PATH "shaders/skybox.frag"
#define DEPTH_VERT_PATH  "shaders/depth.vert"
#define DEPTH_FRAG_PATH  "shaders/depth.frag"

#define CUBE_MODEL_PATH    "assets/models/cube.obj"
#define SPHERE_MODEL_PATH  "assets/models/sphere.obj"
#define PLANE_MODEL_PATH   "assets/models/plane.obj"
#define SUBJECT_MODEL_PATH "assets/models/babyyoda.obj"
#define SKYBOX_FOLDER      "assets/textures/skybox/"

/////////////////// projection ///////////////////////
// clip planes shared by the observer, the cine camera and the LED wall (metres)
#define NEAR_PLANE 0.1f
#define FAR_PLANE  100.0f

/////////////////// observer camera ///////////////////////
// vertical field of view of the observer (degrees)
#define OBSERVER_FOV 45.0f

/////////////////// cine camera ///////////////////////
// sensor format of the cine camera
#define CINE_START_SENSOR SENSOR_PRESET::FULL_FRAME

// zoom range of the lens (mm)
#define MIN_FOCAL_LENGTH 18.0f
#define MAX_FOCAL_LENGTH 50.0f
// tilt limit: at +-90 degrees front and world up are parallel and the right vector is not defined
#define MAX_TILT 89.0f
// below this distance the look-at direction is not defined (metres)
#define LOOKAT_MIN_DISTANCE 1e-4f

/////////////////// off-axis projection ///////////////////////
// minimum distance of the eye in front of the screen plane (metres)
#define OFFAXIS_MIN_DISTANCE 1e-4f

// sensor sizes of the presets (mm)
#define FULL_FRAME_SENSOR_WIDTH  36.0f
#define FULL_FRAME_SENSOR_HEIGHT 24.0f
#define SUPER_35_SENSOR_WIDTH    24.89f
#define SUPER_35_SENSOR_HEIGHT   18.66f

/////////////////// LED wall ///////////////////////
// resolution of the render target displayed on the wall (pixels)
#define WALL_RT_WIDTH  2048
#define WALL_RT_HEIGHT 1024

/////////////////// shadows ///////////////////////
// resolution of the (square) shadow map (pixels)
#define SHADOW_MAP_SIZE 2048
// half size of the orthographic volume of the set light (metres): it encloses the 10x10 m stage (half diagonal ~7.07 m)
#define SHADOW_ORTHO_HALF_SIZE 7.5f
// near and far planes of the light (metres): the light is 15 m from the target
#define SHADOW_NEAR 1.0f
#define SHADOW_FAR  30.0f
// texture unit of the shadow map in the scene shader (unit 0 is used by the wall and the skybox)
#define SHADOW_TEXTURE_UNIT 1

// limits of the light position slider in the GUI (metres): the light stays above and around the 10x10 m stage
#define LIGHT_POSITION_MIN -15.0f
#define LIGHT_POSITION_MAX  15.0f
// above this value the light direction is considered parallel to the world up (about 2.6 degrees)
#define LIGHT_PARALLEL_UP_DOT 0.999f

/////////////////// debug lines ///////////////////////
// capacity of the line renderer (2 vertices per segment)
#define LINE_MAX_VERTICES 1024
// lengths of the drawn helpers (metres)
#define WORLD_AXES_LENGTH 1.0f
#define GIZMO_LENGTH      0.5f
// far plane used only to draw the cine camera frustum (metres)
#define FRUSTUM_DRAW_FAR  7.0f

/////////////////// input ///////////////////////
// size of the pressed keys table (GLFW key codes go up to GLFW_KEY_LAST = 348)
#define MAX_KEYS 1024

/////////////////// scene ///////////////////////
// uniform scale of the subject model (the original model is too big)
#define SUBJECT_SCALE 0.3f

/////////////////// config namespace ///////////////////////
namespace config
{
    // world reference frame
    const glm::vec3 WORLD_ORIGIN(0.0f, 0.0f, 0.0f);
    const glm::vec3 WORLD_X(1.0f, 0.0f, 0.0f);
    const glm::vec3 WORLD_Y(0.0f, 1.0f, 0.0f);
    const glm::vec3 WORLD_Z(0.0f, 0.0f, 1.0f);

    // initial positions of the cameras (metres)
    const glm::vec3 OBSERVER_START_POSITION(0.0f, 1.7f, 6.0f);
    const glm::vec3 CINE_START_POSITION(0.0f, 1.5f, 3.0f);
    // point the active camera looks at when re-centred (right mouse button)
    const glm::vec3 SUBJECT_TARGET(0.0f, 0.5f, 0.0f);

    // point the set light looks at (centre of the stage)
    const glm::vec3 SHADOW_TARGET(0.0f, 0.0f, 0.0f);

    // corners of the LED wall (pd is computed from these three)
    const glm::vec3 WALL_LOWER_LEFT(-3.0f, 0.0f, -3.0f);
    const glm::vec3 WALL_LOWER_RIGHT(3.0f, 0.0f, -3.0f);
    const glm::vec3 WALL_UPPER_LEFT(-3.0f, 3.0f, -3.0f);

    // virtual environment: positions and scales of the objects
    const glm::vec3 FAR_PILLAR_POSITION(0.0f, 3.0f, -15.0f);
    const glm::vec3 FAR_PILLAR_SCALE(1.0f, 3.0f, 1.0f);
    const glm::vec3 NEAR_PILLAR_POSITION(-2.0f, 1.5f, -5.0f);
    const glm::vec3 NEAR_PILLAR_SCALE(0.4f, 1.5f, 0.4f);
    const glm::vec3 SPHERE_POSITION(1.5f, 1.0f, -8.0f);

    // colours of the stage
    const glm::vec3 PLANE_COLOR(0.0f, 0.5f, 0.0f);
    const glm::vec3 SUBJECT_COLOR(1.0f, 0.0f, 0.0f);

    // colours of the virtual environment
    const glm::vec3 FAR_PILLAR_COLOR(1.0f, 1.0f, 1.0f);
    const glm::vec3 NEAR_PILLAR_COLOR(1.0f, 0.5f, 0.0f);
    const glm::vec3 SPHERE_COLOR(1.0f, 1.0f, 0.0f);

    // colours of the light
    const glm::vec3 AMBIENT_COLOR(0.1f, 0.1f, 0.1f);
    const glm::vec3 SPECULAR_COLOR(1.0f, 1.0f, 1.0f);

    // sun of the virtual environment: directional light from this position towards the target, independent of the set light
    const glm::vec3 ENVIRONMENT_LIGHT_POSITION(-10.0f, 8.0f, 2.0f);

    // clear colours
    const glm::vec3 BACKGROUND_COLOR(0.2f, 0.3f, 0.3f);
    const glm::vec3 LETTERBOX_COLOR(0.0f, 0.0f, 0.0f);

    // colours of the debug lines
    const glm::vec3 AXIS_X_COLOR(1.0f, 0.0f, 0.0f);
    const glm::vec3 AXIS_Y_COLOR(0.0f, 1.0f, 0.0f);
    const glm::vec3 AXIS_Z_COLOR(0.0f, 0.0f, 1.0f);
    const glm::vec3 WALL_FRAME_COLOR(1.0f, 1.0f, 0.0f);
    const glm::vec3 CINE_FRUSTUM_COLOR(0.0f, 1.0f, 1.0f);
    const glm::vec3 OFFAXIS_FRUSTUM_COLOR(1.0f, 0.0f, 1.0f);
}
