#version 330 core

out vec3 color;

uniform vec3 g_color;

void main(){
    color = g_color;
}