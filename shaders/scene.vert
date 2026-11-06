#version 410 core

// vertex position in object coordinates
layout (location = 0) in vec3 position;
// vertex normal in object coordinate
layout (location = 1) in vec3 normal;
// the numbers used for the location in the layout qualifier are the positions of the vertex attribute
// as defined in the Mesh class

// model matrix
uniform mat4 model;
// view matrix
uniform mat4 view;
// projection matrix
uniform mat4 projection;

// normals transformation matrix (= transpose of the inverse of the model-view matrix)
uniform mat3 normals;

// the transformed normal is set as an output variable, to be "passed" to the fragment shader
// this means that the normal values in each vertex will be interpolated on each fragment created during rasterization between two vertices
out vec3 N;

void main()
{
    // transformations are applied to each vertex
    vec4 hom_position = vec4(position, 1.0f);
    gl_Position = projection * view * model * hom_position;
	// transformations are applied to the normal
    N = normalize(normals * normal);
}
