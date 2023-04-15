#version 330 core

in vec2 texCoords;

uniform vec4 color;

out vec4 fragColor;

void main(){
    fragColor = color;
}