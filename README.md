# virtual-led-volume

Real-time simulation of an LED wall for virtual production (in-camera VFX), written in C++ and OpenGL 4.1 core.

Project for the course *Real-Time Graphics Programming*, Master Degree in Computer Science, Università degli Studi di Milano, a.y. 2025/2026.

## Build (Linux)

```
make        # build
make run    # build and run
make clean
```

## Credits

- `src/utils/shader.h`, `src/utils/model.h`, `src/utils/mesh.h`, `src/utils/camera.h` and the `Makefile` structure are based on the laboratory code of the course by Davide Gadia and Michael Marchesan, in turn adapted from [LearnOpenGL](https://github.com/JoeyDeVries/LearnOpenGL) by Joey de Vries.
- Models in `assets/models/` come from the course laboratory material.
- Third-party libraries in `external/`: GLAD, GLFW, GLM, Assimp, stb_image.
