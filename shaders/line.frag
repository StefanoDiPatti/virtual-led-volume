#version 410 core

out vec4 colorFrag;

// colour of the line, from the vertex shader (unlit)
in vec3 vColor;

/////////// MAIN //////////////////
void main(void)
{
    colorFrag = vec4(vColor, 1.0f);
}