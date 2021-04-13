#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_colour;

out vec3 vert_colour;

uniform float time;

void main()
{
    gl_Position.xyz = in_position;
    gl_Position.w = 1.0 / (0.1 + time);
    vert_colour = in_colour * time;
}
