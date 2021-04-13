#version 330 core
in vec2 texCoords;
out vec4 colour;

uniform sampler2D tex;

void main()
{
    colour = texture(tex, texCoords);
}