#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in int in_face_id;
layout(location = 2) in vec2 in_texcoord;
flat out int face_id;
out vec2 texcoord;

uniform vec3 Position;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;


void main()
{
    gl_Position =  ProjectionMatrix * ViewMatrix * (ModelMatrix * vec4(in_position, 1) + vec4(Position, 0));
    face_id = in_face_id;
    texcoord = in_texcoord;
}
