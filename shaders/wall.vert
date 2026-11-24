#version 410 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 vUV;

// vertex position in world coordinates: the fragment shader projects it with the cine camera
out vec3 worldPosition;

void main()
{
    vec4 hom_position = vec4(position, 1.0f);
    vUV = uv;

    worldPosition = (model * hom_position).xyz;

    gl_Position = projection * view * model * hom_position;
}
