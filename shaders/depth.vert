#version 410 core

layout (location = 0) in vec3 position;

uniform mat4 model;

// light space matrix (= projection * view of the set light): from world coordinates to the clip space of the light
uniform mat4 lightSpaceMatrix;

void main()
{
    vec4 hom_position = vec4(position, 1.0);
    // vertex position in the clip space of the light: the rasterizer writes its depth in the shadow map
    gl_Position = lightSpaceMatrix * model * hom_position;
}
