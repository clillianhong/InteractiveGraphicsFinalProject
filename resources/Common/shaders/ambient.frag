#version 330

const float PI = 3.14159265358979323846264;

in vec2 geom_texCoord;

out vec4 fragColor;

// Ambient
uniform vec3 ambientRadiance;
uniform float occlusionRange;

uniform sampler2D diffuseReflectance;
uniform sampler2D shaderParamTexture;
uniform sampler2D normalTexture; 
uniform sampler2D depthTexture;

uniform mat4 mProjInv;
uniform mat4 mProj;
uniform mat4 mViewInv;

// Sky stuff
uniform vec3 A, B, C, D, E, zenith;
uniform float thetaSun = 60 * PI/180, phiSun = PI;
const float sunAngularRadius = 0.5 * PI/180;
const vec3 solarDiscRadiance = vec3(10000);
const vec3 groundRadiance = vec3(0.5);
const float skyScale = 0.06;

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

// Ambient shading stuff

// These are from RTUtils, which are given in the provided code
vec2 squareToUniformDiskConcentric(vec2 sample) {
    float r1 = 2.0f*sample.x - 1.0f;
    float r2 = 2.0f*sample.y - 1.0f;

    /* Modified concencric map code with less branching (by Dave Cline), see
       http://psgraphics.blogspot.ch/2011/01/improved-code-for-concentric-map.html */
    float phi, r;
    if (r1 == 0 && r2 == 0) {
        r = phi = 0;
    } else if (r1*r1 > r2*r2) {
        r = r1;
        phi = (PI/4.0f) * (r2/r1);
    } else {
        r = r2;
        phi = (PI/2.0f) - (r1/r2) * (PI/4.0f);
    }

    float cosPhi = cos(phi);
    float sinPhi = sin(phi);

    return vec2(r * cosPhi, r * sinPhi);
}

// These are from RTUtils, which are given in the provided code
vec3 squareToCosineHemisphere(vec2 sample) {
    vec2 p = squareToUniformDiskConcentric(sample);
    float z = sqrt(max(1.0f - p.x*p.x - p.y*p.y, 0.0));

    /* Guard against numerical imprecisions */
    if (z == 0) z = 1e-10f;

    return vec3(p.x, p.y, z);
}

float random(vec2 co) {
    return fract(sin(dot(co.xy,vec2(12.9898,78.233))) * 43758.5453);
}

float fractionOccluded(mat4 frameToEye, int numSamples, vec3 eyePos, float occlusionRange) {
    int numOccluded = 0; 

    for (int x = 0; x < numSamples; x++){
        
        vec2 sqrPt = vec2(
            random(geom_texCoord + vec2(x, -1)),
            random(geom_texCoord + vec2(-1, x))
        );
        vec3 sampleDir = squareToCosineHemisphere(sqrPt); 
        sampleDir = normalize(sampleDir);

        // Scale the direction based on an r^2 distribution between 0 and the range
        float r = random(geom_texCoord + vec2(x, x));
        r = r * r;
        r *= occlusionRange;

        vec3 samplePt = sampleDir * r; 

        vec4 ptInFrame = vec4(samplePt, 1.0);
        vec4 ptInEye = frameToEye * ptInFrame;
        vec4 ptInScreen = mProj * ptInEye;

        ptInScreen /= ptInScreen.w; 
        ptInScreen = (ptInScreen / 2.0) + 0.5;
        float actualDepth = texture(depthTexture, ptInScreen.xy).x;
        float ptDepth = ptInScreen.z; 

        vec4 occludingPtNDC = vec4(vec3(ptInScreen.xy, texture(depthTexture, ptInScreen.xy).x) * 2.0 - 1.0, 1.0);
        vec4 occludingPt = mProjInv * occludingPtNDC;
        occludingPt /= occludingPt.w;
        
        if (distance(ptInEye.xyz, occludingPt.xyz) > 4 * occlusionRange) {
            continue;
        }
        // if (distance(vec3(ptInScreen), vec3(ptInScreen.xy, texture(depthTexture, ptInScreen.xy))) > 0.005 * occlusionRange) {
        //     continue;
        // }

        if (ptDepth > actualDepth + 0.00001 /*&& ptDepth - actualDepth < 0.005 * occlusionRange*/) {
            numOccluded++; 
        }
    }

    return float(numOccluded) / float(numSamples);
}

// From Xi Deng in office hours
vec3 base(vec3 normal) {
    vec3 base;
    if(abs(normal.x) > abs(normal.y)){
        float invLen = 1.0 / sqrt(normal.x * normal.x + normal.z * normal.z);
        base = vec3(normal.z * invLen, 0.f, -normal.x * invLen);
    } else {
        float invLen = 1.0 / sqrt(normal.y * normal.y + normal.z * normal.z);
        base = vec3(0.f, normal.z * invLen, -normal.y * invLen);
    }
    return base;
}

void main() {
    // If the point is on the surface or not
    if (texture(shaderParamTexture, geom_texCoord).b < 0.5) {
        // Draw the sky if it is not on the surface
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
        // fragColor = vec4(0, 0, 0, 1.0);
    
    } else {

        // fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        // return;

        // Do ambient shading if it is on the surface
        vec3 normal = normalize((2.0 * texture(normalTexture, geom_texCoord).xyz) - 1.0); 

        //create the coordinate frame
        vec3 nonParallel;
        
        // float absX = abs(normal.x), absY = abs(normal.y), absZ = abs(normal.z); 

        // if (absX <= absY && absX <= absZ) {
        //     nonParallel = vec3(1.0, 0.0, 0.0);
        // } else if (absZ <= absY && absZ <= absX) {
        //     nonParallel = vec3(0, 0, 1.0);
        // } else {
        //     nonParallel = vec3(0, 1.0, 0);
        // }

        nonParallel = base(normal);

        // if (normal.x <= normal.y && normal.x <= normal.z) {
        //     nonParallel = vec3(1.0, 0.0, 0.0);
        // } else if (normal.z <= normal.y && normal.z <= normal.x) {
        //     nonParallel = vec3(0, 0, 1.0);
        // } else {
        //     nonParallel = vec3(0, 1.0, 0);
        // }

        vec3 zAxis = normal; // Z axis must be pointing up
        vec3 yAxis = normalize(cross(zAxis, nonParallel));
        vec3 xAxis = normalize(cross(zAxis, yAxis));

        // eye position 
        vec4 positionNDC = vec4(vec3(geom_texCoord, texture(depthTexture, geom_texCoord).x) * 2.0 - 1.0, 1.0);
        vec4 eyePos = mProjInv * positionNDC;
        eyePos /= eyePos.w;
        vec3 vEyePosition = eyePos.xyz;

        mat4 frameToEye;
        frameToEye[0] = vec4(xAxis, 0.0);
        frameToEye[1] = vec4(yAxis, 0.0);
        frameToEye[2] = vec4(zAxis, 0.0);
        frameToEye[3] = vec4(vEyePosition, 1.0);

        float fracOcc = fractionOccluded(frameToEye, 20, vEyePosition, occlusionRange);
        // fragColor = vec4(fracOcc, 0,0,1);
        fragColor = (1.f - fracOcc) * vec4(ambientRadiance, 1.0) * texture(diffuseReflectance, geom_texCoord) * PI;

        // fragColor = vec4(zAxis, 1.f);

        // vec4 examplePointInFrame = vec4(0.0, 0.0, 0.0, 1.0);
        // vec4 exPtInEye = frameToEye * examplePointInFrame;
        // vec4 exPtInScreen = mProj * exPtInEye;
        // exPtInScreen /= exPtInScreen.w; 
        // exPtInScreen = (exPtInScreen / 2) + 0.5;

        // float actualDepth = texture(depthTexture, exPtInScreen.xy).x;
        // float ptDepth = exPtInScreen.z; 

        // if (ptDepth > actualDepth + 0.00001) {
        //     fragColor = vec4(0.0, 0.0, 0.0, 1.0); 
        // } else {
        //     fragColor = vec4(ambientRadiance, 1.0) * texture(diffuseReflectance, geom_texCoord) * PI;
        // }
    }
}