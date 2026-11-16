#version 410 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// vertex colour
layout (location = 1) in vec3 color;

// view matrix
uniform mat4 view;
// projection matrix
uniform mat4 projection;

// colour passed to the fragment shader
out vec3 vColor;

void main()
{
    vColor = color;
    // the vertices are already in world coordinates: no model matrix
    gl_Position = projection * view * vec4(position, 1.0f);
}
