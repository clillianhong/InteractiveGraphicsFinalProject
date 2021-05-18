#version 140

uniform sampler2D image;
uniform float thresholds[10];


//camera mats 
uniform mat4 mV; 
uniform mat4 mProjInv;
// uniform mat4 mVInv; 
uniform mat4 mProj; 

uniform mat4 mL;


uniform vec3 lightPosition;
uniform sampler2D normalTexture; 
uniform sampler2D depthTexture;

in vec2 geom_texCoord;

out vec4 fragColor;

void main() {		
	vec4 color = texture(image, geom_texCoord);

	vec3 normal = normalize((2.0 * texture(normalTexture, geom_texCoord).xyz) - 1.0); 
    
    vec4 positionNDC = vec4(vec3(geom_texCoord, texture(depthTexture, geom_texCoord)) * 2.0 - 1.0, 1.0);
    vec4 eyePos = mProjInv * positionNDC;
    eyePos /= eyePos.w;
    vec3 vEyePosition = eyePos.xyz;
    // vec3 vPosition = vec3(positionEye);

    // vec4 color;

   
    // float isBg = (surfaceHit != 1.0) ? 0 : 1;
    vec4 lightPos4 = (mV * mL * vec4(lightPosition, 1.0));
    vec3 lightPos = (lightPos4).xyz;

    vec3 lightDir = lightPos - vEyePosition;

    float intensity = dot(lightDir,normalize(normal));

    // if (intensity > 0.95)
	// 	color = vec4(color.xyz*0.95, 1.0);
	// else if (intensity > 0.5)
	// 	color = vec4(color.xyz*0.50, 1.0);
	// else if (intensity > 0.25)
	// 	color = vec4(color.xyz*0.25, 1.0);
	// else
	// 	color = vec4(color.xyz*0.15, 1.0);

    
    fragColor = vec4(color.xyz, 1);
}
