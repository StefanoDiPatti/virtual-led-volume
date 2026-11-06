#version 410 core

// output shader variable
out vec4 colorFrag;

// the transformed uv have been calculated per-vertex in the wall shader
in vec2 vUV;

/////////// MAIN //////////////////
void main(void)
{
    colorFrag = vec4(vUV.x, vUV.y, 0.0, 1.0);
}