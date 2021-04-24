#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;

out vec2 offset;

uniform float ScreenRatio;

void main() {
    gl_Position = vec4(position, 1.0);
    gl_Position.x /= ScreenRatio;
    offset = texcoord;
}
