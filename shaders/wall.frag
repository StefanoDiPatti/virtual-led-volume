#version 410 core

// output shader variable
out vec4 colorFrag;

// the transformed uv interpolated by the rasterizer
in vec2 vUV;

// fragment position in world coordinates
in vec3 worldPosition;

// inner frustum (rendered from the cine camera) and outer frustum (rendered from the fixed eye)
uniform sampler2D innerTexture;
uniform sampler2D outerTexture;

// projection * view of the cine camera
uniform mat4 cameraViewProjection;

// content shown on the wall: 0 INNER, 1 OUTER, 2 COMPOSITE (same order of the WALL_CONTENT enum)
uniform int content;

// overscan of the inner frustum and width of the soft edge (NDC)
uniform float innerMargin;
uniform float innerBlend;

// weight of the inner frustum: 1 inside the camera frame (+ overscan), 0 outside, smooth in between
float InnerWeight()
{
    vec4 clipPosition = cameraViewProjection * vec4(worldPosition, 1.0);
    
    // behind the cine camera: outer frustum
    if(clipPosition.w <= 0.0)
        return 0.0;
    
    vec2 ndc = clipPosition.xy / clipPosition.w;
    // distance from the frame centre in NDC: 1 on the frame edge
    float d = max(abs(ndc.x), abs(ndc.y));
    return 1.0 - smoothstep(1.0 + innerMargin, 1.0 + innerMargin + innerBlend, d);
}

/////////// MAIN //////////////////
void main(void)
{
    vec4 innerColor = texture(innerTexture, vUV);
    vec4 outerColor = texture(outerTexture, vUV);

    if(content == 0)
        colorFrag = innerColor;
    else if(content == 1)
        colorFrag = outerColor;
    // composite: inner where the cine camera looks, outer elsewhere, soft edge in between
    else
        colorFrag = mix(outerColor, innerColor, InnerWeight());
}