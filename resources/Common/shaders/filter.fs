#version 330

const float PI = 3.14159265358979323846264;
const float INV_SQRT_TWOPI = 1.0 / sqrt(2 * PI);

uniform sampler2D image;

// uniform float stdev = 6;
uniform vec2 dir = vec2(1.0, 0.0);
// uniform int level = 0;
// uniform float weight = 1.0;

uniform bool isActive = true;

uniform int convFilter[3];

const int RADIUS = 1;

in vec2 geom_texCoord;

out vec4 fragColor;

// float gaussianWeight(int r) {
//     return exp(-r*r/(2.0*stdev*stdev)) * INV_SQRT_TWOPI / stdev;
// }

void main() {
    vec4 color;

    if (isActive) {
        vec2 d = 1.0 / textureSize(image, 0);
        vec3 s = vec3(0);
        for (int i = -RADIUS; i <= RADIUS; i++) {
            float w = convFilter[i + 1];
            s += w * texture(image, geom_texCoord + d * i).rgb;
            // s += w * texture(image, geom_texCoord + d * i * dir).rgb;
        }
        color = vec4(s, 1.0);
    } else {
        color = texture(image, geom_texCoord);
    }
    fragColor = color;
    // fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}