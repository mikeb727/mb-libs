#version 330 core
layout (location = 0) in vec4 bufpos;

uniform mat4 transform;

out vec2 texCoords;

void main(){
    gl_Position = transform * vec4(bufpos.xy, 0.0, 1.0);
    texCoords = bufpos.zw;
}