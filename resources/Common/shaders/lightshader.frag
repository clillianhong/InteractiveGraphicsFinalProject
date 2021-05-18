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
// uniform float refractiveIndex;
// uniform float roughness;
// uniform vec3 diffuseReflectance;

uniform sampler2D diffuseTexture; 
uniform sampler2D shaderParamTexture;
uniform sampler2D normalTexture; 
uniform sampler2D depthTexture;
uniform sampler2D shadowMapTexture;
// uniform sampler2D lightDirTexture; 

in vec2 geom_texCoord;

// in vec3 vPosition;
// in vec3 vNormal;
out vec4 fragColor;



float sRGBSingle(float c) {
    float a = 0.055;	
    if (c <= 0) 
        return 0;		
    else if (c < 0.0031308) {
        return 12.92*c;
    } else {
        if (c >= 1.0)
          return 1.0;
        else 
            return (1.0+a)*pow(c, 1.0/2.4)-a;
    }
}

vec4 sRGB(vec4 c) {
	  return vec4(sRGBSingle(c.r), sRGBSingle(c.g), sRGBSingle(c.b), c.a);
}

// The Fresnel reflection factor
//   i -- incoming direction
//   m -- microsurface normal
//   eta -- refractive index
float fresnel(vec3 i, vec3 m, float eta) {
    float c = abs(dot(i,m));
    float g = sqrt(eta*eta - 1.0 + c*c);

    float gmc = g-c;
    float gpc = g+c;
    float nom = c*(g+c)-1.0;
    float denom = c*(g-c)+1.0;
    return 0.5*gmc*gmc/gpc/gpc*(1.0 + nom*nom/denom/denom);
}

// The one-sided Smith shadowing/masking function
//   v -- in or out vector
//   m -- microsurface normal
//   n -- (macro) surface normal
//   alpha -- surface roughness
float G1(vec3 v, vec3 m, vec3 n, float alpha) {
    float vm = dot(v,m);
    float vn = dot(v,n);
    if (vm*vn > 0.0) {
        float cosThetaV = dot(n,v);
        float sinThetaV2 = 1.0 - cosThetaV*cosThetaV;
        float tanThetaV2 = sinThetaV2 / cosThetaV / cosThetaV;
        return 2.0 / (1.0 + sqrt(1.0 + alpha*alpha*tanThetaV2));
    } else {
        return 0;
    }
}

// The GGX slope distribution function
//   m -- microsurface normal
//   n -- (macro) surface normal
//   alpha -- surface roughness
float D(vec3 m, vec3 n, float alpha) {
    float mn = dot(m,n);
    if (mn > 0.0) {
        float cosThetaM = mn;
        float cosThetaM2 = cosThetaM*cosThetaM;
        float tanThetaM2 = (1.0 - cosThetaM2) / cosThetaM2;
        float cosThetaM4 =  cosThetaM*cosThetaM*cosThetaM*cosThetaM;
        float X = (alpha*alpha + tanThetaM2);
        return alpha*alpha / (PI * cosThetaM4 * X * X);
    } else {
        return 0.0;
    }
}

// Evalutate the Microfacet BRDF (GGX variant) for the paramters:
//   i -- incoming direction (unit vector, pointing away from surface)
//   o -- outgoing direction (unit vector, pointing away from surface)
//   n -- outward pointing surface normal vector
//   eta -- refractive index
//   alpha -- surface roughness
// return: scalar BRDF value
float isotropicMicrofacet(vec3 i, vec3 o, vec3 n, float eta, float alpha) {

    float odotn = dot(o,n);
    vec3 m = normalize(i + o);

    float idotn = dot(i,n);
    if (idotn <= 0.0 || odotn <= 0.0)
        return 0;

    float idotm = dot(i,m);
    float F = (idotm > 0.0) ? fresnel(i,m,eta) : 0.0;
    float G = G1(i,m,n,alpha) * G1(o,m,n,alpha);
    return F * G * D(m,n,alpha) / (4.0*idotn*odotn);
}


void main() {
    // Extract values from previous pass from framebuffer textures
    vec3 normal = normalize((2.0 * texture(normalTexture, geom_texCoord).xyz) - 1.0); 
    vec3 diffuseReflectance = texture(diffuseTexture, geom_texCoord).xyz; 
    vec4 params = texture(shaderParamTexture, geom_texCoord); 
    float refractiveIndex = params.x * 20.0f; 
    float roughness = params.y;
    float surfaceHit = params.z;
    
    // normalize((gl_FrontFacing) ? vNormal : -vNormal);

    // From the FSQ texture coordinates and the depth stored in the G-buffer, get the NDC coordinates of the fragment by scaling from [0,1] to [-1,1].
    vec4 positionNDC = vec4(vec3(geom_texCoord, texture(depthTexture, geom_texCoord)) * 2.0 - 1.0, 1.0);
    vec4 eyePos = mProjInv * positionNDC;
    eyePos /= eyePos.w;
    vec3 vEyePosition = eyePos.xyz;
    // vec3 vPosition = vec3(positionEye);


    //get vEyePosition by recovering NDC coordinates https://stackoverflow.com/questions/11277501/how-to-recover-view-space-position-given-view-space-depth-value-and-ndc-xy
    // vec3 ndc = vec3(geom_texCoord, texture(depthTexture, geom_texCoord).x) * 2.0 - 1.0; 
    // float A     = mProj[2][2];
    // float B     = mProj[3][2];
    // float z_eye = B / (A + ndc.z);
    // float x_eye = z_eye * ndc.x / mProj[0][0];
    // float y_eye = z_eye * ndc.y / mProj[1][1];
    // vec3 vEyePosition = vec3(x_eye, y_eye, -z_eye); // eye space position

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
    // fragColor = vec4(fragment_depth, 0,0,1);
    // return;

   
    float isBg = (surfaceHit != 1.0) ? 0 : 1;
    if (inShadowView && fragment_depth > shadow_map_depth + 0.005) {
        color = vec3(0,0,0); 
    } else if (isBg > 0.5) {
        //old light dir code
        vec4 lightPos4 = (mV * mL * vec4(lightPosition, 1.0));
        vec3 lightPos = (lightPos4).xyz;
        // // vec3 lightDir = normalize(vec3(1,1,1));
        vec3 lightDir = lightPos - vEyePosition;

        //grab from lightDir texture where xyz is the normalized lightDir and NdotWDivR = max(dot(normal, lightDir), 0.0) / dot(lightDir, lightDir)
        // vec4 lightDirTex = texture(lightDirTexture, geom_texCoord);
        // vec3 lightDir = lightDirTex.xyz * 2.0 - 1.0; 
        // float NdotWDivR = lightDirTex.w;
        
        vec3 viewDir = -vEyePosition;
        viewDir = normalize(viewDir);
        
        float lightToPtSqrDist = dot(lightDir, lightDir);
        lightDir = normalize(lightDir);
        float NdotW = max(dot(normal, lightDir), 0);
        
        vec3 lightIrr = lightPower / (4.0 * PI);
        vec3 brdf = (diffuseReflectance / PI) + isotropicMicrofacet(lightDir, viewDir, normal, refractiveIndex, roughness);

        color = lightIrr * brdf * (NdotW / lightToPtSqrDist);
        //color = vec3(0,0,0);
        //color = vec3(isotropicMicrofacet(lightDir, viewDir, normal, refractiveIndex, roughness));
    } else {
        color = vec3(0, 0, 0);
    }
    fragColor = vec4(color, isBg);

    // isBg = surfaceHit;
    // fragColor = vec4(lightIrr * brdf * (NdotWDivR), isBg);
    // fragColor = vec4(refractiveIndex / 10, 0, 0, 1);
    // fragColor = sRGB(vec4(lightIrr * brdf * (NdotW / lightToPtSqrDist), 1.0));

    // lambertian shading for sanity check
    // float NdotH = max(dot(normalize(normal), normalize(lightDir)), 0.0);
    // fragColor = vec4(vec3((0.1, 0.1, 0.1)) + NdotH * vec3(0.9, 0.9, 0.9), 1.0); 
}
