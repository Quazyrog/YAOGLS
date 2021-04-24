#version 330 core

in vec3 normal;
out vec4 out_colour;

uniform vec3 voxel_colour;
uniform vec3 SunlightDirection = vec3(0.3, -1, -2);

void main(){
    out_colour = vec4((0.8 + 0.2 * dot(normal, SunlightDirection)) * voxel_colour, 1.0);
}
