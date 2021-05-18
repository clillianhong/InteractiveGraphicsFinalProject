#version 330

uniform mat4 mM;  // Model matrix
uniform mat4 mV;  // View matrix
uniform mat4 mP;  // Projection matrix

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 norm;

out vec3 vNormal;    // vertex normal in eye space
out vec3 vPosition;  // vertex position in eye space

void main()
{
    vNormal = normalize((mV * mM * vec4(norm, 0.0)).xyz);
    vPosition = (mV * mM * vec4(position, 1.0)).xyz;
    gl_Position = mP * mV * mM * vec4(position, 1.0);
}
