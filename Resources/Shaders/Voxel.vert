#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in int in_face_id;
flat out vec3 normal;

uniform vec3 Position;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

vec3 FaceNormal(int face_id)
{
    switch (face_id) {
    case 0: return vec3( 0,  0, -1);
    case 1: return vec3( 0,  0,  1);
    case 2: return vec3(-1,  0,  0);
    case 3: return vec3( 1,  0,  0);
    case 4: return vec3( 0, -1,  0);
    case 5: return vec3( 0,  1,  0);
    }
}

void main()
{
    gl_Position =  ProjectionMatrix * ViewMatrix * (ModelMatrix * vec4(in_position, 1) + vec4(Position, 0));
    normal = FaceNormal(in_face_id);
}
