#version 330 core

out vec4 out_colour;

uniform vec3 voxel_colour;

void main(){
    out_colour = vec4(voxel_colour, 1.0);
}