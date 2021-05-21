#version 330

// This is a shader code fragment (not a complete shader) that contains 
// the functions to evaluate the microfacet BRDF.

const float PI = 3.14159265358979323846264;


//camera mats 
uniform mat4 mV; 
uniform mat4 mProjInv;
uniform mat4 mVInv; 
uniform mat4 mProj; 

//light +  light camera mats 
uniform mat4 mL;
uniform mat4 mLCProj; //projectection matrix 
uniform mat4 mLCView; //view matrix 

uniform vec3 lightPosition; //light pos in world space 
uniform vec3 lightPower; 

uniform sampler2D diffuseTexture; 
uniform sampler2D shaderParamTexture;
uniform sampler2D normalTexture; 
uniform sampler2D depthTexture;
uniform sampler2D shadowMapTexture;

uniform float thresholds[10];

in vec2 geom_texCoord;

out vec4 fragColor;

float isotropicMicrofacet(vec3 i, vec3 o, vec3 n, float eta, float alpha);

void main() {
    // Extract values from previous pass from framebuffer textures
    vec3 normal = normalize((2.0 * texture(normalTexture, geom_texCoord).xyz) - 1.0); 
    vec3 diffuseReflectance = texture(diffuseTexture, geom_texCoord).xyz; 
    vec4 params = texture(shaderParamTexture, geom_texCoord); 
    float refractiveIndex = params.x * 20.0f; 
    float roughness = params.y;
    float surfaceHit = params.z;
    
    // From the FSQ texture coordinates and the depth stored in the G-buffer, get the NDC coordinates of the fragment by scaling from [0,1] to [-1,1].
    vec4 positionNDC = vec4(vec3(geom_texCoord, texture(depthTexture, geom_texCoord)) * 2.0 - 1.0, 1.0);
    vec4 eyePos = mProjInv * positionNDC;
    eyePos /= eyePos.w;
    vec3 vEyePosition = eyePos.xyz;
    
    // Shadow map calculations
    bool inShadowView = true;
    vec4 ndc_light = mLCProj * mLCView * mVInv * vec4(vEyePosition, 1); 
    // vec4 ndc_light = mLCProj * mLCView * mVInv * vec4(vEyePosition, 1);  
    ndc_light /= ndc_light.w; 
    vec4 shadow_scaled = ((ndc_light + 1.0) / 2.0);
    if (shadow_scaled.x > 1.0 || shadow_scaled.x < 0.0 || shadow_scaled.y > 1.0 || shadow_scaled.y < 0.0) {
        inShadowView = false;
    }
    float shadow_map_depth = texture(shadowMapTexture, shadow_scaled.xy).x;
    float fragment_depth = shadow_scaled.z; 

    vec3 color;

    float isBg = (surfaceHit != 1.0) ? 0 : 1;
    if (inShadowView && fragment_depth > shadow_map_depth + 0.005) {
        color = vec3(0,0,0); 
    } else if (isBg > 0.5) {
        //old light dir code
        vec4 lightPos4 = (mV * mL * vec4(lightPosition, 1.0));
        vec3 lightPos = (lightPos4).xyz;

        vec3 lightDir = lightPos - vEyePosition;

        vec3 viewDir = -vEyePosition;
        viewDir = normalize(viewDir);
        
        float lightToPtSqrDist = dot(lightDir, lightDir);
        lightDir = normalize(lightDir);
        float NdotW = max(dot(normal, lightDir), 0);
        
        float intensity = NdotW;

        color = (diffuseReflectance / PI);

        if (intensity > thresholds[0])
            color = color * 0.5;
        else if (intensity > thresholds[1])
            color = color * 1.0;
        else if (intensity > thresholds[2])
            color = color * 1.5;
        else
            color = color * 0.05;

    } else {
        color = vec3(0, 0, 0);
    }
    fragColor = vec4(color, isBg);
}
