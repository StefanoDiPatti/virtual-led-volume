#version 410 core

// output shader variable
out vec4 colorFrag;

// light incidence direction (calculated in vertex shader, interpolated by rasterization)
in vec3 lightDir;
// the transformed normal has been calculated per-vertex in the vertex shader
in vec3 vNormal;
// vector from fragment to camera (in view coordinate)
in vec3 vViewPosition;

// fragment position in the clip space of the set light (for the shadow lookup)
in vec4 posLightSpace;

// depth map rendered from the set light (shadow pass)
uniform sampler2D shadowMap;

// the same depth map read through the hardware PCF sampler: texture() returns the comparison result
uniform sampler2DShadow shadowMapHardware;

// shadow technique: 0 OFF, 1 NO_BIAS, 2 BIAS, 3 PCF, 4 PCF_HARDWARE (same order of the SHADOW_MODE enum)
uniform int shadowMode;

// ambient, diffusive and specular components
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
// weight of the components
// in this case, we can pass separate values from the main application even if Ka+Kd+Ks>1.
// In more "realistic" situations, I have to set this sum = 1, or at least Kd+Ks = 1, by passing Kd as uniform, and then setting Ks = 1.0-Kd
uniform float Ka;
uniform float Kd;
uniform float Ks;

// shininess coefficients
uniform float shininess;

/////////// SHADOW MAPPING //////////////////
// it returns 0 if the fragment is lit, 1 if it is in shadow (values in between with PCF)
float ShadowCalculation()
{
    // shadows disabled
    if(shadowMode == 0)
        return 0.0;

    // perspective divide: from the clip space of the light to NDC [-1, 1]
    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;

    // we remap to [0, 1]: xy are the texture coordinates in the shadow map, z the depth of this fragment seen from the light
    projCoords = projCoords * 0.5 + 0.5;

    // beyond the far plane of the light: never in shadow
    if(projCoords.z > 1.0)
        return 0.0;

    vec3 N = normalize(vNormal);
    vec3 L = normalize(lightDir);

    // slope-scaled bias against shadow acne: larger when the light is grazing the surface.
    // The light projection is orthographic, so depth is linear: 0.001 is ~3 cm in our 29 m depth range
    float bias = (shadowMode == 1) ? 0.0 : max(0.003 * (1.0 - dot(N, L)), 0.0003);

    // PCF_HARDWARE: one fetch, the texture unit compares the reference depth with the 2x2 nearest texels
    // and blends the results bilinearly. It returns the lit fraction, so the shadow is 1 - result
    if(shadowMode == 4)
        return 1.0 - texture(shadowMapHardware, vec3(projCoords.xy, projCoords.z - bias));

    // NO_BIAS and BIAS: a single sample of the shadow map
    if(shadowMode < 3)
    {
        // depth of the closest surface seen from the light in this direction
        float closestDepth = texture(shadowMap, projCoords.xy).r;
        // if this fragment is farther than the closest surface, something is between it and the light
        return (projCoords.z - bias > closestDepth) ? 1.0 : 0.0;
    }

    // PCF: we repeat the test on the 3x3 texels around the fragment and we average the results, to smooth the shadow edges
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    float shadow = 0.0;

    for(int x = -1; x <= 1; x++)
    {
        for(int y = -1; y <= 1; y++)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

/////////// ILLUMINATION MODEL //////////////////
vec3 BlinnPhong()
{
    float shadow = ShadowCalculation();

    // ambient component can be calculated at the beginning
    vec3 color = Ka*ambientColor;

    // normalization of the per-fragment normal
    vec3 N = normalize(vNormal);

    // normalization of the per-fragment light incidence direction
    vec3 L = normalize(lightDir);

    // Lambert coefficient
    float lambertian = max(dot(L,N), 0.0);

    // if the lambert coefficient is positive, then I can calculate the specular component
    if(lambertian > 0.0)
    {
      // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
      vec3 V = normalize( vViewPosition );

      // in the Blinn-Phong model we do not use the reflection vector, but the half vector
      vec3 H = normalize(L + V);

      // we use H to calculate the specular component
      float specAngle = max(dot(H, N), 0.0);
      // shininess application to the specular component
      float specular = pow(specAngle, shininess);

      // We add diffusive and specular components to the final color
      // N.B. ): in this implementation, the sum of the components can be different than 1
      color += vec3((Kd * lambertian * diffuseColor +
                      Ks * specular * specularColor) * (1.0 - shadow));
    }
    return color;
}

/////////// MAIN //////////////////
void main(void)
{
  	vec3 color = BlinnPhong();
    colorFrag = vec4(color, 1.0);
}
