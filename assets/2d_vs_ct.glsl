R"(
#version 330 core
layout (location = 0) in vec4 bufpos;

uniform mat4 transform;
uniform float drawDepth;

out vec2 texCoords;

void main(){
    gl_Position = transform * vec4(bufpos.xy, drawDepth, 1.0);
    texCoords = bufpos.zw;
}
)"