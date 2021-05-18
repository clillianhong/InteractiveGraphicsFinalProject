#version 330 

uniform sampler2D sobelX; 
uniform sampler2D sobelY; 
uniform sampler2D unoutlinedTex; 

const vec3 sobelThreshold = vec3(0.8,0.8,0.8);

in vec2 geom_texCoord;
out vec4 fragColor;


void main() {
    vec3 sobelValue = sqrt(pow(texture(sobelX, geom_texCoord).xyz, vec3(2,2,2)) + pow(texture(sobelY, geom_texCoord).xyz, vec3(2,2,2)));
    float edge = sobelValue.x > sobelThreshold.x ? 0.0 : 1.0;
    fragColor = vec4(sobelValue, 1.0);

    fragColor = vec4(edge * texture(unoutlinedTex, geom_texCoord).xyz, 1.0);

    //fragColor = vec4(textureLod(blurredTextureMipMap, geom_texCoord, level).rgb, 1.0);
    // fragColor = vec4(texture(blurredTextureMipMap, geom_texCoord).rgb, 1.0);

}
