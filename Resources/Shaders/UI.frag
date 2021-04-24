#version 330

in vec2 offset;

uniform sampler2D ui_canvas;

void main() {
    gl_FragColor = texture(ui_canvas, offset);
}
