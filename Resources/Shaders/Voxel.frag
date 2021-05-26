#version 330 core

flat in int face_id;
out vec4 out_colour;

uniform vec3 SunlightDirection = vec3(0.5, -2, -1);
uniform sampler2DArray AtlasArray;
uniform int FaceTextures[6];

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
    return vec3(0);
}


void main(){
    vec3 normal = FaceNormal(face_id);
    vec3 color = texture(AtlasArray, vec3(0.5, 0.5, 3)).rgb;
    float brightness = 0.8 + 0.2 * dot(normal, -normalize(SunlightDirection));
    out_colour = vec4(brightness * color, 1.0);
}
