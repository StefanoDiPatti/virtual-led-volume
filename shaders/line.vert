#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

uniform mat4 view;
uniform mat4 projection;

out vec3 vColor;

void main()
{
    vColor = color;
    // the vertices are already in world coordinates: no model matrix
    gl_Position = projection * view * vec4(position, 1.0f);
}
