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

// the position of the point light is passed as uniform
uniform vec3 pointLightPosition;

// light incidence direction (in view coordinates)
out vec3 lightDir;
// the transformed normal (in view coordinate) is set as an output variable, to be "passed" to the fragment shader
// this means that the normal values in each vertex will be interpolated on each fragment created during rasterization between two vertices
out vec3 vNormal;

// in the subroutines in fragment shader where specular reflection is considered,
// we need to calculate also the reflection vector for each fragment
// to do this, we need to calculate in the vertex shader the view direction (in view coordinates) for each vertex, and to have it interpolated for each fragment by the rasterization stage
out vec3 vViewPosition;

void main()
{
    // transformations are applied to each vertex
    vec4 hom_position = vec4(position, 1.0f);

    // vertex position in ModelView coordinate (see the last line for the application of projection)
    // when I need to use coordinates in camera coordinates, I need to split the application of model and view transformations from the projection transformations
    vec4 mvPosition = view * model * hom_position;

    // view direction, negated to have vector from the vertex to the camera
    vViewPosition = -mvPosition.xyz;

    // transformations are applied to the normal
    vNormal = normalize(normals * normal);

    // light incidence direction (in view coordinate)
    vec4 lightPos = view  * vec4(pointLightPosition, 1.0);
    lightDir = normalize(lightPos.xyz - mvPosition.xyz);

    // apply the projection matrix
    gl_Position = projection * mvPosition;
}
