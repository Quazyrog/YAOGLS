#version 330

layout(location = 0) in vec2 position;

out vec3 color;

uniform vec3 Position;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
    vec4 wspos;
    wspos.x = 0.48 * position.x + 0.5;
    wspos.y = 0.05;
    wspos.z = 0.48 * position.y + 0.5;
    wspos.w = 1;
    wspos.xyz += Position;
    gl_Position = ProjectionMatrix * ViewMatrix * wspos;
    color = vec3(0.5);
}
