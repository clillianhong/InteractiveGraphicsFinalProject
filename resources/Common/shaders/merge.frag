#version 330 

//uniform sampler2D originalTexture;
uniform sampler2D blurredTextureMipMap; 

const float weights[5] = float[5](0.8843, 0.1, 0.012, 0.0027, 0.001);
const int levels[5] = int[5](0, 1, 2, 3, 4);

in vec2 geom_texCoord;
out vec4 fragColor;



void main() {
    vec3 color = weights[0] * textureLod(blurredTextureMipMap, geom_texCoord, levels[0]).rgb; 
   // vec3 color = vec3(0,0,0);

    for(int i = 1; i < 5; i++) {
       color += weights[i] * textureLod(blurredTextureMipMap, geom_texCoord, levels[i]).rgb; 
   }

    fragColor = vec4(color, 1.0);

    //fragColor = vec4(textureLod(blurredTextureMipMap, geom_texCoord, level).rgb, 1.0);
    // fragColor = vec4(texture(blurredTextureMipMap, geom_texCoord).rgb, 1.0);

}
