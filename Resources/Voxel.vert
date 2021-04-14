#version 330 core

layout(location = 0) in vec3 in_position;
out vec3 cc;

uniform vec3 Position;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
    gl_Position =  ProjectionMatrix * ViewMatrix * (ModelMatrix * vec4(in_position, 1) + vec4(Position, 0));
}
