#version 330

// This is a shader code fragment (not a complete shader) that contains 
// the functions to evaluate the microfacet BRDF.

const float PI = 3.14159265358979323846264;

uniform mat4 mL;
uniform mat4 mV; 
uniform vec3 lightPosition; //light pos in model space
uniform vec3 lightPower; 
uniform float refractiveIndex;
uniform float roughness;
uniform vec3 diffuseReflectance; 

in vec3 vPosition;
in vec3 vNormal;

// Note: looked at tutorial by Joey DeVries, "LearnOpenGL"
// https://learnopengl.com/Advanced-Lighting/Deferred-Shading

layout(location = 0) out vec4 diffuseColor;
layout(location = 1) out vec4 specularParamColor; 
layout(location = 2) out vec4 normalColor; 
// layout(location = 3) out vec4 lightDirColor; 


// Diffuse color texture (diffuse.r, diffuse.g, diffuse.b, ALPHA)
// Specular parameter texture (refractiveIndex, roughness, onSurfaceOrNot, ALPHA)
// Surface normal texture (norm.x, norm.y, norm.z, ALPHA) 

void main() {
	// Diffuse color texture (diffuse.r, diffuse.g, diffuse.b, ALPHA)
	diffuseColor = vec4(diffuseReflectance, 1.0);

	// Specular parameter texture (refractiveIndex, roughness, onSurfaceOrNot, ALPHA)
	specularParamColor = vec4(refractiveIndex/20.0f, roughness, 1.0, 1.0);

	// Surface normal texture (norm.x, norm.y, norm.z, ALPHA) 
	vec3 normal = (gl_FrontFacing) ? vNormal : -vNormal;
	normalColor = 0.5 * (1.0 + vec4(normalize(normal), 1.0));


	// vec4 lightEyePos = mV * mL * vec4(lightPosition,1); 
	// vec3 lightDir = lightEyePos.xyz - vPosition;

	// float NdotWDivR = max(dot(normalize(normal), normalize(lightDir)), 0.0) / dot(lightDir, lightDir);

	// vec3 lightDirScaled = (1.0 + normalize(lightDir)) * 0.5;

	// lightDirColor = vec4(lightDirScaled, NdotWDivR);
}
