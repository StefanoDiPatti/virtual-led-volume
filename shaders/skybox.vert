#version 410 core

layout (location = 0) in vec3 position;

// texture coordinates for the environment map sampling (we use 3 coordinates because we are sampling in 3 dimensions)
out vec3 interp_UVW;

uniform mat4 view;
uniform mat4 projection;

void main()
{
		// in this case, we are not using the UV coordinates of the models, but we use the vertex position as 3D texture coordinates, in order to have a 1:1 mapping from the cube map and the cube used as "the world"
		interp_UVW = position;

    vec4 pos = projection * view * vec4(position, 1.0);
		// we want to set the Z coordinate of the projected vertex at the maximum depth (i.e., we want Z to be equal to 1.0 after the projection divide)
		// -> we set Z equal to W (because in the projection divide, after clipping, all the components will be divided by W).
		gl_Position = pos.xyww;
}
