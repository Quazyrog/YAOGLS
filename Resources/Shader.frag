#version 330 core

in vec3 vert_colour;
out vec4 out_colour;

void main(){
    out_colour = vec4(vert_colour, 1.0);
}