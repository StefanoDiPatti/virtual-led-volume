# virtual-led-volume

Real-time simulation of an LED wall for virtual production (in-camera VFX), written in C++ and OpenGL 4.1 core.

Project for the course *Real-Time Graphics Programming*, Master Degree in Computer Science, Università degli Studi di Milano, a.y. 2025/2026.

## Overview

In a virtual production stage the background is not added in post-production: a virtual environment is displayed on a large LED wall behind the actors and filmed directly by the camera. For the background to look correct from the camera, the image on the wall must be rendered from the position of the tracked camera, so that the wall behaves like a window onto the virtual world and shows the right parallax.

The application simulates this setup:

- the virtual environment is rendered into a render target (FBO) mapped on a 6 x 3 m LED wall, using an **off-axis (generalized) perspective projection** computed from the position of the **cine camera**;
- the cine camera films the stage (floor, subject and wall) as a real camera would, with physical focal length and sensor size;
- an **observer camera** shows the whole stage from outside, with debug lines for the frustums and the wall;
- the stage is lit by a directional **set light** with Blinn-Phong shading and **shadow mapping**, while the virtual environment has its own independent light.

## Features

- Off-axis projection (Kooima's generalized perspective projection) from the cine camera position; the environment is rendered into a 2048 x 1024 FBO shown on the wall
- Cine camera with focal length 18-50 mm, Full Frame (36 x 24 mm) and Super 35 (24.89 x 18.66 mm) sensors, letterbox
- Observer camera with debug lines: cine camera frustum (cyan), off-axis frustum (magenta), wall frame (yellow), world axes and camera gizmo
- Five views, including a ground truth view and a difference view (black where the wall image matches the direct view)
- Shadow mapping from the set light: no bias, slope-scaled bias, 3x3 PCF and hardware PCF (`sampler2DShadow`)
- Parameter panel (Dear ImGui) for cameras, views, lights, shadow mode and debug lines, with frame time
- CPU unit tests of the off-axis projection matrix

## Requirements

- Linux with a GPU and driver supporting OpenGL 4.1 core
- `g++` with C++17 support and `make`

All the libraries are included in `external/`.

## Build (Linux)

```
make        # build
make run    # build and run
make test   # CPU tests of the off-axis projection
make clean
```

The executable must be run from the repository root, because shader and asset paths are relative.

## Controls

| Key | Action |
|---|---|
| `Tab` | switch the controlled camera: observer / cine camera |
| `W` `A` `S` `D` | move forward, left, back, right (dolly / truck) |
| `E` `Q` | move up / down (cine camera pedestal) |
| Mouse | look around (pan / tilt) |
| Mouse wheel | zoom (cine camera focal length) |
| Right click | look at the subject |
| `1` - `5` | select the view |
| `G` | UI mode: free the cursor to use the parameter panel |
| `Esc` | quit |

## Views

| Key | View | Content |
|---|---|---|
| `1` | Split | cine camera (left) and observer (right) |
| `2` | Cine camera | the final shot: the subject in front of the LED wall |
| `3` | Stage | the observer view, full screen |
| `4` | Ground truth | the virtual environment seen directly from the cine camera, without the wall |
| `5` | Difference | ground truth minus the wall image: black where the projection is correct |

## Rendering passes

Each frame `src/main.cpp` runs three passes:

1. **Shadow pass**: floor and subject are rendered from the set light into a 2048 x 2048 depth texture (orthographic projection).
2. **Wall pass**: the virtual environment is rendered into the wall FBO with the off-axis projection from the cine camera position.
3. **Screen pass**: the selected view is rendered to the screen; the wall is a textured quad that shows the wall FBO, the stage uses the shadow map.

## Project structure

```
src/main.cpp   render loop
src/core/      window, input, GUI, named constants (config.h)
src/render/    shader wrappers, framebuffers, shadow map, off-axis math, debug lines
src/scene/     stage, LED wall, virtual environment, cine camera
src/utils/     shader, model, mesh and camera classes (see Credits)
shaders/       GLSL 4.10 shaders
assets/        models and textures
tests/         CPU tests of the off-axis projection
external/      third-party libraries
```

## Credits

- `src/utils/shader.h`, `src/utils/model.h`, `src/utils/mesh.h`, `src/utils/camera.h` and the `Makefile` structure are based on the laboratory code of the course by Davide Gadia and Michael Marchesan, in turn adapted from [LearnOpenGL](https://github.com/JoeyDeVries/LearnOpenGL) by Joey de Vries.
- The texture and cubemap loading functions in `src/render/texture.h` and the skybox shaders (`shaders/skybox.vert`, `shaders/skybox.frag`) are based on the same laboratory code.
- Models in `assets/models/` and textures in `assets/textures/` (UV test grid, skybox) come from the course laboratory material.
- Third-party libraries in `external/`: GLAD, GLFW, GLM, Assimp, stb_image.
- [Dear ImGui](https://github.com/ocornut/imgui) v1.91.9b by Omar Cornut (MIT license, see `external/imgui/LICENSE.txt`): core files and the GLFW / OpenGL 3 backends in `external/imgui/`, unmodified. `external/include/GLFW` is a symbolic link to `external/include/glfw`, because the backend includes `<GLFW/glfw3.h>`.
