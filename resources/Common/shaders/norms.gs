#version 330

layout (triangles) in;
layout (line_strip, max_vertices = 2) out;

in vec3 vNormal;   // in eye space

void main() {
    vNormal = normalize(cross(vPosition[1] - vPosition[0], vPosition[2] - vPosition[0]));
    for (int i = 0; i < gl_in.length(); i++) {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();


}