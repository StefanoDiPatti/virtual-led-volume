#version 410 core

// output shader variable
out vec4 colorFrag;

// the transformed uv interpolated by the rasterizer
in vec2 vUV;

// texture sampler
uniform sampler2D wallTexture;

/////////// MAIN //////////////////
void main(void)
{
    colorFrag = texture(wallTexture, vUV);
}