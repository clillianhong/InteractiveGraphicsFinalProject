#version 330

const float PI = 3.14159265358979323846264;

// Sky stuff
uniform vec3 A, B, C, D, E, zenith;
uniform float thetaSun = 60 * PI/180, phiSun = PI;
const float sunAngularRadius = 0.5 * PI/180;
const vec3 solarDiscRadiance = vec3(10000);
const vec3 groundRadiance = vec3(0.5);
const float skyScale = 0.06;

// Our stuff
uniform mat4 mProjInv; 
uniform mat4 mViewInv; 

in vec2 geom_texCoord;
out vec4 fragColor;

// Sky funcs
vec3 perez(float theta, float gamma) {
    return (1 + A * exp(B / cos(theta))) * (1 + C * exp(D * gamma) + E * pow(cos(gamma), 2.0));
}

vec3 sunRadiance(vec3 dir) {
    vec3 sunDir = vec3(sin(thetaSun) * cos(phiSun), cos(thetaSun), sin(thetaSun) * sin(phiSun));
    return dot(dir, sunDir) > cos(sunAngularRadius) ? solarDiscRadiance : vec3(0);
}

const mat3 XYZ2RGB = mat3(
   3.2404542, -0.969266 ,  0.0556434,
  -1.5371385,  1.8760108, -0.2040259,
  -0.4985314,  0.041556 ,  1.0572252
);

vec3 skyRadiance(vec3 dir) {
    vec3 sunDir = vec3(sin(thetaSun) * cos(phiSun), cos(thetaSun), sin(thetaSun) * sin(phiSun));
    float gamma = acos(min(1.0, dot(dir, sunDir)));
    if (dir.y > 0) {
        float theta = acos(dir.y);
        vec3 Yxy = zenith * perez(theta, gamma) / perez(0, thetaSun);
        return skyScale * XYZ2RGB * vec3(Yxy[1] * (Yxy[0]/Yxy[2]), Yxy[0], (1 - Yxy[1] - Yxy[2])*(Yxy[0]/Yxy[2]));
    } else {
        return groundRadiance;
    }
}

vec3 sunskyRadiance(vec3 dir) {
    return sunRadiance(dir) + skyRadiance(dir);
}

void main() {
    vec4 front = vec4(geom_texCoord, 1.0, 1.0);
    vec4 back = vec4(geom_texCoord, -1.0, 1.0);
    
    front = mProjInv * front;
    front /= front.w;
    front = mViewInv * front;

    back = mProjInv * back;
    back /= back.w;
    back = mViewInv * back;

    vec3 dir = normalize(front.xyz - back.xyz);

    // fragColor = vec4(normalize(dir), 1.0);

    fragColor = vec4(sunskyRadiance(dir), 1.0);
}