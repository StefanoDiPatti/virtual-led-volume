#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// normals transformation matrix (= transpose of the inverse of the model-view matrix)
uniform mat3 normals;

// direction of the directional light (world coordinates, from the light to the target)
uniform vec3 lightDirection;
uniform vec3 wallLightDirection;

// light space matrix (projection * view of the set light), for the shadow lookup
uniform mat4 lightSpaceMatrix;

// light incidence direction (in view coordinates)
out vec3 lightDir;
out vec3 wallLightDir;
out vec3 vNormal;

out vec3 vViewPosition;

// vertex position in the clip space of the set light: interpolated, it is used in the fragment shader to read the shadow map
out vec4 posLightSpace;

void main()
{
    vec4 hom_position = vec4(position, 1.0f);

    vec4 mvPosition = view * model * hom_position;

    // view direction, negated to have vector from the vertex to the camera
    vViewPosition = -mvPosition.xyz;

    vNormal = normalize(normals * normal);

    lightDir = normalize(-(mat3(view) * lightDirection));
    wallLightDir = normalize(-(mat3(view) * wallLightDirection));

    // the light is not related to the camera: we start from world coordinates (model only, no view)
    posLightSpace = lightSpaceMatrix * model * hom_position;

    gl_Position = projection * mvPosition;
}
