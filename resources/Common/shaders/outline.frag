#version 330 

uniform sampler2D sobelDepthX; 
uniform sampler2D sobelDepthY; 
uniform sampler2D sobelNormsX; 
uniform sampler2D sobelNormsY; 
uniform sampler2D unoutlinedTex; 

// const vec3 sobelThreshold = vec3(0.8,0.8,0.8);
const vec3 sobelThreshold = vec3(0.01,0.01,0.01);

in vec2 geom_texCoord;
out vec4 fragColor;

vec3 sobel(sampler2D x, sampler2D y) {
    return sqrt(pow(texture(x, geom_texCoord).xyz, vec3(2,2,2)) + pow(texture(y, geom_texCoord).xyz, vec3(2,2,2)));
}


void main() {
    // fragColor = vec4(abs(texture(sobelX, geom_texCoord).xyz), 1.0);
    // return;

    vec3 sobelDepthVec = sobel(sobelDepthX, sobelDepthY);
    vec3 sobelNormsVec = sobel(sobelNormsX, sobelNormsY);
    float sobelDepth = sobelDepthVec.x;
    float sobelNorms = sobelNormsVec.x + sobelNormsVec.y + sobelNormsVec.z;

    const float DEPTH_THRESHOLD = 0.01;
    const float NORMS_THRESHOLD = 1.25;

    float edge = (sobelDepth > DEPTH_THRESHOLD || sobelNorms > NORMS_THRESHOLD) ? 0.0 : 1.0;

    // vec3 sobelDepth = sqrt(pow(texture(sobelDepthX, geom_texCoord).xyz, vec3(2,2,2)) + pow(texture(sobelY, geom_texCoord).xyz, vec3(2,2,2)));
    // float edge = sobelValue.x > sobelThreshold.x ? 0.0 : 1.0;
    // fragColor = vec4(sobelValue, 1.0);

    fragColor = vec4(edge * texture(unoutlinedTex, geom_texCoord).xyz, 1.0);

    //fragColor = vec4(textureLod(blurredTextureMipMap, geom_texCoord, level).rgb, 1.0);
    // fragColor = vec4(texture(blurredTextureMipMap, geom_texCoord).rgb, 1.0);

}
