R"(
#version 330 core

in vec2 texCoords;

uniform bool useTex;
uniform sampler2D tex;
uniform vec4 color;

out vec4 fragColor;

void main(){
    fragColor = color * vec4(1.0, 1.0, 1.0, useTex ? texture(tex, texCoords).r : 1.0);
}
)"