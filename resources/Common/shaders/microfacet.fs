#version 330

// This is a shader code fragment (not a complete shader) that contains 
// the functions to evaluate the microfacet BRDF.

const float PI = 3.14159265358979323846264;

uniform mat4 mL;
uniform mat4 mV; 
uniform vec3 lightPosition; //light pos in world space 
uniform vec3 lightPower; 
uniform float refractiveIndex;
uniform float roughness;
uniform vec3 diffuseReflectance; 

in vec3 vPosition;
in vec3 vNormal;
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
  vec3 normal = normalize((gl_FrontFacing) ? vNormal : -vNormal);
  // float NdotH = max(dot(normalize(normal), normalize(lightDir)), 0.0);

  vec3 lightPos = (mV * mL * vec4(lightPosition, 1.0)).xyz;

  vec3 lightDir = lightPos - vPosition;
  vec3 viewDir = -vPosition;
  float lightToPtSqrDist = dot(lightDir, lightDir);

  lightDir = normalize(lightDir);
  viewDir = normalize(viewDir);

  float NdotW = max(dot(normal, lightDir), 0.0);

  vec3 lightIrr = lightPower / (4.0 * PI);

  vec3 brdf = (diffuseReflectance / PI) + isotropicMicrofacet(lightDir, viewDir, normal, refractiveIndex, roughness);

  // fragColor = sRGB(vec4(refractiveIndex / 10, 0, 0, 1));

  fragColor = sRGB(vec4(lightIrr * brdf * (NdotW / lightToPtSqrDist), 1.0));
}
