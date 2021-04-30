#version 330 core

flat in vec3 normal;
out vec4 out_colour;

uniform vec3 voxel_colour;
uniform vec3 SunlightDirection = vec3(0.5, -2, -1);

void main(){
    float brightness = 0.8 + 0.2 * dot(normal, -normalize(SunlightDirection));
    out_colour = vec4(brightness * voxel_colour, 1.0);
}
