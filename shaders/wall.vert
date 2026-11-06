#version 410 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// plane coordinates
layout (location = 2) in vec2 uv;

// model matrix
uniform mat4 model;
// view matrix
uniform mat4 view;
// projection matrix
uniform mat4 projection;

// // UV coordinates passed to the fragment shader
out vec2 vUV;

void main()
{
    // transformations are applied to each vertex
    vec4 hom_position = vec4(position, 1.0f);
    vUV = uv;

    // apply the projection matrix
    gl_Position = projection * view * model * hom_position;
}
